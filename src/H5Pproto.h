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
 * This file contains function prototypes for each exported function in the H5P module
 */

#ifndef H5PPROTO_H
#define H5PPROTO_H

/* Define atomic datatypes */
#define H5P_SCALAR  MAKE_ATOM(H5_DATASPACE,0)

typedef struct {
    uintn rank;
    uint32 *dims;
  } H5P_dim_t;

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */

/* Functions in H5P.c */
hatom_t H5P_create(hatom_t owner_id, hobjtype_t type, const char *name);
uintn H5Pnelem(hatom_t dim_id);
herr_t H5P_release(hatom_t oid);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif /* H5PPROTO_H */

