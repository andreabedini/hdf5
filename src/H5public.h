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

/*
 * This file contains public declarations for the H5 module.
 */
#ifndef _H5public_H
#define _H5public_H

#include <H5config.h>           /*from configure                             */
#include <sys/types.h>
#include <stddef.h>
#ifdef HAVE_PARALLEL
#  include <mpi.h>
#  include <mpio.h>
#endif

/* Version numbers */
#define H5_VERS_MAJOR	1       /* For major interface changes    	     */
#define H5_VERS_MINOR	0       /* For minor interface changes    	     */
#define H5_VERS_RELEASE	19       /* For interface tweaks & bug-fixes	     */
#define H5_VERS_PATCH	0       /* For small groups of bug fixes	     */

#define H5check()	H5vers_check(H5_VERS_MAJOR,H5_VERS_MINOR,\
				     H5_VERS_RELEASE, H5_VERS_PATCH)

/*
 * Status return values.  Failed integer functions in HDF5 result almost
 * always in a negative value (unsigned failing functions sometimes return
 * zero for failure) while successfull return is non-negative (often zero).
 * The negative failure value is most commonly -1, but don't bet on it.  The
 * proper way to detect failure is something like:
 *
 * 	if ((dset = H5Dopen (file, name))<0) {
 *	   fprintf (stderr, "unable to open the requested dataset\n");
 *	}
 */
typedef int herr_t;

/*
 * Boolean type.
 */
typedef int hbool_t;

/*
 * The sizes of file-objects in hdf5 have there own types defined here.  On
 * most systems, these are the same as size_t and ssize_t, but on systems
 * with small address spaces these are defined to be larger.
 */
#if defined(HAVE_LARGE_HSIZET) && SIZEOF_SIZE_T<SIZEOF_LONG_LONG
typedef unsigned long long hsize_t;
typedef signed long long hssize_t;
#else
typedef size_t hsize_t;
typedef ssize_t hssize_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Functions in H5.c */
herr_t H5open (void);
herr_t H5close (void);
herr_t H5dont_atexit (void);
herr_t H5version (unsigned *majnum, unsigned *minnum, unsigned *relnum,
		  unsigned *patnum);
herr_t H5vers_check (unsigned majnum, unsigned minnum, unsigned relnum,
		     unsigned patnum);

#ifdef __cplusplus
}
#endif
#endif
