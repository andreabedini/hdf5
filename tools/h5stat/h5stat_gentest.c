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
 * Generate the binary hdf5 files for the h5stat tests.
 * Usage: just execute the program without any arguments will
 * generate all the binary hdf5 files in the ./testfiles directory.
 *
 * If you regenerate the test files (e.g., changing some code,
 * trying it on a new platform, ...), you need to verify the correctness
 * of the expected output and update the corresponding *.ddl files.
 */

#include <assert.h>
#include "hdf5.h"

#define FILE     "h5stat_newgrat.h5"
#define DATASET_NAME  "DATASET_NAME"
#define GROUP_NAME  "GROUP"
#define ATTR_NAME  "ATTR"
#define NUM_GRPS   35000
#define NUM_ATTRS  100

/*
 * Generate 1.8 HDF5 file
 * with NUM_GRPS groups
 * with NUM_ATTRS attributes on the dataset
 */
static void gen_file(void)
{
    hid_t  fcpl;   /* File creation property */
    hid_t  fapl;   /* File access property */
    hid_t     file;  /* File id */
    hid_t  gid;  /* Group id */
    hid_t  type_id;  /* Datatype id */
    hid_t  space_id;   /* Dataspace id */
    hid_t  attr_id;   /* Attribute id */
    hid_t  dset_id;  /* Dataset id */
    char  name[30];  /* Group name */
    char  attrname[30];  /* Attribute name */
    int       ret;  /* Return value */
    int   i;  /* Local index variable */

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    ret = H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    HDassert(ret >= 0);

    /* Set file space handling strategy */
    fcpl = H5Pcreate(H5P_FILE_CREATE);
    ret = H5Pset_file_space(fcpl, H5F_FILE_SPACE_ALL_PERSIST, (hsize_t)0);
    HDassert(ret >= 0);

     /* Create dataset */
    file = H5Fcreate(FILE, H5F_ACC_TRUNC, fcpl, fapl);
    for(i = 1; i <= NUM_GRPS; i++) {
        sprintf(name, "%s%d", GROUP_NAME,i);
        gid = H5Gcreate2(file, name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        H5Gclose(gid);
    } /* end for */

    /* Create a datatype to commit and use */
    type_id = H5Tcopy(H5T_NATIVE_INT);

    /* Create dataspace for dataset */
    space_id = H5Screate(H5S_SCALAR);

     /* Create dataset */
    dset_id = H5Dcreate2(file, DATASET_NAME, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    for(i = 1; i <= NUM_ATTRS; i++) {
        sprintf(attrname, "%s%d", ATTR_NAME,i);
        attr_id = H5Acreate2(dset_id, attrname, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT);
        ret = H5Aclose(attr_id);
        HDassert(ret >= 0);
    } /* end for */

    ret = H5Dclose(dset_id);
    HDassert(ret >= 0);
    ret = H5Sclose(space_id);
    HDassert(ret >= 0);
    ret = H5Tclose(type_id);
    HDassert(ret >= 0);
    ret = H5Fclose(file);
    HDassert(ret >= 0);
}

int main(void)
{
    gen_file();

    return 0;
}

