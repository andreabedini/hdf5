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

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "H5DOprivate.h"

/*-------------------------------------------------------------------------
 * Function:	H5DOwrite_chunk
 *
 * Purpose:     Writes an entire chunk to the file directly.	
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Raymond Lu
 *		30 July 2012
 *
 * Modifications:
 *-------------------------------------------------------------------------
 */
herr_t
H5DOwrite_chunk(hid_t dset_id, hid_t dxpl_id, uint32_t filters, const hsize_t *offset, 
         size_t data_size, const void *buf)
{
    hbool_t created_dxpl = FALSE;
    herr_t  ret_value = SUCCEED;  /* Return value */
    
    if(dset_id < 0) {
        ret_value = FAIL;
        goto done;
    }

    if(!buf) {
        ret_value = FAIL;
        goto done;
    }

    if(!offset) {
        ret_value = FAIL;
        goto done;
    }

    if(!data_size) {
        ret_value = FAIL;
        goto done;
    }

    if(H5P_DEFAULT == dxpl_id) {
	if((dxpl_id = H5Pcreate(H5P_DATASET_XFER)) < 0) {
            ret_value = FAIL;
            goto done;
        }

        created_dxpl = TRUE;
    }

    if(H5DO_write_chunk(dset_id, dxpl_id, filters, offset, data_size, buf) < 0) {
        ret_value = FAIL;
        goto done;
    }

done:
    if(created_dxpl) {
        if(H5Pclose(dxpl_id) < 0)
            ret_value = FAIL;
    }

    return ret_value;
}

/*-------------------------------------------------------------------------
 * Function:	H5DO_write_chunk
 *
 * Purpose:     Private function for H5DOwrite_chunk
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Raymond Lu
 *		30 July 2012
 *
 * Modifications:
 *-------------------------------------------------------------------------
 */
herr_t
H5DO_write_chunk(hid_t dset_id, hid_t dxpl_id, uint32_t filters, const hsize_t *offset, 
         size_t data_size, const void *buf)
{
    hbool_t do_direct_write = TRUE;
    herr_t  ret_value = SUCCEED;  /* Return value */

    if(H5Pset(dxpl_id, H5D_XFER_DIRECT_CHUNK_WRITE_FLAG_NAME, &do_direct_write) < 0) {
        ret_value = FAIL;
        goto done;
    }

    if(H5Pset(dxpl_id, H5D_XFER_DIRECT_CHUNK_WRITE_FILTERS_NAME, &filters) < 0) {
        ret_value = FAIL;
        goto done;
    }

    if(H5Pset(dxpl_id, H5D_XFER_DIRECT_CHUNK_WRITE_OFFSET_NAME, &offset) < 0) {
        ret_value = FAIL;
        goto done;
    }

    if(H5Pset(dxpl_id, H5D_XFER_DIRECT_CHUNK_WRITE_DATASIZE_NAME, &data_size) < 0) {
        ret_value = FAIL;
        goto done;
    }

    if(H5Dwrite(dset_id, 0, H5S_ALL, H5S_ALL, dxpl_id, buf) < 0) {
        ret_value = FAIL;
        goto done;
    }

done:
    do_direct_write = FALSE;
    if(H5Pset(dxpl_id, H5D_XFER_DIRECT_CHUNK_WRITE_FLAG_NAME, &do_direct_write) < 0)
        ret_value = FAIL;

    return ret_value;
}


/*
 * Function:	H5DOappend()
 *
 * Purpose:     To append elements to a dataset.
 *		axis: the dataset dimension (zero-based) for the append
 *		extension: the # of elements to append for the axis-th dimension
 *		memtype: the datatype
 *		buf: buffer with data for the append
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Vailin Choi; Jan 2014
 *
 * Note:
 * 	This routine is copied from the fast forward feature branch: features/hdf5_ff
 *	src/H5FF.c:H5DOappend() with the following modifications:
 * 	1) Remove and replace macro calls such as
 *		FUNC_ENTER_API, H5TRACE, HGOTO_ERROR 
 * 	   accordingly because hl does not have these macros
 *	2) Replace H5I_get_type() by H5Iget_type()
 *	3) Replace H5P_isa_class() by H5Pisa_class()
 *	4) Fix a bug in the following: replace extension by size[axis]
 *		if(extension < old_size) {
 *		  ret_value = FAIL;
 *		  goto done;
 *   		}
 */
herr_t
H5DOappend(hid_t dset_id, hid_t dxpl_id, unsigned axis, size_t extension, 
           hid_t memtype, const void *buf)
{

    hsize_t size[H5S_MAX_RANK];		/* The new size (after extension */
    hsize_t old_size=0; 		/* The size of the dimension to be extended */
    int ndims, i; 			/* Number of dimensions in dataspace */
    hid_t space_id = FAIL; 		/* Old file space */
    hid_t new_space_id = FAIL; 		/* New file space (after extension) */
    hid_t mem_space_id = FAIL; 		/* Memory space for data buffer */
    hsize_t nelmts; 			/* Number of elements in selection */
    hid_t dapl = FAIL;			/* Dataset access property list */

    hsize_t start[H5S_MAX_RANK];	/* H5Sselect_Hyperslab: starting offset */
    hsize_t count[H5S_MAX_RANK];	/* H5Sselect_hyperslab: # of blocks to select */
    hsize_t stride[H5S_MAX_RANK];	/* H5Sselect_hyperslab: # of elements to move when selecting */
    hsize_t block[H5S_MAX_RANK];	/* H5Sselect_hyperslab: # of elements in a block */

    hsize_t *boundary = NULL;		/* Boundary set in append flush property */
    H5D_append_cb_t append_cb;		/* Callback function set in append flush property */
    void *udata;			/* User data set in append flush property */
    hbool_t hit = FALSE;		/* Boundary is hit or not */
    hsize_t k;				/* Local index variable */
    herr_t ret_value = SUCCEED;		/* Return value */

    /* check arguments */
    if(H5I_DATASET != H5Iget_type(dset_id)) {
        ret_value = FAIL;
	goto done;
    }

    /* Get the default dataset transfer property list if the user didn't provide one */
    if(H5P_DEFAULT == dxpl_id)
        dxpl_id = H5P_DATASET_XFER_DEFAULT;
    else
	if(TRUE != H5Pisa_class(dxpl_id, H5P_DATASET_XFER)) {
	    ret_value = FAIL;
	    goto done;
	}

    /* Get the dataspace of the dataset */
    if(FAIL == (space_id = H5Dget_space(dset_id))) {
	ret_value = FAIL;
	goto done;
    }

    /* Get the rank of this dataspace */
    if((ndims = H5Sget_simple_extent_ndims(space_id)) < 0) {
	ret_value = FAIL;
	goto done;
    }

    /* Verify correct axis */
    if((int)axis >= ndims) {
	ret_value = FAIL;
	goto done;
    }

    /* Get the dimensions sizes of the dataspace */
    if(H5Sget_simple_extent_dims(space_id, size, NULL) < 0) {
	ret_value = FAIL;
	goto done;
    }

    /* Adjust the dimension size of the requested dimension, 
       but first record the old dimension size */
    old_size = size[axis];
    size[axis] += extension;
    if(size[axis] < old_size) {
	ret_value = FAIL;
	goto done;
    }

    /* Set the extent of the dataset to the new dimension */
    if(H5Dset_extent(dset_id, size) < 0) {
	ret_value = FAIL;
	goto done;
    }

    /* Get the new dataspace of the dataset */
    if(FAIL == (new_space_id = H5Dget_space(dset_id))) {
	ret_value = FAIL;
	goto done;
    }

    /* Select a hyperslab corresponding to the append operation */
    for(i=0 ; i<ndims ; i++) {
        start[i] = 0;
        stride[i] = 1;
        count[i] = size[i];
        block[i] = 1;
        if(i == (int)axis) {
            count[i] = extension;
            start[i] = old_size;
        }
    }
    if(FAIL == H5Sselect_hyperslab(new_space_id, H5S_SELECT_SET, start, stride, count, block)) {
	ret_value = FAIL;
	goto done;
    }

    /* The # of elemnts in the new extended dataspace */
    nelmts = H5Sget_select_npoints(new_space_id);

    /* create a memory space */
    mem_space_id = H5Screate_simple(1, &nelmts, NULL);

    /* Write the data */
    if(H5Dwrite(dset_id, memtype, mem_space_id, new_space_id, dxpl_id, buf) < 0) {
	ret_value = FAIL;
	goto done;
    }

    /* Obtain the dataset's access property list */
    if((dapl = H5Dget_access_plist(dset_id)) < 0) {
	ret_value = FAIL;
	goto done;
    }

    /* Allocate the boundary array */
    boundary = (hsize_t *)HDmalloc(ndims * sizeof(hsize_t));

    /* Retrieve the append flush property */
    if(H5Pget_append_flush(dapl, ndims, boundary, &append_cb, &udata) < 0) {
	ret_value = FAIL;
	goto done;
    }

    /* No boundary for this axis */
    if(boundary[axis] == 0)
	goto done;

    /* Determine whether a boundary is hit or not */
    for(k = start[axis]; k < size[axis]; k++)
	if(!((k + 1) % boundary[axis])) {
	    hit = TRUE;
	    break;
	}

    if(hit) { /* Hit the boundary */
	/* Invoke callback if there is one */
	if(append_cb && append_cb(dset_id, size, udata) < 0) {
	    ret_value = FAIL;
	    goto done;
	}
	/* Does a dataset flush */
	if(H5Dflush(dset_id) < 0) {
	    ret_value = FAIL;
	    goto done;
	}
    }

done:
    /* Close old dataspace */
    if(space_id != FAIL && H5Sclose(space_id) < 0)
	ret_value = FAIL;

    /* Close new dataspace */
    if(new_space_id != FAIL && H5Sclose(new_space_id) < 0)
	ret_value = FAIL;

    /* Close memory dataspace */
    if(mem_space_id != FAIL && H5Sclose(mem_space_id) < 0)
	ret_value = FAIL;

    /* Close the dataset access property list */
    if(dapl != FAIL && H5Pclose(dapl) < 0)
	ret_value = FAIL;

    if(boundary)
	HDfree(boundary);

    return ret_value;
} /* H5DOappend() */

