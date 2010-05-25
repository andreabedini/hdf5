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
 * Created:		H5Oattribute.c
 *			Dec 11 2006
 *			Quincey Koziol <koziol@hdfgroup.org>
 *
 * Purpose:		Object header attribute routines.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#define H5A_PACKAGE		/*suppress error about including H5Apkg	  */
#define H5O_PACKAGE		/*suppress error about including H5Opkg	  */

/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Apkg.h"		/* Attributes	  			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5Opkg.h"             /* Object headers			*/
#include "H5SMprivate.h"	/* Shared Object Header Messages	*/
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5Fprivate.h"		/* File 			        */


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/

/* User data for iteration when converting attributes to dense storage */
typedef struct {
    H5F_t      *f;              /* Pointer to file for insertion */
    hid_t dxpl_id;              /* DXPL during iteration */
    H5O_ainfo_t *ainfo;         /* Attribute info struct */
} H5O_iter_cvt_t;

/* User data for iteration when opening an attribute */
typedef struct {
    /* down */
    const char *name;           /* Name of attribute to open */

    /* up */
    H5A_t *attr;                /* Attribute data to update object header with */
} H5O_iter_opn_t;

/* User data for iteration when updating an attribute */
typedef struct {
    /* down */
    H5F_t *f;                   /* Pointer to file attribute is in */
    hid_t dxpl_id;              /* DXPL for operation */
    H5A_t *attr;                /* Attribute data to update object header with */

    /* up */
    hbool_t found;              /* Whether the attribute was found */
} H5O_iter_wrt_t;

/* User data for iteration when renaming an attribute */
typedef struct {
    /* down */
    H5F_t *f;                   /* Pointer to file attribute is in */
    hid_t dxpl_id;              /* DXPL for operation */
    const char *old_name;       /* Old name of attribute */
    const char *new_name;       /* New name of attribute */

    /* up */
    hbool_t found;              /* Whether the attribute was found */
} H5O_iter_ren_t;

/* User data for iteration when iterating over attributes */
typedef struct {
    /* down */
    H5F_t *f;                   /* Pointer to file attribute is in */
    hid_t dxpl_id;              /* DXPL for operation */
    hid_t loc_id;               /* ID of object being iterated over */
    unsigned skip;              /* # of attributes to skip over */
    H5A_operator_t op;          /* Callback routine for each attribute */
    void *op_data;              /* User data for callback */

    /* up */
    unsigned count;             /* Count of attributes examined */
} H5O_iter_itr_t;

/* User data for iteration when removing an attribute */
typedef struct {
    /* down */
    H5F_t *f;                   /* Pointer to file attribute is in */
    hid_t dxpl_id;              /* DXPL for operation */
    const char *name;           /* Name of attribute to open */

    /* up */
    hbool_t found;              /* Found attribute to delete */
} H5O_iter_rm_t;

/* User data for iteration when checking if an attribute exists */
typedef struct {
    /* down */
    const char *name;           /* Name of attribute to open */

    /* up */
    hbool_t found;              /* Found attribute */
} H5O_iter_xst_t;


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/
static herr_t H5O_attr_iterate_real(hid_t loc_id, const H5O_loc_t *loc,
    hid_t dxpl_id, H5_index_t idx_type, H5_iter_order_t order, hsize_t skip,
    hsize_t *last_attr, const H5A_attr_iter_op_t *attr_op, void *op_data);
static htri_t H5O_attr_find_opened_attr(const H5O_loc_t *loc, H5A_t **attr,
    const char* name_to_open);

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
 * Function:	H5O_attr_to_dense_cb
 *
 * Purpose:	Object header iterator callback routine to convert compact
 *              attributes to dense attributes
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  4 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_attr_to_dense_cb(H5O_t *oh, H5O_mesg_t *mesg/*in,out*/,
    unsigned UNUSED sequence, hbool_t *oh_modified, void *_udata/*in,out*/)
{
    H5O_iter_cvt_t *udata = (H5O_iter_cvt_t *)_udata;   /* Operator user data */
    H5A_t *attr = (H5A_t *)mesg->native;        /* Pointer to attribute to insert */
    herr_t ret_value = H5_ITER_CONT;            /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_to_dense_cb)

    /* check args */
    HDassert(oh);
    HDassert(mesg);
    HDassert(udata);
    HDassert(udata->f);
    HDassert(udata->ainfo);
    HDassert(attr);

    /* Insert attribute into dense storage */
    if(H5A_dense_insert(udata->f, udata->dxpl_id, udata->ainfo, attr) < 0)
        HGOTO_ERROR(H5E_OHDR, H5E_CANTINSERT, H5_ITER_ERROR, "unable to add to dense storage")

    /* Convert message into a null message in the header */
    /* (don't delete attribute's space in the file though) */
    if(H5O_release_mesg(udata->f, udata->dxpl_id, oh, mesg, FALSE) < 0)
        HGOTO_ERROR(H5E_OHDR, H5E_CANTDELETE, H5_ITER_ERROR, "unable to convert into null message")

    /* Indicate that the object header was modified */
    *oh_modified = TRUE;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_to_dense_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_create
 *
 * Purpose:	Create a new attribute in the object header.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Friday, December  8, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_create(const H5O_loc_t *loc, hid_t dxpl_id, H5A_t *attr)
{
    H5O_t *oh = NULL;                   /* Pointer to actual object header */
    H5O_ainfo_t ainfo;                  /* Attribute information for object */
    htri_t shared_mesg;                 /* Should this message be stored in the Shared Message table? */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_create)

    /* Check arguments */
    HDassert(loc);
    HDassert(attr);

    /* Pin the object header */
    if(NULL == (oh = H5O_pin(loc, dxpl_id)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPIN, FAIL, "unable to pin object header")

    /* Check if this object already has attribute information */
    if(oh->version > H5O_VERSION_1) {
        hbool_t new_ainfo = FALSE;      /* Flag to indicate that the attribute information is new */
        htri_t ainfo_exists;            /* Whether the attribute info was retrieved */

        /* Check for (& retrieve if available) attribute info */
        if((ainfo_exists = H5A_get_ainfo(loc->file, dxpl_id, oh, &ainfo)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
        if(!ainfo_exists) {
            /* Initialize attribute information */
            ainfo.track_corder = (hbool_t)((oh->flags & H5O_HDR_ATTR_CRT_ORDER_TRACKED) ? TRUE : FALSE);
            ainfo.index_corder = (hbool_t)((oh->flags & H5O_HDR_ATTR_CRT_ORDER_INDEXED) ? TRUE : FALSE);
            ainfo.max_crt_idx = 0;
            ainfo.corder_bt2_addr = HADDR_UNDEF;
            ainfo.nattrs = 0;
            ainfo.fheap_addr = HADDR_UNDEF;
            ainfo.name_bt2_addr = HADDR_UNDEF;

            /* Set flag to add attribute information to object header */
            new_ainfo = TRUE;
        } /* end if */
        else {
            /* Sanity check attribute info read in */
            HDassert(ainfo.nattrs > 0);
            HDassert(ainfo.track_corder == ((oh->flags & H5O_HDR_ATTR_CRT_ORDER_TRACKED) > 0));
            HDassert(ainfo.index_corder == ((oh->flags & H5O_HDR_ATTR_CRT_ORDER_INDEXED) > 0));
        } /* end else */

        /* Check if switching to "dense" attribute storage is possible */
        if(!H5F_addr_defined(ainfo.fheap_addr)) {
            htri_t sharable;        /* Whether the attribute will be shared */
            size_t raw_size = 0;    /* Raw size of message */

            /* Check for attribute being sharable */
            if((sharable = H5SM_can_share(loc->file, dxpl_id, NULL, NULL, H5O_ATTR_ID, attr)) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_BADMESG, FAIL, "can't determine attribute sharing status")
            else if(sharable == FALSE) {
                /* Compute the size needed to encode the attribute */
                raw_size = (H5O_MSG_ATTR->raw_size)(loc->file, FALSE, attr);
            } /* end if */

            /* Check for condititions for switching to "dense" attribute storage are met */
            if(ainfo.nattrs == oh->max_compact || (!sharable && raw_size >= H5O_MESG_MAX_SIZE)) {
                H5O_iter_cvt_t udata;           /* User data for callback */
                H5O_mesg_operator_t op;         /* Wrapper for operator */

                /* Create dense storage for attributes */
                if(H5A_dense_create(loc->file, dxpl_id, &ainfo) < 0)
                    HGOTO_ERROR(H5E_OHDR, H5E_CANTINIT, FAIL, "unable to create dense storage for attributes")

                /* Set up user data for callback */
                udata.f = loc->file;
                udata.dxpl_id = dxpl_id;
                udata.ainfo = &ainfo;

                /* Iterate over existing attributes, moving them to dense storage */
                op.op_type = H5O_MESG_OP_LIB;
                op.u.lib_op = H5O_attr_to_dense_cb;
                if(H5O_msg_iterate_real(loc->file, oh, H5O_MSG_ATTR, &op, &udata, dxpl_id) < 0)
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTCONVERT, FAIL, "error converting attributes to dense storage")
            } /* end if */
        } /* end if */

        /* Increment attribute count on object */
        ainfo.nattrs++;

        /* Check whether we're tracking the creation index on attributes */
        if(ainfo.track_corder) {
            /* Check for attribute creation order index on the object wrapping around */
            if(ainfo.max_crt_idx == H5O_MAX_CRT_ORDER_IDX)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTINC, FAIL, "attribute creation index can't be incremented")

            /* Set the creation order index on the attribute & incr. creation order index */
            attr->shared->crt_idx = ainfo.max_crt_idx++;
        } /* end if */
        else
            /* Set "bogus" creation index for attribute */
            attr->shared->crt_idx = H5O_MAX_CRT_ORDER_IDX;

        /* Add the attribute information message, if one is needed */
        if(new_ainfo) {
            if(H5O_msg_append_real(loc->file, dxpl_id, oh, H5O_MSG_AINFO, H5O_MSG_FLAG_DONTSHARE, 0, &ainfo) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, FAIL, "unable to create new attribute info message")
        } /* end if */
        /* Otherwise, update existing message */
        else {
            if(H5O_msg_write_real(loc->file, dxpl_id, oh, H5O_MSG_AINFO, H5O_MSG_FLAG_DONTSHARE, 0, &ainfo) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update attribute info message")
        } /* end else */
    } /* end if */
    else {
        /* Set "bogus" creation index for attribute */
        attr->shared->crt_idx = H5O_MAX_CRT_ORDER_IDX;

        /* Set attribute info value to get attribute into object header */
        ainfo.fheap_addr = HADDR_UNDEF;
    } /* end else */

    /* Check for storing attribute with dense storage */
    if(H5F_addr_defined(ainfo.fheap_addr)) {
        /* Insert attribute into dense storage */
        if(H5A_dense_insert(loc->file, dxpl_id, &ainfo, attr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, FAIL, "unable to add to dense storage")
    } /* end if */
    else {
        /* Append new message to object header */
        if(H5O_msg_append_real(loc->file, dxpl_id, oh, H5O_MSG_ATTR, 0, 0, attr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, FAIL, "unable to create new attribute in header")
    } /* end else */

    /* Increment reference count for shared attribute object for the
     * object handle created by the caller function H5A_create.  The count
     * for the cached object header has been incremented in the step above
     * (in H5O_msg_append_real).  The dense storage doesn't need a count. */
    attr->shared->nrefs += 1;

    /* Was new attribute shared? */
    if((shared_mesg = H5O_msg_is_shared(H5O_ATTR_ID, attr)) > 0) {
        hsize_t attr_rc;                /* Attribute's ref count in shared message storage */

        /* Retrieve ref count for shared attribute */
        if(H5SM_get_refcount(loc->file, dxpl_id, H5O_ATTR_ID, &attr->sh_loc, &attr_rc) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't retrieve shared message ref count")

        /* If this is not the first copy of the attribute in the shared message
         *      storage, decrement the reference count on any shared components
         *      of the attribute.  This is done because the shared message
         *      storage's "try delete" call doesn't call the message class's
         *      "delete" callback until the reference count drops to zero.
         *      However, attributes have already increased the reference
         *      count on shared components before passing the attribute
         *      to the shared message code to manage, causing an asymmetry
         *      in the reference counting for any shared components.
         *
         *      The alternate solution is to have the shared message's "try
         *      delete" code always call the message class's "delete" callback,
         *      even when the reference count is positive.  This can be done
         *      without an appreciable performance hit (by using H5HF_op() in
         *      the shared message comparison v2 B-tree callback), but it has
         *      the undesirable side-effect of leaving the reference count on
         *      the attribute's shared components artificially (and possibly
         *      misleadingly) high, because there's only one shared attribute
         *      referencing the shared components, not <refcount for the
         *      shared attribute> objects referencing the shared components.
         *
         *      *ick* -QAK, 2007/01/08
         */
        if(attr_rc > 1) {
            if(H5O_attr_delete(loc->file, dxpl_id, oh, attr) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete attribute")
        } /* end if */
    } /* end if */
    else if(shared_mesg < 0)
	HGOTO_ERROR(H5E_ATTR, H5E_WRITEERROR, FAIL, "error determining if message should be shared")

    /* Update the modification time, if any */
    if(H5O_touch_oh(loc->file, dxpl_id, oh, FALSE) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update time on object")

done:
    if(oh && H5O_unpin(oh) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPIN, FAIL, "unable to unpin object header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_create() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_open_cb
 *
 * Purpose:	Object header iterator callback routine to open an
 *              attribute stored compactly.
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
H5O_attr_open_cb(H5O_t *oh, H5O_mesg_t *mesg/*in,out*/, unsigned sequence,
    hbool_t UNUSED *oh_modified, void *_udata/*in,out*/)
{
    H5O_iter_opn_t *udata = (H5O_iter_opn_t *)_udata;   /* Operator user data */
    herr_t ret_value = H5_ITER_CONT;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_open_cb)

    /* check args */
    HDassert(oh);
    HDassert(mesg);
    HDassert(!udata->attr);

    /* Check for correct attribute message to modify */
    if(HDstrcmp(((H5A_t *)mesg->native)->shared->name, udata->name) == 0) {
        /* Make a copy of the attribute to return */
        if(NULL == (udata->attr = H5A_copy(NULL, (H5A_t *)mesg->native)))
            HGOTO_ERROR(H5E_ATTR, H5E_CANTCOPY, H5_ITER_ERROR, "unable to copy attribute")

        /* Assign [somewhat arbitrary] creation order value, for older versions of the format */
        if(oh->version == H5O_VERSION_1)
            udata->attr->shared->crt_idx = sequence;

        /* Stop iterating */
        ret_value = H5_ITER_STOP;
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_open_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_open_by_name
 *
 * Purpose:	Open an existing attribute in an object header.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Monday, December 11, 2006
 *
 * Modification:Raymond Lu
 *              23 June 2008
 *              If the attribute is in dense storage and has already been
 *              opened, make a copy of already opened object to share some
 *              object information.
 *-------------------------------------------------------------------------
 */
H5A_t *
H5O_attr_open_by_name(const H5O_loc_t *loc, const char *name, hid_t dxpl_id)
{
    H5O_t *oh = NULL;                   /* Pointer to actual object header */
    H5O_ainfo_t ainfo;                  /* Attribute information for object */
    H5A_t *ret_value;                   /* Return value */
    H5A_t *exist_attr = NULL;           /* Opened attribute object */
    htri_t found_open_attr = FALSE;     /* Whether opened object is found */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_open_by_name)

    /* Check arguments */
    HDassert(loc);
    HDassert(name);

    /* Protect the object header to iterate over */
    if(NULL == (oh = H5O_protect(loc, dxpl_id, H5AC_READ)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPROTECT, NULL, "unable to load object header")

    /* Check for attribute info stored */
    ainfo.fheap_addr = HADDR_UNDEF;
    if(oh->version > H5O_VERSION_1) {
        /* Check for (& retrieve if available) attribute info */
        if(H5A_get_ainfo(loc->file, dxpl_id, oh, &ainfo) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, NULL, "can't check for attribute info message")
    } /* end if */

    /* If found the attribute is already opened, make a copy of it to share the
     * object information.  If not, open attribute as a new object
     */
    if((found_open_attr = H5O_attr_find_opened_attr(loc, &exist_attr, name)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, NULL, "failed in finding opened attribute")
    else if(found_open_attr == TRUE) {
        if(NULL == (ret_value = H5A_copy(NULL, exist_attr)))
            HGOTO_ERROR(H5E_ATTR, H5E_CANTCOPY, NULL, "can't copy existing attribute")
    } /* end else if */
    else {
        /* Check for attributes in dense storage */
        if(H5F_addr_defined(ainfo.fheap_addr)) {
            /* Open attribute with dense storage */
            if(NULL == (ret_value = H5A_dense_open(loc->file, dxpl_id, &ainfo, name)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, NULL, "can't open attribute")
        } /* end if */
        else {
            H5O_iter_opn_t udata;           /* User data for callback */
            H5O_mesg_operator_t op;         /* Wrapper for operator */

            /* Set up user data for callback */
            udata.name = name;
            udata.attr = NULL;

            /* Iterate over attributes, to locate correct one to open */
            op.op_type = H5O_MESG_OP_LIB;
            op.u.lib_op = H5O_attr_open_cb;
            if(H5O_msg_iterate_real(loc->file, oh, H5O_MSG_ATTR, &op, &udata, dxpl_id) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, NULL, "error updating attribute")

            /* Check that we found the attribute */
            if(!udata.attr)
                HGOTO_ERROR(H5E_ATTR, H5E_NOTFOUND, NULL, "can't locate attribute")

            /* Get attribute opened from object header */
            HDassert(udata.attr);
            ret_value = udata.attr;
        } /* end else */

        /* Mark datatype as being on disk now */
        if(H5T_set_loc(ret_value->shared->dt, loc->file, H5T_LOC_DISK) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, NULL, "invalid datatype location")

    } /* end else */

done:
    if(oh && H5O_unprotect(loc, dxpl_id, oh, H5AC__NO_FLAGS_SET) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, NULL, "unable to release object header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_open_by_name() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_open_by_idx_cb
 *
 * Purpose:	Callback routine opening an attribute by index
 *
 * Return:	Success:        Non-negative
 *		Failure:	Negative
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec 18 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_attr_open_by_idx_cb(const H5A_t *attr, void *_ret_attr)
{
    H5A_t **ret_attr = (H5A_t **)_ret_attr;     /* 'User data' passed in */
    herr_t ret_value = H5_ITER_STOP;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_open_by_idx_cb)

    /* check arguments */
    HDassert(attr);
    HDassert(ret_attr);

    /* Copy attribute information.  Shared some attribute information. */
    if(NULL == (*ret_attr = H5A_copy(NULL, attr)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTCOPY, H5_ITER_ERROR, "can't copy attribute")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_open_by_idx_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_open_by_idx
 *
 * Purpose:	Open an existing attribute in an object header according to
 *              an index.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Monday, December 18, 2006
 *
 * Modification:Raymond Lu
 *              23 June 2008
 *              After opening the attribute, check whether it's in dense
 *              storage and has already been opened.  If it has, close the
 *              opened object and make a copy of already opened object.
 *-------------------------------------------------------------------------
 */
H5A_t *
H5O_attr_open_by_idx(const H5O_loc_t *loc, H5_index_t idx_type,
    H5_iter_order_t order, hsize_t n, hid_t dxpl_id)
{
    H5O_t *oh = NULL;                   /* Object header */
    H5A_attr_iter_op_t attr_op;         /* Attribute operator */
    H5A_t *exist_attr = NULL;           /* Opened attribute object */
    htri_t found_open_attr = FALSE;     /* Whether opened object is found */
    H5A_t *ret_value = NULL;            /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_open_by_idx)

    /* Check arguments */
    HDassert(loc);

    /* Build attribute operator info */
    attr_op.op_type = H5A_ATTR_OP_LIB;
    attr_op.u.lib_op = H5O_attr_open_by_idx_cb;

    /* Iterate over attributes to locate correct one */
    if(H5O_attr_iterate_real((hid_t)-1, loc, dxpl_id, idx_type, order, n, NULL, &attr_op, &ret_value) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_BADITER, NULL, "can't locate attribute")

    /* Protect the object header to iterate over */
    if(NULL == (oh = H5O_protect(loc, dxpl_id, H5AC_READ)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPROTECT, NULL, "unable to load object header")

    /* Find out whether it has already been opened.  If it has, close the object
     * and make a copy of the already opened object to share the object info.
     */
    if(ret_value) {
        if((found_open_attr = H5O_attr_find_opened_attr(loc, &exist_attr, ret_value->shared->name)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, NULL, "failed in finding opened attribute")

        /* If found that the attribute is already opened, make a copy of it
         * and close the object just opened.
         */
        if(found_open_attr && exist_attr) {
            if(H5A_close(ret_value) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTCLOSEOBJ, NULL, "can't close attribute")
            if(NULL == (ret_value = H5A_copy(NULL, exist_attr)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTCOPY, NULL, "can't copy existing attribute")
        } else {
            /* Mark datatype as being on disk now */
            if(H5T_set_loc(ret_value->shared->dt, loc->file, H5T_LOC_DISK) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, NULL, "invalid datatype location")
        } /* end if */
    } /* end if */

done:
    if(oh && H5O_unprotect(loc, dxpl_id, oh, H5AC__NO_FLAGS_SET) < 0)
	HDONE_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, NULL, "unable to release object header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_open_by_idx() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_find_opened_attr
 *
 * Purpose:	Find out whether an attribute has been opened by giving
 *              the name.  Return the pointer to the object if found.
 *
 * Return:	TRUE:	found the already opened object
 *              FALSE:  didn't find the opened object
 *              FAIL:	function failed.
 *
 * Programmer:	Raymond Lu
 *		23 June 2008
 *
 *-------------------------------------------------------------------------
 */
static htri_t
H5O_attr_find_opened_attr(const H5O_loc_t *loc, H5A_t **attr, const char* name_to_open)
{
    hid_t *attr_id_list = NULL;         /* List of IDs for opened attributes */
    unsigned long loc_fnum;             /* File serial # for object */
    size_t num_open_attr;               /* Number of opened attributes */
    htri_t ret_value = FALSE;           /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_find_opened_attr)

    /* Get file serial number for the location of attribute */
    if(H5F_get_fileno(loc->file, &loc_fnum) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_BADVALUE, FAIL, "can't get file serial number")

    /* Count all opened attributes */
    num_open_attr = H5F_get_obj_count(loc->file, H5F_OBJ_ATTR | H5F_OBJ_LOCAL, FALSE);

    /* Find out whether the attribute has been opened */
    if(num_open_attr) {
        size_t u;          /* Local index variable */

        /* Allocate space for the attribute ID list */
        if(NULL == (attr_id_list = (hid_t *)H5MM_malloc(num_open_attr * sizeof(hid_t))))
            HGOTO_ERROR(H5E_ATTR, H5E_NOSPACE, FAIL, "unable to allocate memory for attribute ID list")

        /* Retrieve the IDs of all opened attributes */
        H5F_get_obj_ids(loc->file, H5F_OBJ_ATTR | H5F_OBJ_LOCAL, num_open_attr, attr_id_list, FALSE);

        /* Iterate over the attributes */
        for(u = 0; u < num_open_attr; u++) {
            unsigned long attr_fnum;        /* Attributes file serial number */

            /* Get pointer to attribute */
            if(NULL == (*attr = (H5A_t *)H5I_object_verify(attr_id_list[u], H5I_ATTR)))
                HGOTO_ERROR(H5E_ATTR, H5E_BADTYPE, FAIL, "not an attribute")

            /* Get file serial number for attribute */
            if(H5F_get_fileno((*attr)->oloc.file, &attr_fnum) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_BADVALUE, FAIL, "can't get file serial number")

            /* Verify whether it's the right object.  The attribute name, object
             *  address to which the attribute is attached, and file serial
             *  number should all match.
             */
            if(!HDstrcmp(name_to_open, (*attr)->shared->name) &&
                    loc->addr == (*attr)->oloc.addr &&
                    loc_fnum == attr_fnum) {
                ret_value = TRUE;
                break;
            } /* end if */
        } /* end for */
    } /* end if */

done:
    if(attr_id_list)
        H5MM_free(attr_id_list);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_find_opened_attr */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_update_shared
 *
 * Purpose:	Update a shared attribute.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Jan  2 2007
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_update_shared(H5F_t *f, hid_t dxpl_id, H5O_t *oh, H5A_t *attr,
    H5O_shared_t *update_sh_mesg)
{
    H5O_shared_t sh_mesg;               /* Shared object header message */
    hsize_t attr_rc;                    /* Attribute's ref count in shared message storage */
    htri_t shared_mesg;                 /* Whether the message should be shared */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_update_shared)

    /* check args */
    HDassert(f);
    HDassert(attr);

    /* Extract shared message info from current attribute (for later use) */
    if(H5O_set_shared(&sh_mesg, &(attr->sh_loc)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTCOPY, FAIL, "can't get shared message")

    /* Reset existing sharing information */
    if(H5O_msg_reset_share(H5O_ATTR_ID, attr) < 0)
        HGOTO_ERROR(H5E_OHDR, H5E_CANTINIT, FAIL, "unable to reset attribute sharing")

    /* Store new version of message as a SOHM */
    /* (should always work, since we're not changing the size of the attribute) */
    if((shared_mesg = H5SM_try_share(f, dxpl_id, oh, H5O_ATTR_ID, attr, NULL)) == 0)
        HGOTO_ERROR(H5E_ATTR, H5E_BADMESG, FAIL, "attribute changed sharing status")
    else if(shared_mesg < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_BADMESG, FAIL, "can't share attribute")

    /* Retrieve shared message storage ref count for new shared attribute */
    if(H5SM_get_refcount(f, dxpl_id, H5O_ATTR_ID, &attr->sh_loc, &attr_rc) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't retrieve shared message ref count")

    /* If the newly shared attribute needs to share "ownership" of the shared
     *      components (ie. its reference count is 1), increment the reference
     *      count on any shared components of the attribute, so that they won't
     *      be removed from the file by the following "delete" operation on the
     *      original attribute shared message info.  (Essentially a "copy on
     *      write" operation).
     *
     *      *ick* -QAK, 2007/01/08
     */
    if(attr_rc == 1) {
        /* Increment reference count on attribute components */
        if(H5O_attr_link(f, dxpl_id, oh, attr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_LINKCOUNT, FAIL, "unable to adjust attribute link count")
    } /* end if */

    /* Remove the old attribute from the SOHM storage */
    if(H5SM_delete(f, dxpl_id, oh, &sh_mesg) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTFREE, FAIL, "unable to delete shared attribute in shared storage")

    /* Extract updated shared message info from modified attribute, if requested */
    if(update_sh_mesg)
        if(H5O_set_shared(update_sh_mesg, &(attr->sh_loc)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTCOPY, FAIL, "can't get shared message")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_update_shared() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_write_cb
 *
 * Purpose:	Object header iterator callback routine to update an
 *              attribute stored compactly.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  4 2006
 *
 * Modification:Raymond Lu
 *              4 June 2008
 *              Took out the data copying part because the attribute data
 *              is shared between attribute handle and object header.
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_attr_write_cb(H5O_t *oh, H5O_mesg_t *mesg/*in,out*/,
    unsigned UNUSED sequence, hbool_t *oh_modified, void *_udata/*in,out*/)
{
    H5O_iter_wrt_t *udata = (H5O_iter_wrt_t *)_udata;   /* Operator user data */
    H5O_chunk_proxy_t *chk_proxy = NULL;        /* Chunk that message is in */
    hbool_t chk_dirtied = FALSE;        /* Flag for unprotecting chunk */
    herr_t ret_value = H5_ITER_CONT;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_write_cb)

    /* check args */
    HDassert(oh);
    HDassert(mesg);
    HDassert(!udata->found);

    /* Check for correct attribute message to modify */
    if(0 == HDstrcmp(((H5A_t *)mesg->native)->shared->name, udata->attr->shared->name)) {
        /* Protect chunk */
        if(NULL == (chk_proxy = H5O_chunk_protect(udata->f, udata->dxpl_id, oh, mesg->chunkno)))
            HGOTO_ERROR(H5E_ATTR, H5E_CANTPROTECT, H5_ITER_ERROR, "unable to load object header chunk")

        /* Allocate storage for the message's data, if necessary */
        if(NULL == ((H5A_t *)mesg->native)->shared->data)
            if(NULL == (((H5A_t *)mesg->native)->shared->data = H5FL_BLK_MALLOC(attr_buf, udata->attr->shared->data_size)))
                HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, H5_ITER_ERROR, "memory allocation failed")

        /* Copy the data into the header message */
        /* (Needs to occur before updating the shared message, or the hash
         *      value on the old & new messages will be the same)
         */
        HDmemcpy(((H5A_t *)mesg->native)->shared->data, udata->attr->shared->data, udata->attr->shared->data_size);

        /* Mark the message as modified */
        mesg->dirty = TRUE;
        chk_dirtied = TRUE;

        /* Release chunk */
        if(H5O_chunk_unprotect(udata->f, udata->dxpl_id, chk_proxy, chk_dirtied) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, H5_ITER_ERROR, "unable to unprotect object header chunk")
        chk_proxy = NULL;

        /* Update the shared attribute in the SOHM storage */
        if(mesg->flags & H5O_MSG_FLAG_SHARED)
            if(H5O_attr_update_shared(udata->f, udata->dxpl_id, oh, udata->attr, (H5O_shared_t *)mesg->native) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, H5_ITER_ERROR, "unable to update attribute in shared storage")

        /* Indicate that the object header was modified */
        *oh_modified = TRUE;

        /* Indicate that the attribute was found */
        udata->found = TRUE;

        /* Stop iterating */
        ret_value = H5_ITER_STOP;
    } /* end if */

done:
    /* Release chunk, if not already done */
    if(chk_proxy && H5O_chunk_unprotect(udata->f, udata->dxpl_id, chk_proxy, chk_dirtied) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, H5_ITER_ERROR, "unable to unprotect object header chunk")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_write_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_write
 *
 * Purpose:	Write a new value to an attribute.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Monday, December  4, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_write(const H5O_loc_t *loc, hid_t dxpl_id, H5A_t *attr)
{
    H5O_t *oh = NULL;                   /* Pointer to actual object header */
    H5O_ainfo_t ainfo;                  /* Attribute information for object */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_write)

    /* Check arguments */
    HDassert(loc);
    HDassert(attr);

    /* Pin the object header */
    if(NULL == (oh = H5O_pin(loc, dxpl_id)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPIN, FAIL, "unable to pin object header")

    /* Check for attribute info stored */
    ainfo.fheap_addr = HADDR_UNDEF;
    if(oh->version > H5O_VERSION_1) {
        /* Check for (& retrieve if available) attribute info */
        if(H5A_get_ainfo(loc->file, dxpl_id, oh, &ainfo) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
    } /* end if */

    /* Check for attributes stored densely */
    if(H5F_addr_defined(ainfo.fheap_addr)) {
        /* Modify the attribute data in dense storage */
        if(H5A_dense_write(loc->file, dxpl_id, &ainfo, attr) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "error updating attribute")
    } /* end if */
    else {
        H5O_iter_wrt_t udata;           /* User data for callback */
        H5O_mesg_operator_t op;         /* Wrapper for operator */

        /* Set up user data for callback */
        udata.f = loc->file;
        udata.dxpl_id = dxpl_id;
        udata.attr = attr;
        udata.found = FALSE;

        /* Iterate over attributes, to locate correct one to update */
        op.op_type = H5O_MESG_OP_LIB;
        op.u.lib_op = H5O_attr_write_cb;
        if(H5O_msg_iterate_real(loc->file, oh, H5O_MSG_ATTR, &op, &udata, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "error updating attribute")

        /* Check that we found the attribute */
        if(!udata.found)
            HGOTO_ERROR(H5E_ATTR, H5E_NOTFOUND, FAIL, "can't locate open attribute?")
    } /* end else */

    /* Update the modification time, if any */
    if(H5O_touch_oh(loc->file, dxpl_id, oh, FALSE) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update time on object")

done:
    if(oh && H5O_unpin(oh) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPIN, FAIL, "unable to unpin object header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_write */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_rename_chk_cb
 *
 * Purpose:	Object header iterator callback routine to check for
 *              duplicate name during rename
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  5 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_attr_rename_chk_cb(H5O_t UNUSED *oh, H5O_mesg_t *mesg/*in,out*/,
    unsigned UNUSED sequence, hbool_t UNUSED *oh_modified, void *_udata/*in,out*/)
{
    H5O_iter_ren_t *udata = (H5O_iter_ren_t *)_udata;   /* Operator user data */
    herr_t ret_value = H5_ITER_CONT;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5O_attr_rename_chk_cb)

    /* check args */
    HDassert(oh);
    HDassert(mesg);
    HDassert(!udata->found);

    /* Check for existing attribute with new name */
    if(HDstrcmp(((H5A_t *)mesg->native)->shared->name, udata->new_name) == 0) {
        /* Indicate that we found an existing attribute with the new name*/
        udata->found = TRUE;

        /* Stop iterating */
        ret_value = H5_ITER_STOP;
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_rename_chk_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_rename_mod_cb
 *
 * Purpose:	Object header iterator callback routine to change name of
 *              attribute during rename
 *
 * Note:	This routine doesn't currently allow an attribute to change
 *              its "shared" status, if the name change would cause a size
 *              difference that would put it into a different category.
 *              Something for later...
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Dec  5 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_attr_rename_mod_cb(H5O_t *oh, H5O_mesg_t *mesg/*in,out*/,
    unsigned UNUSED sequence, hbool_t *oh_modified, void *_udata/*in,out*/)
{
    H5O_iter_ren_t *udata = (H5O_iter_ren_t *)_udata;   /* Operator user data */
    H5O_chunk_proxy_t *chk_proxy = NULL;        /* Chunk that message is in */
    hbool_t chk_dirtied = FALSE;        /* Flag for unprotecting chunk */
    herr_t ret_value = H5_ITER_CONT;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_rename_mod_cb)

    /* check args */
    HDassert(oh);
    HDassert(mesg);
    HDassert(!udata->found);

    /* Find correct attribute message to rename */
    if(HDstrcmp(((H5A_t *)mesg->native)->shared->name, udata->old_name) == 0) {
        unsigned old_version = ((H5A_t *)mesg->native)->shared->version;        /* Old version of the attribute */

        /* Protect chunk */
        if(NULL == (chk_proxy = H5O_chunk_protect(udata->f, udata->dxpl_id, oh, mesg->chunkno)))
            HGOTO_ERROR(H5E_ATTR, H5E_CANTPROTECT, H5_ITER_ERROR, "unable to load object header chunk")

        /* Change the name for the attribute */
        H5MM_xfree(((H5A_t *)mesg->native)->shared->name);
        ((H5A_t *)mesg->native)->shared->name = H5MM_xstrdup(udata->new_name);

        /* Recompute the version to encode the attribute with */
        if(H5A_set_version(udata->f, ((H5A_t *)mesg->native)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTSET, H5_ITER_ERROR, "unable to update attribute version")

        /* Mark the message as modified */
        mesg->dirty = TRUE;
        chk_dirtied = TRUE;

        /* Release chunk */
        if(H5O_chunk_unprotect(udata->f, udata->dxpl_id, chk_proxy, chk_dirtied) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, H5_ITER_ERROR, "unable to unprotect object header chunk")
        chk_proxy = NULL;

        /* Check for shared message */
        if(mesg->flags & H5O_MSG_FLAG_SHARED) {
            /* Update the shared attribute in the SOHM storage */
            if(H5O_attr_update_shared(udata->f, udata->dxpl_id, oh, (H5A_t *)mesg->native, NULL) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, H5_ITER_ERROR, "unable to update attribute in shared storage")
        } /* end if */
        else {
            /* Sanity check */
            HDassert(H5O_msg_is_shared(H5O_ATTR_ID, (H5A_t *)mesg->native) == FALSE);

            /* Check for attribute message changing size */
            if(HDstrlen(udata->new_name) != HDstrlen(udata->old_name) ||
                    old_version != ((H5A_t *)mesg->native)->shared->version) {
                H5A_t *attr;            /* Attribute to re-add */

                /* Take ownership of the message's native info (the attribute)
                 *      so any shared objects in the file aren't adjusted (and
                 *      possibly deleted) when the message is released.
                 */
                /* (We do this more complicated sequence of actions because the
                 *      simpler solution of adding the modified attribute first
                 *      and then deleting the old message can re-allocate the
                 *      list of messages during the "add the modified attribute"
                 *      step, invalidating the message pointer we have here - QAK)
                 */
                attr = (H5A_t *)mesg->native;
                mesg->native = NULL;

                /* Delete old attribute */
                /* (doesn't decrement the link count on shared components becuase
                 *      the "native" pointer has been reset)
                 */
                if(H5O_release_mesg(udata->f, udata->dxpl_id, oh, mesg, FALSE) < 0)
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, H5_ITER_ERROR, "unable to release previous attribute")

                /* Append renamed attribute to object header */
                /* (Don't let it become shared) */
                if(H5O_msg_append_real(udata->f, udata->dxpl_id, oh, H5O_MSG_ATTR, (mesg->flags | H5O_MSG_FLAG_DONTSHARE), 0, attr) < 0)
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTINSERT, H5_ITER_ERROR, "unable to relocate renamed attribute in header")

                /* Sanity check */
                HDassert(H5O_msg_is_shared(H5O_ATTR_ID, attr) == FALSE);

                /* Close the local copy of the attribute */
                H5A_close(attr);
            } /* end if */
        } /* end else */

        /* Indicate that the object header was modified */
        *oh_modified = TRUE;

        /* Indicate that we found an existing attribute with the old name */
        udata->found = TRUE;

        /* Stop iterating */
        ret_value = H5_ITER_STOP;
    } /* end if */

done:
    /* Release chunk, if not already done */
    if(chk_proxy && H5O_chunk_unprotect(udata->f, udata->dxpl_id, chk_proxy, chk_dirtied) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, H5_ITER_ERROR, "unable to unprotect object header chunk")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_rename_mod_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_rename
 *
 * Purpose:	Rename an attribute.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  5, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_rename(const H5O_loc_t *loc, hid_t dxpl_id, const char *old_name,
    const char *new_name)
{
    H5O_t *oh = NULL;                   /* Pointer to actual object header */
    H5O_ainfo_t ainfo;                  /* Attribute information for object */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_rename)

    /* Check arguments */
    HDassert(loc);
    HDassert(old_name);
    HDassert(new_name);

    /* Pin the object header */
    if(NULL == (oh = H5O_pin(loc, dxpl_id)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPIN, FAIL, "unable to pin object header")

    /* Check for attribute info stored */
    ainfo.fheap_addr = HADDR_UNDEF;
    if(oh->version > H5O_VERSION_1) {
        /* Check for (& retrieve if available) attribute info */
        if(H5A_get_ainfo(loc->file, dxpl_id, oh, &ainfo) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
    } /* end if */

    /* Check for attributes stored densely */
    if(H5F_addr_defined(ainfo.fheap_addr)) {
        /* Rename the attribute data in dense storage */
        if(H5A_dense_rename(loc->file, dxpl_id, &ainfo, old_name, new_name) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "error updating attribute")
    } /* end if */
    else {
        H5O_iter_ren_t udata;           /* User data for callback */
        H5O_mesg_operator_t op;         /* Wrapper for operator */

        /* Set up user data for callback */
        udata.f = loc->file;
        udata.dxpl_id = dxpl_id;
        udata.old_name = old_name;
        udata.new_name = new_name;
        udata.found = FALSE;

        /* Iterate over attributes, to check if "new name" exists already */
        op.op_type = H5O_MESG_OP_LIB;
        op.u.lib_op = H5O_attr_rename_chk_cb;
        if(H5O_msg_iterate_real(loc->file, oh, H5O_MSG_ATTR, &op, &udata, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "error updating attribute")

        /* If the new name was found, indicate an error */
        if(udata.found)
            HGOTO_ERROR(H5E_ATTR, H5E_EXISTS, FAIL, "attribute with new name already exists")

        /* Iterate over attributes again, to actually rename attribute with old name */
        op.op_type = H5O_MESG_OP_LIB;
        op.u.lib_op = H5O_attr_rename_mod_cb;
        if(H5O_msg_iterate_real(loc->file, oh, H5O_MSG_ATTR, &op, &udata, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "error updating attribute")

        /* Check that we found the attribute to rename */
        if(!udata.found)
            HGOTO_ERROR(H5E_ATTR, H5E_NOTFOUND, FAIL, "can't locate attribute with old name")
    } /* end else */

    /* Update the modification time, if any */
    if(H5O_touch_oh(loc->file, dxpl_id, oh, FALSE) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update time on object")

done:
    if(oh && H5O_unpin(oh) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPIN, FAIL, "unable to unpin object header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_rename */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_iterate_real
 *
 * Purpose:	Internal routine to iterate over attributes for an object.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  5, 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_attr_iterate_real(hid_t loc_id, const H5O_loc_t *loc, hid_t dxpl_id,
    H5_index_t idx_type, H5_iter_order_t order, hsize_t skip,
    hsize_t *last_attr, const H5A_attr_iter_op_t *attr_op, void *op_data)
{
    H5O_t *oh = NULL;                   /* Pointer to actual object header */
    H5O_ainfo_t ainfo;                  /* Attribute information for object */
    H5A_attr_table_t atable = {0, NULL};        /* Table of attributes */
    herr_t ret_value;                   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_iterate_real)

    /* Check arguments */
    HDassert(loc);
    HDassert(loc->file);
    HDassert(H5F_addr_defined(loc->addr));
    HDassert(attr_op);

    /* Protect the object header to iterate over */
    if(NULL == (oh = H5O_protect(loc, dxpl_id, H5AC_READ)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPROTECT, FAIL, "unable to load object header")

    /* Check for attribute info stored */
    ainfo.fheap_addr = HADDR_UNDEF;
    if(oh->version > H5O_VERSION_1) {
        /* Check for (& retrieve if available) attribute info */
        if(H5A_get_ainfo(loc->file, dxpl_id, oh, &ainfo) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
    } /* end if */

    /* Check for attributes stored densely */
    if(H5F_addr_defined(ainfo.fheap_addr)) {
        /* Check for skipping too many attributes */
        if(skip > 0 && skip >= ainfo.nattrs)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid index specified")

        /* Release the object header */
        if(H5O_unprotect(loc, dxpl_id, oh, H5AC__NO_FLAGS_SET) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, FAIL, "unable to release object header")
        oh = NULL;

        /* Iterate over attributes in dense storage */
        if((ret_value = H5A_dense_iterate(loc->file, dxpl_id, loc_id, &ainfo, idx_type, order, skip, last_attr, attr_op, op_data)) < 0)
            HERROR(H5E_ATTR, H5E_BADITER, "error iterating over attributes");
    } /* end if */
    else {
        /* Build table of attributes for compact storage */
        if(H5A_compact_build_table(loc->file, dxpl_id, oh, idx_type, order, &atable) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, FAIL, "error building attribute table")

        /* Release the object header */
        if(H5O_unprotect(loc, dxpl_id, oh, H5AC__NO_FLAGS_SET) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, FAIL, "unable to release object header")
        oh = NULL;

        /* Check for skipping too many attributes */
        if(skip > 0 && skip >= atable.nattrs)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid index specified")

        /* Iterate over attributes in table */
        if((ret_value = H5A_attr_iterate_table(&atable, skip, last_attr, loc_id, attr_op, op_data)) < 0)
            HERROR(H5E_ATTR, H5E_CANTNEXT, "iteration operator failed");
    } /* end else */

done:
    /* Release resources */
    if(oh && H5O_unprotect(loc, dxpl_id, oh, H5AC__NO_FLAGS_SET) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, FAIL, "unable to release object header")
    if(atable.attrs && H5A_attr_release_table(&atable) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTFREE, FAIL, "unable to release attribute table")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_iterate_real() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_iterate
 *
 * Purpose:	Iterate over attributes for an object.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  5, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_iterate(hid_t loc_id, hid_t dxpl_id,
    H5_index_t idx_type, H5_iter_order_t order, hsize_t skip,
    hsize_t *last_attr, const H5A_attr_iter_op_t *attr_op, void *op_data)
{
    H5G_loc_t loc;	        /* Object location */
    herr_t ret_value;           /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_iterate)

    /* Check arguments */
    HDassert(attr_op);

    /* Look up location for location ID */
    if(H5G_loc(loc_id, &loc) < 0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")

    /* Iterate over attributes to locate correct one */
    if((ret_value = H5O_attr_iterate_real(loc_id, loc.oloc, dxpl_id, idx_type, order, skip, last_attr, attr_op, op_data)) < 0)
        HERROR(H5E_ATTR, H5E_BADITER, "error iterating over attributes");

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_iterate() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_remove_update
 *
 * Purpose:	Check for reverting from dense to compact attribute storage
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Wednesday, February 14, 2007
 *
 * Modification:Raymond Lu
 *              24 June 2008
 *              When converting storage from dense to compact, if found
 *              the attribute is already opened, use the opened message
 *              to insert.  If not, still use the message in the attribute
 *              table. This will guarantee that the attribute message is
 *              shared between the object in metadata cache and the opened
 *              object.
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_attr_remove_update(const H5O_loc_t *loc, H5O_t *oh, H5O_ainfo_t *ainfo,
    hid_t dxpl_id)
{
    H5A_attr_table_t atable = {0, NULL};        /* Table of attributes */
    herr_t ret_value = SUCCEED;                 /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_remove_update)

    /* Check arguments */
    HDassert(loc);
    HDassert(oh);
    HDassert(ainfo);

    /* Decrement the number of attributes on the object */
    ainfo->nattrs--;

    /* Check for shifting from dense storage back to compact storage */
    if(H5F_addr_defined(ainfo->fheap_addr) && ainfo->nattrs < oh->min_dense) {
        hbool_t can_convert = TRUE;     /* Whether converting to attribute messages is possible */
        size_t u;                       /* Local index */

        /* Build the table of attributes for this object */
        if(H5A_dense_build_table(loc->file, dxpl_id, ainfo, H5_INDEX_NAME, H5_ITER_NATIVE, &atable) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, FAIL, "error building attribute table")

        /* Inspect attributes in table for ones that can't be converted back
         * into attribute message form (currently only attributes which
         * can't fit into an object header message)
         */
        for(u = 0; u < ainfo->nattrs; u++)
            if(H5O_msg_size_oh(loc->file, oh, H5O_ATTR_ID, (atable.attrs[u]), (size_t)0) >= H5O_MESG_MAX_SIZE) {
                can_convert = FALSE;
                break;
            } /* end if */

        /* If ok, insert attributes as object header messages */
        if(can_convert) {
            H5A_t *exist_attr = NULL;
            htri_t found_open_attr = FALSE;

            /* Iterate over attributes, to put them into header */
            for(u = 0; u < ainfo->nattrs; u++) {
                htri_t shared_mesg;             /* Should this message be stored in the Shared Message table? */

                /* Check if attribute is shared */
                if((shared_mesg = H5O_msg_is_shared(H5O_ATTR_ID, (atable.attrs[u]))) < 0)
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "error determining if message is shared")
                else if(shared_mesg == 0) {
                    /* Increment reference count on attribute components */
                    /* (so that they aren't deleted when the dense attribute storage is deleted) */
                    if(H5O_attr_link(loc->file, dxpl_id, oh, (atable.attrs[u])) < 0)
                        HGOTO_ERROR(H5E_ATTR, H5E_LINKCOUNT, FAIL, "unable to adjust attribute link count")
                } /* end if */
                else {
                    /* Reset 'shared' status, so attribute will be shared again */
                    (atable.attrs[u])->sh_loc.type = H5O_SHARE_TYPE_UNSHARED;
                } /* end else */

                /* Insert attribute message into object header (Will increment
                   reference count on shared attributes) */
                /* Find out whether the attribute has been opened */
                if((found_open_attr = H5O_attr_find_opened_attr(loc, &exist_attr, (atable.attrs[u])->shared->name)) < 0)
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "failed in finding opened attribute")

                /* If found the attribute is already opened, use the opened message to insert.
                   If not, still use the message in the attribute table. */
                if(found_open_attr && exist_attr) {
                    if(H5O_msg_append_real(loc->file, dxpl_id, oh, H5O_MSG_ATTR, 0, 0, exist_attr) < 0)
                        HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, FAIL, "can't create message")

                } else {
                    if(H5O_msg_append_real(loc->file, dxpl_id, oh, H5O_MSG_ATTR, 0, 0, (atable.attrs[u])) < 0)
                        HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, FAIL, "can't create message")
                }
            } /* end for */

            /* Remove the dense storage */
            if(H5A_dense_delete(loc->file, dxpl_id, ainfo) < 0)
                HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete dense attribute storage")
        } /* end if */
    } /* end if */

    /* Check if we have deleted all the attributes and the attribute info
     *  message should be deleted itself.
     */
    if(ainfo->nattrs == 0) {
        if(H5O_msg_remove_real(loc->file, oh, H5O_MSG_AINFO, H5O_ALL, NULL, NULL, TRUE, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete attribute info")
    } /* end if */
    else {
        if(H5O_msg_write_real(loc->file, dxpl_id, oh, H5O_MSG_AINFO, H5O_MSG_FLAG_DONTSHARE, 0, ainfo) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update attribute info message")
    } /* end else */

done:
    /* Release resources */
    if(atable.attrs && H5A_attr_release_table(&atable) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTFREE, FAIL, "unable to release attribute table")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_remove_update() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_remove_cb
 *
 * Purpose:	Object header iterator callback routine to remove an
 *              attribute stored compactly.
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
H5O_attr_remove_cb(H5O_t *oh, H5O_mesg_t *mesg/*in,out*/,
    unsigned UNUSED sequence, hbool_t *oh_modified, void *_udata/*in,out*/)
{
    H5O_iter_rm_t *udata = (H5O_iter_rm_t *)_udata;   /* Operator user data */
    herr_t ret_value = H5_ITER_CONT;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_remove_cb)

    /* check args */
    HDassert(oh);
    HDassert(mesg);
    HDassert(!udata->found);

    /* Check for correct attribute message to modify */
    if(HDstrcmp(((H5A_t *)mesg->native)->shared->name, udata->name) == 0) {
        /* Convert message into a null message (i.e. delete it) */
        if(H5O_release_mesg(udata->f, udata->dxpl_id, oh, mesg, TRUE) < 0)
            HGOTO_ERROR(H5E_OHDR, H5E_CANTDELETE, H5_ITER_ERROR, "unable to convert into null message")

        /* Indicate that the object header was modified */
        *oh_modified = TRUE;

        /* Indicate that this message is the attribute to be deleted */
        udata->found = TRUE;

        /* Stop iterating */
        ret_value = H5_ITER_STOP;
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_remove_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_remove
 *
 * Purpose:	Delete an attribute on an object.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Monday, December 11, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_remove(const H5O_loc_t *loc, const char *name, hid_t dxpl_id)
{
    H5O_t *oh = NULL;                   /* Pointer to actual object header */
    H5O_ainfo_t ainfo;                  /* Attribute information for object */
    htri_t ainfo_exists = FALSE;        /* Whether the attribute info exists in the file */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_remove)

    /* Check arguments */
    HDassert(loc);
    HDassert(name);

    /* Pin the object header */
    if(NULL == (oh = H5O_pin(loc, dxpl_id)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPIN, FAIL, "unable to pin object header")

    /* Check for attribute info stored */
    ainfo.fheap_addr = HADDR_UNDEF;
    if(oh->version > H5O_VERSION_1) {
        /* Check for (& retrieve if available) attribute info */
        if((ainfo_exists = H5A_get_ainfo(loc->file, dxpl_id, oh, &ainfo)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
    } /* end if */

    /* Check for attributes stored densely */
    if(H5F_addr_defined(ainfo.fheap_addr)) {
        /* Delete attribute from dense storage */
        if(H5A_dense_remove(loc->file, dxpl_id, &ainfo, name) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete attribute in dense storage")
    } /* end if */
    else {
        H5O_iter_rm_t udata;            /* User data for callback */
        H5O_mesg_operator_t op;         /* Wrapper for operator */

        /* Set up user data for callback */
        udata.f = loc->file;
        udata.dxpl_id = dxpl_id;
        udata.name = name;
        udata.found = FALSE;

        /* Iterate over attributes, to locate correct one to delete */
        op.op_type = H5O_MESG_OP_LIB;
        op.u.lib_op = H5O_attr_remove_cb;
        if(H5O_msg_iterate_real(loc->file, oh, H5O_MSG_ATTR, &op, &udata, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "error deleting attribute")

        /* Check that we found the attribute */
        if(!udata.found)
            HGOTO_ERROR(H5E_ATTR, H5E_NOTFOUND, FAIL, "can't locate attribute")
    } /* end else */

    /* Update the attribute information after removing an attribute */
    if(ainfo_exists)
        if(H5O_attr_remove_update(loc, oh, &ainfo, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update attribute info")

    /* Update the modification time, if any */
    if(H5O_touch_oh(loc->file, dxpl_id, oh, FALSE) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update time on object")

done:
    if(oh && H5O_unpin(oh) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPIN, FAIL, "unable to unpin object header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_remove() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_remove_by_idx
 *
 * Purpose:	Delete an attribute on an object, according to an order within
 *		an index.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Wednesday, February 14, 2007
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_remove_by_idx(const H5O_loc_t *loc, H5_index_t idx_type,
    H5_iter_order_t order, hsize_t n, hid_t dxpl_id)
{
    H5O_t *oh = NULL;                   /* Pointer to actual object header */
    H5O_ainfo_t ainfo;                  /* Attribute information for object */
    htri_t ainfo_exists = FALSE;        /* Whether the attribute info exists in the file */
    H5A_attr_table_t atable = {0, NULL};        /* Table of attributes */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_remove_by_idx)

    /* Check arguments */
    HDassert(loc);

    /* Pin the object header */
    if(NULL == (oh = H5O_pin(loc, dxpl_id)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPIN, FAIL, "unable to pin object header")

    /* Check for attribute info stored */
    ainfo.fheap_addr = HADDR_UNDEF;
    if(oh->version > H5O_VERSION_1) {
        /* Check for (& retrieve if available) attribute info */
        if((ainfo_exists = H5A_get_ainfo(loc->file, dxpl_id, oh, &ainfo)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
    } /* end if */

    /* Check for attributes stored densely */
    if(H5F_addr_defined(ainfo.fheap_addr)) {
        /* Delete attribute from dense storage */
        if(H5A_dense_remove_by_idx(loc->file, dxpl_id, &ainfo, idx_type, order, n) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete attribute in dense storage")
    } /* end if */
    else {
        H5O_iter_rm_t udata;            /* User data for callback */
        H5O_mesg_operator_t op;         /* Wrapper for operator */

        /* Build table of attributes for compact storage */
        if(H5A_compact_build_table(loc->file, dxpl_id, oh, idx_type, order, &atable) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTINIT, FAIL, "error building attribute table")

        /* Check for skipping too many attributes */
        if(n >= atable.nattrs)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid index specified")

        /* Set up user data for callback, to remove the attribute by name */
        udata.f = loc->file;
        udata.dxpl_id = dxpl_id;
        udata.name = ((atable.attrs[n])->shared)->name;
        udata.found = FALSE;

        /* Iterate over attributes, to locate correct one to delete */
        op.op_type = H5O_MESG_OP_LIB;
        op.u.lib_op = H5O_attr_remove_cb;
        if(H5O_msg_iterate_real(loc->file, oh, H5O_MSG_ATTR, &op, &udata, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "error deleting attribute")

        /* Check that we found the attribute */
        if(!udata.found)
            HGOTO_ERROR(H5E_ATTR, H5E_NOTFOUND, FAIL, "can't locate attribute")
    } /* end else */

    /* Update the attribute information after removing an attribute */
    if(ainfo_exists)
        if(H5O_attr_remove_update(loc, oh, &ainfo, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update attribute info")

    /* Update the modification time, if any */
    if(H5O_touch_oh(loc->file, dxpl_id, oh, FALSE) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTUPDATE, FAIL, "unable to update time on object")

done:
    if(oh && H5O_unpin(oh) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPIN, FAIL, "unable to unpin object header")
    if(atable.attrs && H5A_attr_release_table(&atable) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTFREE, FAIL, "unable to release attribute table")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_remove_by_idx() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_count_real
 *
 * Purpose:	Determine the # of attributes on an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Thursday, March  9, 2007
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_count_real(H5F_t *f, hid_t dxpl_id, H5O_t *oh, hsize_t *nattrs)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_count_real)

    /* Check arguments */
    HDassert(f);
    HDassert(oh);
    HDassert(nattrs);

    /* Check for attributes stored densely */
    if(oh->version > H5O_VERSION_1) {
        htri_t ainfo_exists = FALSE;        /* Whether the attribute info exists in the file */
        H5O_ainfo_t ainfo;                  /* Attribute information for object */

        /* Attempt to get the attribute information from the object header */
        if((ainfo_exists = H5A_get_ainfo(f, dxpl_id, oh, &ainfo)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
        else if(ainfo_exists > 0)
            *nattrs = ainfo.nattrs;
        else
            *nattrs = 0;
    } /* end if */
    else {
        hsize_t attr_count;     /* Number of attributes found */
        unsigned u;             /* Local index variable */

        /* Loop over all messages, counting the attributes */
        attr_count = 0;
        for(u = 0; u < oh->nmesgs; u++)
            if(oh->mesg[u].type == H5O_MSG_ATTR)
                attr_count++;
        *nattrs = attr_count;
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_count_real */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_exists_cb
 *
 * Purpose:	Object header iterator callback routine to check for an
 *              attribute stored compactly, by name.
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
H5O_attr_exists_cb(H5O_t UNUSED *oh, H5O_mesg_t *mesg/*in,out*/,
    unsigned UNUSED sequence, hbool_t UNUSED *oh_modified, void *_udata/*in,out*/)
{
    H5O_iter_rm_t *udata = (H5O_iter_rm_t *)_udata;   /* Operator user data */
    herr_t ret_value = H5_ITER_CONT;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5O_attr_exists_cb)

    /* check args */
    HDassert(mesg);
    HDassert(!udata->found);

    /* Check for correct attribute message */
    if(HDstrcmp(((H5A_t *)mesg->native)->shared->name, udata->name) == 0) {
        /* Indicate that this message is the attribute sought */
        udata->found = TRUE;

        /* Stop iterating */
        ret_value = H5_ITER_STOP;
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_exists_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5O_attr_exists
 *
 * Purpose:	Determine if an attribute with a particular name exists on an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Monday, December 11, 2006
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5O_attr_exists(const H5O_loc_t *loc, const char *name, hid_t dxpl_id)
{
    H5O_t *oh = NULL;           /* Pointer to actual object header */
    H5O_ainfo_t ainfo;          /* Attribute information for object */
    htri_t ret_value;           /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_exists)

    /* Check arguments */
    HDassert(loc);
    HDassert(name);

    /* Protect the object header to iterate over */
    if(NULL == (oh = H5O_protect(loc, dxpl_id, H5AC_READ)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPROTECT, FAIL, "unable to load object header")

    /* Check for attribute info stored */
    ainfo.fheap_addr = HADDR_UNDEF;
    if(oh->version > H5O_VERSION_1) {
        /* Check for (& retrieve if available) attribute info */
        if(H5A_get_ainfo(loc->file, dxpl_id, oh, &ainfo) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
    } /* end if */

    /* Check for attributes stored densely */
    if(H5F_addr_defined(ainfo.fheap_addr)) {
        /* Check if attribute exists in dense storage */
        if((ret_value = H5A_dense_exists(loc->file, dxpl_id, &ainfo, name)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_BADITER, FAIL, "error checking for existence of attribute")
    } /* end if */
    else {
        H5O_iter_rm_t udata;            /* User data for callback */
        H5O_mesg_operator_t op;         /* Wrapper for operator */

        /* Set up user data for callback */
        udata.f = loc->file;
        udata.dxpl_id = dxpl_id;
        udata.name = name;
        udata.found = FALSE;

        /* Iterate over existing attributes, checking for attribute with same name */
        op.op_type = H5O_MESG_OP_LIB;
        op.u.lib_op = H5O_attr_exists_cb;
        if(H5O_msg_iterate_real(loc->file, oh, H5O_MSG_ATTR, &op, &udata, dxpl_id) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_BADITER, FAIL, "error checking for existence of attribute")

        /* Check that we found the attribute */
        ret_value = (htri_t)udata.found;
    } /* end else */

done:
    if(oh && H5O_unprotect(loc, dxpl_id, oh, H5AC__NO_FLAGS_SET) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, FAIL, "unable to release object header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_exists */


/*-------------------------------------------------------------------------
 * Function:    H5O_attr_bh_info
 *
 * Purpose:     For 1.8 attribute, returns storage amount for btree and fractal heap
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Vailin Choi
 *              June 19, 2007
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_attr_bh_info(H5F_t *f, hid_t dxpl_id, H5O_t *oh, H5_ih_info_t *bh_info)
{
    H5HF_t      *fheap = NULL;              /* Fractal heap handle */
    H5B2_t      *bt2_name = NULL;           /* v2 B-tree handle for name index */
    H5B2_t      *bt2_corder = NULL;         /* v2 B-tree handle for creation order index */
    herr_t      ret_value = SUCCEED;        /* Return value */

    FUNC_ENTER_NOAPI(H5O_attr_bh_info, FAIL)

    HDassert(f);
    HDassert(oh);
    HDassert(bh_info);

    /* Attributes are only stored in fractal heap & indexed w/v2 B-tree in later versions */
    if(oh->version > H5O_VERSION_1) {
        H5O_ainfo_t ainfo;          /* Attribute information for object */
        htri_t ainfo_exists = FALSE;        /* Whether the attribute info exists in the file */

        /* Check for (& retrieve if available) attribute info */
        if((ainfo_exists = H5A_get_ainfo(f, dxpl_id, oh, &ainfo)) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't check for attribute info message")
        else if(ainfo_exists > 0) {
            /* Check if name index available */
            if(H5F_addr_defined(ainfo.name_bt2_addr)) {
                /* Open the name index v2 B-tree */
                if(NULL == (bt2_name = H5B2_open(f, dxpl_id, ainfo.name_bt2_addr, NULL)))
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open v2 B-tree for name index")

                /* Get name index B-tree size */
                if(H5B2_size(bt2_name, dxpl_id, &(bh_info->index_size)) < 0)
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't retrieve B-tree storage info")
            } /* end if */

            /* Check if creation order index available */
            if(H5F_addr_defined(ainfo.corder_bt2_addr)) {
                /* Open the creation order index v2 B-tree */
                if(NULL == (bt2_corder = H5B2_open(f, dxpl_id, ainfo.corder_bt2_addr, NULL)))
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open v2 B-tree for creation order index")

                /* Get creation order index B-tree size */
                if(H5B2_size(bt2_corder, dxpl_id, &(bh_info->index_size)) < 0)
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't retrieve B-tree storage info")
            } /* end if */

            /* Get storage size of fractal heap, if it's used */
            if(H5F_addr_defined(ainfo.fheap_addr)) {
                /* Open the fractal heap for attributes */
                if(NULL == (fheap = H5HF_open(f, dxpl_id, ainfo.fheap_addr)))
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open fractal heap")

                /* Get heap storage size */
                if(H5HF_size(fheap, dxpl_id, &(bh_info->heap_size)) < 0)
                    HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't retrieve B-tree storage info")
            } /* end if */
        } /* end else */
    } /* end if */

done:
    /* Release resources */
    if(fheap && H5HF_close(fheap, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTCLOSEOBJ, FAIL, "can't close fractal heap")
    if(bt2_name && H5B2_close(bt2_name, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTCLOSEOBJ, FAIL, "can't close v2 B-tree for name index")
    if(bt2_corder && H5B2_close(bt2_corder, dxpl_id) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTCLOSEOBJ, FAIL, "can't close v2 B-tree for creation order index")

    FUNC_LEAVE_NOAPI(ret_value)
}   /* H5O_attr_bh_info() */

#ifndef H5_NO_DEPRECATED_SYMBOLS

/*-------------------------------------------------------------------------
 * Function:	H5O_attr_count
 *
 * Purpose:	Determine the # of attributes on an object
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Monday, December 11, 2006
 *
 *-------------------------------------------------------------------------
 */
int
H5O_attr_count(const H5O_loc_t *loc, hid_t dxpl_id)
{
    H5O_t *oh = NULL;           /* Pointer to actual object header */
    hsize_t nattrs;             /* Number of attributes */
    int ret_value;              /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_attr_count)

    /* Check arguments */
    HDassert(loc);

    /* Protect the object header to iterate over */
    if(NULL == (oh = H5O_protect(loc, dxpl_id, H5AC_READ)))
	HGOTO_ERROR(H5E_ATTR, H5E_CANTPROTECT, FAIL, "unable to load object header")

    /* Retrieve # of attributes on object */
    if(H5O_attr_count_real(loc->file, dxpl_id, oh, &nattrs) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't retrieve attribute count")

    /* Set return value */
    ret_value = (int)nattrs;

done:
    if(oh && H5O_unprotect(loc, dxpl_id, oh, H5AC__NO_FLAGS_SET) < 0)
        HDONE_ERROR(H5E_ATTR, H5E_CANTUNPROTECT, FAIL, "unable to release object header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_attr_count */
#endif /* H5_NO_DEPRECATED_SYMBOLS */

