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

#include <stdio.h>
#include <stdlib.h>
#include "hdf5.h"
#include "H5IM.h"



/*-------------------------------------------------------------------------
 * Program: h52giftst
 *
 * Purpose: generate files for h52gif testing
 *
 * Programmer: Pedro Vicente, pvn@hdfgroup.org
 *
 * Date: March 15, 2007
 *
 *-------------------------------------------------------------------------
 */

#define FILENAME    "h52giftst.h5"
#define WIDTH       (hsize_t)400
#define HEIGHT      (hsize_t)200
#define PAL_ENTRIES  256
#define IMAGE1_NAME  "1234567"
#define PAL_NAME     "palette"

/*-------------------------------------------------------------------------
 * Function: main
 *
 * Purpose: main program
 *
 *-------------------------------------------------------------------------
 */

int main(void) 
{
    hid_t         fid;
    int           i, j, n, space;
    unsigned char buf [ WIDTH*HEIGHT ];
    unsigned char pal[ PAL_ENTRIES * 3 ];        /* palette array */
    hsize_t       pal_dims[2] = {PAL_ENTRIES,3}; /* palette dimensions */

    
    /* create a file  */
    if ((fid = H5Fcreate(FILENAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))<0)
        return 1;

    /* create an image */
    space = WIDTH*HEIGHT / PAL_ENTRIES;
    for (i=0, j=0, n=0; i < WIDTH*HEIGHT; i++, j++ )
    {
        buf[i] = n;
        if ( j > space )
        {
            n++;
            j=0;
        }
        if (n>PAL_ENTRIES-1) n=0;
    }

    /* make the image */
    if (H5IMmake_image_8bit( fid, IMAGE1_NAME, WIDTH, HEIGHT, buf )<0)
        return 1;

   /*-------------------------------------------------------------------------
    * define a palette, blue to red tones 
    *-------------------------------------------------------------------------
    */
    for ( i=0, n=0; i<PAL_ENTRIES*3; i+=3, n++)
    {
        pal[i]  =n;      /* red */
        pal[i+1]=0;      /* green */
        pal[i+2]=255-n;  /* blue */
    }
    
    /* make a palette */
    if (H5IMmake_palette( fid, PAL_NAME, pal_dims, pal )<0)
        return 1;
    
    /* attach the palette to the image */
    if (H5IMlink_palette( fid, IMAGE1_NAME, PAL_NAME )<0)
        return 1;
    
    if(H5Fclose(fid)<0)
        return 1;
    
    return 0;
}

