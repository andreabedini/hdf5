
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
/* src/H5config.h.  Generated by configure.  */
/* src/H5config.h.in.  Generated from configure.in by autoheader.  */

/* Define if your system can handle converting denormalized floating-point
   values. */
#define H5_CONVERT_DENORMAL_FLOAT 1

/* Define if your system can convert long double to unsigned int values
   correctly. */
#define H5_CV_LDOUBLE_TO_UINT_WORKS 1

/* Define if `dev_t' is a scalar */
#define H5_DEV_T_IS_SCALAR 1

/* Define to dummy `main' function (if any) required to link to the Fortran
   libraries. */
/* #undef H5_FC_DUMMY_MAIN */

/* Define if F77 and FC dummy `main' functions are identical. */
/* #undef H5_FC_DUMMY_MAIN_EQ_F77 */

/* Define to a macro mangling the given C identifier (in lower and upper
   case), which must not contain underscores, for linking with Fortran. */
/* #undef H5_FC_FUNC */

/* As FC_FUNC, but for C identifiers containing underscores. */
/* #undef H5_FC_FUNC_ */

/* Define if your system roundup accurately convert floating-point to unsigned
   long long values. */
#define H5_FP_TO_ULLONG_BOTTOM_BIT_WORKS 1

/* Define if your system has right maximum convert floating-point to unsigned
   long long values. */
#define H5_FP_TO_ULLONG_RIGHT_MAXIMUM 1

/* Define if gettimeofday() populates the tz pointer passed in */
#define H5_GETTIMEOFDAY_GIVES_TZ 1

/* Define if the __attribute__(()) extension is present */
/*#define H5_HAVE_ATTRIBUTE a 1 */

/* Define to 1 if you have the `BSDgettimeofday' function. */
/* #undef H5_HAVE_BSDGETTIMEOFDAY */

/* Define to 1 if you have the `difftime' function. */
#define H5_HAVE_DIFFTIME 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define H5_HAVE_DLFCN_H 1

/* Define to 1 if you have the <dmalloc.h> header file. */
/* #undef H5_HAVE_DMALLOC_H */

/* Define to 1 if you have the <features.h> header file. */
/*EIP #define H5_HAVE_FEATURES_H */

/* Define if support for deflate (zlib) filter is enabled */
#define H5_HAVE_FILTER_DEFLATE 1

/* Define if support for Fletcher32 checksum is enabled */
#define H5_HAVE_FILTER_FLETCHER32 1

/* Define if support for nbit filter is enabled */
#define H5_HAVE_FILTER_NBIT 1

/* Define if support for scaleoffset filter is enabled */
#define H5_HAVE_FILTER_SCALEOFFSET 1

/* Define if support for shuffle filter is enabled */
#define H5_HAVE_FILTER_SHUFFLE 1

/* Define if support for szip filter is enabled */
/* #undef H5_HAVE_FILTER_SZIP */

/* Define to 1 if you have the `fork' function. */
/*#undefine H5_HAVE_FORK */

/* Define to 1 if you have the `frexpf' function. */
#define H5_HAVE_FREXPF 1

/* Define to 1 if you have the `frexpl' function. */
#define H5_HAVE_FREXPL 1

/* Define to 1 if you have the `fseek64' function. */
/* #undef H5_HAVE_FSEEK64 */

/* Define if the function stack tracing code is to be compiled in */
/*#define H5_HAVE_CODESTACK 1*/

/* Define if the compiler understand the __FUNCTION__ keyword */
/* EIP #define H5_HAVE_FUNCTION 1*/

/* Define to 1 if you have the `GetConsoleScreenBufferInfo' function. */
/* #undef H5_HAVE_GETCONSOLESCREENBUFFERINFO */

/* Define to 1 if you have the `gethostname' function. */
#define H5_HAVE_GETHOSTNAME 1

/* Define to 1 if you have the `getpwuid' function. */
/*#define H5_HAVE_GETPWUID 1*/

/* Define to 1 if you have the `getrusage' function. */
/*EIP #define H5_HAVE_GETRUSAGE 1*/

/* Define to 1 if you have the `gettextinfo' function. */
/* #undef H5_HAVE_GETTEXTINFO */

/* Define to 1 if you have the `gettimeofday' function. */
#define H5_HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `get_fpc_csr' function. */
/* #undef H5_HAVE_GET_FPC_CSR */

/* Define if we have GPFS support */
/* #undef H5_HAVE_GPFS */

/* Define to 1 if you have the <gpfs.h> header file. */
/* #undef H5_HAVE_GPFS_H */

/* Define if library will contain instrumentation to detect correct
   optimization operation */
#define H5_HAVE_INSTRUMENTED_LIBRARY 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define H5_HAVE_INTTYPES_H 1

/* Define to 1 if you have the `ioctl' function. */
#define H5_HAVE_IOCTL 1

/* Define to 1 if you have the <io.h> header file. */
/* #undef H5_HAVE_IO_H */

/* Define if it's safe to use `long long' for hsize_t and hssize_t */
#define H5_HAVE_LARGE_HSIZET 1

/* Define to 1 if you have the `dmalloc' library (-ldmalloc). */
/* #undef H5_HAVE_LIBDMALLOC */

/* Define to 1 if you have the `lmpe' library (-llmpe). */
/* #undef H5_HAVE_LIBLMPE */

/* Define to 1 if you have the `m' library (-lm). */
#define H5_HAVE_LIBM 1

/* Define to 1 if you have the `mpe' library (-lmpe). */
/* #undef H5_HAVE_LIBMPE */

/* Define to 1 if you have the `mpi' library (-lmpi). */
/* #undef H5_HAVE_LIBMPI */

/* Define to 1 if you have the `mpio' library (-lmpio). */
/* #undef H5_HAVE_LIBMPIO */

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef H5_HAVE_LIBNSL */

/* Define to 1 if you have the `pdb' library (-lpdb). */
/* #undef H5_HAVE_LIBPDB */

/* Define to 1 if you have the `pthread' library (-lpthread). */
/* #undef H5_HAVE_LIBPTHREAD */

/* Define to 1 if you have the `silo' library (-lsilo). */
/* #undef H5_HAVE_LIBSILO */

/* Define to 1 if you have the `sz' library (-lsz). */
/* #undef H5_HAVE_LIBSZ */

/* Define to 1 if you have the `z' library (-lz). */
/* #undefine H5_HAVE_LIBZ*/

/* Define to 1 if you have the `longjmp' function. */
#define H5_HAVE_LONGJMP 1

/* Define to 1 if you have the `lseek64' function. */
/*#define H5_HAVE_LSEEK64 1*/

/* Define to 1 if you have the <memory.h> header file. */
#define H5_HAVE_MEMORY_H 1

/* Define if we have MPE support */
/* #undef H5_HAVE_MPE */

/* Define to 1 if you have the <mpe.h> header file. */
/* #undef H5_HAVE_MPE_H */

/* Define if `MPI_Comm_c2f' and `MPI_Comm_f2c' exists */
/* #undef H5_HAVE_MPI_MULTI_LANG_Comm */

/* Define if `MPI_Info_c2f' and `MPI_Info_f2c' exists */
/* #undef H5_HAVE_MPI_MULTI_LANG_Info */

/* Define to 1 if you have the <netinet/tcp.h> header file. */
#define H5_HAVE_NETINET_TCP_H 1

/* Define if we have parallel support */
/* #undef H5_HAVE_PARALLEL */

/* Define to 1 if you have the <pdb.h> header file. */
/* #undef H5_HAVE_PDB_H */

/* Define to 1 if you have the <pthread.h> header file. */
/* #undef H5_HAVE_PTHREAD_H */

/* Define to 1 if you have the `random' function. */
#define H5_HAVE_RANDOM 1

/* Define to 1 if you have the <setjmp.h> header file. */
#define H5_HAVE_SETJMP_H 1

/* Define to 1 if you have the `setsysinfo' function. */
/* #undef H5_HAVE_SETSYSINFO */

/* Define to 1 if you have the `sigaction' function. */
#define H5_HAVE_SIGACTION 1

/* Define to 1 if you have the `signal' function. */
#define H5_HAVE_SIGNAL 1

/* Define to 1 if you have the `snprintf' function. */
#define H5_HAVE_SNPRINTF 1

/* Define if `socklen_t' is defined */
#define H5_HAVE_SOCKLEN_T 1

/* Define to 1 if you have the `srandom' function. */
#define H5_HAVE_SRANDOM 1

/* Define if `struct stat' has the `st_blocks' field */
/* #undef H5_HAVE_STAT_ST_BLOCKS */

/* Define to 1 if you have the <stddef.h> header file. */
#define H5_HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
/*EIP #define H5_HAVE_STDINT_H 1 */

/* Define to 1 if you have the <stdlib.h> header file. */
#define H5_HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define H5_HAVE_STRDUP 1

/* Define if the stream virtual file driver should be compiled */
/*#define H5_HAVE_STREAM 1*/

/* Define to 1 if you have the <strings.h> header file. */
#define H5_HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define H5_HAVE_STRING_H 1

/* Define if `struct text_info' is defined */
/* #undef H5_HAVE_STRUCT_TEXT_INFO */

/* Define if `struct timezone' is defined */
#define H5_HAVE_STRUCT_TIMEZONE 1

/* Define to 1 if `tm_zone' is member of `struct tm'. */
#define H5_HAVE_STRUCT_TM_TM_ZONE 1

/* Define if `struct videoconfig' is defined */
/* #undef H5_HAVE_STRUCT_VIDEOCONFIG */

/* Define to 1 if you have the `system' function. */
#define H5_HAVE_SYSTEM 1

/* Define to 1 if you have the <sys/filio.h> header file. */
/* #undef H5_HAVE_SYS_FILIO_H */

/* Define to 1 if you have the <sys/fpu.h> header file. */
/* #undef H5_HAVE_SYS_FPU_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define H5_HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/proc.h> header file. */
/* #undef H5_HAVE_SYS_PROC_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
#define H5_HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define H5_HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define H5_HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/sysinfo.h> header file. */
/* #undef H5_HAVE_SYS_SYSINFO_H */

/* Define to 1 if you have the <sys/timeb.h> header file. */
#define H5_HAVE_SYS_TIMEB_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define H5_HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define H5_HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <szlib.h> header file. */
/* #undef H5_HAVE_SZLIB_H */

/* Define if we have thread safe support */
/* #undef H5_HAVE_THREADSAFE */

/* Define if `timezone' is a global variable */
/* #undef H5_HAVE_TIMEZONE */
#define H5_HAVE_TIMEZONE 1
/* Define if the ioctl TIOCGETD is defined */
#define H5_HAVE_TIOCGETD 1

/* Define if the ioctl TIOGWINSZ is defined */
#define H5_HAVE_TIOCGWINSZ 1

/* Define if `tm_gmtoff' is a member of `struct tm' */
#define H5_HAVE_TM_GMTOFF 1

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#define H5_HAVE_TM_ZONE 1

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
/* #undef H5_HAVE_TZNAME */

/* Define to 1 if you have the <unistd.h> header file. */
#define H5_HAVE_UNISTD_H 1

/* Define to 1 if you have the `vasprintf' function. */
/*#undefine H5_HAVE_VASPRINTF*/

/* Define to 1 if you have the `vsnprintf' function. */
#define H5_HAVE_VSNPRINTF 1

/* Define to 1 if you have the `waitpid' function. */
/*#undefine H5_HAVE_WAITPID */

/* Define to 1 if you have the <winsock.h> header file. */
/* #undef H5_HAVE_WINSOCK_H */

/* Define to 1 if you have the <zlib.h> header file. */
#define H5_HAVE_ZLIB_H 1

/* Define to 1 if you have the `_getvideoconfig' function. */
/* #undef H5_HAVE__GETVIDEOCONFIG */

/* Define to 1 if you have the `_scrsize' function. */
/* #undef H5_HAVE__SCRSIZE */

/* Define if `__tm_gmtoff' is a member of `struct tm' */
/* #undef H5_HAVE___TM_GMTOFF */

/* Define if your system can compile long long to floating-point casts. */
#define H5_LLONG_TO_FP_CAST_WORKS 1

/* Define if your system can handle complicated MPI derived datatype
   correctly. */
/* #undef H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS */

/* Define if your system's `MPI_File_set_size' function works for files over
   2GB. */
/* #undef H5_MPI_FILE_SET_SIZE_BIG */

/* Define if we can violate pointer alignment restrictions */
/* #undef H5_NO_ALIGNMENT_RESTRICTIONS */

/* Define if shared writing must be disabled (CodeWarrior only) */
#define H5_NO_SHARED_WRITING 

/* Name of package */
#define H5_PACKAGE "hdf5"

/* Define to the address where bug reports for this package should be sent. */
#define H5_PACKAGE_BUGREPORT "help@hdfgroup.org"

/* Define to the full name of this package. */
#define H5_PACKAGE_NAME "HDF5"

/* Define to the full name and version of this package. */
#define H5_PACKAGE_STRING "HDF5 1.9.20"

/* Define to the one symbol short name of this package. */
#define H5_PACKAGE_TARNAME "hdf5"

/* Define to the version of this package. */
#define H5_PACKAGE_VERSION "1.9.20"

/* Width for printf() for type `long long' or `__int64', use `ll' */
#define H5_PRINTF_LL_WIDTH "ll"

/* The size of a `char', as computed by sizeof. */
#define H5_SIZEOF_CHAR 1

/* The size of a `double', as computed by sizeof. */
#define H5_SIZEOF_DOUBLE 8

/* The size of a `float', as computed by sizeof. */
#define H5_SIZEOF_FLOAT 4

/* The size of a `int', as computed by sizeof. */
#define H5_SIZEOF_INT 4

/* The size of a `int16_t', as computed by sizeof. */
#define H5_SIZEOF_INT16_T 2

/* The size of a `int32_t', as computed by sizeof. */
#define H5_SIZEOF_INT32_T 4

/* The size of a `int64_t', as computed by sizeof. */
#define H5_SIZEOF_INT64_T 8

/* The size of a `int8_t', as computed by sizeof. */
#define H5_SIZEOF_INT8_T 1

/* The size of a `int_fast16_t', as computed by sizeof. */
/*#define H5_SIZEOF_INT_FAST16_T 4 */

/* The size of a `int_fast32_t', as computed by sizeof. */
/*#define H5_SIZEOF_INT_FAST32_T 4*/

/* The size of a `int_fast64_t', as computed by sizeof. */
/*#define H5_SIZEOF_INT_FAST64_T 8*/

/* The size of a `int_fast8_t', as computed by sizeof. */
/*#define H5_SIZEOF_INT_FAST8_T 1*/

/* The size of a `int_least16_t', as computed by sizeof. */
/*#define H5_SIZEOF_INT_LEAST16_T 2*/

/* The size of a `int_least32_t', as computed by sizeof. */
/*#define H5_SIZEOF_INT_LEAST32_T 4*/

/* The size of a `int_least64_t', as computed by sizeof. */
/*#define H5_SIZEOF_INT_LEAST64_T 8*/

/* The size of a `int_least8_t', as computed by sizeof. */
/*#define H5_SIZEOF_INT_LEAST8_T 1*/

/* The size of a `long', as computed by sizeof. */
#define H5_SIZEOF_LONG 4

/* The size of a `long double', as computed by sizeof. */
#define H5_SIZEOF_LONG_DOUBLE 16

/* The size of a `long long', as computed by sizeof. */
#define H5_SIZEOF_LONG_LONG 8

/* The size of a `off_t', as computed by sizeof. */
#define H5_SIZEOF_OFF_T 4

/* The size of a `short', as computed by sizeof. */
#define H5_SIZEOF_SHORT 2

/* The size of a `size_t', as computed by sizeof. */
#define H5_SIZEOF_SIZE_T 4

/* The size of a `ssize_t', as computed by sizeof. */
#define H5_SIZEOF_SSIZE_T 4

/* The size of a `uint16_t', as computed by sizeof. */
#define H5_SIZEOF_UINT16_T 2

/* The size of a `uint32_t', as computed by sizeof. */
#define H5_SIZEOF_UINT32_T 4 

/* The size of a `uint64_t', as computed by sizeof. */
#define H5_SIZEOF_UINT64_T 8

/* The size of a `uint8_t', as computed by sizeof. */
#define H5_SIZEOF_UINT8_T 1

/* The size of a `uint_fast16_t', as computed by sizeof. */
/*#define H5_SIZEOF_UINT_FAST16_T 4*/

/* The size of a `uint_fast32_t', as computed by sizeof. */
/*#define H5_SIZEOF_UINT_FAST32_T 4*/

/* The size of a `uint_fast64_t', as computed by sizeof. */
/*#define H5_SIZEOF_UINT_FAST64_T 8 */

/* The size of a `uint_fast8_t', as computed by sizeof. */
/*#define H5_SIZEOF_UINT_FAST8_T 1*/

/* The size of a `uint_least16_t', as computed by sizeof. */
/*#define H5_SIZEOF_UINT_LEAST16_T 2 */

/* The size of a `uint_least32_t', as computed by sizeof. */
/*#define H5_SIZEOF_UINT_LEAST32_T 4*/

/* The size of a `uint_least64_t', as computed by sizeof. */
/*#define H5_SIZEOF_UINT_LEAST64_T 8 */

/* The size of a `uint_least8_t', as computed by sizeof. */
/*#define H5_SIZEOF_UINT_LEAST8_T 1*/

/* The size of a `__int64', as computed by sizeof. */
#define H5_SIZEOF___INT64 0

/* Define to 1 if you have the ANSI C header files. */
#define H5_STDC_HEADERS 1

/* Define if your system can accurately convert from integers to long double
   values. */
#define H5_INTEGER_TO_LDOUBLE_ACCURATE 1

/* Define if your system can convert long double to integers accurately. */
#define H5_LDOUBLE_TO_INTEGER_ACCURATE 1

/* Define if your system can accurately convert from long double to integer
   values. */
#define H5_LDOUBLE_TO_INTEGER_WORKS 1

/* Define if your system can convert long double to unsigned int values
   correctly. */
#define H5_LDOUBLE_TO_UINT_ACCURATE 1

/* Define if your system can accurately convert unsigned (long) long values to
   floating-point values. */
#define H5_ULONG_TO_FP_BOTTOM_BIT_WORKS 1

/* Define if your system supports pthread_attr_setscope(&attribute,
   PTHREAD_SCOPE_SYSTEM) call. */
#define H5_SYSTEM_SCOPE_THREADS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define H5_TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef H5_TM_IN_SYS_TIME */

/* Define if your system can compile unsigned long long to floating-point
   casts. */
#define H5_ULLONG_TO_FP_CAST_WORKS 1

/* Define if your system can convert unsigned long long to long double with
   correct precision. */
#define H5_ULLONG_TO_LDOUBLE_PRECISION 1

/* Version number of package */
#define H5_VERSION "1.9.20"

/* Define if vsnprintf() returns the correct value for formatted strings that
   don't fit into size allowed */
#define H5_VSNPRINTF_WORKS 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef H5_WORDS_BIGENDIAN */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef H5_const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef H5_inline */
#endif

/* Define to `long' if <sys/types.h> does not define. */
/* #undef H5_off_t */

/* Define to `unsigned long' if <sys/types.h> does not define. */
/* #undef H5_size_t */

/* Define to `long' if <sys/types.h> does not define. */
/* #undef H5_ssize_t */
#define H5_HAVE_FILE_VERSIONS 1
#define H5_CANNOT_OPEN_TWICE  1
#define H5_WANT_DATA_ACCURACY 1
#define H5_WANT_DCONV_EXCEPTION 1
#define H5_DEFAULT_VFD H5FD_SEC2
