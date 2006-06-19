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
 * Created:		H5FSprivate.h
 *			May  2 2006
 *			Quincey Koziol <koziol@ncsa.uiuc.edu>
 *
 * Purpose:		Private header for library accessible file free space routines.
 *
 *-------------------------------------------------------------------------
 */

#ifndef _H5FSprivate_H
#define _H5FSprivate_H

/* Include package's public header */
#include "H5FSpublic.h"

/* Private headers needed by this file */
#include "H5Fprivate.h"		/* File access				*/
#include "H5FLprivate.h"	/* Free Lists                           */
#include "H5SLprivate.h"	/* Skip lists				*/

/**************************/
/* Library Private Macros */
/**************************/

/* Flags for H5FS_add() */
#define H5FS_ADD_DESERIALIZING  0x01    /* Free space is being deserialized
                                         *      (for package use only)
                                         */
#define H5FS_ADD_RETURNED_SPACE 0x02    /* Section was previously allocated
                                         *      and is being returned to the
                                         *      free space manager (usually
                                         *      as a result of freeing an
                                         *      object)
                                         */


/****************************/
/* Library Private Typedefs */
/****************************/

/* Free space info (forward decl - defined in H5FSpkg.h) */
typedef struct H5FS_t H5FS_t;

/* Forward declaration free space section info */
typedef struct H5FS_section_info_t H5FS_section_info_t;

/* Free space section class info */
typedef struct H5FS_section_class_t {
    /* Class variables */
    const unsigned type;                        /* Type of free space section */
    size_t serial_size;                         /* Size of serialized form of section */

    /* Class methods */
    herr_t (*init_cls)(struct H5FS_section_class_t *, const void *);    /* Routine to initialize class-specific settings */

    /* Object methods */
    herr_t (*serialize)(const H5FS_section_info_t *, uint8_t *);        /* Routine to serialize a "live" section into a buffer */
    H5FS_section_info_t *(*deserialize)(const uint8_t *, haddr_t, hsize_t);     /* Routine to deserialize a buffer into a "live" section */
    htri_t (*can_merge)(H5FS_section_info_t *, H5FS_section_info_t *, void *);  /* Routine to determine if two nodes are mergable */
    herr_t (*merge)(H5FS_section_info_t *, H5FS_section_info_t *, void *);      /* Routine to merge two nodes */
    htri_t (*can_shrink)(H5FS_section_info_t *, void *);        /* Routine to determine if node can shrink container */
    herr_t (*shrink)(H5FS_section_info_t **, void *);   /* Routine to shrink container */
    herr_t (*free)(H5FS_section_info_t *);              /* Routine to free node */
    herr_t (*debug)(const H5FS_section_info_t *, FILE *, int , int );   /* Routine to dump debugging information about a section */
} H5FS_section_class_t;

/* State of section ("live" or "serialized") */
typedef enum H5FS_section_state_t {
    H5FS_SECT_LIVE,             /* Section has "live" memory references */
    H5FS_SECT_SERIALIZED        /* Section is in "serialized" form */
} H5FS_section_state_t;

/* Free space section info */
struct H5FS_section_info_t {
    haddr_t     addr;                   /* Address of free space section in the address space */
    hsize_t     size;                   /* Size of free space section */
    unsigned    type;                   /* Type of free space section (i.e. class) */
    H5FS_section_state_t state;         /* Whether the section is in "serialized" or "live" form */
};

/* Free space client IDs for identifying user of free space */
typedef enum H5FS_client_t {
    H5FS_CLIENT_FHEAP_ID = 0,	/* Free space is used by fractal heap */
    H5FS_NUM_CLIENT_ID          /* Number of free space client IDs (must be last)   */
} H5FS_client_t;

/* Free space creation parameters */
typedef struct H5FS_create_t {
    H5FS_client_t client;               /* Client's ID */
    unsigned shrink_percent;            /* Percent of "normal" serialized size to shrink serialized space at */
    unsigned expand_percent;            /* Percent of "normal" serialized size to expand serialized space at */
    unsigned max_sect_addr;             /* Size of address space free sections are within (log2 of actual value) */
    hsize_t max_sect_size;              /* Maximum size of section to track */
} H5FS_create_t;

/* Typedef for iteration operations */
typedef herr_t (*H5FS_operator_t)(const H5FS_section_info_t *sect,
        void *operator_data/*in,out*/);


/*****************************/
/* Library-private Variables */
/*****************************/
 
/* Declare a free list to manage the H5FS_section_class_t sequence information */
H5FL_SEQ_EXTERN(H5FS_section_class_t);


/***************************************/
/* Library-private Function Prototypes */
/***************************************/
H5_DLL H5FS_t *H5FS_create(H5F_t *f, hid_t dxpl_id, haddr_t *fs_addr,
    const H5FS_create_t *fs_create, size_t nclasses,
    const H5FS_section_class_t *classes[], const void *cls_init_udata);
H5_DLL H5FS_t *H5FS_open(H5F_t *f, hid_t dxpl_id, haddr_t fs_addr,
    size_t nclasses, const H5FS_section_class_t *classes[], const void *cls_init_udata);
H5_DLL herr_t H5FS_add(H5F_t *f, hid_t dxpl_id, H5FS_t *fspace,
    H5FS_section_info_t *node, unsigned flags, void *op_data);
H5_DLL htri_t H5FS_find(H5F_t *f, hid_t dxpl_id, H5FS_t *fspace,
    hsize_t request, H5FS_section_info_t **node);
H5_DLL herr_t H5FS_iterate(H5FS_t *fspace, H5FS_operator_t op, void *op_data);
H5_DLL herr_t H5FS_get_sect_count(const H5FS_t *fspace, hsize_t *nsects);
H5_DLL herr_t H5FS_flush(H5F_t *f, hid_t dxpl_id, unsigned flags);
H5_DLL herr_t H5FS_delete(H5F_t *f, hid_t dxpl_id, haddr_t fs_addr);
H5_DLL herr_t H5FS_close(H5F_t *f, hid_t dxpl_id, H5FS_t *fspace);

/* Debugging routines for dumping file structures */
H5_DLL herr_t H5FS_sect_debug(const H5FS_t *fspace, const H5FS_section_info_t *sect,
    FILE *stream, int indent, int fwidth);

#endif /* _H5FSprivate_H */

