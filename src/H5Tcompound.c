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

/*
 * Module Info: This module contains the functionality for compound datatypes
 *      in the H5T interface.
 */

/****************/
/* Module Setup */
/****************/

#define H5T_PACKAGE		/*suppress error about including H5Tpkg	  */

/* Interface initialization */
#define H5_INTERFACE_INIT_FUNC	H5T_init_compound_interface


/***********/
/* Headers */
/***********/
#include "H5private.h"		/*generic functions			  */
#include "H5Eprivate.h"		/*error handling			  */
#include "H5Iprivate.h"		/*ID functions		   		  */
#include "H5MMprivate.h"	/*memory management			  */
#include "H5Tpkg.h"		/*data-type functions			  */

/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/
/* "Key" (+ user data) for bsearch callback */
typedef struct{
    size_t              offset;     /* Offset of member to be added */
    const H5T_cmemb_t   *max_under; /* Member with maximum offset seen that is not above "offset" */
} H5T_insert_compar_t;


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/
static herr_t H5T_pack(const H5T_t *dt);


/*********************/
/* Public Variables */
/*********************/


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/



/*--------------------------------------------------------------------------
NAME
   H5T_init_compound_interface -- Initialize interface-specific information
USAGE
    herr_t H5T_init_compound_interface()
RETURNS
    Non-negative on success/Negative on failure
DESCRIPTION
    Initializes any interface-specific data or routines.  (Just calls
    H5T_init() currently).
--------------------------------------------------------------------------*/
static herr_t
H5T_init_compound_interface(void)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5T_init_compound_interface)

    FUNC_LEAVE_NOAPI(H5T_init())
} /* H5T_init_compound_interface() */


/*-------------------------------------------------------------------------
 * Function:	H5Tget_member_offset
 *
 * Purpose:	Returns the byte offset of the beginning of a member with
 *		respect to the beginning of the compound datatype datum.
 *
 * Return:	Success:	Byte offset.
 *
 *		Failure:	Zero. Zero is a valid offset, but this
 *				function will fail only if a call to
 *				H5Tget_member_dims() fails with the same
 *				arguments.
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January  7, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
size_t
H5Tget_member_offset(hid_t type_id, unsigned membno)
{
    H5T_t	*dt;            /* Datatype to query */
    size_t	ret_value;      /* Return value */

    FUNC_ENTER_API(H5Tget_member_offset, 0)
    H5TRACE2("z", "iIu", type_id, membno);

    /* Check args */
    if(NULL == (dt = (H5T_t *)H5I_object_verify(type_id, H5I_DATATYPE)) || H5T_COMPOUND != dt->shared->type)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, 0, "not a compound datatype")
    if(membno >= dt->shared->u.compnd.nmembs)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, 0, "invalid member number")

    /* Value */
    ret_value = H5T_GET_MEMBER_OFFSET(dt->shared, membno);

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Tget_member_offset() */


/*-------------------------------------------------------------------------
 * Function:	H5T_get_member_offset
 *
 * Purpose:	Private function for H5Tget_member_offset.  Returns the byte
 *              offset of the beginning of a member with respect to the
 *              beginning of the compound datatype datum.
 *
 * Return:	Success:	Byte offset.
 *
 *		Failure:	Zero. Zero is a valid offset, but this
 *				function will fail only if a call to
 *				H5Tget_member_dims() fails with the same
 *				arguments.
 *
 * Programmer:	Raymond Lu
 *		October 8, 2002
 *
 *-------------------------------------------------------------------------
 */
size_t
H5T_get_member_offset(const H5T_t *dt, unsigned membno)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5T_get_member_offset)

    HDassert(dt);
    HDassert(membno < dt->shared->u.compnd.nmembs);

    FUNC_LEAVE_NOAPI(dt->shared->u.compnd.memb[membno].offset)
} /* end H5T_get_member_offset() */


/*-------------------------------------------------------------------------
 * Function:	H5Tget_member_class
 *
 * Purpose:	Returns the datatype class of a member of a compound datatype.
 *
 * Return:	Success: Non-negative
 *
 *		Failure: H5T_NO_CLASS
 *
 * Programmer:	Quincey Koziol
 *		Thursday, November  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5T_class_t
H5Tget_member_class(hid_t type_id, unsigned membno)
{
    H5T_t	*dt;            /* Datatype to query */
    H5T_class_t	ret_value;      /* Return value */

    FUNC_ENTER_API(H5Tget_member_class, H5T_NO_CLASS)
    H5TRACE2("Tt", "iIu", type_id, membno);

    /* Check args */
    if(NULL == (dt = (H5T_t *)H5I_object_verify(type_id, H5I_DATATYPE)) || H5T_COMPOUND != dt->shared->type)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5T_NO_CLASS, "not a compound datatype")
    if(membno >= dt->shared->u.compnd.nmembs)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5T_NO_CLASS, "invalid member number")

    /* Get the type's class.  We have to use this function to get type class
     *  because of the concern of variable-length string.
     */
    ret_value = H5T_GET_CLASS(dt->shared->u.compnd.memb[membno].type->shared, FALSE);

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Tget_member_class() */


/*-------------------------------------------------------------------------
 * Function:	H5Tget_member_type
 *
 * Purpose:	Returns the datatype of the specified member.	The caller
 *		should invoke H5Tclose() to release resources associated with
 *		the type.
 *
 * Return:	Success:	An OID of a copy of the member datatype;
 *				modifying the returned datatype does not
 *				modify the member type.
 *
 *		Failure:	Negative
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January  7, 1998
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Tget_member_type(hid_t type_id, unsigned membno)
{
    H5T_t	*dt;                    /* Datatype to query */
    H5T_t	*memb_dt = NULL;        /* Member datatype */
    hid_t	ret_value;              /* Return value */

    FUNC_ENTER_API(H5Tget_member_type, FAIL)
    H5TRACE2("i", "iIu", type_id, membno);

    /* Check args */
    if(NULL == (dt = (H5T_t *)H5I_object_verify(type_id, H5I_DATATYPE)) || H5T_COMPOUND != dt->shared->type)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a compound datatype")
    if(membno >= dt->shared->u.compnd.nmembs)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid member number")
    if(NULL == (memb_dt = H5T_get_member_type(dt, membno, H5T_COPY_REOPEN)))
	HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINIT, FAIL, "unable to retrieve member type")
    if((ret_value = H5I_register(H5I_DATATYPE, memb_dt, TRUE)) < 0)
	HGOTO_ERROR(H5E_DATATYPE, H5E_CANTREGISTER, FAIL, "unable register datatype atom")

done:
    if(ret_value < 0)
        if(memb_dt && H5T_close(memb_dt) < 0)
            HDONE_ERROR(H5E_DATATYPE, H5E_CANTCLOSEOBJ, FAIL, "can't close datatype")

    FUNC_LEAVE_API(ret_value)
} /* end H5Tget_member_type() */


/*-------------------------------------------------------------------------
 * Function:	H5T_get_member_type
 *
 * Purpose:	Private function for H5Tget_member_type.  Returns the data
 *              type of the specified member.
 *
 * Return:	Success:	A copy of the member datatype;
 *				modifying the returned datatype does not
 *				modify the member type.
 *
 *		Failure:        NULL
 *
 * Programmer:	Raymond Lu
 *	        October 8, 2002
 *
 *-------------------------------------------------------------------------
 */
H5T_t *
H5T_get_member_type(const H5T_t *dt, unsigned membno, H5T_copy_t method)
{
    H5T_t	*ret_value;     /* Return value */

    FUNC_ENTER_NOAPI(H5T_get_member_type, NULL)

    HDassert(dt);
    HDassert(membno < dt->shared->u.compnd.nmembs);

    /* Copy datatype into an atom */
    if(NULL == (ret_value = H5T_copy(dt->shared->u.compnd.memb[membno].type, method)))
	HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINIT, NULL, "unable to copy member datatype")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5T_get_member_type() */


/*-------------------------------------------------------------------------
 * Function:	H5T_get_member_size
 *
 * Purpose:	Returns the size of the specified member.
 *
 * Return:	Success:	The size in bytes of the member's datatype.
 *		Failure:        0
 *
 * Programmer:	Quincey Koziol
 *	        October 4, 2004
 *
 *-------------------------------------------------------------------------
 */
size_t
H5T_get_member_size(const H5T_t *dt, unsigned membno)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5T_get_member_size)

    HDassert(dt);
    HDassert(membno < dt->shared->u.compnd.nmembs);


    FUNC_LEAVE_NOAPI(dt->shared->u.compnd.memb[membno].type->shared->size)
} /* end H5T_get_member_size() */


/*-------------------------------------------------------------------------
 * Function:	H5Tinsert
 *
 * Purpose:	Adds another member to the compound datatype PARENT_ID.  The
 *		new member has a NAME which must be unique within the
 *		compound datatype. The OFFSET argument defines the start of
 *		the member in an instance of the compound datatype, and
 *		MEMBER_ID is the type of the new member.
 *
 * Return:	Success:	Non-negative, the PARENT_ID compound data
 *				type is modified to include a copy of the
 *				member type MEMBER_ID.
 *
 *		Failure:	Negative
 *
 * Errors:
 *
 * Programmer:	Robb Matzke
 *		Monday, December  8, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Tinsert(hid_t parent_id, const char *name, size_t offset, hid_t member_id)
{
    H5T_t	*parent;		/* The compound parent datatype */
    H5T_t	*member;		/* The member datatype	*/
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(H5Tinsert, FAIL)
    H5TRACE4("e", "i*szi", parent_id, name, offset, member_id);

    /* Check args */
    if(parent_id == member_id)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "can't insert compound datatype within itself")
    if(NULL == (parent = (H5T_t *)H5I_object_verify(parent_id, H5I_DATATYPE)) || H5T_COMPOUND != parent->shared->type)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a compound datatype")
    if(H5T_STATE_TRANSIENT != parent->shared->state)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "parent type read-only")
    if(!name || !*name)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no member name")
    if(NULL == (member = (H5T_t *)H5I_object_verify(member_id, H5I_DATATYPE)))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a datatype")

    /* Insert */
    if(H5T_insert(parent, name, offset, member) < 0)
        HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINSERT, FAIL, "unable to insert member")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Tinsert() */


/*-------------------------------------------------------------------------
 * Function:	H5Tpack
 *
 * Purpose:	Recursively removes padding from within a compound datatype
 *		to make it more efficient (space-wise) to store that data.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January  7, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Tpack(hid_t type_id)
{
    H5T_t	*dt;                    /* Datatype to modify */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(H5Tpack, FAIL)
    H5TRACE1("e", "i", type_id);

    /* Check args */
    if(NULL == (dt = (H5T_t *)H5I_object_verify(type_id, H5I_DATATYPE)) || H5T_detect_class(dt, H5T_COMPOUND) <= 0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a compound datatype")

    /* Pack */
    if(H5T_pack(dt) < 0)
	HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINIT, FAIL, "unable to pack compound datatype")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Tpack() */


/*-------------------------------------------------------------------------
 * Function:	H5T_insert_compar
 *
 * Purpose:	Callback function for bsearch called from H5T_insert.
 *              Reports whether obj has a lower of higher offset than
 *              that stored in key.  Also keeps track of the highest
 *              offset seen that is not higher than that in key.
 *
 * Return:	-1 if key < obj
 *              0 if key == obj
 *              1 if key > obj
 *
 * Programmer:	Neil Fortner
 *		Wednesday, January  7, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
H5T_insert_compar(const void *_key, const void *_obj)
{
    H5T_insert_compar_t *key = *(H5T_insert_compar_t * const *)_key;    /* User data */
    const H5T_cmemb_t   *memb = (const H5T_cmemb_t *)_obj;              /* Compound member being examined */
    int                 ret_value;                                      /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5T_insert_compar)

    if(key->offset > memb->offset) {
        if(key->max_under == NULL || memb->offset > key->max_under->offset)
            key->max_under = memb;
        ret_value = 1;
    } /* end if */
    else if(key->offset < memb->offset)
        ret_value = -1;
    else
        ret_value = 0;  /* Should not happen */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5T_insert_compar() */


/*-------------------------------------------------------------------------
 * Function:	H5T_insert
 *
 * Purpose:	Adds a new MEMBER to the compound datatype PARENT.  The new
 *		member will have a NAME that is unique within PARENT and an
 *		instance of PARENT will have the member begin at byte offset
 *		OFFSET from the beginning.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Monday, December  8, 1997
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5T_insert(H5T_t *parent, const char *name, size_t offset, const H5T_t *member)
{
    unsigned	idx;                        /* Index of member to insert */
    size_t	total_size;
    H5T_insert_compar_t key;                /* Key for bsearch compare function */
    H5T_insert_compar_t *keyptr = &key;     /* Pointer to key */
    unsigned	i;                          /* Local index variable */
    herr_t      ret_value = SUCCEED;        /* Return value */

    FUNC_ENTER_NOAPI(H5T_insert, FAIL)

    /* check args */
    HDassert(parent && H5T_COMPOUND == parent->shared->type);
    HDassert(H5T_STATE_TRANSIENT == parent->shared->state);
    HDassert(member);
    HDassert(name && *name);

    /* Does NAME already exist in PARENT? */
    for(i = 0; i < parent->shared->u.compnd.nmembs; i++)
	if(!HDstrcmp(parent->shared->u.compnd.memb[i].name, name))
	    HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINSERT, FAIL, "member name is not unique")

    total_size = member->shared->size;

    /* Does the new member overlap the end of the compound type? */
    if((offset + total_size) > parent->shared->size)
        HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINSERT, FAIL, "member extends past end of compound type")

    if(parent->shared->u.compnd.sorted != H5T_SORT_VALUE)
        if(H5T_sort_value(parent, NULL) < 0)
            HGOTO_ERROR(H5E_INTERNAL, H5E_CANTCOMPARE, FAIL, "value sort failed")

    /* Find the position to insert the new member */
    if(parent->shared->u.compnd.nmembs == 0)
        idx = 0;
    else {
        /* Key value (including user data) for compar callback */
        key.offset = offset;
        key.max_under = NULL;

        /* Do a binary search on the offsets of the (now sorted) members.  We do
         * not expect to find an exact match (if we do it is an error), rely on
         * the user data in the key to keep track of the closest member below
         * the new member. */
        if(NULL != HDbsearch(&keyptr, parent->shared->u.compnd.memb, parent->shared->u.compnd.nmembs,
                sizeof(parent->shared->u.compnd.memb[0]), H5T_insert_compar))
            HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINSERT, FAIL, "member overlaps with another member")
        idx = (key.max_under == NULL) ? 0 : (unsigned) (key.max_under - parent->shared->u.compnd.memb + 1);
    } /* end else */

    /* Does the new member overlap any existing member ? */
    if((idx < parent->shared->u.compnd.nmembs && (offset + total_size) > parent->shared->u.compnd.memb[idx].offset) ||
            (idx && (parent->shared->u.compnd.memb[idx-1].offset + parent->shared->u.compnd.memb[idx-1].size) > offset))
        HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINSERT, FAIL, "member overlaps with another member")

    /* Increase member array if necessary */
    if(parent->shared->u.compnd.nmembs >= parent->shared->u.compnd.nalloc) {
        unsigned na = MAX(1, parent->shared->u.compnd.nalloc * 2);
        H5T_cmemb_t *x = (H5T_cmemb_t *)H5MM_realloc(parent->shared->u.compnd.memb, na * sizeof(H5T_cmemb_t));

        if(!x)
            HGOTO_ERROR(H5E_DATATYPE, H5E_CANTALLOC, FAIL, "memory allocation failed")
        parent->shared->u.compnd.nalloc = na;
        parent->shared->u.compnd.memb = x;
    } /* end if */

    /* Determine if the compound datatype stays packed */
    if(parent->shared->u.compnd.packed) {
        /* Check if the member type is packed */
        if(H5T_is_packed(member) > 0) {
            if(idx == 0) {
                /* If the is the first member, the datatype is not packed
                 * if the first member isn't at offset 0
                 */
                if(offset > 0)
                    parent->shared->u.compnd.packed = FALSE;
            } /* end if */
            else {
                /* If the is not the first member, the datatype is not
                 * packed if the new member isn't adjoining the previous member
                 */
                if(offset != (parent->shared->u.compnd.memb[idx - 1].offset + parent->shared->u.compnd.memb[idx - 1].size))
                    parent->shared->u.compnd.packed = FALSE;
            } /* end else */
        } /* end if */
        else
            parent->shared->u.compnd.packed = FALSE;
    } /* end if */
    else
        /* Check if inserting this member causes the parent to become packed */
        /* First check if it completely closes a gap */
        /* No need to check if it's being appended to the end */
        if(idx != parent->shared->u.compnd.nmembs
                && (offset + total_size) == parent->shared->u.compnd.memb[idx].offset
                && (idx == 0 ? offset == 0  : (parent->shared->u.compnd.memb[idx-1].offset
                + parent->shared->u.compnd.memb[idx-1].size) == offset)
                && H5T_is_packed(member) > 0) {

            /* Start out packed */
            parent->shared->u.compnd.packed = TRUE;

            /* Check if the entire type is now packed */
            if((idx != 0 && parent->shared->u.compnd.memb[0].offset != 0)
                    || !H5T_is_packed(parent->shared->u.compnd.memb[0].type))
                parent->shared->u.compnd.packed = FALSE;
            else
                for(i = 1; i < parent->shared->u.compnd.nmembs; i++)
                    if((i != idx && parent->shared->u.compnd.memb[i].offset
                            != (parent->shared->u.compnd.memb[i - 1].offset
                            + parent->shared->u.compnd.memb[i - 1].size))
                            || !H5T_is_packed(parent->shared->u.compnd.memb[i].type)) {
                        parent->shared->u.compnd.packed = FALSE;
                        break;
                    } /* end if */
        } /* end if */

    /* Reshape the memb array to accomodate the new member */
    if(idx != parent->shared->u.compnd.nmembs)
        HDmemmove(&parent->shared->u.compnd.memb[idx+1], &parent->shared->u.compnd.memb[idx],
                (parent->shared->u.compnd.nmembs - idx) * sizeof(parent->shared->u.compnd.memb[0]));

    /* Add member to member array */
    parent->shared->u.compnd.memb[idx].name = H5MM_xstrdup(name);
    parent->shared->u.compnd.memb[idx].offset = offset;
    parent->shared->u.compnd.memb[idx].size = total_size;
    parent->shared->u.compnd.memb[idx].type = H5T_copy(member, H5T_COPY_ALL);

    parent->shared->u.compnd.nmembs++;

    /* Set the "force conversion" flag if the field's datatype indicates */
    if(member->shared->force_conv == TRUE)
        parent->shared->force_conv = TRUE;

    /* Check for member having a later version than the parent */
    if(parent->shared->version < member->shared->version)
        /* Upgrade parent datatype (and all other members also) */
        /* (can't use a partial datatype and later versions of the format are
         *  more efficient, so might as well upgrade all members also... -QAK)
         */
        if(H5T_upgrade_version(parent, member->shared->version) < 0)
	    HGOTO_ERROR(H5E_DATATYPE, H5E_CANTSET, FAIL, "can't upgrade member encoding version")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5T_insert() */


/*-------------------------------------------------------------------------
 * Function:	H5T_pack
 *
 * Purpose:	Recursively packs a compound datatype by removing padding
 *		bytes. This is done in place (that is, destructively).
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January  7, 1998
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5T_pack(const H5T_t *dt)
{
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5T_pack)

    HDassert(dt);

    if(H5T_detect_class(dt, H5T_COMPOUND) > 0) {
        /* If datatype has been packed, skip packing it and indicate success */
        if(TRUE == H5T_is_packed(dt))
            HGOTO_DONE(SUCCEED)

        /* Check for packing unmodifiable datatype */
        if(H5T_STATE_TRANSIENT != dt->shared->state)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "datatype is read-only")

        if(dt->shared->parent) {
            if (H5T_pack(dt->shared->parent) < 0)
                HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINIT, FAIL, "unable to pack parent of datatype")

            /* Adjust size of datatype appropriately */
            if(dt->shared->type == H5T_ARRAY)
                dt->shared->size = dt->shared->parent->shared->size * dt->shared->u.array.nelem;
            else if(dt->shared->type != H5T_VLEN)
                dt->shared->size = dt->shared->parent->shared->size;
        } /* end if */
        else if(dt->shared->type == H5T_COMPOUND) {
            size_t	offset;                 /* Offset of member */
            unsigned i;                     /* Local index variable */

            /* Recursively pack the members */
            for(i = 0; i < dt->shared->u.compnd.nmembs; i++) {
                if(H5T_pack(dt->shared->u.compnd.memb[i].type) < 0)
                    HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINIT, FAIL, "unable to pack part of a compound datatype")

                /* Update the member size */
                dt->shared->u.compnd.memb[i].size = (dt->shared->u.compnd.memb[i].type)->shared->size;
            } /* end for */

            /* Remove padding between members */
            if(H5T_sort_value(dt, NULL) < 0)
		HGOTO_ERROR(H5E_INTERNAL, H5E_CANTCOMPARE, FAIL, "value sort failed")
            for(i = 0, offset = 0; i < dt->shared->u.compnd.nmembs; i++) {
                dt->shared->u.compnd.memb[i].offset = offset;
                offset += dt->shared->u.compnd.memb[i].size;
            }

            /* Change total size */
            dt->shared->size = MAX(1, offset);

            /* Mark the type as packed now */
            dt->shared->u.compnd.packed = TRUE;
        } /* end if */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5T_pack() */


/*-------------------------------------------------------------------------
 * Function:	H5T_is_packed
 *
 * Purpose:	Checks whether a datatype which is compound (or has compound
 *              components) is packed.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Thursday, September 11, 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5T_is_packed(const H5T_t *dt)
{
    htri_t      ret_value = TRUE;       /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5T_is_packed)

    HDassert(dt);

    /* Go up the chain as far as possible */
    while(dt->shared->parent)
        dt = dt->shared->parent;

    /* If this is a compound datatype, check if it is packed */
    if(dt->shared->type == H5T_COMPOUND) {
        H5T_compnd_t *compnd = &(dt->shared->u.compnd); /* Convenience pointer to compound info */
        ret_value = (htri_t)(compnd->packed && compnd->nmembs > 0
                && compnd->memb[compnd->nmembs - 1].offset
                + compnd->memb[compnd->nmembs - 1].size
                == dt->shared->size);
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5T_is_packed() */

