      SUBROUTINE h5init_fortran_f(error)
        USE H5GLOBAL

        IMPLICIT NONE
        INTEGER, INTENT(OUT) :: error
        INTEGER :: error_1, error_2
        INTEGER, EXTERNAL :: h5init_types_c
        INTEGER, EXTERNAL :: h5init_flags_c
        error_1 = h5init_types_c(predef_types, floating_types, integer_types)
        error_1 = h5init_flags_c(H5D_flags, &
                                H5E_flags, &
                                H5F_flags, &
                                H5FD_flags, &
                                H5G_flags, &
                                H5I_flags, &
                                H5P_flags, &
                                H5R_flags, &
                                H5S_flags, &
                                H5T_flags  )
        error = error_1 + error_2

      END SUBROUTINE h5init_fortran_f

      SUBROUTINE h5close_fortran_f(error)
        USE H5GLOBAL

        IMPLICIT NONE
        INTEGER, INTENT(OUT) :: error
        INTEGER, EXTERNAL :: h5close_types_c
        error = h5close_types_c(predef_types, PREDEF_TYPES_LEN, &
                                floating_types, FLOATING_TYPES_LEN, &
                                integer_types, INTEGER_TYPES_LEN )

      END SUBROUTINE h5close_fortran_f
        
