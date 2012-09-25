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
 * Programmer:  Mohamad Chaarawi <chaarawi@hdfgroup.org>
 *              Septemeber 11, 2012
 *
 * Purpose:	The header file for the mdc driver.
 */
#ifndef H5FDmdc_H
#define H5FDmdc_H

#ifdef H5_HAVE_PARALLEL
#   define H5FD_MDC	(H5FD_mdc_init())
#else
#   define H5FD_MDC	(-1)
#endif /* H5_HAVE_PARALLEL */

/* Macros */

#define IS_H5FD_MDC(f)	/* (H5F_t *f) */				    \
    (H5FD_MDC==H5F_DRIVER_ID(f))

#ifdef H5_HAVE_PARALLEL
/*Turn on H5FDmdc_debug if H5F_DEBUG is on */
#ifdef H5F_DEBUG
#ifndef H5FDmdc_DEBUG
#define H5FDmdc_DEBUG
#endif
#endif

/* Function prototypes */
#ifdef __cplusplus
extern "C" {
#endif
H5_DLL hid_t H5FD_mdc_init(void);
H5_DLL herr_t H5P_set_fapl_mdc(hid_t fapl_id, const char *name, hid_t plist_id);
#ifdef __cplusplus
}
#endif

#endif /* H5_HAVE_PARALLEL */

#endif

