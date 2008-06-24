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
! This file contains Fortran90 interfaces for H5P functions.
!
MODULE H5P_F03

  USE H5GLOBAL

  INTERFACE h5pset_fill_value_f
     MODULE PROCEDURE h5pset_fill_value_integer
     MODULE PROCEDURE h5pset_fill_value_real
     ! Comment if on Crays
     MODULE PROCEDURE h5pset_fill_value_double
     ! End comment if on Crays
     MODULE PROCEDURE h5pset_fill_value_char
  END INTERFACE
    
  INTERFACE h5pget_fill_value_f
     MODULE PROCEDURE h5pget_fill_value_integer
     MODULE PROCEDURE h5pget_fill_value_real
     ! Comment if on Crays
     MODULE PROCEDURE h5pget_fill_value_double
     ! End comment if on Crays
     MODULE PROCEDURE h5pget_fill_value_char
  END INTERFACE

  INTERFACE h5pset_f
     MODULE PROCEDURE h5pset_integer
     MODULE PROCEDURE h5pset_real
     ! Comment if on Crays
     MODULE PROCEDURE h5pset_double
     ! End comment if on Crays
     MODULE PROCEDURE h5pset_char
  END INTERFACE

  INTERFACE h5pget_f
     MODULE PROCEDURE h5pget_integer
     MODULE PROCEDURE h5pget_real
     ! Comment if on Crays
     MODULE PROCEDURE h5pget_double
     ! End comment if on Crays
     MODULE PROCEDURE h5pget_char
  END INTERFACE

  INTERFACE h5pregister_f
     MODULE PROCEDURE h5pregister_integer
     MODULE PROCEDURE h5pregister_real
     ! Comment if on Crays
     MODULE PROCEDURE h5pregister_double
     ! End comment if on Crays
     MODULE PROCEDURE h5pregister_char
  END INTERFACE

  INTERFACE h5pinsert_f
     MODULE PROCEDURE h5pinsert_integer
     MODULE PROCEDURE h5pinsert_real
     ! Comment if on Crays
     MODULE PROCEDURE h5pinsert_double
     ! End comment if on Crays
     MODULE PROCEDURE h5pinsert_char
  END INTERFACE

CONTAINS

  !----------------------------------------------------------------------
  ! Name:		h5pset(get)fill_value_f 
  !
  ! Purpose: 	Sets(gets) fill value for a dataset creation property list
  !
  ! Inputs:  
  !		prp_id		- dataset creation property list identifier
  !		type_id		- datatype identifier for fill value
  !		fillvalue	- fill value
  ! Outputs:  
  !	(	type_id		- datatype identifier for fill value )
  !	(		fillvalue	- fill value )
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !		August 12, 1999	
  !
  ! Modifications: 	Explicit Fortran interfaces were added for 
  !			called C functions (it is needed for Windows
  !			port).  March 14, 2001 
  !
  ! Comment:	h5pset(get)fill_value_f function is overloaded to support
  !		INTEGER, REAL, DOUBLE PRECISION and CHARACTER dtatypes.		
  !----------------------------------------------------------------------


  SUBROUTINE h5pset_fill_value_integer(prp_id, type_id, fillvalue, &
       hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pset_fill_value_integer
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id ! Property list identifier 
    INTEGER(HID_T), INTENT(IN) :: type_id ! Datatype identifier of 
    ! of fillvalue datatype 
    ! (in memory)
    INTEGER, INTENT(IN) :: fillvalue   ! Fillvalue
    INTEGER, INTENT(OUT) :: hdferr  ! Error code

    !            INTEGER, EXTERNAL :: h5pset_fill_value_integer_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5pset_fill_value_integer_c(prp_id, type_id, fillvalue)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PSET_FILL_VALUE_INTEGER_C'::h5pset_fill_value_integer_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: prp_id
         INTEGER(HID_T), INTENT(IN) :: type_id
         INTEGER, INTENT(IN) :: fillvalue
       END FUNCTION h5pset_fill_value_integer_c
    END INTERFACE

    hdferr = h5pset_fill_value_integer_c(prp_id, type_id, fillvalue)
  END SUBROUTINE h5pset_fill_value_integer


  SUBROUTINE h5pget_fill_value_integer(prp_id, type_id, fillvalue, &
       hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pget_fill_value_integer
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id ! Property list identifier 
    INTEGER(HID_T), INTENT(IN) :: type_id ! Datatype identifier of 
    ! of fillvalue datatype
    ! (in memory) 
    INTEGER, INTENT(IN) :: fillvalue   ! Fillvalue
    INTEGER, INTENT(OUT) :: hdferr  ! Error code

    !            INTEGER, EXTERNAL :: h5pget_fill_value_integer_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5pget_fill_value_integer_c(prp_id, type_id, fillvalue)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PGET_FILL_VALUE_INTEGER_C'::h5pget_fill_value_integer_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: prp_id
         INTEGER(HID_T), INTENT(IN) :: type_id
         INTEGER :: fillvalue
       END FUNCTION h5pget_fill_value_integer_c
    END INTERFACE

    hdferr = h5pget_fill_value_integer_c(prp_id, type_id, fillvalue)
  END SUBROUTINE h5pget_fill_value_integer


  SUBROUTINE h5pset_fill_value_real(prp_id, type_id, fillvalue, &
       hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pset_fill_value_real
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id ! Property list identifier 
    INTEGER(HID_T), INTENT(IN) :: type_id ! Datatype identifier of 
    ! of fillvalue datatype 
    ! (in memory)
    REAL, INTENT(IN) :: fillvalue   ! Fillvalue
    INTEGER, INTENT(OUT) :: hdferr  ! Error code

    !            INTEGER, EXTERNAL :: h5pset_fill_value_real_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5pset_fill_value_real_c(prp_id, type_id, fillvalue)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PSET_FILL_VALUE_REAL_C'::h5pset_fill_value_real_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: prp_id
         INTEGER(HID_T), INTENT(IN) :: type_id
         REAL, INTENT(IN) :: fillvalue
       END FUNCTION h5pset_fill_value_real_c
    END INTERFACE

    hdferr = h5pset_fill_value_real_c(prp_id, type_id, fillvalue)
  END SUBROUTINE h5pset_fill_value_real


  SUBROUTINE h5pget_fill_value_real(prp_id, type_id, fillvalue, &
       hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pget_fill_value_real
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id ! Property list identifier 
    INTEGER(HID_T), INTENT(IN) :: type_id ! Datatype identifier of 
    ! of fillvalue datatype
    ! (in memory) 
    REAL, INTENT(IN) :: fillvalue   ! Fillvalue
    INTEGER, INTENT(OUT) :: hdferr  ! Error code

    !            INTEGER, EXTERNAL :: h5pget_fill_value_real_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5pget_fill_value_real_c(prp_id, type_id, fillvalue)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PGET_FILL_VALUE_REAL_C'::h5pget_fill_value_real_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: prp_id
         INTEGER(HID_T), INTENT(IN) :: type_id
         REAL :: fillvalue
       END FUNCTION h5pget_fill_value_real_c
    END INTERFACE

    hdferr = h5pget_fill_value_real_c(prp_id, type_id, fillvalue)
  END SUBROUTINE h5pget_fill_value_real


  SUBROUTINE h5pset_fill_value_double(prp_id, type_id, fillvalue, &
       hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pset_fill_value_double
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id ! Property list identifier 
    INTEGER(HID_T), INTENT(IN) :: type_id ! Datatype identifier of 
    ! of fillvalue datatype 
    ! (in memory)
    DOUBLE PRECISION, INTENT(IN) :: fillvalue   ! Fillvalue
    INTEGER, INTENT(OUT) :: hdferr  ! Error code

    !            INTEGER, EXTERNAL :: h5pset_fill_value_double_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5pset_fill_value_double_c(prp_id, type_id, fillvalue)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PSET_FILL_VALUE_DOUBLE_C'::h5pset_fill_value_double_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: prp_id
         INTEGER(HID_T), INTENT(IN) :: type_id
         DOUBLE PRECISION, INTENT(IN) :: fillvalue
       END FUNCTION h5pset_fill_value_double_c
    END INTERFACE

    hdferr = h5pset_fill_value_double_c(prp_id, type_id, fillvalue)
  END SUBROUTINE h5pset_fill_value_double


  SUBROUTINE h5pget_fill_value_double(prp_id, type_id, fillvalue, &
       hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pget_fill_value_double
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id ! Property list identifier 
    INTEGER(HID_T), INTENT(IN) :: type_id ! Datatype identifier of 
    ! of fillvalue datatype
    ! (in memory) 
    DOUBLE PRECISION, INTENT(IN) :: fillvalue   ! Fillvalue
    INTEGER, INTENT(OUT) :: hdferr  ! Error code

    !            INTEGER, EXTERNAL :: h5pget_fill_value_double_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5pget_fill_value_double_c(prp_id, type_id, fillvalue)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PGET_FILL_VALUE_DOUBLE_C'::h5pget_fill_value_double_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: prp_id
         INTEGER(HID_T), INTENT(IN) :: type_id
         DOUBLE PRECISION :: fillvalue
       END FUNCTION h5pget_fill_value_double_c
    END INTERFACE

    hdferr = h5pget_fill_value_double_c(prp_id, type_id, fillvalue)
  END SUBROUTINE h5pget_fill_value_double

  SUBROUTINE h5pset_fill_value_char(prp_id, type_id, fillvalue, &
       hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pset_fill_value_char
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id ! Property list identifier 
    INTEGER(HID_T), INTENT(IN) :: type_id ! Datatype identifier of 
    ! of fillvalue datatype 
    ! (in memory)
    CHARACTER, INTENT(IN) :: fillvalue   ! Fillvalue
    INTEGER, INTENT(OUT) :: hdferr  ! Error code

    !            INTEGER, EXTERNAL :: h5pset_fill_valuec_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5pset_fill_valuec_c(prp_id, type_id, fillvalue)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PSET_FILL_VALUEC_C'::h5pset_fill_valuec_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: fillvalue 
         INTEGER(HID_T), INTENT(IN) :: prp_id
         INTEGER(HID_T), INTENT(IN) :: type_id
         CHARACTER, INTENT(IN) :: fillvalue
       END FUNCTION h5pset_fill_valuec_c
    END INTERFACE

    hdferr = h5pset_fill_valuec_c(prp_id, type_id, fillvalue)
  END SUBROUTINE h5pset_fill_value_char

  SUBROUTINE h5pget_fill_value_char(prp_id, type_id, fillvalue, &
       hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pget_fill_value_char
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id ! Property list identifier 
    INTEGER(HID_T), INTENT(IN) :: type_id ! Datatype identifier of 
    ! of fillvalue datatype
    ! (in memory) 
    CHARACTER, INTENT(IN) :: fillvalue   ! Fillvalue
    INTEGER, INTENT(OUT) :: hdferr  ! Error code

    !            INTEGER, EXTERNAL :: h5pget_fill_valuec_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5pget_fill_valuec_c(prp_id, type_id, fillvalue)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PGET_FILL_VALUEC_C'::h5pget_fill_valuec_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: fillvalue 
         INTEGER(HID_T), INTENT(IN) :: prp_id
         INTEGER(HID_T), INTENT(IN) :: type_id
         CHARACTER :: fillvalue
       END FUNCTION h5pget_fill_valuec_c
    END INTERFACE

    hdferr = h5pget_fill_valuec_c(prp_id, type_id, fillvalue)
  END SUBROUTINE h5pget_fill_value_char


  !----------------------------------------------------------------------
  ! Name:		h5pset_integer 
  !
  ! Purpose: 	Sets a property list value
  !
  ! Inputs:  
  !		prp_id		- iproperty list identifier to modify
  !		name 		- name of property to modify
  !		value		- value to set property to
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 9, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pset_integer(prp_id, name, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pset_integer
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id  ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to modify
    INTEGER,   INTENT(IN) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pset_integer_c(prp_id, name, name_len, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PSET_INTEGER_C'::h5pset_integer_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: prp_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER, INTENT(IN) :: value
       END FUNCTION h5pset_integer_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pset_integer_c(prp_id, name , name_len, value)
  END SUBROUTINE h5pset_integer

  !----------------------------------------------------------------------
  ! Name:		h5pset_real
  !
  ! Purpose: 	Sets a property list value
  !
  ! Inputs:  
  !		prp_id		- iproperty list identifier to modify
  !		name 		- name of property to modify
  !		value		- value to set property to
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 9, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pset_real(prp_id, name, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pset_real
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id  ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to modify
    REAL,   INTENT(IN) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pset_real_c(prp_id, name, name_len, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PSET_REAL_C'::h5pset_real_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: prp_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         REAL, INTENT(IN) :: value
       END FUNCTION h5pset_real_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pset_real_c(prp_id, name , name_len, value)
  END SUBROUTINE h5pset_real

  !----------------------------------------------------------------------
  ! Name:		h5pset_double
  !
  ! Purpose: 	Sets a property list value
  !
  ! Inputs:  
  !		prp_id		- iproperty list identifier to modify
  !		name 		- name of property to modify
  !		value		- value to set property to
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 9, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pset_double(prp_id, name, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pset_double
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id  ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to modify
    DOUBLE PRECISION,   INTENT(IN) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pset_double_c(prp_id, name, name_len, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PSET_DOUBLE_C'::h5pset_double_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: prp_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         DOUBLE PRECISION, INTENT(IN) :: value
       END FUNCTION h5pset_double_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pset_double_c(prp_id, name , name_len, value)
  END SUBROUTINE h5pset_double

  !----------------------------------------------------------------------
  ! Name:		h5pset_char
  !
  ! Purpose: 	Sets a property list value
  !
  ! Inputs:  
  !		prp_id		- iproperty list identifier to modify
  !		name 		- name of property to modify
  !		value		- value to set property to
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 9, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pset_char(prp_id, name, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pset_char
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id  ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to modify
    CHARACTER(LEN=*),   INTENT(IN) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len
    INTEGER :: value_len

    INTERFACE
       INTEGER FUNCTION h5psetc_c(prp_id, name, name_len, value, value_len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PSETC_C'::h5psetc_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         !DEC$ATTRIBUTES reference :: value
         INTEGER(HID_T), INTENT(IN) :: prp_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         CHARACTER(LEN=*), INTENT(IN) :: value
         INTEGER, INTENT(IN)         :: value_len
       END FUNCTION h5psetc_c
    END INTERFACE

    name_len = LEN(name)
    value_len = LEN(value)
    hdferr = h5psetc_c(prp_id, name , name_len, value, value_len)
  END SUBROUTINE h5pset_char

  !----------------------------------------------------------------------
  ! Name:		h5pget_integer 
  !
  ! Purpose: 	Gets a property list value
  !
  ! Inputs:  
  !		prp_id		- iproperty list identifier to modify
  !		name 		- name of property to modify
  ! Outputs:  
  !		value		- value of property
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 9, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pget_integer(prp_id, name, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pget_integer
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id  ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to modify
    INTEGER,   INTENT(OUT) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pget_integer_c(prp_id, name, name_len, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PGET_INTEGER_C'::h5pget_integer_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: prp_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER, INTENT(OUT) :: value
       END FUNCTION h5pget_integer_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pget_integer_c(prp_id, name , name_len, value)
  END SUBROUTINE h5pget_integer

  !----------------------------------------------------------------------
  ! Name:		h5pget_real
  !
  ! Purpose: 	Gets a property list value
  !
  ! Inputs:  
  !		prp_id		- iproperty list identifier to modify
  !		name 		- name of property to modify
  ! Outputs:  
  !		value		- value of property
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 9, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pget_real(prp_id, name, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pget_real
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id  ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to modify
    REAL,   INTENT(OUT) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pget_real_c(prp_id, name, name_len, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PGET_REAL_C'::h5pget_real_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: prp_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         REAL, INTENT(OUT) :: value
       END FUNCTION h5pget_real_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pget_real_c(prp_id, name , name_len, value)
  END SUBROUTINE h5pget_real

  !----------------------------------------------------------------------
  ! Name:		h5pget_double
  !
  ! Purpose: 	Gets a property list value
  !
  ! Inputs:  
  !		prp_id		- iproperty list identifier to modify
  !		name 		- name of property to modify
  ! Outputs:  
  !		value		- value of property
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 9, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pget_double(prp_id, name, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pget_double
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id  ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to modify
    DOUBLE PRECISION,   INTENT(OUT) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pget_double_c(prp_id, name, name_len, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PGET_DOUBLE_C'::h5pget_double_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: prp_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         DOUBLE PRECISION, INTENT(OUT) :: value
       END FUNCTION h5pget_double_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pget_double_c(prp_id, name , name_len, value)
  END SUBROUTINE h5pget_double

  !----------------------------------------------------------------------
  ! Name:		h5pget_char
  !
  ! Purpose: 	Gets a property list value
  !
  ! Inputs:  
  !		prp_id		- iproperty list identifier to modify
  !		name 		- name of property to modify
  ! Outputs:  
  !		value		- value of property
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 9, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pget_char(prp_id, name, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pget_char
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: prp_id  ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to modify
    CHARACTER(LEN=*),   INTENT(OUT) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len
    INTEGER :: value_len

    INTERFACE
       INTEGER FUNCTION h5pgetc_c(prp_id, name, name_len, value, value_len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PGETC_C'::h5pgetc_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         !DEC$ATTRIBUTES reference :: value
         INTEGER(HID_T), INTENT(IN) :: prp_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         CHARACTER(LEN=*), INTENT(OUT) :: value
         INTEGER, INTENT(IN)         :: value_len
       END FUNCTION h5pgetc_c
    END INTERFACE

    name_len = LEN(name)
    value_len = LEN(value)
    hdferr = h5pgetc_c(prp_id, name , name_len, value, value_len)
  END SUBROUTINE h5pget_char

  !----------------------------------------------------------------------
  ! Name:		h5pregister_integer
  !
  ! Purpose: 	Registers a permanent property with a property list class.
  !
  ! Inputs:  
  !		class		- property list class to register 
  !                                 permanent property within
  !		name 		- name of property to register
  !               size            - size of property in bytes
  !		value		- default value for property in newly 
  !                                 created property lists
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 10, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pregister_integer(class, name, size, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pregister_integer
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: class   ! Property list class identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to register
    INTEGER(SIZE_T), INTENT(IN) :: size   ! Size of the property value	
    INTEGER,   INTENT(IN) :: value        ! Property value
    INTEGER, INTENT(OUT) :: hdferr        ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pregister_integer_c(class, name, name_len, size, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PREGISTER_INTEGER_C'::h5pregister_integer_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: class
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER(SIZE_T), INTENT(IN) :: size 
         INTEGER, INTENT(IN) :: value
       END FUNCTION h5pregister_integer_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pregister_integer_c(class, name , name_len, size, value)
  END SUBROUTINE h5pregister_integer

  !----------------------------------------------------------------------
  ! Name:		h5pregister_real
  !
  ! Purpose: 	Registers a permanent property with a property list class.
  !
  ! Inputs:  
  !		class		- property list class to register 
  !                                 permanent property within
  !		name 		- name of property to register
  !               size            - size of property in bytes
  !		value		- default value for property in newly 
  !                                 created property lists
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 10, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pregister_real(class, name, size, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pregister_real
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: class   ! Property list class identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to register
    INTEGER(SIZE_T), INTENT(IN) :: size   ! size of the property value	
    REAL,   INTENT(IN) :: value           ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pregister_real_c(class, name, name_len, size, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PREGISTER_REAL_C'::h5pregister_real_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: class
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER(SIZE_T), INTENT(IN) :: size 
         REAL, INTENT(IN) :: value
       END FUNCTION h5pregister_real_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pregister_real_c(class, name , name_len, size, value)
  END SUBROUTINE h5pregister_real

  !----------------------------------------------------------------------
  ! Name:		h5pregister_double
  !
  ! Purpose: 	Registers a permanent property with a property list class.
  !
  ! Inputs:  
  !		class		- property list class to register 
  !                                 permanent property within
  !		name 		- name of property to register
  !               size            - size of property in bytes
  !		value		- default value for property in newly 
  !                                 created property lists
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 10, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pregister_double(class, name, size, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pregister_double
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: class   ! Property list class identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to register
    INTEGER(SIZE_T), INTENT(IN) :: size  ! size of the property value	
    DOUBLE PRECISION,   INTENT(IN) :: value        ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pregister_double_c(class, name, name_len, size, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PREGISTER_DOUBLE_C'::h5pregister_double_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: class
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER(SIZE_T), INTENT(IN) :: size 
         DOUBLE PRECISION, INTENT(IN) :: value
       END FUNCTION h5pregister_double_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pregister_double_c(class, name , name_len, size, value)
  END SUBROUTINE h5pregister_double

  !----------------------------------------------------------------------
  ! Name:		h5pregister_char
  !
  ! Purpose: 	Registers a permanent property with a property list class.
  !
  ! Inputs:  
  !		class		- property list class to register 
  !                                 permanent property within
  !		name 		- name of property to register
  !               size            - size of property in bytes
  !		value		- default value for property in newly 
  !                                 created property lists
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 10, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pregister_char(class, name, size, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pregister_char
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: class   ! Property list class identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to register
    INTEGER(SIZE_T), INTENT(IN) :: size  ! size of the property value	
    CHARACTER(LEN=*),   INTENT(IN) :: value        ! Property value
    INTEGER, INTENT(OUT) :: hdferr  ! Error code
    INTEGER :: name_len
    INTEGER :: value_len

    INTERFACE
       INTEGER FUNCTION h5pregisterc_c(class, name, name_len, size, value, &
            value_len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PREGISTERC_C'::h5pregisterc_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         !DEC$ATTRIBUTES reference :: value
         INTEGER(HID_T), INTENT(IN) :: class
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER(SIZE_T), INTENT(IN) :: size 
         CHARACTER(LEN=*), INTENT(IN) :: value
         INTEGER, INTENT(IN)          :: value_len
       END FUNCTION h5pregisterc_c
    END INTERFACE

    name_len = LEN(name)
    value_len = LEN(value)
    hdferr = h5pregisterc_c(class, name , name_len, size, value, value_len)
  END SUBROUTINE h5pregister_char

  !----------------------------------------------------------------------
  ! Name:		h5pinsert_integer
  !
  ! Purpose: 	Registers a temporary property with a property list class.
  !
  ! Inputs:  
  !		plist		- property list identifier
  !		name 		- name of property to insert
  !               size            - size of property in bytes
  !		value		- initial value for the property 
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 10, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pinsert_integer(plist, name, size, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pinsert_integer
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: plist   ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to insert 
    INTEGER(SIZE_T), INTENT(IN) :: size   ! Size of the property value	
    INTEGER,   INTENT(IN) :: value        ! Property value
    INTEGER, INTENT(OUT) :: hdferr        ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pinsert_integer_c(plist, name, name_len, size, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PINSERT_INTEGER_C'::h5pinsert_integer_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: plist
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER(SIZE_T), INTENT(IN) :: size 
         INTEGER, INTENT(IN) :: value
       END FUNCTION h5pinsert_integer_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pinsert_integer_c(plist, name , name_len, size, value)
  END SUBROUTINE h5pinsert_integer

  !----------------------------------------------------------------------
  ! Name:		h5pinsert_real
  !
  ! Purpose: 	Registers a temporary property with a property list class.
  !
  ! Inputs:  
  !		plist		- property list identifier
  !                                 permanent property within
  !		name 		- name of property to insert
  !               size            - size of property in bytes
  !		value		- initial value for the property 
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 10, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pinsert_real(plist, name, size, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pinsert_real
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: plist   ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to insert 
    INTEGER(SIZE_T), INTENT(IN) :: size   ! Size of the property value	
    REAL,   INTENT(IN) :: value           ! Property value
    INTEGER, INTENT(OUT) :: hdferr        ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pinsert_real_c(plist, name, name_len, size, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PINSERT_REAL_C'::h5pinsert_real_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: plist
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER(SIZE_T), INTENT(IN) :: size 
         REAL, INTENT(IN) :: value
       END FUNCTION h5pinsert_real_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pinsert_real_c(plist, name , name_len, size, value)
  END SUBROUTINE h5pinsert_real

  !----------------------------------------------------------------------
  ! Name:		h5pinsert_double
  !
  ! Purpose: 	Registers a temporary property with a property list class.
  !
  ! Inputs:  
  !		plist		- property list identifier
  !                                 permanent property within
  !		name 		- name of property to insert
  !               size            - size of property in bytes
  !		value		- initial value for the property 
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 10, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pinsert_double(plist, name, size, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pinsert_double
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: plist   ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of property to insert 
    INTEGER(SIZE_T), INTENT(IN) :: size   ! Size of the property value	
    DOUBLE PRECISION, INTENT(IN) :: value ! Property value
    INTEGER, INTENT(OUT) :: hdferr        ! Error code
    INTEGER :: name_len

    INTERFACE
       INTEGER FUNCTION h5pinsert_double_c(plist, name, name_len, size, value)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PINSERT_DOUBLE_C'::h5pinsert_double_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: plist
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER(SIZE_T), INTENT(IN) :: size 
         DOUBLE PRECISION, INTENT(IN) :: value
       END FUNCTION h5pinsert_double_c
    END INTERFACE

    name_len = LEN(name)
    hdferr = h5pinsert_double_c(plist, name , name_len, size, value)
  END SUBROUTINE h5pinsert_double

  !----------------------------------------------------------------------
  ! Name:		h5pinsert_char
  !
  ! Purpose: 	Registers a temporary property with a property list class.
  !
  ! Inputs:  
  !		plist		- property list identifier
  !                                 permanent property within
  !		name 		- name of property to insert
  !               size            - size of property in bytes
  !		value		- initial value for the property 
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !				NONE
  !
  ! Programmer:	Elena Pourmal
  !	        October 10, 2002	
  !
  ! Modifications: 	
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5pinsert_char(plist, name, size, value, hdferr) 
    !
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5pinsert_char
    !DEC$endif
    !
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: plist      ! Property list identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name     ! Name of property to insert 
    INTEGER(SIZE_T), INTENT(IN) :: size      ! Size of property value	
    CHARACTER(LEN=*),   INTENT(IN) :: value  ! Property value
    INTEGER, INTENT(OUT) :: hdferr           ! Error code
    INTEGER :: name_len
    INTEGER :: value_len

    INTERFACE
       INTEGER FUNCTION h5pinsertc_c(plist, name, name_len, size, value, value_len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5PINSERTC_C'::h5pinsertc_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         !DEC$ATTRIBUTES reference :: value
         INTEGER(HID_T), INTENT(IN) :: plist
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER, INTENT(IN)         :: name_len
         INTEGER(SIZE_T), INTENT(IN) :: size 
         CHARACTER(LEN=*), INTENT(IN) :: value
         INTEGER, INTENT(IN)         :: value_len
       END FUNCTION h5pinsertc_c
    END INTERFACE

    name_len = LEN(name)
    value_len = LEN(value)
    hdferr = h5pinsertc_c(plist, name , name_len, size, value, value_len)
  END SUBROUTINE h5pinsert_char

END MODULE H5P_F03

