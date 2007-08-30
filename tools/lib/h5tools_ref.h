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

#ifndef H5TOOLS_REF_H__
#define H5TOOLS_REF_H__

#include "hdf5.h"

#ifdef __cplusplus
extern "C" {
#endif

int init_ref_path_table(hid_t fid);
const char *lookup_ref_path(haddr_t ref);
herr_t      fill_ref_path_table(hid_t, const char *, const H5L_info_t *, void *);
int         get_next_xid(void);
haddr_t     get_fake_xid (void);
haddr_t ref_path_table_lookup(const char *);
haddr_t ref_path_table_gen_fake(const char *);
int term_ref_path_table(void);

#ifdef __cplusplus
}
#endif

#endif

