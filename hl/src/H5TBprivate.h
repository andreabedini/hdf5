/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _H5TBprivate_H
#define _H5TBprivate_H

/* High-level library internal header file */
#include "H5HLprivate2.h"

/* public TB prototypes			*/
#include "H5TBpublic.h"


#define TABLE_CLASS         "TABLE"
#define HLTB_MAX_FIELD_LEN  255

/*-------------------------------------------------------------------------
 *
 * Private write function used by H5TB and H5PT
 *
 *-------------------------------------------------------------------------
 */

herr_t H5TB_common_append_records( hid_t dataset_id,
                                  hid_t mem_type_id,
                                  size_t nrecords,
                                  hsize_t orig_table_size,
                                  const void * data);

/*-------------------------------------------------------------------------
 *
 * Private read function used by H5TB and H5PT
 *
 *-------------------------------------------------------------------------
 */


herr_t H5TB_common_read_records( hid_t dataset_id,
                                hid_t mem_type_id,
                                hsize_t start,
                                size_t nrecords,
                                hsize_t table_size,
                                void *data);




#endif

