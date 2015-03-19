!****h* ROBODoc/H5GLOBAL
!
! NAME
!  MODULE H5GLOBAL
!
! FILE
!  src/fortran/H5f90global.f90
!
! PURPOSE
!  This module is used to pass C stubs for H5 Fortran APIs. The C stubs are
!  packed into arrays in H5_f.c and these arrays are then passed to Fortran.
!  The Fortran values listed in this file are actually assigned in H5_ff.f90
!  in H5open_f from the elements of the array that is being passed back from C.
!
! NOTES
!  The size of the C arrays in H5_f.c has to match the values of the variables
!  declared as PARAMETER, hence if the size of an array in H5_f.c is changed
!  then the PARAMETER of that corresponding array in Fortran must also be changed.
!
!                         *** IMPORTANT ***
!  If you add a new variable here then you must add the variable name to the
!  Windows dll file 'hdf5_fortrandll.def.in' in the fortran/src directory.
!  This is needed for Windows based operating systems.  
!
!
! USES
!  H5FORTRAN_TYPES 	 - This module is generated at run time. See
!
! COPYRIGHT
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
!   Copyright by The HDF Group.                                               *
!   Copyright by the Board of Trustees of the University of Illinois.         *
!   All rights reserved.                                                      *
!                                                                             *
!   This file is part of HDF5.  The full HDF5 copyright notice, including     *
!   terms governing use, modification, and redistribution, is contained in    *
!   the files COPYING and Copyright.html.  COPYING can be found at the root   *
!   of the source code distribution tree; Copyright.html can be found at the  *
!   root level of an installed copy of the electronic HDF5 document set and   *
!   is linked from the top-level documents page.  It can also be found at     *
!   http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
!   access to either file, you may request a copy from help@hdfgroup.org.     *
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
!
! AUTHOR
!  Elena Pourmal
!
!*****

MODULE H5GLOBAL
  USE H5FORTRAN_TYPES

  IMPLICIT NONE

  ! Definitions for reference datatypes.
  ! If you change the value of these parameters, do not forget to change corresponding
  ! values in the H5f90.h file.
  INTEGER, PARAMETER :: REF_REG_BUF_LEN = 3

  ! Parameters used in the function 'h5kind_to_type' located in H5_ff.f90.
  ! The flag is used to tell the function whether the kind input variable
  ! is for a REAL or INTEGER data type.

  INTEGER, PARAMETER :: H5_INTEGER_KIND = 0
  INTEGER, PARAMETER :: H5_REAL_KIND    = 1

  TYPE :: hobj_ref_t_f
     INTEGER(HADDR_T) ref
  END TYPE hobj_ref_t_f

  TYPE :: hdset_reg_ref_t_f
     INTEGER, DIMENSION(1:REF_REG_BUF_LEN) :: ref
  END TYPE hdset_reg_ref_t_f

  ! These constants need to be global because they are used in 
  ! both h5open_f and in h5close_f
  INTEGER, PARAMETER :: PREDEF_TYPES_LEN   = 17
  INTEGER, PARAMETER :: FLOATING_TYPES_LEN = 4  
  INTEGER, PARAMETER :: INTEGER_TYPES_LEN  = 27
  
  ! These arrays need to be global because they are used in 
  ! both h5open_f and in h5close_f
  INTEGER(HID_T), DIMENSION(PREDEF_TYPES_LEN)   :: predef_types
  INTEGER(HID_T), DIMENSION(FLOATING_TYPES_LEN) :: floating_types
  INTEGER(HID_T), DIMENSION(INTEGER_TYPES_LEN)  :: integer_types

  INTEGER(HID_T) :: H5T_NATIVE_INTEGER_1
  INTEGER(HID_T) :: H5T_NATIVE_INTEGER_2
  INTEGER(HID_T) :: H5T_NATIVE_INTEGER_4
  INTEGER(HID_T) :: H5T_NATIVE_INTEGER_8
  INTEGER(HID_T) :: H5T_NATIVE_REAL_4
  INTEGER(HID_T) :: H5T_NATIVE_REAL_8
  INTEGER(HID_T) :: H5T_NATIVE_REAL_16
  INTEGER(HID_T) :: H5T_NATIVE_INTEGER
  INTEGER(HID_T) :: H5T_NATIVE_REAL
  INTEGER(HID_T) :: H5T_NATIVE_DOUBLE
  INTEGER(HID_T) :: H5T_NATIVE_CHARACTER 
  INTEGER(HID_T) :: H5T_STD_REF_OBJ
  INTEGER(HID_T) :: H5T_STD_REF_DSETREG
  INTEGER(HID_T) :: H5T_IEEE_F32BE
  INTEGER(HID_T) :: H5T_IEEE_F32LE
  INTEGER(HID_T) :: H5T_IEEE_F64BE
  INTEGER(HID_T) :: H5T_IEEE_F64LE
  INTEGER(HID_T) :: H5T_STD_I8BE
  INTEGER(HID_T) :: H5T_STD_I8LE
  INTEGER(HID_T) :: H5T_STD_I16BE
  INTEGER(HID_T) :: H5T_STD_I16LE
  INTEGER(HID_T) :: H5T_STD_I32BE
  INTEGER(HID_T) :: H5T_STD_I32LE
  INTEGER(HID_T) :: H5T_STD_I64BE
  INTEGER(HID_T) :: H5T_STD_I64LE
  INTEGER(HID_T) :: H5T_STD_U8BE
  INTEGER(HID_T) :: H5T_STD_U8LE
  INTEGER(HID_T) :: H5T_STD_U16BE
  INTEGER(HID_T) :: H5T_STD_U16LE
  INTEGER(HID_T) :: H5T_STD_U32BE
  INTEGER(HID_T) :: H5T_STD_U32LE
  INTEGER(HID_T) :: H5T_STD_U64BE
  INTEGER(HID_T) :: H5T_STD_U64LE
  INTEGER(HID_T) :: H5T_STRING
  INTEGER(HID_T) :: H5T_STD_B8BE
  INTEGER(HID_T) :: H5T_STD_B8LE
  INTEGER(HID_T) :: H5T_STD_B16BE
  INTEGER(HID_T) :: H5T_STD_B16LE
  INTEGER(HID_T) :: H5T_STD_B32BE
  INTEGER(HID_T) :: H5T_STD_B32LE
  INTEGER(HID_T) :: H5T_STD_B64BE
  INTEGER(HID_T) :: H5T_STD_B64LE
  INTEGER(HID_T) :: H5T_NATIVE_B8
  INTEGER(HID_T) :: H5T_NATIVE_B16
  INTEGER(HID_T) :: H5T_NATIVE_B32
  INTEGER(HID_T) :: H5T_NATIVE_B64
  INTEGER(HID_T) :: H5T_FORTRAN_S1
  INTEGER(HID_T) :: H5T_C_S1
  !
  ! H5F flags
  !
  INTEGER :: H5F_ACC_RDWR_F
  INTEGER :: H5F_ACC_RDONLY_F
  INTEGER :: H5F_ACC_TRUNC_F
  INTEGER :: H5F_ACC_EXCL_F
  INTEGER :: H5F_ACC_DEBUG_F
  INTEGER :: H5F_SCOPE_LOCAL_F
  INTEGER :: H5F_SCOPE_GLOBAL_F
  INTEGER :: H5F_CLOSE_DEFAULT_F
  INTEGER :: H5F_CLOSE_WEAK_F
  INTEGER :: H5F_CLOSE_SEMI_F
  INTEGER :: H5F_CLOSE_STRONG_F
  INTEGER :: H5F_OBJ_FILE_F
  INTEGER :: H5F_OBJ_DATASET_F
  INTEGER :: H5F_OBJ_GROUP_F
  INTEGER :: H5F_OBJ_DATATYPE_F
  INTEGER :: H5F_OBJ_ALL_F
  INTEGER :: H5F_LIBVER_EARLIEST_F
  INTEGER :: H5F_LIBVER_LATEST_F
  INTEGER :: H5F_UNLIMITED_F
  !
  ! H5generic flags declaration
  !
  INTEGER :: H5_INDEX_UNKNOWN_F
  INTEGER :: H5_INDEX_NAME_F
  INTEGER :: H5_INDEX_CRT_ORDER_F
  INTEGER :: H5_INDEX_N_F
  INTEGER :: H5_ITER_UNKNOWN_F
  INTEGER :: H5_ITER_INC_F
  INTEGER :: H5_ITER_DEC_F
  INTEGER :: H5_ITER_NATIVE_F
  INTEGER :: H5_ITER_N_F
  !
  ! H5G flags declaration
  !
  INTEGER :: H5G_UNKNOWN_F
  INTEGER :: H5G_GROUP_F
  INTEGER :: H5G_DATASET_F
  INTEGER :: H5G_TYPE_F
  INTEGER :: H5G_LINK_F
  INTEGER :: H5G_UDLINK_F
  INTEGER :: H5G_SAME_LOC_F
  INTEGER :: H5G_LINK_ERROR_F
  INTEGER :: H5G_LINK_HARD_F
  INTEGER :: H5G_LINK_SOFT_F
  INTEGER :: H5G_STORAGE_TYPE_UNKNOWN_F
  INTEGER :: H5G_STORAGE_TYPE_SYMBOL_TABLE_F
  INTEGER :: H5G_STORAGE_TYPE_COMPACT_F
  INTEGER :: H5G_STORAGE_TYPE_DENSE_F
  !
  ! H5D flags declaration
  !
  INTEGER :: H5D_COMPACT_F
  INTEGER :: H5D_CONTIGUOUS_F
  INTEGER :: H5D_CHUNKED_F
  INTEGER :: H5D_ALLOC_TIME_ERROR_F
  INTEGER :: H5D_ALLOC_TIME_DEFAULT_F
  INTEGER :: H5D_ALLOC_TIME_EARLY_F
  INTEGER :: H5D_ALLOC_TIME_LATE_F
  INTEGER :: H5D_ALLOC_TIME_INCR_F
  INTEGER :: H5D_SPACE_STS_ERROR_F
  INTEGER :: H5D_SPACE_STS_NOT_ALLOCATED_F
  INTEGER :: H5D_SPACE_STS_PART_ALLOCATED_F
  INTEGER :: H5D_SPACE_STS_ALLOCATED_F
  INTEGER :: H5D_FILL_TIME_ERROR_F
  INTEGER :: H5D_FILL_TIME_ALLOC_F
  INTEGER :: H5D_FILL_TIME_NEVER_F
  INTEGER :: H5D_FILL_VALUE_ERROR_F
  INTEGER :: H5D_FILL_VALUE_UNDEFINED_F
  INTEGER :: H5D_FILL_VALUE_DEFAULT_F
  INTEGER :: H5D_FILL_VALUE_USER_DEFINED_F

! shortened "_DEFAULT" to "_DFLT" to satisfy the limit of 31
! characters for variable names in Fortran.
! shortened "_CONTIGUOUS" to "_CONTIG" to satisfy the limit of 31
! characters for variable names in Fortran.

  INTEGER(SIZE_T) :: H5D_CHUNK_CACHE_NSLOTS_DFLT_F
  INTEGER(SIZE_T) :: H5D_CHUNK_CACHE_NBYTES_DFLT_F
  INTEGER :: H5D_CHUNK_CACHE_W0_DFLT_F
  INTEGER :: H5D_MPIO_NO_COLLECTIVE_F
  INTEGER :: H5D_MPIO_CHUNK_INDEPENDENT_F
  INTEGER :: H5D_MPIO_CHUNK_COLLECTIVE_F
  INTEGER :: H5D_MPIO_CHUNK_MIXED_F
  INTEGER :: H5D_MPIO_CONTIG_COLLECTIVE_F
  !
  ! H5E flags declaration
  !
  INTEGER(HID_T) :: H5E_DEFAULT_F
  INTEGER :: H5E_MAJOR_F
  INTEGER :: H5E_MINOR_F
  INTEGER :: H5E_WALK_UPWARD_F
  INTEGER :: H5E_WALK_DOWNWARD_F
  !
  ! H5FD flags declaration
  !
  INTEGER :: H5FD_MPIO_INDEPENDENT_F
  INTEGER :: H5FD_MPIO_COLLECTIVE_F
  INTEGER :: H5FD_MEM_NOLIST_F
  INTEGER :: H5FD_MEM_DEFAULT_F
  INTEGER :: H5FD_MEM_SUPER_F
  INTEGER :: H5FD_MEM_BTREE_F
  INTEGER :: H5FD_MEM_DRAW_F
  INTEGER :: H5FD_MEM_GHEAP_F
  INTEGER :: H5FD_MEM_LHEAP_F
  INTEGER :: H5FD_MEM_OHDR_F
  INTEGER :: H5FD_MEM_NTYPES_F
  !
  ! H5FD file drivers flags declaration
  !
  INTEGER(HID_T) :: H5FD_CORE_F
  INTEGER(HID_T) :: H5FD_FAMILY_F
  INTEGER(HID_T) :: H5FD_LOG_F
  INTEGER(HID_T) :: H5FD_MPIO_F
  INTEGER(HID_T) :: H5FD_MULTI_F
  INTEGER(HID_T) :: H5FD_SEC2_F
  INTEGER(HID_T) :: H5FD_STDIO_F
  !
  ! H5I flags declaration
  !
  INTEGER ::  H5I_FILE_F
  INTEGER ::  H5I_GROUP_F
  INTEGER ::  H5I_DATATYPE_F
  INTEGER ::  H5I_DATASPACE_F
  INTEGER ::  H5I_DATASET_F
  INTEGER ::  H5I_ATTR_F
  INTEGER ::  H5I_BADID_F
  !
  ! H5L flags declaration
  !
  INTEGER :: H5L_TYPE_ERROR_F
  INTEGER :: H5L_TYPE_HARD_F
  INTEGER :: H5L_TYPE_SOFT_F
  INTEGER :: H5L_TYPE_EXTERNAL_F
  INTEGER :: H5L_SAME_LOC_F
  INTEGER :: H5L_LINK_CLASS_T_VERS_F
  !
  ! H5O flags declaration
  !
  INTEGER :: H5O_COPY_SHALLOW_HIERARCHY_F ! *** THESE VARIABLES DO
  INTEGER :: H5O_COPY_EXPAND_SOFT_LINK_F  ! NOT MATCH THE C VARIABLE
  INTEGER :: H5O_COPY_EXPAND_EXT_LINK_F   ! IN ORDER
  INTEGER :: H5O_COPY_EXPAND_REFERENCE_F  ! TO STAY UNDER THE
  INTEGER :: H5O_COPY_WITHOUT_ATTR_FLAG_F
  INTEGER :: H5O_COPY_PRESERVE_NULL_FLAG_F
  INTEGER :: H5O_COPY_ALL_F
  INTEGER :: H5O_SHMESG_NONE_FLAG_F
  INTEGER :: H5O_SHMESG_SDSPACE_FLAG_F
  INTEGER :: H5O_SHMESG_DTYPE_FLAG_F
  INTEGER :: H5O_SHMESG_FILL_FLAG_F
  INTEGER :: H5O_SHMESG_PLINE_FLAG_F
  INTEGER :: H5O_SHMESG_ATTR_FLAG_F
  INTEGER :: H5O_SHMESG_ALL_FLAG_F
  INTEGER :: H5O_HDR_CHUNK0_SIZE_F
  INTEGER :: H5O_HDR_ATTR_CRT_ORDER_TRACK_F ! 32 CHARACTER
  INTEGER :: H5O_HDR_ATTR_CRT_ORDER_INDEX_F ! VARIABLE
  INTEGER :: H5O_HDR_ATTR_STORE_PHASE_CHA_F ! LENGTH ***
  INTEGER :: H5O_HDR_STORE_TIMES_F
  INTEGER :: H5O_HDR_ALL_FLAGS_F
  INTEGER :: H5O_SHMESG_MAX_NINDEXES_F
  INTEGER :: H5O_SHMESG_MAX_LIST_SIZE_F
  INTEGER :: H5O_TYPE_UNKNOWN_F
  INTEGER :: H5O_TYPE_GROUP_F
  INTEGER :: H5O_TYPE_DATASET_F
  INTEGER :: H5O_TYPE_NAMED_DATATYPE_F
  INTEGER :: H5O_TYPE_NTYPES_F
  !
  ! H5P flags declaration
  !
  INTEGER(HID_T) :: H5P_FILE_CREATE_F
  INTEGER(HID_T) :: H5P_FILE_ACCESS_F
  INTEGER(HID_T) :: H5P_DATASET_CREATE_F
  INTEGER(HID_T) :: H5P_DATASET_XFER_F
  INTEGER(HID_T) :: H5P_FILE_MOUNT_F
  INTEGER(HID_T) :: H5P_DEFAULT_F
  INTEGER(HID_T) :: H5P_ROOT_F
  INTEGER(HID_T) :: H5P_OBJECT_CREATE_F
  INTEGER(HID_T) :: H5P_DATASET_ACCESS_F
  INTEGER(HID_T) :: H5P_GROUP_CREATE_F
  INTEGER(HID_T) :: H5P_GROUP_ACCESS_F
  INTEGER(HID_T) :: H5P_DATATYPE_CREATE_F
  INTEGER(HID_T) :: H5P_DATATYPE_ACCESS_F
  INTEGER(HID_T) :: H5P_STRING_CREATE_F
  INTEGER(HID_T) :: H5P_ATTRIBUTE_CREATE_F
  INTEGER(HID_T) :: H5P_OBJECT_COPY_F
  INTEGER(HID_T) :: H5P_LINK_CREATE_F
  INTEGER(HID_T) :: H5P_LINK_ACCESS_F
  !
  ! H5P integers flags declaration
  !
  INTEGER :: H5P_CRT_ORDER_INDEXED_F
  INTEGER :: H5P_CRT_ORDER_TRACKED_F
  !
  ! H5R flags declaration
  !
  INTEGER :: H5R_OBJECT_F
  INTEGER :: H5R_DATASET_REGION_F
  !
  ! H5S flags declaration
  !
  INTEGER(HSIZE_T) :: H5S_UNLIMITED_F
  INTEGER :: H5S_SCALAR_F
  INTEGER :: H5S_SIMPLE_F
  INTEGER :: H5S_NULL_F
  INTEGER :: H5S_ALL_F
  INTEGER :: H5S_SELECT_NOOP_F
  INTEGER :: H5S_SELECT_SET_F
  INTEGER :: H5S_SELECT_OR_F
  INTEGER :: H5S_SELECT_AND_F
  INTEGER :: H5S_SELECT_XOR_F
  INTEGER :: H5S_SELECT_NOTB_F
  INTEGER :: H5S_SELECT_NOTA_F
  INTEGER :: H5S_SELECT_APPEND_F
  INTEGER :: H5S_SELECT_PREPEND_F
  INTEGER :: H5S_SELECT_INVALID_F
  INTEGER :: H5S_SEL_ERROR_F
  INTEGER :: H5S_SEL_NONE_F
  INTEGER :: H5S_SEL_POINTS_F
  INTEGER :: H5S_SEL_HYPERSLABS_F
  INTEGER :: H5S_SEL_ALL_F
  !
  ! H5T flags declaration
  !
  INTEGER :: H5T_NO_CLASS_F
  INTEGER :: H5T_INTEGER_F
  INTEGER :: H5T_FLOAT_F
  INTEGER :: H5T_TIME_F
  INTEGER :: H5T_STRING_F
  INTEGER :: H5T_BITFIELD_F
  INTEGER :: H5T_OPAQUE_F
  INTEGER :: H5T_COMPOUND_F
  INTEGER :: H5T_REFERENCE_F
  INTEGER :: H5T_ENUM_F
  INTEGER :: H5T_VLEN_F
  INTEGER :: H5T_ARRAY_F
  INTEGER :: H5T_ORDER_LE_F
  INTEGER :: H5T_ORDER_BE_F
  INTEGER :: H5T_ORDER_VAX_F
  INTEGER :: H5T_ORDER_NONE_F
  INTEGER :: H5T_ORDER_MIXED_F
  INTEGER :: H5T_PAD_ZERO_F
  INTEGER :: H5T_PAD_ONE_F
  INTEGER :: H5T_PAD_BACKGROUND_F
  INTEGER :: H5T_PAD_ERROR_F
  INTEGER :: H5T_SGN_NONE_F
  INTEGER :: H5T_SGN_2_F
  INTEGER :: H5T_SGN_ERROR_F
  INTEGER :: H5T_NORM_IMPLIED_F
  INTEGER :: H5T_NORM_MSBSET_F
  INTEGER :: H5T_NORM_NONE_F
  INTEGER :: H5T_CSET_ASCII_F
  INTEGER :: H5T_CSET_UTF8_F
  INTEGER :: H5T_STR_NULLTERM_F
  INTEGER :: H5T_STR_NULLPAD_F
  INTEGER :: H5T_STR_SPACEPAD_F
  INTEGER :: H5T_STR_ERROR_F
  INTEGER :: H5T_DIR_ASCEND_F
  INTEGER :: H5T_DIR_DESCEND_F
  !
  ! H5Z flags declaration
  !
  INTEGER :: H5Z_FILTER_ERROR_F
  INTEGER :: H5Z_FILTER_NONE_F
  INTEGER :: H5Z_FILTER_ALL_F
  INTEGER :: H5Z_FILTER_DEFLATE_F
  INTEGER :: H5Z_FILTER_SHUFFLE_F
  INTEGER :: H5Z_FILTER_FLETCHER32_F
  INTEGER :: H5Z_FILTER_SZIP_F
  INTEGER :: H5Z_ERROR_EDC_F
  INTEGER :: H5Z_DISABLE_EDC_F
  INTEGER :: H5Z_ENABLE_EDC_F
  INTEGER :: H5Z_NO_EDC_F
  INTEGER :: H5Z_FLAG_OPTIONAL_F
  INTEGER :: H5Z_FILTER_ENCODE_ENABLED_F
  INTEGER :: H5Z_FILTER_DECODE_ENABLED_F
  INTEGER :: H5Z_FILTER_NBIT_F
  INTEGER :: H5Z_FILTER_SCALEOFFSET_F
  INTEGER :: H5Z_SO_FLOAT_DSCALE_F
  INTEGER :: H5Z_SO_FLOAT_ESCALE_F
  INTEGER :: H5Z_SO_INT_F
  INTEGER :: H5Z_SO_INT_MINBITS_DEFAULT_F
  !
  ! H5 Library flags declaration
  !
  INTEGER :: H5_SZIP_EC_OM_F
  INTEGER :: H5_SZIP_NN_OM_F

END MODULE H5GLOBAL

