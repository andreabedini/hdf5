/*
 * Copyright (C) 1998 NCSA
 *                    All rights reserved.
 *
 * Programmer:  Robb Matzke <robb@arborea.spizella.com>
 *              Thursday, May 14, 1998
 *
 * Purpose:	Checks the effect of various I/O request sizes and raw data
 *		cache sizes.  Performance depends on the amount of data read
 *		from disk and we use a filter to get that number.

 */
#include <assert.h>
#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <H5config.h>
#ifndef HAVE_ATTRIBUTE
#   undef __attribute__
#   define __attribute__(X) /*void*/
#   define __unused__ /*void*/
#else
#   define __unused__ __attribute__((unused))
#endif

#define FILE_NAME	"chunk.h5"
#define LINESPOINTS	"lines"
#define CH_SIZE		100		/*squared in terms of bytes    	*/
#define DS_SIZE		20		/*squared in terms of chunks	*/
#define FILTER_COUNTER	305
#define READ		0
#define WRITE		1
#define MIN(X,Y)	((X)<(Y)?(X):(Y))
#define MAX(X,Y)	((X)>(Y)?(X):(Y))
#define SQUARE(X)	((X)*(X))

/* The row-major test */
#define RM_CACHE_STRT	25
#define RM_CACHE_END	25
#define RM_CACHE_DELT	5
#define RM_START	0.20
#define RM_END	        5.00
#define RM_DELTA	0.0025

/* Diagonal test */
#define DIAG_CACHE_STRT	25
#define DIAG_CACHE_END	25
#define DIAG_CACHE_DELT	5
#define DIAG_START	0.20
#define DIAG_END	5.00
#define DIAG_DELTA	0.0025

static size_t	nio_g;
static hid_t	fapl_g = -1;


/*-------------------------------------------------------------------------
 * Function:	counter
 *
 * Purpose:	Count number of bytes but don't do anything.
 *
 * Return:	Success:	src_nbytes-1
 *
 *		Failure:	never fails
 *
 * Programmer:	Robb Matzke
 *              Thursday, May 14, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static size_t
counter (unsigned __unused__ flags, size_t __unused__ cd_nelmts,
	 const unsigned __unused__ *cd_values, size_t nbytes,
	 size_t __unused__ *buf_size, void __unused__ **buf)
{
    nio_g += nbytes;
    return nbytes;
}


/*-------------------------------------------------------------------------
 * Function:	create_dataset
 *
 * Purpose:	Creates a square dataset with square chunks, registers a
 *		stupid compress/uncompress pair for counting I/O, and
 *		initializes the dataset.  The chunk size is in bytes, the
 *		dataset size is in terms of chunks.
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *              Thursday, May 14, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
create_dataset (void)
{
    hid_t	file, space, dcpl, dset;
    hsize_t	size[2];
    char	*buf;

    /* The file */
    file = H5Fcreate (FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_g);

    /* The data space */
    size[0] = size[1] = DS_SIZE * CH_SIZE;
    space = H5Screate_simple (2, size, size);

    /* The storage layout and compression */
    dcpl = H5Pcreate (H5P_DATASET_CREATE);
    size[0] = size[1] = CH_SIZE;
    H5Pset_chunk (dcpl, 2, size);
    H5Zregister (FILTER_COUNTER, "counter", counter);
    H5Pset_filter (dcpl, FILTER_COUNTER, 0, 0, NULL);
        
    /* The dataset */
    dset = H5Dcreate (file, "dset", H5T_NATIVE_CHAR, space, dcpl);
    assert (dset>=0);

    /* The data */
    buf = calloc (1, SQUARE (DS_SIZE*CH_SIZE));
    H5Dwrite (dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
    free (buf);

    /* Close */
    H5Dclose (dset);
    H5Sclose (space);
    H5Pclose (dcpl);
    H5Fclose (file);
}


/*-------------------------------------------------------------------------
 * Function:	test_rowmaj
 *
 * Purpose:	Reads the entire dataset using the specified size-squared
 *		I/O requests in row major order.
 *
 * Return:	Efficiency: data requested divided by data actually read.
 *
 * Programmer:	Robb Matzke
 *              Thursday, May 14, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static double
test_rowmaj (int op, hsize_t cache_size, hsize_t io_size)
{
    hid_t	file, dset, mem_space, file_space;
    char	*buf = calloc (1, SQUARE(io_size));
    hsize_t	i, j, hs_size[2];
    hssize_t	hs_offset[2];
    int		mdc_nelmts;
    double	w0;

    H5Pget_cache (fapl_g, &mdc_nelmts, NULL, &w0);
    H5Pset_cache (fapl_g, mdc_nelmts, cache_size*SQUARE (CH_SIZE), w0);
    file = H5Fopen (FILE_NAME, H5F_ACC_RDWR, fapl_g);
    dset = H5Dopen (file, "dset");
    file_space = H5Dget_space (dset);
    nio_g = 0;

    for (i=0; i<CH_SIZE*DS_SIZE; i+=io_size) {
#if 0
	fprintf (stderr, "%5d\b\b\b\b\b", (int)i);
	fflush (stderr);
#endif
	for (j=0; j<CH_SIZE*DS_SIZE; j+=io_size) {
	    hs_offset[0] = i;
	    hs_size[0] = MIN (io_size, CH_SIZE*DS_SIZE-i);
	    hs_offset[1] = j;
	    hs_size[1] = MIN (io_size, CH_SIZE*DS_SIZE-j);
	    mem_space = H5Screate_simple (2, hs_size, hs_size);
	    H5Sselect_hyperslab (file_space, H5S_SELECT_SET, hs_offset,
				 NULL, hs_size, NULL);

	    if (READ==op) {
		H5Dread (dset, H5T_NATIVE_CHAR, mem_space, file_space,
			 H5P_DEFAULT, buf);
	    } else {
		H5Dwrite (dset, H5T_NATIVE_CHAR, mem_space, file_space,
			  H5P_DEFAULT, buf);
	    }
	    H5Sclose (mem_space);
	}
    }
    
    free (buf); 
    H5Sclose (file_space);
    H5Dclose (dset);
    H5Fclose (file);

    return (double)SQUARE(CH_SIZE*DS_SIZE)/(double)nio_g;
}


/*-------------------------------------------------------------------------
 * Function:	test_diag
 *
 * Purpose:	Reads windows diagonally across the dataset.  Each window is
 *		offset from the previous window by OFFSET in the x and y
 *		directions.  The reading ends after the (k,k) value is read
 *		where k is the maximum index in the dataset.
 *
 * Return:	Efficiency.
 *
 * Programmer:	Robb Matzke
 *              Friday, May 15, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static double
test_diag (int op, hsize_t cache_size, hsize_t io_size, hsize_t offset)
{
    hid_t	file, dset, mem_space, file_space;
    hsize_t	i, hs_size[2], nio=0;
    hssize_t	hs_offset[2];
    char	*buf = calloc (1, SQUARE (io_size));
    int		mdc_nelmts;
    double	w0;

    H5Pget_cache (fapl_g, &mdc_nelmts, NULL, &w0);
    H5Pset_cache (fapl_g, mdc_nelmts, cache_size*SQUARE (CH_SIZE), w0);
    file = H5Fopen (FILE_NAME, H5F_ACC_RDWR, fapl_g);
    dset = H5Dopen (file, "dset");
    file_space = H5Dget_space (dset);
    nio_g = 0;
    
    for (i=0, hs_size[0]=io_size; hs_size[0]==io_size; i+=offset) {
	hs_offset[0] = hs_offset[1] = i;
	hs_size[0] = hs_size[1] = MIN (io_size, CH_SIZE*DS_SIZE-i);
	mem_space = H5Screate_simple (2, hs_size, hs_size);
	H5Sselect_hyperslab (file_space, H5S_SELECT_SET, hs_offset, NULL,
			     hs_size, NULL);
	if (READ==op) {
	    H5Dread (dset, H5T_NATIVE_CHAR, mem_space, file_space,
		     H5P_DEFAULT, buf);
	} else {
	    H5Dwrite (dset, H5T_NATIVE_CHAR, mem_space, file_space,
		      H5P_DEFAULT, buf);
	}
	H5Sclose (mem_space);
	nio += hs_size[0]*hs_size[1];
	if (i>0) nio -= SQUARE (io_size-offset);
    }

    free (buf);
    H5Sclose (file_space);
    H5Dclose (dset);
    H5Fclose (file);

    return (double)nio/(double)nio_g;
}


/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	See file prologue.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Robb Matzke
 *              Thursday, May 14, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
main (void)
{
    hsize_t	io_size;
    double	effic, io_percent;
    FILE	*f, *d;
    int		cache_size;
    double	w0;

    /*
     * Create a global file access property list.
     */
    fapl_g = H5Pcreate (H5P_FILE_ACCESS);
    H5Pget_cache (fapl_g, NULL, NULL, &w0);

    /* Create the file */
    create_dataset ();
    f = fopen ("x-gnuplot", "w");


#if 1
    /*
     * Test row-major reading of the dataset with various sizes of request
     * windows.
     */
    if (RM_CACHE_STRT==RM_CACHE_END) {
	fprintf (f, "set yrange [0:1.2]\n");
	fprintf (f, "set ytics 0, 0.1, 1\n");
	fprintf (f, "set xlabel \"%s\"\n",
		 "Request size as a fraction of chunk size");
	fprintf (f, "set ylabel \"Efficiency\"\n");
	fprintf (f, "set title \"Cache %d chunks, w0=%g, "
		 "Size=(total=%d, chunk=%d)\"\n",
		 RM_CACHE_STRT, w0, DS_SIZE*CH_SIZE, CH_SIZE);
    } else {
	fprintf (f, "set autoscale\n");
	fprintf (f, "set hidden3d\n");
    }

    fprintf (f, "set terminal postscript\nset output \"x-rowmaj-rd.ps\"\n");
    fprintf (f, "%s \"x-rowmaj-rd.dat\" title \"RowMaj-Read\" with %s\n",
	     RM_CACHE_STRT==RM_CACHE_END?"plot":"splot",
	     LINESPOINTS);
    fprintf (f, "set terminal x11\nreplot\n");
    d = fopen ("x-rowmaj-rd.dat", "w");
    for (cache_size=RM_CACHE_STRT;
	 cache_size<=RM_CACHE_END;
	 cache_size+=RM_CACHE_DELT) {
	for (io_percent=RM_START; io_percent<=RM_END; io_percent+=RM_DELTA) {
	    io_size = MAX (1, (int)(CH_SIZE*io_percent));
	    printf ("Rowmaj-rd %8d %8.2f", cache_size, io_percent);
	    fflush (stdout);
	    effic = test_rowmaj (READ, cache_size, io_size);
	    printf (" %8.2f\n", effic);
	    if (RM_CACHE_STRT==RM_CACHE_END) {
		fprintf (d, "%g %g\n", io_percent, effic);
	    } else {
		fprintf (d, "%g\n", effic);
	    }
	}
	fprintf (d, "\n");
    }
    fclose (d);
    fprintf (f, "pause -1\n");
#endif

#if 1
    /*
     * Test row-major writing of the dataset with various sizes of request
     * windows.
     */
    if (RM_CACHE_STRT==RM_CACHE_END) {
	fprintf (f, "set yrange [0:1.2]\n");
	fprintf (f, "set ytics 0, 0.1, 1\n");
	fprintf (f, "set xlabel \"%s\"\n",
		 "Request size as a fraction of chunk size");
	fprintf (f, "set ylabel \"Efficiency\"\n");
	fprintf (f, "set title \"Cache %d chunks,w0=%g, "
		 "Size=(total=%d, chunk=%d)\"\n",
		 RM_CACHE_STRT, w0, DS_SIZE*CH_SIZE, CH_SIZE);
    } else {
	fprintf (f, "set autoscale\n");
	fprintf (f, "set hidden3d\n");
    }

    fprintf (f, "set terminal postscript\nset output \"x-rowmaj-wr.ps\"\n");
    fprintf (f, "%s \"x-rowmaj-wr.dat\" title \"RowMaj-Write\" with %s\n",
	     RM_CACHE_STRT==RM_CACHE_END?"plot":"splot",
	     LINESPOINTS);
    fprintf (f, "set terminal x11\nreplot\n");
    d = fopen ("x-rowmaj-wr.dat", "w");
    for (cache_size=RM_CACHE_STRT;
	 cache_size<=RM_CACHE_END;
	 cache_size+=RM_CACHE_DELT) {
	for (io_percent=RM_START; io_percent<=RM_END; io_percent+=RM_DELTA) {
	    io_size = MAX (1, (int)(CH_SIZE*io_percent));
	    printf ("Rowmaj-wr %8d %8.2f", cache_size, io_percent);
	    fflush (stdout);
	    effic = test_rowmaj (WRITE, cache_size, io_size);
	    printf (" %8.2f\n", effic);
	    if (RM_CACHE_STRT==RM_CACHE_END) {
		fprintf (d, "%g %g\n", io_percent, effic);
	    } else {
		fprintf (d, "%g\n", effic);
	    }
	}
	fprintf (d, "\n");
    }
    fclose (d);
    fprintf (f, "pause -1\n");
#endif

#if 1
    /*
     * Test diagonal read
     */
    if (DIAG_CACHE_STRT==DIAG_CACHE_END) {
	fprintf (f, "set yrange [0:1.2]\n");
	fprintf (f, "set ytics 0, 0.1, 1\n");
	fprintf (f, "set xlabel \"%s\"\n",
		 "Request size as a fraction of chunk size");
	fprintf (f, "set ylabel \"Efficiency\"\n");
	fprintf (f, "set title \"Cache %d chunks,w0=%g, "
		 "Size=(total=%d, chunk=%d)\"\n",
		 DIAG_CACHE_STRT, w0, DS_SIZE*CH_SIZE, CH_SIZE);
    } else {
	fprintf (f, "set autoscale\n");
	fprintf (f, "set hidden3d\n");
    }
    fprintf (f, "set terminal postscript\nset output \"x-diag-rd.ps\"\n");
    fprintf (f, "%s \"x-diag-rd.dat\" title \"Diag-Read\" with %s\n",
	     DIAG_CACHE_STRT==DIAG_CACHE_END?"plot":"splot", LINESPOINTS);
    fprintf (f, "set terminal x11\nreplot\n");
    d = fopen ("x-diag-rd.dat", "w");
    for (cache_size=DIAG_CACHE_STRT;
	 cache_size<=DIAG_CACHE_END;
	 cache_size+=DIAG_CACHE_DELT) {
	for (io_percent=DIAG_START;
	     io_percent<=DIAG_END;
	     io_percent+=DIAG_DELTA) {
	    io_size = MAX (1, (int)(CH_SIZE*io_percent));
	    printf ("Diag-rd   %8d %8.2f", cache_size, io_percent);
	    fflush (stdout);
	    effic = test_diag (READ, cache_size, io_size, MAX (1, io_size/2));
	    printf (" %8.2f\n", effic);
	    if (DIAG_CACHE_STRT==DIAG_CACHE_END) {
		fprintf (d, "%g %g\n", io_percent, effic);
	    } else {
		fprintf (d, "%g\n", effic);
	    }
	}
	fprintf (d, "\n");
    }
    fclose (d);
    fprintf (f, "pause -1\n");
#endif

#if 1
    /*
     * Test diagonal write
     */
    if (DIAG_CACHE_STRT==DIAG_CACHE_END) {
	fprintf (f, "set yrange [0:1.2]\n");
	fprintf (f, "set ytics 0, 0.1, 1\n");
	fprintf (f, "set xlabel \"%s\"\n",
		 "Request size as a fraction of chunk size");
	fprintf (f, "set ylabel \"Efficiency\"\n");
	fprintf (f, "set title \"Cache %d chunks, w0=%g, "
		 "Size=(total=%d, chunk=%d)\"\n",
		 DIAG_CACHE_STRT, w0, DS_SIZE*CH_SIZE, CH_SIZE);
    } else {
	fprintf (f, "set autoscale\n");
	fprintf (f, "set hidden3d\n");
    }
    fprintf (f, "set terminal postscript\nset output \"x-diag-wr.ps\"\n");
    fprintf (f, "%s \"x-diag-wr.dat\" title \"Diag-Write\" with %s\n",
	     DIAG_CACHE_STRT==DIAG_CACHE_END?"plot":"splot", LINESPOINTS);
    fprintf (f, "set terminal x11\nreplot\n");
    d = fopen ("x-diag-wr.dat", "w");
    for (cache_size=DIAG_CACHE_STRT;
	 cache_size<=DIAG_CACHE_END;
	 cache_size+=DIAG_CACHE_DELT) {
	for (io_percent=DIAG_START;
	     io_percent<=DIAG_END;
	     io_percent+=DIAG_DELTA) {
	    io_size = MAX (1, (int)(CH_SIZE*io_percent));
	    printf ("Diag-wr   %8d %8.2f", cache_size, io_percent);
	    fflush (stdout);
	    effic = test_diag (WRITE, cache_size, io_size, MAX (1, io_size/2));
	    printf (" %8.2f\n", effic);
	    if (DIAG_CACHE_STRT==DIAG_CACHE_END) {
		fprintf (d, "%g %g\n", io_percent, effic);
	    } else {
		fprintf (d, "%g\n", effic);
	    }
	}
	fprintf (d, "\n");
    }
    fclose (d);
    fprintf (f, "pause -1\n");
#endif


    H5Pclose (fapl_g);
    fclose (f);
    return 0;
}

