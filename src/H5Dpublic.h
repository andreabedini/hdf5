/****************************************************************************
 * NCSA HDF								    *
 * Software Development Group						    *
 * National Center for Supercomputing Applications			    *
 * University of Illinois at Urbana-Champaign				    *
 * 605 E. Springfield, Champaign IL 61820				    *
 *									    *
 * For conditions of distribution and use, see the accompanying		    *
 * hdf/COPYING file.							    *
 *									    *
 ****************************************************************************/

/*
 * This file contains public declarations for the H5D module.
 */
#ifndef _H5Dpublic_H
#define _H5Dpublic_H

/* Public headers needed by this file */
#include "H5public.h"
#include "H5Ipublic.h"

/* Values for the H5D_LAYOUT property */
typedef enum H5D_layout_t {
    H5D_LAYOUT_ERROR	= -1,

    H5D_COMPACT		= 0,	/*raw data is very small		     */
    H5D_CONTIGUOUS	= 1,	/*the default				     */
    H5D_CHUNKED		= 2,	/*slow and fancy			     */
    H5D_NLAYOUTS	= 3	/*this one must be last!		     */
} H5D_layout_t;

/* Values for the space allocation time property */
typedef enum H5D_alloc_time_t {
    H5D_ALLOC_TIME_ERROR	=-1,
    H5D_ALLOC_TIME_DEFAULT  	=0,
    H5D_ALLOC_TIME_EARLY	=1,
    H5D_ALLOC_TIME_LATE	=2,
    H5D_ALLOC_TIME_INCR	=3
} H5D_alloc_time_t;   

/* Values for the status of space allocation */
typedef enum H5D_space_status_t {
    H5D_SPACE_STATUS_ERROR	=-1,
    H5D_SPACE_STATUS_NOT_ALLOCATED	=0,
    H5D_SPACE_STATUS_PART_ALLOCATED	=1,
    H5D_SPACE_STATUS_ALLOCATED		=2
} H5D_space_status_t;

/* Values for time of writing fill value property */
typedef enum H5D_fill_time_t {
    H5D_FILL_TIME_ERROR	=-1,
    H5D_FILL_TIME_ALLOC =0,
    H5D_FILL_TIME_NEVER	=1
} H5D_fill_time_t;

/* Values for fill value status */
typedef enum H5D_fill_value_t {
    H5D_FILL_VALUE_ERROR        =-1,
    H5D_FILL_VALUE_UNDEFINED    =0,
    H5D_FILL_VALUE_DEFAULT      =1,
    H5D_FILL_VALUE_USER_DEFINED =2
} H5D_fill_value_t;

/* Define the operator function pointer for H5Diterate() */
typedef herr_t (*H5D_operator_t)(void *elem, hid_t type_id, hsize_t ndim,
				 hssize_t *point, void *operator_data);

#ifdef __cplusplus
extern "C" {
#endif

__DLL__ hid_t H5Dcreate (hid_t file_id, const char *name, hid_t type_id,
			 hid_t space_id, hid_t plist_id);
__DLL__ hid_t H5Dopen (hid_t file_id, const char *name);
__DLL__ herr_t H5Dclose (hid_t dset_id);
__DLL__ hid_t H5Dget_space (hid_t dset_id);
__DLL__ herr_t H5Dget_space_status(hid_t dset_id, 
				H5D_space_status_t *allocation);
__DLL__ hid_t H5Dget_type (hid_t dset_id);
__DLL__ hid_t H5Dget_create_plist (hid_t dset_id);
__DLL__ hsize_t H5Dget_storage_size(hid_t dset_id);
__DLL__ herr_t H5Dread (hid_t dset_id, hid_t mem_type_id, hid_t mem_space_id,
			hid_t file_space_id, hid_t plist_id, void *buf/*out*/);
__DLL__ herr_t H5Dwrite (hid_t dset_id, hid_t mem_type_id, hid_t mem_space_id,
			 hid_t file_space_id, hid_t plist_id, const void *buf);
__DLL__ herr_t H5Dextend (hid_t dset_id, const hsize_t *size);
__DLL__ herr_t H5Diterate(void *buf, hid_t type_id, hid_t space_id,
            H5D_operator_t op, void *operator_data);
__DLL__ herr_t H5Dvlen_reclaim(hid_t type_id, hid_t space_id, hid_t plist_id, void *buf);
__DLL__ herr_t H5Dvlen_get_buf_size(hid_t dataset_id, hid_t type_id, hid_t space_id, hsize_t *size);
__DLL__ herr_t H5Dfill(const void *fill, hid_t fill_type, void *buf,
        hid_t buf_type, hid_t space);
__DLL__ herr_t H5Ddebug(hid_t dset_id, unsigned int flags);
__DLL__ herr_t H5Dset_extent (hid_t dset_id, const hsize_t *size);


#ifdef __cplusplus
}
#endif
#endif
