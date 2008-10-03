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
 * This file contains private information about the H5D module
 */
#ifndef _H5Dprivate_H
#define _H5Dprivate_H

/* Include package's public header */
#include "H5Dpublic.h"

/* Private headers needed by this file */
#include "H5FDprivate.h"	/* File drivers				*/
#include "H5Oprivate.h"		/* Object headers		  	*/
#include "H5Sprivate.h"		/* Dataspaces 				*/
#include "H5Zprivate.h"		/* Data filters				*/

/**************************/
/* Library Private Macros */
/**************************/

/*
 * Feature: Define H5D_DEBUG on the compiler command line if you want to
 *	    debug dataset I/O. NDEBUG must not be defined in order for this
 *	    to have any effect.
 */
#ifdef NDEBUG
#  undef H5D_DEBUG
#endif

/* ========  Dataset creation property names ======== */
#define H5D_CRT_LAYOUT_NAME        "layout"             /* Storage layout */
#define H5D_CRT_CHUNK_DIM_NAME     "chunk_ndims"        /* Chunk dimensionality */
#define H5D_CRT_CHUNK_SIZE_NAME    "chunk_size"         /* Chunk size */
#define H5D_CRT_FILL_VALUE_NAME    "fill_value"         /* Fill value */
#define H5D_CRT_ALLOC_TIME_STATE_NAME "alloc_time_state" /* Space allocation time state */
#define H5D_CRT_EXT_FILE_LIST_NAME "efl"                /* External file list */
#define H5D_CRT_DATA_PIPELINE_NAME "pline"              /* Data filter pipeline */

/* ======== Data transfer properties ======== */
#define H5D_XFER_MAX_TEMP_BUF_NAME      "max_temp_buf"  /* Maximum temp buffer size */
#define H5D_XFER_TCONV_BUF_NAME         "tconv_buf"     /* Type conversion buffer */
#define H5D_XFER_BKGR_BUF_NAME          "bkgr_buf"      /* Background buffer */
#define H5D_XFER_BKGR_BUF_TYPE_NAME     "bkgr_buf_type" /* Background buffer type */
#define H5D_XFER_BTREE_SPLIT_RATIO_NAME "btree_split_ratio" /* B-tree node splitting ratio */
#define H5D_XFER_VLEN_ALLOC_NAME        "vlen_alloc"    /* Vlen allocation function */
#define H5D_XFER_VLEN_ALLOC_INFO_NAME   "vlen_alloc_info" /* Vlen allocation info */
#define H5D_XFER_VLEN_FREE_NAME         "vlen_free"     /* Vlen free function */
#define H5D_XFER_VLEN_FREE_INFO_NAME    "vlen_free_info" /* Vlen free info */
#define H5D_XFER_VLEN_CONV_NAME         "vlen_conv"     /* whether vlen conversion is needed */ 
#define H5D_XFER_VFL_ID_NAME            "vfl_id"        /* File driver ID */
#define H5D_XFER_VFL_INFO_NAME          "vfl_info"      /* File driver info */
#define H5D_XFER_HYPER_VECTOR_SIZE_NAME "vec_size"      /* Hyperslab vector size */
#ifdef H5_HAVE_PARALLEL
#define H5D_XFER_IO_XFER_MODE_NAME      "io_xfer_mode"  /* I/O transfer mode */
#define H5D_XFER_IO_XFER_OPT_MODE_NAME  "io_xfer_opt_mode" /* I/O optimization transfer mode (using MPI-IO independent IO with file set view) */
#define H5D_XFER_MPIO_COLLECTIVE_OPT_NAME "mpio_collective_opt" /* Optimization of MPI-IO transfer mode */
#define H5D_XFER_MPIO_CHUNK_OPT_HARD_NAME "mpio_chunk_opt_hard"
#define H5D_XFER_MPIO_CHUNK_OPT_NUM_NAME "mpio_chunk_opt_num"
#define H5D_XFER_MPIO_CHUNK_OPT_RATIO_NAME "mpio_chunk_opt_ratio"
#endif /* H5_HAVE_PARALLEL */
#define H5D_XFER_EDC_NAME               "err_detect"    /* EDC */
#define H5D_XFER_FILTER_CB_NAME         "filter_cb"     /* Filter callback function */
#define H5D_XFER_CONV_CB_NAME           "type_conv_cb"  /* Type conversion callback function */
#define H5D_XFER_XFORM_NAME             "data_transform" /* Data transform */
#ifdef H5_HAVE_INSTRUMENTED_LIBRARY
/* Collective chunk instrumentation properties */
#define H5D_XFER_COLL_CHUNK_LINK_HARD_NAME "coll_chunk_link_hard"
#define H5D_XFER_COLL_CHUNK_MULTI_HARD_NAME "coll_chunk_multi_hard"
#define H5D_XFER_COLL_CHUNK_LINK_NUM_TRUE_NAME "coll_chunk_link_true"
#define H5D_XFER_COLL_CHUNK_LINK_NUM_FALSE_NAME "coll_chunk_link_false"
#define H5D_XFER_COLL_CHUNK_MULTI_RATIO_COLL_NAME "coll_chunk_multi_coll"
#define H5D_XFER_COLL_CHUNK_MULTI_RATIO_IND_NAME "coll_chunk_multi_ind"
/* Definitions for all collective chunk instrumentation properties */
#define H5D_XFER_COLL_CHUNK_SIZE        sizeof(unsigned)
#define H5D_XFER_COLL_CHUNK_DEF         1
#endif /* H5_HAVE_INSTRUMENTED_LIBRARY */

/* Default temporary buffer size */
#define H5D_TEMP_BUF_SIZE       (1024 * 1024)

/* Default I/O vector size */
#define H5D_IO_VECTOR_SIZE      1024

/* Default VL allocation & free info */
#define H5D_VLEN_ALLOC          NULL
#define H5D_VLEN_ALLOC_INFO     NULL
#define H5D_VLEN_FREE           NULL
#define H5D_VLEN_FREE_INFO      NULL

/****************************/
/* Library Private Typedefs */
/****************************/

/* Typedef for dataset in memory (defined in H5Dpkg.h) */
typedef struct H5D_t H5D_t;

/* Typedef for dataset storage information */
typedef struct {
    hsize_t index;          /* "Index" of chunk in dataset (must be first for TBBT routines) */
    hsize_t *offset;        /* Chunk's coordinates in elements */
    hssize_t linear_offset; /* Chunk's coordinates translated into one-dimensional offset */
} H5D_chunk_storage_t;

typedef struct {
    haddr_t dset_addr;      /* Address of dataset in file */
    hsize_t dset_size;      /* Total size of dataset in file */
} H5D_contig_storage_t;

typedef union H5D_storage_t {
    H5O_efl_t   efl;            /* External file list information for dataset */
    H5D_chunk_storage_t chunk;  /* Chunk information for dataset */
    H5D_contig_storage_t contig; /* Contiguous information for dataset */
} H5D_storage_t;

/* Typedef for cached dataset transfer property list information */
typedef struct H5D_dxpl_cache_t {
    size_t max_temp_buf;        /* Maximum temporary buffer size (H5D_XFER_MAX_TEMP_BUF_NAME) */
    void *tconv_buf;            /* Temporary conversion buffer (H5D_XFER_TCONV_BUF_NAME) */
    void *bkgr_buf;             /* Background conversion buffer (H5D_XFER_BKGR_BUF_NAME) */
    H5T_bkg_t bkgr_buf_type;    /* Background buffer type (H5D_XFER_BKGR_BUF_NAME) */
    H5Z_EDC_t err_detect;       /* Error detection info (H5D_XFER_EDC_NAME) */
    double btree_split_ratio[3];/* B-tree split ratios (H5D_XFER_BTREE_SPLIT_RATIO_NAME) */
    size_t vec_size;            /* Size of hyperslab vector (H5D_XFER_HYPER_VECTOR_SIZE_NAME) */
#ifdef H5_HAVE_PARALLEL
    H5FD_mpio_xfer_t xfer_mode; /* Parallel transfer for this request (H5D_XFER_IO_XFER_MODE_NAME) */
    H5FD_mpio_collective_opt_t xfer_opt_mode; /* Parallel transfer with independent IO or collective IO with this mode */
#endif /*H5_HAVE_PARALLEL*/
    H5Z_cb_t filter_cb;         /* Filter callback function (H5D_XFER_FILTER_CB_NAME) */
    H5Z_data_xform_t *data_xform_prop; /* Data transform prop (H5D_XFER_XFORM_NAME) */
} H5D_dxpl_cache_t;

/* Typedef for cached dataset creation property list information */
typedef struct H5D_dcpl_cache_t {
    H5O_fill_t fill;            /* Fill value info (H5D_CRT_FILL_VALUE_NAME) */
    H5O_pline_t pline;          /* I/O pipeline info (H5D_CRT_DATA_PIPELINE_NAME) */
    H5O_efl_t efl;              /* External file list info (H5D_CRT_EXT_FILE_LIST_NAME) */
} H5D_dcpl_cache_t;

/*****************************/
/* Library Private Variables */
/*****************************/

/******************************/
/* Library Private Prototypes */
/******************************/

H5_DLL herr_t H5D_init(void);
H5_DLL H5D_t *H5D_open(const H5G_loc_t *loc, hid_t dxpl_id);
H5_DLL herr_t H5D_close(H5D_t *dataset);
H5_DLL H5O_loc_t *H5D_oloc(H5D_t *dataset);
H5_DLL H5G_name_t *H5D_nameof(H5D_t *dataset);
H5_DLL H5T_t *H5D_typeof(const H5D_t *dset);
H5_DLL herr_t H5D_flush(const H5F_t *f, hid_t dxpl_id, unsigned flags);
H5_DLL herr_t H5D_get_dxpl_cache(hid_t dxpl_id, H5D_dxpl_cache_t **cache);
H5_DLL herr_t H5D_get_dxpl_cache_real(hid_t dxpl_id, H5D_dxpl_cache_t *cache);
/*H5_DLL herr_t H5D_init_type(H5F_t *file, const H5D_t *dset, hid_t type_id,
    const H5T_t *type);
H5_DLL hsize_t H5D_get_storage_size(H5D_t *dset, hid_t dxpl_id);*/
H5_DLL herr_t H5D_modify_dtype(H5F_t *file, H5D_t *dataset, hid_t type_id, H5T_t *type, 
    hid_t dxpl_id);

/* Functions that operate on vlen data */
H5_DLL herr_t H5D_vlen_reclaim(hid_t type_id, H5S_t *space, hid_t plist_id,
    void *buf);

/* Functions that operate on contiguous storage */
H5_DLL herr_t H5D_contig_delete(H5F_t *f, hid_t dxpl_id,
    const H5O_layout_t *layout);

/* Functions that operate on indexed storage */
H5_DLL herr_t H5D_istore_delete(H5F_t *f, hid_t dxpl_id,
    const H5O_layout_t *layout);
H5_DLL herr_t H5D_istore_debug(H5F_t *f, hid_t dxpl_id, haddr_t addr, FILE * stream,
				int indent, int fwidth, unsigned ndims);
H5_DLL hssize_t H5D_istore_calc_offset(H5S_t *space, hsize_t offset[]);

#endif /* _H5Dprivate_H */

