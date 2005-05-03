/* C Program to find type sizes */

#include <stdio.h>
#include <assert.h>

#include "H5public.h"
/* Include H5Ipublic.h for hid_t type */
#include "H5Ipublic.h"

/* Definitions of which fortran type sizes exist */
#include "H5fort_type_defines.h"

/* File pointers for files */
FILE * c_header;
FILE * fort_header;

#define CFILE "H5f90i_gen.h"
#define FFILE "H5fortran_types.f90"

void initCfile()
{
  fprintf(c_header,
    "/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\
* Copyright by the Board of Trustees of the University of Illinois.         *\n\
 * All rights reserved.                                                      *\n\
 *                                                                           *\n\
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *\n\
 * terms governing use, modification, and redistribution, is contained in    *\n\
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *\n\
 * of the source code distribution tree; Copyright.html can be found at the  *\n\
 * root level of an installed copy of the electronic HDF5 document set and   *\n\
 * is linked from the top-level documents page.  It can also be found at     *\n\
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *\n\
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *\n\
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */\n\
\n\n\
\#ifndef _H5f90i_gen_H\n\
\#define _H5f90i_gen_H\n\
\n\
/* This file is automatically generated by H5match_types.c at build time. */\n\
\n\
#include \"H5public.h\"\n\n");
}

void initFfile()
{
  fprintf(fort_header,
    "! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n\
!   Copyright by the Board of Trustees of the University of Illinois.         *\n\
!   All rights reserved.                                                      *\n\
!                                                                             *\n\
!   This file is part of HDF5.  The full HDF5 copyright notice, including     *\n\
!   terms governing use, modification, and redistribution, is contained in    *\n\
!   the files COPYING and Copyright.html.  COPYING can be found at the root   *\n\
!   of the source code distribution tree; Copyright.html can be found at the  *\n\
!   root level of an installed copy of the electronic HDF5 document set and   *\n\
!   is linked from the top-level documents page.  It can also be found at     *\n\
!   http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *\n\
!   access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *\n\
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\
!\n!\n\
! This file is automatically generated and contains HDF5 Fortran90 type definitions.\n!\n\
       MODULE H5FORTRAN_TYPES\n\
         !\n\
         !  HDF5 integers\n\
         !\n");

}

void endCfile()
{
  fprintf(c_header, "\n#endif /* _H5f90i_gen_H */\n");
}
void endFfile()
{
  fprintf(fort_header, "\n        INTEGER(SIZE_T), PARAMETER :: OBJECT_NAMELEN_DEFAULT_F = -1\n\n");
  fprintf(fort_header, "        END MODULE H5FORTRAN_TYPES\n");
}

/* Define a c_int_x type in the C header */
void writeTypedef(const char* c_type, unsigned int size)
{
  fprintf(c_header, "#define c_int_%d %s\n", size, c_type);
}

/* Call this function if there is no matching C type for sizes > 1 */
void writeTypedefDefault(unsigned int size)
{
  assert(size %2 == 0);

  fprintf(c_header, "typedef struct {c_int_%d a; c_int_%d b;} c_int_%d\n", size / 2, size / 2, size);
}

/* Create matching Fortran and C types by writing to both files */
void writeToFiles(const char* fortran_type, const char* c_type, unsigned int size)
{
  fprintf(fort_header, "        INTEGER, PARAMETER :: %s = %d\n", fortran_type, size);
  fprintf(c_header, "typedef c_int_%d %s;\n", size, c_type);
}

/* hid_t and hssize_t don't have their sizes defined anywhere.
 * Use sizeof() instead. */
#define H5_SIZEOF_HID_T sizeof(hid_t)
#define H5_SIZEOF_HSSIZE_T sizeof(hssize_t)

int main()
{
  /* Open target files */
  c_header = fopen(CFILE, "w");
  fort_header = fopen(FFILE, "w");

  /* Write copyright, boilerplate to both files */
  initCfile();
  initFfile();

  /* First, define c_int_x */
  if(sizeof(long_long) == 1)
    writeTypedef("long_long", 1);
  else if(sizeof(long) == 1)
    writeTypedef("long", 1);
  else if(sizeof(int) == 1)
    writeTypedef("int", 1);
  else if(sizeof(short) == 1)
    writeTypedef("short", 1);
  else
    writeTypedef("char", 1);
  /* Actually, char is not necessarily one byte.
   * But if char isn't, then nothing is, so this
   * is as close as we can get. */

  if(sizeof(long_long) == 2)
    writeTypedef("long_long", 2);
  else if(sizeof(long) == 2)
    writeTypedef("long", 2);
  else if(sizeof(int) == 2)
    writeTypedef("int", 2);
  else if(sizeof(short) == 2)
    writeTypedef("short", 2);
  else
    writeTypedefDefault(2);

  if(sizeof(long_long) == 4)
    writeTypedef("long_long", 4);
  else if(sizeof(long) == 4)
    writeTypedef("long", 4);
  else if(sizeof(int) == 4)
    writeTypedef("int", 4);
  else if(sizeof(short) == 4)
    writeTypedef("short", 4);
  else
    writeTypedefDefault(4);

  if(sizeof(long_long) == 8)
    writeTypedef("long_long", 8);
  else if(sizeof(long) == 8)
    writeTypedef("long", 8);
  else if(sizeof(int) == 8)
    writeTypedef("int", 8);
  else if(sizeof(short) == 8)
    writeTypedef("short", 8);
  else
    writeTypedefDefault(8);

  /* Now begin defining fortran types. */
  fprintf(c_header, "\n");

  /* haddr_t */
  if( H5_SIZEOF_HADDR_T >= 8) {
    #ifdef H5_FORTRAN_HAS_INTEGER_8
      writeToFiles("HADDR_T", "haddr_t_f", 8);
      goto hsize_t_start;
    #endif
  }
  if( H5_SIZEOF_HADDR_T >= 4) {
    #ifdef H5_FORTRAN_HAS_INTEGER_4
      writeToFiles("HADDR_T", "haddr_t_f", 4);
      goto hsize_t_start;
    #endif
  }
  if( H5_SIZEOF_HADDR_T >= 2) {
    #ifdef H5_FORTRAN_HAS_INTEGER_2
      writeToFiles("HADDR_T", "haddr_t_f", 2);
      goto hsize_t_start;
    #endif
  }
  if( H5_SIZEOF_HADDR_T >= 1) {
    #ifdef H5_FORTRAN_HAS_INTEGER_1
      writeToFiles("HADDR_T", "haddr_t_f", 1);
      goto hsize_t_start;
    #endif
  }
    /* Error: couldn't find a size for haddr_t */
    return -1;

  hsize_t_start:

  /* hsize_t */
  if( H5_SIZEOF_HSIZE_T >= 8) {
    #ifdef H5_FORTRAN_HAS_INTEGER_8
      writeToFiles("HSIZE_T", "hsize_t_f", 8);
      goto hssize_t_start;
    #endif
  }
  if( H5_SIZEOF_HSIZE_T >= 4) {
    #ifdef H5_FORTRAN_HAS_INTEGER_4
      writeToFiles("HSIZE_T", "hsize_t_f", 4);
      goto hssize_t_start;
    #endif
  }
  if( H5_SIZEOF_HSIZE_T >= 2) {
    #ifdef H5_FORTRAN_HAS_INTEGER_2
      writeToFiles("HSIZE_T", "hsize_t_f", 2);
      goto hssize_t_start;
    #endif
  }
  if( H5_SIZEOF_HSIZE_T >= 1) {
    #ifdef H5_FORTRAN_HAS_INTEGER_1
      writeToFiles("HSIZE_T", "hsize_t_f", 1);
      goto hssize_t_start;
    #endif
  }
    /* Error: couldn't find a size for hsize_t */
    return -1;

  hssize_t_start:

  /* hssize_t */
  if( H5_SIZEOF_HSSIZE_T >= 8) {
    #ifdef H5_FORTRAN_HAS_INTEGER_8
      writeToFiles("HSSIZE_T", "hssize_t_f", 8);
      goto size_t_start;
    #endif
  }
  if( H5_SIZEOF_HSSIZE_T >= 4) {
    #ifdef H5_FORTRAN_HAS_INTEGER_4
      writeToFiles("HSSIZE_T", "hssize_t_f", 4);
      goto size_t_start;
    #endif
  }
  if( H5_SIZEOF_HSSIZE_T >= 2) {
    #ifdef H5_FORTRAN_HAS_INTEGER_2
      writeToFiles("HSSIZE_T", "hssize_t_f", 2);
      goto size_t_start;
    #endif
  }
  if( H5_SIZEOF_HSSIZE_T >= 1) {
    #ifdef H5_FORTRAN_HAS_INTEGER_1
      writeToFiles("HSSIZE_T", "hssize_t_f", 1);
      goto size_t_start;
    #endif
  }
    /* Error: couldn't find a size for hssize_t */
    return -1;


  size_t_start:
  /* size_t */
  if( H5_SIZEOF_SIZE_T >= 8) {
    #ifdef H5_FORTRAN_HAS_INTEGER_8
      writeToFiles("SIZE_T", "size_t_f", 8);
      goto int_start;
    #endif
  }
  if( H5_SIZEOF_SIZE_T >= 4) {
    #ifdef H5_FORTRAN_HAS_INTEGER_4
      writeToFiles("SIZE_T", "size_t_f", 4);
      goto int_start;
    #endif
  }
  if( H5_SIZEOF_SIZE_T >= 2) {
    #ifdef H5_FORTRAN_HAS_INTEGER_2
      writeToFiles("SIZE_T", "size_t_f", 2);
      goto int_start;
    #endif
  }
  if( H5_SIZEOF_SIZE_T >= 1) {
    #ifdef H5_FORTRAN_HAS_INTEGER_1
      writeToFiles("SIZE_T", "size_t_f", 1);
      goto int_start;
    #endif
  }
    /* Error: couldn't find a size for size_t */
    return -1;

  int_start:
  /* int */
  if( H5_SIZEOF_INT >= 8) {
    #ifdef H5_FORTRAN_HAS_INTEGER_8
      writeToFiles("INT", "int_f", 8);
      goto hid_t_start;
    #endif
  }
  if( H5_SIZEOF_INT >= 4) {
    #ifdef H5_FORTRAN_HAS_INTEGER_4
      writeToFiles("INT", "int_f", 4);
      goto hid_t_start;
    #endif
  }
  if( H5_SIZEOF_INT >= 2) {
    #ifdef H5_FORTRAN_HAS_INTEGER_2
      writeToFiles("INT", "int_f", 2);
      goto hid_t_start;
    #endif
  }
  if( H5_SIZEOF_INT >= 1) {
    #ifdef H5_FORTRAN_HAS_INTEGER_1
      writeToFiles("INT", "int_f", 1);
      goto hid_t_start;
    #endif
  }
    /* Error: couldn't find a size for int */
    return -1;

  hid_t_start:
  /* hid_t */
  if( H5_SIZEOF_HID_T >= 8) {
    #ifdef H5_FORTRAN_HAS_INTEGER_8
      writeToFiles("HID_T", "hid_t_f", 8);
      goto real_start;
    #endif
  }
  if( H5_SIZEOF_HID_T >= 4) {
    #ifdef H5_FORTRAN_HAS_INTEGER_4
      writeToFiles("HID_T", "hid_t_f", 4);
      goto real_start;
    #endif
  }
  if( H5_SIZEOF_HID_T >= 2) {
    #ifdef H5_FORTRAN_HAS_INTEGER_2
      writeToFiles("HID_T", "hid_t_f", 2);
      goto real_start;
    #endif
  }
  if( H5_SIZEOF_HID_T >= 1) {
    #ifdef H5_FORTRAN_HAS_INTEGER_1
      writeToFiles("HID_T", "hid_t_f", 1);
      goto real_start;
    #endif
  }
    /* Error: couldn't find a size for hid_t */
    return -1;

  real_start:
    goto done;
  done:

  /* Close files */
  endCfile();
  endFfile();
  fclose(c_header);
  fclose(fort_header);
  return 0;
}

