!****h* root/fortran/test/tH5T_F03.f90
!
! NAME
!  tH5T_F03.f90
!
! FUNCTION
!  Test FORTRAN HDF5 H5T APIs which are dependent on FORTRAN 2003
!  features. 
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
!
! CONTAINS SUBROUTINES
!  test_array_compound_atomic, test_array_compound_array,
!  test_array_bkg, test_h5kind_to_type
!
!*****

! *****************************************
! ***        H 5 T   T E S T S
! *****************************************

!/****************************************************************
!**
!**  test_array_compound_atomic(): Test basic array datatype code.
!**  Tests 1-D array of compound datatypes (with no array fields)
!**
!****************************************************************/
!
SUBROUTINE test_array_compound_atomic(total_error)

  USE HDF5 
  USE ISO_C_BINDING
  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error
  ! /* 1-D dataset WITH fixed dimensions */
  CHARACTER(LEN=6), PARAMETER :: SPACE1_NAME = "Space1"
  INTEGER, PARAMETER :: SPACE1_RANK = 1
  INTEGER, PARAMETER :: SPACE1_DIM1 = 4
  ! /* 1-D array datatype */
  INTEGER, PARAMETER :: ARRAY1_RANK= 1
  INTEGER, PARAMETER :: ARRAY1_DIM1= 4
  CHARACTER(LEN=10), PARAMETER :: FILENAME = "tarray1.h5"

  TYPE s1_t
     INTEGER :: i
     REAL :: f
  END TYPE s1_t
  TYPE(s1_t), DIMENSION(:,:), ALLOCATABLE, TARGET :: wdata ! /* Information to write */
  TYPE(s1_t), DIMENSION(:,:), ALLOCATABLE, TARGET :: rdata ! /* Information read in */
  INTEGER(hid_t)	:: fid1	!/* HDF5 File IDs		*/
  INTEGER(hid_t) :: dataset	!/* Dataset ID			*/
  INTEGER(hid_t)	:: sid1       !/* Dataspace ID			*/
  INTEGER(hid_t)	:: tid1       !/* Array Datatype ID			*/
  INTEGER(hid_t)	:: tid2       !/* Compound Datatype ID			*/

  INTEGER(HSIZE_T), DIMENSION(1) :: sdims1 = (/SPACE1_DIM1/)
  INTEGER(HSIZE_T), DIMENSION(1) :: tdims1=(/ARRAY1_DIM1/)
  INTEGER :: ndims ! /* Array rank for reading */
  INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims1 !/* Array dimensions for reading */
  INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims !/* Array dimensions for reading */
  INTEGER :: nmemb !/* Number of compound members */
  CHARACTER(LEN=20) :: mname !/* Name of compound field */
  INTEGER(size_t) :: off   !/* Offset of compound field */
  INTEGER(hid_t) :: mtid   !/* Datatype ID for field */
  INTEGER :: i,j      ! /* counting variables */ 
  INTEGER(SIZE_T) :: type_sizei  ! Size of the integer datatype 
  INTEGER(SIZE_T) :: type_sizer  ! Size of the real datatype

  INTEGER(SIZE_T) :: sizeof_compound ! total size of compound
  INTEGER :: error    ! /* Generic RETURN value */
  INTEGER(SIZE_T)     ::   offset     ! Member's offset
  INTEGER :: namelen
  LOGICAL :: flag

  TYPE(C_PTR) :: f_ptr ! Needed to pass the pointer, for g95 compiler to work

  ALLOCATE( wdata(1:SPACE1_DIM1,1:ARRAY1_DIM1) )
  ALLOCATE( rdata(1:SPACE1_DIM1,1:ARRAY1_DIM1) )

  !/* Initialize array data to write */
  DO i = 1, SPACE1_DIM1
     DO j = 1, ARRAY1_DIM1
        wdata(i,j)%i = i * 10 + j
        wdata(i,j)%f = i * 2.5 + j
     ENDDO
  ENDDO

  !/* Create file */
  CALL h5fcreate_f(FILENAME,H5F_ACC_TRUNC_F,fid1,error)
  CALL check("h5fcreate_f", error, total_error)    

  !/* Create dataspace for datasets */
  CALL h5screate_simple_f(SPACE1_RANK, sdims1, sid1, error)
  CALL check("h5screate_simple_f", error, total_error)

  CALL h5tcreate_f(H5T_COMPOUND_F, H5OFFSETOF(C_LOC(wdata(1,1)), C_LOC(wdata(2,1))), tid2, error)
  CALL check("h5tcreate_f", error, total_error)

  !/* Insert integer field */
  CALL h5tinsert_f(tid2, "i", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%i)), H5T_NATIVE_INTEGER, error)
  CALL check("h5tinsert_f", error, total_error)

  !/* Insert float field */

  CALL h5tinsert_f(tid2, "f", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%f)), H5T_NATIVE_REAL, error)
  CALL check("h5tinsert_f", error, total_error)

  ! /* Create an array datatype to refer to */
  CALL h5tarray_create_f(tid2, ARRAY1_RANK, tdims1, tid1, error)
  CALL check("h5tarray_create_f", error, total_error)

  !/* Close compound datatype */
  CALL h5tclose_f(tid2,error)
  CALL check("h5tclose_f", error, total_error)


  !/* Create a dataset */
  CALL h5dcreate_f(fid1,"Dataset1",tid1, sid1, dataset,error)
  CALL check("h5dcreate_f", error, total_error)

  !/* Write dataset to disk */

  ALLOCATE(rdims(1:2)) ! dummy not needed

  f_ptr = C_LOC(wdata(1,1))
  CALL h5dwrite_f(dataset, tid1, f_ptr, error )
  CALL check("h5dwrite_f", error, total_error)
  !/* Close Dataset */ 
  CALL h5dclose_f(dataset, error)
  CALL check("h5dclose_f", error, total_error)

  !/* Close datatype */
  CALL h5tclose_f(tid1,error)
  CALL check("h5tclose_f", error, total_error)

  !/* Close disk dataspace */
  CALL h5sclose_f(sid1,error)
  CALL check("h5sclose_f", error, total_error)

  !/* Close file */
  CALL h5fclose_f(fid1,error)
  CALL check("h5fclose_f", error, total_error)

  !/* Re-open file */
  CALL h5fopen_f (FILENAME, H5F_ACC_RDONLY_F, fid1, error)
  CALL check("h5fopen_f", error, total_error)

  !/* Open the dataset */ 
  CALL h5dopen_f(fid1, "Dataset1", dataset, error)
  CALL check("h5dopen_f", error, total_error)

  !/* Get the datatype */    
  CALL h5dget_type_f(dataset, tid1, error)
  CALL check("h5dget_type_f", error, total_error)

  !/* Check the array rank */
  CALL h5tget_array_ndims_f(tid1, ndims, error)
  CALL check("h5tget_array_ndims_f", error, total_error)
  CALL VERIFY("h5tget_array_ndims_f",ndims, ARRAY1_RANK, total_error)

  !/* Get the array dimensions */
  ALLOCATE(rdims1(1:ndims))
  CALL h5tget_array_dims_f(tid1, rdims1, error)
  CALL check("h5tget_array_dims_f", error, total_error)


  !/* Check the array dimensions */
  DO i = 1, ndims
     CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims1(i)), total_error)
  ENDDO

  !/* Get the compound datatype */
  CALL h5tget_super_f(tid1, tid2, error)
  CALL check("h5tget_super_f", error, total_error)

  !/* Check the number of members */
  CALL h5tget_nmembers_f(tid2, nmemb, error)
  CALL check("h5tget_nmembers_f", error, total_error)
  CALL VERIFY("h5tget_nmembers_f", nmemb, 2, total_error)

  !/* Check the 1st field's name */
  CALL H5Tget_member_name_f(tid2, 0, mname, namelen,error)
  CALL check("H5Tget_member_name_f", error, total_error)
  CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"i", total_error)

  ! /* Check the 1st field's offset */
  CALL H5Tget_member_offset_f(tid2, 0, off, error)
  CALL check("H5Tget_member_offset_f", error, total_error)
  CALL VERIFY("H5Tget_member_offset_f",INT(off),0, total_error)    

  !/* Check the 1st field's datatype */
  CALL H5Tget_member_type_f(tid2, 0, mtid, error)
  CALL check("H5Tget_member_type_f", error, total_error)

  CALL H5Tequal_f(mtid, H5T_NATIVE_INTEGER, flag, error)
  CALL check("H5Tequal_f", error, total_error) 
  CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

  CALL h5tclose_f(mtid,error)
  CALL check("h5tclose_f", error, total_error)

  !/* Check the 2nd field's name */
  CALL H5Tget_member_name_f(tid2, 1, mname, namelen,error)
  CALL check("H5Tget_member_name_f", error, total_error)
  CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"f", total_error)

  ! /* Check the 2nd field's offset */
  CALL H5Tget_member_offset_f(tid2, 1, off, error)
  CALL check("H5Tget_member_offset_f", error, total_error)
  CALL VERIFY("H5Tget_member_offset_f",INT(off),H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%f)), total_error)    

  !/* Check the 2nd field's datatype */
  CALL H5Tget_member_type_f(tid2, 1, mtid, error)
  CALL check("H5Tget_member_type_f", error, total_error)

  CALL H5Tequal_f(mtid, H5T_NATIVE_REAL, flag, error)
  CALL check("H5Tequal_f", error, total_error) 
  CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

  CALL h5tclose_f(mtid,error)
  CALL check("h5tclose_f", error, total_error)

  ! /* Close Compound Datatype */
  CALL h5tclose_f(tid2, error)
  CALL check("h5tclose_f", error, total_error)

  !/* Read dataset from disk */

  f_ptr = C_LOC(rdata(1,1))
  CALL H5Dread_f(dataset, tid1, f_ptr, error)
  CALL check("H5Dread_f", error, total_error)

  !/* Compare data read in */
  DO i = 1, SPACE1_DIM1
     DO j = 1, ARRAY1_DIM1
        IF(wdata(i,j)%i.NE.rdata(i,j)%i)THEN
           PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
           total_error = total_error + 1
        ENDIF
        IF(wdata(i,j)%f.NE.rdata(i,j)%f)THEN
           PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
           total_error = total_error + 1
        ENDIF
     ENDDO
  ENDDO

  !/* Close Datatype */
  CALL h5tclose_f(tid1,error)
  CALL check("h5tclose_f", error, total_error)

  !/* Close Dataset */
  CALL h5dclose_f(dataset, error)
  CALL check("h5dclose_f", error, total_error)

  !/* Close file */
  CALL h5fclose_f(fid1,error)
  CALL check("h5fclose_f", error, total_error)

END SUBROUTINE test_array_compound_atomic
!!$
!!$!/****************************************************************
!!$!**
!!$!**  test_array_compound_array(): Test basic array datatype code.
!!$!**      Tests 1-D array of compound datatypes (with array fields)
!!$!**
!!$!****************************************************************/
!!$
  SUBROUTINE test_array_compound_array(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    IMPLICIT NONE
    
    INTEGER, INTENT(INOUT) :: total_error

    ! /* 1-D array datatype */
    INTEGER, PARAMETER :: ARRAY1_RANK= 1
    INTEGER, PARAMETER :: ARRAY1_DIM1= 3
    INTEGER, PARAMETER :: ARRAY2_DIM1= 5

    INTEGER, PARAMETER :: SPACE1_RANK = 1
    INTEGER, PARAMETER :: SPACE1_DIM1 = 4
    CHARACTER(LEN=10), PARAMETER :: FILENAME = "tarray2.h5"

    TYPE st_t_struct ! /* Typedef for compound datatype */
       INTEGER :: i
       REAL, DIMENSION(1:ARRAY2_DIM1) :: f
       CHARACTER(LEN=2), DIMENSION(1:ARRAY2_DIM1) :: c
    END TYPE st_t_struct
    ! /* Information to write */
    TYPE(st_t_struct), DIMENSION(1:SPACE1_DIM1,1:ARRAY1_DIM1), TARGET :: wdata
    ! /* Information read in */
    TYPE(st_t_struct), DIMENSION(1:SPACE1_DIM1,1:ARRAY1_DIM1), TARGET :: rdata


    INTEGER(hid_t) :: fid1		! /* HDF5 File IDs		*/
    INTEGER(hid_t) :: dataset	! /* Dataset ID			*/
    integer(hid_t) :: sid1      ! /* Dataspace ID			*/
    integer(hid_t) :: tid1      ! /* Array Datatype ID	*/
    integer(hid_t) :: tid2      ! /* Compound Datatype ID	*/
    integer(hid_t) :: tid3      ! /* Nested Array Datatype ID	*/
    integer(hid_t) :: tid4      ! /* Nested Array Datatype ID	*/
    INTEGER(HID_T) :: dt5_id      ! Memory datatype identifier 

    INTEGER(HSIZE_T), DIMENSION(1) :: sdims1 = (/SPACE1_DIM1/)
    INTEGER(HSIZE_T), DIMENSION(1) :: tdims1=(/ARRAY1_DIM1/)
    INTEGER(HSIZE_T), DIMENSION(1) :: tdims2=(/ARRAY2_DIM1/)

    INTEGER  ndims      !/* Array rank for reading */

    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims1 !/* Array dimensions for reading */
    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims !/* Array dimensions for reading */

    INTEGER :: nmemb !/* Number of compound members */
    CHARACTER(LEN=20) :: mname !/* Name of compound field */
    INTEGER(size_t) :: off   !/* Offset of compound field */
    INTEGER(size_t) :: offset   !/* Offset of compound field */
    INTEGER(hid_t) :: mtid   !/* Datatype ID for field */ 
    INTEGER(hid_t) :: mtid2   !/* Datatype ID for field */ 
    INTEGER(SIZE_T) :: type_sizei  ! Size of the integer datatype 
    INTEGER(SIZE_T) :: type_sizer  ! Size of the real datatype 
    INTEGER(SIZE_T) :: type_sizec  ! Size of the character datatype
    INTEGER(SIZE_T) :: sizeof_compound ! total size of compound

    INTEGER :: mclass     ! /* Datatype class for field */
    INTEGER :: i,j,k      !/* counting variables */

    INTEGER :: error
    CHARACTER(LEN=2) :: ichr2
    INTEGER(SIZE_T) :: sizechar
    INTEGER :: namelen
    LOGICAL :: flag 
    INTEGER(HID_T) :: atype_id       !String Attribute Datatype identifier
    INTEGER(SIZE_T) :: attrlen    ! Length of the attribute string 

    TYPE(c_ptr) :: f_ptr

    ! /* Initialize array data to write */
    DO i = 1, SPACE1_DIM1
       DO j = 1, array1_DIM1
          wdata(i,j)%i = i*10+j
          DO k = 1, ARRAY2_DIM1
             wdata(i,j)%f(k) = 10*i+j+.5
             WRITE(ichr2,'(I2.2)') k
             wdata(i,j)%c(k) = ichr2
          ENDDO
       ENDDO
    ENDDO

    ! /* Create file */
    CALL h5fcreate_f(FILENAME,H5F_ACC_TRUNC_F,fid1,error)
    CALL check("h5fcreate_f", error, total_error)   


    ! /* Create dataspace for datasets */
    CALL h5screate_simple_f(SPACE1_RANK, sdims1, sid1, error)
    CALL check("h5screate_simple_f", error, total_error)

    ! /* Create a compound datatype to refer to */
    !
    CALL h5tcreate_f(H5T_COMPOUND_F,  H5OFFSETOF(C_LOC(wdata(1,1)), C_LOC(wdata(2,1))), tid2, error)
    CALL check("h5tcreate_f", error, total_error)

    !/* Insert integer field */
    CALL h5tinsert_f(tid2, "i", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%i)), H5T_NATIVE_INTEGER, error)
    CALL check("h5tinsert_f", error, total_error)

    !/* Create an array of floats datatype */
    CALL h5tarray_create_f(H5T_NATIVE_REAL, ARRAY1_RANK, tdims2, tid3, error)
    CALL check("h5tarray_create_f", error, total_error)
    !/* Insert float array field */

    CALL h5tinsert_f(tid2, "f", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%f)), tid3, error)
    CALL check("h5tinsert_f", error, total_error)

    !
    ! Create datatype for the String attribute.
    !
    CALL h5tcopy_f(H5T_NATIVE_CHARACTER, atype_id, error)
    CALL check("h5tcopy_f",error,total_error)
 
    attrlen = LEN(wdata(1,1)%c(1)) 
    CALL h5tset_size_f(atype_id, attrlen, error)
    CALL check("h5tset_size_f",error,total_error)    

    !/* Create an array of character datatype */
    CALL h5tarray_create_f(atype_id, ARRAY1_RANK, tdims2, tid4, error)
    CALL check("h5tarray_create_f", error, total_error)

    !/* Insert character array field */
    CALL h5tinsert_f(tid2, "c", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%c(1))), tid4, error)
    CALL check("h5tinsert2_f", error, total_error)

    ! /* Close array of floats field datatype */
    CALL h5tclose_f(tid3,error)
    CALL check("h5tclose_f", error, total_error)

    CALL h5tclose_f(tid4,error)
    CALL check("h5tclose_f", error, total_error)

    !/* Create an array datatype to refer to */
    CALL h5tarray_create_f(tid2, ARRAY1_RANK, tdims1, tid1, error)
    CALL check("h5tarray_create_f", error, total_error)

    !/* Close compound datatype */
    CALL h5tclose_f(tid2,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Create a dataset */
    CALL h5dcreate_f(fid1,"Dataset1",tid1, sid1, dataset,error)
    CALL check("h5dcreate_f", error, total_error)


    !/* Write dataset to disk */
    f_ptr = C_LOC(wdata(1,1))
    CALL h5dwrite_f(dataset, tid1, f_ptr, error )
    CALL check("h5dwrite_f", error, total_error)

    !/* Close Dataset */
    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    ! /* Close datatype */
    CALL h5tclose_f(tid1,error)
    CALL check("h5tclose_f", error, total_error)

    !/* Close disk dataspace */
    CALL h5sclose_f(sid1,error)
    CALL check("h5sclose_f", error, total_error)

    !/* Close file */
    CALL h5fclose_f(fid1,error)
    CALL check("h5fclose_f", error, total_error)

    ! /* Re-open file */
    CALL h5fopen_f (FILENAME, H5F_ACC_RDONLY_F, fid1, error)
    CALL check("h5fopen_f", error, total_error)

    !/* Open the dataset */

    CALL h5dopen_f(fid1, "Dataset1", dataset, error)
    CALL check("h5dopen_f", error, total_error)
    
    ! /* Get the datatype */
    CALL h5dget_type_f(dataset, tid1, error)
    CALL check("h5dget_type_f", error, total_error)

    ! /* Check the array rank */
    CALL h5tget_array_ndims_f(tid1, ndims, error)
    CALL check("h5tget_array_ndims_f", error, total_error)
    CALL VERIFY("h5tget_array_ndims_f",ndims, ARRAY1_RANK, total_error)


    !/* Get the array dimensions */
    ALLOCATE(rdims1(1:ndims))
    CALL h5tget_array_dims_f(tid1, rdims1, error)
    CALL check("h5tget_array_dims_f", error, total_error)

    ! /* Check the array dimensions */
    DO i = 1, ndims
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims1(i)), total_error)
    ENDDO

    ! /* Get the compound datatype */
    CALL h5tget_super_f(tid1, tid2, error)
    CALL check("h5tget_super_f", error, total_error)

    ! /* Check the number of members */
    CALL h5tget_nmembers_f(tid2, nmemb, error)
    CALL check("h5tget_nmembers_f", error, total_error)
    CALL VERIFY("h5tget_nmembers_f", nmemb, 3, total_error)

    ! /* Check the 1st field's name */
    CALL H5Tget_member_name_f(tid2, 0, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"i", total_error)

    ! /* Check the 1st field's offset */

    CALL H5Tget_member_offset_f(tid2, 0, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),0, total_error) 

    ! /* Check the 1st field's datatype */
    CALL H5Tget_member_type_f(tid2, 0, mtid, error)
    CALL check("H5Tget_member_type_f", error, total_error)

    CALL H5Tequal_f(mtid, H5T_NATIVE_INTEGER, flag, error)
    CALL check("H5Tequal_f", error, total_error) 
    CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

    CALL h5tclose_f(mtid,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Check the 2nd field's name */
    CALL H5Tget_member_name_f(tid2, 1, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"f", total_error)

    ! /* Check the 2nd field's offset */
    CALL H5Tget_member_offset_f(tid2, 1, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%f)), total_error) 

    ! /* Check the 2nd field's datatype */
    CALL H5Tget_member_type_f(tid2, 1, mtid, error)
    CALL check("H5Tget_member_type_f", error, total_error)

    ! /* Get the 2nd field's class */
    CALL H5Tget_class_f(mtid, mclass, error)
    CALL check("H5Tget_class_f", error, total_error)
    CALL VERIFY("H5Tget_class_f",mclass, H5T_ARRAY_F, total_error)

    ! /* Check the array rank */
    CALL h5tget_array_ndims_f(mtid, ndims, error)
    CALL check("h5tget_array_ndims_f", error, total_error)
    CALL VERIFY("h5tget_array_ndims_f",ndims, ARRAY1_RANK, total_error)

    ! /* Get the array dimensions */
    CALL h5tget_array_dims_f(mtid, rdims1, error)
    CALL check("h5tget_array_dims_f", error, total_error)

    ! /* Check the array dimensions */
    DO i = 1, ndims
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims2(i)), total_error)
    ENDDO

    ! /* Check the 3rd field's name */
    CALL H5Tget_member_name_f(tid2, 2, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"c", total_error)

    ! /* Check the 3rd field's offset */
    CALL H5Tget_member_offset_f(tid2, 2, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),&
         H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%c(1))), total_error) 

    ! /* Check the 3rd field's datatype */
    CALL H5Tget_member_type_f(tid2, 2, mtid2, error)
    CALL check("H5Tget_member_type_f", error, total_error)

    ! /* Get the 3rd field's class */
    CALL H5Tget_class_f(mtid2, mclass, error)
    CALL check("H5Tget_class_f", error, total_error)
    CALL VERIFY("H5Tget_class_f",mclass, H5T_ARRAY_F, total_error)

    ! /* Check the array rank */
    CALL h5tget_array_ndims_f(mtid2, ndims, error)
    CALL check("h5tget_array_ndims_f", error, total_error)
    CALL VERIFY("h5tget_array_ndims_f",ndims, ARRAY1_RANK, total_error)

    ! /* Get the array dimensions */
    CALL h5tget_array_dims_f(mtid2, rdims1, error)
    CALL check("h5tget_array_dims_f", error, total_error)

    ! /* Check the array dimensions */
    DO i = 1, ndims
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims2(i)), total_error)
    ENDDO

    ! /* Check the nested array's datatype */
    CALL H5Tget_super_f(mtid, tid3, error)
    CALL check("H5Tget_super_f", error, total_error)

    CALL H5Tequal_f(tid3, H5T_NATIVE_REAL, flag, error)
    CALL check("H5Tequal_f", error, total_error) 
    CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

    ! /* Check the nested array's datatype */
    CALL H5Tget_super_f(mtid2, tid3, error)
    CALL check("H5Tget_super_f", error, total_error)

    CALL H5Tequal_f(tid3, atype_id, flag, error)
    CALL check("H5Tequal_f", error, total_error) 
    CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

    ! /* Close the array's base type datatype */
    CALL h5tclose_f(tid3, error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close the member datatype */
    CALL h5tclose_f(mtid,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close the member datatype */
    CALL h5tclose_f(mtid2,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close Compound Datatype */
    CALL h5tclose_f(tid2,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* READ dataset from disk */
    
    f_ptr = c_null_ptr
    f_ptr = C_LOC(rdata(1,1))
    CALL H5Dread_f(dataset, tid1, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    ! /* Compare data read in */
    DO i = 1, SPACE1_DIM1
       DO j = 1, ARRAY1_DIM1
          IF(wdata(i,j)%i.NE.rdata(i,j)%i)THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          DO k = 1, ARRAY2_DIM1
             IF(wdata(i,j)%f(k).NE.rdata(i,j)%f(k))THEN
                PRINT*, 'ERROR: Wrong real array data is read back by H5Dread_f '
                total_error = total_error + 1
             ENDIF
             IF(wdata(i,j)%c(k).NE.rdata(i,j)%c(k))THEN
                PRINT*, 'ERROR: Wrong character array data is read back by H5Dread_f '
                total_error = total_error + 1
             ENDIF
          ENDDO
       ENDDO
    ENDDO

    ! /* Close Datatype */
    CALL h5tclose_f(tid1,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close Dataset */
    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    ! /* Close file */
    CALL h5fclose_f(fid1,error)
    CALL check("h5fclose_f", error, total_error)
  END SUBROUTINE test_array_compound_array
!!$
!!$!/****************************************************************
!!$!**
!!$!**  test_array_bkg(): Test basic array datatype code.
!!$!**      Tests reading compound datatype with array fields and
!!$!**          writing partial fields.
!!$!**
!!$!****************************************************************/
!!$
  SUBROUTINE test_array_bkg(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    IMPLICIT NONE

    INTEGER, PARAMETER :: r_k4 = SELECTED_REAL_KIND(5)
    INTEGER, PARAMETER :: r_k8 = SELECTED_REAL_KIND(10)

    INTEGER, INTENT(INOUT) :: total_error

    INTEGER, PARAMETER :: LENGTH = 5
    INTEGER, PARAMETER :: ALEN = 10
    INTEGER, PARAMETER :: RANK = 1
    INTEGER, PARAMETER :: NMAX = 100
    CHARACTER(LEN=17), PARAMETER :: FIELDNAME = "ArrayofStructures"

    INTEGER(hid_t) :: fid, array_dt
    INTEGER(hid_t) :: space
    INTEGER(hid_t) :: type
    INTEGER(hid_t) :: dataset

    INTEGER(hsize_t), DIMENSION(1:1) :: dim =(/LENGTH/)
    INTEGER(hsize_t), DIMENSION(1:1) :: dima =(/ALEN/)

    INTEGER :: i, j
    INTEGER, DIMENSION(1:3) :: ndims = (/1,1,1/)

    TYPE CmpField_struct
       INTEGER, DIMENSION(1:ALEN) :: a
       REAL(KIND=r_k4), DIMENSION(1:ALEN) :: b
       REAL(KIND=r_k8), DIMENSION(1:ALEN) :: c
    ENDTYPE CmpField_struct

    TYPE(CmpField_struct), DIMENSION(1:LENGTH), TARGET :: cf
    TYPE(CmpField_struct), DIMENSION(1:LENGTH), TARGET :: cfr
    
    TYPE CmpDTSinfo_struct
       INTEGER :: nsubfields
       CHARACTER(LEN=5), DIMENSION(1:nmax) :: name
       INTEGER(size_t), DIMENSION(1:nmax) :: offset
       INTEGER(hid_t), DIMENSION(1:nmax) :: datatype
    END TYPE CmpDTSinfo_struct

    TYPE(CmpDTSinfo_struct) :: dtsinfo

    TYPE fld_t_struct
       REAL(KIND=r_k4), DIMENSION(1:ALEN) :: b
    END TYPE fld_t_struct
 
    INTEGER(SIZE_T) :: type_sizei  ! Size of the integer datatype 
    INTEGER(SIZE_T) :: type_sizer  ! Size of the real datatype 
    INTEGER(SIZE_T) :: type_sized  ! Size of the double datatype
    INTEGER(SIZE_T) :: sizeof_compound ! total size of compound

    TYPE(fld_t_struct), DIMENSION(1:LENGTH), TARGET :: fld
    TYPE(fld_t_struct), DIMENSION(1:LENGTH), TARGET :: fldr

    CHARACTER(LEN=10), PARAMETER :: FILENAME = "tarray3.h5"

    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims1 !/* Array dimensions for reading */
    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims !/* Array dimensions for reading */

    INTEGER :: error
    TYPE(c_ptr) :: f_ptr
    
    TYPE(c_funptr) :: func

!    /* Initialize the data */
!    /* ------------------- */

    DO i = 1, LENGTH
       DO j = 1, ALEN
          cf(i)%a(j) = 100*(i+1) + j
          cf(i)%b(j) = (100.*(i+1) + 0.01*j)
          cf(i)%c(j) = 100.*(i+1) + 0.02*j
       ENDDO
    ENDDO

    !/* Set the number of data members */
    !/* ------------------------------ */

    dtsinfo%nsubfields = 3

    !/* Initialize the offsets  */
    !/* ----------------------- */
    CALL h5tget_size_f(H5T_NATIVE_INTEGER, type_sizei, error)
    CALL check("h5tget_size_f", error, total_error)
    IF(sizeof(cf(1)%b(1)).EQ.4)THEN
       CALL h5tget_size_f(H5T_NATIVE_REAL_4, type_sizer, error)
       CALL check("h5tget_size_f", error, total_error)
    ELSE IF(sizeof(cf(1)%b(1)).EQ.8)THEN
       CALL h5tget_size_f(H5T_NATIVE_REAL_8, type_sizer, error)
       CALL check("h5tget_size_f", error, total_error)
    ENDIF

    CALL h5tget_size_f(H5T_NATIVE_DOUBLE, type_sized, error)
    CALL check("h5tget_size_f", error, total_error)

    dtsinfo%offset(1)   = H5OFFSETOF(C_LOC(cf(1)),C_LOC(cf(1)%a(1)))
    dtsinfo%offset(2)   = H5OFFSETOF(C_LOC(cf(1)),C_LOC(cf(1)%b(1))) 
    dtsinfo%offset(3)   = H5OFFSETOF(C_LOC(cf(1)),C_LOC(cf(1)%c(1)))


    !/* Initialize the data type IDs */
    !/* ---------------------------- */
    dtsinfo%datatype(1) = H5T_NATIVE_INTEGER;
    dtsinfo%datatype(2) = H5T_NATIVE_REAL_4;
    dtsinfo%datatype(3) = H5T_NATIVE_REAL_8;


    !/* Initialize the names of data members */
    !/* ------------------------------------ */
     
    dtsinfo%name(1) = "One  "
    dtsinfo%name(2) = "Two  "
    dtsinfo%name(3) = "Three"
       
    !/* Create file */
    !/* ----------- */
    CALL h5fcreate_f(FILENAME,H5F_ACC_TRUNC_F,fid,error)
    CALL check("h5fcreate_f", error, total_error)   


    !/* Create data space */
    !/* ----------------- */
    CALL h5screate_simple_f(RANK, dim, space, error)
    CALL check("h5screate_simple_f", error, total_error)


    !/* Create the memory data type */
    !/* --------------------------- */

    CALL h5tcreate_f(H5T_COMPOUND_F, H5OFFSETOF(C_LOC(cf(1)), C_LOC(cf(2))), type, error)
    CALL check("h5tcreate_f", error, total_error)

    !/* Add  members to the compound data type */
    !/* -------------------------------------- */

    DO i = 1, dtsinfo%nsubfields
       CALL h5tarray_create_f(dtsinfo%datatype(i), ndims(i), dima, array_dt, error)
       CALL check("h5tarray_create_f", error, total_error)
       CALL H5Tinsert_f(type, dtsinfo%name(i), dtsinfo%offset(i), array_dt, error)
       CALL check("h5tinsert_f", error, total_error)

       CALL h5tclose_f(array_dt,error)
       CALL check("h5tclose_f", error, total_error)
    ENDDO

    !/* Create the dataset */
    !/* ------------------ *//
    CALL h5dcreate_f(fid,FIELDNAME,type, space, dataset,error)
    CALL check("h5dcreate_f", error, total_error)

    !/* Write data to the dataset */
    !/* ------------------------- */

    ALLOCATE(rdims(1:2)) ! dummy not needed

    f_ptr = C_LOC(cf(1))

    CALL h5dwrite_f(dataset, type, f_ptr, error )
    CALL check("h5dwrite_f", error, total_error)


    ALLOCATE(rdims1(1:2)) ! dummy not needed
    f_ptr = C_LOC(cfr(1))
    CALL H5Dread_f(dataset, type, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    !/* Verify correct data */
    !/* ------------------- */
    DO i = 1, LENGTH
       DO j = 1, ALEN
           IF( cf(i)%a(j) .NE. cfr(i)%a(j) )THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%b(j) .NE. cfr(i)%b(j) )THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%c(j) .NE. cfr(i)%c(j) )THEN
             PRINT*, 'ERROR: Wrong double data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO


    !/* Release IDs */
    !/* ----------- */
    CALL h5tclose_f(type,error)
    CALL check("h5tclose_f", error, total_error)
    CALL h5sclose_f(space,error)
    CALL check("h5sclose_f", error, total_error)
    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)
    CALL h5fclose_f(fid,error)
    CALL check("h5fclose_f", error, total_error)

    !/******************************/
    !/* Reopen the file and update */
    !/******************************/

    CALL h5fopen_f (FILENAME, H5F_ACC_RDWR_F, fid, error)
    CALL check("h5fopen_f", error, total_error)

    CALL h5dopen_f(fid, FIELDNAME, dataset, error)
    CALL check("h5dopen_f", error, total_error)

    sizeof_compound =  INT( type_sizer*ALEN, size_t)

    CALL h5tcreate_f(H5T_COMPOUND_F, sizeof_compound , type, error)
    CALL check("h5tcreate_f", error, total_error)

    CALL h5tarray_create_f(H5T_NATIVE_REAL_4, 1, dima, array_dt, error)
    CALL check("h5tarray_create_f", error, total_error)

    CALL h5tinsert_f(TYPE, "Two", 0_size_t, array_dt, error)
    CALL check("h5tinsert_f", error, total_error)

    !/* Initialize the data to overwrite */
    !/* -------------------------------- */
    DO i = 1, LENGTH
       DO j = 1, ALEN
          fld(i)%b(j) = 1.313
          cf(i)%b(j) = fld(i)%b(j)
       ENDDO
    ENDDO

    f_ptr = C_LOC(fld(1))

    CALL h5dwrite_f(dataset, TYPE, f_ptr, error )
    CALL check("h5dwrite_f", error, total_error)


    ! /* Read just the field changed */
    
    f_ptr = C_LOC(fldr(1))
    CALL H5Dread_f(dataset, TYPE, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    DO i = 1, LENGTH
       DO j = 1, ALEN
          IF( fld(i)%b(j) .NE. fldr(i)%b(j) )THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO
    CALL h5tclose_f(TYPE,error)
    CALL check("h5tclose_f", error, total_error)
    CALL h5tclose_f(array_dt,error)
    CALL check("h5tclose_f", error, total_error)

    CALL h5dget_type_f(dataset, type, error)
    CALL check("h5dget_type_f", error, total_error)


    ! /* Read the entire dataset again */

    f_ptr = C_LOC(cfr(1))
    CALL H5Dread_f(dataset, TYPE, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)


    !/* Verify correct data */
    !/* ------------------- */

    DO i = 1, LENGTH
       DO j = 1, ALEN
           IF( cf(i)%a(j) .NE. cfr(i)%a(j) )THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%b(j) .NE. cfr(i)%b(j) )THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%c(j) .NE. cfr(i)%c(j) )THEN
             PRINT*, 'ERROR: Wrong double data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO

    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    CALL h5tclose_f(type,error)
    CALL check("h5tclose_f", error, total_error)

    CALL h5fclose_f(fid,error)
    CALL check("h5fclose_f", error, total_error)

!/****************************************************/
!/* Reopen the file and print out all the data again */
!/****************************************************/

    CALL h5fopen_f (FILENAME, H5F_ACC_RDWR_F, fid, error)
    CALL check("h5fopen_f", error, total_error)


    CALL h5dopen_f(fid, FIELDNAME, dataset, error)
    CALL check("h5dopen_f", error, total_error)


    CALL h5dget_type_f(dataset, type, error)
    CALL check("h5dget_type_f", error, total_error)


    !/* Reset the data to read in */
    !/* ------------------------- */

    DO i = 1, LENGTH
       cfr(i)%a(:) = 0
       cfr(i)%b(:) = 0
       cfr(i)%c(:) = 0
    ENDDO

    f_ptr = C_LOC(cfr(1))
    CALL H5Dread_f(dataset, TYPE, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    !/* Verify correct data */
    !/* ------------------- */

    DO i = 1, LENGTH
       DO j = 1, ALEN
           IF( cf(i)%a(j) .NE. cfr(i)%a(j) )THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%b(j) .NE. cfr(i)%b(j) )THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%c(j) .NE. cfr(i)%c(j) )THEN
             PRINT*, 'ERROR: Wrong double data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO

    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    CALL h5tclose_f(type,error)
    CALL check("h5tclose_f", error, total_error)

    CALL h5fclose_f(fid,error)
    CALL check("h5fclose_f", error, total_error)

  END SUBROUTINE test_array_bkg



  SUBROUTINE test_h5kind_to_type(total_error)

    USE ISO_C_BINDING
    USE HDF5 ! This module contains all necessary modules

    IMPLICIT NONE
    
    INTEGER, INTENT(INOUT) :: total_error
    
    INTEGER, PARAMETER :: int_kind_1 = SELECTED_INT_KIND(Fortran_INTEGER_1)  !should map to INTEGER*1 on most modern processors
    INTEGER, PARAMETER :: int_kind_4 = SELECTED_INT_KIND(Fortran_INTEGER_2)  !should map to INTEGER*2 on most modern processors
    INTEGER, PARAMETER :: int_kind_8 = SELECTED_INT_KIND(Fortran_INTEGER_4)  !should map to INTEGER*4 on most modern processors
    INTEGER, PARAMETER :: int_kind_16 = SELECTED_INT_KIND(Fortran_INTEGER_8) !should map to INTEGER*8 on most modern processors
    
    INTEGER, PARAMETER :: real_kind_7 = SELECTED_REAL_KIND(Fortran_REAL_4) !should map to REAL*4 on most modern processors
    INTEGER, PARAMETER :: real_kind_15 = SELECTED_REAL_KIND(Fortran_REAL_8) !should map to REAL*8 on most modern processors
    
    CHARACTER(LEN=8), PARAMETER :: filename = "dsetf.h5" ! File name
    CHARACTER(LEN=5), PARAMETER :: dsetname1 = "dset1"     ! Dataset name
    CHARACTER(LEN=5), PARAMETER :: dsetname2 = "dset2"     ! Dataset name
    CHARACTER(LEN=5), PARAMETER :: dsetname4 = "dset4"     ! Dataset name
    CHARACTER(LEN=5), PARAMETER :: dsetname8 = "dset8"     ! Dataset name
    CHARACTER(LEN=6), PARAMETER :: dsetnamer = "dsetr"     ! Dataset name
    CHARACTER(LEN=6), PARAMETER :: dsetnamer4 = "dsetr4"     ! Dataset name
    CHARACTER(LEN=6), PARAMETER :: dsetnamer8 = "dsetr8"     ! Dataset name
    
    INTEGER(HID_T) :: file_id       ! File identifier 
    INTEGER(HID_T) :: dset_id1      ! Dataset identifier  
    INTEGER(HID_T) :: dset_id4      ! Dataset identifier   
    INTEGER(HID_T) :: dset_id8      ! Dataset identifier  
    INTEGER(HID_T) :: dset_id16     ! Dataset identifier     
    INTEGER(HID_T) :: dset_idr       ! Dataset identifier 
    INTEGER(HID_T) :: dset_idr4      ! Dataset identifier   
    INTEGER(HID_T) :: dset_idr8      ! Dataset identifier 
    
    INTEGER :: error ! Error flag
    INTEGER :: i, j
    
! Data buffers:

    INTEGER, DIMENSION(1:4) :: dset_data

    INTEGER(int_kind_1), DIMENSION(1:4), TARGET :: dset_data_i1, data_out_i1
    INTEGER(int_kind_4), DIMENSION(1:4), TARGET :: dset_data_i4, data_out_i4
    INTEGER(int_kind_8), DIMENSION(1:4), TARGET :: dset_data_i8, data_out_i8
    INTEGER(int_kind_16), DIMENSION(1:4), TARGET :: dset_data_i16, data_out_i16

    REAL, DIMENSION(1:4), TARGET :: dset_data_r, data_out_r
    REAL(real_kind_7), DIMENSION(1:4), TARGET :: dset_data_r7, data_out_r7
    REAL(real_kind_15), DIMENSION(1:4), TARGET :: dset_data_r15, data_out_r15
    
    INTEGER(HSIZE_T), DIMENSION(1:1) :: data_dims = (/4/) 
    INTEGER(HID_T) :: dspace_id     ! Dataspace identifier
    
    TYPE(C_PTR) :: f_ptr
    INTEGER(hid_t) :: datatype         !/* Common datatype ID */

    !
    ! Initialize the dset_data array.
    !
    DO i = 1, 4
       dset_data_i1(i)  = i
       dset_data_i4(i)  = i
       dset_data_i8(i)  = i
       dset_data_i16(i) = i

       dset_data_r(i) = (i)*100.
       dset_data_r7(i) = (i)*100.
       dset_data_r15(i) = (i)*1000.
       
    END DO

    CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file_id, error)
    CALL check("h5fcreate_f",error, total_error)
  !
  ! Create dataspaces for datasets
  !
    CALL h5screate_simple_f(1, data_dims , dspace_id, error)
    CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset.
  !
    CALL H5Dcreate_f(file_id, dsetname1, h5kind_to_type(int_kind_1,H5_INTEGER_KIND),  dspace_id, dset_id1, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetname2, h5kind_to_type(int_kind_4,H5_INTEGER_KIND),  dspace_id, dset_id4, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetname4, h5kind_to_type(int_kind_8,H5_INTEGER_KIND),  dspace_id, dset_id8, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetname8, h5kind_to_type(int_kind_16,H5_INTEGER_KIND), dspace_id, dset_id16, error)
    CALL check("H5Dcreate_f",error, total_error)
    
    CALL H5Dcreate_f(file_id, dsetnamer, H5T_NATIVE_REAL, dspace_id, dset_idr, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetnamer4, h5kind_to_type(real_kind_7,H5_REAL_KIND),  dspace_id, dset_idr4, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetnamer8, h5kind_to_type(real_kind_15,H5_REAL_KIND), dspace_id, dset_idr8, error)
    CALL check("H5Dcreate_f",error, total_error)

  !
  ! Write the dataset.
  !
    f_ptr = C_LOC(dset_data_i1(1))
    CALL h5dwrite_f(dset_id1, h5kind_to_type(int_kind_1,H5_INTEGER_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_i4(1))
    CALL h5dwrite_f(dset_id4, h5kind_to_type(int_kind_4,H5_INTEGER_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_i8(1))
    CALL h5dwrite_f(dset_id8, h5kind_to_type(int_kind_8,H5_INTEGER_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_i16(1))
    CALL h5dwrite_f(dset_id16, h5kind_to_type(int_kind_16,H5_INTEGER_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_r(1))
    CALL h5dwrite_f(dset_idr, H5T_NATIVE_REAL, f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_r7(1))
    CALL h5dwrite_f(dset_idr4, h5kind_to_type(real_kind_7,H5_REAL_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_r15(1))
    CALL h5dwrite_f(dset_idr8, h5kind_to_type(real_kind_15,H5_REAL_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
  !
  ! Close the file
  !
    CALL h5fclose_f(file_id, error)
    CALL check("h5fclose_f",error, total_error)

  ! Open the file

    CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file_id, error)
    CALL check("h5fopen_f",error, total_error)
  !
  ! Read the dataset.
  !
  ! Read data back into an integer size that is larger then the original size used for 
  ! writing the data
    f_ptr = C_LOC(data_out_i1)
    CALL h5dread_f(dset_id1, h5kind_to_type(int_kind_1,H5_INTEGER_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_i4)
    CALL h5dread_f(dset_id4, h5kind_to_type(int_kind_4,H5_INTEGER_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_i8)
    CALL h5dread_f(dset_id8, h5kind_to_type(int_kind_8,H5_INTEGER_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_i16)
    CALL h5dread_f(dset_id16, h5kind_to_type(int_kind_16,H5_INTEGER_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_r)
    CALL h5dread_f(dset_idr, H5T_NATIVE_REAL, f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_r7)
    CALL h5dread_f(dset_idr4, h5kind_to_type(real_kind_7,H5_REAL_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_r15)
    CALL h5dread_f(dset_idr8, h5kind_to_type(real_kind_15,H5_REAL_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    
    DO i = 1, 4
       
       CALL verify_Fortran_INTEGER_4("h5kind_to_type1",INT(dset_data_i1(i),int_kind_8),INT(data_out_i1(i),int_kind_8),total_error)
       CALL verify_Fortran_INTEGER_4("h5kind_to_type2",INT(dset_data_i4(i),int_kind_8),INT(data_out_i4(i),int_kind_8),total_error)
       CALL verify_Fortran_INTEGER_4("h5kind_to_type3",INT(dset_data_i8(i),int_kind_8),INT(data_out_i8(i),int_kind_8),total_error)
       CALL verify_Fortran_INTEGER_4("h5kind_to_type4",INT(dset_data_i16(i),int_kind_8),INT(data_out_i16(i),int_kind_8),total_error)

       CALL verify_real_kind_7("h5kind_to_type5",REAL(dset_data_r(i),real_kind_7),REAL(data_out_r(i),real_kind_7),total_error)
       CALL verify_real_kind_7("h5kind_to_type6",REAL(dset_data_r7(i),real_kind_7),REAL(data_out_r7(i),real_kind_7),total_error)
       CALL verify_real_kind_7("h5kind_to_type7",REAL(dset_data_r15(i),real_kind_7),REAL(data_out_r15(i),real_kind_7),total_error)

    END DO

  !
  ! Close the dataset.
  !
    CALL h5dclose_f(dset_id1, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_id4, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_id8, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_id16, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_idr4, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_idr8, error)
    CALL check("h5dclose_f",error, total_error)
  !
  ! Close the file.
  !
    CALL h5fclose_f(file_id, error)
    CALL check("h5fclose_f",error, total_error)

END SUBROUTINE test_h5kind_to_type

!************************************************************
!
!  This test reads and writes array datatypes
!  to a dataset.  The test first writes integers arrays of
!  dimension ADIM0xADIM1 to a dataset with a dataspace of
!  DIM0, then closes the  file.  Next, it reopens the file,
!  reads back the data.
!
!************************************************************
SUBROUTINE t_array(total_error)

  USE HDF5
  USE ISO_C_BINDING
  
  IMPLICIT NONE
    
  INTEGER, INTENT(INOUT) :: total_error

  CHARACTER(LEN=19), PARAMETER :: filename  = "t_array_F03.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"
  INTEGER          , PARAMETER :: dim0      = 4
  INTEGER          , PARAMETER :: adim0     = 3
  INTEGER          , PARAMETER :: adim1     = 5
  INTEGER(HID_T)  :: file, filetype, memtype, space, dset ! Handles
  INTEGER :: hdferr
  INTEGER(HSIZE_T), DIMENSION(1:1)   :: dims = (/dim0/)
  INTEGER(HSIZE_T), DIMENSION(1:2)   :: adims = (/adim0, adim1/)
  INTEGER(HSIZE_T), DIMENSION(1:3)   :: bdims = (/dim0, adim0, adim1/)
  INTEGER(HSIZE_T), DIMENSION(1:2)   :: maxdims
  INTEGER, DIMENSION(1:dim0, 1:adim0, 1:adim1), TARGET :: wdata ! Write buffer 
  INTEGER, DIMENSION(:,:,:), ALLOCATABLE, TARGET :: rdata    ! Read buffer
  INTEGER :: i, j, k
  TYPE(C_PTR) :: f_ptr
  INTEGER :: error ! Error flag

  !
  ! Initialize data.  i is the element in the dataspace, j and k the
  ! elements within the array datatype.
  !
  DO i = 1, dim0
     DO j = 1, adim0
        DO k = 1, adim1
           wdata(i,j,k) = (i-1)*(j-1)-(j-1)*(k-1)+(i-1)*(k-1)
        ENDDO
     ENDDO
  ENDDO
  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, error)
  !
  ! Create array datatypes for file and memory.
  !
  CALL H5Tarray_create_f(INT(H5T_STD_I64LE, HID_T), 2, adims, filetype, error)
  CALL check("H5Tarray_create_f",error, total_error)
  CALL H5Tarray_create_f(H5T_NATIVE_INTEGER, 2, adims, memtype, error)
  CALL check("H5Tarray_create_f",error, total_error)
  !
  ! Create dataspace.  Setting maximum size to be the current size.
  !
  CALL h5screate_simple_f(1, dims, space, error)
  CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset and write the array data to it.
  !
  CALL h5dcreate_f(file, dataset, filetype, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)
  f_ptr = C_LOC(wdata)
  CALL h5dwrite_f(dset, memtype, f_ptr, error)
  CALL check("h5dwrite_f",error, total_error)
  !
  ! Close and release resources.
  !
  CALL H5Dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL H5Sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(filetype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL H5Tclose_f(memtype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL H5Fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)
  !
  ! Now we begin the read section of this example. 
  !
  ! Open file, dataset, and attribute.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  !
  ! Get the datatype and its dimensions.
  !
  CALL h5dget_type_f(dset, filetype, error)
  CALL check("h5dget_type_f",error, error)
  CALL H5Tget_array_dims_f(filetype, adims, error)
  CALL check("h5dget_type_f",error, total_error)
  CALL VERIFY("H5Tget_array_dims_f", adims(1), adim0, total_error)
  CALL VERIFY("H5Tget_array_dims_f", adims(2), adim1, total_error)
  !
  ! Get dataspace and allocate memory for read buffer.  This is a
  ! three dimensional attribute when the array datatype is included.
  !
  CALL H5Dget_space_f(dset, space, error)
  CALL check("H5Dget_space_f",error, error)
  CALL H5Sget_simple_extent_dims_f(space, dims, maxdims, error)
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)

  ALLOCATE(rdata(1:dims(1),1:adims(1),1:adims(2)))
  !
  ! Create the memory datatype.
  ! 
  CALL H5Tarray_create_f(H5T_NATIVE_INTEGER, 2, adims, memtype, error)
  CALL check("H5Tarray_create_f",error, total_error)
  !
  ! Read the data.
  !

  f_ptr = C_LOC(rdata)
  CALL H5Dread_f(dset, memtype, f_ptr, error)
  CALL check("H5Dread_f",error, total_error)
  !
  ! Output the data to the screen.
  !
  i_loop: DO i = 1, dims(1)
             DO j=1, adim0
                DO k = 1, adim1
                   CALL VERIFY("H5Sget_simple_extent_dims_f",  rdata(i,j,k), wdata(i,j,k), total_error)
                   IF(total_error.NE.0) EXIT i_loop
                ENDDO
             ENDDO
          ENDDO i_loop
  !
  ! Close and release resources.
  !
  DEALLOCATE(rdata)
  CALL H5Dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL H5Sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(filetype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL H5Tclose_f(memtype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL H5Fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)

END SUBROUTINE t_array

SUBROUTINE t_enum(total_error)

  USE HDF5
  USE ISO_C_BINDING

  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error

  CHARACTER(LEN=19), PARAMETER :: filename  = "t_enum_F03.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"
  INTEGER          , PARAMETER :: dim0      = 4
  INTEGER          , PARAMETER :: dim1      = 7
  INTEGER(HID_T)               :: F_BASET  ! File base type
  INTEGER(HID_T)               :: M_BASET  ! Memory base type
  INTEGER(SIZE_T)  , PARAMETER :: NAME_BUF_SIZE = 16

! Enumerated type
  INTEGER, PARAMETER :: SOLID=0, LIQUID=1, GAS=2, PLASMA=3

  INTEGER(HID_T) :: file, filetype, memtype, space, dset ! Handles

  INTEGER(hsize_t),   DIMENSION(1:2) :: dims = (/dim0, dim1/)
  INTEGER, DIMENSION(1:dim0, 1:dim1), TARGET :: wdata ! Write buffer
  INTEGER, DIMENSION(:,:), ALLOCATABLE, TARGET :: rdata ! Read buffer
  INTEGER, DIMENSION(1:1), TARGET :: val

  CHARACTER(LEN=6), DIMENSION(1:4) :: &
       names = (/"SOLID ", "LIQUID", "GAS   ", "PLASMA"/)
  CHARACTER(LEN=NAME_BUF_SIZE) :: name
  INTEGER(HSIZE_T), DIMENSION(1:1) :: maxdims
  INTEGER :: i, j, idx
  TYPE(C_PTR) :: f_ptr
  INTEGER :: error ! Error flag
  !
  ! Initialize DATA.
  !
  F_BASET   = H5T_STD_I16BE      ! File base type
  M_BASET   = H5T_NATIVE_INTEGER ! Memory base type
  DO i = 1, dim0
     DO j = 1, dim1 
        wdata(i,j) = MOD( (j-1)*(i-1), PLASMA+1)
     ENDDO
  ENDDO
  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, total_error)
  !
  ! Create the enumerated datatypes for file and memory.  This
  ! process is simplified IF native types are used for the file,
  ! as only one type must be defined.
  !
  CALL h5tenum_create_f(F_BASET, filetype, error)
  CALL check("h5tenum_create_f",error, total_error)
  
  CALL h5tenum_create_f(M_BASET, memtype, error)
  CALL check("h5tenum_create_f",error, total_error)

  DO i = SOLID, PLASMA
     !
     ! Insert enumerated value for memtype.
     !
     val(1) = i
     CALL H5Tenum_insert_f(memtype, TRIM(names(i+1)), val(1), error)
     CALL check("H5Tenum_insert_f", error, total_error)
     !
     ! Insert enumerated value for filetype.  We must first convert
     ! the numerical value val to the base type of the destination.
     !
     f_ptr = C_LOC(val(1))
     CALL H5Tconvert_f(M_BASET, F_BASET, INT(1,SIZE_T), f_ptr, error)
     CALL check("H5Tconvert_f",error, total_error)
     CALL H5Tenum_insert_f(filetype, TRIM(names(i+1)), val(1), error)
     CALL check("H5Tenum_insert_f",error, total_error)
  ENDDO
  !
  ! Create dataspace.  Setting maximum size to be the current size.
  !
  CALL h5screate_simple_f(2, dims, space, total_error)
  CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset and write the enumerated data to it.
  ! 
  CALL h5dcreate_f(file, dataset, filetype, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)
  f_ptr = C_LOC(wdata(1,1))
  CALL h5dwrite_f(dset, memtype, f_ptr, error)
  CALL check("h5dwrite_f",error, total_error)
  !
  ! Close and release resources.
  !
  CALL h5dclose_f(dset , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL h5tclose_f(filetype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)

  !
  ! Now we begin the read section of this example.
  !
  ! Open file and dataset.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f (file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  !
  ! Get dataspace and allocate memory for read buffer.
  !
  CALL h5dget_space_f(dset,space, error)
  CALL check("H5Dget_space_f",error, total_error)
  CALL h5sget_simple_extent_dims_f (space, dims, maxdims, error)
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(2), dim1, total_error)

  ALLOCATE(rdata(1:dims(1),1:dims(2)))
  !
  ! Read the data.
  !
  f_ptr = C_LOC(rdata(1,1))
  CALL h5dread_f(dset, memtype, f_ptr, error)
  CALL check("H5Dread_f",error, total_error)
  !
  ! Output the data to the screen.
  !
  i_loop: DO i = 1, dims(1)
             DO j = 1, dims(2)
                !
                ! Get the name of the enumeration member.
                !
                CALL h5tenum_nameof_f( memtype, rdata(i,j), NAME_BUF_SIZE, name, error)
                CALL check("h5tenum_nameof_f",error, total_error)
                idx = MOD( (j-1)*(i-1), PLASMA+1 ) + 1
                CALL verifystring("h5tenum_nameof_f",TRIM(name),TRIM(names(idx)), total_error)
                IF(total_error.NE.0) EXIT i_loop
             ENDDO
          ENDDO i_loop
  !
  ! Close and release resources.
  !
  DEALLOCATE(rdata)
  CALL h5dclose_f(dset , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL h5tclose_f(memtype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)
  
END SUBROUTINE t_enum

SUBROUTINE t_bit(total_error)

  USE HDF5
  USE ISO_C_BINDING

  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error

  CHARACTER(LEN=20), PARAMETER :: filename  = "t_bit_F03.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"
  INTEGER          , PARAMETER :: dim0      = 4
  INTEGER          , PARAMETER :: dim1      = 7

  INTEGER(HID_T)  :: file, space, dset ! Handles
  INTEGER(HSIZE_T), DIMENSION(1:2)   :: dims = (/dim0, dim1/)
  INTEGER(HSIZE_T), DIMENSION(1:2)   :: maxdims
  INTEGER(C_SIGNED_CHAR), DIMENSION(1:dim0, 1:dim1), TARGET :: wdata              ! Write buffer 
  INTEGER(C_SIGNED_CHAR), DIMENSION(:,:), ALLOCATABLE, TARGET :: rdata    ! Read buffer
  INTEGER :: A, B, C, D
  INTEGER :: Aw, Bw, Cw, Dw
  INTEGER :: i, j
  INTEGER, PARAMETER :: hex =  Z'00000003'
  TYPE(C_PTR) :: f_ptr
  INTEGER :: error     ! Error flag
  !
  ! Initialize data.  We will manually pack 4 2-bit integers into
  ! each unsigned char data element.
  !
  DO i = 0, dim0-1
     DO j = 0, dim1-1
        wdata(i+1,j+1) = 0
        wdata(i+1,j+1) = IOR( wdata(i+1,j+1), IAND(i * j - j, hex))   ! Field "A"
        wdata(i+1,j+1) = IOR( wdata(i+1,j+1), ISHFT(IAND(i,hex),2))   ! Field "B"
        wdata(i+1,j+1) = IOR( wdata(i+1,j+1), ISHFT(IAND(j,hex),4))   ! Field "C"
        wdata(i+1,j+1) = IOR( wdata(i+1,j+1), ISHFT(IAND(i+j,hex),6)) ! Field "D"
     ENDDO
  ENDDO
  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, total_error)
  !
  ! Create dataspace.  Setting maximum size to be the current size.
  !
  CALL h5screate_simple_f(2, dims, space, error)
  CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset and write the bitfield data to it.
  !
  CALL H5Dcreate_f(file, dataset, H5T_STD_B8BE, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)
  f_ptr = C_LOC(wdata(1,1))
  CALL H5Dwrite_f(dset, H5T_NATIVE_B8, f_ptr, error)
  CALL check("h5dwrite_f",error, total_error)
  !
  ! Close and release resources.
  !
  CALL H5Dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL H5Sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)
  !
  ! Now we begin the read section of this example. 
  !
  ! Open file, dataset.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  !
  ! Get dataspace and allocate memory for read buffer.
  !
  CALL H5Dget_space_f(dset, space, error)
  CALL check("H5Dget_space_f",error, total_error)
  CALL H5Sget_simple_extent_dims_f(space, dims, maxdims, error)
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(2), dim1, total_error)
  ALLOCATE(rdata(1:dims(1),1:dims(2)))
  !
  ! Read the data.
  !
  f_ptr = C_LOC(rdata)
  CALL H5Dread_f(dset,  H5T_NATIVE_B8, f_ptr, error)
  CALL check("H5Dread_f",error, total_error)
  !
  ! Output the data to the screen.
  !
  i_loop: DO i = 1, dims(1)
            DO j = 1, dims(2)
               A = IAND(rdata(i,j), hex) ! Retrieve field "A"
               B = IAND(ISHFT(rdata(i,j),-2), hex) ! Retrieve field "B"
               C = IAND(ISHFT(rdata(i,j),-4), hex) ! Retrieve field "C"
               D = IAND(ISHFT(rdata(i,j),-6), hex) ! Retrieve field "D"
               
               Aw = IAND(wdata(i,j), hex) 
               Bw = IAND(ISHFT(wdata(i,j),-2), hex)
               Cw = IAND(ISHFT(wdata(i,j),-4), hex)
               Dw = IAND(ISHFT(wdata(i,j),-6), hex)

               CALL VERIFY("bitfield", A, Aw, total_error)
               CALL VERIFY("bitfield", B, Bw, total_error)
               CALL VERIFY("bitfield", C, Cw, total_error)
               CALL VERIFY("bitfield", D, Dw, total_error)
               IF(total_error.NE.0) EXIT i_loop
            ENDDO
         ENDDO i_loop
  !
  ! Close and release resources.
  !
  DEALLOCATE(rdata)
  CALL H5Dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL H5Sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)

END SUBROUTINE t_bit

SUBROUTINE t_opaque(total_error)

  USE HDF5
  USE ISO_C_BINDING

  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error
  CHARACTER(LEN=20), PARAMETER :: filename  = "t_opaque_F03.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"
  INTEGER          , PARAMETER :: dim0      = 4
  INTEGER(SIZE_T)  , PARAMETER :: size      = 7
  INTEGER(HID_T)  :: file, space, dtype, dset ! Handles
  INTEGER(size_t) :: len
  INTEGER(hsize_t),   DIMENSION(1:1) :: dims = (/DIM0/)

  CHARACTER(LEN=size), DIMENSION(1:dim0), TARGET :: wdata ! Write buffer
  CHARACTER(LEN=size), DIMENSION(:), ALLOCATABLE, TARGET :: rdata ! Read buffer
  CHARACTER(LEN=size-1) :: str = "OPAQUE"
  
  CHARACTER(LEN=14) :: tag_sm    ! Test reading obaque tag into 
  CHARACTER(LEN=15) :: tag_exact ! buffers that are: to small, exact
  CHARACTER(LEN=17) :: tag_big   ! and to big.

  INTEGER :: taglen
  INTEGER(HSIZE_T), DIMENSION(1:1) :: maxdims
  INTEGER :: i
  CHARACTER(LEN=1) :: ichr
  TYPE(C_PTR) :: f_ptr
  INTEGER :: error
  !
  ! Initialize data.
  !
  DO i = 1, dim0
     WRITE(ichr,'(I1)') i-1 
     wdata(i) = str//ichr
  ENDDO
  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, total_error)
  !
  ! Create opaque datatype and set the tag to something appropriate.
  ! For this example we will write and view the data as a character
  ! array.
  !
  CALL h5tcreate_f(h5T_OPAQUE_F, size, dtype, error)
  CALL check("h5tcreate_f",error, total_error)
  CALL h5tset_tag_f(dtype,"Character array",error)
  CALL check("h5tset_tag_f",error, total_error)
  !
  ! Create dataspace.  Setting maximum size to be the current size.
  !
  CALL h5screate_simple_f(1, dims, space, error)
  CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset and write the opaque data to it.
  !
  CALL h5dcreate_f(file, dataset, dtype, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)
  f_ptr = C_LOC(wdata(1))
  CALL h5dwrite_f(dset, dtype, f_ptr, error)
  CALL check("h5dwrite_f",error, total_error)
  !
  ! Close and release resources.
  !
  CALL H5Dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL H5Sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(dtype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL H5Fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)
  !
  ! Now we begin the read section of this example.
  !
  ! Open file and dataset.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  !
  ! Get datatype and properties for the datatype.
  !
  CALL h5dget_type_f(dset, dtype, error)
  CALL check("h5dget_type_f",error, total_error)
  CALL h5tget_size_f(dtype, len, error)
  CALL check("h5tget_size_f",error, total_error)

  ! Next tests should return 
  ! opaque_tag = tag = "Character array" and the actual length = 15
  
  ! Test reading into a string that is to small
  CALL h5tget_tag_f(dtype, tag_sm, taglen, error)
  CALL check("h5tget_tag_f",error, total_error)
  CALL VERIFY("h5tget_tag_f", taglen, 15, total_error)
  CALL verifystring("h5tget_tag_f",tag_sm,"Character arra", total_error)
  
  ! Test reading into a string that is exact
  CALL h5tget_tag_f(dtype, tag_exact, taglen, error)
  CALL check("h5tget_tag_f",error, total_error)
  CALL VERIFY("h5tget_tag_f", taglen, 15, total_error)
  CALL verifystring("h5tget_tag_f",tag_exact,"Character array", total_error)

  ! Test reading into a string that is to big
  CALL h5tget_tag_f(dtype, tag_big, taglen, error)
  CALL check("h5tget_tag_f",error, total_error)
  CALL VERIFY("h5tget_tag_f", taglen, 15, total_error)
  CALL verifystring("h5tget_tag_f",tag_big,"Character array  ", total_error)
  
  !
  ! Get dataspace and allocate memory for read buffer.
  !
  CALL h5dget_space_f(dset, space, error)
  CALL check("H5Dget_space_f",error, total_error)
  CALL h5sget_simple_extent_dims_f(space, dims, maxdims, error)
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)
  ALLOCATE(rdata(1:dims(1)))
  !
  ! Read the data.
  !
  f_ptr = C_LOC(rdata(1))
  CALL h5dread_f(dset, dtype, f_ptr, error)
  CALL check("H5Dread_f",error, total_error)
  !
  DO i = 1, dims(1)
     CALL verifystring("t_opaque",TRIM(rdata(i)),TRIM(wdata(i)), total_error)
  ENDDO
  !
  ! Close and release resources.
  !
  DEALLOCATE(rdata)
  CALL H5Dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL H5Sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(dtype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL H5Fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)
  
END SUBROUTINE t_opaque

SUBROUTINE t_objref(total_error)

  USE HDF5
  USE ISO_C_BINDING

  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error
  CHARACTER(LEN=20), PARAMETER :: filename  = "t_objref_F03.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"
  INTEGER          , PARAMETER :: dim0      = 2

  INTEGER(HID_T)  :: file, space, dset, obj ! Handles
  INTEGER :: error

  INTEGER(hsize_t),   DIMENSION(1:1) :: dims = (/dim0/)
  TYPE(hobj_ref_t_f), DIMENSION(1:dim0), TARGET :: wdata ! Write buffer
  TYPE(hobj_ref_t_f), DIMENSION(:), ALLOCATABLE, TARGET :: rdata ! Read buffer
  INTEGER :: objtype
  INTEGER(SIZE_T) :: name_size
  CHARACTER(LEN=80) :: name
  INTEGER(HSIZE_T), DIMENSION(1:1) :: maxdims
  INTEGER :: i
  TYPE(C_PTR) :: f_ptr
  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, total_error)
  !
  ! Create a dataset with a null dataspace.
  !
  CALL h5screate_f(H5S_NULL_F,space,error)
  CALL check("h5screate_f",error, total_error)
  CALL h5dcreate_f(file, "DS2", H5T_STD_I32LE, space, obj, error)
  CALL check("h5dcreate_f",error, total_error)
  !
  CALL h5dclose_f(obj  , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  !
  ! Create a group.
  !
  CALL h5gcreate_f(file, "G1", obj, error)
  CALL check("h5gcreate_f",error, total_error)
  CALL h5gclose_f(obj, error)
  CALL check("h5gclose_f",error, total_error)
  !
  ! Create references to the previously created objects. note, space_id
  ! is not needed for object references.
  !
  f_ptr = C_LOC(wdata(1))
  CALL H5Rcreate_f(file, "G1", H5R_OBJECT_F, f_ptr, error)
  CALL check("H5Rcreate_f",error, total_error)
  f_ptr = C_LOC(wdata(2))
  CALL H5Rcreate_f(file, "DS2", H5R_OBJECT_F, f_ptr, error)
  CALL check("H5Rcreate_f",error, total_error)
  !
  ! Create dataspace.  Setting maximum size to be the current size.
  !
  CALL h5screate_simple_f(1, dims, space, error)
  CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset and write the object references to it.
  !
  CALL h5dcreate_f(file, dataset, H5T_STD_REF_OBJ, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)
  
  f_ptr = C_LOC(wdata(1))
  CALL h5dwrite_f(dset, H5T_STD_REF_OBJ, f_ptr, error)
  CALL check("h5dwrite_f",error, total_error)
  !
  ! Close and release resources.
  !
  CALL h5dclose_f(dset , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)
  !
  ! Now we begin the read section of this example.
  !
  ! Open file and dataset.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  !
  ! Get dataspace and allocate memory for read buffer.
  !
  CALL h5dget_space_f(dset, space, error)
  CALL check("H5Dget_space_f",error, total_error)
  CALL h5sget_simple_extent_dims_f(space, dims, maxdims, error)
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)

  ALLOCATE(rdata(1:maxdims(1)))
  !
  ! Read the data.
  !
  f_ptr = C_LOC(rdata(1))
  CALL h5dread_f( dset, H5T_STD_REF_OBJ, f_ptr, error)
  CALL check("H5Dread_f",error, total_error)
  !
  ! Output the data to the screen.
  !
  DO i = 1, maxdims(1)
     !
     ! Open the referenced object, get its name and type.
     !
     f_ptr = C_LOC(rdata(i))
     CALL H5Rdereference_f(dset, H5R_OBJECT_F, f_ptr, obj, error)
     CALL check("H5Rdereference_f",error, total_error)
     CALL H5Rget_obj_type_f(dset, H5R_OBJECT_F, f_ptr, objtype, error)
     CALL check("H5Rget_obj_type_f",error, total_error)
     !
     ! Get the length of the name and name
     !
     CALL H5Iget_name_f(obj, name, 80_size_t, name_size, error)
     CALL check("H5Iget_name_f",error, total_error)
     !
     ! Print the object type and close the object.
     !
     IF(objtype.EQ.H5G_GROUP_F)THEN
        CALL verifystring("t_objref", name(1:name_size),"/G1", total_error)
     ELSE IF(objtype.EQ.H5G_DATASET_F)THEN
        CALL verifystring("t_objref", name(1:name_size),"/DS2", total_error)
     ELSE
        total_error = total_error + 1
     ENDIF
     CALL h5oclose_f(obj, error)
     CALL check("h5oclose_f",error, total_error)

  END DO
  !
  ! Close and release resources.
  !
  DEALLOCATE(rdata)
  CALL h5dclose_f(dset , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)

END SUBROUTINE t_objref


SUBROUTINE t_regref(total_error)

  USE HDF5
  USE ISO_C_BINDING

  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error
  CHARACTER(LEN=22), PARAMETER :: filename  = "t_regref_F03.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"
  CHARACTER(LEN=3) , PARAMETER :: dataset2  = "DS2"
  INTEGER          , PARAMETER :: dim0      = 2
  INTEGER          , PARAMETER :: ds2dim0   = 16
  INTEGER          , PARAMETER :: ds2dim1   = 3

  INTEGER(HID_T)  :: file, memspace, space, dset, dset2 ! Handles
  INTEGER :: error

  INTEGER(HSIZE_T), DIMENSION(1:1)   :: dims = (/dim0/)
  INTEGER(HSIZE_T), DIMENSION(1:1)   :: dims3 
  INTEGER(HSIZE_T), DIMENSION(1:2)   :: dims2 = (/ds2dim0,ds2dim1/)

  INTEGER(HSIZE_T), DIMENSION(1:2,1:4) :: coords = RESHAPE((/2,1,12,3,1,2,5,3/),(/2,4/))
  
  INTEGER(HSIZE_T), DIMENSION(1:2) :: start=(/0,0/),stride=(/11,2/),count=(/2,2/), BLOCK=(/3,1/)

  INTEGER(HSIZE_T), DIMENSION(1:1) :: maxdims
  INTEGER(hssize_t) :: npoints
  TYPE(hdset_reg_ref_t_f), DIMENSION(1:dim0), TARGET :: wdata ! Write buffer
  TYPE(hdset_reg_ref_t_f), DIMENSION(:), ALLOCATABLE, TARGET :: rdata ! Read buffer

  INTEGER(size_t) :: size
  CHARACTER(LEN=1), DIMENSION(1:ds2dim0,1:ds2dim1), TARGET :: wdata2

  CHARACTER(LEN=80),DIMENSION(1:1), TARGET :: rdata2
  CHARACTER(LEN=80) :: name
  INTEGER :: i
  TYPE(C_PTR) :: f_ptr
  CHARACTER(LEN=ds2dim0) :: chrvar
  CHARACTER(LEN=20), DIMENSION(1:2) :: chrref_correct

  chrvar = "The quick brown "
  READ(chrvar,'(16A1)') wdata2(1:16,1)
  chrvar = "fox jumps over  "
  READ(chrvar,'(16A1)') wdata2(1:16,2)
  chrvar = "the 5 lazy dogs "
  READ(chrvar,'(16A1)') wdata2(1:16,3)

  chrref_correct(1) = 'hdf5'
  chrref_correct(2) = 'Therowthedog'

  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, total_error)
  !
  ! Create a dataset with character data.
  !
  CALL h5screate_simple_f(2, dims2, space, error)
  CALL check("h5screate_simple_f",error, total_error)
  CALL h5dcreate_f(file,dataset2, H5T_STD_I8LE, space, dset2, error)
  CALL check("h5dcreate_f",error, total_error)
  f_ptr = C_LOC(wdata2(1,1))
  CALL h5dwrite_f(dset2, H5T_NATIVE_INTEGER_1, f_ptr, error)
  CALL check("h5dwrite_f",error, total_error)
  !
  ! Create reference to a list of elements in dset2.
  !
  CALL h5sselect_elements_f(space, H5S_SELECT_SET_F, 2, INT(4,size_t), coords, error)
  CALL check("h5sselect_elements_f",error, total_error)
  f_ptr = C_LOC(wdata(1))
  CALL h5rcreate_f(file, DATASET2, H5R_DATASET_REGION_F, f_ptr, error, space)
  CALL check("h5rcreate_f",error, total_error)
  !
  ! Create reference to a hyperslab in dset2, close dataspace.
  !
  CALL h5sselect_hyperslab_f (space, H5S_SELECT_SET_F, start, count, error, stride, block)
  CALL check("h5sselect_hyperslab_f",error, total_error)
  f_ptr = C_LOC(wdata(2))
  CALL h5rcreate_f(file, DATASET2, H5R_DATASET_REGION_F, f_ptr, error, space)
  CALL check("h5rcreate_f",error, total_error)

  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  !
  ! Create dataspace.  Setting maximum size to the current size.
  !
  CALL h5screate_simple_f(1, dims, space, error)
  CALL check("h5screate_simple_f",error, total_error)

  !
  ! Create the dataset and write the region references to it.
  !
  CALL h5dcreate_f(file, dataset, H5T_STD_REF_DSETREG, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)
  f_ptr = C_LOC(wdata(1))
  CALL h5dwrite_f(dset, H5T_STD_REF_DSETREG, f_ptr, error)
  CALL check("h5dwrite_f",error, total_error)
  !
  ! Close and release resources.
  !
  CALL h5dclose_f(dset , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5dclose_f(dset2, error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)

  !
  ! Now we begin the read section of this example.
  !
  !
  ! Open file and dataset.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  !
  ! Get dataspace and allocate memory for read buffer.
  !
  CALL h5dget_space_f(dset, space, error)
  CALL check("H5Dget_space_f",error, total_error)
  CALL h5sget_simple_extent_dims_f(space, dims, maxdims, error)
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)
  ALLOCATE(rdata(1:dims(1)))
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  !
  ! Read the data.
  !
  f_ptr = C_LOC(rdata(1))
  CALL h5dread_f( dset, H5T_STD_REF_DSETREG, f_ptr, error)
  CALL check("H5Dread_f",error, total_error)
  !
  ! Output the data to the screen.
  !
  DO i = 1, dims(1)
     
     !
     ! Open the referenced object, retrieve its region as a
     ! dataspace selection.
     !
     CALL H5Rdereference_f(dset, rdata(i), dset2, error)
     CALL check("H5Rdereference_f",error, total_error)
     
     CALL H5Rget_region_f(dset, rdata(i), space, error)
     CALL check("H5Rget_region_f",error, total_error)
  
     !
     ! Get the object's name
     !
     CALL H5Iget_name_f(dset2, name, 80_size_t, size, error)
     CALL check("H5Iget_name_f",error, total_error)
     CALL VERIFY("H5Iget_name_f", size, LEN_TRIM(name), total_error)
     CALL verifystring("H5Iget_name_f",name(1:size),TRIM(name), total_error)
     !
     ! Allocate space for the read buffer.
     !
     CALL H5Sget_select_npoints_f(space, npoints, error)
     CALL check("H5Sget_select_npoints_f",error, total_error)
     CALL VERIFY("H5Sget_select_npoints_f", npoints, LEN_TRIM(chrref_correct(i)), total_error)
     
     dims3(1) = npoints
     !
     ! Read the dataset region.
     !
     CALL h5screate_simple_f(1, dims3, memspace, error)
     CALL check("h5screate_simple_f",error, total_error)

     f_ptr = C_LOC(rdata2(1))
     CALL h5dread_f( dset2, H5T_NATIVE_INTEGER_1, f_ptr, error, memspace, space)
     CALL check("H5Dread_f",error, total_error)
     CALL verifystring("h5dread_f",rdata2(1)(1:npoints),TRIM(chrref_correct(i)), total_error)

     CALL H5Sclose_f(space, error)
     CALL check("h5sclose_f",error, total_error)
     CALL H5Sclose_f(memspace, error)
     CALL check("h5sclose_f",error, total_error)
     CALL H5Dclose_f(dset2, error)
     CALL check("h5dclose_f",error, total_error)

  END DO
  !
  ! Close and release resources.
  !
  DEALLOCATE(rdata)
  CALL H5Dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL H5Fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)

END SUBROUTINE t_regref

SUBROUTINE t_vlen(total_error)

  USE HDF5
  USE ISO_C_BINDING

  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error
  CHARACTER(LEN=18), PARAMETER :: filename  = "t_vlen_F03.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"
  INTEGER, PARAMETER :: LEN0 = 3
  INTEGER, PARAMETER :: LEN1 = 12
  INTEGER :: dim0

  INTEGER(HID_T)  :: file, filetype, memtype, space, dset ! Handles
  INTEGER :: error
  INTEGER(HSIZE_T), DIMENSION(1:2)   :: maxdims
  INTEGER :: i, j

  ! vl data
  TYPE vl
     INTEGER, DIMENSION(:), POINTER :: data
  END TYPE vl
  TYPE(vl), DIMENSION(:), ALLOCATABLE :: ptr

  TYPE hvl_t
     INTEGER(size_t) :: len ! Length of VL data (in base type units)
     TYPE(C_PTR) :: p       ! Pointer to VL data
  END TYPE hvl_t
  TYPE(hvl_t), DIMENSION(1:2), TARGET :: wdata ! Array of vlen structures
  TYPE(hvl_t), DIMENSION(1:2), TARGET :: rdata ! Pointer to vlen structures

  INTEGER(hsize_t), DIMENSION(1:1) :: dims = (/2/)
  INTEGER, DIMENSION(:), POINTER :: ptr_r 
  TYPE(C_PTR) :: f_ptr
  
  !
  ! Initialize variable-length data.  wdata(1) is a countdown of
  ! length LEN0, wdata(2) is a Fibonacci sequence of length LEN1.
  !
  wdata(1)%len = LEN0
  wdata(2)%len = LEN1

  ALLOCATE( ptr(1:2) )
  ALLOCATE( ptr(1)%data(1:wdata(1)%len) )
  ALLOCATE( ptr(2)%data(1:wdata(2)%len) )

  DO i=1, wdata(1)%len
     ptr(1)%data(i) = wdata(1)%len - i + 1 ! 3 2 1
  ENDDO
  wdata(1)%p = C_LOC(ptr(1)%data(1))

  ptr(2)%data(1:2) = 1
  DO i = 3, wdata(2)%len
     ptr(2)%data(i) = ptr(2)%data(i-1) + ptr(2)%data(i-2) ! (1 1 2 3 5 8 etc.)
  ENDDO
  wdata(2)%p = C_LOC(ptr(2)%data(1))

  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, total_error)
  !
  ! Create variable-length datatype for file and memory.
  !
  CALL H5Tvlen_create_f(H5T_STD_I32LE, filetype, error)
  CALL check("H5Tvlen_create_f",error, total_error)
  CALL H5Tvlen_create_f(H5T_NATIVE_INTEGER, memtype, error)
  CALL check("H5Tvlen_create_f",error, total_error)
  !
  ! Create dataspace.
  !
  CALL h5screate_simple_f(1, dims, space, error)
  CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset and write the variable-length data to it.
  !
  CALL H5Dcreate_f(file, dataset, filetype, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)
 
  f_ptr = C_LOC(wdata(1))
  CALL h5dwrite_f(dset, memtype, f_ptr, error)
  CALL check("h5dwrite_f",error, total_error)
  !
  ! Close and release resources.  Note the use of H5Dvlen_reclaim
  ! removes the need to manually deallocate the previously allocated
  ! data.
  !

  CALL h5dclose_f(dset , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(filetype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL H5Tclose_f(memtype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)

  !
  ! Now we begin the read section of this example.

  !
  ! Open file and dataset.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)

  !
  ! Get dataspace and allocate memory for array of vlen structures.
  ! This does not actually allocate memory for the vlen data, that
  ! will be done by the library.
  !
  CALL H5Dget_space_f(dset, space, error)
  CALL check("H5Dget_space_f",error, total_error)
  dim0 = dims(1)
  CALL H5Sget_simple_extent_dims_f(space, dims, maxdims, error) 
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)

  !
  ! Create the memory datatype.
  !
  CALL H5Tvlen_create_f(H5T_NATIVE_INTEGER, memtype, error) 
  CALL check("H5Tvlen_create_f",error, total_error)

  !
  ! Read the data.
  !
  f_ptr = C_LOC(rdata(1))
  CALL H5Dread_f(dset, memtype, f_ptr, error)
  CALL check("H5Dread_f",error, total_error)

  DO i = 1, dims(1)
     CALL c_f_pointer(rdata(i)%p, ptr_r, [rdata(i)%len] )
     DO j = 1, rdata(i)%len
        CALL VERIFY("t_vlen", ptr_r(j), ptr(i)%data(j), total_error)
     ENDDO
  ENDDO
  !
  ! Close and release resources.
  !
  DEALLOCATE(ptr)
  CALL h5dvlen_reclaim_f(memtype, space, H5P_DEFAULT_F, f_ptr, error)
  CALL h5dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(memtype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL h5fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)

END SUBROUTINE t_vlen


SUBROUTINE t_vlstring(total_error)

  USE HDF5
  USE ISO_C_BINDING

  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error

  CHARACTER(LEN=18), PARAMETER :: filename  = "t_vlstring.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"

  INTEGER(SIZE_T), PARAMETER :: dim0      = 4
  INTEGER(SIZE_T), PARAMETER :: sdim      = 7
  INTEGER(HID_T)  :: file, filetype, space, dset ! Handles
  INTEGER :: error
  INTEGER(HSIZE_T), DIMENSION(1:1) :: dims = (/dim0/)
  INTEGER(HSIZE_T), DIMENSION(1:2) :: maxdims
  
  CHARACTER(LEN=sdim), DIMENSION(1:dim0), TARGET :: &
       wdata = (/"Parting", "is such", "sweet  ", "sorrow."/) ! Write buffer
  CHARACTER(LEN=sdim), DIMENSION(:), ALLOCATABLE :: rdata ! Read buffer
  INTEGER(HSIZE_T), DIMENSION(2) :: data_dims = (/sdim,dim0/)
  INTEGER(SIZE_T), DIMENSION(4) :: str_len = (/7,7,5,7/)
  INTEGER :: i

  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, total_error)
  !
  ! Create file and memory datatypes.  For this example we will save
  ! the strings as C variable length strings, H5T_STRING is defined
  ! as a variable length string.
  !
  CALL H5Tcopy_f(H5T_STRING, filetype, error)
  CALL check("H5Tcopy_f",error, total_error)
  CALL H5Tset_strpad_f(filetype, H5T_STR_NULLPAD_F, error)
  CALL check("H5Tset_strpad_f",error, total_error)
  !
  ! Create dataspace.
  !
  CALL h5screate_simple_f(1, dims, space, error)
  CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset and write the variable-length string data to
  ! it.
  !
  CALL h5dcreate_f(file, dataset, filetype, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)

  CALL h5dwrite_vl_f(dset, filetype, wdata, data_dims, str_len, error, space)
  CALL check("h5dwrite_vl_f",error, total_error)

  !
  ! Close and release resources.
  !
  CALL h5dclose_f(dset , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(filetype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)

  !
  ! Now we begin the read section of this example.
  !
  !
  ! Open file and dataset.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  !
  ! Get the datatype.
  !
  CALL H5Dget_type_f(dset, filetype, error)
  CALL check("H5Dget_type_f",error, total_error)
  !
  ! Get dataspace and allocate memory for read buffer.
  !
  CALL H5Dget_space_f(dset, space, error)
  CALL check("H5Dget_space_f",error, total_error)
  CALL H5Sget_simple_extent_dims_f(space, dims, maxdims, error) 
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)

  ALLOCATE(rdata(1:dims(1)))

  !
  ! Read the data.
  !
  CALL h5dread_vl_f(dset, filetype, rdata, data_dims, str_len, error, space)
  CALL check("H5Dread_vl_f",error, total_error)

  !
  ! Output the data to the screen.
  !
  DO i = 1, dims(1)
     CALL verifystring("h5dopen_f",TRIM(rdata(i)),TRIM(wdata(i)) , total_error)
  END DO

  DEALLOCATE(rdata)
  CALL h5dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(filetype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)

END SUBROUTINE t_vlstring


SUBROUTINE t_string(total_error)

  USE HDF5
  USE ISO_C_BINDING

  IMPLICIT NONE

  INTEGER, INTENT(INOUT) :: total_error

  CHARACTER(LEN=20), PARAMETER :: filename  = "t_string_F03.h5"
  CHARACTER(LEN=3) , PARAMETER :: dataset   = "DS1"
  INTEGER          , PARAMETER :: dim0      = 4
  INTEGER(SIZE_T)  , PARAMETER :: sdim      = 8

  INTEGER(HID_T)  :: file, filetype, memtype, space, dset ! Handles
  INTEGER :: error

  INTEGER(HSIZE_T), DIMENSION(1:1) :: dims = (/dim0/)
  INTEGER(HSIZE_T), DIMENSION(1:1) :: maxdims

  CHARACTER(LEN=sdim), DIMENSION(1:dim0), TARGET :: &
       wdata = (/"Parting", "is such", "sweet  ", "sorrow."/)
  CHARACTER(LEN=sdim), DIMENSION(:), ALLOCATABLE, TARGET :: rdata
  INTEGER :: i
  INTEGER(SIZE_T) :: size
  TYPE(C_PTR) :: f_ptr
  !
  ! Create a new file using the default properties.
  !
  CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file, error)
  CALL check("h5fcreate_f",error, total_error)
  !
  ! Create file datatypes.  For this example we will save
  ! the strings as FORTRAN strings
  !
  CALL H5Tcopy_f(H5T_FORTRAN_S1, filetype, error)
  CALL check("H5Tcopy_f",error, total_error)
  CALL H5Tset_size_f(filetype, sdim, error)
  CALL check("H5Tset_size_f",error, total_error)
  !
  ! Create dataspace.
  !
  CALL h5screate_simple_f(1, dims, space, error)
  CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset and write the string data to it.
  !
  CALL h5dcreate_f(file, dataset, filetype, space, dset, error)
  CALL check("h5dcreate_f",error, total_error)

  f_ptr = C_LOC(wdata(1)(1:1))
  CALL H5Dwrite_f(dset, filetype, f_ptr, error)
  CALL check("H5Dwrite_f",error, total_error)
  !
  ! Close and release resources.
  !
  CALL h5dclose_f(dset , error)
  CALL check("h5dclose_f",error, total_error)
  CALL h5sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(filetype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL h5fclose_f(file , error)
  CALL check("h5fclose_f",error, total_error)
  !
  ! Now we begin the read section of this example.
  !
  ! Open file and dataset.
  !
  CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file, error)
  CALL check("h5fopen_f",error, total_error)
  CALL h5dopen_f(file, dataset, dset, error)
  CALL check("h5dopen_f",error, total_error)
  !
  ! Get the datatype and its size.
  !
  CALL H5Dget_type_f(dset, filetype, error)
  CALL check("H5Dget_type_f",error, total_error)
  CALL H5Tget_size_f(filetype, size, error)
  CALL check("H5Tget_size_f",error, total_error)
  CALL VERIFY("H5Tget_size_f", size, sdim, total_error)
  !
  ! Get dataspace.
  !
  CALL H5Dget_space_f(dset, space, error)
  CALL check("H5Dget_space_f",error, total_error)
  CALL H5Sget_simple_extent_dims_f(space, dims, maxdims, error) 
  CALL check("H5Sget_simple_extent_dims_f",error, total_error)
  CALL VERIFY("H5Sget_simple_extent_dims_f", dims(1), dim0, total_error)

  ALLOCATE(rdata(1:dims(1)))
  !
  ! Create the memory datatype.
  !
  CALL H5Tcopy_f(H5T_FORTRAN_S1, memtype, error) 
  CALL check("H5Tcopy_f",error, total_error)
  CALL H5Tset_size_f(memtype, sdim, error) 
  CALL check("H5Tset_size_f",error, total_error)
  !
  ! Read the data.
  !
  f_ptr = C_LOC(rdata(1)(1:1))
  CALL H5Dread_f(dset, memtype, f_ptr, error, space)
  CALL check("H5Dread_f",error, total_error)

  DO i = 1, dims(1)
     CALL verifystring("h5dopen_f",TRIM(rdata(i)),TRIM(wdata(i)) , total_error)
  END DO

  DEALLOCATE(rdata)

  !
  ! Close and release resources.
  !
  CALL H5Dclose_f(dset, error)
  CALL check("h5dclose_f",error, total_error)
  CALL H5Sclose_f(space, error)
  CALL check("h5sclose_f",error, total_error)
  CALL H5Tclose_f(memtype, error)
  CALL check("h5tclose_f",error, total_error)
  CALL H5Fclose_f(file, error)
  CALL check("h5fclose_f",error, total_error)


END SUBROUTINE t_string


