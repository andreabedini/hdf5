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
 * Programmer:	Raymond Lu
 *		13 February 2013
 *
 * Purpose:	Tests the plugin module (H5PL)
 */
#include <stdlib.h>
#include <time.h>

#include "h5test.h"
#include "H5srcdir.h"

/*
 * This file needs to access private datatypes from the H5Z package.
 */
#define H5Z_PACKAGE
#include "H5Zpkg.h"

const char *FILENAME[] = {
    "plugin",
    NULL
};
#define FILENAME_BUF_SIZE       1024
#define KB                      1024

#define FILE_DEFLATE_NAME       "deflate.h5"

/* Dataset names for testing filters */
#define DSET_DEFAULT_NAME	"default"
#define DSET_CHUNKED_NAME	"chunked"
#define DSET_COMPACT_NAME       "compact"
#define DSET_SIMPLE_IO_NAME	"simple_io"
#define DSET_USERBLOCK_IO_NAME	"userblock_io"
#define DSET_COMPACT_IO_NAME    "compact_io"
#define DSET_COMPACT_MAX_NAME   "max_compact"
#define DSET_COMPACT_MAX2_NAME   "max_compact_2"
#define DSET_CONV_BUF_NAME	"conv_buf"
#define DSET_TCONV_NAME		"tconv"
#define DSET_DEFLATE_NAME	"deflate"
#define DSET_BZIP2_NAME	        "bzip2"
#define DSET_DYNLIB1_NAME	"dynlib1"
#define DSET_DYNLIB2_NAME       "dynlib2"
#ifdef H5_HAVE_FILTER_SZIP
#define DSET_SZIP_NAME          "szip"
#endif /* H5_HAVE_FILTER_SZIP */
#define DSET_SHUFFLE_NAME	"shuffle"
#define DSET_FLETCHER32_NAME	"fletcher32"
#define DSET_FLETCHER32_NAME_2	"fletcher32_2"
#define DSET_FLETCHER32_NAME_3	"fletcher32_3"
#define DSET_SHUF_DEF_FLET_NAME	"shuffle+deflate+fletcher32"
#define DSET_SHUF_DEF_FLET_NAME_2	"shuffle+deflate+fletcher32_2"
#if defined H5_HAVE_FILTER_SZIP && defined H5_HAVE_FILTER_SHUFFLE && defined H5_HAVE_FILTER_FLETCHER32
#define DSET_SHUF_SZIP_FLET_NAME	"shuffle+szip+fletcher32"
#define DSET_SHUF_SZIP_FLET_NAME_2	"shuffle+szip+fletcher32_2"
#endif /* defined H5_HAVE_FILTER_SZIP && defined H5_HAVE_FILTER_SHUFFLE && defined H5_HAVE_FILTER_FLETCHER32 */

#define DSET_MISSING_NAME	"missing"
#define DSET_CAN_APPLY_NAME	"can_apply"
#define DSET_CAN_APPLY_NAME2	"can_apply2"
#ifdef H5_HAVE_FILTER_SZIP
#define DSET_CAN_APPLY_SZIP_NAME	"can_apply_szip"
#endif /* H5_HAVE_FILTER_SZIP */
#define DSET_SET_LOCAL_NAME	"set_local"
#define DSET_SET_LOCAL_NAME_2	"set_local_2"
#define DSET_ONEBYTE_SHUF_NAME	"onebyte_shuffle"
#define DSET_NBIT_INT_NAME             "nbit_int"
#define DSET_NBIT_FLOAT_NAME           "nbit_float"
#define DSET_NBIT_DOUBLE_NAME          "nbit_double"
#define DSET_NBIT_ARRAY_NAME           "nbit_array"
#define DSET_NBIT_COMPOUND_NAME        "nbit_compound"
#define DSET_NBIT_COMPOUND_NAME_2      "nbit_compound_2"
#define DSET_NBIT_COMPOUND_NAME_3      "nbit_compound_3"
#define DSET_NBIT_INT_SIZE_NAME        "nbit_int_size"
#define DSET_NBIT_FLT_SIZE_NAME        "nbit_flt_size"
#define DSET_SCALEOFFSET_INT_NAME      "scaleoffset_int"
#define DSET_SCALEOFFSET_INT_NAME_2    "scaleoffset_int_2"
#define DSET_SCALEOFFSET_FLOAT_NAME    "scaleoffset_float"
#define DSET_SCALEOFFSET_FLOAT_NAME_2  "scaleoffset_float_2"
#define DSET_SCALEOFFSET_DOUBLE_NAME   "scaleoffset_double"
#define DSET_SCALEOFFSET_DOUBLE_NAME_2 "scaleoffset_double_2"
#define DSET_COMPARE_DCPL_NAME		"compare_dcpl"
#define DSET_COMPARE_DCPL_NAME_2	"compare_dcpl_2"
#define DSET_COPY_DCPL_NAME_1		"copy_dcpl_1"
#define DSET_COPY_DCPL_NAME_2		"copy_dcpl_2"
#define COPY_DCPL_EXTFILE_NAME          "ext_file"
#define DSET_DEPREC_NAME		"deprecated"
#define DSET_DEPREC_NAME_CHUNKED	"deprecated_chunked"
#define DSET_DEPREC_NAME_COMPACT	"deprecated_compact"
#define DSET_DEPREC_NAME_FILTER         "deprecated_filter"

#define USER_BLOCK              1024
#define SIXTY_FOUR_KB           65536

/* Temporary filter IDs used for testing */

/* Flags for testing filters */
#define DISABLE_FLETCHER32      0
#define ENABLE_FLETCHER32       1
#define DATA_CORRUPTED          1
#define DATA_NOT_CORRUPTED      0

/* Parameters for the "set local" test */
#define BOGUS2_PERM_NPARMS      2       /* Number of "permanent" parameters */
#define BOGUS2_PARAM_1          13      /* (No particular meaning, just for checking value) */
#define BOGUS2_PARAM_2          35      /* (No particular meaning, just for checking value) */
#define BOGUS2_ALL_NPARMS       4       /* Total number of parameter = permanent + "local" parameters */

/* Dimensionality for conversion buffer test */
#define DIM1          100  /* Dim. Size of data member # 1 */
#define DIM2         5000  /* Dim. Size of data member # 2 */
#define DIM3           10  /* Dim. Size of data member # 3 */

/* Parameters for internal filter test */
#define FILTER_CHUNK_DIM1       2
#define FILTER_CHUNK_DIM2       25
#define FILTER_HS_OFFSET1       7
#define FILTER_HS_OFFSET2       30
#define FILTER_HS_SIZE1         4
#define FILTER_HS_SIZE2         50

/* Names for noencoder test */
#ifdef H5_HAVE_FILTER_SZIP
#define NOENCODER_FILENAME "noencoder.h5"
#define NOENCODER_COPY_FILENAME "noencoder.h5.copy"
#define NOENCODER_TEST_DATASET "noencoder_tdset.h5"
#define NOENCODER_SZIP_DATASET "noencoder_szip_dset.h5"
#define NOENCODER_SZIP_SHUFF_FLETCH_DATASET "noencoder_szip_shuffle_fletcher_dset.h5"
#endif /* H5_HAVE_FILTER_SZIP */

/* Names for zero-dim test */
#define ZERODIM_DATASET "zerodim"

/* Parameters for zero-dim test */
#define MISSING_CHUNK_DATASET   "missing_chunk"
#define MISSING_CHUNK_DIM       100

/* Names for random chunks test */
#define NPOINTS         50

/* Parameters for huge chunks test */
#define HUGE_DATASET            "Dataset"
#define HUGE_DIM                ((hsize_t)16 * 1024 * 1024 * 1024)
#define HUGE_CHUNK_DIM          ((hsize_t)2 * 1024 * 1024 * 1024)
#define TOO_HUGE_CHUNK_DIM      ((hsize_t)4 * 1024 * 1024 * 1024)
#define HUGE_DATASET2           "Dataset2"
#define HUGE_DIM2_0             ((hsize_t)16 * 1024)
#define HUGE_DIM2_1             ((hsize_t)16 * 1024)
#define HUGE_DIM2_2             ((hsize_t)16 * 1024)
#define HUGE_CHUNK_DIM2_0       ((hsize_t)2 * 1024)
#define HUGE_CHUNK_DIM2_1       ((hsize_t)1024)
#define HUGE_CHUNK_DIM2_2       ((hsize_t)1024)
#define TOO_HUGE_CHUNK_DIM2_0   ((hsize_t)4 * 1024)
#define TOO_HUGE_CHUNK_DIM2_1   ((hsize_t)1024)
#define TOO_HUGE_CHUNK_DIM2_2   ((hsize_t)1024)

/* Parameters for testing bypassing chunk cache */
#define BYPASS_DATASET1          "Dset1"
#define BYPASS_DATASET2          "Dset2"
#define BYPASS_DIM               1000
#define BYPASS_CHUNK_DIM         500
#define BYPASS_FILL_VALUE        7

/* Shared global arrays */
#define DSET_DIM1       100
#define DSET_DIM2       200

int	points_deflate[DSET_DIM1][DSET_DIM2], 
        points_dynlib1[DSET_DIM1][DSET_DIM2],
        points_dynlib2[DSET_DIM1][DSET_DIM2],
        points_bzip2[DSET_DIM1][DSET_DIM2];


/*-------------------------------------------------------------------------
 * Function:	test_filter_internal
 *
 * Purpose:	Tests 
 *
 * Return:	Success:	0
 *		Failure:	-1
 *
 * Programmer:	Raymond Lu
 *              27 February 2013
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_filter_internal(hid_t fid, const char *name, hid_t dcpl, hsize_t *dset_size)
{
    hid_t		dataset;        /* Dataset ID */
    hid_t		dxpl;           /* Dataset xfer property list ID */
    hid_t		write_dxpl;     /* Dataset xfer property list ID for writing */
    hid_t		sid;            /* Dataspace ID */
    const hsize_t	size[2] = {DSET_DIM1, DSET_DIM2};           /* Dataspace dimensions */
    const hsize_t	hs_offset[2] = {FILTER_HS_OFFSET1, FILTER_HS_OFFSET2}; /* Hyperslab offset */
    const hsize_t	hs_size[2] = {FILTER_HS_SIZE1, FILTER_HS_SIZE2};   /* Hyperslab size */
    void		*tconv_buf = NULL;      /* Temporary conversion buffer */
    int	                points[DSET_DIM1][DSET_DIM2], check[DSET_DIM1][DSET_DIM2];
    size_t		i, j;        /* Local index variables */
    int                 n = 0;
    herr_t              status;         /* Error status */

    /* Create the data space */
    if((sid = H5Screate_simple(2, size, NULL)) < 0) goto error;

    /*
     * Create a small conversion buffer to test strip mining. We
     * might as well test all we can!
     */
    if((dxpl = H5Pcreate(H5P_DATASET_XFER)) < 0) goto error;
    tconv_buf = HDmalloc((size_t)1000);
    if(H5Pset_buffer(dxpl, (size_t)1000, tconv_buf, NULL) < 0) goto error;
    if((write_dxpl = H5Pcopy(dxpl)) < 0) TEST_ERROR;

    TESTING("    filters (setup)");

    /* Check if all the filters are available */
    if(H5Pall_filters_avail(dcpl)!=TRUE) {
        H5_FAILED();
        printf("    Line %d: Incorrect filter availability\n",__LINE__);
        goto error;
    } /* end if */

    /* Create the dataset */
    if((dataset = H5Dcreate2(fid, name, H5T_NATIVE_INT, sid, H5P_DEFAULT,
			     dcpl, H5P_DEFAULT)) < 0) goto error;

    PASSED();

    /*----------------------------------------------------------------------
     * STEP 1: Read uninitialized data.  It should be zero.
     *----------------------------------------------------------------------
     */
    TESTING("    filters (uninitialized read)");

    if(H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, dxpl, check) < 0)
	TEST_ERROR;

    for(i=0; i<(size_t)size[0]; i++) {
	for(j=0; j<(size_t)size[1]; j++) {
	    if(0!=check[i][j]) {
		H5_FAILED();
		printf("    Read a non-zero value.\n");
		printf("    At index %lu,%lu\n",
		       (unsigned long)i, (unsigned long)j);
		goto error;
	    }
	}
    }
    PASSED();

    /*----------------------------------------------------------------------
     * STEP 2: Test filters by setting up a chunked dataset and writing
     * to it.
     *----------------------------------------------------------------------
     */
    TESTING("    filters (write)");

    for(i=n=0; i<size[0]; i++) {
	for(j=0; j<size[1]; j++) {
	    points[i][j] = (int)(n++);
	}
    }

    if(H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, write_dxpl, points) < 0)
	TEST_ERROR;

    if((*dset_size=H5Dget_storage_size(dataset))==0) TEST_ERROR;

    PASSED();

    /*----------------------------------------------------------------------
     * STEP 3: Try to read the data we just wrote.
     *----------------------------------------------------------------------
     */
    TESTING("    filters (read)");

    /* Read the dataset back */
        if(H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, dxpl, check) < 0)
	   TEST_ERROR;

        /* Check that the values read are the same as the values written */
        for(i=0; i<size[0]; i++) {
	   for(j=0; j<size[1]; j++) {
	       if(points[i][j] != check[i][j]) {
		  H5_FAILED();
		  fprintf(stderr,"    Read different values than written.\n");
		  fprintf(stderr,"    At index %lu,%lu\n", (unsigned long)i, (unsigned long)j);
		  fprintf(stderr,"    At original: %d\n", (int)points[i][j]);
		  fprintf(stderr,"    At returned: %d\n", (int)check[i][j]);
		  goto error;
	       }
	   }
        }

    PASSED();

    /*----------------------------------------------------------------------
     * STEP 4: Write new data over the top of the old data.  The new data is
     * random thus not very compressible, and will cause the chunks to move
     * around as they grow.  We only change values for the left half of the
     * dataset although we rewrite the whole thing.
     *----------------------------------------------------------------------
     */
    TESTING("    filters (modify)");

    for(i=0; i<size[0]; i++) {
	for(j=0; j<size[1]/2; j++) {
	    /*points[i][j] = (int)7;*/
	    points[i][j] = (int)HDrandom ();
	}
    }
    if(H5Dwrite (dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, write_dxpl, points) < 0)
	TEST_ERROR;

        /* Read the dataset back and check it */
        if(H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, dxpl, check) < 0)
	   TEST_ERROR;

        /* Check that the values read are the same as the values written */
        for(i=0; i<size[0]; i++) {
	   for(j=0; j<size[1]; j++) {
	       if(points[i][j] != check[i][j]) {
		  H5_FAILED();
		  printf("    Read different values than written.\n");
		  printf("    At index %lu,%lu\n",
		           (unsigned long)i, (unsigned long)j);
		  goto error;
	       }
	   }
        }

    if((*dset_size=H5Dget_storage_size(dataset))==0) TEST_ERROR;
    PASSED();

    /*----------------------------------------------------------------------
     * STEP 5: Close the dataset and then open it and read it again.  This
     * insures that the filters message is picked up properly from the
     * object header.
     *----------------------------------------------------------------------
     */
    TESTING("    filters (re-open)");

    if(H5Dclose(dataset) < 0) TEST_ERROR;
    if((dataset = H5Dopen2(fid, name, H5P_DEFAULT)) < 0) TEST_ERROR;

        if(H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, dxpl, check) < 0)
            TEST_ERROR;

        /* Check that the values read are the same as the values written */
        for(i = 0; i < size[0]; i++)
	   for(j = 0; j < size[1]; j++)
	       if(points[i][j] != check[i][j]) {
		  H5_FAILED();
		  printf("    Read different values than written.\n");
		  printf("    At index %lu,%lu\n",
		        (unsigned long)i, (unsigned long)j);
		  goto error;
	       } /* end if */

    PASSED();

    /*----------------------------------------------------------------------
     * STEP 6: Test partial I/O by writing to and then reading from a
     * hyperslab of the dataset.  The hyperslab does not line up on chunk
     * boundaries (we know that case already works from above tests).
     *----------------------------------------------------------------------
     */
    TESTING("    filters (partial I/O)");

    for(i=0; i<(size_t)hs_size[0]; i++) {
	for(j=0; j<(size_t)hs_size[1]; j++) {
	    points[(size_t)hs_offset[0]+i][(size_t)hs_offset[1]+j] = (int)HDrandom();
	}
    }
    if(H5Sselect_hyperslab(sid, H5S_SELECT_SET, hs_offset, NULL, hs_size,
			    NULL) < 0) TEST_ERROR;
    /* (Use the "read" DXPL because partial I/O on corrupted data test needs to ignore errors during writing) */
    if(H5Dwrite (dataset, H5T_NATIVE_INT, sid, sid, dxpl, points) < 0)
	TEST_ERROR;

        if(H5Dread (dataset, H5T_NATIVE_INT, sid, sid, dxpl, check) < 0)
	   TEST_ERROR;

        /* Check that the values read are the same as the values written */
        for(i=0; i<(size_t)hs_size[0]; i++) {
	   for(j=0; j<(size_t)hs_size[1]; j++) {
	       if(points[(size_t)hs_offset[0]+i][(size_t)hs_offset[1]+j] !=
                      check[(size_t)hs_offset[0]+i][(size_t)hs_offset[1]+j]) {
		  H5_FAILED();
		  fprintf(stderr,"    Read different values than written.\n");
		  fprintf(stderr,"    At index %lu,%lu\n",
		         (unsigned long)((size_t)hs_offset[0]+i),
		         (unsigned long)((size_t)hs_offset[1]+j));
		  fprintf(stderr,"    At original: %d\n",
		         (int)points[(size_t)hs_offset[0]+i][(size_t)hs_offset[1]+j]);
		  fprintf(stderr,"    At returned: %d\n",
		         (int)check[(size_t)hs_offset[0]+i][(size_t)hs_offset[1]+j]);
		  goto error;
	       }
	   }
        }

    PASSED();

    for(i=0; i<size[0]; i++) {
        for(j=0; j<size[1]; j++) {
            if(!HDstrcmp(name, DSET_DEFLATE_NAME)) {
	        points_deflate[i][j] = points[i][j];
            } else if(!HDstrcmp(name, DSET_DYNLIB1_NAME)) {
	        points_dynlib1[i][j] = points[i][j];
            } else if(!HDstrcmp(name, DSET_DYNLIB2_NAME)) {
	        points_dynlib2[i][j] = points[i][j];
            } else if(!HDstrcmp(name, DSET_BZIP2_NAME)) {
	        points_bzip2[i][j] = points[i][j];
            }
	}
    }

    /* Get the storage size of the dataset */
    if((*dset_size=H5Dget_storage_size(dataset))==0) goto error;
    /* Clean up objects used for this test */
    if(H5Dclose (dataset) < 0) goto error;
    if(H5Sclose (sid) < 0) goto error;
    if(H5Pclose (dxpl) < 0) goto error;
    free (tconv_buf);

    return(0);

error:
    if(tconv_buf)
        free (tconv_buf);
    return -1;
}

/*-------------------------------------------------------------------------
 * Function:	test_filters
 *
 * Purpose:	Tests dataset filter.
 *
 * Return:	Success:	0
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Wednesday, April 15, 1998
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_filters(hid_t file, hid_t fapl)
{
    hid_t	dc;                 /* Dataset creation property list ID */
    const hsize_t chunk_size[2] = {FILTER_CHUNK_DIM1, FILTER_CHUNK_DIM2};  /* Chunk dimensions */
    hsize_t     null_size;          /* Size of dataset with null filter */
    int         compress_level = 9;

#ifdef H5_HAVE_FILTER_DEFLATE
    hsize_t     deflate_size;       /* Size of dataset with deflate filter */
#endif /* H5_HAVE_FILTER_DEFLATE */

    /*----------------------------------------------------------
     * STEP 2: Test deflation by itself.
     *----------------------------------------------------------
     */
#ifdef H5_HAVE_FILTER_DEFLATE
    puts("Testing deflate filter");
    if((dc = H5Pcreate(H5P_DATASET_CREATE)) < 0) goto error;
    if(H5Pset_chunk (dc, 2, chunk_size) < 0) goto error;
    if(H5Pset_deflate (dc, 6) < 0) goto error;

    if(test_filter_internal(file,DSET_DEFLATE_NAME,dc,&deflate_size) < 0) goto error;
    /* Clean up objects used for this test */
    if(H5Pclose (dc) < 0) goto error;
#else /* H5_HAVE_FILTER_DEFLATE */
    TESTING("deflate filter");
    SKIPPED();
    puts("    Deflate filter not enabled");
#endif /* H5_HAVE_FILTER_DEFLATE */

    /*----------------------------------------------------------
     * STEP 2: Test DYNLIB1 by itself.
     *----------------------------------------------------------
     */
    puts("Testing DYNLIB1 filter");
    if((dc = H5Pcreate(H5P_DATASET_CREATE)) < 0) goto error;
    if(H5Pset_chunk (dc, 2, chunk_size) < 0) goto error;
    if(H5Pset_filter (dc, H5Z_FILTER_DYNLIB1, H5Z_FLAG_MANDATORY, 1, &compress_level) < 0) goto error;

    if(test_filter_internal(file,DSET_DYNLIB1_NAME,dc,&deflate_size) < 0) goto error;
    /* Clean up objects used for this test */
    if(H5Pclose (dc) < 0) goto error;
    /* Unregister the dynamic filter DYNLIB1 for testing purpose. The next time when this test is run for 
     * the new file format, the library's H5PL code has to search in the table of loaded plugin libraries
     * for this filter. */
    if(H5Zunregister(H5Z_FILTER_DYNLIB1) < 0) goto error;

#ifdef TMP
    /*----------------------------------------------------------
     * STEP 2: Test Bogus2 by itself.
     *----------------------------------------------------------
     */
    puts("Testing DYNLIB2 filter");
    if((dc = H5Pcreate(H5P_DATASET_CREATE)) < 0) goto error;
    if(H5Pset_chunk (dc, 2, chunk_size) < 0) goto error;
    if(H5Pset_filter (dc, H5Z_FILTER_DYNLIB2, H5Z_FLAG_MANDATORY, 1, &compress_level) < 0) goto error;

    if(test_filter_internal(file,DSET_DYNLIB2_NAME,dc,&deflate_size) < 0) goto error;
    /* Clean up objects used for this test */
    if(H5Pclose (dc) < 0) goto error;
    /* Unregister the dynamic filter BOGUS for testing purpose. The next time when this test is run for 
     * the new file format, the library's H5PL code has to search in the table of loaded plugin libraries
     * for this filter. */
    if(H5Zunregister(H5Z_FILTER_DYNLIB2) < 0) goto error;


    /*----------------------------------------------------------
     * STEP 3: Test BZIP2 by itself.
     *----------------------------------------------------------
     */
    puts("Testing BZIP2 filter");
    if((dc = H5Pcreate(H5P_DATASET_CREATE)) < 0) goto error;
    if(H5Pset_chunk (dc, 2, chunk_size) < 0) goto error;
    if(H5Pset_filter (dc, H5Z_FILTER_BZIP2, H5Z_FLAG_MANDATORY, 1, &compress_level) < 0) goto error;

    if(test_filter_internal(file,DSET_BZIP2_NAME,dc,&deflate_size) < 0) goto error;
    /* Clean up objects used for this test */
    if(H5Pclose (dc) < 0) goto error;
    /* Unregister the dynamic filter BOGUS for testing purpose. The next time when this test is run for 
     * the new file format, the library's H5PL code has to search in the table of loaded plugin libraries
     * for this filter. */
    if(H5Zunregister(H5Z_FILTER_BZIP2) < 0) goto error;
#endif

    return 0;

error:
    return -1;
}

/*-------------------------------------------------------------------------
 * Function:	test_read_data
 *
 * Purpose:	Tests dataset filter.
 *
 * Return:	Success:	0
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Wednesday, April 15, 1998
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_read_data(hid_t dataset, int *origin_data)
{
    int	                check[DSET_DIM1][DSET_DIM2];
    const hsize_t	size[2] = {DSET_DIM1, DSET_DIM2};           /* Dataspace dimensions */
    int                 *data_p = origin_data;
    size_t		i, j;        /* Local index variables */

    /* Read the dataset back */
    if(H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, check) < 0)
        TEST_ERROR;

    /* Check that the values read are the same as the values written */
    for(i=0; i<size[0]; i++) {
        for(j=0; j<size[1]; j++) {
	   if(*data_p != check[i][j]) {
	       H5_FAILED();
	       fprintf(stderr,"    Read different values than written.\n");
	       fprintf(stderr,"    At index %lu,%lu\n", (unsigned long)i, (unsigned long)j);
	       fprintf(stderr,"    At original: %d\n", *data_p);
	       fprintf(stderr,"    At returned: %d\n", (int)check[i][j]);
	       goto error;
	   }
           data_p++;
	}
    }

    PASSED();
    return 0;

error:
    return -1;
}

/*-------------------------------------------------------------------------
 * Function:	test_read_with_filters
 *
 * Purpose:	Tests dataset filter.
 *
 * Return:	Success:	0
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Wednesday, April 15, 1998
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_read_with_filters(hid_t file, hid_t fapl)
{
    hid_t	dset;                 /* Dataset ID */
    hsize_t     null_size;          /* Size of dataset with null filter */
    int         compress_level = 9;

#ifdef H5_HAVE_FILTER_DEFLATE
    hsize_t     deflate_size;       /* Size of dataset with deflate filter */
#endif /* H5_HAVE_FILTER_DEFLATE */

    /*----------------------------------------------------------
     * STEP 2: Test deflation by itself.
     *----------------------------------------------------------
     */
#ifdef H5_HAVE_FILTER_DEFLATE
    TESTING("Testing deflate filter");

    if(H5Zfilter_avail(H5Z_FILTER_DEFLATE) != TRUE) TEST_ERROR

    if((dset = H5Dopen2(file,DSET_DEFLATE_NAME,H5P_DEFAULT)) < 0) TEST_ERROR

    if(test_read_data(dset, points_deflate) < 0) TEST_ERROR

    if(H5Dclose(dset) < 0) TEST_ERROR

    /* Clean up objects used for this test */
#else /* H5_HAVE_FILTER_DEFLATE */
    TESTING("deflate filter");
    SKIPPED();
    puts("    Deflate filter not enabled");
#endif /* H5_HAVE_FILTER_DEFLATE */

    /*----------------------------------------------------------
     * STEP 2: Test DYNLIB1 by itself.
     *----------------------------------------------------------
     */
    TESTING("Testing DYNLIB1 filter");

    if((dset = H5Dopen2(file,DSET_DYNLIB1_NAME,H5P_DEFAULT)) < 0) TEST_ERROR

    if(test_read_data(dset, points_dynlib1) < 0) TEST_ERROR

    if(H5Dclose(dset) < 0) TEST_ERROR

#ifdef TMP
    /*----------------------------------------------------------
     * STEP 2: Test Bogus2 by itself.
     *----------------------------------------------------------
     */
    TESTING("Testing DYNLIB2 filter");

    if((dset = H5Dopen2(file,DSET_DYNLIB2_NAME,H5P_DEFAULT)) < 0) TEST_ERROR

    if(test_read_data(dset, points_dynlib2) < 0) TEST_ERROR

    if(H5Dclose(dset) < 0) TEST_ERROR

    /*----------------------------------------------------------
     * STEP 3: Test BZIP2 by itself.
     *----------------------------------------------------------
     */
    TESTING("Testing BZIP2 filter");

    if((dset = H5Dopen2(file,DSET_BZIP2_NAME,H5P_DEFAULT)) < 0) TEST_ERROR

    if(test_read_data(dset, points_bzip2) < 0) TEST_ERROR

    if(H5Dclose(dset) < 0) TEST_ERROR

#endif

    return 0;

error:
    return -1;
}


/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	Tests the dataset interface (H5D)
 *
 * Return:	Success:	exit(0)
 *
 *		Failure:	exit(1)
 *
 * Programmer:	Robb Matzke
 *		Tuesday, December  9, 1997
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    char		filename[FILENAME_BUF_SIZE];
    hid_t		file, grp, fapl, fapl2;
    hbool_t new_format;
    int mdc_nelmts;
    size_t rdcc_nelmts;
    size_t rdcc_nbytes;
    double rdcc_w0;
    int	nerrors = 0;
    const char *envval;

    /* Don't run this test using certain file drivers */
    envval = HDgetenv("HDF5_DRIVER");
    if(envval == NULL)
        envval = "nomatch";

    /* Set the random # seed */
    HDsrandom((unsigned)HDtime(NULL));

    /* Testing setup */
    h5_reset();
    fapl = h5_fileaccess();

    /* Turn off the chunk cache, so all the chunks are immediately written to disk */
    if(H5Pget_cache(fapl, &mdc_nelmts, &rdcc_nelmts, &rdcc_nbytes, &rdcc_w0) < 0)
        TEST_ERROR 
    rdcc_nbytes = 0;
    if(H5Pset_cache(fapl, mdc_nelmts, rdcc_nelmts, rdcc_nbytes, rdcc_w0) < 0)
        TEST_ERROR 

    /* Copy the file access property list */
    if((fapl2 = H5Pcopy(fapl)) < 0) TEST_ERROR

    /* Set the "use the latest version of the format" bounds for creating objects in the file */
    if(H5Pset_libver_bounds(fapl2, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) TEST_ERROR

    h5_fixname(FILENAME[0], fapl, filename, sizeof filename);

    /* Test with old & new format groups */
    for(new_format = FALSE; new_format <= TRUE; new_format++) {
        hid_t my_fapl;

        /* Set the FAPL for the type of format */
        if(new_format) {
            puts("\nTesting with new file format:");
            my_fapl = fapl2;
        } /* end if */
        else {
            puts("Testing with old file format:");
            my_fapl = fapl;
        } /* end else */

        /* Create the file for this test */
        if((file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, my_fapl)) < 0)
            TEST_ERROR 

        /* Cause the library to emit initial messages */
        if((grp = H5Gcreate2(file, "emit diagnostics", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
            TEST_ERROR 
        if(H5Oset_comment(grp, "Causes diagnostic messages to be emitted") < 0)
            TEST_ERROR 
        if(H5Gclose(grp) < 0)
            TEST_ERROR 

        nerrors += (test_filters(file, my_fapl) < 0		? 1 : 0);

        if(H5Fclose(file) < 0)
            TEST_ERROR 
    } /* end for */

    /* Close FAPL */
    if(H5Pclose(fapl2) < 0) TEST_ERROR
    if(H5Pclose(fapl) < 0) TEST_ERROR
   
    puts("\nTesting reading data with with dynamic plugin filters:");

    /* Close the library so that all loaded plugin libraries are unloaded */
    h5_reset();
    fapl = h5_fileaccess();

    /* Reopen the file for testing data reading */
    if((file = H5Fopen(filename, H5F_ACC_RDONLY, fapl)) < 0)
        TEST_ERROR 

    nerrors += (test_read_with_filters(file, fapl) < 0		? 1 : 0);

    if(H5Fclose(file) < 0)
        TEST_ERROR 

    if(nerrors)
        TEST_ERROR 
    printf("All dataset tests passed.\n");
    h5_cleanup(FILENAME, fapl);

    return 0;

error:
    nerrors = MAX(1, nerrors);
    printf("***** %d DATASET TEST%s FAILED! *****\n",
            nerrors, 1 == nerrors ? "" : "S");
    return 1;
}
