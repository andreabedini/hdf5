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
 * Created:		H5Adense.c
 *			Dec  4 2006
 *			Quincey Koziol <koziol@hdfgroup.org>
 *
 * Purpose:		Routines for operating on "dense" attribute storage
 *                      for an object.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#define H5A_PACKAGE		/*suppress error about including H5Apkg  */
#define H5O_PACKAGE		/*suppress error about including H5Opkg  */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Apkg.h"		/* Attributes	  			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5Opkg.h"		/* Object headers			*/
#include "H5SMprivate.h"	/* Shared object header messages        */


/****************/
/* Local Macros */
/****************/

/* v2 B-tree creation macros for 'name' field index */
#define H5A_NAME_BT2_NODE_SIZE          512
#define H5A_NAME_BT2_MERGE_PERC         40
#define H5A_NAME_BT2_SPLIT_PERC         100

/* v2 B-tree creation macros for 'corder' field index */
#define H5A_CORDER_BT2_NODE_SIZE        512
#define H5A_CORDER_BT2_MERGE_PERC       40
#define H5A_CORDER_BT2_SPLIT_PERC       100

/* Size of stack buffer for serialized attributes */
#define H5A_ATTR_BUF_SIZE               128


/******************/
/* Local Typedefs */
/******************/

/*
 * Data exchange structure for dense attribute storage.  This structure is
 * passed through the v2 B-tree layer when modifying the attribute data value.
 */
typedef struct H5A_bt2_od_wrt_t {
    /* downward */
    H5F_t  *f;                  /* Pointer to file that fractal heap is in */
    hid_t dxpl_id;              /* DXPL for operation */
    H5HF_t *fheap;              /* Fractal heap handle to operate on */
    H5HF_t *shared_fheap;       /* Fractal heap handle for shared messages */
    H5A_t  *attr;               /* Attribute to write */
} H5A_bt2_od_wrt_t;

/*
 * Data exchange structure to pass through the v2 B-tree layer for the
 * H5B2_iterate function when iterating over densely stored attributes.
 */
typedef struct {
    /* downward (internal) */
    H5F_t       *f;                     /* Pointer to file that fractal heap is in */
    hid_t       dxpl_id;                /* DXPL for operation                */
    H5HF_t      *fheap;                 /* Fractal heap handle */
    H5HF_t      *shared_fheap;          /* Fractal heap handle for shared messages */

    /* downward (from application) */
    hid_t       loc_id;                 /* Object ID for application callback */
    unsigned    skip;                   /* Number of attributes to skip      */
    unsigned    count;                  /* The # of attributes visited       */
    const H5A_attr_iter_op_t *attr_op;  /* Callback for each attribute       */
    void        *op_data;               /* Callback data for each attribute  */

    /* upward */
    int         op_ret;                 /* Return value from callback        */
} H5A_bt2_ud_it_t;

/*
 * Data exchange structure to pass through the fractal heap layer for the
 * H5HF_op function when copying an attribute stored in densely stored attributes.
 * (or the shared message heap)
 */
typedef struct {
    /* downward (internal) */
    H5F_t       *f;                     /* Pointer to file that fractal heap is in */
    hid_t       dxpl_id;                /* DXPL for operation                */
    const H5A_dense_bt2_name_rec_t *record;     /* v2 B-tree record for attribute */

    /* upward */
    H5A_t  *attr;                       /* Copy of attribute                 */
} H5A_fh_ud_cp_t;


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/

/* Declare a free list to manage the serialized attribute information */
H5FL_BLK_DEFINE(ser_attr);



/*-------------------------------------------------------------------------
 * Function:	H5A_dense_create
 *
 * Purpose:	Creates dense attribute storage structures for an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  4 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5A_dense_create(H5F_t *f, hid_t dxpl_id, H5O_t *oh)
{
    H5HF_create_t fheap_cparam;         /* Fractal heap creation parameters */
    H5HF_t *fheap;                      /* Fractal heap handle */
    size_t bt2_rrec_size;               /* v2 B-tree raw record size */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_create, FAIL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(oh);

    /* Set fractal heap creation parameters */
/* XXX: Give some control of these to applications? */
    HDmemset(&fheap_cparam, 0, sizeof(fheap_cparam));
    fheap_cparam.managed.width = H5O_FHEAP_MAN_WIDTH;
    fheap_cparam.managed.start_block_size = H5O_FHEAP_MAN_START_BLOCK_SIZE;
    fheap_cparam.managed.max_direct_size = H5O_FHEAP_MAN_MAX_DIRECT_SIZE;
    fheap_cparam.managed.max_index = H5O_FHEAP_MAN_MAX_INDEX;
    fheap_cparam.managed.start_root_rows = H5O_FHEAP_MAN_START_ROOT_ROWS;
    fheap_cparam.checksum_dblocks = H5O_FHEAP_CHECKSUM_DBLOCKS;
    fheap_cparam.max_man_size = H5O_FHEAP_MAX_MAN_SIZE;

    /* Create fractal heap for storing attributes */
    if(NULL == (fheap = H5HF_create(f, dxpl_id, &fheap_cparam)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, FAIL, "unable to create fractal heap")

    /* Retrieve the heap's address in the file */
    if(H5HF_get_heap_addr(fheap, &(oh->attr_fheap_addr)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGETSIZE, FAIL, "can't get fractal heap address")
#ifdef QAK
HDfprintf(stderr, "%s: oh->attr_fheap_addr = %a\n", FUNC, oh->attr_fheap_addr);
#endif /* QAK */

#ifndef NDEBUG
{
    size_t fheap_id_len;                /* Fractal heap ID length */

    /* Retrieve the heap's ID length in the file */
    if(H5HF_get_id_len(fheap, &fheap_id_len) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGETSIZE, FAIL, "can't get fractal heap ID length")
    HDassert(fheap_id_len == H5O_FHEAP_ID_LEN);
#ifdef QAK
HDfprintf(stderr, "%s: fheap_id_len = %Zu\n", FUNC, fheap_id_len);
#endif /* QAK */
}
#endif /* NDEBUG */

    /* Close the fractal heap */
    if(H5HF_close(fheap, dxpl_id) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")

    /* Create the name index v2 B-tree */
    bt2_rrec_size = 4 +                 /* Name's hash value */
            4 +                         /* Creation order index */
            1 +                         /* Message flags */
            H5O_FHEAP_ID_LEN;           /* Fractal heap ID */
    if(H5B2_create(f, dxpl_id, H5A_BT2_NAME,
            (size_t)H5A_NAME_BT2_NODE_SIZE, bt2_rrec_size,
            H5A_NAME_BT2_SPLIT_PERC, H5A_NAME_BT2_MERGE_PERC,
            &(oh->name_bt2_addr)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, FAIL, "unable to create v2 B-tree for name index")
#ifdef QAK
HDfprintf(stderr, "%s: oh->name_bt2_addr = %a\n", FUNC, oh->name_bt2_addr);
#endif /* QAK */

    /* Check if we should create a creation order index v2 B-tree */
    if(oh->flags & H5P_CRT_ORDER_INDEXED) {
        /* Create the creation order index v2 B-tree */
        bt2_rrec_size = 4 +             /* Creation order index */
                1 +                     /* Message flags */
                H5O_FHEAP_ID_LEN;       /* Fractal heap ID */
        if(H5B2_create(f, dxpl_id, H5A_BT2_CORDER,
                (size_t)H5A_CORDER_BT2_NODE_SIZE, bt2_rrec_size,
                H5A_CORDER_BT2_SPLIT_PERC, H5A_CORDER_BT2_MERGE_PERC,
                &(oh->corder_bt2_addr)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, FAIL, "unable to create v2 B-tree for name index")
#ifdef QAK
HDfprintf(stderr, "%s: oh->corder_bt2_addr = %a\n", FUNC, oh->corder_bt2_addr);
#endif /* QAK */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_create() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_fnd_cb
 *
 * Purpose:	Callback when an attribute is located in an index
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec 11 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5A_dense_fnd_cb(const void *_attr, void *_user_attr)
{
    const H5A_t *attr = (const H5A_t *)_attr; /* Record from B-tree */
    H5A_t **user_attr = (H5A_t **)_user_attr; /* User data from v2 B-tree attribute lookup */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5A_dense_fnd_cb)

    /*
     * Check arguments.
     */
    HDassert(attr);
    HDassert(user_attr);

    /* Copy attribute information */
    if(NULL == (*user_attr = H5A_copy(NULL, attr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTCOPY, H5_ITER_ERROR, "can't copy attribute")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_fnd_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_open
 *
 * Purpose:	Open an attribute in dense storage structures for an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec 11 2006
 *
 *-------------------------------------------------------------------------
 */
H5A_t *
H5A_dense_open(H5F_t *f, hid_t dxpl_id, const H5O_t *oh, const char *name)
{
    H5A_bt2_ud_common_t udata;          /* User data for v2 B-tree modify */
    H5HF_t *fheap = NULL;               /* Fractal heap handle */
    H5HF_t *shared_fheap = NULL;        /* Fractal heap handle for shared header messages */
    htri_t attr_sharable;               /* Flag indicating attributes are sharable */
    H5A_t *ret_value = NULL;            /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_open, NULL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(oh);
    HDassert(name);

    /* Open the fractal heap */
    if(NULL == (fheap = H5HF_open(f, dxpl_id, oh->attr_fheap_addr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, NULL, "unable to open fractal heap")

    /* Check if attributes are shared in this file */
    if((attr_sharable = H5SM_type_shared(f, H5O_ATTR_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, NULL, "can't determine if attributes are shared")

    /* Get handle for shared message heap, if attributes are sharable */
    if(attr_sharable) {
        haddr_t shared_fheap_addr;      /* Address of fractal heap to use */

        /* Retrieve the address of the shared message's fractal heap */
        if(H5SM_get_fheap_addr(f, dxpl_id, H5O_ATTR_ID, &shared_fheap_addr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, NULL, "can't get shared message heap address")

        /* Check if there are any shared messages currently */
        if(H5F_addr_defined(shared_fheap_addr)) {
            /* Open the fractal heap for shared header messages */
            if(NULL == (shared_fheap = H5HF_open(f, dxpl_id, shared_fheap_addr)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, NULL, "unable to open fractal heap")
        } /* end if */
    } /* end if */

    /* Create the "udata" information for v2 B-tree record modify */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.fheap = fheap;
    udata.shared_fheap = shared_fheap;
    udata.name = name;
    udata.name_hash = H5_checksum_lookup3(name, HDstrlen(name), 0);
    udata.flags = 0;
    udata.corder = 0;
    udata.found_op = H5A_dense_fnd_cb;       /* v2 B-tree comparison callback */
    udata.found_op_data = &ret_value;

    /* Find & copy the attribute in the 'name' index */
    if(H5B2_find(f, dxpl_id, H5A_BT2_NAME, oh->name_bt2_addr, &udata, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_NOTFOUND, NULL, "can't locate attribute in name index")

done:
    /* Release resources */
    if(shared_fheap && H5HF_close(shared_fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, NULL, "can't close fractal heap")
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, NULL, "can't close fractal heap")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_open() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_insert
 *
 * Purpose:	Insert an attribute into dense storage structures for an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  4 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5A_dense_insert(H5F_t *f, hid_t dxpl_id, const H5O_t *oh, H5A_t *attr)
{
    H5A_bt2_ud_ins_t udata;             /* User data for v2 B-tree insertion */
    H5HF_t *fheap = NULL;               /* Fractal heap handle for attributes */
    H5HF_t *shared_fheap = NULL;        /* Fractal heap handle for shared header messages */
    uint8_t attr_buf[H5A_ATTR_BUF_SIZE]; /* Buffer for serializing message */
    void *attr_ptr = NULL;              /* Pointer to serialized message */
    unsigned mesg_flags = 0;            /* Flags for storing message */
    htri_t attr_sharable;               /* Flag indicating attributes are sharable */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_insert, FAIL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(oh);
    HDassert(attr);

    /* Check if attributes are shared in this file */
    if((attr_sharable = H5SM_type_shared(f, H5O_ATTR_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't determine if attributes are shared")

    /* Get handle for shared message heap, if attributes are sharable */
    if(attr_sharable) {
        haddr_t shared_fheap_addr;      /* Address of fractal heap to use */
        htri_t shared_mesg;             /* Should this message be stored in the Shared Message table? */

        /* Check if message is already shared */
        if((shared_mesg = H5O_msg_is_shared(H5O_ATTR_ID, attr)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "error determining if message is shared")
        else if(shared_mesg > 0)
            /* Mark the message as shared */
            mesg_flags |= H5O_MSG_FLAG_SHARED;
        else {
            /* Should this attribute be written as a SOHM? */
            if((shared_mesg = H5SM_try_share(f, dxpl_id, H5O_ATTR_ID, attr)) > 0)
                /* Mark the message as shared */
                mesg_flags |= H5O_MSG_FLAG_SHARED;
            else if(shared_mesg < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_WRITEERROR, FAIL, "error determining if message should be shared")
        } /* end else */

        /* Retrieve the address of the shared message's fractal heap */
        if(H5SM_get_fheap_addr(f, dxpl_id, H5O_ATTR_ID, &shared_fheap_addr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get shared message heap address")

        /* Check if there are any shared messages currently */
        if(H5F_addr_defined(shared_fheap_addr)) {
            /* Open the fractal heap for shared header messages */
            if(NULL == (shared_fheap = H5HF_open(f, dxpl_id, shared_fheap_addr)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")
        } /* end if */
    } /* end if */

    /* Open the fractal heap */
    if(NULL == (fheap = H5HF_open(f, dxpl_id, oh->attr_fheap_addr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")

    /* Check for inserting shared attribute */
    if(mesg_flags & H5O_MSG_FLAG_SHARED) {
        /* Sanity check */
        HDassert(attr_sharable);

        /* Use heap ID for shared message heap */
        udata.id = attr->sh_loc.u.heap_id;
    } /* end if */
    else {
        size_t attr_size;                   /* Size of serialized attribute in the heap */

        /* Find out the size of buffer needed for serialized message */
        if((attr_size = H5O_msg_raw_size(f, H5O_ATTR_ID, FALSE, attr)) == 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGETSIZE, FAIL, "can't get message size")

        /* Allocate space for serialized message, if necessary */
        if(attr_size > sizeof(attr_buf)) {
            if(NULL == (attr_ptr = H5FL_BLK_MALLOC(ser_attr, attr_size)))
                HGOTO_ERROR(H5E_ATTR, H5E_NOSPACE, FAIL, "memory allocation failed")
        } /* end if */
        else
            attr_ptr = attr_buf;

        /* Create serialized form of attribute or shared message */
        if(H5O_msg_encode(f, H5O_ATTR_ID, FALSE, (unsigned char *)attr_ptr, attr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTENCODE, FAIL, "can't encode attribute")

        /* Insert the serialized attribute into the fractal heap */
        /* (sets the heap ID in the user data) */
        if(H5HF_insert(fheap, dxpl_id, attr_size, attr_ptr, &udata.id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, FAIL, "unable to insert attribute into fractal heap")
    } /* end else */

    /* Create the callback information for v2 B-tree record insertion */
    udata.common.f = f;
    udata.common.dxpl_id = dxpl_id;
    udata.common.fheap = fheap;
    udata.common.shared_fheap = shared_fheap;
    udata.common.name = attr->name;
    udata.common.name_hash = H5_checksum_lookup3(attr->name, HDstrlen(attr->name), 0);
    udata.common.flags = mesg_flags;
    udata.common.corder = attr->crt_idx;
    udata.common.found_op = NULL;
    udata.common.found_op_data = NULL;
    /* udata.id already set */

    /* Insert attribute into 'name' tracking v2 B-tree */
    if(H5B2_insert(f, dxpl_id, H5A_BT2_NAME, oh->name_bt2_addr, &udata) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, FAIL, "unable to insert record into v2 B-tree")

    /* Check if we should create a creation order index v2 B-tree record */
    if(oh->flags & H5P_CRT_ORDER_INDEXED) {
        /* Insert the record into the creation order index v2 B-tree */
        HDassert(H5F_addr_defined(oh->corder_bt2_addr));
        if(H5B2_insert(f, dxpl_id, H5A_BT2_CORDER, oh->corder_bt2_addr, &udata) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTINSERT, FAIL, "unable to insert record into v2 B-tree")
    } /* end if */

done:
    /* Release resources */
    if(shared_fheap && H5HF_close(shared_fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(attr_ptr && attr_ptr != attr_buf)
        (void)H5FL_BLK_FREE(ser_attr, attr_ptr);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_insert() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_write_bt2_cb
 *
 * Purpose:	v2 B-tree 'modify' callback to update the data for an attribute
 *
 * Return:	Success:	0
 *		Failure:	1
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, December  5, 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5A_dense_write_bt2_cb(void *_record, void *_op_data, hbool_t *changed)
{
    H5A_dense_bt2_name_rec_t *record = (H5A_dense_bt2_name_rec_t *)_record; /* Record from B-tree */
    H5A_bt2_od_wrt_t *op_data = (H5A_bt2_od_wrt_t *)_op_data;       /* "op data" from v2 B-tree modify */
    uint8_t attr_buf[H5A_ATTR_BUF_SIZE]; /* Buffer for serializing attribute */
    void *attr_ptr = NULL;              /* Pointer to serialized attribute */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5A_dense_write_bt2_cb)

    /*
     * Check arguments.
     */
    HDassert(record);
    HDassert(op_data);

    /* Check for modifying shared attribute */
    if(record->flags & H5O_MSG_FLAG_SHARED) {
        /* Update the shared attribute in the SOHM info */
        if(H5O_attr_update_shared(op_data->f, op_data->dxpl_id, op_data->attr, NULL) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update attribute in shared storage")

        /* Update record's heap ID */
        record->id = op_data->attr->sh_loc.u.heap_id;

        /* Note that the record changed */
        *changed = TRUE;
    } /* end if */
    else {
        size_t attr_size;               /* Size of serialized attribute in the heap */

        /* Find out the size of buffer needed for serialized attribute */
        if((attr_size = H5O_msg_raw_size(op_data->f, H5O_ATTR_ID, FALSE, op_data->attr)) == 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGETSIZE, FAIL, "can't get attribute size")

        /* Allocate space for serialized attribute, if necessary */
        if(attr_size > sizeof(attr_buf)) {
            if(NULL == (attr_ptr = H5FL_BLK_MALLOC(ser_attr, attr_size)))
                HGOTO_ERROR(H5E_ATTR, H5E_NOSPACE, FAIL, "memory allocation failed")
        } /* end if */
        else
            attr_ptr = attr_buf;

        /* Create serialized form of attribute */
        if(H5O_msg_encode(op_data->f, H5O_ATTR_ID, FALSE, (unsigned char *)attr_ptr, op_data->attr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTENCODE, FAIL, "can't encode attribute")

/* Sanity check */
#ifndef NDEBUG
{
    size_t obj_len;             /* Length of existing encoded attribute */

    if(H5HF_get_obj_len(op_data->fheap, op_data->dxpl_id, &record->id, &obj_len) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGETSIZE, FAIL, "can't get object size")
    HDassert(obj_len == attr_size);
}
#endif /* NDEBUG */
        /* Update existing attribute in heap */
        /* (might be more efficient as fractal heap 'op' callback, but leave that for later -QAK) */
        if(H5HF_write(op_data->fheap, op_data->dxpl_id, &record->id, changed, attr_ptr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update attribute in heap")
    } /* end else */

done:
    /* Release resources */
    if(attr_ptr && attr_ptr != attr_buf)
        (void)H5FL_BLK_FREE(ser_attr, attr_ptr);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_write_bt2_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_write
 *
 * Purpose:	Modify an attribute in dense storage structures for an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  4 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5A_dense_write(H5F_t *f, hid_t dxpl_id, const H5O_t *oh, H5A_t *attr)
{
    H5A_bt2_ud_common_t udata;          /* User data for v2 B-tree modify */
    H5A_bt2_od_wrt_t op_data;           /* "Op data" for v2 B-tree modify */
    H5HF_t *fheap = NULL;               /* Fractal heap handle */
    H5HF_t *shared_fheap = NULL;        /* Fractal heap handle for shared header messages */
    htri_t attr_sharable;               /* Flag indicating attributes are sharable */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_write, FAIL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(oh);
    HDassert(attr);

    /* Check if attributes are shared in this file */
    if((attr_sharable = H5SM_type_shared(f, H5O_ATTR_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't determine if attributes are shared")

    /* Get handle for shared message heap, if attributes are sharable */
    if(attr_sharable) {
        haddr_t shared_fheap_addr;      /* Address of fractal heap to use */

        /* Retrieve the address of the shared message's fractal heap */
        if(H5SM_get_fheap_addr(f, dxpl_id, H5O_ATTR_ID, &shared_fheap_addr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get shared message heap address")

        /* Check if there are any shared messages currently */
        if(H5F_addr_defined(shared_fheap_addr)) {
            /* Open the fractal heap for shared header messages */
            if(NULL == (shared_fheap = H5HF_open(f, dxpl_id, shared_fheap_addr)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")
        } /* end if */
    } /* end if */

    /* Open the fractal heap */
    if(NULL == (fheap = H5HF_open(f, dxpl_id, oh->attr_fheap_addr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")

    /* Create the "udata" information for v2 B-tree record modify */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.fheap = fheap;
    udata.shared_fheap = shared_fheap;
    udata.name = attr->name;
    udata.name_hash = H5_checksum_lookup3(attr->name, HDstrlen(attr->name), 0);
    udata.flags = 0;
    udata.corder = 0;
    udata.found_op = NULL;
    udata.found_op_data = NULL;

    /* Create the "op_data" for the v2 B-tree record 'modify' callback */
    op_data.f = f;
    op_data.dxpl_id = dxpl_id;
    op_data.fheap = fheap;
    op_data.shared_fheap = shared_fheap;
    op_data.attr = attr;

    /* Modify attribute through 'name' tracking v2 B-tree */
    if(H5B2_modify(f, dxpl_id, H5A_BT2_NAME, oh->name_bt2_addr, &udata, H5A_dense_write_bt2_cb, &op_data) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, FAIL, "unable to modify record in v2 B-tree")

done:
    /* Release resources */
    if(shared_fheap && H5HF_close(shared_fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_write() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_copy_fh_cb
 *
 * Purpose:	Callback for fractal heap operator, to make copy of attribute
 *              for calling routine
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  5 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5A_dense_copy_fh_cb(const void *obj, size_t UNUSED obj_len, void *_udata)
{
    H5A_fh_ud_cp_t *udata = (H5A_fh_ud_cp_t *)_udata;       /* User data for fractal heap 'op' callback */
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5A_dense_copy_fh_cb)

    /* Decode attribute information & keep a copy */
    /* (we make a copy instead of calling the user/library callback directly in
     *  this routine because this fractal heap 'op' callback routine is called
     *  with the direct block protected and if the callback routine invokes an
     *  HDF5 routine, it could attempt to re-protect that direct block for the
     *  heap, causing the HDF5 routine called to fail)
     */
    if(NULL == (udata->attr = (H5A_t *)H5O_msg_decode(udata->f, udata->dxpl_id, H5O_ATTR_ID, (const unsigned char *)obj)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTDECODE, FAIL, "can't decode attribute")

    /* Check whether we should "reconstitute" the shared message info */
    if(udata->record->flags & H5O_MSG_FLAG_SHARED)
        H5SM_reconstitute(&(udata->attr->sh_loc), udata->record->id);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_copy_fh_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_rename
 *
 * Purpose:	Rename an attribute in dense storage structures for an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Jan  3 2007
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5A_dense_rename(H5F_t *f, hid_t dxpl_id, const H5O_t *oh, const char *old_name,
    const char *new_name)
{
    H5A_bt2_ud_common_t udata;          /* User data for v2 B-tree modify */
    H5HF_t *fheap = NULL;               /* Fractal heap handle */
    H5HF_t *shared_fheap = NULL;        /* Fractal heap handle for shared header messages */
    H5A_t *attr_copy = NULL;            /* Copy of attribute to rename */
    htri_t attr_sharable;               /* Flag indicating attributes are sharable */
    htri_t shared_mesg;                 /* Should this message be stored in the Shared Message table? */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_rename, FAIL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(oh);
    HDassert(old_name);
    HDassert(new_name);

    /* Check if attributes are shared in this file */
    if((attr_sharable = H5SM_type_shared(f, H5O_ATTR_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't determine if attributes are shared")

    /* Get handle for shared message heap, if attributes are sharable */
    if(attr_sharable) {
        haddr_t shared_fheap_addr;      /* Address of fractal heap to use */

        /* Retrieve the address of the shared message's fractal heap */
        if(H5SM_get_fheap_addr(f, dxpl_id, H5O_ATTR_ID, &shared_fheap_addr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get shared message heap address")

        /* Check if there are any shared messages currently */
        if(H5F_addr_defined(shared_fheap_addr)) {
            /* Open the fractal heap for shared header messages */
            if(NULL == (shared_fheap = H5HF_open(f, dxpl_id, shared_fheap_addr)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")
        } /* end if */
    } /* end if */

    /* Open the fractal heap */
    if(NULL == (fheap = H5HF_open(f, dxpl_id, oh->attr_fheap_addr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")

    /* Create the "udata" information for v2 B-tree record modify */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.fheap = fheap;
    udata.shared_fheap = shared_fheap;
    udata.name = old_name;
    udata.name_hash = H5_checksum_lookup3(old_name, HDstrlen(old_name), 0);
    udata.flags = 0;
    udata.corder = 0;
    udata.found_op = H5A_dense_fnd_cb;       /* v2 B-tree comparison callback */
    udata.found_op_data = &attr_copy;

    /* Get copy of attribute through 'name' tracking v2 B-tree */
    if(H5B2_find(f, dxpl_id, H5A_BT2_NAME, oh->name_bt2_addr, &udata, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, FAIL, "unable to find record in v2 B-tree")
    HDassert(attr_copy);

    /* Check if message is already shared */
    if((shared_mesg = H5O_msg_is_shared(H5O_ATTR_ID, attr_copy)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "error determining if message is shared")
    else if(shared_mesg > 0) {
        /* Reset shared status of copy */
        /* (so it will get shared again if necessary) */
        attr_copy->sh_loc.flags = 0;
    } /* end if */

    /* Change name of attribute */
    H5MM_xfree(attr_copy->name);
    attr_copy->name = H5MM_xstrdup(new_name);

    /* Insert renamed attribute back into dense storage */
    /* (Possibly making it shared) */
    if(H5A_dense_insert(f, dxpl_id, oh, attr_copy) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, FAIL, "unable to add to dense storage")

    /* Was this attribute shared? */
    if((shared_mesg = H5O_msg_is_shared(H5O_ATTR_ID, attr_copy)) > 0) {
        hsize_t attr_rc;                /* Attribute's ref count in shared message storage */

        /* Retrieve ref count for shared attribute */
        if(H5SM_get_refcount(f, dxpl_id, H5O_ATTR_ID, &attr_copy->sh_loc, &attr_rc) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't retrieve shared message ref count")

        /* If the newly shared attribute needs to share "ownership" of the shared
         *      components (ie. its reference count is 1), increment the reference
         *      count on any shared components of the attribute, so that they won't
         *      be removed from the file.  (Essentially a "copy on write" operation).
         *
         *      *ick* -QAK, 2007/01/08
         */
        if(attr_rc == 1) {
            /* Increment reference count on attribute components */
            if(H5O_attr_link(f, dxpl_id, attr_copy) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_LINKCOUNT, FAIL, "unable to adjust attribute link count")
        } /* end if */
    } /* end if */
    else if(shared_mesg == 0) {
        /* Increment reference count on attribute components */
        /* (so that they aren't deleted when the attribute is removed shortly) */
        if(H5O_attr_link(f, dxpl_id, attr_copy) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_LINKCOUNT, FAIL, "unable to adjust attribute link count")
    } /* end if */
    else if(shared_mesg < 0)
	HGOTO_ERROR(H5E_ATTR, H5E_WRITEERROR, FAIL, "error determining if message should be shared")

    /* Delete old attribute from dense storage */
    if(H5A_dense_remove(f, dxpl_id, oh, old_name) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete attribute in dense storage")

done:
    /* Release resources */
    if(shared_fheap && H5HF_close(shared_fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(attr_copy)
        H5O_msg_free(H5O_ATTR_ID, attr_copy);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_rename() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_iterate_bt2_cb
 *
 * Purpose:	v2 B-tree callback for dense attribute storage iterator
 *
 * Return:	H5_ITER_ERROR/H5_ITER_CONT/H5_ITER_STOP
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  5 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5A_dense_iterate_bt2_cb(const void *_record, void *_bt2_udata)
{
    const H5A_dense_bt2_name_rec_t *record = (const H5A_dense_bt2_name_rec_t *)_record; /* Record from B-tree */
    H5A_bt2_ud_it_t *bt2_udata = (H5A_bt2_ud_it_t *)_bt2_udata;         /* User data for callback */
    herr_t ret_value = H5_ITER_CONT;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5A_dense_iterate_bt2_cb)

    /* Check for skipping attributes */
    if(bt2_udata->skip > 0)
        --bt2_udata->skip;
    else {
        H5A_fh_ud_cp_t fh_udata;       /* User data for fractal heap 'op' callback */
        H5HF_t *fheap;                 /* Fractal heap handle for attribute storage */

        /* Check for iterating over shared attribute */
        if(record->flags & H5O_MSG_FLAG_SHARED)
            fheap = bt2_udata->shared_fheap;
        else
            fheap = bt2_udata->fheap;

        /* Prepare user data for callback */
        /* down */
        fh_udata.f = bt2_udata->f;
        fh_udata.dxpl_id = bt2_udata->dxpl_id;
        fh_udata.record = record;
        fh_udata.attr = NULL;

        /* Call fractal heap 'op' routine, to copy the attribute information */
        if(H5HF_op(fheap, bt2_udata->dxpl_id, &record->id, H5A_dense_copy_fh_cb, &fh_udata) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTOPERATE, H5_ITER_ERROR, "heap op callback failed")

        /* Check which type of callback to make */
        switch(bt2_udata->attr_op->op_type) {
            case H5A_ATTR_OP_APP:
                /* Make the application callback */
                ret_value = (bt2_udata->attr_op->u.app_op)(bt2_udata->loc_id, fh_udata.attr->name, bt2_udata->op_data);
                break;

            case H5A_ATTR_OP_LIB:
                /* Call the library's callback */
                ret_value = (bt2_udata->attr_op->u.lib_op)(fh_udata.attr, bt2_udata->op_data);
        } /* end switch */

        /* Release the space allocated for the attribute */
        H5O_msg_free(H5O_ATTR_ID, fh_udata.attr);
    } /* end else */

    /* Increment the number of attributes passed through */
    /* (whether we skipped them or not) */
    bt2_udata->count++;

    /* Check for callback failure and pass along return value */
    if(ret_value < 0)
        HERROR(H5E_ATTR, H5E_CANTNEXT, "iteration operator failed");

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_iterate_bt2_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_iterate
 *
 * Purpose:	Iterate over attributes in dense storage structures for an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  5 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5A_dense_iterate(H5F_t *f, hid_t dxpl_id, hid_t loc_id, haddr_t attr_fheap_addr,
    haddr_t name_bt2_addr, H5_iter_order_t order, unsigned skip,
    unsigned *last_attr, const H5A_attr_iter_op_t *attr_op, void *op_data)
{
    H5A_bt2_ud_it_t udata;              /* User data for iterator callback */
    H5HF_t *fheap = NULL;               /* Fractal heap handle */
    H5HF_t *shared_fheap = NULL;        /* Fractal heap handle for shared header messages */
    H5A_attr_table_t atable = {0, NULL};        /* Table of attributes */
    hsize_t nrec;                       /* # of records in v2 B-tree */
    herr_t ret_value;                   /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_iterate, FAIL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(H5F_addr_defined(attr_fheap_addr));
    HDassert(H5F_addr_defined(name_bt2_addr));
    HDassert(attr_op);

    /* Retrieve # of records in name index */
    /* (# of records in all indices the same) */
    if(H5B2_get_nrec(f, dxpl_id, H5A_BT2_NAME, name_bt2_addr, &nrec) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't retrieve # of records in index")

    /* Check for skipping too many attributes */
    if(skip > 0) {
        /* Check for bad starting index */
        if((hsize_t)skip >= nrec)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid index specified")
    } /* end if */

    /* Check on iteration order */
    /* ("native" iteration order is unordered for this attribute storage mechanism) */
    if(order == H5_ITER_NATIVE) {
        htri_t attr_sharable;               /* Flag indicating attributes are sharable */

        /* Open the fractal heap */
        if(NULL == (fheap = H5HF_open(f, dxpl_id, attr_fheap_addr)))
            HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")

        /* Check if attributes are shared in this file */
        if((attr_sharable = H5SM_type_shared(f, H5O_ATTR_ID, dxpl_id)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't determine if attributes are shared")

        /* Get handle for shared message heap, if attributes are sharable */
        if(attr_sharable) {
            haddr_t shared_fheap_addr;      /* Address of fractal heap to use */

            /* Retrieve the address of the shared message's fractal heap */
            if(H5SM_get_fheap_addr(f, dxpl_id, H5O_ATTR_ID, &shared_fheap_addr) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get shared message heap address")

            /* Check if there are any shared messages currently */
            if(H5F_addr_defined(shared_fheap_addr)) {
                /* Open the fractal heap for shared header messages */
                if(NULL == (shared_fheap = H5HF_open(f, dxpl_id, shared_fheap_addr)))
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")
            } /* end if */
        } /* end if */

        /* Construct the user data for v2 B-tree iterator callback */
        udata.f = f;
        udata.dxpl_id = dxpl_id;
        udata.fheap = fheap;
        udata.shared_fheap = shared_fheap;
        udata.loc_id = loc_id;
        udata.skip = skip;
        udata.count = 0;
        udata.attr_op = attr_op;
        udata.op_data = op_data;

        /* Iterate over the records in the v2 B-tree's "native" order */
        /* (by hash of name) */
        if((ret_value = H5B2_iterate(f, dxpl_id, H5A_BT2_NAME, name_bt2_addr,
                H5A_dense_iterate_bt2_cb, &udata)) < 0)
            HERROR(H5E_ATTR, H5E_BADITER, "attribute iteration failed");

        /* Update last attribute looked at */
        if(last_attr)
            *last_attr = udata.count;
    } /* end if */
    else {
        /* Build the table of attributes for this object */
        if(H5A_dense_build_table(f, dxpl_id, nrec, attr_fheap_addr, name_bt2_addr,
                H5_INDEX_NAME, order, &atable) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "error building table of attributes")

        /* Iterate over attributes in table */
        if((ret_value = H5A_attr_iterate_table(&atable, skip, last_attr, loc_id, attr_op, op_data)) < 0)
            HERROR(H5E_ATTR, H5E_CANTNEXT, "iteration operator failed");
    } /* end else */

done:
    /* Release resources */
    if(shared_fheap && H5HF_close(shared_fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(atable.attrs && H5A_attr_release_table(&atable) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTFREE, FAIL, "unable to release attribute table")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_iterate() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_remove_bt2_cb
 *
 * Purpose:	v2 B-tree callback for dense attribute storage record removal
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec 11 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5A_dense_remove_bt2_cb(const void *_record, void *_bt2_udata)
{
    const H5A_dense_bt2_name_rec_t *record = (const H5A_dense_bt2_name_rec_t *)_record;
    H5A_bt2_ud_common_t *bt2_udata = (H5A_bt2_ud_common_t *)_bt2_udata;         /* User data for callback */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5A_dense_remove_bt2_cb)

    /* Check for inserting shared attribute */
    if(record->flags & H5O_MSG_FLAG_SHARED) {
        /* Decrement the reference count on the shared attribute message */
        if(H5SM_try_delete(bt2_udata->f, bt2_udata->dxpl_id, H5O_ATTR_ID, &((*(H5A_t **)bt2_udata->found_op_data)->sh_loc)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTFREE, FAIL, "unable to delete shared attribute")
    } /* end if */
    else {
        /* Perform the deletion action on the attribute */
        /* (takes care of shared & committed datatype/dataspace components) */
        if(H5O_attr_delete(bt2_udata->f, bt2_udata->dxpl_id, *(H5A_t **)bt2_udata->found_op_data) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete attribute")

        /* Remove record from fractal heap */
        if(H5HF_remove(bt2_udata->fheap, bt2_udata->dxpl_id, &record->id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTREMOVE, FAIL, "unable to remove attribute from fractal heap")
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_remove_bt2_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_remove
 *
 * Purpose:	Remove an attribute from the dense storage of an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec 11 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5A_dense_remove(H5F_t *f, hid_t dxpl_id, const H5O_t *oh, const char *name)
{
    H5A_bt2_ud_common_t udata;          /* User data for v2 B-tree record removal */
    H5HF_t *fheap = NULL;               /* Fractal heap handle */
    H5HF_t *shared_fheap = NULL;        /* Fractal heap handle for shared header messages */
    H5A_t *attr_copy = NULL;            /* Copy of attribute to remove */
    htri_t attr_sharable;               /* Flag indicating attributes are sharable */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_remove, FAIL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(oh);
    HDassert(name && *name);

    /* Open the fractal heap */
    if(NULL == (fheap = H5HF_open(f, dxpl_id, oh->attr_fheap_addr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")

    /* Check if attributes are shared in this file */
    if((attr_sharable = H5SM_type_shared(f, H5O_ATTR_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't determine if attributes are shared")

    /* Get handle for shared message heap, if attributes are sharable */
    if(attr_sharable) {
        haddr_t shared_fheap_addr;      /* Address of fractal heap to use */

        /* Retrieve the address of the shared message's fractal heap */
        if(H5SM_get_fheap_addr(f, dxpl_id, H5O_ATTR_ID, &shared_fheap_addr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get shared message heap address")

        /* Check if there are any shared messages currently */
        if(H5F_addr_defined(shared_fheap_addr)) {
            /* Open the fractal heap for shared header messages */
            if(NULL == (shared_fheap = H5HF_open(f, dxpl_id, shared_fheap_addr)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")
        } /* end if */
    } /* end if */

    /* Set up the user data for the v2 B-tree 'record remove' callback */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.fheap = fheap;
    udata.shared_fheap = shared_fheap;
    udata.name = name;
    udata.name_hash = H5_checksum_lookup3(name, HDstrlen(name), 0);
    udata.flags = 0;
    udata.corder = 0;
    udata.found_op = H5A_dense_fnd_cb;       /* v2 B-tree comparison callback */
    udata.found_op_data = &attr_copy;

    /* Remove the record from the name index v2 B-tree */
    if(H5B2_remove(f, dxpl_id, H5A_BT2_NAME, oh->name_bt2_addr, &udata, H5A_dense_remove_bt2_cb, &udata) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTREMOVE, FAIL, "unable to remove attribute from name index v2 B-tree")

done:
    /* Release resources */
    if(shared_fheap && H5HF_close(shared_fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(attr_copy)
        H5O_msg_free_real(H5O_MSG_ATTR, attr_copy);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_remove() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_exists
 *
 * Purpose:	Check if an attribute exists in dense storage structures for
 *              an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec 11 2006
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5A_dense_exists(H5F_t *f, hid_t dxpl_id, const H5O_t *oh, const char *name)
{
    H5A_bt2_ud_common_t udata;          /* User data for v2 B-tree modify */
    H5HF_t *fheap = NULL;               /* Fractal heap handle */
    H5HF_t *shared_fheap = NULL;        /* Fractal heap handle for shared header messages */
    htri_t attr_sharable;               /* Flag indicating attributes are sharable */
    htri_t ret_value = TRUE;            /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_exists, NULL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(oh);
    HDassert(name);

    /* Open the fractal heap */
    if(NULL == (fheap = H5HF_open(f, dxpl_id, oh->attr_fheap_addr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")

    /* Check if attributes are shared in this file */
    if((attr_sharable = H5SM_type_shared(f, H5O_ATTR_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't determine if attributes are shared")

    /* Get handle for shared message heap, if attributes are sharable */
    if(attr_sharable) {
        haddr_t shared_fheap_addr;      /* Address of fractal heap to use */

        /* Retrieve the address of the shared message's fractal heap */
        if(H5SM_get_fheap_addr(f, dxpl_id, H5O_ATTR_ID, &shared_fheap_addr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get shared message heap address")

        /* Check if there are any shared messages currently */
        if(H5F_addr_defined(shared_fheap_addr)) {
            /* Open the fractal heap for shared header messages */
            if(NULL == (shared_fheap = H5HF_open(f, dxpl_id, shared_fheap_addr)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")
        } /* end if */
    } /* end if */

    /* Create the "udata" information for v2 B-tree record 'find' */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.fheap = fheap;
    udata.shared_fheap = shared_fheap;
    udata.name = name;
    udata.name_hash = H5_checksum_lookup3(name, HDstrlen(name), 0);
    udata.flags = 0;
    udata.corder = 0;
    udata.found_op = NULL;       /* v2 B-tree comparison callback */
    udata.found_op_data = NULL;

    /* Find the attribute in the 'name' index */
    if(H5B2_find(f, dxpl_id, H5A_BT2_NAME, oh->name_bt2_addr, &udata, NULL, NULL) < 0) {
        /* Assume that the failure was just not finding the attribute & clear stack */
        H5E_clear_stack(NULL);

        ret_value = FALSE;
    } /* end if */

done:
    /* Release resources */
    if(shared_fheap && H5HF_close(shared_fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_exists() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_delete_bt2_cb
 *
 * Purpose:	v2 B-tree callback for dense attribute storage deletion
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Jan  3 2007
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5A_dense_delete_bt2_cb(const void *_record, void *_bt2_udata)
{
    const H5A_dense_bt2_name_rec_t *record = (const H5A_dense_bt2_name_rec_t *)_record; /* Record from B-tree */
    H5A_bt2_ud_common_t *bt2_udata = (H5A_bt2_ud_common_t *)_bt2_udata;         /* User data for callback */
    H5HF_t *fheap;                      /* Fractal heap handle for attribute storage */
    H5A_fh_ud_cp_t fh_udata;            /* User data for fractal heap 'op' callback */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5A_dense_delete_bt2_cb)

    /* Check for shared attribute */
    if(record->flags & H5O_MSG_FLAG_SHARED)
        fheap = bt2_udata->shared_fheap;
    else
        fheap = bt2_udata->fheap;

    /* Prepare user data for callback */
    /* down */
    fh_udata.f = bt2_udata->f;
    fh_udata.dxpl_id = bt2_udata->dxpl_id;
    fh_udata.record = record;
    /* up */
    fh_udata.attr = NULL;

    /* Call fractal heap 'op' routine, to copy the attribute information */
    if(H5HF_op(fheap, bt2_udata->dxpl_id, &record->id, H5A_dense_copy_fh_cb, &fh_udata) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPERATE, FAIL, "heap op callback failed")

    /* Check for shared attribute */
    if(record->flags & H5O_MSG_FLAG_SHARED) {
        /* Decrement the reference count on the shared attribute message */
        if(H5SM_try_delete(bt2_udata->f, bt2_udata->dxpl_id, H5O_ATTR_ID, &(fh_udata.attr->sh_loc)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTFREE, FAIL, "unable to delete shared attribute")
    } /* end if */
    else {
        /* Perform the deletion action on the attribute */
        /* (takes care of shared/committed datatype & dataspace components) */
        if(H5O_attr_delete(bt2_udata->f, bt2_udata->dxpl_id, fh_udata.attr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete attribute")
    } /* end else */

done:
    /* Release resources */
    if(fh_udata.attr)
        H5O_msg_free_real(H5O_MSG_ATTR, fh_udata.attr);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_delete_bt2_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5A_dense_delete
 *
 * Purpose:	Delete all dense storage structures for attributes on an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  6 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5A_dense_delete(H5F_t *f, hid_t dxpl_id, H5O_t *oh)
{
    H5A_bt2_ud_common_t udata;          /* v2 B-tree user data for deleting attributes */
    H5HF_t *fheap = NULL;               /* Fractal heap handle */
    H5HF_t *shared_fheap = NULL;        /* Fractal heap handle for shared header messages */
    htri_t attr_sharable;               /* Flag indicating attributes are sharable */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(H5A_dense_delete, FAIL)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(oh);

    /* Open the fractal heap */
    if(NULL == (fheap = H5HF_open(f, dxpl_id, oh->attr_fheap_addr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")

    /* Check if attributes are shared in this file */
    if((attr_sharable = H5SM_type_shared(f, H5O_ATTR_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't determine if attributes are shared")

    /* Get handle for shared message heap, if attributes are sharable */
    if(attr_sharable) {
        haddr_t shared_fheap_addr;      /* Address of fractal heap to use */

        /* Retrieve the address of the shared message's fractal heap */
        if(H5SM_get_fheap_addr(f, dxpl_id, H5O_ATTR_ID, &shared_fheap_addr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get shared message heap address")

        /* Check if there are any shared messages currently */
        if(H5F_addr_defined(shared_fheap_addr)) {
            /* Open the fractal heap for shared header messages */
            if(NULL == (shared_fheap = H5HF_open(f, dxpl_id, shared_fheap_addr)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")
        } /* end if */
    } /* end if */

    /* Create the "udata" information for v2 B-tree 'delete' */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.fheap = fheap;
    udata.shared_fheap = shared_fheap;
    udata.name = NULL;
    udata.name_hash = 0;
    udata.flags = 0;
    udata.corder = 0;
    udata.found_op = NULL;       /* v2 B-tree comparison callback */
    udata.found_op_data = NULL;

    /* Delete name index v2 B-tree */
    if(H5B2_delete(f, dxpl_id, H5A_BT2_NAME, oh->name_bt2_addr, H5A_dense_delete_bt2_cb, &udata) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete v2 B-tree for name index")
    oh->name_bt2_addr = HADDR_UNDEF;

    /* Release resources */
    if(shared_fheap) {
        if(H5HF_close(shared_fheap, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
        shared_fheap = NULL;
    } /* end if */
    if(H5HF_close(fheap, dxpl_id) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    fheap = NULL;

    /* Delete fractal heap */
    if(H5HF_delete(f, dxpl_id, oh->attr_fheap_addr) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete fractal heap")
    oh->attr_fheap_addr = HADDR_UNDEF;

done:
    /* Release resources */
    if(shared_fheap && H5HF_close(shared_fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, FAIL, "can't close fractal heap")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5A_dense_delete() */

