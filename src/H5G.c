/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:	H5G.c
 *		Jul 18 1997
 *		Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:	Symbol table functions.	 The functions that begin with
 *		`H5G_stab_' don't understand the naming system; they operate
 * 		on a single symbol table at a time.
 *
 *		The functions that begin with `H5G_node_' operate on the leaf
 *		nodes of a symbol table B-tree.  They should be defined in
 *		the H5Gnode.c file.
 *
 *		The remaining functions know how to traverse the group
 *		directed graph.
 *
 * Names:	Object names are a slash-separated list of components.  If
 *		the name begins with a slash then it's absolute, otherwise
 *		it's relative ("/foo/bar" is absolute while "foo/bar" is
 *		relative).  Multiple consecutive slashes are treated as
 *		single slashes and trailing slashes are ignored.  The special
 *		case `/' is the root group.  Every file has a root group.
 *
 *		API functions that look up names take a location ID and a
 *		name.  The location ID can be a file ID or a group ID and the
 *		name can be relative or absolute.
 *
 *              +--------------+----------- +--------------------------------+
 * 		| Location ID  | Name       | Meaning                        |
 *              +--------------+------------+--------------------------------+
 * 		| File ID      | "/foo/bar" | Find `foo' within `bar' within |
 *		|              |            | the root group of the specified|
 *		|              |            | file.                          |
 *              +--------------+------------+--------------------------------+
 * 		| File ID      | "foo/bar"  | Find `foo' within `bar' within |
 *		|              |            | the root group of the specified|
 *		|              |            | file.                          |
 *              +--------------+------------+--------------------------------+
 * 		| File ID      | "/"        | The root group of the specified|
 *		|              |            | file.                          |
 *              +--------------+------------+--------------------------------+
 * 		| File ID      | "."        | The root group of the specified|
 *		|              |            | the specified file.            |
 *              +--------------+------------+--------------------------------+
 * 		| Group ID     | "/foo/bar" | Find `foo' within `bar' within |
 *		|              |            | the root group of the file     |
 *		|              |            | containing the specified group.|
 *              +--------------+------------+--------------------------------+
 * 		| Group ID     | "foo/bar"  | File `foo' within `bar' within |
 *		|              |            | the specified group.           |
 *              +--------------+------------+--------------------------------+
 * 		| Group ID     | "/"        | The root group of the file     |
 *		|              |            | containing the specified group.|
 *              +--------------+------------+--------------------------------+
 * 		| Group ID     | "."        | The specified group.           |
 *              +--------------+------------+--------------------------------+
 *
 *
 *-------------------------------------------------------------------------
 */

#define H5F_PACKAGE		/*suppress error about including H5Fpkg	  */
#define H5G_PACKAGE		/*suppress error about including H5Gpkg   */

/* Interface initialization */
#define H5_INTERFACE_INIT_FUNC	H5G_init_interface

/* Packages needed by this file... */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fpkg.h"		/* File access				*/
#include "H5FLprivate.h"	/* Free Lists                           */
#include "H5Gpkg.h"		/* Groups		  		*/
#include "H5HLprivate.h"	/* Local Heaps				*/
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5Lprivate.h"         /* Links			  	*/
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5Pprivate.h"         /* Property lists                       */

/* Local macros */
#define H5G_RESERVED_ATOMS	0

/* Local typedefs */

/* User data for path traversal routine for "insertion file" routine */
typedef struct {
    H5G_loc_t *loc;         /* Pointer to the location for insertion */
} H5G_trav_ins_t;

/* Package variables */

/* Local variables */

/* Declare a free list to manage the H5G_t struct */
H5FL_DEFINE(H5G_t);
H5FL_DEFINE(H5G_shared_t);

/* Private prototypes */
static H5G_t *H5G_create(H5F_t *file, hid_t dxpl_id, hid_t gcpl_id, hid_t gapl_id);
static herr_t H5G_open_oid(H5G_t *grp, hid_t dxpl_id);
static herr_t H5G_insertion_loc_cb(H5G_loc_t *grp_loc/*in*/, const char *name,
    const H5O_link_t *lnk, H5G_loc_t *obj_loc, void *_udata/*in,out*/,
    H5G_own_loc_t *own_loc/*out*/);


/*-------------------------------------------------------------------------
 * Function:	H5Gcreate
 *
 * Purpose:	Creates a new group relative to LOC_ID and gives it the
 *		specified NAME.  The group is opened for write access
 *		and it's object ID is returned.
 *
 *		The optional SIZE_HINT specifies how much file space to
 *		reserve to store the names that will appear in this
 *		group. If a non-positive value is supplied for the SIZE_HINT
 *		then a default size is chosen.
 *
 * See also:	H5Gset(), H5Gpush(), H5Gpop()
 *
 * Errors:
 *
 * Return:	Success:	The object ID of a new, empty group open for
 *				writing.  Call H5Gclose() when finished with
 *				the group.
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, September 24, 1997
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Gcreate(hid_t loc_id, const char *name, size_t size_hint)
{
    H5G_loc_t	    loc;
    H5G_loc_t	    grp_loc;
    H5G_t	   *grp = NULL;
    H5G_loc_t       insertion_loc;      /* Loc of group in which to create object */
    H5G_name_t      insert_path;        /* Path of group in which to create object */
    H5O_loc_t       insert_oloc;        /* oloc of group in which to create object */
    hbool_t         insert_loc_valid = FALSE;  /* Is insertion_loc valid? */
    H5F_t          *file;               /* File the group will be inserted into */
    hid_t           tmp_gcpl = (-1);    /* Temporary group creation property list */
    hid_t	    grp_id = (-1);      /* ID of group being created */
    hid_t	    ret_value;

    FUNC_ENTER_API(H5Gcreate, FAIL)
    H5TRACE3("i","isz",loc_id,name,size_hint);

    /* Check arguments */
    if(H5G_loc(loc_id, &loc) < 0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(!name || !*name)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name given")

    /* Check if we need to create a non-standard GCPL */
    if(size_hint > 0) {
        H5P_genplist_t  *gc_plist;  /* Property list created */
        H5O_ginfo_t     ginfo;          /* Group info property */

        /* Get the default property list */
        if(NULL == (gc_plist = H5I_object(H5P_GROUP_CREATE_DEFAULT)))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a property list")

        /* Make a copy of the default property list */
        if((tmp_gcpl = H5P_copy_plist(gc_plist)) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTGET, FAIL, "unable to copy the creation property list")

        /* Get the copy of the property list */
        if(NULL == (gc_plist = H5I_object(H5P_GROUP_CREATE_DEFAULT)))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a property list")

        /* Get the group info property */
        if(H5P_get(gc_plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

        /* Set the non-default local heap size hint */
        ginfo.lheap_size_hint = size_hint;
        if(H5P_set(gc_plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set group info")
    } /* end if */
    else
        tmp_gcpl = H5P_GROUP_CREATE_DEFAULT;

    /* What file is the group being added to?  This may not be the same file
     * that loc_id is in if mounting is being used. */
    insertion_loc.path = &insert_path;
    insertion_loc.oloc = &insert_oloc;
    H5G_loc_reset(&insertion_loc);
    if(H5G_insertion_loc(&loc, name, &insertion_loc, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to locate insertion point")
    insert_loc_valid=TRUE;
    file = insertion_loc.oloc->file;

    /* Create the group */
    if(NULL == (grp = H5G_create(file, H5AC_dxpl_id, tmp_gcpl, H5P_DEFAULT)))
        HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to create group")

    /* Get an ID for the newly created group */
    if((grp_id = H5I_register(H5I_GROUP, grp)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register group")

    /* Get the new group's location */
    if(H5G_loc(grp_id, &grp_loc) <0)
        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "unable to get location for new group")

    /* Link the new group to its parent group */
    if(H5L_link(&loc, name, &grp_loc, H5P_DEFAULT, H5P_DEFAULT, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_LINK, H5E_CANTINIT, FAIL, "unable to create link to group")

    /* Set the return value */
    ret_value = grp_id;

done:
    if(tmp_gcpl > 0 && tmp_gcpl != H5P_GROUP_CREATE_DEFAULT)
        if(H5I_dec_ref(tmp_gcpl) < 0)
            HDONE_ERROR(H5E_SYM, H5E_CLOSEERROR, FAIL, "unable to release property list")

    if(insert_loc_valid) {
        if(H5G_loc_free(&insertion_loc) < 0)
            HDONE_ERROR(H5E_OHDR, H5E_CANTRELEASE, FAIL, "unable to free location")
    }
    if(ret_value < 0) {
        if(grp_id >= 0)
            H5I_dec_ref(grp_id);
        else if(grp!=NULL)
            H5G_close(grp);
    } /* end if */

    FUNC_LEAVE_API(ret_value)
} /* end H5Gcreate() */


/*-------------------------------------------------------------------------
 * Function:	H5Gcreate_expand
 *
 * Purpose:	Creates a new group relative to LOC_ID, giving it the
 *              specified creation property list GCPL_ID and access
 *              property list GAPL_ID.
 *
 *              The resulting ID should be linked into the file with
 *              H5Llink or it will be deleted when closed.
 *
 *              Given the default setting, H5Gcreate_expand() followed by
 *              H5Llink() will have the same function as H5Gcreate().
 *
 * Usage:       H5Gcreate_expand(loc_id, char *name, gcpl_id, gapl_id)
 *              hid_t loc_id;	  IN: File or group identifier
 *              const char *name; IN: Absolute or relative name of the new group
 *              hid_t gcpl_id;	  IN: Property list for group creation
 *              hid_t gapl_id;	  IN: Property list for group access
 *
 * Example:	To create missing groups "A" and "B01" along the given path "/A/B01/grp"
 *              hid_t create_id = H5Pcreate(H5P_GROUP_CREATE);
 *              int   status = H5Pset_create_intermediate_group(create_id, TRUE);
 *              hid_t gid = H5Gcreate_expand(file_id, "/A/B01/grp", create_id, H5P_DEFAULT);
 *
 * See also:	H5Gcreate(), H5Dcreate_expand(), H5Pset_create_intermediate_group()
 *
 * Errors:
 *
 * Return:	Success:	The object ID of a new, empty group open for
 *				writing.  Call H5Gclose() when finished with
 *				the group.
 *
 *		Failure:	FAIL
 *
 * Programmer:  Peter Cao
 *	        May 08, 2005
 *-------------------------------------------------------------------------
 */
hid_t
H5Gcreate_expand(hid_t loc_id, hid_t gcpl_id, hid_t gapl_id)
{
    H5G_loc_t	    loc;
    H5G_t	   *grp = NULL;
    hid_t	    ret_value;

    FUNC_ENTER_API(H5Gcreate_expand, FAIL)
    H5TRACE3("i","iii",loc_id,gcpl_id,gapl_id);

    /* Check arguments */
    if(H5G_loc(loc_id, &loc) < 0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")

    /* Check group creation property list */
    if(H5P_DEFAULT == gcpl_id)
        gcpl_id = H5P_GROUP_CREATE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(gcpl_id, H5P_GROUP_CREATE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not group create property list")

    /* Check the group access property list */
    if(H5P_DEFAULT == gapl_id)
        gapl_id = H5P_GROUP_ACCESS_DEFAULT;
    else
        if(TRUE != H5P_isa_class(gapl_id, H5P_GROUP_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not group access property list")

    /* Create the new group & get its ID */
    if(NULL == (grp = H5G_create(loc.oloc->file, H5AC_dxpl_id, gcpl_id, gapl_id)))
        HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to create group")
    if((ret_value = H5I_register(H5I_GROUP, grp)) < 0)
	HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register group")

done:
    if(ret_value < 0) {
        if(grp!=NULL)
            H5G_close(grp);
    } /* end if */

    FUNC_LEAVE_API(ret_value)
} /* end H5Gcreate_expand() */


/*-------------------------------------------------------------------------
 * Function:	H5Gopen
 *
 * Purpose:	Opens an existing group for modification.  When finished,
 *		call H5Gclose() to close it and release resources.
 *
 * Errors:
 *
 * Return:	Success:	Object ID of the group.
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, December 31, 1997
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Gopen(hid_t loc_id, const char *name)
{
    H5G_t       *grp = NULL;
    H5G_loc_t	loc;
    H5G_loc_t   grp_loc;                /* Location used to open group */
    H5G_name_t  grp_path;            	/* Opened object group hier. path */
    H5O_loc_t   grp_oloc;            	/* Opened object object location */
    H5O_type_t  obj_type;               /* Type of object at location */
    hbool_t     loc_found = FALSE;      /* Location at 'name' found */
    hid_t       ret_value;              /* Return value */

    FUNC_ENTER_API(H5Gopen, FAIL)
    H5TRACE2("i","is",loc_id,name);

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name")

    /* Set up opened group location to fill in */
    grp_loc.oloc = &grp_oloc;
    grp_loc.path = &grp_path;
    H5G_loc_reset(&grp_loc);

    /* Find the group object */
    if(H5G_loc_find(&loc, name, &grp_loc/*out*/, H5P_DEFAULT, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "group not found")
    loc_found = TRUE;

    /* Check that the object found is the correct type */
    if(H5O_obj_type(&grp_oloc, &obj_type, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTGET, FAIL, "can't get object type")
    if(obj_type != H5O_TYPE_GROUP)
        HGOTO_ERROR(H5E_SYM, H5E_BADTYPE, FAIL, "not a group")

    /* Open the group */
    if((grp = H5G_open(&grp_loc, H5AC_dxpl_id)) == NULL)
        HGOTO_ERROR(H5E_SYM, H5E_CANTOPENOBJ, FAIL, "unable to open group")

    /* Register an atom for the group */
    if((ret_value = H5I_register(H5I_GROUP, grp)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register group")

done:
    if(ret_value < 0) {
        if(grp != NULL)
            H5G_close(grp);
        else {
            if(loc_found) {
                if(H5G_loc_free(&grp_loc) < 0)
                    HDONE_ERROR(H5E_SYM, H5E_CANTRELEASE, FAIL, "can't free location")
            }
        } /* end else */
    } /* end if */

    FUNC_LEAVE_API(ret_value)
} /* end H5Gopen() */


/*-------------------------------------------------------------------------
 * Function:	H5Gopen_expand
 *
 * Purpose:	Opens an existing group for modification.  When finished,
 *		call H5Gclose() to close it and release resources.
 *
 *              This function allows the user the pass in a Group Access
 *              Property List, which H5Gopen() does not.
 *
 * Return:	Success:	Object ID of the group.
 *		Failure:	FAIL
 *
 * Programmer:	James Laird
 *		Thursday, July 27, 2006
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Gopen_expand(hid_t loc_id, const char *name, hid_t gapl_id)
{
    H5G_t       *grp = NULL;
    H5G_loc_t	loc;
    H5G_loc_t   grp_loc;                /* Location used to open group */
    H5G_name_t  grp_path;            	/* Opened object group hier. path */
    H5O_loc_t   grp_oloc;            	/* Opened object object location */
    H5O_type_t  obj_type;               /* Type of object at location */
    hbool_t     loc_found = FALSE;      /* Location at 'name' found */
    hid_t       ret_value;              /* Return value */

    FUNC_ENTER_API(H5Gopen_expand, FAIL);
    H5TRACE3("i","isi",loc_id,name,gapl_id);

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name")

    /* Check the group access property list */
    if(H5P_DEFAULT == gapl_id)
        gapl_id = H5P_GROUP_ACCESS_DEFAULT;
    else
        if(TRUE != H5P_isa_class(gapl_id, H5P_GROUP_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not group access property list")

    /* Set up opened group location to fill in */
    grp_loc.oloc = &grp_oloc;
    grp_loc.path = &grp_path;
    H5G_loc_reset(&grp_loc);

    /* Find the group object using the gapl passed in */
    if(H5G_loc_find(&loc, name, &grp_loc/*out*/, gapl_id, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "group not found")
    loc_found = TRUE;

    /* Check that the object found is the correct type */
    if(H5O_obj_type(&grp_oloc, &obj_type, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTGET, FAIL, "can't get object type")
    if(obj_type != H5O_TYPE_GROUP)
        HGOTO_ERROR(H5E_SYM, H5E_BADTYPE, FAIL, "not a group")

    /* Open the group */
    if((grp = H5G_open(&grp_loc, H5AC_dxpl_id)) == NULL)
        HGOTO_ERROR(H5E_SYM, H5E_CANTOPENOBJ, FAIL, "unable to open group")

    /* Register an atom for the group */
    if((ret_value = H5I_register(H5I_GROUP, grp)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register group")

done:
    if(ret_value < 0) {
        if(grp != NULL)
            H5G_close(grp);
        else {
            if(loc_found) {
                if(H5G_loc_free(&grp_loc) < 0)
                    HDONE_ERROR(H5E_SYM, H5E_CANTRELEASE, FAIL, "can't free location")
            }
        } /* end else */
    } /* end if */

    FUNC_LEAVE_API(ret_value)
} /* end H5Gopen_expand() */


/*-------------------------------------------------------------------------
 * Function:	H5Gget_create_plist
 *
 * Purpose:	Returns a copy of the group creation property list.
 *
 * Return:	Success:	ID for a copy of the group creation
 *				property list.  The property list ID should be
 *				released by calling H5Pclose().
 *
 *		Failure:	FAIL
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, October 25, 2005
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Gget_create_plist(hid_t group_id)
{
    htri_t	        ginfo_exists = 0;
    htri_t	        linfo_exists = 0;
    H5G_t		*grp = NULL;
    H5P_genplist_t      *gcpl_plist;
    H5P_genplist_t      *new_plist;
    hid_t		new_gcpl_id = FAIL;
    hid_t		ret_value = FAIL;

    FUNC_ENTER_API(H5Gget_create_plist, FAIL)
    H5TRACE1("i","i",group_id);

    /* Check args */
    if(NULL == (grp = H5I_object_verify(group_id, H5I_GROUP)))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a group")

    /* Copy the default group creation property list */
    if(NULL == (gcpl_plist = H5I_object(H5P_LST_GROUP_CREATE_g)))
         HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "can't get default group creation property list")
    if((new_gcpl_id = H5P_copy_plist(gcpl_plist)) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTGET, FAIL, "unable to copy the creation property list")
    if(NULL == (new_plist = H5I_object(new_gcpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "can't get property list")

    /* Retrieve any object creation properties */
    if(H5O_get_create_plist(&grp->oloc, H5AC_ind_dxpl_id, new_plist) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTGET, FAIL, "can't get object creation info")

    /* Check for the group having a group info message */
    if((ginfo_exists = H5O_msg_exists(&(grp->oloc), H5O_GINFO_ID, 0, H5AC_ind_dxpl_id)) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to read object header")
    if(ginfo_exists) {
        H5O_ginfo_t ginfo;		/* Group info message            */

        /* Read the group info */
        if(NULL == H5O_msg_read(&(grp->oloc), H5O_GINFO_ID, 0, &ginfo, H5AC_ind_dxpl_id))
            HGOTO_ERROR(H5E_SYM, H5E_BADMESG, FAIL, "can't get group info")

        /* Set the group info for the property list */
        if(H5P_set(new_plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set group info")
    } /* end if */

    /* Check for the group having a link info message */
    if((linfo_exists = H5O_msg_exists(&(grp->oloc), H5O_LINFO_ID, 0, H5AC_ind_dxpl_id)) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to read object header")
    if(linfo_exists) {
        H5O_linfo_t linfo;		/* Link info message            */

        /* Read the link info */
        if(NULL == H5O_msg_read(&(grp->oloc), H5O_LINFO_ID, 0, &linfo, H5AC_ind_dxpl_id))
            HGOTO_ERROR(H5E_SYM, H5E_BADMESG, FAIL, "can't get link info")

        /* Set the link info for the property list */
        if(H5P_set(new_plist, H5G_CRT_LINK_INFO_NAME, &linfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set link info")
    } /* end if */

    /* Set the return value */
    ret_value = new_gcpl_id;

done:
    if(ret_value < 0) {
        if(new_gcpl_id > 0)
            (void)H5I_dec_ref(new_gcpl_id);
    } /* end if */

    FUNC_LEAVE_API(ret_value)
} /* end H5Gget_create_plist() */


/*-------------------------------------------------------------------------
 * Function:	H5Gget_info
 *
 * Purpose:	Retrieve information about a group.
 *
 * Return:	Success:	Non-negative
 *		Failure:	Negative
 *
 * Programmer:	Quincey Koziol
 *		November 27 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Gget_info(hid_t loc_id, const char *name, H5G_info_t *grp_info, hid_t lapl_id)
{
    H5G_loc_t	loc;                    /* Location of group */
    H5G_loc_t   grp_loc;                /* Location used to open group */
    H5G_name_t  grp_path;            	/* Opened object group hier. path */
    H5O_loc_t   grp_oloc;            	/* Opened object object location */
    hbool_t     loc_found = FALSE;      /* Location at 'name' found */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(H5Gget_info, FAIL)

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name")
    if(!grp_info)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no info struct")
    if(H5P_DEFAULT == lapl_id)
        lapl_id = H5P_LINK_ACCESS_DEFAULT;
    else
        if(TRUE != H5P_isa_class(lapl_id, H5P_LINK_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not link access property list ID")

    /* Set up opened group location to fill in */
    grp_loc.oloc = &grp_oloc;
    grp_loc.path = &grp_path;
    H5G_loc_reset(&grp_loc);

    /* Find the group object */
    if(H5G_loc_find(&loc, name, &grp_loc/*out*/, lapl_id, H5AC_ind_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "group not found")
    loc_found = TRUE;

    /* Retrieve the group's information */
    if(H5G_obj_info(grp_loc.oloc, grp_info/*out*/, H5AC_ind_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTGET, FAIL, "can't retrieve group info")

done:
    if(loc_found && H5G_loc_free(&grp_loc) < 0)
        HDONE_ERROR(H5E_SYM, H5E_CANTRELEASE, FAIL, "can't free location")

    FUNC_LEAVE_API(ret_value)
} /* end H5Gget_info() */


/*-------------------------------------------------------------------------
 * Function:	H5Gget_info_by_idx
 *
 * Purpose:	Retrieve information about a group, according to the order
 *              of an index.
 *
 * Return:	Success:	Non-negative
 *		Failure:	Negative
 *
 * Programmer:	Quincey Koziol
 *		November 27 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Gget_info_by_idx(hid_t loc_id, const char *group_name, H5_index_t idx_type,
    H5_iter_order_t order, hsize_t n, H5G_info_t *grp_info, hid_t lapl_id)
{
    H5G_loc_t	loc;                    /* Location of group */
    H5G_loc_t   grp_loc;                /* Location used to open group */
    H5G_name_t  grp_path;            	/* Opened object group hier. path */
    H5O_loc_t   grp_oloc;            	/* Opened object object location */
    hbool_t     loc_found = FALSE;      /* Entry at 'name' found */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(H5Gget_info_by_idx, FAIL)

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(!group_name || !*group_name)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name specified")
    if(idx_type <= H5_INDEX_UNKNOWN || idx_type >= H5_INDEX_N)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid index type specified")
    if(order <= H5_ITER_UNKNOWN || order >= H5_ITER_N)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid iteration order specified")
    if(!grp_info)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no info struct")
    if(H5P_DEFAULT == lapl_id)
        lapl_id = H5P_LINK_ACCESS_DEFAULT;
    else
        if(TRUE != H5P_isa_class(lapl_id, H5P_LINK_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not link access property list ID")

    /* Set up opened group location to fill in */
    grp_loc.oloc = &grp_oloc;
    grp_loc.path = &grp_path;
    H5G_loc_reset(&grp_loc);

    /* Find the object's location, according to the order in the index */
    if(H5G_loc_find_by_idx(&loc, group_name, idx_type, order, n, &grp_loc/*out*/, lapl_id, H5AC_ind_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "group not found")
    loc_found = TRUE;

    /* Retrieve the group's information */
    if(H5G_obj_info(grp_loc.oloc, grp_info/*out*/, H5AC_ind_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTGET, FAIL, "can't retrieve group info")

done:
    /* Release the object location */
    if(loc_found && H5G_loc_free(&grp_loc) < 0)
        HDONE_ERROR(H5E_SYM, H5E_CANTRELEASE, FAIL, "can't free location")

    FUNC_LEAVE_API(ret_value)
} /* end H5Gget_info_by_idx() */


/*-------------------------------------------------------------------------
 * Function:	H5Gclose
 *
 * Purpose:	Closes the specified group.  The group ID will no longer be
 *		valid for accessing the group.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Wednesday, December 31, 1997
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Gclose(hid_t group_id)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(H5Gclose, FAIL);
    H5TRACE1("e","i",group_id);

    /* Check args */
    if(NULL == H5I_object_verify(group_id,H5I_GROUP))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a group")

    /*
     * Decrement the counter on the group atom.	 It will be freed if the count
     * reaches zero.
     */
    if(H5I_dec_ref(group_id) < 0)
    	HGOTO_ERROR(H5E_SYM, H5E_CANTRELEASE, FAIL, "unable to close group")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Gclose() */

/*
 *-------------------------------------------------------------------------
 *-------------------------------------------------------------------------
 *   N O   A P I   F U N C T I O N S   B E Y O N D   T H I S   P O I N T
 *-------------------------------------------------------------------------
 *-------------------------------------------------------------------------
 */


/*-------------------------------------------------------------------------
 * Function:	H5G_init
 *
 * Purpose:	Initialize the interface from some other package.
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *              Saturday, November 11, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_init(void)
{
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI(H5G_init, FAIL)
    /* FUNC_ENTER() does all the work */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_init() */


/*-------------------------------------------------------------------------
 * Function:	H5G_init_interface
 *
 * Purpose:	Initializes the H5G interface.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 * Notes:       The group creation properties are registered in the property
 *              list interface initialization routine (H5P_init_interface)
 *              so that the file creation property class can inherit from it
 *              correctly. (Which allows the file creation property list to
 *              control the group creation properties of the root group of
 *              a file) QAK - 24/10/2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_init_interface(void)
{
    herr_t          ret_value = SUCCEED;  /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5G_init_interface)

    /* Initialize the atom group for the group IDs */
    if(H5I_register_type(H5I_GROUP, (size_t)H5I_GROUPID_HASHSIZE, H5G_RESERVED_ATOMS, (H5I_free_t)H5G_close) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to initialize interface")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_init_interface() */


/*-------------------------------------------------------------------------
 * Function:	H5G_term_interface
 *
 * Purpose:	Terminates the H5G interface
 *
 * Return:	Success:	Positive if anything is done that might
 *				affect other interfaces; zero otherwise.
 *
 * 		Failure:	Negative.
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 *-------------------------------------------------------------------------
 */
int
H5G_term_interface(void)
{
    int	n = 0;

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_term_interface)

    if (H5_interface_initialize_g) {
	if ((n = H5I_nmembers(H5I_GROUP))) {
	    H5I_clear_type(H5I_GROUP, FALSE);
	} else {
	    /* Destroy the group object id group */
	    H5I_dec_type_ref(H5I_GROUP);

            /* Free the global component buffer */
            H5G_traverse_term_interface();

	    /* Mark closed */
	    H5_interface_initialize_g = 0;
	    n = 1; /*H5I*/
	} /* end else */
    } /* end if */

    FUNC_LEAVE_NOAPI(n)
} /* end H5G_term_interface() */


/*-------------------------------------------------------------------------
 * Function:	H5G_component
 *
 * Purpose:	Returns the pointer to the first component of the
 *		specified name by skipping leading slashes.  Returns
 *		the size in characters of the component through SIZE_P not
 *		counting leading slashes or the null terminator.
 *
 * Return:	Success:	Ptr into NAME.
 *
 *		Failure:	Ptr to the null terminator of NAME.
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug 11 1997
 *
 *-------------------------------------------------------------------------
 */
const char *
H5G_component(const char *name, size_t *size_p)
{
    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_component)

    assert(name);

    while ('/' == *name)
        name++;
    if (size_p)
        *size_p = HDstrcspn(name, "/");

    FUNC_LEAVE_NOAPI(name)
} /* end H5G_component() */


/*-------------------------------------------------------------------------
 * Function:	H5G_normalize
 *
 * Purpose:	Returns a pointer to a new string which has duplicate and
 *              trailing slashes removed from it.
 *
 * Return:	Success:	Ptr to normalized name.
 *		Failure:	NULL
 *
 * Programmer:	Quincey Koziol
 *              Saturday, August 16, 2003
 *
 *-------------------------------------------------------------------------
 */
char *
H5G_normalize(const char *name)
{
    char *norm;         /* Pointer to the normalized string */
    size_t	s,d;    /* Positions within the strings */
    unsigned    last_slash;     /* Flag to indicate last character was a slash */
    char *ret_value;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5G_normalize);

    /* Sanity check */
    assert(name);

    /* Duplicate the name, to return */
    if (NULL==(norm=H5MM_strdup(name)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed for normalized string");

    /* Walk through the characters, omitting duplicated '/'s */
    s=d=0;
    last_slash=0;
    while(name[s]!='\0') {
        if(name[s]=='/')
            if(last_slash)
                ;
            else {
                norm[d++]=name[s];
                last_slash=1;
            } /* end else */
        else {
            norm[d++]=name[s];
            last_slash=0;
        } /* end else */
        s++;
    } /* end while */

    /* Terminate normalized string */
    norm[d]='\0';

    /* Check for final '/' on normalized name & eliminate it */
    if(d>1 && last_slash)
        norm[d-1]='\0';

    /* Set return value */
    ret_value=norm;

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5G_normalize() */


/*-------------------------------------------------------------------------
 * Function:	H5G_mkroot
 *
 * Purpose:	Creates a root group in an empty file and opens it.  If a
 *		root group is already open then this function immediately
 *		returns.   If ENT is non-null then it's the symbol table
 *		entry for an existing group which will be opened as the root
 *		group.  Otherwise a new root group is created and then
 *		opened.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug 11 1997
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_mkroot(H5F_t *f, hid_t dxpl_id, H5G_loc_t *loc)
{
    H5O_loc_t	new_root_oloc;		/* New root object location */
    H5G_name_t	new_root_path;		/* New root path */
    H5G_loc_t   new_root_loc;           /* New root location information */
    H5G_loc_t   root_loc;               /* Root location information */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(H5G_mkroot, FAIL)

    /* check args */
    HDassert(f);
    if(f->shared->root_grp)
        HGOTO_DONE(SUCCEED)

    /* Create information needed for group nodes */
    if(H5G_node_init(f) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to create group node info")

    /*
     * If there is no root object then create one. The root group always starts
     * with a hard link count of one since it's pointed to by the boot block.
     */
    if (loc == NULL) {
        H5P_genplist_t *fc_plist;       /* File creation property list */
        H5O_ginfo_t     ginfo;          /* Group info parameters */
        H5O_linfo_t     linfo;          /* Link info parameters */

        /* Get the file creation property list */
        /* (Which is a sub-class of the group creation property class) */
        if(NULL == (fc_plist = H5I_object(f->shared->fcpl_id)))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a property list")

        /* Get the group info property */
        if(H5P_get(fc_plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

        /* Get the link info property */
        if(H5P_get(fc_plist, H5G_CRT_LINK_INFO_NAME, &linfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get link info")

        /* Set up group location for root group */
        new_root_loc.oloc = &new_root_oloc;
        new_root_loc.path = &new_root_path;
        H5G_loc_reset(&new_root_loc);
        loc = &new_root_loc;

	if(H5G_obj_create(f, dxpl_id, &ginfo, &linfo, f->shared->fcpl_id, loc->oloc/*out*/) < 0)
	    HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to create group entry")
	if(1 != H5O_link(loc->oloc, 1, dxpl_id))
	    HGOTO_ERROR(H5E_SYM, H5E_LINKCOUNT, FAIL, "internal error (wrong link count)")
    } else {
	/*
	 * Open the root object as a group.
	 */
	if(H5O_open(loc->oloc) < 0)
	    HGOTO_ERROR(H5E_SYM, H5E_CANTOPENOBJ, FAIL, "unable to open root group")
    } /* end else */

    /* Create the path names for the root group's entry */
    H5G_name_init(loc->path, "/");

    /*
     * Create the group pointer.  Also decrement the open object count so we
     * don't count the root group as an open object.  The root group will
     * never be closed.
     */
    if(NULL == (f->shared->root_grp = H5FL_CALLOC(H5G_t)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed")
    if(NULL == (f->shared->root_grp->shared = H5FL_CALLOC(H5G_shared_t))) {
        H5FL_FREE(H5G_t, f->shared->root_grp);
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed")
    } /* end if */

    /* Shallow copy (take ownership) of the group object info */
    root_loc.oloc = &(f->shared->root_grp->oloc);
    root_loc.path = &(f->shared->root_grp->path);
    if(H5G_loc_copy(&root_loc, loc, H5_COPY_SHALLOW) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTCOPY, FAIL, "can't copy group object location")

    f->shared->root_grp->shared->fo_count = 1;
    HDassert(1 == f->nopen_objs);
    f->nopen_objs = 0;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_mkroot() */


/*-------------------------------------------------------------------------
 * Function:	H5G_create
 *
 * Purpose:	Creates a new empty group with the specified name. The name
 *		is either an absolute name or is relative to LOC.
 *
 * Return:	Success:	A handle for the group.	 The group is opened
 *				and should eventually be close by calling
 *				H5G_close().
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug 11 1997
 *
 *-------------------------------------------------------------------------
 */
static H5G_t *
H5G_create(H5F_t *file, hid_t dxpl_id, hid_t gcpl_id, hid_t UNUSED gapl_id)
{
    H5G_t	*grp = NULL;	/*new group			*/
    H5P_genplist_t  *gc_plist;  /* Property list created */
    H5O_ginfo_t ginfo;          /* Group info */
    H5O_linfo_t linfo;          /* Link info */
    unsigned    oloc_init = 0;  /* Flag to indicate that the group object location was created successfully */
    H5G_t	*ret_value;	/* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5G_create)

    /* check args */
    HDassert(file);
    HDassert(gcpl_id != H5P_DEFAULT);
#ifdef LATER
    HDassert(gapl_id != H5P_DEFAULT);
#endif /* LATER */

    /* create an open group */
    if(NULL == (grp = H5FL_CALLOC(H5G_t)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")
    if(NULL == (grp->shared = H5FL_CALLOC(H5G_shared_t)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")

    /* Get the property list */
    if(NULL == (gc_plist = H5I_object(gcpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a property list")

    /* Get the group info property */
    if(H5P_get(gc_plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, NULL, "can't get group info")

    /* Get the link info property */
    if(H5P_get(gc_plist, H5G_CRT_LINK_INFO_NAME, &linfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, NULL, "can't get group info")

    /* Create the group object header */
    if(H5G_obj_create(file, dxpl_id, &ginfo, &linfo, gcpl_id, &(grp->oloc)/*out*/) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, NULL, "unable to create group object header")
    oloc_init = 1;    /* Indicate that the object location information is valid */

    /* Add group to list of open objects in file */
    if(H5FO_top_incr(grp->oloc.file, grp->oloc.addr) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTINC, NULL, "can't incr object ref. count")
    if(H5FO_insert(grp->oloc.file, grp->oloc.addr, grp->shared, TRUE) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTINSERT, NULL, "can't insert group into list of open objects")

    /* Set the count of times the object is opened */
    grp->shared->fo_count = 1;
    
    /* Set return value */
    ret_value = grp;

done:
    if(ret_value == NULL) {
        /* Check if we need to release the file-oriented symbol table info */
        if(oloc_init) {
            if(H5O_close(&(grp->oloc)) < 0)
                HDONE_ERROR(H5E_SYM, H5E_CLOSEERROR, NULL, "unable to release object header")
            if(H5O_delete(file, dxpl_id, grp->oloc.addr) < 0)
                HDONE_ERROR(H5E_SYM, H5E_CANTDELETE, NULL, "unable to delete object header")
        } /* end if */
        if(grp != NULL) {
            if(grp->shared != NULL)
                H5FL_FREE(H5G_shared_t, grp->shared);
            H5FL_FREE(H5G_t,grp);
        } /* end if */
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_create() */


/*-------------------------------------------------------------------------
 * Function:	H5G_open
 *
 * Purpose:	Opens an existing group.  The group should eventually be
 *		closed by calling H5G_close().
 *
 * Return:	Success:	Ptr to a new group.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 *-------------------------------------------------------------------------
 */
H5G_t *
H5G_open(H5G_loc_t *loc, hid_t dxpl_id)
{
    H5G_t           *grp = NULL;
    H5G_shared_t    *shared_fo = NULL;
    H5G_t           *ret_value = NULL;

    FUNC_ENTER_NOAPI(H5G_open, NULL)

    /* Check args */
    HDassert(loc);

    /* Allocate the group structure */
    if(NULL == (grp = H5FL_CALLOC(H5G_t)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "can't allocate space for group")

    /* Shallow copy (take ownership) of the group location object */
    if(H5O_loc_copy(&(grp->oloc), loc->oloc, H5_COPY_SHALLOW) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTCOPY, NULL, "can't copy object location")
    if(H5G_name_copy(&(grp->path), loc->path, H5_COPY_SHALLOW) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTCOPY, NULL, "can't copy path")

    /* Check if group was already open */
    if((shared_fo = H5FO_opened(grp->oloc.file, grp->oloc.addr)) == NULL) {

        /* Clear any errors from H5FO_opened() */
        H5E_clear_stack(NULL);

        /* Open the group object */
        if(H5G_open_oid(grp, dxpl_id) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, NULL, "not found")

        /* Add group to list of open objects in file */
        if(H5FO_insert(grp->oloc.file, grp->oloc.addr, grp->shared, FALSE) < 0) {
            H5FL_FREE(H5G_shared_t, grp->shared);
            HGOTO_ERROR(H5E_SYM, H5E_CANTINSERT, NULL, "can't insert group into list of open objects")
        } /* end if */

        /* Increment object count for the object in the top file */
        if(H5FO_top_incr(grp->oloc.file, grp->oloc.addr) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTINC, NULL, "can't increment object count")

        /* Set open object count */
        grp->shared->fo_count = 1;
    } /* end if */
    else {
        /* Point to shared group info */
        grp->shared = shared_fo;

        /* Increment shared reference count */
        shared_fo->fo_count++;

        /* Check if the object has been opened through the top file yet */
        if(H5FO_top_count(grp->oloc.file, grp->oloc.addr) == 0) {
            /* Open the object through this top file */
            if(H5O_open(&(grp->oloc)) < 0)
                HGOTO_ERROR(H5E_SYM, H5E_CANTOPENOBJ, NULL, "unable to open object header")
        } /* end if */

        /* Increment object count for the object in the top file */
        if(H5FO_top_incr(grp->oloc.file, grp->oloc.addr) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTINC, NULL, "can't increment object count")
    } /* end else */

    /* Set return value */
    ret_value = grp;

done:
    if (!ret_value && grp)
    {
        H5O_loc_free(&(grp->oloc));
        H5G_name_free(&(grp->path));
        H5FL_FREE(H5G_t,grp);
    }

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_open() */


/*-------------------------------------------------------------------------
 * Function:	H5G_open_oid
 *
 * Purpose:	Opens an existing group.  The group should eventually be
 *		closed by calling H5G_close().
 *
 * Return:	Success:	Ptr to a new group.
 *
 *		Failure:	NULL
 *
 * Programmer:	Quincey Koziol
 *	    Wednesday, March	17, 1999
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_open_oid(H5G_t *grp, hid_t dxpl_id)
{
    hbool_t             obj_opened = FALSE;
    herr_t		ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT(H5G_open_oid)

    /* Check args */
    HDassert(grp);

    /* Allocate the shared information for the group */
    if(NULL == (grp->shared = H5FL_CALLOC(H5G_shared_t)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed")

    /* Grab the object header */
    if(H5O_open(&(grp->oloc)) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTOPENOBJ, FAIL, "unable to open group")
    obj_opened = TRUE;

    /* Check if this object has the right message(s) to be treated as a group */
    if((H5O_msg_exists(&(grp->oloc), H5O_STAB_ID, 0, dxpl_id) <= 0)
            && (H5O_msg_exists(&(grp->oloc), H5O_LINFO_ID, 0, dxpl_id) <= 0))
        HGOTO_ERROR(H5E_SYM, H5E_CANTOPENOBJ, FAIL, "not a group")

done:
    if(ret_value < 0) {
        if(obj_opened)
            H5O_close(&(grp->oloc));
        if(grp->shared)
            H5FL_FREE(H5G_shared_t, grp->shared);
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_open_oid() */


/*-------------------------------------------------------------------------
 * Function:	H5G_close
 *
 * Purpose:	Closes the specified group.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_close(H5G_t *grp)
{
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_NOAPI(H5G_close, FAIL)

    /* Check args */
    HDassert(grp && grp->shared);
    HDassert(grp->shared->fo_count > 0);

    --grp->shared->fo_count;

    if(0 == grp->shared->fo_count) {
        HDassert(grp != H5G_rootof(H5G_fileof(grp)));

        /* Remove the group from the list of opened objects in the file */
        if(H5FO_top_decr(grp->oloc.file, grp->oloc.addr) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTRELEASE, FAIL, "can't decrement count for object")
        if(H5FO_delete(grp->oloc.file, H5AC_dxpl_id, grp->oloc.addr) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTRELEASE, FAIL, "can't remove group from list of open objects")
        if(H5O_close(&(grp->oloc)) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to close")
        H5FL_FREE(H5G_shared_t, grp->shared);
    } else {
        /* Decrement the ref. count for this object in the top file */
        if(H5FO_top_decr(grp->oloc.file, grp->oloc.addr) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTRELEASE, FAIL, "can't decrement count for object")

        /* Check reference count for this object in the top file */
        if(H5FO_top_count(grp->oloc.file, grp->oloc.addr) == 0)
            if(H5O_close(&(grp->oloc)) < 0)
                HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to close")

        /* If this group is a mount point and the mount point is the last open
         * reference to the group, then attempt to close down the file hierarchy
         */
        if(grp->shared->mounted && grp->shared->fo_count == 1) {
            /* Attempt to close down the file hierarchy */
            if(H5F_try_close(grp->oloc.file) < 0)
                HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEFILE, FAIL, "problem attempting file close")
        } /* end if */
    } /* end else */

    if(H5G_name_free(&(grp->path)) < 0) {
        H5FL_FREE(H5G_t,grp);
        HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "can't free group entry name")
    } /* end if */

    H5FL_FREE(H5G_t,grp);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_close() */


/*-------------------------------------------------------------------------
 * Function:    H5G_free
 *
 * Purpose:	Free memory used by an H5G_t struct (and its H5G_shared_t).
 *          Does not close the group or decrement the reference count.
 *          Used to free memory used by the root group.
 *
 * Return:  Success:    Non-negative
 *	        Failure:    Negative
 *
 * Programmer:  James Laird
 *              Tuesday, September 7, 2004
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_free(H5G_t *grp)
{
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_NOAPI(H5G_free, FAIL)

    HDassert(grp && grp->shared);

    H5FL_FREE(H5G_shared_t, grp->shared);
    H5FL_FREE(H5G_t, grp);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_free() */


/*-------------------------------------------------------------------------
 * Function:	H5G_rootof
 *
 * Purpose:	Return a pointer to the root group of the file.  If the file
 *		is part of a virtual file then the root group of the virtual
 *		file is returned.
 *
 * Return:	Success:	Ptr to the root group of the file.  Do not
 *				free the pointer -- it points directly into
 *				the file struct.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Tuesday, October 13, 1998
 *
 *-------------------------------------------------------------------------
 */
H5G_t *
H5G_rootof(H5F_t *f)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_rootof)

    while(f->mtab.parent)
        f = f->mtab.parent;

    FUNC_LEAVE_NOAPI(f->shared->root_grp)
} /* end H5G_rootof() */


/*-------------------------------------------------------------------------
 * Function:	H5G_oloc
 *
 * Purpose:	Returns a pointer to the object location for a group.
 *
 * Return:	Success:	Ptr to group entry
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March 24, 1998
 *
 *-------------------------------------------------------------------------
 */
H5O_loc_t *
H5G_oloc(H5G_t *grp)
{
    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_oloc)

    FUNC_LEAVE_NOAPI(grp ? &(grp->oloc) : NULL)
} /* end H5G_oloc() */


/*-------------------------------------------------------------------------
 * Function:	H5G_nameof
 *
 * Purpose:	Returns a pointer to the hier. name for a group.
 *
 * Return:	Success:	Ptr to hier. name
 *		Failure:	NULL
 *
 * Programmer:	Quincey Koziol
 *              Monday, September 12, 2005
 *
 *-------------------------------------------------------------------------
 */
H5G_name_t *
H5G_nameof(H5G_t *grp)
{
    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_nameof)

    FUNC_LEAVE_NOAPI(grp ? &(grp->path) : NULL)
} /* end H5G_nameof() */


/*-------------------------------------------------------------------------
 * Function:	H5G_fileof
 *
 * Purpose:	Returns the file to which the specified group belongs.
 *
 * Return:	Success:	File pointer.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March 24, 1998
 *
 *-------------------------------------------------------------------------
 */
H5F_t *
H5G_fileof(H5G_t *grp)
{
    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_fileof)

    HDassert(grp);

    FUNC_LEAVE_NOAPI(grp->oloc.file)
} /* end H5G_fileof() */


/*-------------------------------------------------------------------------
 * Function:	H5G_map_obj_type
 *
 * Purpose:	Maps the object type to the older "group" object type
 *
 * Return:	Object type (can't fail)
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, November 21, 2006
 *
 *-------------------------------------------------------------------------
 */
H5G_obj_t
H5G_map_obj_type(H5O_type_t obj_type)
{
    H5G_obj_t ret_value;        /* Return value */

    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_map_obj_type)

    /* Map object type to older "group" object type */
    switch(obj_type) {
        case H5O_TYPE_GROUP:
            ret_value = H5G_GROUP;
            break;

        case H5O_TYPE_DATASET:
            ret_value = H5G_DATASET;
            break;

        case H5O_TYPE_NAMED_DATATYPE:
            ret_value = H5G_TYPE;
            break;

        default:
            ret_value = H5G_UNKNOWN;
    } /* end switch */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_map_obj_type() */


/*-------------------------------------------------------------------------
 * Function:	H5G_insertion_loc_cb
 *
 * Purpose:	Callback for finding insertion location.  This routine sets the
 *              correct information in the location passed in through the udata.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	James Laird
 *              Wednesday, August 16, 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_insertion_loc_cb(H5G_loc_t *grp_loc/*in*/, const char UNUSED *name, const H5O_link_t UNUSED *lnk,
    H5G_loc_t *obj_loc, void *_udata/*in,out*/, H5G_own_loc_t *own_loc/*out*/)
{
    H5G_trav_ins_t *udata = (H5G_trav_ins_t *)_udata;   /* User data passed in */
    herr_t ret_value = SUCCEED;              /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5G_insertion_loc_cb)

    /* Check if the name in this group resolves to a valid location */
    /* (which is not what we want) */
    if(obj_loc != NULL)
        HGOTO_ERROR(H5E_SYM, H5E_EXISTS, FAIL, "name already exists")

    /* Take ownership of the grp_loc */
    if(H5G_loc_copy(udata->loc, grp_loc, H5_COPY_SHALLOW) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTCOPY, FAIL, "couldn't take ownership of group location")
    *own_loc = H5G_OWN_GRP_LOC;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_insertion_loc_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5G_insertion_loc
 *
 * Purpose:	Given a location and name that specifies a not-yet-existing
 *		object return the location of the group into which the object
 *              is about to be inserted.
 *
 * Return:	Success:	H5G_loc_t pointer
 *                              (should be released with H5G_loc_free)
 *
 *		Failure:	NULL
 *
 * Programmer:	James Laird
 *              Wednesday, August 16, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_insertion_loc(H5G_loc_t *src_loc, const char *name,
    H5G_loc_t *insertion_loc/*out*/, hid_t dxpl_id)
{
    H5G_trav_ins_t udata;           /* User data for traversal */
    herr_t         ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_NOAPI(H5G_insertion_loc, FAIL)

    HDassert(src_loc);
    HDassert(insertion_loc);
    HDassert(name && *name);

    /*
     * Look up the name to get the containing group's location and to make
     * sure the name doesn't already exist.
     */
    udata.loc = insertion_loc;
    if(H5G_traverse(src_loc, name, H5G_TARGET_NORMAL, H5G_insertion_loc_cb, &udata, H5P_DEFAULT, dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_EXISTS, FAIL, "name already exists")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_insertion_loc() */


/*-------------------------------------------------------------------------
 * Function:	H5G_free_grp_name
 *
 * Purpose:	Free the 'ID to name' buffers.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: August 22, 2002
 *
 * Comments: Used now only on the root group close, in H5F_close()
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_free_grp_name(H5G_t *grp)
{
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI(H5G_free_grp_name, FAIL)

    /* Check args */
    HDassert(grp && grp->shared);
    HDassert(grp->shared->fo_count > 0);

    /* Free the path */
    H5G_name_free(&(grp->path));

done:
     FUNC_LEAVE_NOAPI(ret_value)
} /* end H5G_free_grp_name() */


/*-------------------------------------------------------------------------
 * Function:	H5G_get_shared_count
 *
 * Purpose:	Queries the group object's "shared count"
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, July	 5, 2005
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_get_shared_count(H5G_t *grp)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_get_shared_count);

    /* Check args */
    HDassert(grp && grp->shared);

    FUNC_LEAVE_NOAPI(grp->shared->fo_count);
} /* end H5G_get_shared_count() */


/*-------------------------------------------------------------------------
 * Function:	H5G_mount
 *
 * Purpose:	Sets the 'mounted' flag for a group
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, July 19, 2005
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_mount(H5G_t *grp)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_mount);

    /* Check args */
    HDassert(grp && grp->shared);
    HDassert(grp->shared->mounted == FALSE);

    /* Set the 'mounted' flag */
    grp->shared->mounted = TRUE;

    FUNC_LEAVE_NOAPI(SUCCEED);
} /* end H5G_mount() */


/*-------------------------------------------------------------------------
 * Function:	H5G_unmount
 *
 * Purpose:	Resets the 'mounted' flag for a group
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, July 19, 2005
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_unmount(H5G_t *grp)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_unmount);

    /* Check args */
    HDassert(grp && grp->shared);
    HDassert(grp->shared->mounted == TRUE);

    /* Reset the 'mounted' flag */
    grp->shared->mounted = FALSE;

    FUNC_LEAVE_NOAPI(SUCCEED);
} /* end H5G_unmount() */

