/****************************************************************************
* NCSA HDF                                                                 *
* Software Development Group                                               *
* National Center for Supercomputing Applications                          *
* University of Illinois at Urbana-Champaign                               *
* 605 E. Springfield, Champaign IL 61820                                   *
*                                                                          *
* For conditions of distribution and use, see the accompanying             *
* hdf/COPYING file.                                                        *
*                                                                          *
****************************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)$Revision$";
#endif

/* $Id$ */

/*LINTLIBRARY */
/*+
   FILE
       hdf5file.c
   HDF5 "personality template" routines

   EXPORTED ROUTINES
       H5Csetparm   -- Set a parameter for a template
       H5Cgetparm   -- Get a parameter for a template

   LIBRARY-SCOPED ROUTINES

   LOCAL ROUTINES
       H5C_init_interface    -- initialize the interface
   + */

/* Private header files */
#include <H5private.h>      	/* Generic Functions 			*/
#include <H5Aprivate.h>		/* Atoms				*/
#include <H5Bprivate.h>	    	/* B-tree subclass names 		*/
#include <H5Cprivate.h>     	/* Template information 		*/
#include <H5Eprivate.h>		/* Error handling			*/

#define PABLO_MASK	H5C_mask

/*--------------------- Locally scoped variables -----------------------------*/

/* Whether we've installed the library termination function yet for this interface */
static intn interface_initialize_g = FALSE;

/* Define the library's default file creation template (constants in hdf5lims.h) */
const file_create_temp_t default_file_create={
    H5C_USERBLOCK_DEFAULT,      /* Default user-block size */
    H5C_SYM_LEAF_K_DEFAULT,	/* Default 1/2 rank for symtab leaf nodes */
    H5C_BTREE_K_DEFAULT,	/* Default 1/2 rank for btree internal nodes */
    H5C_OFFSETSIZE_DEFAULT,     /* Default offset size */
    H5C_LENGTHSIZE_DEFAULT,     /* Default length size */
    HDF5_BOOTBLOCK_VERSION,     /* Current Boot-Block version # */
    HDF5_SMALLOBJECT_VERSION,   /* Current Small-Object heap version # */
    HDF5_FREESPACE_VERSION,     /* Current Free-Space info version # */
    HDF5_OBJECTDIR_VERSION,     /* Current Object Directory info version # */
    HDF5_SHAREDHEADER_VERSION   /* Current Shared-Header format version # */
};
static hatom_t default_file_id=FAIL;   /* Atom for the default file-creation template */

/*--------------------- Local function prototypes ----------------------------*/
static herr_t H5C_init_interface(void);

/*--------------------------------------------------------------------------
NAME
   H5C_init_interface -- Initialize interface-specific information
USAGE
    herr_t H5C_init_interface()
   
RETURNS
   SUCCEED/FAIL
DESCRIPTION
    Initializes any interface-specific data or routines.

--------------------------------------------------------------------------*/
static herr_t H5C_init_interface(void)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER (H5C_init_interface, NULL, FAIL);

    /* Initialize the atom group for the file IDs */
    ret_value=H5Ainit_group(H5_TEMPLATE,H5A_TEMPID_HASHSIZE,0);

    FUNC_LEAVE(ret_value);
}	/* H5C_init_interface */

/*--------------------------------------------------------------------------
 NAME
    H5C_get_default_atom
 PURPOSE
    Retrive an atom for a default HDF5 template.
 USAGE
    hatom_t H5C_create(type)
        hobjtype_t type;        IN: Type of object to retrieve default template of
 RETURNS
    Returns template ID (atom) of the default object for a template type on
    success, FAIL on failure
 DESCRIPTION
        This is function retrieves atoms for the default templates for the
    different types of HDF5 templates.

 MODIFICATIONS
    Robb Matzke, 4 Aug 1997
    The `FUNC' auto variable was changed from `H5C_create' to
    `H5C_get_default_atom'.
--------------------------------------------------------------------------*/
hatom_t H5C_get_default_atom(hobjtype_t type)
{
    hatom_t        ret_value = FAIL;

    FUNC_ENTER(H5C_get_default_atom, H5C_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    switch(type)
      {
        case H5_TEMPLATE:
            if(default_file_id==FAIL)
              {
                if((default_file_id=H5Aregister_atom(H5_TEMPLATE, (const VOIDP)&default_file_create))==FAIL)
                    HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL);
              } /* end else */
            HGOTO_DONE(default_file_id);
            break;

        default:
            HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);
      } /* end switch */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5C_get_default_atom() */

/*--------------------------------------------------------------------------
 NAME
    H5C_init
 PURPOSE
    Initialize a new HDF5 template with a copy of an existing template.
 USAGE
    herr_t H5C_init(dst_atm, src)
        hatom_t dst_atm;               IN: Atom for the template to initialize
        file_create_temp_t *src;       IN: Template to use to initialize with
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function copies the contents of the source template into the
    newly created destination template.
--------------------------------------------------------------------------*/
herr_t H5C_init(hatom_t dst_atm, const file_create_temp_t *src)
{
    file_create_temp_t *dst;    /* destination template */
    herr_t ret_value = SUCCEED;   /* return value */

    FUNC_ENTER(H5C_init, H5C_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;
    if(src==NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);

    /* Get the template to initialize */
    if((dst=H5Aatom_object(dst_atm))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);

    /* Copy in the source template */
    HDmemcpy(dst,src,sizeof(file_create_temp_t));

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5C_init() */

/*--------------------------------------------------------------------------
 NAME
    H5C_create
 PURPOSE
    Create a new HDF5 template.
 USAGE
    hatom_t H5C_create(owner_id, type, name)
        hatom_t owner_id;       IN: Group/file which owns this template
        hobjtype_t type;        IN: Type of template to create
        const char *name;       IN: Name of the template to create
 RETURNS
    Returns template ID (atom) on success, FAIL on failure
 DESCRIPTION
        This is the primary function for creating different HDF5 templates.
    Currently the name of template is not used and may be NULL.
--------------------------------------------------------------------------*/
hatom_t H5C_create(hatom_t owner_id, hobjtype_t type, const char *name)
{
    hatom_t ret_value = FAIL;   /* atom for template object to return */

    FUNC_ENTER(H5C_create, H5C_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    switch(type)
      {
        case H5_TEMPLATE:
            {
                file_create_temp_t *new_create_temp;    /* new template object to create */

                if((new_create_temp=HDmalloc(sizeof(file_create_temp_t)))==NULL)
                    HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);
                if((ret_value=H5Aregister_atom(H5_TEMPLATE, (const VOIDP)new_create_temp))==FAIL)
                    HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL);
            } /* end case/block */
            break;

        default:
            HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);
      } /* end switch */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5C_create() */

/*--------------------------------------------------------------------------
 NAME
    H5C_release
 PURPOSE
    Release access to a template object.
 USAGE
    herr_t H5C_release(oid)
        hatom_t oid;       IN: Template object to release access to
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function releases access to a template object
--------------------------------------------------------------------------*/
herr_t H5C_release(hatom_t oid)
{
    file_create_temp_t *template;     /* template to destroy */
    herr_t        ret_value = SUCCEED;

    FUNC_ENTER(H5C_release, H5C_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Chuck the object! :-) */
    if((template=H5Aremove_atom(oid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);
    HDfree(template);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5C_release() */

/*--------------------------------------------------------------------------
 NAME
    H5Cgetparm
 PURPOSE
    Get a parameter from a template
 USAGE
    herr_t H5Cgetparm(tid, parm, buf)
        hatom_t tid;        IN: Template object to retrieve parameter from
        file_create_param_t parm;   IN: Paramter to retrieve
        VOIDP buf;          OUT: Pointer to buffer to store parameter in
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function retrieves the value of a specific parameter from a
    template

 MODIFICATIONS
 	Robb Matzke, 13 Aug 1997
	Removed H5_BTREE_SIZE and replaced it with H5_SYM_LEAF_K and
	H5_SYM_INTERN_K.
--------------------------------------------------------------------------*/
herr_t H5Cgetparm(hatom_t tid, file_create_param_t parm, VOIDP buf)
{
    file_create_temp_t *template;    /* template to query */
    herr_t ret_value = SUCCEED;

    FUNC_ENTER(H5Cgetparm, H5C_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;
    if(H5Aatom_group(tid)!=H5_TEMPLATE)
        HGOTO_ERROR(H5E_ATOM, H5E_BADTYPE, FAIL);
    if(buf==NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);

    /* Get a pointer the template to query */
    if((template=H5Aatom_object(tid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);

    switch(parm)
      {
        case H5_USERBLOCK_SIZE:
            *(uintn *)buf=template->userblock_size;
            break;

        case H5_OFFSET_SIZE:
            *(uint8 *)buf=template->offset_size;
            break;

        case H5_LENGTH_SIZE:
            *(uint8 *)buf=template->length_size;
            break;

        case H5_SYM_LEAF_K:
	    *(uintn *)buf=template->sym_leaf_k;
	    break;

        case H5_SYM_INTERN_K:
	    *(uintn *)buf = template->btree_k[H5B_SNODE_ID];
	    break;

        case H5_BOOTBLOCK_VER:
            *(uint8 *)buf=template->bootblock_ver;
            break;

        case H5_SMALLOBJECT_VER:
            *(uint8 *)buf=template->smallobject_ver;
            break;

        case H5_FREESPACE_VER:
            *(uint8 *)buf=template->freespace_ver;
            break;

        case H5_OBJECTDIR_VER:
            *(uint8 *)buf=template->objectdir_ver;
            break;

        case H5_SHAREDHEADER_VER:
            *(uint8 *)buf=template->sharedheader_ver;
            break;

        default:
            HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);
      } /* end switch */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        /* none */
    } /* end if */

    /* Normal function cleanup */
        /* none */

    FUNC_LEAVE(ret_value);
} /* end H5Cgetparm() */

/*--------------------------------------------------------------------------
 NAME
    H5Csetparm
 PURPOSE
    Set a parameter from a template
 USAGE
    herr_t H5Csetparm(tid, parm, buf)
        hatom_t tid;        IN: Template object to store parameter in
        file_create_param_t parm;   IN: Parameter to store
        VOIDP buf;          OUT: Pointer to parameter buffer
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function stores the value of a specific parameter for a template

 MODIFICATIONS
 	Robb Matzke, 13 Aug 1997
	Removed H5_BTREE_SIZE and replaced it with H5_SYM_LEAF_K and
	H5_SYM_INTERN_K.
--------------------------------------------------------------------------*/
herr_t H5Csetparm(hatom_t tid, file_create_param_t parm, const VOIDP buf)
{
    file_create_temp_t *template;    /* template to query */
    herr_t ret_value = SUCCEED;
    uintn val;

    FUNC_ENTER(H5Csetparm, H5C_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;
    if(H5Aatom_group(tid)!=H5_TEMPLATE)
        HGOTO_ERROR(H5E_ATOM, H5E_BADTYPE, FAIL);
    if(buf==NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);

    /* Get a pointer the template to query */
    if((template=H5Aatom_object(tid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);

    switch(parm)
      {
        case H5_USERBLOCK_SIZE:
            val = *(const uintn *)buf;
            /* If anyone knows a faster test for a power of two, please change this silly code -QAK */
            if (val!=0 && !(val==512 || val==1024 || val==2048 || val==4096
                || val==8192 || val==16374 || val==32768 || val==65536
                || val==131072 || val==262144 || val==524288 || val==1048576
                || val==2097152 || val==4194304 || val==8388608
                || val==16777216 || val==33554432 || val==67108864
                || val==134217728 || val==268435456)) {
               HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL);
            }
            template->userblock_size=val;
            break;

        case H5_OFFSET_SIZE:
            val = *(const uintn *)buf;
            if(!(val==2 || val==4 || val==8 || val==16 || val==32 || val==64 || val==128 || hash_size==256))
               HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL);
            template->offset_size=val;
            break;

        case H5_LENGTH_SIZE:
            val = *(const uintn *)buf;
            if(!(val==2 || val==4 || val==8 || val==16 || val==32 || val==64 || val==128 || hash_size==256))
               HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL);
            template->length_size=val;
            break;

        case H5_SYM_LEAF_K:
            val = *(const uintn *)buf;
            if (val<2) {
               HGOTO_ERROR (H5E_ARGS, H5E_BADRANGE, FAIL);
            }
            template->sym_leaf_k = val;
            break;

        case H5_SYM_INTERN_K:
            val = *(const uintn *)buf;
            if (val<2) {
               HGOTO_ERROR (H5E_ARGS, H5E_BADRANGE, FAIL);
            }
            template->btree_k[H5B_SNODE_ID] = val;
            break;
	    
        case H5_BOOTBLOCK_VER:  /* this should be range checked */
            template->bootblock_ver=*(const uint8 *)buf;
            break;

        case H5_SMALLOBJECT_VER:    /* this should be range checked */
            template->smallobject_ver=*(const uint8 *)buf;
            break;

        case H5_FREESPACE_VER:  /* this should be range checked */
            template->freespace_ver=*(const uint8 *)buf;
            break;

        case H5_OBJECTDIR_VER:  /* this should be range checked */
            template->objectdir_ver=*(const uint8 *)buf;
            break;

        case H5_SHAREDHEADER_VER:   /* this should be range checked */
            template->sharedheader_ver=*(const uint8 *)buf;
            break;

        default:
            HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);
      } /* end switch */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        /* none */
    } /* end if */

    /* Normal function cleanup */
        /* none */

    FUNC_LEAVE(ret_value);
} /* end H5Csetparm() */

/*--------------------------------------------------------------------------
 NAME
    H5C_copy
 PURPOSE
    Copy a template
 USAGE
    hatom_t H5C_copy(tid)
        hatom_t tid;        IN: Template object to copy
 RETURNS
    Returns template ID (atom) on success, FAIL on failure
 DESCRIPTION
    This function creates a new copy of a template with all the same parameter
    settings.
--------------------------------------------------------------------------*/
hatom_t H5C_copy(hatom_t tid)
{
    file_create_temp_t *template, *new_template;    /* template to query */
    herr_t ret_value = SUCCEED;

    FUNC_ENTER(H5C_copy, H5C_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Get a pointer the template to query */
    if((template=H5Aatom_object(tid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);

    /* Allocate space for the new template */
    if((new_template=HDmalloc(sizeof(file_create_temp_t)))==NULL)
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);

    /* Copy over the information from the old template */
    HDmemcpy(new_template,template,sizeof(file_create_temp_t));

    /* Register the atom for the new template */
    if((ret_value=H5Aregister_atom(H5_TEMPLATE, (const VOIDP)new_template))==FAIL)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5C_copy() */

