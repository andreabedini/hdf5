/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:		H5HFhuge.c
 *			Aug  7 2006
 *			Quincey Koziol <koziol@hdfgroup.org>
 *
 * Purpose:		Routines for "huge" objects in fractal heap
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#define H5HF_PACKAGE		/*suppress error about including H5HFpkg  */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5HFpkg.h"		/* Fractal heaps			*/
#include "H5MFprivate.h"	/* File memory management		*/
#include "H5MMprivate.h"	/* Memory management			*/


/****************/
/* Local Macros */
/****************/

/* v2 B-tree creation macros */
#define H5HF_HUGE_BT2_NODE_SIZE         512
#define H5HF_HUGE_BT2_SPLIT_PERC        100
#define H5HF_HUGE_BT2_MERGE_PERC         40


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/

/* Local v2 B-tree operations */
static herr_t H5HF_huge_bt2_create(H5HF_hdr_t *hdr, hid_t dxpl_id);

/* v2 B-tree function callbacks (in H5HFbtree2.c) */
herr_t H5HF_huge_bt2_indir_found(const void *nrecord, void *op_data);
herr_t H5HF_huge_bt2_indir_remove(const void *nrecord, void *op_data);
herr_t H5HF_huge_bt2_filt_indir_found(const void *nrecord, void *op_data);
herr_t H5HF_huge_bt2_filt_indir_remove(const void *nrecord, void *op_data);
herr_t H5HF_huge_bt2_dir_remove(const void *nrecord, void *op_data);
herr_t H5HF_huge_bt2_filt_dir_found(const void *nrecord, void *op_data);
herr_t H5HF_huge_bt2_filt_dir_remove(const void *nrecord, void *op_data);

/* Local 'huge' object support routines */
static hsize_t H5HF_huge_new_id(H5HF_hdr_t *hdr);
static herr_t H5HF_huge_op_real(H5HF_hdr_t *hdr, hid_t dxpl_id,
    const uint8_t *id, hbool_t is_read, H5HF_operator_t op, void *op_data);

/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_bt2_create
 *
 * Purpose:	Create the v2 B-tree for tracking the huge objects in the heap
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug  7 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5HF_huge_bt2_create(H5HF_hdr_t *hdr, hid_t dxpl_id)
{
    const H5B2_class_t *bt2_class;      /* v2 B-tree class to use */
    size_t rrec_size;                   /* Size of 'raw' records on disk */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_bt2_create)

    /*
     * Check arguments.
     */
    HDassert(hdr);

    /* Compute the size of 'raw' records on disk */
    /* (Note: the size for huge IDs could be set to 'huge_id_size', instead
     *  of 'sizeof_size', but that would make the v2 B-tree callback routines
     *  depend on the heap header, which makes the v2 B-tree flush routines
     *  difficult to write.  "Waste" an extra byte or for small heaps (where
     *  the 'huge_id_size' is < 'sizeof_size' in order to make this easier -QAK)
     */
    if(hdr->huge_ids_direct) {
        if(hdr->filter_len > 0) {
            rrec_size = hdr->sizeof_addr    /* Address of object */
                + hdr->sizeof_size          /* Length of object */
                + 4                         /* Filter mask for filtered object */
                + hdr->sizeof_size;         /* Size of de-filtered object in memory */
            bt2_class = H5HF_BT2_FILT_DIR;
        } /* end if */
        else {
            rrec_size = hdr->sizeof_addr    /* Address of object */
                + hdr->sizeof_size;         /* Length of object */
            bt2_class = H5HF_BT2_DIR;
        } /* end else */
    } /* end if */
    else {
        if (hdr->filter_len > 0) {
            rrec_size = hdr->sizeof_addr    /* Address of filtered object */
                + hdr->sizeof_size          /* Length of filtered object */
                + 4                         /* Filter mask for filtered object */
                + hdr->sizeof_size          /* Size of de-filtered object in memory */
                + hdr->sizeof_size;         /* Unique ID for object */
            bt2_class = H5HF_BT2_FILT_INDIR;
        } /* end if */
        else {
            rrec_size = hdr->sizeof_addr    /* Address of object */
                + hdr->sizeof_size          /* Length of object */
                + hdr->sizeof_size;         /* Unique ID for object */
            bt2_class = H5HF_BT2_INDIR;
        } /* end else */
    } /* end else */

    /* Create v2 B-tree for tracking 'huge' objects */
    if(H5B2_create(hdr->f, dxpl_id, bt2_class, (size_t)H5HF_HUGE_BT2_NODE_SIZE, rrec_size,
            H5HF_HUGE_BT2_SPLIT_PERC, H5HF_HUGE_BT2_MERGE_PERC, &hdr->huge_bt2_addr/*out*/) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_CANTCREATE, FAIL, "can't create v2 B-tree for tracking 'huge' heap objects")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_bt2_create() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_init
 *
 * Purpose:	Initialize information for tracking 'huge' objects
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug  7 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_init(H5HF_hdr_t *hdr)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5HF_huge_init)

    /*
     * Check arguments.
     */
    HDassert(hdr);

    /* Compute information about 'huge' objects for the heap */

    /* Check if we can completely hold the 'huge' object's offset & length in
     *  the file in the heap ID (which will speed up accessing it) and we don't
     *  have any I/O pipeline filters.
     */
#ifdef QAK
HDfprintf(stderr, "%s: hdr->id_len = %u\n", "H5HF_huge_init", (unsigned)hdr->id_len);
HDfprintf(stderr, "%s: hdr->filter_len = %u\n", "H5HF_huge_init", (unsigned)hdr->filter_len);
#endif /* QAK */
    if(hdr->filter_len > 0) {
        if((hdr->id_len - 1) >= (hdr->sizeof_addr + hdr->sizeof_size + 4 + hdr->sizeof_size)) {
            /* Indicate that v2 B-tree doesn't have to be used to locate object */
            hdr->huge_ids_direct = TRUE;

            /* Set the size of 'huge' object IDs */
            hdr->huge_id_size = hdr->sizeof_addr + hdr->sizeof_size + hdr->sizeof_size;
        } /* end if */
        else
            /* Indicate that v2 B-tree must be used to access object */
            hdr->huge_ids_direct = FALSE;
    } /* end if */
    else {
        if((hdr->sizeof_addr + hdr->sizeof_size) <= (hdr->id_len - 1)) {
            /* Indicate that v2 B-tree doesn't have to be used to locate object */
            hdr->huge_ids_direct = TRUE;

            /* Set the size of 'huge' object IDs */
            hdr->huge_id_size = hdr->sizeof_addr + hdr->sizeof_size;
        } /* end if */
        else
            /* Indicate that v2 B-tree must be used to locate object */
            hdr->huge_ids_direct = FALSE;
    } /* end else */
    if(!hdr->huge_ids_direct) {
        /* Set the size and maximum value of 'huge' object ID */
        if((hdr->id_len - 1) < sizeof(hsize_t)) {
            hdr->huge_id_size = hdr->id_len - 1;
            hdr->huge_max_id = ((hsize_t)1 << (hdr->huge_id_size * 8)) - 1;
        } /*end if */
        else {
            hdr->huge_id_size = sizeof(hsize_t);
            hdr->huge_max_id = HSIZET_MAX;
        } /* end else */
    } /* end if */

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5HF_huge_init() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_new_id
 *
 * Purpose:	Determine a new ID for an indirectly accessed 'huge' object
 *              (either filtered or not)
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug 15 2006
 *
 *-------------------------------------------------------------------------
 */
static hsize_t
H5HF_huge_new_id(H5HF_hdr_t *hdr)
{
    hsize_t new_id;             /* New object's ID */
    hsize_t ret_value;          /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_new_id)

    /*
     * Check arguments.
     */
    HDassert(hdr);

    /* Check for wrapping around 'huge' object ID space */
    if(hdr->huge_ids_wrapped)
        /* Fail for now - eventually should iterate through v2 B-tree, looking for available ID */
        HGOTO_ERROR(H5E_HEAP, H5E_UNSUPPORTED, 0, "wrapping 'huge' object IDs not supported yet")
    else {
        /* Get new 'huge' object ID to use for object */
        /* (avoids using ID 0) */
        new_id = ++hdr->huge_next_id;

        /* Check for wrapping 'huge' object IDs around */
        if(hdr->huge_next_id == hdr->huge_max_id)
            hdr->huge_ids_wrapped = TRUE;
    } /* end else */

    /* Set return value */
    ret_value = new_id;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_new_id() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_insert
 *
 * Purpose:	Insert a 'huge' object into the file and track it
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug  7 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_insert(H5HF_hdr_t *hdr, hid_t dxpl_id, size_t obj_size, void *obj,
    void *_id)
{
    uint8_t *id = (uint8_t *)_id;       /* Pointer to ID buffer */
    haddr_t obj_addr;                   /* Address of object in the file */
    void *write_buf;                    /* Pointer to buffer to write */
    size_t write_size;                  /* Size of [possibly filtered] object written to file */
    unsigned filter_mask = 0;           /* Filter mask for object (only used for filtered objects) */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_insert)
#ifdef QAK
HDfprintf(stderr, "%s: obj_size = %Zu\n", FUNC, obj_size);
#endif /* QAK */

    /*
     * Check arguments.
     */
    HDassert(hdr);
    HDassert(obj_size > hdr->max_man_size);
    HDassert(obj);
    HDassert(id);

    /* Check if the v2 B-tree for tracking 'huge' heap objects has been created yet */
    if(!H5F_addr_defined(hdr->huge_bt2_addr))
        if(H5HF_huge_bt2_create(hdr, dxpl_id) < 0)
            HGOTO_ERROR(H5E_HEAP, H5E_CANTCREATE, FAIL, "can't create v2 B-tree for tracking 'huge' heap objects")

    /* Check for I/O pipeline filter on heap */
    if(hdr->filter_len > 0) {
        H5Z_cb_t filter_cb = {NULL, NULL};  /* Filter callback structure */
        size_t nbytes;                      /* Number of bytes used */

        /* Allocate buffer to perform I/O filtering on */
        write_size = obj_size;
        if(NULL == (write_buf = H5MM_malloc(write_size)))
            HGOTO_ERROR(H5E_HEAP, H5E_NOSPACE, FAIL, "memory allocation failed for pipeline buffer")
        HDmemcpy(write_buf, obj, write_size);

        /* Push direct block data through I/O filter pipeline */
        nbytes = write_size;
        if(H5Z_pipeline(&(hdr->pline), 0, &filter_mask, H5Z_NO_EDC,
                 filter_cb, &nbytes, &write_size, &write_buf) < 0)
            HGOTO_ERROR(H5E_HEAP, H5E_CANTFILTER, FAIL, "output pipeline failed")
#ifdef QAK
HDfprintf(stderr, "%s: nbytes = %Zu, write_size = %Zu, write_buf = %p\n", FUNC, nbytes, write_size, write_buf);
HDfprintf(stderr, "%s: obj_size = %Zu, obj = %p\n", FUNC, obj_size, obj);
#endif /* QAK */

        /* Update size of object on disk */
        write_size = nbytes;
    } /* end if */
    else {
        write_buf = obj;
        write_size = obj_size;
    } /* end else */

    /* Allocate space in the file for storing the 'huge' object */
    if(HADDR_UNDEF == (obj_addr = H5MF_alloc(hdr->f, H5FD_MEM_FHEAP_HUGE_OBJ, dxpl_id, (hsize_t)write_size)))
        HGOTO_ERROR(H5E_HEAP, H5E_NOSPACE, FAIL, "file allocation failed for fractal heap huge object")

    /* Write the object's data to disk */
    if(H5F_block_write(hdr->f, H5FD_MEM_FHEAP_HUGE_OBJ, obj_addr, write_size, dxpl_id, write_buf) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_WRITEERROR, FAIL, "writing 'huge' object to file failed")

    /* Release buffer for writing, if we had one */
    if(write_buf != obj) {
        HDassert(hdr->filter_len > 0);
        H5MM_xfree(write_buf);
    } /* end if */

    /* Perform different actions for directly & indirectly accessed 'huge' objects */
    if(hdr->huge_ids_direct) {
        if(hdr->filter_len > 0) {
            H5HF_huge_bt2_filt_dir_rec_t obj_rec;        /* Record for tracking object */

            /* Initialize record for tracking object in v2 B-tree */
            obj_rec.addr = obj_addr;
            obj_rec.len = write_size;
            obj_rec.filter_mask = filter_mask;
            obj_rec.obj_size = obj_size;
#ifdef QAK
HDfprintf(stderr, "%s: obj_rec = {%a, %Hu, %x, %Hu}\n", FUNC, obj_rec.addr, obj_rec.len, obj_rec.filter_mask, obj_rec.obj_size);
#endif /* QAK */

            /* Insert record for object in v2 B-tree */
            if(H5B2_insert(hdr->f, dxpl_id, H5HF_BT2_FILT_DIR, hdr->huge_bt2_addr, &obj_rec) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTINSERT, FAIL, "couldn't insert object tracking record in v2 B-tree")

            /* Encode ID for user */
            *id++ = H5HF_ID_VERS_CURR | H5HF_ID_TYPE_HUGE;
            H5F_addr_encode(hdr->f, &id, obj_addr);
            H5F_ENCODE_LENGTH(hdr->f, id, (hsize_t)write_size);
            UINT32ENCODE(id, filter_mask);
            H5F_ENCODE_LENGTH(hdr->f, id, (hsize_t)obj_size);
        } /* end if */
        else {
            H5HF_huge_bt2_dir_rec_t obj_rec;        /* Record for tracking object */

            /* Initialize record for tracking object in v2 B-tree */
            obj_rec.addr = obj_addr;
            obj_rec.len = write_size;
#ifdef QAK
HDfprintf(stderr, "%s: obj_rec = {%a, %Hu}\n", FUNC, obj_rec.addr, obj_rec.len);
#endif /* QAK */

            /* Insert record for object in v2 B-tree */
            if(H5B2_insert(hdr->f, dxpl_id, H5HF_BT2_DIR, hdr->huge_bt2_addr, &obj_rec) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTINSERT, FAIL, "couldn't insert object tracking record in v2 B-tree")

            /* Encode ID for user */
            *id++ = H5HF_ID_VERS_CURR | H5HF_ID_TYPE_HUGE;
            H5F_addr_encode(hdr->f, &id, obj_addr);
            H5F_ENCODE_LENGTH(hdr->f, id, (hsize_t)write_size);
        } /* end if */
    } /* end if */
    else {
        H5HF_huge_bt2_filt_indir_rec_t filt_indir_rec;      /* Record for tracking filtered object */
        H5HF_huge_bt2_indir_rec_t indir_rec;    /* Record for tracking non-filtered object */
        const H5B2_class_t *bt2_class;          /* v2 B-tree class to use */
        void *ins_rec;          /* Pointer to record to insert */
        hsize_t new_id;         /* New ID for object */

        /* Get new ID for object */
        if(0 == (new_id = H5HF_huge_new_id(hdr)))
            HGOTO_ERROR(H5E_HEAP, H5E_CANTINIT, FAIL, "can't generate new ID for object")

        if(hdr->filter_len > 0) {
            /* Initialize record for object in v2 B-tree */
            filt_indir_rec.addr = obj_addr;
            filt_indir_rec.len = write_size;
            filt_indir_rec.filter_mask = filter_mask;
            filt_indir_rec.obj_size = obj_size;
            filt_indir_rec.id = new_id;
#ifdef QAK
HDfprintf(stderr, "%s: filt_indir_rec = {%a, %Hu, %x, %Hu, %Hu}\n", FUNC, filt_indir_rec.addr, filt_indir_rec.len, filt_indir_rec.filter_mask, filt_indir_rec.obj_size, filt_indir_rec.id);
#endif /* QAK */

            /* Set pointer to record to insert */
            ins_rec = &filt_indir_rec;
            bt2_class = H5HF_BT2_FILT_INDIR;
        } /* end if */
        else {
            /* Initialize record for object in v2 B-tree */
            indir_rec.addr = obj_addr;
            indir_rec.len = write_size;
            indir_rec.id = new_id;
#ifdef QAK
HDfprintf(stderr, "%s: indir_rec = {%a, %Hu, %Hu}\n", FUNC, indir_rec.addr, indir_rec.len, indir_rec.id);
#endif /* QAK */

            /* Set pointer to record to insert */
            ins_rec = &indir_rec;
            bt2_class = H5HF_BT2_INDIR;
        } /* end else */

        /* Insert record for tracking object in v2 B-tree */
        if(H5B2_insert(hdr->f, dxpl_id, bt2_class, hdr->huge_bt2_addr, ins_rec) < 0)
            HGOTO_ERROR(H5E_HEAP, H5E_CANTINSERT, FAIL, "couldn't insert object tracking record in v2 B-tree")

        /* Encode ID for user */
        *id++ = H5HF_ID_VERS_CURR | H5HF_ID_TYPE_HUGE;
        UINT64ENCODE_VAR(id, new_id, hdr->huge_id_size)
    } /* end else */

    /* Update statistics about heap */
    hdr->huge_size += obj_size;
    hdr->huge_nobjs++;

    /* Mark heap header as modified */
    if(H5HF_hdr_dirty(hdr) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_CANTDIRTY, FAIL, "can't mark heap header as dirty")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_insert() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_get_obj_len
 *
 * Purpose:	Get the size of a 'huge' object in a fractal heap
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug  8 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_get_obj_len(H5HF_hdr_t *hdr, hid_t dxpl_id, const uint8_t *id,
    size_t *obj_len_p)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_get_obj_len)

    /*
     * Check arguments.
     */
    HDassert(hdr);
    HDassert(H5F_addr_defined(hdr->huge_bt2_addr));
    HDassert(id);
    HDassert(obj_len_p);

    /* Skip over the flag byte */
    id++;

    /* Check if 'huge' object ID encodes address & length directly */
    if(hdr->huge_ids_direct) {
        if(hdr->filter_len > 0) {
            /* Skip over filtered object info */
            id += hdr->sizeof_addr + hdr->sizeof_size + 4;

            /* Retrieve the object's length */
            H5F_DECODE_LENGTH(hdr->f, id, *obj_len_p);
        } /* end if */
        else {
            /* Skip over object offset in file */
            id += hdr->sizeof_addr;

            /* Retrieve the object's length */
            H5F_DECODE_LENGTH(hdr->f, id, *obj_len_p);
        } /* end else */
    } /* end if */
    else {
        if(hdr->filter_len > 0) {
            H5HF_huge_bt2_filt_indir_rec_t found_rec;  /* Record found from tracking object */
            H5HF_huge_bt2_filt_indir_rec_t search_rec; /* Record for searching for object */

            /* Get ID for looking up 'huge' object in v2 B-tree */
            UINT64DECODE_VAR(id, search_rec.id, hdr->huge_id_size)

            /* Look up object in v2 B-tree */
            if(H5B2_find(hdr->f, dxpl_id, H5HF_BT2_FILT_INDIR, hdr->huge_bt2_addr,
                        &search_rec, H5HF_huge_bt2_filt_indir_found, &found_rec) != TRUE)
                HGOTO_ERROR(H5E_HEAP, H5E_NOTFOUND, FAIL, "can't find object in B-tree")

            /* Retrieve the object's length */
            *obj_len_p = (size_t)found_rec.obj_size;
        } /* end if */
        else {
            H5HF_huge_bt2_indir_rec_t found_rec;  /* Record found from tracking object */
            H5HF_huge_bt2_indir_rec_t search_rec; /* Record for searching for object */

            /* Get ID for looking up 'huge' object in v2 B-tree */
            UINT64DECODE_VAR(id, search_rec.id, hdr->huge_id_size)

            /* Look up object in v2 B-tree */
            if(H5B2_find(hdr->f, dxpl_id, H5HF_BT2_INDIR, hdr->huge_bt2_addr,
                        &search_rec, H5HF_huge_bt2_indir_found, &found_rec) != TRUE)
                HGOTO_ERROR(H5E_HEAP, H5E_NOTFOUND, FAIL, "can't find object in B-tree")

            /* Retrieve the object's length */
            *obj_len_p = (size_t)found_rec.len;
        } /* end else */
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_get_obj_len() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_op_real
 *
 * Purpose:	Internal routine to perform an operation on a 'huge' object
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug  8 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5HF_huge_op_real(H5HF_hdr_t *hdr, hid_t dxpl_id, const uint8_t *id,
    hbool_t is_read, H5HF_operator_t op, void *op_data)
{
    void *read_buf = NULL;              /* Pointer to buffer for reading */
    haddr_t obj_addr;                   /* Object's address in the file */
    size_t obj_size = 0;                /* Object's size in the file */
    unsigned filter_mask = 0;           /* Filter mask for object (only used for filtered objects) */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_op_real)

    /*
     * Check arguments.
     */
    HDassert(hdr);
    HDassert(id);
    HDassert(is_read || op);

    /* Skip over the flag byte */
    id++;

    /* Check for 'huge' object ID that encodes address & length directly */
    if(hdr->huge_ids_direct) {
        /* Retrieve the object's address and length (common) */
        H5F_addr_decode(hdr->f, &id, &obj_addr);
        H5F_DECODE_LENGTH(hdr->f, id, obj_size);

        /* Retrieve extra information needed for filtered objects */
        if(hdr->filter_len > 0)
            UINT32DECODE(id, filter_mask);
    } /* end if */
    else {
        if(hdr->filter_len > 0) {
            H5HF_huge_bt2_filt_indir_rec_t found_rec;  /* Record found from tracking object */
            H5HF_huge_bt2_filt_indir_rec_t search_rec; /* Record for searching for object */

            /* Get ID for looking up 'huge' object in v2 B-tree */
            UINT64DECODE_VAR(id, search_rec.id, hdr->huge_id_size)

            /* Look up object in v2 B-tree */
            if(H5B2_find(hdr->f, dxpl_id, H5HF_BT2_FILT_INDIR, hdr->huge_bt2_addr,
                        &search_rec, H5HF_huge_bt2_filt_indir_found, &found_rec) != TRUE)
                HGOTO_ERROR(H5E_HEAP, H5E_NOTFOUND, FAIL, "can't find object in B-tree")

            /* Retrieve the object's address & length */
            obj_addr = found_rec.addr;
            H5_ASSIGN_OVERFLOW(/* To: */ obj_size, /* From: */ found_rec.len, /* From: */ hsize_t, /* To: */ size_t);
            filter_mask = found_rec.filter_mask;
        } /* end if */
        else {
            H5HF_huge_bt2_indir_rec_t found_rec;  /* Record found from tracking object */
            H5HF_huge_bt2_indir_rec_t search_rec; /* Record for searching for object */

            /* Get ID for looking up 'huge' object in v2 B-tree */
            UINT64DECODE_VAR(id, search_rec.id, hdr->huge_id_size)

            /* Look up object in v2 B-tree */
            if(H5B2_find(hdr->f, dxpl_id, H5HF_BT2_INDIR, hdr->huge_bt2_addr,
                        &search_rec, H5HF_huge_bt2_indir_found, &found_rec) != TRUE)
                HGOTO_ERROR(H5E_HEAP, H5E_NOTFOUND, FAIL, "can't find object in B-tree")

            /* Retrieve the object's address & length */
            obj_addr = found_rec.addr;
            H5_ASSIGN_OVERFLOW(/* To: */ obj_size, /* From: */ found_rec.len, /* From: */ hsize_t, /* To: */ size_t);
        } /* end else */
    } /* end else */

    /* Set up buffer for reading */
    if(hdr->filter_len > 0 || !is_read) {
        if(NULL == (read_buf = H5MM_malloc((size_t)obj_size)))
            HGOTO_ERROR(H5E_HEAP, H5E_NOSPACE, FAIL, "memory allocation failed for pipeline buffer")
    } /* end if */
    else
        read_buf = op_data;

    /* Read the object's (possibly filtered) data from the file */
    /* (reads directly into application's buffer if no filters are present) */
    if(H5F_block_read(hdr->f, H5FD_MEM_FHEAP_HUGE_OBJ, obj_addr, (size_t)obj_size, dxpl_id, read_buf) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_READERROR, FAIL, "can't read 'huge' object's data from the file")

    /* Check for I/O pipeline filter on heap */
    if(hdr->filter_len > 0) {
        H5Z_cb_t filter_cb = {NULL, NULL};  /* Filter callback structure */
        size_t read_size;                   /* Object's size in the file */
        size_t nbytes;                      /* Number of bytes used */

        /* De-filter the object */
        read_size = nbytes = obj_size;
        if(H5Z_pipeline(&(hdr->pline), H5Z_FLAG_REVERSE, &filter_mask, H5Z_NO_EDC, filter_cb, &nbytes, &read_size, &read_buf) < 0)
            HGOTO_ERROR(H5E_HEAP, H5E_CANTFILTER, FAIL, "input filter failed")
        obj_size = nbytes;
    } /* end if */

    /* Perform correct operation on buffer read in */
    if(is_read) {
        /* Copy object to user's buffer if there's filters on heap data */
        /* (if there's no filters, the object was read directly into the user's buffer) */
        if(hdr->filter_len > 0)
            HDmemcpy(op_data, read_buf, (size_t)obj_size);
    } /* end if */
    else {
        /* Call the user's 'op' callback */
        if(op(read_buf, (size_t)obj_size, op_data) < 0) {
            /* Release buffer */
            H5MM_xfree(read_buf);

            /* Indicate error */
            HGOTO_ERROR(H5E_HEAP, H5E_CANTOPERATE, FAIL, "application's callback failed")
        } /* end if */
    } /* end if */

done:
    /* Release the buffer for reading */
    if(read_buf && read_buf != op_data)
        H5MM_xfree(read_buf);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_op_real() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_write
 *
 * Purpose:	Write a 'huge' object to the heap
 *
 * Note:	This implementation somewhat limited: it doesn't handle
 *		heaps with filters, which would require re-compressing the
 *		huge object and probably changing the address of the object
 *		on disk (and possibly the heap ID for "direct" huge IDs).
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Feb 21 2007
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_write(H5HF_hdr_t *hdr, hid_t dxpl_id, const uint8_t *id,
    const void *obj)
{
    haddr_t obj_addr;                   /* Object's address in the file */
    size_t obj_size;                    /* Object's size in the file */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_write)

    /*
     * Check arguments.
     */
    HDassert(hdr);
    HDassert(id);
    HDassert(obj);

    /* Check for filters on the heap */
    if(hdr->filter_len > 0)
        HGOTO_ERROR(H5E_HEAP, H5E_UNSUPPORTED, FAIL, "modifying 'huge' object with filters not supported yet")

    /* Skip over the flag byte */
    id++;

    /* Check for 'huge' object ID that encodes address & length directly */
    if(hdr->huge_ids_direct) {
        /* Retrieve the object's address and length (common) */
        H5F_addr_decode(hdr->f, &id, &obj_addr);
        H5F_DECODE_LENGTH(hdr->f, id, obj_size);
    } /* end if */
    else {
        H5HF_huge_bt2_indir_rec_t found_rec;  /* Record found from tracking object */
        H5HF_huge_bt2_indir_rec_t search_rec; /* Record for searching for object */

        /* Get ID for looking up 'huge' object in v2 B-tree */
        UINT64DECODE_VAR(id, search_rec.id, hdr->huge_id_size)

        /* Look up object in v2 B-tree */
        if(H5B2_find(hdr->f, dxpl_id, H5HF_BT2_INDIR, hdr->huge_bt2_addr,
                    &search_rec, H5HF_huge_bt2_indir_found, &found_rec) != TRUE)
            HGOTO_ERROR(H5E_HEAP, H5E_NOTFOUND, FAIL, "can't find object in B-tree")

        /* Retrieve the object's address & length */
        obj_addr = found_rec.addr;
        H5_ASSIGN_OVERFLOW(/* To: */ obj_size, /* From: */ found_rec.len, /* From: */ hsize_t, /* To: */ size_t);
    } /* end else */

    /* Write the object's data to the file */
    /* (writes directly from application's buffer) */
    if(H5F_block_write(hdr->f, H5FD_MEM_FHEAP_HUGE_OBJ, obj_addr, obj_size, dxpl_id, obj) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_WRITEERROR, FAIL, "writing 'huge' object to file failed")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_write() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_read
 *
 * Purpose:	Read a 'huge' object from the heap
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Sept 11 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_read(H5HF_hdr_t *hdr, hid_t dxpl_id, const uint8_t *id, void *obj)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_read)

    /*
     * Check arguments.
     */
    HDassert(hdr);
    HDassert(id);
    HDassert(obj);

    /* Call the internal 'op' routine */
    if(H5HF_huge_op_real(hdr, dxpl_id, id, TRUE, NULL, obj) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_CANTOPERATE, FAIL, "unable to operate on heap object")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_read() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_op
 *
 * Purpose:	Operate directly on a 'huge' object
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Sept 11 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_op(H5HF_hdr_t *hdr, hid_t dxpl_id, const uint8_t *id,
    H5HF_operator_t op, void *op_data)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_op)

    /*
     * Check arguments.
     */
    HDassert(hdr);
    HDassert(id);
    HDassert(op);

    /* Call the internal 'op' routine routine */
    if(H5HF_huge_op_real(hdr, dxpl_id, id, FALSE, op, op_data) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_CANTOPERATE, FAIL, "unable to operate on heap object")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_op() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_remove
 *
 * Purpose:	Remove a 'huge' object from the file and the v2 B-tree tracker
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug  8 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_remove(H5HF_hdr_t *hdr, hid_t dxpl_id, const uint8_t *id)
{
    H5HF_huge_remove_ud1_t udata;       /* User callback data for v2 B-tree remove call */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_remove)

    /*
     * Check arguments.
     */
    HDassert(hdr);
    HDassert(id);

    /* Skip over the flag byte */
    id++;

    /* Set up the common callback info */
    udata.hdr = hdr;
    udata.dxpl_id = dxpl_id;

    /* Check for 'huge' object ID that encodes address & length directly */
    if(hdr->huge_ids_direct) {
        if(hdr->filter_len > 0) {
            H5HF_huge_bt2_filt_dir_rec_t search_rec;     /* Record for searching for object */

            /* Retrieve the object's address and length */
            /* (used as key in v2 B-tree record) */
            H5F_addr_decode(hdr->f, &id, &search_rec.addr);
            H5F_DECODE_LENGTH(hdr->f, id, search_rec.len);

            /* Remove the record for tracking the 'huge' object from the v2 B-tree */
            /* (space in the file for the object is freed in the 'remove' callback) */
            if(H5B2_remove(hdr->f, dxpl_id, H5HF_BT2_FILT_DIR, hdr->huge_bt2_addr,
                        &search_rec, H5HF_huge_bt2_filt_dir_remove, &udata) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTREMOVE, FAIL, "can't remove object from B-tree")
        } /* end if */
        else {
            H5HF_huge_bt2_dir_rec_t search_rec;     /* Record for searching for object */

            /* Retrieve the object's address and length */
            /* (used as key in v2 B-tree record) */
            H5F_addr_decode(hdr->f, &id, &search_rec.addr);
            H5F_DECODE_LENGTH(hdr->f, id, search_rec.len);

            /* Remove the record for tracking the 'huge' object from the v2 B-tree */
            /* (space in the file for the object is freed in the 'remove' callback) */
            if(H5B2_remove(hdr->f, dxpl_id, H5HF_BT2_DIR, hdr->huge_bt2_addr,
                        &search_rec, H5HF_huge_bt2_dir_remove, &udata) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTREMOVE, FAIL, "can't remove object from B-tree")
        } /* end else */
    } /* end if */
    else {
        if(hdr->filter_len > 0) {
            H5HF_huge_bt2_filt_indir_rec_t search_rec;     /* Record for searching for object */

            /* Get ID for looking up 'huge' object in v2 B-tree */
            UINT64DECODE_VAR(id, search_rec.id, hdr->huge_id_size)

            /* Remove the record for tracking the 'huge' object from the v2 B-tree */
            /* (space in the file for the object is freed in the 'remove' callback) */
            if(H5B2_remove(hdr->f, dxpl_id, H5HF_BT2_FILT_INDIR, hdr->huge_bt2_addr,
                        &search_rec, H5HF_huge_bt2_filt_indir_remove, &udata) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTREMOVE, FAIL, "can't remove object from B-tree")
        } /* end if */
        else {
            H5HF_huge_bt2_indir_rec_t search_rec;     /* Record for searching for object */

            /* Get ID for looking up 'huge' object in v2 B-tree */
            UINT64DECODE_VAR(id, search_rec.id, hdr->huge_id_size)

            /* Remove the record for tracking the 'huge' object from the v2 B-tree */
            /* (space in the file for the object is freed in the 'remove' callback) */
            if(H5B2_remove(hdr->f, dxpl_id, H5HF_BT2_INDIR, hdr->huge_bt2_addr,
                        &search_rec, H5HF_huge_bt2_indir_remove, &udata) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTREMOVE, FAIL, "can't remove object from B-tree")
        } /* end else */
    } /* end else */

    /* Update statistics about heap */
    hdr->huge_size -= udata.obj_len;
    hdr->huge_nobjs--;

    /* Mark heap header as modified */
    if(H5HF_hdr_dirty(hdr) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_CANTDIRTY, FAIL, "can't mark heap header as dirty")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_remove() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_term
 *
 * Purpose:	Shut down the information for tracking 'huge' objects
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug  8 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_term(H5HF_hdr_t *hdr, hid_t dxpl_id)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_term)

    /*
     * Check arguments.
     */
    HDassert(hdr);

    /* Check if there are no more 'huge' objects in the heap and delete the
     *  v2 B-tree that tracks them, if so
     */
    if(H5F_addr_defined(hdr->huge_bt2_addr) && hdr->huge_nobjs == 0) {
        /* Sanity check */
        HDassert(hdr->huge_size == 0);

        /* Delete the v2 B-tree */
        /* (any v2 B-tree class will work here) */
        if(H5B2_delete(hdr->f, dxpl_id, H5HF_BT2_INDIR, hdr->huge_bt2_addr, NULL, NULL) < 0)
            HGOTO_ERROR(H5E_HEAP, H5E_CANTDELETE, FAIL, "can't delete v2 B-tree")

        /* Reset the information about 'huge' objects in the file */
        hdr->huge_bt2_addr = HADDR_UNDEF;
        hdr->huge_next_id = 0;
        hdr->huge_ids_wrapped = FALSE;

        /* Mark heap header as modified */
        if(H5HF_hdr_dirty(hdr) < 0)
            HGOTO_ERROR(H5E_HEAP, H5E_CANTDIRTY, FAIL, "can't mark heap header as dirty")
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_term() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_huge_delete
 *
 * Purpose:	Delete all the 'huge' objects in the heap, and the v2 B-tree
 *              tracker for them
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Aug  8 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_huge_delete(H5HF_hdr_t *hdr, hid_t dxpl_id)
{
    H5HF_huge_remove_ud1_t udata;       /* User callback data for v2 B-tree remove call */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_huge_delete)

    /*
     * Check arguments.
     */
    HDassert(hdr);
    HDassert(H5F_addr_defined(hdr->huge_bt2_addr));
    HDassert(hdr->huge_nobjs);
    HDassert(hdr->huge_size);

    /* Set up the callback info */
    udata.hdr = hdr;
    udata.dxpl_id = dxpl_id;

    /* Delete the v2 B-tree */
    if(hdr->huge_ids_direct) {
        if(hdr->filter_len > 0) {
            if(H5B2_delete(hdr->f, dxpl_id, H5HF_BT2_FILT_DIR, hdr->huge_bt2_addr, H5HF_huge_bt2_filt_dir_remove, &udata) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTDELETE, FAIL, "can't delete v2 B-tree")
        } /* end if */
        else {
            if(H5B2_delete(hdr->f, dxpl_id, H5HF_BT2_DIR, hdr->huge_bt2_addr, H5HF_huge_bt2_dir_remove, &udata) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTDELETE, FAIL, "can't delete v2 B-tree")
        } /* end else */
    } /* end if */
    else {
        if(hdr->filter_len > 0) {
            if(H5B2_delete(hdr->f, dxpl_id, H5HF_BT2_FILT_INDIR, hdr->huge_bt2_addr, H5HF_huge_bt2_filt_indir_remove, &udata) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTDELETE, FAIL, "can't delete v2 B-tree")
        } /* end if */
        else {
            if(H5B2_delete(hdr->f, dxpl_id, H5HF_BT2_INDIR, hdr->huge_bt2_addr, H5HF_huge_bt2_indir_remove, &udata) < 0)
                HGOTO_ERROR(H5E_HEAP, H5E_CANTDELETE, FAIL, "can't delete v2 B-tree")
        } /* end else */
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_huge_delete() */

