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

/* $Id$ */

/*
 * This file contains function prototypes for each exported function in the H5F module
 */

#ifndef H5FPROTO_H
#define H5FPROTO_H

/* file access codes */
#define H5ACC_WRITE         0x0001  /* User in H5Fopen to open a file with write access */
#define H5ACC_OVERWRITE     0x0002  /* User in H5Fcreate truncate an existing file */

/*
 * If we're using POSIXUNBUFIO and lseek64() is available, then use
 * 64-bit file offsets.  Otherwise use whatever `off_t' is.
 */
#if (FILELIB==POSIXUNBUFIO) && defined(H5_HAVE_OFF64_T) && defined(H5_HAVE_LSEEK64)
typedef off64_t haddr_t;
#else
typedef off_t haddr_t;
#endif

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */

/* Functions in H5F.c */
hbool_t H5Fis_hdf5(const char *filename);
hatom_t H5Fcreate(const char *filename, uintn flags, hatom_t create_template, hatom_t access_template);
hatom_t H5Fopen(const char *filename, uintn flags, hatom_t access_template);
herr_t H5Fclose(hatom_t fid);
hatom_t H5Fget_create_template(hatom_t fid);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif /* H5FPROTO_H */

