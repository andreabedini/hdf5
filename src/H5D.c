/****************************************************************************
* NCSA HDF								   *
* Software Development Group						   *
* National Center for Supercomputing Applications			   *
* University of Illinois at Urbana-Champaign				   *
* 605 E. Springfield, Champaign IL 61820				   *
*									   *
* For conditions of distribution and use, see the accompanying		   *
* hdf/COPYING file.							   *
*									   *
****************************************************************************/

#ifdef RCSID
static char		RcsId[] = "@(#)$Revision$";
#endif

/* $Id$ */

#include <H5private.h>		/* Generic Functions			*/
#include <H5Iprivate.h>		/* IDs			  		*/
#include <H5ACprivate.h>	/* Cache			  	*/
#include <H5Dprivate.h>		/* Dataset functions			*/
#include <H5Eprivate.h>		/* Error handling		  	*/
#include <H5Gprivate.h>		/* Group headers		  	*/
#include <H5HLprivate.h>	/* Name heap				*/
#include <H5MFprivate.h>	/* File space allocation header		*/
#include <H5MMprivate.h>	/* Memory management			*/
#include <H5Oprivate.h>		/* Object headers		  	*/
#include <H5Pprivate.h>		/* Property lists			*/
#include <H5Zprivate.h>		/* Data compression			*/

#define PABLO_MASK	H5D_mask

/*
 * Define this to be zero or one depending on whether the I/O pipeline should
 * be optimized.
 */
#define H5D_OPTIMIZE_PIPE 1

/*
 * A dataset is the following struct.
 */
struct H5D_t {
    H5G_entry_t		ent;		/*cached object header stuff	*/
    H5T_t		*type;		/*datatype of this dataset	*/
    H5S_t		*space;		/*dataspace of this dataset	*/
    H5D_create_t	*create_parms;	/*creation parameters		*/
    H5O_layout_t	layout;		/*data layout			*/
};

/* Default dataset creation property list */
const H5D_create_t	H5D_create_dflt = {
    H5D_CONTIGUOUS,		/* Layout				*/
    1,				/* Chunk dimensions			*/
    {1, 1, 1, 1, 1, 1, 1, 1,	/* Chunk size.	These default values....*/
     1, 1, 1, 1, 1, 1, 1, 1,	/*...are quite useless.	 Larger chunks..*/
     1, 1, 1, 1, 1, 1, 1, 1,	/*...produce fewer, but larger I/O......*/
     1, 1, 1, 1, 1, 1, 1, 1},	/*...requests.				*/

    /* External file list */
    {H5F_ADDR_UNDEF,		/* External file list heap address	*/
     0,				/*...slots allocated			*/
     0,				/*...slots used				*/
     NULL}, 			/*...slot array				*/

    /* Compression */
    {H5Z_NONE, 			/* No compression			*/
     0,				/*...flags				*/
     0, NULL}			/*...client data			*/
};

/* Default dataset transfer property list */
const H5D_xfer_t	H5D_xfer_dflt = {
    1024*1024,			/* Temporary buffer size		*/
    NULL,			/* Type conversion buffer or NULL	*/
    NULL, 			/* Background buffer or NULL		*/
    H5T_BKG_NO,			/* Type of background buffer needed	*/
#ifdef HAVE_PARALLEL
    H5D_XFER_INDEPENDENT,      	/* Independent data transfer      	*/
#endif
};

/* Interface initialization? */
static hbool_t interface_initialize_g = FALSE;
#define INTERFACE_INIT H5D_init_interface
static herr_t H5D_init_interface(void);
static void H5D_term_interface(void);


/*--------------------------------------------------------------------------
NAME
   H5D_init_interface -- Initialize interface-specific information
USAGE
    herr_t H5D_init_interface()
   
RETURNS
   SUCCEED/FAIL
DESCRIPTION
    Initializes any interface-specific data or routines.

--------------------------------------------------------------------------*/
static herr_t
H5D_init_interface(void)
{
    herr_t		    ret_value = SUCCEED;
    FUNC_ENTER(H5D_init_interface, FAIL);

    /* Initialize the atom group for the dataset IDs */
    if ((ret_value = H5I_init_group(H5_DATASET, H5I_DATASETID_HASHSIZE,
				    H5D_RESERVED_ATOMS,
				    (herr_t (*)(void *)) H5D_close)) != FAIL) {
	ret_value = H5_add_exit(H5D_term_interface);
    }
    FUNC_LEAVE(ret_value);
}


/*--------------------------------------------------------------------------
 NAME
    H5D_term_interface
 PURPOSE
    Terminate various H5D objects
 USAGE
    void H5D_term_interface()
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
    Release the atom group and any other resources allocated.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
     Can't report errors...
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
static void
H5D_term_interface(void)
{
    H5I_destroy_group(H5_DATASET);
}

/*-------------------------------------------------------------------------
 * Function:	H5Dcreate
 *
 * Purpose:	Creates a new dataset named NAME at LOC_ID, opens the
 *		dataset for access, and associates with that dataset constant
 *		and initial persistent properties including the type of each
 *		datapoint as stored in the file (TYPE_ID), the size of the
 *		dataset (SPACE_ID), and other initial miscellaneous
 *		properties (CREATE_PARMS_ID).
 *
 *		All arguments are copied into the dataset, so the caller is
 *		allowed to derive new types, data spaces, and creation
 *		parameters from the old ones and reuse them in calls to
 *		create other datasets.
 *
 * Return:	Success:	The object ID of the new dataset.  At this
 *				point, the dataset is ready to receive its
 *				raw data.  Attempting to read raw data from
 *				the dataset will probably return the fill
 *				value.	The dataset should be closed when
 *				the caller is no longer interested in it.
 *
 *		Failure:	FAIL
 *
 * Errors:
 *		ARGS	  BADTYPE	Not a data space. 
 *		ARGS	  BADTYPE	Not a dataset creation plist. 
 *		ARGS	  BADTYPE	Not a file. 
 *		ARGS	  BADTYPE	Not a type. 
 *		ARGS	  BADVALUE	No name. 
 *		DATASET	  CANTINIT	Can't create dataset. 
 *		DATASET	  CANTREGISTER	Can't register dataset. 
 *
 * Programmer:	Robb Matzke
 *		Wednesday, December  3, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dcreate(hid_t loc_id, const char *name, hid_t type_id, hid_t space_id,
	  hid_t create_parms_id)
{
    H5G_t		   *loc = NULL;
    H5T_t		   *type = NULL;
    H5S_t		   *space = NULL;
    H5D_t		   *new_dset = NULL;
    hid_t		    ret_value = FAIL;
    const H5D_create_t	   *create_parms = NULL;

    FUNC_ENTER(H5Dcreate, FAIL);

    /* Check arguments */
    if (NULL == (loc = H5G_loc(loc_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    }
    if (!name || !*name) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name");
    }
    if (H5_DATATYPE != H5I_group(type_id) ||
	NULL == (type = H5I_object(type_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a type");
    }
    if (H5_DATASPACE != H5I_group(space_id) ||
	NULL == (space = H5I_object(space_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data space");
    }
    if (create_parms_id >= 0) {
	if (H5P_DATASET_CREATE != H5Pget_class(create_parms_id) ||
	    NULL == (create_parms = H5I_object(create_parms_id))) {
	    HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL,
			  "not a dataset creation property list");
	}
    } else {
	create_parms = &H5D_create_dflt;
    }

    /* build and open the new dataset */
    if (NULL == (new_dset = H5D_create(loc, name, type, space,
				       create_parms))) {
	HRETURN_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL,
		      "unable to create dataset");
    }
    /* Register the new datatype and get an ID for it */
    if ((ret_value = H5I_register(H5_DATASET, new_dset)) < 0) {
	H5D_close(new_dset);
	HRETURN_ERROR(H5E_DATASET, H5E_CANTREGISTER, FAIL,
		      "unable to register dataset");
    }
    FUNC_LEAVE(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5Dopen
 *
 * Purpose:	Finds a dataset named NAME at LOC_ID, opens it, and returns
 *		its ID.	 The dataset should be close when the caller is no
 *		longer interested in it.
 *
 * Return:	Success:	A new dataset ID
 *
 *		Failure:	FAIL
 *
 * Errors:
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dopen(hid_t loc_id, const char *name)
{
    H5G_t	*loc = NULL;		/*location holding the dataset	*/
    H5D_t	*dataset = NULL;	/*the dataset			*/
    hid_t	ret_value = FAIL;

    FUNC_ENTER(H5Dopen, FAIL);

    /* Check args */
    if (NULL == (loc = H5G_loc(loc_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    }
    if (!name || !*name) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name");
    }
    
    /* Find the dataset */
    if (NULL == (dataset = H5D_open(loc, name))) {
	HRETURN_ERROR(H5E_DATASET, H5E_NOTFOUND, FAIL, "dataset not found");
    }
    
    /* Create an atom for the dataset */
    if ((ret_value = H5I_register(H5_DATASET, dataset)) < 0) {
	H5D_close(dataset);
	HRETURN_ERROR(H5E_DATASET, H5E_CANTREGISTER, FAIL,
		      "can't register dataset");
    }
    FUNC_LEAVE(ret_value);
}

/*-------------------------------------------------------------------------
 * Function:	H5Dclose
 *
 * Purpose:	Closes access to a dataset (DATASET_ID) and releases
 *		resources used by it. It is illegal to subsequently use that
 *		same dataset ID in calls to other dataset functions.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Errors:
 *		ARGS	  BADTYPE	Not a dataset. 
 *		DATASET	  CANTINIT	Can't free. 
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dclose(hid_t dataset_id)
{
    H5D_t		   *dataset = NULL;	/* dataset object to release */

    FUNC_ENTER(H5Dclose, FAIL);

    /* Check args */
    if (H5_DATASET != H5I_group(dataset_id) ||
	NULL == (dataset = H5I_object(dataset_id)) ||
	NULL == dataset->ent.file) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");
    }
    /*
     * Decrement the counter on the dataset.  It will be freed if the count
     * reaches zero.
     */
    if (H5I_dec_ref(dataset_id) < 0) {
	HRETURN_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "can't free");
    }
    FUNC_LEAVE(SUCCEED);
}

/*-------------------------------------------------------------------------
 * Function:	H5Dget_space
 *
 * Purpose:	Returns a copy of the file data space for a dataset.
 *
 * Return:	Success:	ID for a copy of the data space.  The data
 *				space should be released by calling
 *				H5Sclose().
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January 28, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_space (hid_t dataset_id)
{
    H5D_t	*dataset = NULL;
    H5S_t	*copied_space = NULL;
    hid_t	ret_value = FAIL;
    
    FUNC_ENTER (H5Dget_space, FAIL);

    /* Check args */
    if (H5_DATASET!=H5I_group (dataset_id) ||
	NULL==(dataset=H5I_object (dataset_id))) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");
    }

    /* Copy the data space */
    if (NULL==(copied_space=H5S_copy (dataset->space))) {
	HRETURN_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
		       "unable to copy the data space");
    }

    /* Create an atom */
    if ((ret_value=H5I_register (H5_DATASPACE, copied_space))<0) {
	HRETURN_ERROR (H5E_ATOM, H5E_CANTREGISTER, FAIL,
		       "unable to register data space");
    }

    FUNC_LEAVE (ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5Dget_type
 *
 * Purpose:	Returns a copy of the file data type for a dataset.
 *
 * Return:	Success:	ID for a copy of the data type.	 The data
 *				type should be released by calling
 *				H5Tclose().
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Tuesday, February  3, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_type (hid_t dataset_id)
{
    
    H5D_t	*dataset = NULL;
    H5T_t	*copied_type = NULL;
    hid_t	ret_value = FAIL;
    
    FUNC_ENTER (H5Dget_type, FAIL);

    /* Check args */
    if (H5_DATASET!=H5I_group (dataset_id) ||
	NULL==(dataset=H5I_object (dataset_id))) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");
    }

    /* Copy the data type */
    if (NULL==(copied_type=H5T_copy (dataset->type))) {
	HRETURN_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
		       "unable to copy the data type");
    }

    /* Create an atom */
    if ((ret_value=H5I_register (H5_DATATYPE, copied_type))<0) {
	HRETURN_ERROR (H5E_ATOM, H5E_CANTREGISTER, FAIL,
		       "unable to register data type");
    }

    FUNC_LEAVE (ret_value);
}

/*-------------------------------------------------------------------------
 * Function:	H5Dget_create_plist
 *
 * Purpose:	Returns a copy of the dataset creation property list.
 *
 * Return:	Success:	ID for a copy of the dataset creation
 *				property list.  The template should be
 *				released by calling H5Pclose().
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Tuesday, February  3, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_create_plist (hid_t dataset_id)
{
    H5D_t		*dataset = NULL;
    H5D_create_t	*copied_parms = NULL;
    hid_t		ret_value = FAIL;
    
    FUNC_ENTER (H5Dget_create_plist, FAIL);

    /* Check args */
    if (H5_DATASET!=H5I_group (dataset_id) ||
	NULL==(dataset=H5I_object (dataset_id))) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");
    }

    /* Copy the creation property list */
    if (NULL==(copied_parms=H5P_copy (H5P_DATASET_CREATE,
				      dataset->create_parms))) {
	HRETURN_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
		       "unable to copy the creation property list");
    }

    /* Create an atom */
    if ((ret_value=H5I_register ((H5I_group_t)(H5_TEMPLATE_0+
					       H5P_DATASET_CREATE),
				 copied_parms))<0) {
	HRETURN_ERROR (H5E_ATOM, H5E_CANTREGISTER, FAIL,
		       "unable to register creation property list");
    }

    FUNC_LEAVE (ret_value);
}

/*-------------------------------------------------------------------------
 * Function:	H5Dread
 *
 * Purpose:	Reads (part of) a DATASET from the file into application
 *		memory BUF. The part of the dataset to read is defined with
 *		MEM_SPACE_ID and FILE_SPACE_ID.	 The data points are
 *		converted from their file type to the MEM_TYPE_ID specified. 
 *		Additional miscellaneous data transfer properties can be
 *		passed to this function with the XFER_PARMS_ID argument.
 *
 *		The FILE_SPACE_ID can be the constant H5S_ALL which indicates
 *		that the entire file data space is to be referenced.
 *
 *		The MEM_SPACE_ID can be the constant H5S_ALL in which case
 *		the memory data space is the same as the file data space
 *		defined when the dataset was created.
 *
 *		The number of elements in the memory data space must match
 *		the number of elements in the file data space.
 *
 *		The XFER_PARMS_ID can be the constant H5P_DEFAULT in which
 *		case the default data transfer properties are used.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Errors:
 *		ARGS	  BADTYPE	Not a data space. 
 *		ARGS	  BADTYPE	Not a data type. 
 *		ARGS	  BADTYPE	Not a dataset. 
 *		ARGS	  BADTYPE	Not xfer parms. 
 *		ARGS	  BADVALUE	No output buffer. 
 *		DATASET	  READERROR	Can't read data. 
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dread(hid_t dataset_id, hid_t mem_type_id, hid_t mem_space_id,
	hid_t file_space_id, hid_t xfer_parms_id, void *buf/*out*/)
{
    H5D_t		   *dataset = NULL;
    const H5T_t		   *mem_type = NULL;
    const H5S_t		   *mem_space = NULL;
    const H5S_t		   *file_space = NULL;
    const H5D_xfer_t	   *xfer_parms = NULL;

    FUNC_ENTER(H5Dread, FAIL);

    /* check arguments */
    if (H5_DATASET != H5I_group(dataset_id) ||
	NULL == (dataset = H5I_object(dataset_id)) ||
	NULL == dataset->ent.file) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");
    }
    if (H5_DATATYPE != H5I_group(mem_type_id) ||
	NULL == (mem_type = H5I_object(mem_type_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data type");
    }
    if (H5S_ALL != mem_space_id) {
	if (H5_DATASPACE != H5I_group(mem_space_id) ||
	    NULL == (mem_space = H5I_object(mem_space_id))) {
	    HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data space");
	}
    }
    if (H5S_ALL != file_space_id) {
	if (H5_DATASPACE != H5I_group(file_space_id) ||
	    NULL == (file_space = H5I_object(file_space_id))) {
	    HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data space");
	}
    }
    if (H5P_DEFAULT == xfer_parms_id) {
	xfer_parms = &H5D_xfer_dflt;
    } else if (H5P_DATASET_XFER != H5Pget_class(xfer_parms_id) ||
	       NULL == (xfer_parms = H5I_object(xfer_parms_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not xfer parms");
    }
    if (!buf) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no output buffer");
    }

    /* read raw data */
    if (H5D_read(dataset, mem_type, mem_space, file_space, xfer_parms,
		 buf/*out*/) < 0) {
	HRETURN_ERROR(H5E_DATASET, H5E_READERROR, FAIL, "can't read data");
    }
    FUNC_LEAVE(SUCCEED);
}

/*-------------------------------------------------------------------------
 * Function:	H5Dwrite
 *
 * Purpose:	Writes (part of) a DATASET from application memory BUF to the
 *		file.  The part of the dataset to write is defined with the
 *		MEM_SPACE_ID and FILE_SPACE_ID arguments. The data points
 *		are converted from their current type (MEM_TYPE_ID) to their
 *		file data type.	 Additional miscellaneous data transfer
 *		properties can be passed to this function with the
 *		XFER_PARMS_ID argument.
 *
 *		The FILE_SPACE_ID can be the constant H5S_ALL which indicates
 *		that the entire file data space is to be referenced.
 *
 *		The MEM_SPACE_ID can be the constant H5S_ALL in which case
 *		the memory data space is the same as the file data space
 *		defined when the dataset was created.
 *
 *		The number of elements in the memory data space must match
 *		the number of elements in the file data space.
 *
 *		The XFER_PARMS_ID can be the constant H5P_DEFAULT in which
 *		case the default data transfer properties are used.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Errors:
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dwrite(hid_t dataset_id, hid_t mem_type_id, hid_t mem_space_id,
	 hid_t file_space_id, hid_t xfer_parms_id, const void *buf)
{
    H5D_t		   *dataset = NULL;
    const H5T_t		   *mem_type = NULL;
    const H5S_t		   *mem_space = NULL;
    const H5S_t		   *file_space = NULL;
    const H5D_xfer_t	   *xfer_parms = NULL;

    FUNC_ENTER(H5Dwrite, FAIL);

    /* check arguments */
    if (H5_DATASET != H5I_group(dataset_id) ||
	NULL == (dataset = H5I_object(dataset_id)) ||
	NULL == dataset->ent.file) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");
    }
    if (H5_DATATYPE != H5I_group(mem_type_id) ||
	NULL == (mem_type = H5I_object(mem_type_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data type");
    }
    if (H5S_ALL != mem_space_id) {
	if (H5_DATASPACE != H5I_group(mem_space_id) ||
	    NULL == (mem_space = H5I_object(mem_space_id))) {
	    HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data space");
	}
    }
    if (H5S_ALL != file_space_id) {
	if (H5_DATASPACE != H5I_group(file_space_id) ||
	    NULL == (file_space = H5I_object(file_space_id))) {
	    HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data space");
	}
    }
    if (H5P_DEFAULT == xfer_parms_id) {
	xfer_parms = &H5D_xfer_dflt;
    } else if (H5P_DATASET_XFER != H5Pget_class(xfer_parms_id) ||
	       NULL == (xfer_parms = H5I_object(xfer_parms_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not xfer parms");
    }
    if (!buf) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no output buffer");
    }

    /* write raw data */
    if (H5D_write(dataset, mem_type, mem_space, file_space, xfer_parms,
		  buf) < 0) {
	HRETURN_ERROR(H5E_DATASET, H5E_READERROR, FAIL, "can't write data");
    }
    FUNC_LEAVE(SUCCEED);
}

/*-------------------------------------------------------------------------
 * Function:	H5Dextend
 *
 * Purpose:	This function makes sure that the dataset is at least of size
 *		SIZE. The dimensionality of SIZE is the same as the data
 *		space of the dataset being changed.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Friday, January 30, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dextend (hid_t dataset_id, const hsize_t *size)
{
    H5D_t	*dataset = NULL;
    
    FUNC_ENTER (H5Dextend, FAIL);

    /* Check args */
    if (H5_DATASET!=H5I_group (dataset_id) ||
	NULL==(dataset=H5I_object (dataset_id))) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");
    }
    if (!size) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL, "no size specified");
    }

    /* Increase size */
    if (H5D_extend (dataset, size)<0) {
	HRETURN_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
		       "unable to extend dataset");
    }

    FUNC_LEAVE (SUCCEED);
}

/*-------------------------------------------------------------------------
 * Function:	H5D_create
 *
 * Purpose:	Creates a new dataset with name NAME in file F and associates
 *		with it a datatype TYPE for each element as stored in the
 *		file, dimensionality information or dataspace SPACE, and
 *		other miscellaneous properties CREATE_PARMS.  All arguments
 *		are deep-copied before being associated with the new dataset,
 *		so the caller is free to subsequently modify them without
 *		affecting the dataset.
 *
 * Return:	Success:	Pointer to a new dataset
 *
 *		Failure:	NULL
 *
 * Errors:
 *		DATASET	  CANTINIT	Can't update dataset header. 
 *		DATASET	  CANTINIT	Problem with the dataset name. 
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5D_t *
H5D_create(H5G_t *loc, const char *name, const H5T_t *type, const H5S_t *space,
	   const H5D_create_t *create_parms)
{
    H5D_t		*new_dset = NULL;
    H5D_t		*ret_value = NULL;
    intn		i, ndims;
    hsize_t		max_dim[H5O_LAYOUT_NDIMS];
    H5O_efl_t		*efl = NULL;
    H5F_t		*f = H5G_fileof (loc);

    FUNC_ENTER(H5D_create, NULL);

    /* check args */
    assert (f);
    assert (loc);
    assert (name && *name);
    assert (type);
    assert (space);
    assert (create_parms);
    if (H5Z_NONE!=create_parms->compress.method &&
	H5D_CHUNKED!=create_parms->layout) {
	HGOTO_ERROR (H5E_DATASET, H5E_BADVALUE, NULL,
		     "compression can only be used with chunked layout");
    }

    /* Initialize the dataset object */
    new_dset = H5MM_xcalloc(1, sizeof(H5D_t));
    H5F_addr_undef(&(new_dset->ent.header));
    new_dset->type = H5T_copy(type);
    new_dset->space = H5S_copy(space);
    new_dset->create_parms = H5P_copy (H5P_DATASET_CREATE, create_parms);
    efl = &(new_dset->create_parms->efl);

    /* Total raw data size */
    new_dset->layout.type = new_dset->create_parms->layout;
    new_dset->layout.ndims = H5S_get_ndims(space) + 1;
    assert((unsigned)(new_dset->layout.ndims) <= NELMTS(new_dset->layout.dim));
    new_dset->layout.dim[new_dset->layout.ndims-1] = H5T_get_size(type);

    switch (new_dset->create_parms->layout) {
    case H5D_CONTIGUOUS:
	/*
	 * The maximum size of the dataset cannot exceed the storage size.
	 * Also, only the slowest varying dimension of a simple data space
	 * can be extendible.
	 */
	if ((ndims=H5S_get_dims(space, new_dset->layout.dim, max_dim)) < 0) {
	    HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, NULL,
			"unable to initialize contiguous storage");
	}

    /* Don't go through all these checks for scalar dataspaces */
    if(ndims>0) {
        for (i=1; i<ndims; i++) {
            if (max_dim[i]>new_dset->layout.dim[i]) {
            HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
                     "only the first dimension can be extendible");
            }
        }
        if (efl->nused>0) {
            hsize_t max_points = H5S_get_npoints_max (space);
            hsize_t max_storage = H5O_efl_total_size (efl);

            if (H5S_UNLIMITED==max_points) {
            if (H5O_EFL_UNLIMITED!=max_storage) {
                HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
                     "unlimited data space but finite storage");
            }
            } else if (max_points * H5T_get_size (type) < max_points) {
            HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
                     "data space * type size overflowed");
            } else if (max_points * H5T_get_size (type) > max_storage) {
            HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
                     "data space size exceeds external storage size");
            }
        } else if (max_dim[0]>new_dset->layout.dim[0]) {
            HGOTO_ERROR (H5E_DATASET, H5E_UNSUPPORTED, NULL,
                 "extendible contiguous non-external dataset");
        }
    }
	break;

    case H5D_CHUNKED:
	/*
	 * Chunked storage allows any type of data space extension, so we
	 * don't even bother checking.
	 */
	if (new_dset->create_parms->chunk_ndims != H5S_get_ndims(space)) {
	    HGOTO_ERROR(H5E_DATASET, H5E_BADVALUE, NULL,
		   "dimensionality of chunks doesn't match the data space");
	}
	if (efl->nused>0) {
	    HGOTO_ERROR (H5E_DATASET, H5E_BADVALUE, NULL,
			 "external storage not supported with chunked layout");
	}
	for (i=0; i<new_dset->layout.ndims-1; i++) {
	    new_dset->layout.dim[i] = new_dset->create_parms->chunk_size[i];
	}
	break;

    default:
	HGOTO_ERROR(H5E_DATASET, H5E_UNSUPPORTED, NULL, "not implemented yet");
    }

    /* Create (open for write access) an object header */
    if (H5O_create(f, 96, &(new_dset->ent)) < 0) {
	HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, NULL,
		    "unable to create dataset object header");
    }

    /* Update the type and space header messages */
    if (H5O_modify(&(new_dset->ent), H5O_DTYPE, 0,
		   (H5O_FLAG_CONSTANT|H5O_FLAG_SHARED),
		   new_dset->type) < 0 ||
	H5S_modify(&(new_dset->ent), new_dset->space) < 0) {
	HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, NULL,
		    "unable to update type or space header messages");
    }

    /* Update the compression message */
    if (H5Z_NONE!=new_dset->create_parms->compress.method &&
	H5O_modify (&(new_dset->ent), H5O_COMPRESS, 0, H5O_FLAG_CONSTANT,
		    &(new_dset->create_parms->compress))<0) {
	HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
		     "unable to update compression header message");
    }
    
    /*
     * Initialize storage.  We assume that external storage is already
     * initialized by the caller, or at least will be before I/O is
     * performed.
     */
    if (0==efl->nused) {
	if (H5F_arr_create(f, &(new_dset->layout)) < 0) {
	    HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, NULL,
			"unable to initialize storage");
	}
    } else {
	H5F_addr_undef (&(new_dset->layout.addr));
    }

    /* Update layout message */
    if (H5O_modify (&(new_dset->ent), H5O_LAYOUT, 0, H5O_FLAG_CONSTANT,
		    &(new_dset->layout)) < 0) {
	HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
		     "unable to update layout message");
    }

    /* Update external storage message */
    if (efl->nused>0) {
	size_t heap_size = H5HL_ALIGN (1);
	for (i=0; i<efl->nused; i++) {
	    heap_size += H5HL_ALIGN (strlen (efl->slot[i].name)+1);
	}
	if (H5HL_create (f, heap_size, &(efl->heap_addr))<0 ||
	    (size_t)(-1)==H5HL_insert (f, &(efl->heap_addr), 1, "")) {
	    HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
			 "unable to create external file list name heap");
	}
	if (H5O_modify (&(new_dset->ent), H5O_EFL, 0, H5O_FLAG_CONSTANT,
			efl)<0) {
	    HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
			 "unable to update external file list message");
	}
    }

    /* Give the dataset a name */
    if (H5G_insert(loc, name, &(new_dset->ent)) < 0) {
	HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, NULL, "unable to name dataset");
    }

    /* Success */
    ret_value = new_dset;

  done:
    if (!ret_value && new_dset) {
	if (new_dset->type) H5T_close(new_dset->type);
	if (new_dset->space) H5S_close(new_dset->space);
	if (new_dset->create_parms) {
	    H5P_close (H5P_DATASET_CREATE, new_dset->create_parms);
	    new_dset->create_parms = NULL;
	}
	if (H5F_addr_defined(&(new_dset->ent.header))) {
	    H5O_close(&(new_dset->ent));
	}
	new_dset->ent.file = NULL;
	H5MM_xfree(new_dset);
    }
    FUNC_LEAVE(ret_value);
}

/*-------------------------------------------------------------------------
 * Function:	H5D_open
 *
 * Purpose:	Finds a dataset named NAME in file F and builds a descriptor
 *		for it, opening it for access.
 *
 * Return:	Success:	Pointer to a new dataset descriptor.
 *
 *		Failure:	NULL
 *
 * Errors:
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5D_t *
H5D_open(H5G_t *loc, const char *name)
{
    H5D_t	*dataset = NULL;	/*the dataset which was found	*/
    H5D_t	*ret_value = NULL;	/*return value			*/
    intn	i;
    H5F_t	*f = NULL;
    
    FUNC_ENTER(H5D_open, NULL);

    /* check args */
    assert (loc);
    assert (name && *name);
    
    f = H5G_fileof (loc);
    dataset = H5MM_xcalloc(1, sizeof(H5D_t));
    dataset->create_parms = H5P_copy (H5P_DATASET_CREATE, &H5D_create_dflt);
    H5F_addr_undef(&(dataset->ent.header));

    /* Open the dataset object */
    if (H5G_find(loc, name, NULL, &(dataset->ent)) < 0) {
	HGOTO_ERROR(H5E_DATASET, H5E_NOTFOUND, NULL, "not found");
    }
    if (H5O_open(&(dataset->ent)) < 0) {
	HGOTO_ERROR(H5E_DATASET, H5E_CANTOPENOBJ, NULL, "unable to open");
    }
    
    /* Get the type and space */
    if (NULL==(dataset->type=H5O_read(&(dataset->ent), H5O_DTYPE, 0, NULL)) ||
	NULL==(dataset->space=H5S_read(f, &(dataset->ent)))) {
	HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, NULL,
		    "unable to load type or space info from dataset header");
    }

    /* Get the optional compression message */
    if (NULL==H5O_read (&(dataset->ent), H5O_COMPRESS, 0,
			&(dataset->create_parms->compress))) {
	H5E_clear ();
	HDmemset (&(dataset->create_parms->compress), 0,
		  sizeof(dataset->create_parms->compress));
    }
    
    /*
     * Get the raw data layout info.  It's actually stored in two locations:
     * the storage message of the dataset (dataset->storage) and certain
     * values are copied to the dataset create plist so the user can query
     * them.
     */
    if (NULL==H5O_read(&(dataset->ent), H5O_LAYOUT, 0, &(dataset->layout))) {
	HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, NULL,
		    "unable to read data layout message");
    }
    switch (dataset->layout.type) {
    case H5D_CONTIGUOUS:
	dataset->create_parms->layout = H5D_CONTIGUOUS;
	break;

    case H5D_CHUNKED:
	/*
	 * Chunked storage.  The creation plist's dimension is one less than
	 * the chunk dimension because the chunk includes a dimension for the
	 * individual bytes of the data type.
	 */
	dataset->create_parms->layout = H5D_CHUNKED;
	dataset->create_parms->chunk_ndims = dataset->layout.ndims - 1;
	for (i = 0; i < dataset->layout.ndims - 1; i++) {
	    dataset->create_parms->chunk_size[i] = dataset->layout.dim[i];
	}
	break;

    default:
	HGOTO_ERROR(H5E_DATASET, H5E_UNSUPPORTED, NULL, "not implemented yet");
    }

    /* Get the external file list message, which might not exist */
    if (NULL==H5O_read (&(dataset->ent), H5O_EFL, 0,
			&(dataset->create_parms->efl)) &&
	!H5F_addr_defined (&(dataset->layout.addr))) {
	HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, NULL,
		     "storage address is undefined an no external file list");
    }

    /* Success */
    ret_value = dataset;

  done:
    if (!ret_value && dataset) {
	if (H5F_addr_defined(&(dataset->ent.header))) {
	    H5O_close(&(dataset->ent));
	}
	if (dataset->type) {
	    H5T_close(dataset->type);
	}
	if (dataset->space) {
	    H5S_close(dataset->space);
	}
	if (dataset->create_parms) {
	    H5P_close (H5P_DATASET_CREATE, dataset->create_parms);
	}
	dataset->ent.file = NULL;
	H5MM_xfree(dataset);
    }
    FUNC_LEAVE(ret_value);
}

/*-------------------------------------------------------------------------
 * Function:	H5D_close
 *
 * Purpose:	Insures that all data has been saved to the file, closes the
 *		dataset object header, and frees all resources used by the
 *		descriptor.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Errors:
 *		DATASET	  CANTINIT	Couldn't free the type or space,
 *					but the dataset was freed anyway. 
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_close(H5D_t *dataset)
{
    uintn		    free_failed;

    FUNC_ENTER(H5D_close, FAIL);

    /* check args */
    assert(dataset && dataset->ent.file);

    /*
     * Release dataset type and space - there isn't much we can do if one of
     * these fails, so we just continue.
     */
    free_failed = (H5T_close(dataset->type) < 0 ||
		   H5S_close(dataset->space) < 0 ||
		   H5P_close (H5P_DATASET_CREATE, dataset->create_parms)); 

    /* Close the dataset object */
    H5O_close(&(dataset->ent));

    /*
     * Free memory.  Before freeing the memory set the file pointer to NULL.
     * We always check for a null file pointer in other H5D functions to be
     * sure we're not accessing an already freed dataset (see the assert()
     * above).
     */
    dataset->ent.file = NULL;
    H5MM_xfree(dataset);

    if (free_failed) {
	HRETURN_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL,
		      "couldn't free the type or space, but the dataset was "
		      "freed anyway.");
    }
    FUNC_LEAVE(SUCCEED);
}

/*-------------------------------------------------------------------------
 * Function:	H5D_read
 *
 * Purpose:	Reads (part of) a DATASET into application memory BUF. See
 *		H5Dread() for complete details.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_read(H5D_t *dataset, const H5T_t *mem_type, const H5S_t *mem_space,
	 const H5S_t *file_space, const H5D_xfer_t *xfer_parms,
	 void *buf/*out*/)
{
    hsize_t		nelmts;			/*number of elements	*/
    size_t		smine_start;		/*strip mine start loc	*/
    size_t		smine_nelmts;		/*elements per strip	*/
    uint8		*tconv_buf = NULL;	/*data type conv buffer	*/
    uint8		*bkg_buf = NULL;	/*background buffer	*/
    H5T_conv_t		tconv_func = NULL;	/*conversion function	*/
    hid_t		src_id = -1, dst_id = -1;/*temporary type atoms */
    const H5S_conv_t	*sconv_func = NULL;	/*space conversion funcs*/
    H5S_number_t	numbering;		/*element numbering info*/
    H5T_cdata_t		*cdata = NULL;		/*type conversion data	*/
    herr_t		ret_value = FAIL;
    herr_t		status;
    size_t		src_type_size;		/*size of source type	*/
    size_t		dst_type_size;		/*size of destination type*/
    size_t		target_size;		/*desired buffer size	*/
    size_t		request_nelmts;		/*requested strip mine	*/
    H5T_bkg_t		need_bkg;		/*type of background buf*/
#ifdef H5T_DEBUG
    H5_timer_t		timer;
#endif
#ifdef HAVE_PARALLEL
    int	access_mode_saved = -1;
#endif

    FUNC_ENTER(H5D_read, FAIL);

    /* check args */
    assert(dataset && dataset->ent.file);
    assert(mem_type);
    assert(xfer_parms);
    assert(buf);
    if (!file_space) file_space = dataset->space;
    if (!mem_space) mem_space = file_space;
    nelmts = H5S_get_npoints(mem_space);

    /*
     * Locate the type conversion function and data space conversion
     * functions, and set up the element numbering information. If a data
     * type conversion is necessary then register data type atoms. Data type
     * conversion is necessary if the user has set the `need_bkg' to a high
     * enough value in xfer_parms since turning off data type conversion also
     * turns off background preservation.
     */
    if (nelmts!=H5S_get_npoints (file_space)) {
	HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL,
		     "src and dest data spaces have different sizes");
    }
    if (NULL == (tconv_func = H5T_find(dataset->type, mem_type,
				       xfer_parms->need_bkg, &cdata))) {
	HGOTO_ERROR(H5E_DATASET, H5E_UNSUPPORTED, FAIL,
		    "unable to convert between src and dest data types");
    } else if (H5T_conv_noop!=tconv_func) {
	if ((src_id=H5I_register(H5_DATATYPE, H5T_copy(dataset->type)))<0 ||
	    (dst_id=H5I_register(H5_DATATYPE, H5T_copy(mem_type)))<0) {
	    HGOTO_ERROR(H5E_DATASET, H5E_CANTREGISTER, FAIL,
			"unable to register types for conversion");
	}
    }
    if (NULL==(sconv_func=H5S_find (mem_space, file_space))) {
	HGOTO_ERROR (H5E_DATASET, H5E_UNSUPPORTED, FAIL,
		     "unable to convert from file to memory data space");
    }
	
#ifdef HAVE_PARALLEL
    /*
     * Check if collective data transfer requested.
     */
    if (xfer_parms->xfer_mode == H5D_XFER_COLLECTIVE){
	/* verify that the file can support collective access. */
	/* The check may not be necessarily since collective access */
	/* can always be simulated by independent access. */
	/* Nevertheless, must check driver is MPIO before using those */
	/* access_mode which exists only for MPIO case. */
	if (dataset->ent.file->shared->access_parms.driver == H5F_LOW_MPIO){
	    /* Supports only no conversion, type or space, for now. */
	    if (H5T_conv_noop==tconv_func &&
		NULL!=sconv_func->read) {
		/* 
		 * -AKC-
		 * "plant" the collective access mode into the file information
		 * so that the lower level mpio routines know to use collective
		 * access.
		 * This is not thread-safe, is a klutch for now.
		 * Should change all the I/O routines to pass along the xfer
		 * property list to the low level I/O for proper execution.
		 * Make it to work now.  Must fix it later.
		 * -AKC-
		 */
#ifdef AKC
		printf("%s: collective access requested\n", FUNC);
		printf("%s: current f->access_mode = %x\n", FUNC,
		    dataset->ent.file->shared->access_parms.u.mpio.access_mode);
#endif
		access_mode_saved = dataset->ent.file->shared->access_parms.u.mpio.access_mode;
		dataset->ent.file->shared->access_parms.u.mpio.access_mode = H5D_XFER_COLLECTIVE;
		status = (sconv_func->read)(dataset->ent.file, &(dataset->layout),
					     &(dataset->create_parms->compress),
					     &(dataset->create_parms->efl),
					     H5T_get_size (dataset->type), file_space,
                                             mem_space, buf/*out*/);
		if (status>=0) goto succeed;
		HGOTO_ERROR (H5E_DATASET, H5E_READERROR, FAIL,
		    "collective read failed");
	    }
	}
	HGOTO_ERROR (H5E_DATASET, H5E_UNSUPPORTED, FAIL,
		 "collective access not permissible");
    }
#endif /*HAVE_PARALLEL*/

    
    /*
     * If there is no type conversion then try reading directly into the
     * application's buffer.  This saves at least one mem-to-mem copy.
     */
    if (H5D_OPTIMIZE_PIPE &&
	H5T_conv_noop==tconv_func &&
	NULL!=sconv_func->read) {
	status = (sconv_func->read)(dataset->ent.file, &(dataset->layout),
				    &(dataset->create_parms->compress),
				    &(dataset->create_parms->efl),
				    H5T_get_size (dataset->type), file_space,
				    mem_space, buf/*out*/);
	if (status>=0) goto succeed;
#ifdef H5D_DEBUG
	fprintf (stderr, "H5D: data space conversion could not be optimized "
		 "for this case (using general method instead)\n");
#endif
	H5E_clear ();
    }
    
	
    /*
     * This is the general case.  Figure out the strip mine size.
     */
    src_type_size = H5T_get_size(dataset->type);
    dst_type_size = H5T_get_size(mem_type);
    target_size = xfer_parms->buf_size;
    request_nelmts = target_size / MAX(src_type_size, dst_type_size);
    if (request_nelmts<=0) {
	HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
		     "temporary buffer max size is too small");
    }
    if (sconv_func->init) {
	smine_nelmts = (sconv_func->init)(&(dataset->layout), mem_space,
					  file_space, request_nelmts,
					  &numbering/*out*/);
	if (smine_nelmts<=0) {
	    HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
			 "unable to initialize element numbering information");
	}
    } else {
	smine_nelmts = request_nelmts;
	HDmemset (&numbering, 0, sizeof numbering);
    }

    /*
     * Get a temporary buffer for type conversion unless the app has already
     * supplied one through the xfer properties. Instead of allocating a
     * buffer which is the exact size, we allocate the target size.  The
     * malloc() is usually less resource-intensive if we allocate/free the
     * same size over and over.
     */
    if (cdata->need_bkg) {
	need_bkg = MAX (cdata->need_bkg, xfer_parms->need_bkg);
    } else {
	need_bkg = H5T_BKG_NO; /*never needed even if app says yes*/
    }
    if (NULL==(tconv_buf=xfer_parms->tconv_buf)) {
	tconv_buf = H5MM_xmalloc (target_size);
    }
    if (need_bkg && NULL==(bkg_buf=xfer_parms->bkg_buf)) {
	bkg_buf = H5MM_xmalloc (smine_nelmts * dst_type_size);
    }

#ifdef H5D_DEBUG
    {
	/* Strip mine diagnostics.... */
	size_t buffer_size = smine_nelmts * MAX (src_type_size, dst_type_size);
	if (smine_nelmts<nelmts) {
	    fprintf (stderr, "H5D: strip mine");
	    if (smine_nelmts!=request_nelmts) {
		fprintf (stderr, " got %lu of %lu",
			 (unsigned long)smine_nelmts,
			 (unsigned long)request_nelmts);
	    }
	    if (buffer_size!=target_size) {
		fprintf (stderr, " (%1.1f%% of buffer)",
			 100.0*(double)buffer_size/(double)target_size);
	    }
	    fprintf (stderr, " %1.1f iterations\n",
		     (double)nelmts/(double)smine_nelmts);
	}
    }
#endif

    /* Start strip mining... */
    for (smine_start=0; smine_start<nelmts; smine_start+=smine_nelmts) {
	smine_nelmts = MIN (smine_nelmts, nelmts-smine_start);
	
	/*
	 * Gather the data from disk into the data type conversion
	 * buffer. Also gather data from application to background buffer
	 * if necessary.
	 */
	if ((sconv_func->fgath)(dataset->ent.file, &(dataset->layout),
				&(dataset->create_parms->compress),
				&(dataset->create_parms->efl), 
				H5T_get_size (dataset->type), file_space,
				&numbering, smine_start, smine_nelmts,
				tconv_buf/*out*/)!=smine_nelmts) {
	    HGOTO_ERROR(H5E_IO, H5E_READERROR, FAIL, "file gather failed");
	}
	if ((H5D_OPTIMIZE_PIPE && H5T_BKG_YES==need_bkg) ||
	    (!H5D_OPTIMIZE_PIPE && need_bkg)) {
	    if ((sconv_func->mgath)(buf, H5T_get_size (mem_type), mem_space,
				    &numbering, smine_start, smine_nelmts,
				    bkg_buf/*out*/)!=smine_nelmts) {
		HGOTO_ERROR (H5E_IO, H5E_READERROR, FAIL, "mem gather failed");
	    }
	}

	/*
	 * Perform data type conversion.
	 */
#ifdef H5T_DEBUG
	H5T_timer_begin (&timer, cdata);
#endif
	cdata->command = H5T_CONV_CONV;
	status = (tconv_func)(src_id, dst_id, cdata, smine_nelmts, tconv_buf,
			      bkg_buf);
#ifdef H5T_DEBUG
	H5T_timer_end (&timer, cdata, smine_nelmts);
#endif
	if (status<0) {
	    HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL,
			"data type conversion failed");
	}

	/*
	 * Scatter the data into memory.
	 */
	if ((sconv_func->mscat)(tconv_buf, H5T_get_size (mem_type), mem_space,
				&numbering, smine_start, smine_nelmts,
				buf/*out*/)<0) {
	    HGOTO_ERROR (H5E_IO, H5E_READERROR, FAIL, "scatter failed");
	}
    }
    
 succeed:
    ret_value = SUCCEED;
    
 done:
    if (src_id >= 0) H5I_dec_ref(src_id);
    if (dst_id >= 0) H5I_dec_ref(dst_id);
    if (tconv_buf && NULL==xfer_parms->tconv_buf) {
	H5MM_xfree(tconv_buf);
    }
    if (bkg_buf && NULL==xfer_parms->bkg_buf) {
	H5MM_xfree (bkg_buf);
    }
#ifdef HAVE_PARALLEL
    /*
     * Check if collective data transfer requested.
     * If so, need to restore the access mode.  Shouldnot needed.
     */
    if (xfer_parms->xfer_mode == H5D_XFER_COLLECTIVE){
	dataset->ent.file->shared->access_parms.u.mpio.access_mode = access_mode_saved;
    }
#endif /*HAVE_PARALLEL*/
    FUNC_LEAVE(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5D_write
 *
 * Purpose:	Writes (part of) a DATASET to a file from application memory
 *		BUF. See H5Dwrite() for complete details.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_write(H5D_t *dataset, const H5T_t *mem_type, const H5S_t *mem_space,
	  const H5S_t *file_space, const H5D_xfer_t *xfer_parms,
	  const void *buf)
{
    hsize_t		nelmts;			/*total number of elmts	*/
    size_t		smine_start;		/*strip mine start loc	*/
    size_t		smine_nelmts;		/*elements per strip	*/
    uint8		*tconv_buf = NULL;	/*data type conv buffer	*/
    uint8		*bkg_buf = NULL;	/*background buffer	*/
    H5T_conv_t		tconv_func = NULL;	/*conversion function	*/
    hid_t		src_id = -1, dst_id = -1;/*temporary type atoms */
    const H5S_conv_t	*sconv_func = NULL;	/*space conversion funcs*/
    H5S_number_t	numbering;		/*element numbering info*/
    H5T_cdata_t		*cdata = NULL;		/*type conversion data	*/
    herr_t		ret_value = FAIL, status;
    size_t		src_type_size;		/*size of source type	*/
    size_t		dst_type_size;		/*size of destination type*/
    size_t		target_size;		/*desired buffer size	*/
    size_t		request_nelmts;		/*requested strip mine	*/
    H5T_bkg_t		need_bkg;		/*type of background buf*/
#ifdef H5T_DEBUG
    H5_timer_t		timer;
#endif
#ifdef HAVE_PARALLEL
    int	access_mode_saved = -1;
#endif

    FUNC_ENTER(H5D_write, FAIL);

    /* check args */
    assert(dataset && dataset->ent.file);
    assert(mem_type);
    assert(xfer_parms);
    assert(buf);
    if (!file_space) file_space = dataset->space;
    if (!mem_space) mem_space = file_space;
    nelmts = H5S_get_npoints(mem_space);

    /*
     * Locate the type conversion function and data space conversion
     * functions, and set up the element numbering information. If a data
     * type conversion is necessary then register data type atoms. Data type
     * conversion is necessary if the user has set the `need_bkg' to a high
     * enough value in xfer_parms since turning off data type conversion also
     * turns off background preservation.
     */
    if (nelmts!=H5S_get_npoints (file_space)) {
	HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL,
		     "src and dest data spaces have different sizes");
    }
    if (NULL == (tconv_func = H5T_find(mem_type, dataset->type,
				       xfer_parms->need_bkg, &cdata))) {
	HGOTO_ERROR(H5E_DATASET, H5E_UNSUPPORTED, FAIL,
		    "unable to convert between src and dest data types");
    } else if (H5T_conv_noop!=tconv_func) {
	if ((src_id = H5I_register(H5_DATATYPE, H5T_copy(mem_type)))<0 ||
	    (dst_id = H5I_register(H5_DATATYPE, H5T_copy(dataset->type)))<0) {
	    HGOTO_ERROR(H5E_DATASET, H5E_CANTREGISTER, FAIL,
			"unable to register types for conversion");
	}
    }
    if (NULL==(sconv_func=H5S_find (mem_space, file_space))) {
	HGOTO_ERROR (H5E_DATASET, H5E_UNSUPPORTED, FAIL,
		     "unable to convert from memory to file data space");
    }
    
#ifdef HAVE_PARALLEL
    /*
     * Check if collective data transfer requested.
     */
    if (xfer_parms->xfer_mode == H5D_XFER_COLLECTIVE){
	/* verify that the file can support collective access. */
	/* The check may not be necessarily since collective access */
	/* can always be simulated by independent access. */
	/* Nevertheless, must check driver is MPIO before using those */
	/* access_mode which exists only for MPIO case. */
	if (dataset->ent.file->shared->access_parms.driver == H5F_LOW_MPIO){
	    /* Supports only no conversion, type or space, for now. */
	    if (H5T_conv_noop==tconv_func &&
		NULL!=sconv_func->write) {
		/* 
		 * -AKC-
		 * "plant" the collective access mode into the file information
		 * so that the lower level mpio routines know to use collective
		 * access.
		 * This is not thread-safe, is a klutch for now.
		 * Should change all the I/O routines to pass along the xfer
		 * property list to the low level I/O for proper execution.
		 * Make it to work now.  Must fix it later.
		 * -AKC-
		 */
#ifdef AKC
		printf("%s: collective access requested\n", FUNC);
		printf("%s: current f->access_mode = %x\n", FUNC,
		    dataset->ent.file->shared->access_parms.u.mpio.access_mode);
#endif
		access_mode_saved = dataset->ent.file->shared->access_parms.u.mpio.access_mode;
		dataset->ent.file->shared->access_parms.u.mpio.access_mode = H5D_XFER_COLLECTIVE;
		status = (sconv_func->write)(dataset->ent.file, &(dataset->layout),
				             &(dataset->create_parms->compress),
					     &(dataset->create_parms->efl),
					     H5T_get_size (dataset->type), file_space,
                                             mem_space, buf);
		if (status>=0) goto succeed;
		HGOTO_ERROR (H5E_DATASET, H5E_WRITEERROR, FAIL,
		    "collective write failed");
	    }
	}
	HGOTO_ERROR (H5E_DATASET, H5E_UNSUPPORTED, FAIL,
		 "collective access not permissible");
    }
#endif /*HAVE_PARALLEL*/

    
    /*
     * If there is no type conversion then try writing directly from
     * application buffer to file.
     */
    if (H5D_OPTIMIZE_PIPE &&
	H5T_conv_noop==tconv_func &&
	NULL!=sconv_func->write) {
	status = (sconv_func->write)(dataset->ent.file, &(dataset->layout),
				     &(dataset->create_parms->compress),
				     &(dataset->create_parms->efl),
				     H5T_get_size (dataset->type), file_space,
				     mem_space, buf);
	if (status>=0) goto succeed;
#ifdef H5D_DEBUG
	fprintf (stderr, "H5D: data space conversion could not be optimized "
		 "for this case (using general method instead)\n");
#endif
	H5E_clear ();
    }


    /*
     * This is the general case.  Figure out the strip mine size.
     */
    src_type_size = H5T_get_size(mem_type);
    dst_type_size = H5T_get_size(dataset->type);
    target_size = xfer_parms->buf_size;
    request_nelmts = target_size / MAX (src_type_size, dst_type_size);
    if (request_nelmts<=0) {
	HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
		     "temporary buffer max size is too small");
    }
    if (sconv_func->init) {
	smine_nelmts = (sconv_func->init)(&(dataset->layout), mem_space,
					  file_space, request_nelmts,
					  &numbering/*out*/);
	if (smine_nelmts<=0) {
	    HGOTO_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
			 "unable to initialize element numbering information");
	}
    } else {
	smine_nelmts = request_nelmts;
	HDmemset (&numbering, 0, sizeof numbering);
    }

    /*
     * Get a temporary buffer for type conversion unless the app has already
     * supplied one through the xfer properties.  Instead of allocating a
     * buffer which is the exact size, we allocate the target size. The
     * malloc() is usually less resource-intensive if we allocate/free the
     * same size over and over.
     */
    if (cdata->need_bkg) {
	need_bkg = MAX (cdata->need_bkg, xfer_parms->need_bkg);
    } else {
	need_bkg = H5T_BKG_NO; /*never needed even if app says yes*/
    }
    if (NULL==(tconv_buf=xfer_parms->tconv_buf)) {
	tconv_buf = H5MM_xmalloc (target_size);
    }
    if (need_bkg && NULL==(bkg_buf=xfer_parms->bkg_buf)) {
	bkg_buf = H5MM_xmalloc (smine_nelmts * dst_type_size);
    }

#ifdef H5D_DEBUG
    {
	/* Strip mine diagnostics.... */
	size_t buffer_size = smine_nelmts * MAX (src_type_size, dst_type_size);
	if (smine_nelmts<nelmts) {
	    fprintf (stderr, "H5D: strip mine");
	    if (smine_nelmts!=request_nelmts) {
		fprintf (stderr, " got %lu of %lu",
			 (unsigned long)smine_nelmts,
			 (unsigned long)request_nelmts);
	    }
	    if (buffer_size!=target_size) {
		fprintf (stderr, " (%1.1f%% of buffer)",
			 100.0*(double)buffer_size/(double)target_size);
	    }
	    fprintf (stderr, " %1.1f iterations\n",
		     (double)nelmts/(double)smine_nelmts);
	}
    }
#endif

    /* Start strip mining... */
    for (smine_start=0; smine_start<nelmts; smine_start+=smine_nelmts) {
	smine_nelmts = MIN (smine_nelmts, nelmts-smine_start);

	/*
	 * Gather data from application buffer into the data type conversion
	 * buffer. Also gather data from the file into the background buffer
	 * if necessary.
	 */
	if ((sconv_func->mgath)(buf, H5T_get_size (mem_type), mem_space,
				&numbering, smine_start, smine_nelmts,
				tconv_buf/*out*/)!=smine_nelmts) {
	    HGOTO_ERROR (H5E_IO, H5E_WRITEERROR, FAIL, "mem gather failed");
	}
	if ((H5D_OPTIMIZE_PIPE && H5T_BKG_YES==need_bkg) ||
	    (!H5D_OPTIMIZE_PIPE && need_bkg)) {
	    if ((sconv_func->fgath)(dataset->ent.file, &(dataset->layout),
				    &(dataset->create_parms->compress),
				    &(dataset->create_parms->efl),
				    H5T_get_size (dataset->type), file_space,
				    &numbering, smine_start, smine_nelmts,
				    bkg_buf/*out*/)!=smine_nelmts) {
		HGOTO_ERROR (H5E_IO, H5E_WRITEERROR, FAIL,
			     "file gather failed");
	    }
	}

	/*
	 * Perform data type conversion.
	 */
#ifdef H5T_DEBUG
	H5T_timer_begin (&timer, cdata);
#endif
	cdata->command = H5T_CONV_CONV;
	status = (tconv_func) (src_id, dst_id, cdata, smine_nelmts, tconv_buf,
			       bkg_buf);
#ifdef H5T_DEBUG
	H5T_timer_end (&timer, cdata, smine_nelmts);
#endif
	if (status<0) {
	    HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL,
			"data type conversion failed");
	}

	/*
	 * Scatter the data out to the file.
	 */
	if ((sconv_func->fscat)(dataset->ent.file, &(dataset->layout),
				&(dataset->create_parms->compress),
				&(dataset->create_parms->efl),
				H5T_get_size (dataset->type), file_space,
				&numbering, smine_start, smine_nelmts,
				tconv_buf)<0) {
	    HGOTO_ERROR (H5E_DATASET, H5E_WRITEERROR, FAIL, "scatter failed");
	}
    }

 succeed:
    ret_value = SUCCEED;
    
 done:
    if (src_id >= 0) H5I_dec_ref(src_id);
    if (dst_id >= 0) H5I_dec_ref(dst_id);
    if (tconv_buf && NULL==xfer_parms->tconv_buf) {
	H5MM_xfree(tconv_buf);
    }
    if (bkg_buf && NULL==xfer_parms->bkg_buf) {
	H5MM_xfree (bkg_buf);
    }
#ifdef HAVE_PARALLEL
    /*
     * Check if collective data transfer requested.
     * If so, need to restore the access mode.  Shouldnot needed.
     */
    if (xfer_parms->xfer_mode == H5D_XFER_COLLECTIVE){
	dataset->ent.file->shared->access_parms.u.mpio.access_mode = access_mode_saved;
    }
#endif /*HAVE_PARALLEL*/
    FUNC_LEAVE(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5D_extend
 *
 * Purpose:	Increases the size of a dataset.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Friday, January 30, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_extend (H5D_t *dataset, const hsize_t *size)
{
    herr_t	changed;

    FUNC_ENTER (H5D_extend, FAIL);

    /* Check args */
    assert (dataset);
    assert (size);

    /*
     * Restrictions on extensions were checked when the dataset was created.
     * All extensions are allowed here since none should be able to muck
     * things up.
     */
    /*void*/

    /* Increase the size of the data space */
    if ((changed=H5S_extend (dataset->space, size))<0) {
	HRETURN_ERROR (H5E_DATASET, H5E_CANTINIT, FAIL,
		       "unable to increase size of data space");
    }

    /* Save the new dataspace in the file if necessary */
    if (changed>0 &&
	H5S_modify (&(dataset->ent), dataset->space)<0) {
	HRETURN_ERROR (H5E_DATASET, H5E_WRITEERROR, FAIL,
		       "unable to update file with new dataspace");
    }

    FUNC_LEAVE (SUCCEED);
}
    

/*-------------------------------------------------------------------------
 * Function:	H5D_entof
 *
 * Purpose:	Returns a pointer to the entry for a dataset.
 *
 * Return:	Success:	Ptr to entry
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Friday, April 24, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5G_entry_t *
H5D_entof (H5D_t *dataset)
{
    return dataset ? &(dataset->ent) : NULL;
}
