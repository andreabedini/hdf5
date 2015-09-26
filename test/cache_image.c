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

/* Programmer:  John Mainzer
 *              7/13/15
 *
 *              This file contains tests specific to the cache image 
 *		feature implemented in H5C.c
 */
#include "cache_common.h"
#include "genall5.h"

/* global variable declarations: */


const char *FILENAMES[] = {
        "cache_image_test",
        NULL
};


/* local utility function declarations */

static void create_data_sets(hid_t file_id, int min_dset, int max_dset);

static void open_hdf5_file(const hbool_t create_file,
    const hbool_t mdci_sbem_expected, const hbool_t read_only, 
    const hbool_t set_mdci_fapl, const hbool_t config_fsm,
    const char * hdf_file_name, const unsigned cache_image_flags, 
    hid_t * file_id_ptr, H5F_t ** file_ptr_ptr, H5C_t ** cache_ptr_ptr);

static void verify_data_sets(hid_t file_id, int min_dset, int max_dset);

/* local test function declarations */

static unsigned check_cache_image_ctl_flow_1(void);
static unsigned check_cache_image_ctl_flow_2(void);
static unsigned check_cache_image_ctl_flow_3(void);
static unsigned check_cache_image_ctl_flow_4(void);
static unsigned check_cache_image_ctl_flow_5(void);
static unsigned check_cache_image_ctl_flow_6(void);

static unsigned cache_image_smoke_check_1(void);
static unsigned cache_image_smoke_check_2(void);
static unsigned cache_image_smoke_check_3(void);
static unsigned cache_image_smoke_check_4(void);
static unsigned cache_image_smoke_check_5(void);

static unsigned cache_image_api_error_check_1(void);
static unsigned cache_image_api_error_check_2(void);


/****************************************************************************/
/***************************** Utility Functions ****************************/
/****************************************************************************/

/*-------------------------------------------------------------------------
 * Function:    create_data_sets()
 *
 * Purpose:     If pass is TRUE on entry, create the specified data sets
 *		in the indicated file.
 *
 *		Data sets and their contents must be well know, as we 
 *		will verify that they contain the expected data later.
 *
 *              On failure, set pass to FALSE, and set failure_mssg
 *              to point to an appropriate failure message.
 *
 *              Do nothing if pass is FALSE on entry.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/15/15
 *
 * Modifications:
 *
 *              Added min_dset and max_dset parameters and supporting 
 *		code.  This allows the caller to specify a range of 
 *		datasets to create.
 *						JRM -- 8/20/15
 *
 *-------------------------------------------------------------------------
 */

#define CHUNK_SIZE              10
#define DSET_SIZE               (40 * CHUNK_SIZE)
#define MAX_NUM_DSETS           256

static void 
create_data_sets(hid_t file_id, int min_dset, int max_dset)
{
    const char * fcn_name = "create_data_sets()";
    char dset_name[64];
    hbool_t show_progress = FALSE;
    hbool_t valid_chunk;
    hbool_t verbose = FALSE;
    int cp = 0;
    int i, j, k, l, m;
    int data_chunk[CHUNK_SIZE][CHUNK_SIZE];
    herr_t status;
    hid_t dataspace_id = -1;
    hid_t filespace_ids[MAX_NUM_DSETS];
    hid_t memspace_id = -1;
    hid_t dataset_ids[MAX_NUM_DSETS];
    hid_t properties;
    hsize_t dims[2];
    hsize_t a_size[2];
    hsize_t offset[2];
    hsize_t chunk_size[2];

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    HDassert(0 <= min_dset);
    HDassert(min_dset <= max_dset);
    HDassert(max_dset < MAX_NUM_DSETS);

    /* create the datasets */

    if ( pass ) {

        i = min_dset;

        while ( ( pass ) && ( i <= max_dset ) )
        {
            /* create a dataspace for the chunked dataset */
            dims[0] = DSET_SIZE;
            dims[1] = DSET_SIZE;
            dataspace_id = H5Screate_simple(2, dims, NULL);

            if ( dataspace_id < 0 ) {

                pass = FALSE;
                failure_mssg = "H5Screate_simple() failed.";
            }

            /* set the dataset creation plist to specify that the raw data is
             * to be partioned into 10X10 element chunks.
             */

            if ( pass ) {

                chunk_size[0] = CHUNK_SIZE;
                chunk_size[1] = CHUNK_SIZE;
                properties = H5Pcreate(H5P_DATASET_CREATE);

                if ( properties < 0 ) {

                    pass = FALSE;
                    failure_mssg = "H5Pcreate() failed.";
                }
            }

            if ( pass ) {

                if ( H5Pset_chunk(properties, 2, chunk_size) < 0 ) {

                    pass = FALSE;
                    failure_mssg = "H5Pset_chunk() failed.";
                }
            }

            /* create the dataset */
            if ( pass ) {

                sprintf(dset_name, "/dset%03d", i);
                dataset_ids[i] = H5Dcreate2(file_id, dset_name, H5T_STD_I32BE,
                                            dataspace_id, H5P_DEFAULT,
                                            properties, H5P_DEFAULT);

                if ( dataset_ids[i] < 0 ) {

                    pass = FALSE;
                    failure_mssg = "H5Dcreate() failed.";
                }
            }

            /* get the file space ID */
            if ( pass ) {

                filespace_ids[i] = H5Dget_space(dataset_ids[i]);

                if ( filespace_ids[i] < 0 ) {

                    pass = FALSE;
                    failure_mssg = "H5Dget_space() failed.";
                }
            }

            i++;
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* create the mem space to be used to read and write chunks */
    if ( pass ) {

        dims[0] = CHUNK_SIZE;
        dims[1] = CHUNK_SIZE;
        memspace_id = H5Screate_simple(2, dims, NULL);

        if ( memspace_id < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Screate_simple() failed.";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* select in memory hyperslab */
    if ( pass ) {

        offset[0] = 0;  /*offset of hyperslab in memory*/
        offset[1] = 0;
        a_size[0] = CHUNK_SIZE;  /*size of hyperslab*/
        a_size[1] = CHUNK_SIZE;
        status = H5Sselect_hyperslab(memspace_id, H5S_SELECT_SET, offset, NULL,
                                     a_size, NULL);

        if ( status < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Sselect_hyperslab() failed.";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* initialize all datasets on a round robin basis */
    i = 0;
    while ( ( pass ) && ( i < DSET_SIZE ) )
    {
        j = 0;
        while ( ( pass ) && ( j < DSET_SIZE ) )
        {
            m = min_dset;
            while ( ( pass ) && ( m <= max_dset ) )
            {
                /* initialize the slab */
                for ( k = 0; k < CHUNK_SIZE; k++ )
                {
                    for ( l = 0; l < CHUNK_SIZE; l++ )
                    {
                        data_chunk[k][l] = (DSET_SIZE * DSET_SIZE * m) +
                                           (DSET_SIZE * (i + k)) + j + l;
                    }
                }

                /* select on disk hyperslab */
                offset[0] = (hsize_t)i; /*offset of hyperslab in file*/
                offset[1] = (hsize_t)j;
                a_size[0] = CHUNK_SIZE;   /*size of hyperslab*/
                a_size[1] = CHUNK_SIZE;
                status = H5Sselect_hyperslab(filespace_ids[m], H5S_SELECT_SET,
                                             offset, NULL, a_size, NULL);

                if ( status < 0 ) {

                    pass = FALSE;
                    failure_mssg = "disk H5Sselect_hyperslab() failed.";
                }

                /* write the chunk to file */
                status = H5Dwrite(dataset_ids[m], H5T_NATIVE_INT, memspace_id,
                                  filespace_ids[m], H5P_DEFAULT, data_chunk);

                if ( status < 0 ) {

                    pass = FALSE;
                    failure_mssg = "H5Dwrite() failed.";
                }
                m++;
            }
            j += CHUNK_SIZE;
        }

        i += CHUNK_SIZE;
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* read data from data sets and validate it */
    i = 0;
    while ( ( pass ) && ( i < DSET_SIZE ) )
    {
        j = 0;
        while ( ( pass ) && ( j < DSET_SIZE ) )
        {
            m = min_dset;
            while ( ( pass ) && ( m <= max_dset ) )
            {

                /* select on disk hyperslab */
                offset[0] = (hsize_t)i; /* offset of hyperslab in file */
                offset[1] = (hsize_t)j;
                a_size[0] = CHUNK_SIZE; /* size of hyperslab */
                a_size[1] = CHUNK_SIZE;
                status = H5Sselect_hyperslab(filespace_ids[m], H5S_SELECT_SET,
                                             offset, NULL, a_size, NULL);

                if ( status < 0 ) {

                   pass = FALSE;
                   failure_mssg = "disk hyperslab create failed.";
                }

                /* read the chunk from file */
                if ( pass ) {

                    status = H5Dread(dataset_ids[m], H5T_NATIVE_INT, 
                                     memspace_id, filespace_ids[m], 
                                     H5P_DEFAULT, data_chunk);

                    if ( status < 0 ) {

                       pass = FALSE;
                       failure_mssg = "disk hyperslab create failed.";
                    }
                }

                /* validate the slab */
                if ( pass ) {

                    valid_chunk = TRUE;
                    for ( k = 0; k < CHUNK_SIZE; k++ )
                    {
                        for ( l = 0; l < CHUNK_SIZE; l++ )
                        {
                            if ( data_chunk[k][l]
                                 !=
                                 ((DSET_SIZE * DSET_SIZE * m) +
                                  (DSET_SIZE * (i + k)) + j + l) ) {

                                valid_chunk = FALSE;

				if ( verbose ) {

                                    HDfprintf(stdout,
                                    "data_chunk[%0d][%0d] = %0d, expect %0d.\n",
                                    k, l, data_chunk[k][l],
                                    ((DSET_SIZE * DSET_SIZE * m) +
                                     (DSET_SIZE * (i + k)) + j + l));
                                    HDfprintf(stdout,
                                    "m = %d, i = %d, j = %d, k = %d, l = %d\n",
                                    m, i, j, k, l);
				}
                            }
                        }
                    }

                    if ( ! valid_chunk ) {

                        pass = FALSE;
                        failure_mssg = "slab validation failed.";

			if ( verbose ) {

                            fprintf(stdout, 
                                  "Chunk (%0d, %0d) in /dset%03d is invalid.\n",
                                  i, j, m);
			}
                    }
                }
                m++;
            }
            j += CHUNK_SIZE;
        }
        i += CHUNK_SIZE;
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* close the file spaces */
    i = min_dset;
    while ( ( pass ) && ( i <= max_dset ) )
    {
        if ( H5Sclose(filespace_ids[i]) < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Sclose() failed.";
        }
        i++;
    }


    /* close the datasets */
    i = min_dset;
    while ( ( pass ) && ( i <= max_dset ) )
    {
        if ( H5Dclose(dataset_ids[i]) < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Dclose() failed.";
        }
        i++;
    }

    /* close the mem space */
    if ( pass ) {

        if ( H5Sclose(memspace_id) < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Sclose(memspace_id) failed.";
        }
    }

    return;

} /* create_data_sets() */


/*-------------------------------------------------------------------------
 * Function:    open_hdf5_file()
 *
 * Purpose:     If pass is true on entry, create or open the specified HDF5 
 *		and test to see if it has a metadata cache image superblock 
 *		extension message.  
 *
 *		Set pass to FALSE and issue a suitable failure 
 *		message if either the file contains a metadata cache image
 *		superblock extension and mdci_sbem_expected is TRUE, or 
 *		vise versa.
 *
 *		If mdci_sbem_expected is TRUE, also verify that the metadata 
 *		cache has been advised of this.
 *
 *		If read_only is TRUE, open the file read only.  Otherwise
 *		open the file read/write.
 *
 *		If set_mdci_fapl is TRUE, set the metadata cache image 
 *		FAPL entry when opening the file, and verify that the 
 *		metadata cache is notified.
 *
 *		If config_fsm is TRUE, setup the persistant free space 
 *		manager.  Note that this flag may only be set if 
 *		create_file is also TRUE.
 *
 *              Return pointers to the cache data structure and file data
 *              structures.
 *
 *              On failure, set pass to FALSE, and set failure_mssg
 *              to point to an appropriate failure message.
 *
 *              Do nothing if pass is FALSE on entry.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/14/15
 *
 * Modifications:
 *
 *              None.
 *
 *-------------------------------------------------------------------------
 */

static void
open_hdf5_file(const hbool_t create_file,
               const hbool_t mdci_sbem_expected,
	       const hbool_t read_only,
	       const hbool_t set_mdci_fapl,
	       const hbool_t config_fsm,
	       const char * hdf_file_name,
               const unsigned cache_image_flags,
               hid_t * file_id_ptr,
               H5F_t ** file_ptr_ptr,
               H5C_t ** cache_ptr_ptr)
{
    const char * fcn_name = "open_hdf5_file()";
    hbool_t show_progress = FALSE;
    hbool_t verbose = FALSE;
    int cp = 0;
    hid_t fapl_id = -1;
    hid_t fcpl_id = -1;
    hid_t file_id = -1;
    herr_t result;
    H5F_t * file_ptr = NULL;
    H5C_t * cache_ptr = NULL;
    H5C_cache_image_ctl_t image_ctl;
    H5AC_cache_image_config_t cache_image_config = {
        H5AC__CURR_CACHE_IMAGE_CONFIG_VERSION,
        TRUE,
        0};

    if ( pass )
    {
	/* opening the file both read only and with a cache image 
         * requested is a contradiction.  We resolve it by ignoring 
         * the cache image request silently.
         */
        if ( ( create_file && mdci_sbem_expected ) ||
             ( create_file && read_only ) ||
             ( config_fsm && !create_file ) ||
             ( hdf_file_name == NULL ) ||
             ( ( set_mdci_fapl ) && ( cache_image_flags == 0 ) ) ||
             ( ( set_mdci_fapl ) &&
               ( (cache_image_flags & ~H5C_CI__ALL_FLAGS) != 0 ) ) ||
             ( file_id_ptr == NULL ) ||
             ( file_ptr_ptr == NULL ) ||
             ( cache_ptr_ptr == NULL ) ) {

            failure_mssg =
               "Bad param(s) on entry to open_hdf5_file().\n";
            pass = FALSE;
        } else  if ( verbose ) {

            HDfprintf(stdout, "%s: HDF file name = \"%s\".\n",
                      fcn_name, hdf_file_name);
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* create a file access propertly list. */
    if ( pass ) {

        fapl_id = H5Pcreate(H5P_FILE_ACCESS);

        if ( fapl_id < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Pcreate() failed.\n";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* call H5Pset_libver_bounds() on the fapl_id */
    if ( pass ) {

        if ( H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) 
                < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Pset_libver_bounds() failed.\n";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* get metadata cache image config -- verify that it is the default */
    if ( pass ) {

        result = H5Pget_mdc_image_config(fapl_id, &cache_image_config);

        if ( result < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Pget_mdc_image_config() failed.\n";
        }

        if ( ( cache_image_config.version !=
               H5AC__CURR_CACHE_IMAGE_CONFIG_VERSION ) ||
             ( cache_image_config.generate_image != FALSE ) ||
             ( cache_image_config.max_image_size != 0 ) ) {

            pass = FALSE;
            failure_mssg = "Unexpected default cache image config.\n";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* set metadata cache image fapl entry if indicated */
    if ( ( pass ) && ( set_mdci_fapl ) ) {

        /* set cache image config fields to taste */
        cache_image_config.generate_image = TRUE;
        cache_image_config.max_image_size = 0;

        result = H5Pset_mdc_image_config(fapl_id, &cache_image_config);

        if ( result < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Pset_mdc_image_config() failed.\n";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* setup the persistant free space manager if indicated */
    if ( ( pass ) && ( config_fsm ) ) {

	fcpl_id = H5Pcreate(H5P_FILE_CREATE);

	if ( fcpl_id <= 0 ) {

	    pass = FALSE;
	    failure_mssg = "H5Pcreate(H5P_FILE_CREATE) failed.";
	}
    }

    if ( ( pass ) && ( config_fsm ) ) {

        if ( H5Pset_file_space(fcpl_id, H5F_FILE_SPACE_ALL_PERSIST, 1) < 0 ) {

	    pass = FALSE;
	    failure_mssg = "H5Pset_file_space() failed.";
	}
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* open the file */
    if ( pass ) {

        if ( create_file ) {

	    if ( fcpl_id != -1 )

                file_id = H5Fcreate(hdf_file_name, H5F_ACC_TRUNC, 
				    fcpl_id, fapl_id);
	    else

		file_id = H5Fcreate(hdf_file_name, H5F_ACC_TRUNC, 
                                    H5P_DEFAULT, fapl_id);

        } else {

            if ( read_only )

                file_id = H5Fopen(hdf_file_name, H5F_ACC_RDONLY, fapl_id);

            else

                file_id = H5Fopen(hdf_file_name, H5F_ACC_RDWR, fapl_id);
        }

        if ( file_id < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Fcreate() or H5Fopen() failed.\n";

        } else {

            file_ptr = (struct H5F_t *)H5I_object_verify(file_id, H5I_FILE);

            if ( file_ptr == NULL ) {

                pass = FALSE;
                failure_mssg = "Can't get file_ptr.";

                if ( verbose ) {
                    HDfprintf(stdout, "%s: Can't get file_ptr.\n", fcn_name);
                }
            }
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* get a pointer to the files internal data structure and then
     * to the cache structure
     */
    if ( pass ) {

        if ( file_ptr->shared->cache == NULL ) {

            pass = FALSE;
            failure_mssg = "can't get cache pointer(1).\n";

        } else {

            cache_ptr = file_ptr->shared->cache;
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* verify expected metadata cache status */

    /* get the cache image control structure from the cache, and verify
     * that it contains the expected values.
     *
     * Then set the flags in this structure to the specified value.
     */
    if ( pass ) {

        if ( H5C_get_cache_image_config(cache_ptr, &image_ctl) < 0 ) {

            pass = FALSE;
            failure_mssg = "error returned by H5C_get_cache_image_config().";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    if ( pass ) {

        if ( set_mdci_fapl ) {

	    if ( read_only ) {

                if ( ( image_ctl.version != 
                       H5AC__CURR_CACHE_IMAGE_CONFIG_VERSION ) ||
                     ( image_ctl.generate_image != FALSE ) ||
                     ( image_ctl.max_image_size != 0 ) ||
                     ( image_ctl.flags != H5C_CI__ALL_FLAGS ) ) {

                    pass = FALSE;
                    failure_mssg = "Unexpected image_ctl values(1).\n";
                }
	    } else {

                if ( ( image_ctl.version != 
                       H5AC__CURR_CACHE_IMAGE_CONFIG_VERSION ) ||
                     ( image_ctl.generate_image != TRUE ) ||
                     ( image_ctl.max_image_size != 0 ) ||
                     ( image_ctl.flags != H5C_CI__ALL_FLAGS ) ) {

                    pass = FALSE;
                    failure_mssg = "Unexpected image_ctl values(2).\n";
                }
            }
        } else {

            if ( ( image_ctl.version != 
                   H5AC__CURR_CACHE_IMAGE_CONFIG_VERSION ) ||
                 ( image_ctl.generate_image != FALSE ) ||
                 ( image_ctl.max_image_size != 0 ) ||
                 ( image_ctl.flags != H5C_CI__ALL_FLAGS ) ) {

                pass = FALSE;
                failure_mssg = "Unexpected image_ctl values(3).\n";
            }
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    if ( ( pass ) && ( set_mdci_fapl ) ) {

        image_ctl.flags = cache_image_flags;

        if ( H5C_set_cache_image_config(file_ptr, cache_ptr, &image_ctl) < 0 ) {

            pass = FALSE;
            failure_mssg = "error returned by H5C_set_cache_image_config().";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    if ( pass ) {

        if ( cache_ptr->close_warning_received == TRUE ) {

            pass = FALSE;
            failure_mssg = "Unexpected value of close_warning_received.\n";
        }

        if ( mdci_sbem_expected ) {

            if ( read_only ) {

                if ( ( cache_ptr->load_image != TRUE ) ||
                     ( cache_ptr->delete_image != FALSE ) ) {

                    pass = FALSE;
                    failure_mssg = "mdci sb extension message not present?\n";
                }
            } else {

                if ( ( cache_ptr->load_image != TRUE ) ||
                     ( cache_ptr->delete_image != TRUE ) ) {

                    pass = FALSE;
                    failure_mssg = "mdci sb extension message not present?\n";
                }
	    }
        } else {

	    if ( ( cache_ptr->load_image == TRUE ) ||
                 ( cache_ptr->delete_image == TRUE ) ) {

                pass = FALSE;
                failure_mssg = "mdci sb extension message present?\n";
	    }
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    if ( pass ) {

        *file_id_ptr = file_id;
        *file_ptr_ptr = file_ptr;
        *cache_ptr_ptr = cache_ptr;
    }

    if ( show_progress )
        HDfprintf(stdout, "%s: cp = %d -- exiting.\n", fcn_name, cp++);

    return;

} /* open_hdf5_file() */


/*-------------------------------------------------------------------------
 * Function:    verify_data_sets()
 *
 * Purpose:     If pass is TRUE on entry, verify that the data sets in the 
 *		file exist and contain the expected data.  
 *
 *		Note that these data sets were created by 
 *		create_data_sets() above.  Thus any changes in that 
 *		function must be reflected in this function, and 
 *		vise-versa.
 *
 *              On failure, set pass to FALSE, and set failure_mssg
 *              to point to an appropriate failure message.
 *
 *              Do nothing if pass is FALSE on entry.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/15/15
 *
 * Modifications:
 *
 *              Added min_dset and max_dset parameters and supporting 
 *		code.  This allows the caller to specify a range of 
 *		datasets to verify.
 *						JRM -- 8/20/15
 *
 *-------------------------------------------------------------------------
 */

static void 
verify_data_sets(hid_t file_id, int min_dset, int max_dset)
{
    const char * fcn_name = "verify_data_sets()";
    char dset_name[64];
    hbool_t show_progress = FALSE;
    hbool_t valid_chunk;
    hbool_t verbose = FALSE;
    int cp = 0;
    int i, j, k, l, m;
    int data_chunk[CHUNK_SIZE][CHUNK_SIZE];
    herr_t status;
    hid_t filespace_ids[MAX_NUM_DSETS];
    hid_t memspace_id = -1;
    hid_t dataset_ids[MAX_NUM_DSETS];
    hsize_t dims[2];
    hsize_t a_size[2];
    hsize_t offset[2];

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    HDassert(0 <= min_dset);
    HDassert(min_dset <= max_dset);
    HDassert(max_dset < MAX_NUM_DSETS);

    /* open the datasets */

    if ( pass ) {

        i = min_dset;

        while ( ( pass ) && ( i <= max_dset ) )
        {
            /* open the dataset */
            if ( pass ) {

                sprintf(dset_name, "/dset%03d", i);
                dataset_ids[i] = H5Dopen2(file_id, dset_name, H5P_DEFAULT);

                if ( dataset_ids[i] < 0 ) {

                    pass = FALSE;
                    failure_mssg = "H5Dopen2() failed.";
                }
            }

            /* get the file space ID */
            if ( pass ) {

                filespace_ids[i] = H5Dget_space(dataset_ids[i]);

                if ( filespace_ids[i] < 0 ) {

                    pass = FALSE;
                    failure_mssg = "H5Dget_space() failed.";
                }
            }

            i++;
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* create the mem space to be used to read and write chunks */
    if ( pass ) {

        dims[0] = CHUNK_SIZE;
        dims[1] = CHUNK_SIZE;
        memspace_id = H5Screate_simple(2, dims, NULL);

        if ( memspace_id < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Screate_simple() failed.";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* select in memory hyperslab */
    if ( pass ) {

        offset[0] = 0;  /*offset of hyperslab in memory*/
        offset[1] = 0;
        a_size[0] = CHUNK_SIZE;  /*size of hyperslab*/
        a_size[1] = CHUNK_SIZE;
        status = H5Sselect_hyperslab(memspace_id, H5S_SELECT_SET, offset, NULL,
                                     a_size, NULL);

        if ( status < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Sselect_hyperslab() failed.";
        }
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);


    /* read data from data sets and validate it */
    i = 0;
    while ( ( pass ) && ( i < DSET_SIZE ) )
    {
        j = 0;
        while ( ( pass ) && ( j < DSET_SIZE ) )
        {
            m = min_dset;
            while ( ( pass ) && ( m <= max_dset ) )
            {

                /* select on disk hyperslab */
                offset[0] = (hsize_t)i; /* offset of hyperslab in file */
                offset[1] = (hsize_t)j;
                a_size[0] = CHUNK_SIZE; /* size of hyperslab */
                a_size[1] = CHUNK_SIZE;
                status = H5Sselect_hyperslab(filespace_ids[m], H5S_SELECT_SET,
                                             offset, NULL, a_size, NULL);

                if ( status < 0 ) {

                   pass = FALSE;
                   failure_mssg = "disk hyperslab create failed.";
                }

                /* read the chunk from file */
                if ( pass ) {

                    status = H5Dread(dataset_ids[m], H5T_NATIVE_INT, 
                                     memspace_id, filespace_ids[m], 
                                     H5P_DEFAULT, data_chunk);

                    if ( status < 0 ) {

                       pass = FALSE;
                       failure_mssg = "disk hyperslab create failed.";
                    }
                }

                /* validate the slab */
                if ( pass ) {

                    valid_chunk = TRUE;
                    for ( k = 0; k < CHUNK_SIZE; k++ )
                    {
                        for ( l = 0; l < CHUNK_SIZE; l++ )
                        {
                            if ( data_chunk[k][l]
                                 !=
                                 ((DSET_SIZE * DSET_SIZE * m) +
                                  (DSET_SIZE * (i + k)) + j + l) ) {

                                valid_chunk = FALSE;

				if ( verbose ) {
				
                                    HDfprintf(stdout,
                                    "data_chunk[%0d][%0d] = %0d, expect %0d.\n",
                                    k, l, data_chunk[k][l],
                                    ((DSET_SIZE * DSET_SIZE * m) +
                                     (DSET_SIZE * (i + k)) + j + l));
                                    HDfprintf(stdout,
                                     "m = %d, i = %d, j = %d, k = %d, l = %d\n",
                                     m, i, j, k, l);
				}
                            }
                        }
                    }

                    if ( ! valid_chunk ) {

                        pass = FALSE;
                        failure_mssg = "slab validation failed.";

			if ( verbose ) {

                            fprintf(stdout, 
                                  "Chunk (%0d, %0d) in /dset%03d is invalid.\n",
                                  i, j, m);
			}
                    }
                }
                m++;
            }
            j += CHUNK_SIZE;
        }
        i += CHUNK_SIZE;
    }

    if ( show_progress ) HDfprintf(stdout, "%s: cp = %d.\n", fcn_name, cp++);

    /* close the file spaces */
    i = min_dset;
    while ( ( pass ) && ( i <= max_dset ) )
    {
        if ( H5Sclose(filespace_ids[i]) < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Sclose() failed.";
        }
        i++;
    }


    /* close the datasets */
    i = min_dset;
    while ( ( pass ) && ( i <= max_dset ) )
    {
        if ( H5Dclose(dataset_ids[i]) < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Dclose() failed.";
        }
        i++;
    }

    /* close the mem space */
    if ( pass ) {

        if ( H5Sclose(memspace_id) < 0 ) {

            pass = FALSE;
            failure_mssg = "H5Sclose(memspace_id) failed.";
        }
    }

    return;

} /* verify_data_sets() */


/****************************************************************************/
/******************************* Test Functions *****************************/
/****************************************************************************/

/*-------------------------------------------------------------------------
 * Function:    check_cache_image_ctl_flow_1()
 *
 * Purpose:     This test is one of a sequence of control flow tests intended
 *		to verify that control flow for the cache image feature works
 *		as expected.  
 *
 *		This test is an initial smoke check, so the sequence of 
 *		operations is relatively simple.  In particular, we are 
 *		testing:
 *
 *			i) Creation of file with cache image FAPL entry set
 *			   and insertion of metadata cache image superblock
 *			   message on file close.
 *
 *		       ii) Open of file with metadata cache image superblock
 *			   message, transmission of message to metadata cache,
 *			   and deletion of superblock message prior to close.
 *
 *		Note that in all cases we are performing operations on the 
 *		file.  While this is the typical case, we must repeat this 
 *		test without operations on the file.
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set flags forcing creation of metadata cache image 
 *		   super block extension message only.
 *
 *		2) Create some data sets in the file. 
 *
 *		3) Close the file.
 *
 *		4) Open the file.  
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *		5) Open a data set.
 *
 *		   Verify that the metadata cache image superblock 
 *		   extension message has been deleted.
 *
 *		6) Close the file.
 *
 *		7) Open the file.
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *		8) Close the file.
 *
 *		9) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/15/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
check_cache_image_ctl_flow_1(void)
{
    const char * fcn_name = "check_cache_image_ctl_flow_1()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image control flow test 1");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file with the cache image FAPL entry. 
     *
     *	  Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing creation of metadata cache image super block 
     *	  extension message only.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__GEN_MDCI_SBE_MESG,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Create some data sets in the file. */

    if ( pass ) {

        create_data_sets(file_id, 0, 5);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 4) Open the file.  
     *
     *    Verify that the metadata cache is instructed to load the 
     *    metadata cache image, and that the supplied address and length 
     *    are HADDR_UNDEF and zero respectively.  Note that these values 
     *    indicate that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 5) Open and close a data set.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

    if ( pass ) {

        /* think on how to verify that the superblock extension has been
         * deleted, and if it is necessary to verify this directly.
         */
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 7) Open the file. 
     *
     *    Verify that the file doesn't contain a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 8) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 9) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }

    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* check_cache_image_ctl_flow_1() */


/*-------------------------------------------------------------------------
 * Function:    check_cache_image_ctl_flow_2()
 *
 * Purpose:     This test is one of a sequence of control flow tests intended
 *		to verify that control flow for the cache image feature works
 *		as expected.  
 *
 *		This test is an initial smoke check, so the sequence of 
 *		operations is relatively simple.  In particular, we are 
 *		testing:
 *
 *			i) Creation of file with cache image FAPL entry set
 *			   and insertion of metadata cache image superblock
 *			   message on file close.
 *
 *		       ii) Open of file with metadata cache image superblock
 *			   message, transmission of message to metadata cache,
 *			   and deletion of superblock message prior to close.
 *
 *		Note that unlike the previous test, no operations are performed
 *		on the file.  As a result of this, the metadata cache image 
 *		message is not processed until the metadata cache receives
 *		the file close warning.  (Under normal circumstances, it is 
 *		processed as part of the first protect operation after the 
 *		superblock is loaded.)
 *
 *		In this particular test, we preform the following operations:
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set flags forcing creation of metadata cache image 
 *		   super block extension message only.
 *
 *		2) Close the file.
 *
 *		3) Open the file.  
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *		6) Close the file.
 *
 *		7) Open the file.
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *		8) Close the file.
 *
 *		9) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/15/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
check_cache_image_ctl_flow_2(void)
{
    const char * fcn_name = "check_cache_image_ctl_flow_2()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image control flow test 2");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file with the cache image FAPL entry. 
     *
     *	  Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing creation of metadata cache image super block 
     *	  extension message only.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__GEN_MDCI_SBE_MESG,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Open the file.  
     *
     *    Verify that the metadata cache is instructed to load the 
     *    metadata cache image, and that the supplied address and length 
     *    are HADDR_UNDEF and zero respectively.  Note that these values 
     *    indicate that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 4) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 5) Open the file. 
     *
     *    Verify that the file doesn't contain a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 7) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }

    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* check_cache_image_ctl_flow_2() */


/*-------------------------------------------------------------------------
 * Function:    check_cache_image_ctl_flow_3()
 *
 * Purpose:     This test is one of a sequence of control flow tests intended
 *		to verify that control flow for the cache image feature works
 *		as expected.  
 *
 *		The objectives of this test are to:
 *
 *			i) Test operation of the metadata cache image FAPL 
 *			   entry set on open of an existing file.  This 
 *			   should result in the insertion of a metadata 
 *			   cache image superblock message on file close.
 *
 *		       ii) Test operation of the metadata cache image super
 *			   block extension message when it appears in a file
 *			   that is opened READ ONLY.
 *
 *		Note that in all cases we are performing operations on the 
 *		file between file open and close.  While this is the 
 *		typical case, we must repeat this test without operations 
 *		on the file.
 *
 *		1) Create a HDF5 file WITHOUT the cache image FAPL entry.  
 *
 *		   Verify that the cache is NOT informed of the cache image 
 *		   FAPL entry.
 *
 *		2) Close the file.
 *
 *		3) Open the file WITH the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set flags forcing creation of metadata cache image 
 *		   super block extension message only.
 *
 *		4) Create some data sets.
 *
 *		5) Close the file.
 *
 *		6) Open the file READ ONLY.
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *		7) Verify the contents of the data sets.
 *
 *		8) Close the file.
 *
 *		9) Open the file READ/WRITE.
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *	       10) Verify the contents of the data sets.
 *
 *	       11) Close the file.
 *
 *	       12) Open the file
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *	       13) Close the file.
 *
 *	       14) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/16/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
check_cache_image_ctl_flow_3(void)
{
    const char * fcn_name = "check_cache_image_ctl_flow_3()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image control flow test 3");

    pass = TRUE;

    if ( show_progress ) /* 0 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) /* 1 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
    /* 1) Create a HDF5 file WITHOUT the cache image FAPL entry.  
     *
     *    Verify that the cache is NOT informed of the cache image 
     *    FAPL entry.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 2 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 3 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Open the file WITH the cache image FAPL entry.  
     *
     *    Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing creation of metadata cache image super block 
     *    extension message only.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__GEN_MDCI_SBE_MESG,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 4 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 4) Create some data sets. */

    if ( pass ) {

        create_data_sets(file_id, 0, 5);
    }

    if ( show_progress ) /* 5 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 5) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 6 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 6) Open the file READ ONLY.
     *
     *    Verify that the metadata cache is instructed
     *    to load the metadata cache image, and that the 
     *    supplied address and length are HADDR_UNDEF and 
     *    zero respectively.  Note that these values indicate
     *    that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ TRUE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 7 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 7) Verify the contents of the data sets. */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

    if ( show_progress ) /* 8 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 8) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 9 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 9) Open the file READ/WRITE.
     *
     *    Verify that the metadata cache is instructed
     *    to load the metadata cache image, and that the 
     *    supplied address and length are HADDR_UNDEF and 
     *    zero respectively.  Note that these values indicate
     *    that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 10 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 10) Verify the contents of the data sets. */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

    if ( show_progress ) /* 11 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 11) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 12 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 12) Open the file
     *
     *	   Verify that the file doesn't contain a metadata cache
     *	   image superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 13 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 13) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 14 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 14) Delete the file. */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }


    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* check_cache_image_ctl_flow_3() */


/*-------------------------------------------------------------------------
 * Function:    check_cache_image_ctl_flow_4()
 *
 * Purpose:     This test is one of a sequence of control flow tests intended
 *		to verify that control flow for the cache image feature works
 *		as expected.  
 *
 *		The objectives of this test are to:
 *
 *			i) Test operation of the metadata cache image FAPL 
 *			   entry set on open of an existing file.  This 
 *			   should result in the insertion of a metadata 
 *			   cache image superblock message on file close.
 *
 *		       ii) Test operation of the metadata cache image super
 *			   block extension message when it appears in a file
 *			   that is opened READ ONLY.
 *
 *		In this test we avoid all file access beyond file open 
 *		and close.
 *
 *		1) Create a HDF5 file WITHOUT the cache image FAPL entry.  
 *
 *		   Verify that the cache is NOT informed of the cache image 
 *		   FAPL entry.
 *
 *		2) Close the file.
 *
 *		3) Open the file WITH the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set flags forcing creation of metadata cache image 
 *		   super block extension message only.
 *
 *		4) Close the file.
 *
 *		5) Open the file READ ONLY.
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *		6) Close the file.
 *
 *		7) Open the file READ/WRITE.
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *	        8) Close the file.
 *
 *	        9) Open the file
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *	       10) Close the file.
 *
 *	       11) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/16/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
check_cache_image_ctl_flow_4(void)
{
    const char * fcn_name = "check_cache_image_ctl_flow_4()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image control flow test 4");

    pass = TRUE;

    if ( show_progress ) /* 0 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) /* 1 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
    /* 1) Create a HDF5 file WITHOUT the cache image FAPL entry.  
     *
     *    Verify that the cache is NOT informed of the cache image 
     *    FAPL entry.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 2 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 3 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Open the file WITH the cache image FAPL entry.  
     *
     *    Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing creation of metadata cache image super block 
     *    extension message only.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__GEN_MDCI_SBE_MESG,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 4 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 4) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 5 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 5) Open the file READ ONLY.
     *
     *    Verify that the metadata cache is instructed
     *    to load the metadata cache image, and that the 
     *    supplied address and length are HADDR_UNDEF and 
     *    zero respectively.  Note that these values indicate
     *    that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ TRUE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 6 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 7 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 7) Open the file READ/WRITE.
     *
     *    Verify that the metadata cache is instructed
     *    to load the metadata cache image, and that the 
     *    supplied address and length are HADDR_UNDEF and 
     *    zero respectively.  Note that these values indicate
     *    that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 8 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 8) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 9 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 9) Open the file
     *
     *	   Verify that the file doesn't contain a metadata cache
     *	   image superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 10 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 10) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 11 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 11) Delete the file. */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }


    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* check_cache_image_ctl_flow_4() */


/*-------------------------------------------------------------------------
 * Function:    check_cache_image_ctl_flow_5()
 *
 * Purpose:     This test is one of a sequence of control flow tests intended
 *		to verify that control flow for the cache image feature works
 *		as expected.  
 *
 *		The objective of this test is verify correct control flow
 *		when a file with a metadata cache image superblock extension
 *		message is opened with the metadata cache image FAPL entry.
 *
 *		Note that in all cases we are performing operations on the 
 *		file between file open and close.  While this is the 
 *		typical case, we must repeat this test without operations 
 *		on the file.
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set flags forcing creation of metadata cache image 
 *		   super block extension message only.
 *
 *		2) Create some data sets.
 *
 *		3) Close the file.
 *
 *		4) Open the file WITH the cache image FAPL entry.  
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set flags forcing creation of metadata cache image 
 *		   super block extension message only.
 *
 *		5) Verify the contents of the data sets.
 *
 *		6) Close the file.
 *
 *		7) Open the file.
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *	        8) Verify the contents of the data sets.
 *
 *	        9) Close the file.
 *
 *	       10) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/17/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
check_cache_image_ctl_flow_5(void)
{
    const char * fcn_name = "check_cache_image_ctl_flow_5()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image control flow test 5");

    pass = TRUE;

    if ( show_progress ) /* 0 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) /* 1 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 1) Create a HDF5 file with the cache image FAPL entry.  
     *
     *    Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing creation of metadata cache image 
     *    super block extension message only.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__GEN_MDCI_SBE_MESG,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 2 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 2) Create some data sets. */

    if ( pass ) {

        create_data_sets(file_id, 0, 5);
    }

    if ( show_progress ) /* 3 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 3) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 4 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 4) Open the file WITH the cache image FAPL entry.  
     *
     *    Verify that the metadata cache is instructed
     *    to load the metadata cache image, and that the 
     *    supplied address and length are HADDR_UNDEF and 
     *    zero respectively.  Note that these values indicate
     *    that the metadata image block doesn't exist.
     *
     *    Verify that the cache is informed of the cache image 
     *    FAPL entry.
     *
     *    Set flags forcing creation of metadata cache image 
     *    super block extension message only.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__GEN_MDCI_SBE_MESG,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 5 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    /* 5) Verify the contents of the data sets. */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

    if ( show_progress ) /* 6 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 7 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 7) Open the file. 
     *
     *    Verify that the metadata cache is instructed
     *    to load the metadata cache image, and that the 
     *    supplied address and length are HADDR_UNDEF and 
     *    zero respectively.  Note that these values indicate
     *    that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 8 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    /* 8) Verify the contents of the data sets. */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

    if ( show_progress ) /* 9 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 9) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 10 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 10) Delete the file. */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }


    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* check_cache_image_ctl_flow_5() */


/*-------------------------------------------------------------------------
 * Function:    check_cache_image_ctl_flow_6()
 *
 * Purpose:     This test is one of a sequence of control flow tests intended
 *		to verify that control flow for the cache image feature works
 *		as expected.  
 *
 *		The objective of this test is verify correct control flow
 *		when a file with a metadata cache image superblock extension
 *		message is opened with the metadata cache image FAPL entry.
 *
 *		In this test we avoid all file activity other than open
 *		and close.
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set flags forcing creation of metadata cache image 
 *		   super block extension message only.
 *
 *		2) Close the file.
 *
 *		3) Open the file WITH the cache image FAPL entry.  
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set flags forcing creation of metadata cache image 
 *		   super block extension message only.
 *
 *		4) Close the file.
 *
 *		5) Open the file.
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image, and that the 
 *		   supplied address and length are HADDR_UNDEF and 
 *		   zero respectively.  Note that these values indicate
 *		   that the metadata image block doesn't exist.
 *
 *	        6) Close the file.
 *
 *	        7) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              7/17/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
check_cache_image_ctl_flow_6(void)
{
    const char * fcn_name = "check_cache_image_ctl_flow_6()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image control flow test 6");

    pass = TRUE;

    if ( show_progress ) /* 0 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) /* 1 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 1) Create a HDF5 file with the cache image FAPL entry.  
     *
     *    Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing creation of metadata cache image 
     *    super block extension message only.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__GEN_MDCI_SBE_MESG,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 2 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 2) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 3 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 4) Open the file WITH the cache image FAPL entry.  
     *
     *    Verify that the metadata cache is instructed
     *    to load the metadata cache image, and that the 
     *    supplied address and length are HADDR_UNDEF and 
     *    zero respectively.  Note that these values indicate
     *    that the metadata image block doesn't exist.
     *
     *    Verify that the cache is informed of the cache image 
     *    FAPL entry.
     *
     *    Set flags forcing creation of metadata cache image 
     *    super block extension message only.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__GEN_MDCI_SBE_MESG,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 4 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 5) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 5 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 5) Open the file. 
     *
     *    Verify that the metadata cache is instructed
     *    to load the metadata cache image, and that the 
     *    supplied address and length are HADDR_UNDEF and 
     *    zero respectively.  Note that these values indicate
     *    that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) /* 6 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) /* 7 */
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 7) Delete the file. */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }


    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* check_cache_image_ctl_flow_6() */


/*-------------------------------------------------------------------------
 * Function:    cache_image_smoke_check_1()
 *
 * Purpose:     This test is one of a sequence of tests intended
 *		to exercise the cache image feature verifying that it 
 *		works more or less correctly in common cases.
 *
 *		This test is an initial smoke check, so the sequence of 
 *		operations is relatively simple.  In particular, we are 
 *		testing:
 *
 *			i) Creation of file with metadata cache image 
 *			   superblock extension message and cache image 
 *			   block.
 *
 *		       ii) Open of file with metadata cache image superblock
 *			   extension message and cache image block.  
 *			   Deserialization and removal of both, insertion
 *			   of prefetched cache entries, and deserialization 
 *			   of prefetched cache entries as they are protected.
 *
 *		      iii) Subsequent write of file without metadata cache 
 *			   image.
 *
 *		       iv) Open and close of file with metadata cache image
 *			   image requested, but with no operations on the 
 *			   file.
 *
 *		To do this:
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set all cache image flags, forcing full functionality.
 *
 *		2) Create some data sets in the file. 
 *
 *		3) Close the file.
 *
 *		4) Open the file.  
 *
 *		   Verify that the metadata cache is instructed
 *		   to load the metadata cache image.
 *
 *		5) Open a data set.
 *
 *		   Verify that it contains the expected data
 *
 *		6) Close the file.
 *
 *		7) Open the file.
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *		8) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *		9) Close the file.
 *
 * 	       10) Open the file with the cache image FAPL entry.
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set all cache image flags, forcing full functionality.
 *
 *	       11) Close the file.  Since there has been no access to 
 *		   any entries that would be included in the cache image,
 *		   there should be no cache image.
 *
 *	       12) Open the file.
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *	       13) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *	       14) Close the file.
 *
 *	       15) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              8/17/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
cache_image_smoke_check_1(void)
{
    const char * fcn_name = "cache_image_smoke_check_1()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image smoke check 1");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file with the cache image FAPL entry. 
     *
     *	  Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing full function of the cache image feature.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ TRUE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Create some data sets in the file. */

    if ( pass ) {

        create_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */



    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 4) Open the file.  
     *
     *    Verify that the metadata cache is instructed to load the 
     *    metadata cache image, and that the supplied address and length 
     *    are HADDR_UNDEF and zero respectively.  Note that these values 
     *    indicate that the metadata image block doesn't exist.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 5) Open and close a data set.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 1 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block not loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 7) Open the file. 
     *
     *    Verify that the file doesn't contain a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 8) Open and close a data set.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 9) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 10) Open the file with the cache image FAPL entry.
     *
     *	   Verify that the cache is informed of the cache image 
     *	   FAPL entry.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 11) Close the file.  Since there has been no access to 
     *	   any entries that would be included in the cache image,
     *	   there should be no cache image.
     */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 12) Open the file.
     *
     *	   Verify that the file doesn't contain a metadata cache
     *	   image superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 13) Open a data set.
     *
     *	   Verify that it contains the expected data.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(3).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 14) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 15) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }

    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* cache_image_smoke_check_1() */


/*-------------------------------------------------------------------------
 * Function:    cache_image_smoke_check_2()
 *
 * Purpose:     This test is one of a sequence of tests intended
 *		to exercise the cache image feature verifying that it 
 *		works more or less correctly in common cases.
 *
 *		This test is an initial smoke check, so the sequence of 
 *		operations is relatively simple.  In particular, we are 
 *		testing:
 *
 *			i) Creation of file with metadata cache image 
 *			   superblock extension message and cache image 
 *			   block.
 *
 *		       ii) Open of file with metadata cache image superblock
 *			   extension message and cache image block.  Write
 *			   of prefetched entries to file on file close.
 *
 *		To do this:
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set all cache image flags, forcing full functionality.
 *
 *		2) Create some data sets in the file. 
 *
 *		3) Close the file.
 *
 *		4) Open the file.  
 *
 *		5) Close the file.
 *
 *		6) Open the file.
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *		7) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *		8) Close the file.
 *
 *	        9) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              8/18/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
cache_image_smoke_check_2(void)
{
    const char * fcn_name = "cache_image_smoke_check_2()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image smoke check 2");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file with the cache image FAPL entry. 
     *
     *	  Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing full function of the cache image feature.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ TRUE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Create some data sets in the file. */

    if ( pass ) {

        create_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 4) Open the file.  
     *
     *    Verify that the metadata cache is instructed to load the 
     *    metadata cache image.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    /* can't verify that metadata cache image has been loaded directly,
     * as in this cache, the load will not happen until close, and thus 
     * the images_created stat will not be readily available as it is 
     * incremented just before the cache is shut down.
     */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 5) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 6) Open the file. 
     *
     *    Verify that the file doesn't contain a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 7) Open and close a data set.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 8) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 9) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }

    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* cache_image_smoke_check_2() */


/*-------------------------------------------------------------------------
 * Function:    cache_image_smoke_check_3()
 *
 * Purpose:     This test is one of a sequence of tests intended
 *		to exercise the cache image feature verifying that it 
 *		works more or less correctly in common cases.
 *
 *		This test is an initial smoke check, so the sequence of 
 *		operations is relatively simple.  In particular, we are 
 *		testing:
 *
 *			i) Creation of file with metadata cache image 
 *			   superblock extension message and cache image 
 *			   block.
 *
 *		       ii) Read only open and close of file with metadata 
 *			   cache image superblock extension message and 
 *			   cache image block.
 *
 *		      iii) Subsequent R/W open and close of file with metadata 
 *                         cache image superblock extension message and 
 *                         cache image block.
 *
 *		To do this:
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set all cache image flags, forcing full functionality.
 *
 *		2) Create some data sets in the file. 
 *
 *		3) Close the file.
 *
 *		4) Open the file read only.  
 *
 *		   Verify that the file contains a metadata cache
 *		   image superblock extension message.
 *
 *		5 Open a data set.
 *
 *		  Verify that it contains the expected data.
 *
 *		6) Close the file.
 *
 *		7) Open the file.  
 *
 *		   Verify that the file contains a metadata cache
 *		   image superblock extension message.
 *
 *		8 Open a data set.
 *
 *		  Verify that it contains the expected data.
 *
 *		9) Close the file.
 *
 *	       10) Open the file.
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *	       11) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *	       12) Close the file.
 *
 *	       13) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              8/18/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
cache_image_smoke_check_3(void)
{
    const char * fcn_name = "cache_image_smoke_check_3()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image smoke check 3");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file with the cache image FAPL entry. 
     *
     *	  Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing full function of the cache image feature.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ TRUE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Create some data sets in the file. */

    if ( pass ) {

        create_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 4) Open the file read only.  
     *
     *    Verify that the metadata cache is instructed to load the 
     *    metadata cache image.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ TRUE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 5) Open and close a data set.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded == 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block not loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 7) Open the file. 
     *
     *    Verify that the file contains a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 8) Open and close a data set.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded == 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block not loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 9) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }
 
 
    /* 10) Open the file. 
     *
     *    Verify that the file doesn't contain a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 11) Open and close a data set.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 12) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 13) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }

    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* cache_image_smoke_check_3() */


/*-------------------------------------------------------------------------
 * Function:    cache_image_smoke_check_4()
 *
 * Purpose:     This test attempts to mimic the typical "poor man's 
 *		parallel use case in which the file is passed between
 *		processes, each of which open the file, write some data,
 *		close the file, and then pass control on to the next 
 *		process.
 *
 *		In this case, we only write one data set per process.
 *
 *		Cycle of operation
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set all cache image flags, forcing full functionality.
 *
 *		2) Create and write a data set in the file. 
 *
 *		3) Close the file.
 *
 *		4) Open the file with the cache image FAPL entry.
 *
 *		   Verify that the file contains a metadata cache
 *		   image superblock extension message.
 *
 *		5 Create and write a new data set
 *
 *		6) Close the file.
 *
 *		   If sufficient data sets have been created, continue to 
 *                 7).  Otherwise goto 4)
 *
 *		7) Open the file.  
 *
 *		   Verify that the file contains a metadata cache
 *		   image superblock extension message.
 *
 *	        8) Open all data sets that have been created, and 
 *		   verify that they contain the expected data.
 *
 *	        9) Close the file.
 *
 *	       10) Open the file.
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *	       11) Open all data sets that have been created, and
 *                 verify that they contain the expected data.
 *
 *		   Verify that it contains the expected data.
 *
 *	       12) Close the file.
 *
 *	       13) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              8/18/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
cache_image_smoke_check_4(void)
{
    const char * fcn_name = "cache_image_smoke_check_4()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;
    int min_dset = 0;
    int max_dset = 0;

    TESTING("metadata cache image smoke check 4");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file with the cache image FAPL entry. 
     *
     *	  Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing full function of the cache image feature.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ TRUE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Create a data set in the file. */

    if ( pass ) {

        create_data_sets(file_id, min_dset++, max_dset++);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    while ( ( pass ) && ( max_dset < MAX_NUM_DSETS ) )
    {
 
        /* 4) Open the file.  
         *
         *    Verify that the metadata cache is instructed to load the 
         *    metadata cache image.
         */

        if ( pass ) {

            open_hdf5_file(/* create_file        */ FALSE,
	 	           /* mdci_sbem_expected */ TRUE,
                           /* read_only          */ FALSE,
                           /* set_mdci_fapl      */ TRUE,
		           /* config_fsm         */ FALSE,
                           /* hdf_file_name      */ filename,
                           /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                           /* file_id_ptr        */ &file_id,
                           /* file_ptr_ptr       */ &file_ptr,
                           /* cache_ptr_ptr      */ &cache_ptr);
        }

        if ( show_progress ) 
            HDfprintf(stdout, "%s:L1 cp = %d, max_dset = %d, pass = %d.\n", 
                      fcn_name, cp, max_dset, pass);

 
        /* 5) Create a data set in the file. */

        if ( pass ) {

            create_data_sets(file_id, min_dset++, max_dset++);
        }

#if H5C_COLLECT_CACHE_STATS
        if ( pass ) {

            if ( cache_ptr->images_loaded == 0 ) {

                pass = FALSE;
                failure_mssg = "metadata cache image block not loaded(1).";
            }
        }
#endif /* H5C_COLLECT_CACHE_STATS */

        if ( show_progress ) 
            HDfprintf(stdout, "%s:L2 cp = %d, max_dset = %d, pass = %d.\n", 
                      fcn_name, cp + 1, max_dset, pass);

 
        /* 6) Close the file. */

        if ( pass ) {

            if ( H5Fclose(file_id) < 0  ) {

                pass = FALSE;
                failure_mssg = "H5Fclose() failed.\n";

            }
        }

        if ( show_progress ) 
            HDfprintf(stdout, "%s:L3 cp = %d, max_dset = %d, pass = %d.\n", 
                      fcn_name, cp + 2, max_dset, pass);
    } /* end while */
    cp += 3;
 
 
 
    /* 7) Open the file. 
     *
     *    Verify that the file contains a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 8) Open and close all data sets.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, max_dset - 1);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded == 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block not loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 9) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }
 
 
    /* 10) Open the file. 
     *
     *    Verify that the file doesn't contain a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 11) Open and close all data sets.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, max_dset - 1);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 12) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 13) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }

    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* cache_image_smoke_check_4() */


/*-------------------------------------------------------------------------
 * Function:    cache_image_smoke_check_5()
 *
 * Purpose:     This test attempts to mimic the typical "poor man's 
 *		parallel use case in which the file is passed between
 *		processes, each of which open the file, write some data,
 *		close the file, and then pass control on to the next 
 *		process.
 *
 *		In this case, we create one group for each process, and
 *		populate it with a "zoo" of HDF5 objects selected to 
 *		(ideally) exercise all HDF5 on disk data structures.
 *
 *		Cycle of operation
 *
 *		1) Create a HDF5 file with the cache image FAPL entry.  
 *
 *		   Verify that the cache is informed of the cache image 
 *		   FAPL entry.
 *
 *		   Set all cache image flags, forcing full functionality.
 *
 *		2) Create a process specific group.
 *
 *		3) Construct a "zoo" in the above group, and validate it.
 *
 *		4) Close the file.
 *
 *		5) Open the file with the cache image FAPL entry.
 *
 *		   Verify that the file contains a metadata cache
 *		   image superblock extension message.
 *
 *		6) Validate the "zoo" created in the previous file open.
 *
 *		7) Create a process specific group for this file open
 *
 *		8) Construct a "zoo" in the above group, and validate it.
 *		
 *		9) Close the file.
 *
 *		   If sufficient zoos have been created, continue to 
 *                 10).  Otherwise goto 5)
 *
 *	       10) Open the file.  
 *
 *		   Verify that the file contains a metadata cache
 *		   image superblock extension message.
 *
 *	       11) Validate all the zoos.
 *
 *	       12) Close the file.
 *
 *	       13) Open the file.
 *
 *		   Verify that the file doesn't contain a metadata cache
 *		   image superblock extension message.
 *
 *	       14) Validate all the zoos.
 *
 *	       15) Close the file.
 *
 *	       16) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              9/15/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

#define MAX_NUM_GROUPS 128

static unsigned
cache_image_smoke_check_5(void)
{
    const char * fcn_name = "cache_image_smoke_check_5()";
    char filename[512];
    char process_group_name[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    hid_t proc_gid = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;
    int i;
    int min_group = 0;
    int max_group = 0;

    TESTING("metadata cache image smoke check 5");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file with the cache image FAPL entry. 
     *
     *	  Verify that the cache is informed of the cache image FAPL entry.
     *
     *    Set flags forcing full function of the cache image feature.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ TRUE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 2) Create a process specific group. */
    if ( pass ) {

	sprintf(process_group_name, "/process_%d", min_group);

        proc_gid = H5Gcreate2(file_id, process_group_name, H5P_DEFAULT, 
                              H5P_DEFAULT, H5P_DEFAULT);

        if ( proc_gid < 0 ) {

	    pass = FALSE;
	    failure_mssg = "H5Gcreate2() failed (1).\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 3) Construct a "zoo" in the above group, and validate it. */
    if ( pass ) 
        create_zoo(file_id, process_group_name, min_group);

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 4) Close the file. */

    if ( pass ) {

	if ( H5Gclose(proc_gid) < 0 ) {
    
            pass = FALSE;
            failure_mssg = "H5Gclose(proc_gid) failed. (1)";
        }
    }

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

    while ( ( pass ) && ( max_group < MAX_NUM_GROUPS ) )
    {
 
        /* 5) Open the file.  
         *
         *    Verify that the metadata cache is instructed to load the 
         *    metadata cache image.
         */

        if ( pass ) {

            open_hdf5_file(/* create_file        */ FALSE,
	 	           /* mdci_sbem_expected */ TRUE,
                           /* read_only          */ FALSE,
                           /* set_mdci_fapl      */ TRUE,
		           /* config_fsm         */ FALSE,
                           /* hdf_file_name      */ filename,
                           /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                           /* file_id_ptr        */ &file_id,
                           /* file_ptr_ptr       */ &file_ptr,
                           /* cache_ptr_ptr      */ &cache_ptr);
        }

        if ( show_progress ) 
            HDfprintf(stdout, "%s:L1 cp = %d, max_group = %d, pass = %d.\n", 
                      fcn_name, cp, max_group, pass);


        /* 6) Validate the "zoo" created in the previous file open. */
        if ( pass ) 
            validate_zoo(file_id, process_group_name, max_group);

#if H5C_COLLECT_CACHE_STATS
        if ( pass ) {

            if ( cache_ptr->images_loaded == 0 ) {

                pass = FALSE;
                failure_mssg = "metadata cache image block not loaded(1).";
            }
        }
#endif /* H5C_COLLECT_CACHE_STATS */

        if ( show_progress ) 
            HDfprintf(stdout, "%s:L2 cp = %d, max_group = %d, pass = %d.\n", 
                      fcn_name, cp + 1, max_group, pass);


	/* 7) Create a process specific group for this file open */
        if ( pass ) {

	    max_group++;
	    sprintf(process_group_name, "/process_%d", max_group);

            proc_gid = H5Gcreate2(file_id, process_group_name, H5P_DEFAULT, 
                                  H5P_DEFAULT, H5P_DEFAULT);

            if ( proc_gid < 0 ) {

	        pass = FALSE;
	        failure_mssg = "H5Gcreate2() failed (2).\n";
            }
        }

        if ( show_progress ) 
            HDfprintf(stdout, "%s:L3 cp = %d, max_group = %d, pass = %d.\n", 
                      fcn_name, cp + 2, max_group, pass);
 

	/* 8) Construct a "zoo" in the above group, and validate it. */
        if ( pass ) 
            create_zoo(file_id, process_group_name, max_group);

        if ( show_progress ) 
            HDfprintf(stdout, "%s:L4 cp = %d, max_group = %d, pass = %d.\n", 
                      fcn_name, cp + 3, max_group, pass);

 
        /* 9) Close the file. */

        if ( pass ) {

	    if ( H5Gclose(proc_gid) < 0 ) {
    
                pass = FALSE;
                failure_mssg = "H5Gclose(process_gid) failed. (2)";
            }
        }

        if ( pass ) {

            if ( H5Fclose(file_id) < 0  ) {

                pass = FALSE;
                failure_mssg = "H5Fclose() failed.\n";

            }
        }

        if ( show_progress ) 
            HDfprintf(stdout, "%s:L5 cp = %d, max_group = %d, pass = %d.\n", 
                      fcn_name, cp + 4, max_group, pass);
    } /* end while */
    cp += 5;
 
 
 
    /* 10) Open the file. 
     *
     *    Verify that the file contains a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 11) Validate all the zoos. */
    i = min_group;
    while ( ( pass ) && ( i <= max_group ) ) {

	sprintf(process_group_name, "/process_%d", i);
        validate_zoo(file_id, process_group_name, i++);
    }
 
#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded == 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block not loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 12) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }
 
 
    /* 13) Open the file. 
     *
     *    Verify that the file doesn't contain a metadata cache image 
     *    superblock extension message.
     */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 14) Validate all the zoos.
     *
     *    Verify that the metadata cache image superblock 
     *    extension message has been deleted.
     */
    i = min_group;
    while ( ( pass ) && ( i <= max_group ) ) {

	sprintf(process_group_name, "/process_%d", i);
        validate_zoo(file_id, process_group_name, i++);
    }
 
#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 15) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 16) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }


    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* cache_image_smoke_check_5() */


/*-------------------------------------------------------------------------
 * Function:    cache_image_api_error_check_1()
 *
 * Purpose:     This test is one of a sequence of tests intended
 *		to verify correct management of API errors.
 *
 *		The object of this test is to verify that a file without
 *		a pre-existing cache image that is opened both read only
 *		and with a cache image requested is handle correctly
 *		(the cache image request should be ignored silently).
 *
 *		The test is set up as follows:
 *
 *		1) Create a HDF5 file.  
 *
 *		2) Create some data sets in the file. 
 *
 *		3) Close the file.
 *
 *		4) Open the file read only with a cache image FAPL entry
 *		   requested.  
 *
 *		5) Open a data set.
 *
 *		   Verify that it contains the expected data
 *
 *		   Verify that the cache image was not loaded.
 *
 *		6) Close the file.
 *
 *		7) Open the file read only.
 *
 *		8) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *		   Verify that the cache image was not loaded.
 *
 *		9) Close the file.
 *
 * 	       10) Open the file read write.
 *
 *	       11) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *	       12) Close the file.
 *
 *	       13) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              9/25/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
cache_image_api_error_check_1(void)
{
    const char * fcn_name = "cache_image_api_error_check_1()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image api error check 1");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ TRUE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Create some data sets in the file. */

    if ( pass ) {

        create_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */



    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 4) Open the file read only with a cache image FAPL entry requested. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ TRUE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 5) Open and close a data set.
     *
     *    Verify that it contains the expected data.
     *
     *    Verify that the cache image was not loaded.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 7) Open the file read only. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ TRUE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 8) Open and close a data set.
     *
     *    Verify that it contains the expected data.
     *
     *    Verify that the cache image was not loaded.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(3).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 9) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 10) Open the file read / write. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
    /* 11) Open and close a data set.
     *
     *     Verify that it contains the expected data.
     *
     *     Verify that the cache image was not loaded.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(4).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 12) Close the file.  */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 13) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }

    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* cache_image_api_error_check_1() */


/*-------------------------------------------------------------------------
 * Function:    cache_image_api_error_check_2()
 *
 * Purpose:     This test is one of a sequence of tests intended
 *		to verify correct management of API errors.
 *
 *		The object of this test is to verify that a file with
 *		a pre-existing cache image that is opened both read only
 *		and with a cache image requested is handled correctly
 *		(the cache image request should be ignored silently).
 *
 *		The test is set up as follows:
 *
 *		1) Create a HDF5 file with a cache image requested..  
 *
 *		2) Create some data sets in the file. 
 *
 *		3) Close the file.
 *
 *		4) Open the file read only with a cache image FAPL entry
 *		   requested.  
 *
 *		5) Open a data set.
 *
 *		   Verify that it contains the expected data
 *
 *		   Verify that the cache image was loaded.
 *
 *		6) Close the file.
 *
 *		7) Open the file read only.
 *
 *		8) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *		   Verify that the cache image was loaded.
 *
 *		9) Close the file.
 *
 * 	       10) Open the file read write.
 *
 *	       11) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *		   Verify that the cache image was loaded.
 *
 *	       12) Close the file.
 *
 * 	       13) Open the file read write.
 *
 *	       14) Open a data set.
 *
 *		   Verify that it contains the expected data.
 *
 *		   Verify that the cache image was NOT loaded.
 *
 *	       15) Close the file.
 *
 *	       16) Delete the file.
 *
 * Return:      void
 *
 * Programmer:  John Mainzer
 *              9/25/15
 *
 * Modifications:
 *
 *		None.
 *
 *-------------------------------------------------------------------------
 */

static unsigned
cache_image_api_error_check_2(void)
{
    const char * fcn_name = "cache_image_api_error_check_2()";
    char filename[512];
    hbool_t show_progress = FALSE;
    hid_t file_id = -1;
    H5F_t *file_ptr = NULL;
    H5C_t *cache_ptr = NULL;
    int cp = 0;

    TESTING("metadata cache image api error check 2");

    pass = TRUE;

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* setup the file name */
    if ( pass ) {

        if ( h5_fixname(FILENAMES[0], H5P_DEFAULT, filename, sizeof(filename))
            == NULL ) {

            pass = FALSE;
            failure_mssg = "h5_fixname() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 1) Create a HDF5 file with a cache image requested. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ TRUE,
                       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ TRUE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);

 
    /* 2) Create some data sets in the file. */

    if ( pass ) {

        create_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */



    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 3) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";

        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 4) Open the file read only with a cache image FAPL entry requested. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ TRUE,
                       /* set_mdci_fapl      */ TRUE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 5) Open and close a data set.
     *
     *    Verify that it contains the expected data.
     *
     *    Verify that the cache image was loaded.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 1 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block was not loaded(1).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 6) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 7) Open the file read only. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
		       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ TRUE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ 0,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
 
    /* 8) Open and close a data set.
     *
     *    Verify that it contains the expected data.
     *
     *    Verify that the cache image was loaded.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 1 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block was not loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */


    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 9) Close the file. */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 10) Open the file read / write. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ TRUE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
    /* 11) Open and close a data set.
     *
     *     Verify that it contains the expected data.
     *
     *     Verify that the cache image was loaded.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 1 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block was not loaded(3).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 12) Close the file.  */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);


    /* 13) Open the file read / write. */

    if ( pass ) {

        open_hdf5_file(/* create_file        */ FALSE,
	 	       /* mdci_sbem_expected */ FALSE,
                       /* read_only          */ FALSE,
                       /* set_mdci_fapl      */ FALSE,
		       /* config_fsm         */ FALSE,
                       /* hdf_file_name      */ filename,
                       /* cache_image_flags  */ H5C_CI__ALL_FLAGS,
                       /* file_id_ptr        */ &file_id,
                       /* file_ptr_ptr       */ &file_ptr,
                       /* cache_ptr_ptr      */ &cache_ptr);
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 
    /* 14) Open and close a data set.
     *
     *     Verify that it contains the expected data.
     *
     *     Verify that the cache image was not loaded.
     */

    if ( pass ) {

       verify_data_sets(file_id, 0, 5);
    }

#if H5C_COLLECT_CACHE_STATS
    if ( pass ) {

        if ( cache_ptr->images_loaded != 0 ) {

            pass = FALSE;
            failure_mssg = "metadata cache image block was loaded(2).";
        }
    }
#endif /* H5C_COLLECT_CACHE_STATS */

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 15) Close the file.  */

    if ( pass ) {

        if ( H5Fclose(file_id) < 0  ) {

            pass = FALSE;
            failure_mssg = "H5Fclose() failed.\n";
        }
    }

    if ( show_progress ) 
        HDfprintf(stdout, "%s: cp = %d, pass = %d.\n", fcn_name, cp++, pass);
 

    /* 13) Delete the file */

    if ( pass ) {

        if ( HDremove(filename) < 0 ) {

            pass = FALSE;
            failure_mssg = "HDremove() failed.\n";
        }
    }

    if ( pass ) { PASSED(); } else { H5_FAILED(); }

    if ( ! pass )
        HDfprintf(stdout, "%s: failure_mssg = \"%s\".\n",
                  FUNC, failure_mssg);

    return !pass;

} /* cache_image_api_error_check_2() */


/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Run tests on the cache code contained in H5C.c
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  John Mainzer
 *              6/24/04
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

int
main(void)
{
    unsigned nerrs = 0;
    int express_test;

    H5open();

    express_test = GetTestExpress();

    printf("=========================================\n");
    printf("Cache image tests\n");
    printf("        express_test = %d\n", express_test);
    printf("=========================================\n");

    nerrs += check_cache_image_ctl_flow_1();
    nerrs += check_cache_image_ctl_flow_2();
    nerrs += check_cache_image_ctl_flow_3();
    nerrs += check_cache_image_ctl_flow_4();
    nerrs += check_cache_image_ctl_flow_5();
    nerrs += check_cache_image_ctl_flow_6();

    nerrs += cache_image_smoke_check_1();
    nerrs += cache_image_smoke_check_2();
    nerrs += cache_image_smoke_check_3();
    nerrs += cache_image_smoke_check_4();
    nerrs += cache_image_smoke_check_5();

    nerrs += cache_image_api_error_check_1();
    nerrs += cache_image_api_error_check_2();

    return(nerrs > 0);

} /* main() */


