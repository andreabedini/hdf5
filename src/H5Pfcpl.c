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
 * Created:		H5Pfcpl.c
 *			January  6 1998
 *			Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:		File creation property list class routines
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/
#define H5P_PACKAGE		/*suppress error about including H5Ppkg	  */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Bprivate.h"		/* B-tree subclass names		*/
#include "H5Eprivate.h"		/* Error handling			*/
#include "H5Fprivate.h"		/* Files				*/
#include "H5SMprivate.h"        /* Shared object header messages	*/
#include "H5Ppkg.h"		/* Property lists		 	*/


/****************/
/* Local Macros */
/****************/

/* ========= File Creation properties ============ */
/* Definitions for the size of the file user block in bytes */
#define H5F_CRT_USER_BLOCK_SIZE      sizeof(hsize_t)
#define H5F_CRT_USER_BLOCK_DEF       0
#define H5F_CRT_USER_BLOCK_ENC       H5P__encode_hsize_t
#define H5F_CRT_USER_BLOCK_DEC       H5P__decode_hsize_t
/* Definitions for the 1/2 rank for symbol table leaf nodes */
#define H5F_CRT_SYM_LEAF_SIZE        sizeof(unsigned)
#define H5F_CRT_SYM_LEAF_ENC         H5P__encode_unsigned
#define H5F_CRT_SYM_LEAF_DEC         H5P__decode_unsigned
/* Definitions for the 1/2 rank for btree internal nodes    */
#define H5F_CRT_BTREE_RANK_SIZE      sizeof(unsigned[H5B_NUM_BTREE_ID])
#define H5F_CRT_BTREE_RANK_DEF       {HDF5_BTREE_SNODE_IK_DEF,HDF5_BTREE_CHUNK_IK_DEF}
#define H5F_CRT_BTREE_RANK_ENC       H5P__fcrt_btree_rank_enc
#define H5F_CRT_BTREE_RANK_DEC       H5P__fcrt_btree_rank_dec
/* Definitions for byte number in an address                */
#define H5F_CRT_ADDR_BYTE_NUM_SIZE   sizeof(uint8_t)
#define H5F_CRT_ADDR_BYTE_NUM_DEF    H5F_OBJ_ADDR_SIZE
#define H5F_CRT_ADDR_BYTE_NUM_ENC    H5P__encode_uint8_t
#define H5F_CRT_ADDR_BYTE_NUM_DEC    H5P__decode_uint8_t
/* Definitions for byte number for object size              */
#define H5F_CRT_OBJ_BYTE_NUM_SIZE     sizeof(uint8_t)
#define H5F_CRT_OBJ_BYTE_NUM_DEF      H5F_OBJ_SIZE_SIZE
#define H5F_CRT_OBJ_BYTE_NUM_ENC      H5P__encode_uint8_t
#define H5F_CRT_OBJ_BYTE_NUM_DEC      H5P__decode_uint8_t
/* Definitions for version number of the superblock         */
#define H5F_CRT_SUPER_VERS_SIZE       sizeof(unsigned)
#define H5F_CRT_SUPER_VERS_DEF        HDF5_SUPERBLOCK_VERSION_DEF
/* Definitions for shared object header messages */
#define H5F_CRT_SHMSG_NINDEXES_SIZE    sizeof(unsigned)
#define H5F_CRT_SHMSG_NINDEXES_DEF     (0)
#define H5F_CRT_SHMSG_NINDEXES_ENC     H5P__encode_unsigned
#define H5F_CRT_SHMSG_NINDEXES_DEC     H5P__decode_unsigned
#define H5F_CRT_SHMSG_INDEX_TYPES_SIZE sizeof(unsigned[H5O_SHMESG_MAX_NINDEXES])
#define H5F_CRT_SHMSG_INDEX_TYPES_DEF  {0,0,0,0,0,0}
#define H5F_CRT_SHMSG_INDEX_TYPES_ENC  H5P__fcrt_shmsg_index_types_enc
#define H5F_CRT_SHMSG_INDEX_TYPES_DEC  H5P__fcrt_shmsg_index_types_dec
#define H5F_CRT_SHMSG_INDEX_MINSIZE_SIZE sizeof(unsigned[H5O_SHMESG_MAX_NINDEXES])
#define H5F_CRT_SHMSG_INDEX_MINSIZE_DEF {250,250,250,250,250,250}
#define H5F_CRT_SHMSG_INDEX_MINSIZE_ENC H5P__fcrt_shmsg_index_minsize_enc
#define H5F_CRT_SHMSG_INDEX_MINSIZE_DEC H5P__fcrt_shmsg_index_minsize_dec
/* Definitions for shared object header list/btree phase change cutoffs */
#define H5F_CRT_SHMSG_LIST_MAX_SIZE     sizeof(unsigned)
#define H5F_CRT_SHMSG_LIST_MAX_DEF      (50)
#define H5F_CRT_SHMSG_LIST_MAX_ENC      H5P__encode_unsigned
#define H5F_CRT_SHMSG_LIST_MAX_DEC      H5P__decode_unsigned
#define H5F_CRT_SHMSG_BTREE_MIN_SIZE    sizeof(unsigned)
#define H5F_CRT_SHMSG_BTREE_MIN_DEF     (40)
#define H5F_CRT_SHMSG_BTREE_MIN_ENC     H5P__encode_unsigned
#define H5F_CRT_SHMSG_BTREE_MIN_DEC     H5P__decode_unsigned
/* Definitions for file space handling strategy */
#define H5F_CRT_FILE_SPACE_STRATEGY_SIZE       sizeof(unsigned)
#define H5F_CRT_FILE_SPACE_STRATEGY_DEF        H5F_FILE_SPACE_STRATEGY_DEF
#define H5F_CRT_FILE_SPACE_STRATEGY_ENC        H5P__encode_unsigned
#define H5F_CRT_FILE_SPACE_STRATEGY_DEC        H5P__decode_unsigned
#define H5F_CRT_FREE_SPACE_THRESHOLD_SIZE      sizeof(hsize_t)
#define H5F_CRT_FREE_SPACE_THRESHOLD_DEF       H5F_FREE_SPACE_THRESHOLD_DEF
#define H5F_CRT_FREE_SPACE_THRESHOLD_ENC       H5P__encode_hsize_t
#define H5F_CRT_FREE_SPACE_THRESHOLD_DEC       H5P__decode_hsize_t
/* Definitions for avoiding file truncation */
#define H5F_CRT_AVOID_TRUNCATE_SIZE     sizeof(H5F_avoid_truncate_t)
#define H5F_CRT_AVOID_TRUNCATE_DEF      (H5F_AVOID_TRUNCATE_OFF)
#define H5F_CRT_AVOID_TRUNCATE_ENC      H5P__fcrt_avoid_truncate_enc
#define H5F_CRT_AVOID_TRUNCATE_DEC      H5P__fcrt_avoid_truncate_dec


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/

/* Property class callbacks */
static herr_t H5P_fcrt_reg_prop(H5P_genclass_t *pclass);

/* property callbacks */
static herr_t H5P__fcrt_btree_rank_enc(const void *value, void **_pp, size_t *size);
static herr_t H5P__fcrt_btree_rank_dec(const void **_pp, void *value);
static herr_t H5P__fcrt_shmsg_index_types_enc(const void *value, void **_pp, size_t *size);
static herr_t H5P__fcrt_shmsg_index_types_dec(const void **_pp, void *value);
static herr_t H5P__fcrt_shmsg_index_minsize_enc(const void *value, void **_pp, size_t *size);
static herr_t H5P__fcrt_shmsg_index_minsize_dec(const void **_pp, void *value);
static herr_t H5P__fcrt_avoid_truncate_enc(const void *value, void **_pp, size_t *size);
static herr_t H5P__fcrt_avoid_truncate_dec(const void **_pp, void *value);


/*********************/
/* Package Variables */
/*********************/

/* File creation property list class library initialization object */
const H5P_libclass_t H5P_CLS_FCRT[1] = {{
    "file create",		/* Class name for debugging     */
    H5P_TYPE_FILE_CREATE,       /* Class type                   */

    &H5P_CLS_GROUP_CREATE_g,	/* Parent class                 */
    &H5P_CLS_FILE_CREATE_g,	/* Pointer to class             */
    &H5P_CLS_FILE_CREATE_ID_g,	/* Pointer to class ID          */
    &H5P_LST_FILE_CREATE_ID_g,	/* Pointer to default property list ID */
    H5P_fcrt_reg_prop,		/* Default property registration routine */

    NULL,		        /* Class creation callback      */
    NULL,		        /* Class creation callback info */
    NULL,			/* Class copy callback          */
    NULL,		        /* Class copy callback info     */
    NULL,			/* Class close callback         */
    NULL 		        /* Class close callback info    */
}};


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/

/* Property value defaults */
static const hsize_t H5F_def_userblock_size_g = H5F_CRT_USER_BLOCK_DEF;    /* Default userblock size */
static const unsigned H5F_def_sym_leaf_k_g = H5F_CRT_SYM_LEAF_DEF;         /* Default size for symbol table leaf nodes */
static const unsigned H5F_def_btree_k_g[H5B_NUM_BTREE_ID] = H5F_CRT_BTREE_RANK_DEF;    /* Default 'K' values for B-trees in file */
static const uint8_t H5F_def_sizeof_addr_g = H5F_CRT_ADDR_BYTE_NUM_DEF;     /* Default size of addresses in the file */
static const uint8_t H5F_def_sizeof_size_g = H5F_CRT_OBJ_BYTE_NUM_DEF;      /* Default size of sizes in the file */
static const unsigned H5F_def_superblock_ver_g = H5F_CRT_SUPER_VERS_DEF;   /* Default superblock version # */
static const unsigned H5F_def_num_sohm_indexes_g    = H5F_CRT_SHMSG_NINDEXES_DEF;
static const unsigned H5F_def_sohm_index_flags_g[H5O_SHMESG_MAX_NINDEXES]    = H5F_CRT_SHMSG_INDEX_TYPES_DEF;
static const unsigned H5F_def_sohm_index_minsizes_g[H5O_SHMESG_MAX_NINDEXES] = H5F_CRT_SHMSG_INDEX_MINSIZE_DEF;
static const unsigned H5F_def_sohm_list_max_g  = H5F_CRT_SHMSG_LIST_MAX_DEF;
static const unsigned H5F_def_sohm_btree_min_g  = H5F_CRT_SHMSG_BTREE_MIN_DEF;
static const unsigned H5F_def_file_space_strategy_g = H5F_CRT_FILE_SPACE_STRATEGY_DEF;
static const hsize_t H5F_def_free_space_threshold_g = H5F_CRT_FREE_SPACE_THRESHOLD_DEF;
static const H5F_avoid_truncate_t H5F_def_avoid_truncate_g = H5F_CRT_AVOID_TRUNCATE_DEF;



/*-------------------------------------------------------------------------
 * Function:    H5P_fcrt_reg_prop
 *
 * Purpose:     Register the file creation property list class's properties
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              October 31, 2006
 *-------------------------------------------------------------------------
 */
static herr_t
H5P_fcrt_reg_prop(H5P_genclass_t *pclass)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* Register the user block size */
    if(H5P_register_real(pclass, H5F_CRT_USER_BLOCK_NAME, H5F_CRT_USER_BLOCK_SIZE, &H5F_def_userblock_size_g, 
            NULL, NULL, NULL, H5F_CRT_USER_BLOCK_ENC, H5F_CRT_USER_BLOCK_DEC, 
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the 1/2 rank for symbol table leaf nodes */
    if(H5P_register_real(pclass, H5F_CRT_SYM_LEAF_NAME, H5F_CRT_SYM_LEAF_SIZE, &H5F_def_sym_leaf_k_g, 
            NULL, NULL, NULL, H5F_CRT_SYM_LEAF_ENC, H5F_CRT_SYM_LEAF_DEC, 
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the 1/2 rank for btree internal nodes */
    if(H5P_register_real(pclass, H5F_CRT_BTREE_RANK_NAME, H5F_CRT_BTREE_RANK_SIZE, H5F_def_btree_k_g, 
            NULL, NULL, NULL, H5F_CRT_BTREE_RANK_ENC, H5F_CRT_BTREE_RANK_DEC, 
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the byte number for an address */
    if(H5P_register_real(pclass, H5F_CRT_ADDR_BYTE_NUM_NAME, H5F_CRT_ADDR_BYTE_NUM_SIZE, &H5F_def_sizeof_addr_g, 
            NULL, NULL, NULL, H5F_CRT_ADDR_BYTE_NUM_ENC, H5F_CRT_ADDR_BYTE_NUM_DEC, 
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the byte number for object size */
    if(H5P_register_real(pclass, H5F_CRT_OBJ_BYTE_NUM_NAME, H5F_CRT_OBJ_BYTE_NUM_SIZE, &H5F_def_sizeof_size_g, 
            NULL, NULL, NULL, H5F_CRT_OBJ_BYTE_NUM_ENC, H5F_CRT_OBJ_BYTE_NUM_DEC, 
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the superblock version number */
    /* (Note: this property should not have an encode/decode callback -QAK) */
    if(H5P_register_real(pclass, H5F_CRT_SUPER_VERS_NAME, H5F_CRT_SUPER_VERS_SIZE, &H5F_def_superblock_ver_g, 
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the shared OH message information */
    if(H5P_register_real(pclass, H5F_CRT_SHMSG_NINDEXES_NAME, H5F_CRT_SHMSG_NINDEXES_SIZE, &H5F_def_num_sohm_indexes_g,
            NULL, NULL, NULL, H5F_CRT_SHMSG_NINDEXES_ENC, H5F_CRT_SHMSG_NINDEXES_DEC,
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")
    if(H5P_register_real(pclass, H5F_CRT_SHMSG_INDEX_TYPES_NAME, H5F_CRT_SHMSG_INDEX_TYPES_SIZE, &H5F_def_sohm_index_flags_g,
            NULL, NULL, NULL, H5F_CRT_SHMSG_INDEX_TYPES_ENC, H5F_CRT_SHMSG_INDEX_TYPES_DEC,
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")
    if(H5P_register_real(pclass, H5F_CRT_SHMSG_INDEX_MINSIZE_NAME, H5F_CRT_SHMSG_INDEX_MINSIZE_SIZE, &H5F_def_sohm_index_minsizes_g,
            NULL, NULL, NULL, H5F_CRT_SHMSG_INDEX_MINSIZE_ENC, H5F_CRT_SHMSG_INDEX_MINSIZE_DEC,
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the shared OH cutoff size information */
    if(H5P_register_real(pclass, H5F_CRT_SHMSG_LIST_MAX_NAME, H5F_CRT_SHMSG_LIST_MAX_SIZE, &H5F_def_sohm_list_max_g,
            NULL, NULL, NULL, H5F_CRT_SHMSG_LIST_MAX_ENC, H5F_CRT_SHMSG_LIST_MAX_DEC,
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")
    if(H5P_register_real(pclass, H5F_CRT_SHMSG_BTREE_MIN_NAME, H5F_CRT_SHMSG_BTREE_MIN_SIZE, &H5F_def_sohm_btree_min_g,
            NULL, NULL, NULL, H5F_CRT_SHMSG_BTREE_MIN_ENC, H5F_CRT_SHMSG_BTREE_MIN_DEC,
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the file space handling strategy */
    if(H5P_register_real(pclass, H5F_CRT_FILE_SPACE_STRATEGY_NAME, H5F_CRT_FILE_SPACE_STRATEGY_SIZE, &H5F_def_file_space_strategy_g, 
            NULL, NULL, NULL, H5F_CRT_FILE_SPACE_STRATEGY_ENC, H5F_CRT_FILE_SPACE_STRATEGY_DEC, 
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the free space section threshold */
    if(H5P_register_real(pclass, H5F_CRT_FREE_SPACE_THRESHOLD_NAME, H5F_CRT_FREE_SPACE_THRESHOLD_SIZE, &H5F_def_free_space_threshold_g, 
            NULL, NULL, NULL, H5F_CRT_FREE_SPACE_THRESHOLD_ENC, H5F_CRT_FREE_SPACE_THRESHOLD_DEC, 
            NULL, NULL, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the avoid truncate property */
    if(H5P_register_real(pclass, H5F_CRT_AVOID_TRUNCATE_NAME, H5F_CRT_AVOID_TRUNCATE_SIZE, &H5F_def_avoid_truncate_g,
            NULL, NULL, NULL, H5F_CRT_AVOID_TRUNCATE_ENC, H5F_CRT_AVOID_TRUNCATE_DEC,
            NULL, NULL, NULL, NULL) < 0)
         HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_fcrt_reg_prop() */


/*-------------------------------------------------------------------------
 * Function:	H5Pset_userblock
 *
 * Purpose:	Sets the userblock size field of a file creation property
 *		list.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Tuesday, January  6, 1998
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_userblock(hid_t plist_id, hsize_t size)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t ret_value = SUCCEED; /* return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "ih", plist_id, size);

    /* Sanity check non-zero userblock sizes */
    if(size > 0) {
        /* Check that the userblock size is >=512 */
        if(size < 512)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "userblock size is non-zero and less than 512")

        /* Check that the userblock size is a power of two */
        if(!POWER_OF_TWO(size))
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "userblock size is non-zero and not a power of two")
    } /* end if */

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id, H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Set value */
    if(H5P_set(plist, H5F_CRT_USER_BLOCK_NAME, &size) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set user block")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_userblock() */


/*-------------------------------------------------------------------------
 * Function:	H5Pget_userblock
 *
 * Purpose:	Queries the size of a user block in a file creation property
 *		list.
 *
 * Return:	Success:	Non-negative, size returned through SIZE argument.
 *
 *		Failure:	Negative
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January  7, 1998
 *
 * Modifications:
 *
 *		Raymond Lu, Oct 14, 2001
 *		Changed to the new generic property list.
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_userblock(hid_t plist_id, hsize_t *size)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t ret_value = SUCCEED;   /* return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*h", plist_id, size);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    /* Get value */
    if (size)
        if(H5P_get(plist, H5F_CRT_USER_BLOCK_NAME, size) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL,"can't get user block");

done:
    FUNC_LEAVE_API(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5Pset_sizes
 *
 * Purpose:	Sets file size-of addresses and sizes.	PLIST_ID should be a
 *		file creation property list.  A value of zero causes the
 *		property to not change.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Tuesday, January  6, 1998
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_sizes(hid_t plist_id, size_t sizeof_addr, size_t sizeof_size)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t ret_value = SUCCEED;   /* return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "izz", plist_id, sizeof_addr, sizeof_size);

    /* Check arguments */
    if(sizeof_addr) {
        if(sizeof_addr != 2 && sizeof_addr != 4 && sizeof_addr != 8 && sizeof_addr != 16)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "file haddr_t size is not valid")
    } /* end if */
    if(sizeof_size) {
        if(sizeof_size != 2 && sizeof_size != 4 && sizeof_size != 8 && sizeof_size != 16)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "file size_t size is not valid")
    } /* end if */

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Set value */
    if(sizeof_addr) {
        uint8_t tmp_sizeof_addr = (uint8_t)sizeof_addr;

        if(H5P_set(plist, H5F_CRT_ADDR_BYTE_NUM_NAME, &tmp_sizeof_addr) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set byte number for an address")
    } /* end if */
    if(sizeof_size) {
        uint8_t tmp_sizeof_size = (uint8_t)sizeof_size;

        if(H5P_set(plist, H5F_CRT_OBJ_BYTE_NUM_NAME, &tmp_sizeof_size) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set byte number for object ")
    } /* end if */

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_sizes() */


/*-------------------------------------------------------------------------
 * Function:	H5Pget_sizes
 *
 * Purpose:	Returns the size of address and size quantities stored in a
 *		file according to a file creation property list.  Either (or
 *		even both) SIZEOF_ADDR and SIZEOF_SIZE may be null pointers.
 *
 * Return:	Success:	Non-negative, sizes returned through arguments.
 *		Failure:	Negative
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January  7, 1998
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_sizes(hid_t plist_id, size_t *sizeof_addr, size_t *sizeof_size)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t ret_value = SUCCEED;   /* return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "i*z*z", plist_id, sizeof_addr, sizeof_size);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get values */
    if(sizeof_addr) {
        uint8_t tmp_sizeof_addr;

        if(H5P_get(plist, H5F_CRT_ADDR_BYTE_NUM_NAME, &tmp_sizeof_addr) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get byte number for an address")
        *sizeof_addr = tmp_sizeof_addr;
    } /* end if */
    if(sizeof_size) {
        uint8_t tmp_sizeof_size;

        if(H5P_get(plist, H5F_CRT_OBJ_BYTE_NUM_NAME, &tmp_sizeof_size) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get byte number for object ")
        *sizeof_size = tmp_sizeof_size;
    } /* end if */

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_sizes() */


/*-------------------------------------------------------------------------
 * Function:	H5Pset_sym_k
 *
 * Purpose:	IK is one half the rank of a tree that stores a symbol
 *		table for a group.  Internal nodes of the symbol table are on
 *		average 75% full.  That is, the average rank of the tree is
 *		1.5 times the value of IK.
 *
 *		LK is one half of the number of symbols that can be stored in
 *		a symbol table node.  A symbol table node is the leaf of a
 *		symbol table tree which is used to store a group.  When
 *		symbols are inserted randomly into a group, the group's
 *		symbol table nodes are 75% full on average.  That is, they
 *		contain 1.5 times the number of symbols specified by LK.
 *
 *		Either (or even both) of IK and LK can be zero in which case
 *		that value is left unchanged.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Tuesday, January  6, 1998
 *
 * Modifications:
 *
 *		Raymond Lu, Oct 14, 2001
 *         	Changed to the new generic property list.
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_sym_k(hid_t plist_id, unsigned ik, unsigned lk)
{
    unsigned btree_k[H5B_NUM_BTREE_ID];
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "iIuIu", plist_id, ik, lk);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    /* Set values */
    if (ik > 0) {
	if((ik * 2) >= HDF5_BTREE_IK_MAX_ENTRIES)
	    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "istore IK value exceeds maximum B-tree entries");

        if(H5P_get(plist, H5F_CRT_BTREE_RANK_NAME, btree_k) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get rank for btree interanl nodes");
        btree_k[H5B_SNODE_ID] = ik;
        if(H5P_set(plist, H5F_CRT_BTREE_RANK_NAME, btree_k) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set rank for btree nodes");
    }
    if (lk > 0)
        if(H5P_set(plist, H5F_CRT_SYM_LEAF_NAME, &lk) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set rank for symbol table leaf nodes");

done:
    FUNC_LEAVE_API(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5Pget_sym_k
 *
 * Purpose:	Retrieves the symbol table B-tree 1/2 rank (IK) and the
 *		symbol table leaf node 1/2 size (LK).  See H5Pset_sym_k() for
 *		details. Either (or even both) IK and LK may be null
 *		pointers.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January  7, 1998
 *
 * Modifications:
 *
 *		Raymond Lu
 *		Changed to the new generic property list.
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_sym_k(hid_t plist_id, unsigned *ik /*out */ , unsigned *lk /*out */ )
{
    unsigned btree_k[H5B_NUM_BTREE_ID];
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "ixx", plist_id, ik, lk);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    /* Get values */
    if (ik) {
        if(H5P_get(plist, H5F_CRT_BTREE_RANK_NAME, btree_k) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get rank for btree nodes");
        *ik = btree_k[H5B_SNODE_ID];
    }
    if (lk)
        if(H5P_get(plist, H5F_CRT_SYM_LEAF_NAME, lk) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get rank for symbol table leaf nodes");

done:
    FUNC_LEAVE_API(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5Pset_istore_k
 *
 * Purpose:	IK is one half the rank of a tree that stores chunked raw
 *		data.  On average, such a tree will be 75% full, or have an
 *		average rank of 1.5 times the value of IK.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Tuesday, January  6, 1998
 *
 * Modifications:
 *
 *		Raymond Lu, Oct 14, 2001
 *		Changed to the new generic property list.
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_istore_k(hid_t plist_id, unsigned ik)
{
    unsigned btree_k[H5B_NUM_BTREE_ID];
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "iIu", plist_id, ik);

    /* Check arguments */
    if (ik == 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "istore IK value must be positive");

    if((ik * 2) >= HDF5_BTREE_IK_MAX_ENTRIES)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "istore IK value exceeds maximum B-tree entries");

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    /* Set value */
    if(H5P_get(plist, H5F_CRT_BTREE_RANK_NAME, btree_k) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get rank for btree interanl nodes");
    btree_k[H5B_CHUNK_ID] = ik;
    if(H5P_set(plist, H5F_CRT_BTREE_RANK_NAME, btree_k) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set rank for btree interanl nodes");

done:
    FUNC_LEAVE_API(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5Pget_istore_k
 *
 * Purpose:	Queries the 1/2 rank of an indexed storage B-tree.  See
 *		H5Pset_istore_k() for details.	The argument IK may be the
 *		null pointer.
 *
 * Return:	Success:	Non-negative, size returned through IK
 *
 *		Failure:	Negative
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January  7, 1998
 *
 * Modifications:
 *
 *		Raymond Lu, Oct 14, 2001
 *		Changed to the new generic property list.
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_istore_k(hid_t plist_id, unsigned *ik /*out */ )
{
    unsigned btree_k[H5B_NUM_BTREE_ID];
    H5P_genplist_t *plist;              /* Property list pointer */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "ix", plist_id, ik);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    /* Get value */
    if(ik) {
        if(H5P_get(plist, H5F_CRT_BTREE_RANK_NAME, btree_k) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get rank for btree interanl nodes");
        *ik = btree_k[H5B_CHUNK_ID];
    } /* end if */

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_istore_k() */


/*-------------------------------------------------------------------------
 * Function:       H5P__fcrt_btree_rank_enc
 *
 * Purpose:        Callback routine which is called whenever the index storage
 *                 btree in file creation property list is encoded.
 *
 * Return:	   Success:	Non-negative
 *		   Failure:	Negative
 *
 * Programmer:     Mohamad Chaarawi
 *                 August 7, 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__fcrt_btree_rank_enc(const void *value, void **_pp, size_t *size)
{
    const unsigned *btree_k = (const unsigned *)value; /* Create local alias for values */
    uint8_t **pp = (uint8_t **)_pp;

    FUNC_ENTER_STATIC_NOERR

    /* Sanity check */
    HDassert(btree_k);
    HDassert(size);

    if(NULL != *pp) {
        unsigned u;             /* Local index variable */

        /* Encode the size of an unsigned*/
        *(*pp)++ = (uint8_t)sizeof(unsigned);

        /* Encode all the btree  */
        for(u = 0 ; u < H5B_NUM_BTREE_ID; u++) {
            /* Encode the left split value */
            H5_ENCODE_UNSIGNED(*pp, *(const unsigned *)btree_k)
            btree_k++;
        } /* end for */
    } /* end if */

    /* Size of type flags values */
    *size += 1 + (H5B_NUM_BTREE_ID * sizeof(unsigned));

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5P__fcrt_btree_rank_enc() */


/*-------------------------------------------------------------------------
 * Function:       H5P__fcrt_btree_rank_dec
 *
 * Purpose:        Callback routine which is called whenever the index storage
 *                 btree in file creation property list is decoded.
 *
 * Return:	   Success:	Non-negative
 *		   Failure:	Negative
 *
 * Programmer:     Mohamad Chaarawi
 *                 August 7, 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__fcrt_btree_rank_dec(const void **_pp, void *_value)
{
    unsigned *btree_k = (unsigned *)_value;
    const uint8_t **pp = (const uint8_t **)_pp;
    unsigned enc_size;                  /* Size of encoded property */
    unsigned u;                         /* Local index variable */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_STATIC

    /* Sanity checks */
    HDassert(pp);
    HDassert(*pp);
    HDassert(btree_k);

    /* Decode the size */
    enc_size = *(*pp)++;
    if(enc_size != sizeof(unsigned))
        HGOTO_ERROR(H5E_PLIST, H5E_BADVALUE, FAIL, "unsigned value can't be decoded")

    /* Decode all the type flags */
    for(u = 0 ; u < H5B_NUM_BTREE_ID; u++)
        H5_DECODE_UNSIGNED(*pp, btree_k[u])

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P__fcrt_btree_rank_dec() */


/*-------------------------------------------------------------------------
 * Function:	H5Pset_shared_mesg_nindexes
 *
 * Purpose:	Set the number of Shared Object Header Message (SOHM)
 *              indexes specified in this property list.  If this is
 *              zero then shared object header messages are disabled
 *              for this file.
 *
 *              These indexes can then be configured with
 *              H5Pset_shared_mesg_index.  H5Pset_shared_mesg_phase_chage
 *              also controls settings for all indexes.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	James Laird
 *		Monday, October 9, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_shared_mesg_nindexes(hid_t plist_id, unsigned nindexes)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "iIu", plist_id, nindexes);

    /* Check argument */
    if (nindexes > H5O_SHMESG_MAX_NINDEXES)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "number of indexes is greater than H5O_SHMESG_MAX_NINDEXES");

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id, H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    if(H5P_set(plist, H5F_CRT_SHMSG_NINDEXES_NAME, &nindexes) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set number of indexes");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_shared_mesg_nindexes() */


/*-------------------------------------------------------------------------
 * Function:	H5Pget_shared_mesg_nindexes
 *
 * Purpose:	Get the number of Shared Object Header Message (SOHM)
 *              indexes specified in this property list.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	James Laird
 *		Monday, October 9, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_shared_mesg_nindexes(hid_t plist_id, unsigned *nindexes)
{
    H5P_genplist_t *plist;              /* Property list pointer */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*Iu", plist_id, nindexes);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    if(H5P_get(plist, H5F_CRT_SHMSG_NINDEXES_NAME, nindexes) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get number of indexes");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_shared_mesg_nindexes() */


/*-------------------------------------------------------------------------
 * Function:	H5Pset_shared_mesg_index
 *
 * Purpose:	Configure a given shared message index.  Sets the types of
 *              message that should be stored in this index and the minimum
 *              size of a message in the index.
 *
 *              INDEX_NUM is zero-indexed (in a file with three indexes,
 *              they are numbered 0, 1, and 2).
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	James Laird
 *		Wednesday, April 5, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_shared_mesg_index(hid_t plist_id, unsigned index_num, unsigned mesg_type_flags, unsigned min_mesg_size)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    unsigned    nindexes;               /* Number of SOHM indexes */
    unsigned    type_flags[H5O_SHMESG_MAX_NINDEXES]; /* Array of mesg_type_flags*/
    unsigned    minsizes[H5O_SHMESG_MAX_NINDEXES]; /* Array of min_mesg_sizes*/
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iIuIuIu", plist_id, index_num, mesg_type_flags, min_mesg_size);

    /* Check arguments */
    if(mesg_type_flags > H5O_SHMESG_ALL_FLAG)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "unrecognized flags in mesg_type_flags")

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Read the current number of indexes */
    if(H5P_get(plist, H5F_CRT_SHMSG_NINDEXES_NAME, &nindexes) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get number of indexes")

    /* Range check */
    if(index_num >= nindexes)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "index_num is too large; no such index");

    /* Get arrays of type flags and message sizes */
    if(H5P_get(plist, H5F_CRT_SHMSG_INDEX_TYPES_NAME, type_flags) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get current index type flags")
    if(H5P_get(plist, H5F_CRT_SHMSG_INDEX_MINSIZE_NAME, minsizes) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get current min sizes")

    /* Set values in arrays */
    type_flags[index_num] = mesg_type_flags;
    minsizes[index_num] = min_mesg_size;

    /* Write arrays back to plist */
    if(H5P_set(plist, H5F_CRT_SHMSG_INDEX_TYPES_NAME, type_flags) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set index type flags")
    if(H5P_set(plist, H5F_CRT_SHMSG_INDEX_MINSIZE_NAME, minsizes) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set min mesg sizes")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_shared_mesg_index() */


/*-------------------------------------------------------------------------
 * Function:	H5Pget_shared_mesg_index
 *
 * Purpose:	Get information about a given shared message index.  Gets
 *              the types of message that are stored in the index and the
 *              minimum size of a message in the index.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	James Laird
 *		Wednesday, April 5, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_shared_mesg_index(hid_t plist_id, unsigned index_num, unsigned *mesg_type_flags, unsigned *min_mesg_size)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    unsigned    nindexes;               /* Number of SOHM indexes */
    unsigned    type_flags[H5O_SHMESG_MAX_NINDEXES]; /* Array of mesg_type_flags*/
    unsigned    minsizes[H5O_SHMESG_MAX_NINDEXES]; /* Array of min_mesg_sizes*/
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iIu*Iu*Iu", plist_id, index_num, mesg_type_flags,
             min_mesg_size);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Read the current number of indexes */
    if(H5P_get(plist, H5F_CRT_SHMSG_NINDEXES_NAME, &nindexes) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get number of indexes")

    if(index_num >= nindexes)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "index_num is greater than number of indexes in property list")

    /* Get arrays of type flags and message sizes */
    if(H5P_get(plist, H5F_CRT_SHMSG_INDEX_TYPES_NAME, type_flags) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get current index type flags")
    if(H5P_get(plist, H5F_CRT_SHMSG_INDEX_MINSIZE_NAME, minsizes) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get current min sizes")

    /* Get values from arrays */
    if(mesg_type_flags)
        *mesg_type_flags = type_flags[index_num];
    if(min_mesg_size)
        *min_mesg_size = minsizes[index_num];

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_shared_mesg_index() */


/*-------------------------------------------------------------------------
 * Function:       H5P__fcrt_shmsg_index_types_enc
 *
 * Purpose:        Callback routine which is called whenever the shared
 *                 message indec types in file creation property list
 *                 is encoded.
 *
 * Return:	   Success:	Non-negative
 *		   Failure:	Negative
 *
 * Programmer:     Mohamad Chaarawi
 *                 August 7, 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__fcrt_shmsg_index_types_enc(const void *value, void **_pp, size_t *size)
{
    const unsigned *type_flags = (const unsigned *)value; /* Create local alias for values */
    uint8_t **pp = (uint8_t **)_pp;

    FUNC_ENTER_STATIC_NOERR

    /* Sanity check */
    HDassert(type_flags);
    HDassert(size);

    if(NULL != *pp) {
        unsigned u;             /* Local index variable */

        /* Encode the size of a double*/
        *(*pp)++ = (uint8_t)sizeof(unsigned);

        /* Encode all the type flags */
        for(u = 0; u < H5O_SHMESG_MAX_NINDEXES; u++) {
            /* Encode the left split value */
            H5_ENCODE_UNSIGNED(*pp, *(const unsigned *)type_flags)
            type_flags++;
        } /* end for */
    } /* end if */

    /* Size of type flags values */
    *size += 1 + (H5O_SHMESG_MAX_NINDEXES * sizeof(unsigned));

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5P__fcrt_shmsg_index_types_enc() */


/*-------------------------------------------------------------------------
 * Function:       H5P__fcrt_shmsg_index_types_dec
 *
 * Purpose:        Callback routine which is called whenever the shared
 *                 message indec types in file creation property list
 *                 is decoded.
 *
 * Return:	   Success:	Non-negative
 *		   Failure:	Negative
 *
 * Programmer:     Mohamad Chaarawi
 *                 August 7, 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__fcrt_shmsg_index_types_dec(const void **_pp, void *_value)
{
    unsigned *type_flags = (unsigned *)_value;
    const uint8_t **pp = (const uint8_t **)_pp;
    unsigned enc_size;                  /* Size of encoded property */
    unsigned u;                         /* Local index variable */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_STATIC

    /* Sanity checks */
    HDassert(pp);
    HDassert(*pp);
    HDassert(type_flags);

    /* Decode the size */
    enc_size = *(*pp)++;
    if(enc_size != sizeof(unsigned))
        HGOTO_ERROR(H5E_PLIST, H5E_BADVALUE, FAIL, "unsigned value can't be decoded")

    /* Decode all the type flags */
    for(u = 0; u < H5O_SHMESG_MAX_NINDEXES; u++)
        H5_DECODE_UNSIGNED(*pp, type_flags[u])

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P__fcrt_shmsg_index_types_dec() */


/*-------------------------------------------------------------------------
 * Function:       H5P__fcrt_shmsg_index_minsize_enc
 *
 * Purpose:        Callback routine which is called whenever the shared
 *                 message index minsize in file creation property list
 *                 is encoded.
 *
 * Return:	   Success:	Non-negative
 *		   Failure:	Negative
 *
 * Programmer:     Mohamad Chaarawi
 *                 August 7, 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__fcrt_shmsg_index_minsize_enc(const void *value, void **_pp, size_t *size)
{
    const unsigned *minsizes = (const unsigned *)value; /* Create local alias for values */
    uint8_t **pp = (uint8_t **)_pp;

    FUNC_ENTER_STATIC_NOERR

    /* Sanity check */
    HDassert(minsizes);
    HDassert(size);

    if(NULL != *pp) {
        unsigned u;             /* Local index variable */

        /* Encode the size of a double*/
        *(*pp)++ = (uint8_t)sizeof(unsigned);

        /* Encode all the minsize values */
        for(u = 0 ; u < H5O_SHMESG_MAX_NINDEXES; u++) {
            /* Encode the left split value */
            H5_ENCODE_UNSIGNED(*pp, *(const unsigned *)minsizes)
            minsizes++;
        } /* end for */
    } /* end if */

    /* Size of type flags values */
    *size += 1 + (H5O_SHMESG_MAX_NINDEXES * sizeof(unsigned));

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5P__fcrt_shmsg_index_minsize_enc() */


/*-------------------------------------------------------------------------
 * Function:       H5P__fcrt_shmsg_index_minsize_dec
 *
 * Purpose:        Callback routine which is called whenever the shared
 *                 message indec minsize in file creation property list
 *                 is decoded.
 *
 * Return:	   Success:	Non-negative
 *		   Failure:	Negative
 *
 * Programmer:     Mohamad Chaarawi
 *                 August 7, 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__fcrt_shmsg_index_minsize_dec(const void **_pp, void *_value)
{
    unsigned *minsizes = (unsigned *)_value;
    const uint8_t **pp = (const uint8_t **)_pp;
    unsigned enc_size;                  /* Size of encoded property */
    unsigned u;                         /* Local index variable */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_STATIC

    /* Sanity checks */
    HDassert(pp);
    HDassert(*pp);
    HDassert(minsizes);

    /* Decode the size */
    enc_size = *(*pp)++;
    if(enc_size != sizeof(unsigned))
        HGOTO_ERROR(H5E_PLIST, H5E_BADVALUE, FAIL, "unsigned value can't be decoded")

    /* Decode all the minsize values */
    for(u = 0 ; u < H5O_SHMESG_MAX_NINDEXES; u++)
        H5_DECODE_UNSIGNED(*pp, minsizes[u])

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P__fcrt_shmsg_index_minsize_dec() */


/*-------------------------------------------------------------------------
 * Function:	H5Pset_shared_mesg_phase_change
 *
 * Purpose:	Sets the cutoff values for indexes storing shared object
 *              header messages in this file.  If more than max_list
 *              messages are in an index, that index will become a B-tree.
 *              Likewise, a B-tree index containing fewer than min_btree
 *              messages will be converted to a list.
 *
 *              If the max_list is zero then SOHM indexes in this file will
 *              never be lists but will be created as B-trees.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	James Laird
 *		Wednesday, April 5, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_shared_mesg_phase_change(hid_t plist_id, unsigned max_list, unsigned min_btree)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "iIuIu", plist_id, max_list, min_btree);

    /* Check that values are sensible.  The min_btree value must be no greater
     * than the max list plus one.
     *
     * Range check to make certain they will fit into encoded form.
     */
    if(max_list + 1 < min_btree)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "minimum B-tree value is greater than maximum list value")
    if(max_list > H5O_SHMESG_MAX_LIST_SIZE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "max list value is larger than H5O_SHMESG_MAX_LIST_SIZE")
    if(min_btree > H5O_SHMESG_MAX_LIST_SIZE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "min btree value is larger than H5O_SHMESG_MAX_LIST_SIZE")

    /* Avoid the strange case where max_list == 0 and min_btree == 1, so deleting the
     * last message in a B-tree makes it become an empty list.
     */
    if(max_list == 0)
        min_btree = 0;

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    if(H5P_set(plist, H5F_CRT_SHMSG_LIST_MAX_NAME, &max_list) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set list maximum in property list");
    if(H5P_set(plist, H5F_CRT_SHMSG_BTREE_MIN_NAME, &min_btree) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set B-tree minimum in property list");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_shared_mesg_phase_change() */


/*-------------------------------------------------------------------------
 * Function:	H5Pget_shared_mesg_phase_change
 *
 * Purpose:	Gets the maximum size of a SOHM list index before it becomes
 *              a B-tree.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	James Laird
 *		Wednesday, April 5, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_shared_mesg_phase_change(hid_t plist_id, unsigned *max_list, unsigned *min_btree)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    herr_t      ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "i*Iu*Iu", plist_id, max_list, min_btree);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID");

    /* Get value */
    if (max_list) {
        if(H5P_get(plist, H5F_CRT_SHMSG_LIST_MAX_NAME, max_list) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get list maximum");
    }
    if (min_btree) {
        if(H5P_get(plist, H5F_CRT_SHMSG_BTREE_MIN_NAME, min_btree) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get SOHM information");
    }

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_shared_mesg_phase_change() */


/*-------------------------------------------------------------------------
 * Function:	H5Pset_file_space
 *
 * Purpose:	Sets the strategy that the library employs in managing file space.
 *		If strategy is zero, the property is not changed; the existing
 *			strategy is retained.
 *		Sets the threshold value that the file's free space
 *			manager(s) will use to track free space sections.
 *		If threshold is zero, the property is not changed; the existing
 *			threshold is retained.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Vailin Choi; June 10, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_file_space(hid_t plist_id, H5F_file_space_type_t strategy, hsize_t threshold)
{
    H5P_genplist_t *plist;              /* Property list pointer */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "iFfh", plist_id, strategy, threshold);

    if((unsigned)strategy >= H5F_FILE_SPACE_NTYPES)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid strategy")

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Set value(s), if non-zero */
    if(strategy)
	if(H5P_set(plist, H5F_CRT_FILE_SPACE_STRATEGY_NAME, &strategy) < 0)
	    HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set file space strategy")
    if(threshold)
	if(H5P_set(plist, H5F_CRT_FREE_SPACE_THRESHOLD_NAME, &threshold) < 0)
	    HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set free-space threshold")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Pset_file_space() */


/*-------------------------------------------------------------------------
 * Function:	H5Pget_file_space
 *
 * Purpose:	Retrieves the strategy that the library uses in managing file space.
 *		Retrieves the threshold value that the file's free space
 *			managers use to track free space sections.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Vailin Choi; June 10, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_file_space(hid_t plist_id, H5F_file_space_type_t *strategy, hsize_t *threshold)
{
    H5P_genplist_t *plist;              /* Property list pointer */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "i*Ff*h", plist_id, strategy, threshold);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id,H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get value(s) */
    if(strategy)
        if(H5P_get(plist, H5F_CRT_FILE_SPACE_STRATEGY_NAME, strategy) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get file space strategy")
    if(threshold)
        if(H5P_get(plist, H5F_CRT_FREE_SPACE_THRESHOLD_NAME, threshold) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get free-space threshold")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Pget_file_space() */


/*-------------------------------------------------------------------------
 * Function:    H5Pget_avoid_truncate
 *
 * Purpose:     Gets whether or not the library avoids truncation calls
 *              during file close.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Mike McGreevy
 *              January 26, 2011
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_avoid_truncate(hid_t fcpl_id, H5F_avoid_truncate_t *avoid_truncate)
{
    H5P_genplist_t  *plist;         /* Property list pointer */
    herr_t  ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*Fa", fcpl_id, avoid_truncate);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(fcpl_id, H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get the avoid truncate value */
    if(H5P_get(plist, H5F_CRT_AVOID_TRUNCATE_NAME, avoid_truncate) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get 'avoid truncate' property")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Pget_avoid_truncate() */


/*-------------------------------------------------------------------------
 * Function:    H5Pset_avoid_truncate
 *
 * Purpose:     Sets whether or not the library avoids truncation calls 
 *              during file close.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Mike McGreevy
 *              January 26, 2011
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_avoid_truncate(hid_t fcpl_id, H5F_avoid_truncate_t avoid_truncate)
{
    H5P_genplist_t  *plist;         /* Property list pointer */
    herr_t  ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "iFa", fcpl_id, avoid_truncate);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(fcpl_id, H5P_FILE_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Set the avoid_truncate value */
    if(H5P_set(plist, H5F_CRT_AVOID_TRUNCATE_NAME, &avoid_truncate) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set 'avoid truncate' property")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Pset_avoid_truncate() */


/*-------------------------------------------------------------------------
 * Function:       H5P__fcrt_avoid_truncate_enc
 *
 * Purpose:        Callback routine which is called whenever the file's avoid
 *                 truncate property in the file create property list
 *                 is encoded.
 *
 * Return:	   Success:	Non-negative
 *		   Failure:	Negative
 *
 * Programmer:     Mike McGreevy
 *                 January 26, 2011
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__fcrt_avoid_truncate_enc(const void *value, void **_pp, size_t *size)
{
    const H5F_avoid_truncate_t *avoid_truncate = (const H5F_avoid_truncate_t *)value; /* Create local alias for values */
    uint8_t **pp = (uint8_t **)_pp;

    FUNC_ENTER_STATIC_NOERR

    /* Sanity check */
    HDassert(avoid_truncate);
    HDassert(size);

    if(NULL != *pp)
        /* Encode avoid truncate */
        *(*pp)++ = (uint8_t)*avoid_truncate;

    /* Size of avoid truncate */
    (*size)++;

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5P__fcrt_avoid_truncate_enc() */


/*-------------------------------------------------------------------------
 * Function:       H5P__fcrt_avoid_truncate_dec
 *
 * Purpose:        Callback routine which is called whenever the file's avoid
 *                 truncate property in the file access property list
 *                 is decoded.
 *
 * Return:	   Success:	Non-negative
 *		   Failure:	Negative
 *
 * Programmer:     Mike McGreevy
 *                 January 26, 2011
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__fcrt_avoid_truncate_dec(const void **_pp, void *_value)
{
    H5F_avoid_truncate_t *avoid_truncate = (H5F_avoid_truncate_t *)_value;            /* Avoid truncate */
    const uint8_t **pp = (const uint8_t **)_pp;

    FUNC_ENTER_STATIC_NOERR

    /* Sanity checks */
    HDassert(pp);
    HDassert(*pp);
    HDassert(avoid_truncate);

    /* Decode avoid truncate */
    *avoid_truncate = (H5F_avoid_truncate_t)*(*pp)++;

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5P__fcrt_avoid_truncate_dec() */

