 !
 ! Purpose:	This is the second half of a two-part test that makes sure
 !		that a file can be read after an application crashes as long
 !		as the file was flushed first.  This half tries to read the
 !		file created by the first half.
 !

     PROGRAM FFLUSH2EXAMPLE

     USE HDF5 ! This module contains all necessary modules 
        
     IMPLICIT NONE

     !
     !the respective filename is "fflush1.h5" 
     !
     CHARACTER(LEN=10), PARAMETER :: filename = "fflush1.h5"

     !
     !data space rank and dimensions
     !
     INTEGER, PARAMETER :: RANK = 2
     INTEGER, PARAMETER :: NX = 4
     INTEGER, PARAMETER :: NY = 5

     !
     ! File identifiers
     !
     INTEGER(HID_T) :: file_id 
     
     !
     ! Group identifier
     !
     INTEGER(HID_T) :: gid 

     !
     ! dataset identifier
     !
     INTEGER(HID_T) :: dset_id
 
     !
     ! data space identifier
     !
     INTEGER(HID_T) :: dataspace
 
     !
     ! data type identifier
     !
     INTEGER(HID_T) :: dtype_id

     ! 
     !The dimensions for the dataset.
     !
     INTEGER(HSIZE_T), DIMENSION(2) :: dims = (/NX,NY/)

     !
     !flag to check operation success 
     !         
     INTEGER     ::   error

     !
     !general purpose integer 
     !         
     INTEGER     ::   i, j, total_error = 0

     !
     !data buffers 
     !         
     INTEGER, DIMENSION(NX,NY) :: data_out

     !
     !Initialize FORTRAN predifined datatypes
     !
     CALL h5init_fortran_f(error) 
          CALL check("h5init_types_f",error,total_error)

     !
     !Open the file.
     !
     CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file_id, error)
          CALL check("h5fopen_f",error,total_error)

     !
     !Open the dataset
     ! 
     CALL h5dopen_f(file_id, "/D", dset_id, error)
          CALL check("h5dopen_f",error,total_error)

     !
     !Get dataset's data type.
     ! 
     CALL h5dget_type_f(dset_id, dtype_id, error)
          CALL check("h5dget_type_f",error,total_error)

     !
     !Read the dataset.
     !
     CALL h5dread_f(dset_id, dtype_id, data_out, error)
          CALL check("h5dread_f",error,total_error)

     !
     !Print the dataset.
     !
     do i = 1, NX
          write(*,*) (data_out(i,j), j = 1, NY)
     end do
!
!result of the print statement
!
! 0,  1,  2,  3,  4
! 1,  2,  3,  4,  5
! 2,  3,  4,  5,  6
! 3,  4,  5,  6,  7

     !
     !Open the group.
     !
     CALL h5gopen_f(file_id, "G", gid, error)     
          CALL check("h5gopen_f",error,total_error)
    
     !
     !In case error happens, jump to stop.
     !
     IF (error == -1) THEN
          001 STOP
     END IF
    
     !
     !Close the datatype
     !
     CALL h5tclose_f(dtype_id, error)
          CALL check("h5tclose_f",error,total_error)

     !
     !Close the dataset.
     !
     CALL h5dclose_f(dset_id, error)
          CALL check("h5dclose_f",error,total_error)

     !
     !Close the group.
     !
     CALL h5gclose_f(gid, error)
          CALL check("h5gclose_f",error,total_error)

     !
     !Close the file.
     !
     CALL h5fclose_f(file_id, error)
          CALL check("h5fclose_f",error,total_error)

     !
     !Close FORTRAN predifined datatypes
     !
     CALL h5close_fortran_f(error)
         CALL check("h5close_types_f",error,total_error)

     END PROGRAM FFLUSH2EXAMPLE
