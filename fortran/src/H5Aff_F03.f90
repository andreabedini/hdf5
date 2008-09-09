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
!
! This file contains Fortran2003 interfaces for H5A functions.
!
! ISSUE #1 C_LOC and character strings according to the Fortran 2003 standard:
!
! 15.1.2.5 C_LOC(X)
!
!       Argument. X shall either  
!
!     (1) have interoperable type and type parameters and be
!         (a) a variable that has the TARGET attribute and is interoperable,
!         (b) an allocated allocatable variable that has the TARGET attribute 
!              and is not an array of zero size, or
!         (c) an associated scalar pointer, or
!     (2) be a nonpolymorphic scalar, have no length type parameters, and be
!         (a) a nonallocatable, nonpointer variable that has the TARGET attribute,
!         (b) an allocated allocatable variable that has the TARGET attribute, or
!         (c) an associated pointer.
!
! - When X is a character, for interoperability the standard is:
!
! 15.2.1 Interoperability of intrinsic types
! 
!  ...if the type is character, interoperability also requires that the length type parameter 
!     be omitted or be specified by an initialization expression whose value is one. 
!
! CONCLUSION:
!
! Therefore compilers that have not extended the standard (gfortran and Sun fortran) require
!
! CHARACTER(LEN=1), TARGET :: chr
!  or
! CHARACTER, TARGET :: chr
!

MODULE H5A_PROVISIONAL

  USE H5GLOBAL
  !
  !On Windows there are no big (integer*8) integers, so overloading 
  !for bug #670 does not work. I have to use DEC compilation directives to make
  !Windows DEC Visual Fortran and OSF compilers happy and do right things.
  !						05/01/02 EP
  !
  INTERFACE h5awrite_f

     MODULE PROCEDURE h5awrite_integer_scalar 
     MODULE PROCEDURE h5awrite_integer_1 
     MODULE PROCEDURE h5awrite_integer_2 
     MODULE PROCEDURE h5awrite_integer_3 
     MODULE PROCEDURE h5awrite_integer_4 
     MODULE PROCEDURE h5awrite_integer_5 
     MODULE PROCEDURE h5awrite_integer_6 
     MODULE PROCEDURE h5awrite_integer_7 
     MODULE PROCEDURE h5awrite_char_scalar 
     MODULE PROCEDURE h5awrite_char_1 
     MODULE PROCEDURE h5awrite_char_2 
     MODULE PROCEDURE h5awrite_char_3 
     MODULE PROCEDURE h5awrite_char_4 
     MODULE PROCEDURE h5awrite_char_5 
     MODULE PROCEDURE h5awrite_char_6 
     MODULE PROCEDURE h5awrite_char_7 
     MODULE PROCEDURE h5awrite_real_scalar
     MODULE PROCEDURE h5awrite_real_1
     MODULE PROCEDURE h5awrite_real_2
     MODULE PROCEDURE h5awrite_real_3
     MODULE PROCEDURE h5awrite_real_4
     MODULE PROCEDURE h5awrite_real_5
     MODULE PROCEDURE h5awrite_real_6
     MODULE PROCEDURE h5awrite_real_7
     ! This is the preferred way to call h5awrite
     ! by passing an address
     MODULE PROCEDURE h5awrite_ptr

  END INTERFACE

  INTERFACE h5aread_f

     MODULE PROCEDURE h5aread_integer_scalar
     MODULE PROCEDURE h5aread_integer_1 
     MODULE PROCEDURE h5aread_integer_2 
     MODULE PROCEDURE h5aread_integer_3 
     MODULE PROCEDURE h5aread_integer_4 
     MODULE PROCEDURE h5aread_integer_5 
     MODULE PROCEDURE h5aread_integer_6 
     MODULE PROCEDURE h5aread_integer_7 
     MODULE PROCEDURE h5aread_char_scalar 
     MODULE PROCEDURE h5aread_char_1 
     MODULE PROCEDURE h5aread_char_2 
     MODULE PROCEDURE h5aread_char_3 
     MODULE PROCEDURE h5aread_char_4 
     MODULE PROCEDURE h5aread_char_5 
     MODULE PROCEDURE h5aread_char_6 
     MODULE PROCEDURE h5aread_char_7 
     MODULE PROCEDURE h5aread_real_scalar
     MODULE PROCEDURE h5aread_real_1
     MODULE PROCEDURE h5aread_real_2
     MODULE PROCEDURE h5aread_real_3
     MODULE PROCEDURE h5aread_real_4
     MODULE PROCEDURE h5aread_real_5
     MODULE PROCEDURE h5aread_real_6
     MODULE PROCEDURE h5aread_real_7

     ! This is the preferred way to call h5aread
     ! by passing an address
     MODULE PROCEDURE h5aread_ptr
     !
  END INTERFACE

  ! Interface for the function used to pass the C pointer of the buffer
  ! to the C H5Awrite routine

  INTERFACE
     INTEGER FUNCTION h5awrite_f_c(attr_id, mem_type_id, buf)
       USE H5GLOBAL
       USE, INTRINSIC :: ISO_C_BINDING
       !DEC$ IF DEFINED(HDF5F90_WINDOWS)
       !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5AWRITE_F_C'::h5awrite_f_c
       !DEC$ ENDIF
       INTEGER(HID_T), INTENT(IN) :: attr_id
       INTEGER(HID_T), INTENT(IN) :: mem_type_id
       TYPE(C_PTR), VALUE :: buf
     END FUNCTION h5awrite_f_c
  END INTERFACE

  ! Interface for the function used to pass the C pointer of the buffer
  ! to the C H5Aread routine

  INTERFACE
     INTEGER FUNCTION h5aread_f_c(attr_id, mem_type_id, buf)
       USE H5GLOBAL
       USE, INTRINSIC :: ISO_C_BINDING
       !DEC$ IF DEFINED(HDF5F90_WINDOWS)
       !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5AREAD_F_C'::h5aread_f_c
       !DEC$ ENDIF
       INTEGER(HID_T), INTENT(IN) :: attr_id 
       INTEGER(HID_T), INTENT(IN) :: mem_type_id
       TYPE(C_PTR), VALUE :: buf
     END FUNCTION h5aread_f_c
  END INTERFACE


CONTAINS

  SUBROUTINE h5awrite_integer_scalar(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(IN), TARGET :: buf              ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_integer_scalar

  SUBROUTINE h5awrite_integer_1(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(IN) , &
         DIMENSION(dims(1)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_integer_1


  SUBROUTINE h5awrite_integer_2(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(IN) , &
         DIMENSION(dims(1),dims(2)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_integer_2

  SUBROUTINE h5awrite_integer_3(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(IN), DIMENSION(dims(1),dims(2),dims(3)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_integer_3


  SUBROUTINE h5awrite_integer_4(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_integer_4


  SUBROUTINE h5awrite_integer_5(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(IN), DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_integer_5


  SUBROUTINE h5awrite_integer_6(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_integer_6


  SUBROUTINE h5awrite_integer_7(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_integer_7


  SUBROUTINE h5awrite_real_scalar(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(IN), TARGET :: buf                 ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_real_scalar

  SUBROUTINE h5awrite_real_1(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(IN), &
         DIMENSION(dims(1)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_real_1


  SUBROUTINE h5awrite_real_2(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_real_2


  SUBROUTINE h5awrite_real_3(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_real_3


  SUBROUTINE h5awrite_real_4(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_real_4


  SUBROUTINE h5awrite_real_5(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_real_5


  SUBROUTINE h5awrite_real_6(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_real_6


  SUBROUTINE h5awrite_real_7(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_real_7


  SUBROUTINE h5awrite_double_scalar(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(IN), TARGET :: buf     ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_double_scalar

  SUBROUTINE h5awrite_double_1(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1)), TARGET :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_double_1


  SUBROUTINE h5awrite_double_2(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(IN), & 
         DIMENSION(dims(1),dims(2)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_double_2


  SUBROUTINE h5awrite_double_3(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_double_3


  SUBROUTINE h5awrite_double_4(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_double_4


  SUBROUTINE h5awrite_double_5(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_double_5


  SUBROUTINE h5awrite_double_6(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

  END SUBROUTINE h5awrite_double_6


  SUBROUTINE h5awrite_double_7(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5awrite_double_7

  SUBROUTINE h5awrite_char_scalar(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*),INTENT(IN) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    DO i = 1, chr_len
       chr(i) = buf(i:i)
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

    DEALLOCATE(chr)

  END SUBROUTINE h5awrite_char_scalar

  SUBROUTINE h5awrite_char_1(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id    ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims     ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(IN), DIMENSION(dims(1)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*dims(1)), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i1 = 1, dims(1)
       DO j = 1, chr_len
          k = k + 1
          chr(k) = buf(i1)(j:j)
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

    DEALLOCATE(chr)

  END SUBROUTINE h5awrite_char_1


  SUBROUTINE h5awrite_char_2(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2)) :: buf  ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:2))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i2 = 1, dims(2)
       DO i1 = 1, dims(1)
          DO j = 1, chr_len
             k = k + 1
             chr(k) = buf(i1,i2)(j:j)
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

    DEALLOCATE(chr)

  END SUBROUTINE h5awrite_char_2


  SUBROUTINE h5awrite_char_3(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:3))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i3 = 1, dims(3)
       DO i2 = 1, dims(2)
          DO i1 = 1, dims(1)
             DO j = 1, chr_len
                k = k + 1
                chr(k) = buf(i1,i2,i3)(j:j)
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

    DEALLOCATE(chr)

  END SUBROUTINE h5awrite_char_3


  SUBROUTINE h5awrite_char_4(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,i4,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:4))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i4 = 1, dims(4)
       DO i3 = 1, dims(3)
          DO i2 = 1, dims(2)
             DO i1 = 1, dims(1)
                DO j = 1, chr_len
                   k = k + 1
                   chr(k) = buf(i1,i2,i3,i4)(j:j)
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

    DEALLOCATE(chr)

  END SUBROUTINE h5awrite_char_4


  SUBROUTINE h5awrite_char_5(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,i4,i5,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:5))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i5 = 1, dims(5)
       DO i4 = 1, dims(4)
          DO i3 = 1, dims(3)
             DO i2 = 1, dims(2)
                DO i1 = 1, dims(1)
                   DO j = 1, chr_len
                      k = k + 1
                      chr(k) = buf(i1,i2,i3,i4,i5)(j:j)
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

    DEALLOCATE(chr)
  END SUBROUTINE h5awrite_char_5


  SUBROUTINE h5awrite_char_6(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,i4,i5,i6,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:6))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i6 = 1, dims(6)
       DO i5 = 1, dims(5)
          DO i4 = 1, dims(4)
             DO i3 = 1, dims(3)
                DO i2 = 1, dims(2)
                   DO i1 = 1, dims(1)
                      DO j = 1, chr_len
                         k = k + 1
                         chr(k) = buf(i1,i2,i3,i4,i5,i6)(j:j)
                      ENDDO
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

    DEALLOCATE(chr)
  END SUBROUTINE h5awrite_char_6


  SUBROUTINE h5awrite_char_7(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,i4,i5,i6,i7,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:7))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i7 = 1, dims(7)
       DO i6 = 1, dims(6)
          DO i5 = 1, dims(5)
             DO i4 = 1, dims(4)
                DO i3 = 1, dims(3)
                   DO i2 = 1, dims(2)
                      DO i1 = 1, dims(1)
                         DO j = 1, chr_len
                            k = k + 1
                            chr(k) = buf(i1,i2,i3,i4,i5,i6,i7)(j:j)
                         ENDDO
                      ENDDO
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5awrite_f_c(attr_id, memtype_id, f_ptr)

    DEALLOCATE(chr)

  END SUBROUTINE h5awrite_char_7

  !----------------------------------------------------------------------
  ! Name:		h5aread_f 
  !
  ! Purpose:  	Reads an attribute.
  !
  ! Inputs:  
  !		attr_id		- attribute identifier
  !		memtype_id	- attribute memory type identifier
  !		dims		- 1D array of size 7, stores sizes of the 
  !				- buf array dimensions.
  ! Outputs:  
  !		buf		- buffer to read attribute data in
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE			
  !
  ! Programmer:	Elena Pourmal
  !		August 12, 1999	
  !
  ! Modifications: 	Explicit Fortran interfaces are added for 
  !			called C functions (it is needed for Windows
  !			port).  February 27, 2001 
  !
  !			dims parameter was added to make code portable;
  !			Aprile 4, 2001
  !
  !                       Changed buf intent to INOUT to be consistant
  !                       with how the C functions handles it. The pg
  !                       compiler will return 0 if a buf value is not set.
  !                       February, 2008
  !
  ! Comment:		This function is overloaded to write INTEGER,
  !			REAL, DOUBLE PRECISION and CHARACTER buffers
  !			up to 7 dimensions.	
  !----------------------------------------------------------------------

  SUBROUTINE h5aread_integer_scalar(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(INOUT), TARGET :: buf             ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_integer_scalar

  SUBROUTINE h5aread_integer_1(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(INOUT), DIMENSION(dims(1)), TARGET :: buf
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_integer_1


  SUBROUTINE h5aread_integer_2(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(INOUT),DIMENSION(dims(1),dims(2)), TARGET :: buf
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_integer_2


  SUBROUTINE h5aread_integer_3(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3)), TARGET :: buf
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_integer_3


  SUBROUTINE h5aread_integer_4(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_integer_4


  SUBROUTINE h5aread_integer_5(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_integer_5


  SUBROUTINE h5aread_integer_6(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_integer_6


  SUBROUTINE h5aread_integer_7(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_integer_7


  SUBROUTINE h5aread_real_scalar(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(INOUT), TARGET :: buf                ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_real_scalar

  SUBROUTINE h5aread_real_1(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_real_1


  SUBROUTINE h5aread_real_2(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_real_2


  SUBROUTINE h5aread_real_3(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_real_3


  SUBROUTINE h5aread_real_4(attr_id, memtype_id,  buf, dims, hdferr) 
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_real_4


  SUBROUTINE h5aread_real_5(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_real_5


  SUBROUTINE h5aread_real_6(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_real_6


  SUBROUTINE h5aread_real_7(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_real_7


  SUBROUTINE h5aread_double_scalar(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(INOUT), TARGET :: buf    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_double_scalar

  SUBROUTINE h5aread_double_1(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_double_1


  SUBROUTINE h5aread_double_2(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_double_2


  SUBROUTINE h5aread_double_3(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_double_3


  SUBROUTINE h5aread_double_4(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_double_4


  SUBROUTINE h5aread_double_5(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_double_5


  SUBROUTINE h5aread_double_6(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_double_6


  SUBROUTINE h5aread_double_7(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)), TARGET :: buf
    ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code
    TYPE(C_PTR) :: f_ptr

    f_ptr = C_LOC(buf)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
  END SUBROUTINE h5aread_double_7


  SUBROUTINE h5aread_char_scalar(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id    ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(INOUT) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr         ! Error code

    INTEGER :: i
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr
    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    DO i = 1, chr_len
       chr(i) = buf(i:i)
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)

    DO i = 1, chr_len
       buf(i:i) = chr(i)
    ENDDO

    DEALLOCATE(chr)

  END SUBROUTINE h5aread_char_scalar

  SUBROUTINE h5aread_char_1(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id    ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1)), TARGET :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*dims(1)), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i1 = 1, dims(1)
       DO j = 1, chr_len
          k = k + 1
          chr(k) = buf(i1)(j:j)
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)

    k = 0
    DO i1 = 1, dims(1)
       DO j = 1, chr_len
          k = k + 1
          buf(i1)(j:j) = chr(k)
       ENDDO
    ENDDO

    DEALLOCATE(chr)
  END SUBROUTINE h5aread_char_1


  SUBROUTINE h5aread_char_2(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2)) :: buf  ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:2))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i2 = 1, dims(2)
       DO i1 = 1, dims(1)
          DO j = 1, chr_len
             k = k + 1
             chr(k) = buf(i1,i2)(j:j)
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
    k = 0
    DO i2 = 1, dims(2)
       DO i1 = 1, dims(1)
          DO j = 1, chr_len
             k = k + 1
             buf(i1,i2)(j:j) = chr(k)
          ENDDO
       ENDDO
    ENDDO

    DEALLOCATE(chr)
  END SUBROUTINE h5aread_char_2


  SUBROUTINE h5aread_char_3(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:3))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i3 = 1, dims(3)
       DO i2 = 1, dims(2)
          DO i1 = 1, dims(1)
             DO j = 1, chr_len
                k = k + 1
                chr(k) = buf(i1,i2,i3)(j:j)
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)

    k = 0
    DO i3 = 1, dims(3)
       DO i2 = 1, dims(2)
          DO i1 = 1, dims(1)
             DO j = 1, chr_len
                k = k + 1
                buf(i1,i2,i3)(j:j) = chr(k)
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    DEALLOCATE(chr)
  END SUBROUTINE h5aread_char_3


  SUBROUTINE h5aread_char_4(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
    ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,i4,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:4))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i4 = 1, dims(4)
       DO i3 = 1, dims(3)
          DO i2 = 1, dims(2)
             DO i1 = 1, dims(1)
                DO j = 1, chr_len
                   k = k + 1
                   chr(k) = buf(i1,i2,i3,i4)(j:j)
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
    k = 0
    DO i4 = 1, dims(4)
       DO i3 = 1, dims(3)
          DO i2 = 1, dims(2)
             DO i1 = 1, dims(1)
                DO j = 1, chr_len
                   k = k + 1
                   buf(i1,i2,i3,i4)(j:j) = chr(k)
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO
    DEALLOCATE(chr)
  END SUBROUTINE h5aread_char_4


  SUBROUTINE h5aread_char_5(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,i4,i5,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:5))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i5 = 1, dims(5)
       DO i4 = 1, dims(4)
          DO i3 = 1, dims(3)
             DO i2 = 1, dims(2)
                DO i1 = 1, dims(1)
                   DO j = 1, chr_len
                      k = k + 1
                      chr(k) = buf(i1,i2,i3,i4,i5)(j:j)
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)

    k = 0
    DO i5 = 1, dims(5)
       DO i4 = 1, dims(4)
          DO i3 = 1, dims(3)
             DO i2 = 1, dims(2)
                DO i1 = 1, dims(1)
                   DO j = 1, chr_len
                      k = k + 1
                      buf(i1,i2,i3,i4,i5)(j:j) = chr(k)
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    DEALLOCATE(chr)
  END SUBROUTINE h5aread_char_5


  SUBROUTINE h5aread_char_6(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,i4,i5,i6,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:6))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i6 = 1, dims(6)
       DO i5 = 1, dims(5)
          DO i4 = 1, dims(4)
             DO i3 = 1, dims(3)
                DO i2 = 1, dims(2)
                   DO i1 = 1, dims(1)
                      DO j = 1, chr_len
                         k = k + 1
                         chr(k) = buf(i1,i2,i3,i4,i5,i6)(j:j)
                      ENDDO
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)

    k = 0
    DO i6 = 1, dims(6)
       DO i5 = 1, dims(5)
          DO i4 = 1, dims(4)
             DO i3 = 1, dims(3)
                DO i2 = 1, dims(2)
                   DO i1 = 1, dims(1)
                      DO j = 1, chr_len
                         k = k + 1
                         buf(i1,i2,i3,i4,i5,i6)(j:j) = chr(k)
                      ENDDO
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    DEALLOCATE(chr)
  END SUBROUTINE h5aread_char_6


  SUBROUTINE h5aread_char_7(attr_id, memtype_id,  buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier 
    INTEGER(HID_T), INTENT(IN) :: memtype_id ! Attribute datatype 
                                             ! identifier  (in memory)
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims       ! Array to story buf dimension sizes 
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf ! Attribute data 
    INTEGER, INTENT(OUT) :: hdferr          ! Error code

    INTEGER :: i1,i2,i3,i4,i5,i6,i7,j,k
    CHARACTER(LEN=1), ALLOCATABLE, DIMENSION(:), TARGET :: chr
    INTEGER :: chr_len

    TYPE(C_PTR) :: f_ptr

    ! To resolve Issue #1 outlined in the preamble of this file we
    ! need to pack the character string into an array.

    chr_len = LEN(buf)
    ALLOCATE(chr(1:chr_len*SUM(dims(1:7))), STAT=hdferr)
    IF (hdferr .NE. 0) THEN
       hdferr = -1
       RETURN
    ENDIF

    k = 0
    DO i7 = 1, dims(7)
       DO i6 = 1, dims(6)
          DO i5 = 1, dims(5)
             DO i4 = 1, dims(4)
                DO i3 = 1, dims(3)
                   DO i2 = 1, dims(2)
                      DO i1 = 1, dims(1)
                         DO j = 1, chr_len
                            k = k + 1
                            chr(k) = buf(i1,i2,i3,i4,i5,i6,i7)(j:j)
                         ENDDO
                      ENDDO
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    f_ptr = C_LOC(chr)

    hdferr = h5aread_f_c(attr_id, memtype_id, f_ptr)
    k = 0
    DO i7 = 1, dims(7)
       DO i6 = 1, dims(6)
          DO i5 = 1, dims(5)
             DO i4 = 1, dims(4)
                DO i3 = 1, dims(3)
                   DO i2 = 1, dims(2)
                      DO i1 = 1, dims(1)
                         DO j = 1, chr_len
                            k = k + 1
                            buf(i1,i2,i3,i4,i5,i6,i7)(j:j) = chr(k)
                         ENDDO
                      ENDDO
                   ENDDO
                ENDDO
             ENDDO
          ENDDO
       ENDDO
    ENDDO

    DEALLOCATE(chr)

  END SUBROUTINE h5aread_char_7

  SUBROUTINE h5awrite_ptr(attr_id, mem_type_id, buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    TYPE(C_PTR), INTENT(IN), TARGET :: buf
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims ! not used
    INTEGER, INTENT(OUT) :: hdferr      ! Error code

    hdferr = h5awrite_f_c(attr_id, mem_type_id, buf)

  END SUBROUTINE h5awrite_ptr

  SUBROUTINE h5aread_ptr(attr_id, mem_type_id, buf, dims, hdferr)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: attr_id   ! Attribute identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    TYPE(C_PTR), INTENT(IN), TARGET :: buf
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims ! not used
    INTEGER, INTENT(OUT) :: hdferr      ! Error code

    hdferr = h5aread_f_c(attr_id, mem_type_id, buf)

  END SUBROUTINE h5aread_ptr

END MODULE H5A_PROVISIONAL


