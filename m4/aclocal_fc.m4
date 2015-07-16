dnl -------------------------------------------------------------------------
dnl -------------------------------------------------------------------------
dnl
dnl Copyright by the Board of Trustees of the University of Illinois.
dnl All rights reserved.
dnl
dnl This file is part of HDF5.  The full HDF5 copyright notice, including
dnl terms governing use, modification, and redistribution, is contained in
dnl the files COPYING and Copyright.html.  COPYING can be found at the root
dnl of the source code distribution tree; Copyright.html can be found at the
dnl root level of an installed copy of the electronic HDF5 document set and
dnl is linked from the top-level documents page.  It can also be found at
dnl http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have
dnl access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu.
dnl
dnl -------------------------------------------------------------------------
dnl -------------------------------------------------------------------------

dnl *********************************
dnl PURPOSE
dnl  Contains Macros for HDF5 Fortran
dnl *********************************
dnl
dnl Special characteristics that have no autoconf counterpart but that
dnl we need as part of the C++ support.  To distinquish these, they
dnl have a [PAC] prefix.
dnl
dnl -------------------------------------------------------------------------
dnl
dnl PAC_FC_SEARCH_LIST - expands to a whitespace separated list of modern
dnl fortran compilers for use with AC_PROG_FC that is more suitable for HPC
dnl software packages
AC_DEFUN([PAC_FC_SEARCH_LIST],
         [gfortran ifort pgf90 pathf90 pathf95 xlf90 xlf95 xlf2003 f90 epcf90 f95 fort lf95 g95 ifc efc gfc])
dnl 
dnl PAC_PROG_FC([COMPILERS])
dnl
dnl COMPILERS is a space separated list of Fortran compilers to search for.
dnl
dnl Compilers are ordered by
dnl  1. F90, F95, F2003
dnl  2. Good/tested native compilers, bad/untested native compilers
dnl  3. Wrappers around f2c go last.
dnl
dnl frt is the Fujitsu Fortran compiler.
dnl pgf90 are the Portland Group F90 compilers.
dnl xlf/xlf90/xlf95/xlf2003 are IBM (AIX) F90/F95/F2003 compilers.
dnl lf95 is the Lahey-Fujitsu compiler.
dnl fl32 is the Microsoft Fortran "PowerStation" compiler.
dnl epcf90 is the "Edinburgh Portable Compiler" F90.
dnl fort is the Compaq Fortran 90 (now 95) compiler for Tru64 and Linux/Alpha.
dnl pathf90 is the Pathscale Fortran 90 compiler
dnl ifort is another name for the Intel f90 compiler
dnl efc - An older Intel compiler (?)
dnl ifc - An older Intel compiler
dnl fc  - A compiler on some unknown system.  This has been removed because
dnl       it may also be the name of a command for something other than
dnl       the Fortran compiler (e.g., fc=file system check!)
dnl gfortran - The GNU Fortran compiler (not the same as g95) 
dnl gfc - An alias for gfortran recommended in cygwin installations
dnl NOTE: this macro suffers from a basically intractable "expanded before it
dnl was required" problem when libtool is also used 
dnl [1] MPICH.org
dnl

dnl See if the fortran compiler supports the intrinsic module "ISO_FORTRAN_ENV"

AC_DEFUN([PAC_PROG_FC_ISO_FORTRAN_ENV],[
  HAVE_ISO_FORTRAN_ENV="no"
  AC_MSG_CHECKING([if Fortran compiler supports intrinsic module ISO_FORTRAN_ENV])
  AC_LINK_IFELSE([AC_LANG_SOURCE([ 
   PROGRAM main
     USE, INTRINSIC :: ISO_FORTRAN_ENV
   END PROGRAM
  ])],[AC_MSG_RESULT([yes])
     	HAVE_ISO_FORTRAN_ENV="yes"],
      [AC_MSG_RESULT([no])])
])

dnl See if the fortran compiler supports the intrinsic function "SIZEOF"

AC_DEFUN([PAC_PROG_FC_SIZEOF],[
  HAVE_SIZEOF_FORTRAN="no"
  AC_MSG_CHECKING([if Fortran compiler supports intrinsic SIZEOF])
  AC_LINK_IFELSE([AC_LANG_SOURCE([ 
   PROGRAM main
     i = sizeof(x)
   END PROGRAM
  ])],[AC_MSG_RESULT([yes])
     	HAVE_SIZEOF_FORTRAN="yes"],
      [AC_MSG_RESULT([no])])
])

dnl See if the fortran compiler supports the intrinsic function "C_SIZEOF"

AC_DEFUN([PAC_PROG_FC_C_SIZEOF],[
  HAVE_C_SIZEOF_FORTRAN="no"
  AC_MSG_CHECKING([if Fortran compiler supports intrinsic C_SIZEOF])
  AC_LINK_IFELSE([AC_LANG_SOURCE([ 
   PROGRAM main
     USE ISO_C_BINDING
     INTEGER(C_INT) :: a
     INTEGER(C_SIZE_T) :: result
     result = C_SIZEOF(a)
   END PROGRAM
  ])], [AC_MSG_RESULT([yes])
     	HAVE_C_SIZEOF_FORTRAN="yes"],
     [AC_MSG_RESULT([no])])
])

dnl See if the fortran compiler supports the intrinsic function "STORAGE_SIZE"

AC_DEFUN([PAC_PROG_FC_STORAGE_SIZE],[
  HAVE_STORAGE_SIZE_FORTRAN="no"
  AC_MSG_CHECKING([if Fortran compiler supports intrinsic STORAGE_SIZE])
  AC_LINK_IFELSE([AC_LANG_SOURCE([
   PROGRAM main
     INTEGER :: a
     INTEGER :: result
     result = STORAGE_SIZE(a)
   END PROGRAM
  ])], [AC_MSG_RESULT([yes])
     	HAVE_STORAGE_SIZE_FORTRAN="yes"],
     [AC_MSG_RESULT([no])])

])

dnl Check to see C_LONG_DOUBLE is available

AC_DEFUN([PAC_PROG_FC_HAVE_C_LONG_DOUBLE],[
  HAVE_C_LONG_DOUBLE_FORTRAN="no"
  AC_MSG_CHECKING([if Fortran compiler supports intrinsic C_LONG_DOUBLE])
  AC_LINK_IFELSE([AC_LANG_SOURCE([
   PROGRAM main
     USE ISO_C_BINDING
     REAL(KIND=C_LONG_DOUBLE) :: d
   END PROGRAM
  ])], [AC_MSG_RESULT([yes])
     	HAVE_C_LONG_DOUBLE_FORTRAN="yes"],
     [AC_MSG_RESULT([no])])
])

dnl Check if C_LONG_DOUBLE is different from C_DOUBLE

if  test "X$FORTRAN_HAVE_C_LONG_DOUBLE" = "Xyes"; then
AC_DEFUN([PAC_PROG_FC_C_LONG_DOUBLE_EQ_C_DOUBLE],[
  C_LONG_DOUBLE_IS_UNIQUE_FORTRAN="no"	
  AC_MSG_CHECKING([if Fortran C_LONG_DOUBLE is different from C_DOUBLE])
  
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([
     MODULE type_mod
       USE ISO_C_BINDING
       INTERFACE h5t	
         MODULE PROCEDURE h5t_c_double
         MODULE PROCEDURE h5t_c_long_double
       END INTERFACE
     CONTAINS
       SUBROUTINE h5t_c_double(r)
         REAL(KIND=C_DOUBLE) :: r
       END SUBROUTINE h5t_c_double
       SUBROUTINE h5t_c_long_double(d)
         REAL(KIND=C_LONG_DOUBLE) :: d
       END SUBROUTINE h5t_c_long_double
     END MODULE type_mod
     PROGRAM main
       USE ISO_C_BINDING
       USE type_mod
       REAL(KIND=C_DOUBLE)      :: r
       REAL(KIND=C_LONG_DOUBLE) :: d
       CALL h5t(r)
       CALL h5t(d)
     END PROGRAM main
    ])], [AC_MSG_RESULT([yes]) 
            C_LONG_DOUBLE_IS_UNIQUE_FORTRAN="yes"], 
         [AC_MSG_RESULT([no])])
])
fi

dnl Checking if the compiler supports the required Fortran 2003 features and
dnl disable Fortran 2003 if it does not.

AC_DEFUN([PAC_PROG_FC_HAVE_F2003_REQUIREMENTS],[
   AC_MSG_CHECKING([if Fortran compiler version compatible with Fortran 2003 HDF])
dnl --------------------------------------------------------------------
dnl Default for FORTRAN 2003 compliant compilers
dnl
    HAVE_F2003_REQUIREMENTS="no"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([],[

        USE iso_c_binding
        IMPLICIT NONE
        TYPE(C_PTR) :: ptr
        TYPE(C_FUNPTR) :: funptr
        CHARACTER(LEN=80, KIND=c_char), TARGET :: ichr

        ptr = C_LOC(ichr(1:1))

        ])],[AC_MSG_RESULT([yes])
        HAVE_F2003_REQUIREMENTS=[yes]], 
      [AC_MSG_RESULT([no])])
])

dnl -------------------------------------------------------------------------
dnl AC_F9X_MODS()
dnl
dnl	Check how F9X handles modules. This macro also checks which
dnl	command-line option to use to include the module once it's built.
dnl
AC_DEFUN([AC_F9X_MODS],
[AC_MSG_CHECKING(what $FC does with modules)
AC_LANG_PUSH(Fortran)

test -d conftestdir || mkdir conftestdir
cd conftestdir
rm -rf *

cat >conftest.$ac_ext <<EOF
      module module
         integer foo
      end module module
EOF

eval $ac_compile
modfiles=""
F9XMODEXT=""

for f in conftest.o module.mod MODULE.mod module.M MODULE.M; do
  if test -f "$f" ; then
    modfiles="$f"

    case "$f" in
      *.o)   F9XMODEXT="o" ;;
      *.mod) F9XMODEXT="mod" ;;
      *.M)   F9XMODEXT="M" ;;
    esac
  fi
done

echo $modfiles 6>&1
if test "$modfiles" = file.o; then
  echo $ac_n "checking whether $FC -em is saner""... $ac_c" 1>&6
  OLD_FCFLAGS=$FCFLAGS
  FCFLAGS="$FCFLAGS -em"
  eval $ac_compile
  modfiles=""
  for f in file.o module.mod MODULE.mod module.M MODULE.M; do
    test -f $f && modfiles="$f"
  done
  if test "$modfiles" = "file.o"; then
    FCFLAGS=$OLD_FCFLAGS
    echo no 6>&1
  else
    echo yes 6>&1
  fi
fi
cd ..

AC_MSG_CHECKING(how $FC finds modules)

for flag in "-I" "-M" "-p"; do
  cat >conftest.$ac_ext <<EOF
      program conftest
          use module
      end program conftest
EOF

  ac_compile='${FC-f90} $FCFLAGS ${flag}conftestdir -c conftest.$ac_ext 1>&AS_MESSAGE_LOG_FD'

  if AC_TRY_EVAL(ac_compile); then
    F9XMODFLAG=$flag
    break
  fi
done

if test -n "$F9XMODFLAG"; then
  echo $F9XMODFLAG 1>&6
  FCFLAGS="$F9XMODFLAG. $FCFLAGS"
else
  echo unknown 1>&6
fi
AC_SUBST(F9XMODFLAG)
AC_SUBST(F9XMODEXT)
rm -rf conftest*
AC_LANG_POP(Fortran)
])

dnl ----------------------
dnl Parallel Test Programs
dnl ----------------------

dnl Try link a simple MPI program.

AC_DEFUN([PAC_PROG_FC_MPI_CHECK],[

dnl   Change to the Fortran 90 language
      AC_LANG_PUSH(Fortran)

dnl   Try link a simple MPI program.
      AC_MSG_CHECKING([whether a simple MPI-IO Fortran program can be linked])
      AC_LINK_IFELSE([ 
          PROGRAM main
          INCLUDE 'mpif.h'
          INTEGER :: comm, amode, info, fh, ierror
          CHARACTER(LEN=1) :: filename 
          CALL MPI_File_open( comm, filename, amode, info, fh, ierror)
          END],
	  [AC_MSG_RESULT([yes])],
	  [AC_MSG_RESULT([no])
	   AC_MSG_ERROR([unable to link a simple MPI-IO Fortran program])])

dnl   Change to the C language
      AC_LANG_POP(Fortran)
])

dnl ------------------------------------------------------
dnl Determine the available KINDs for REALs and INTEGERs
dnl ------------------------------------------------------
dnl
dnl This is a runtime test.
dnl
AC_DEFUN([PAC_FC_AVAIL_KINDS],[
AC_LANG_PUSH([Fortran])
rm -f pac_fconftest.out

AC_RUN_IFELSE([
    AC_LANG_SOURCE([
    PROGRAM main
        IMPLICIT NONE
        INTEGER :: ik, k, lastkind, max_decimal_prec
	INTEGER :: num_rkinds, num_ikinds
        num_ikinds = 0
        lastkind=SELECTED_INT_KIND(1)
        OPEN(8, FILE='pac_fconftest.out', form='formatted')
        ! Find integer KINDs
        DO ik=2,36
             k = SELECTED_INT_KIND(ik)
             IF (k .NE. lastkind) THEN
	          num_ikinds = num_ikinds + 1	
                  WRITE(8,'(I0)',ADVANCE='NO') lastkind
                  lastkind = k
             	  IF(k.GT.0) WRITE(8,'(A)',ADVANCE='NO') ','	
             ENDIF
             IF (k .LE. 0) EXIT
        ENDDO
	IF (lastkind.NE.-1) THEN
	   num_ikinds = num_ikinds + 1	
           WRITE(8,'(I0)') lastkind
	ELSE
           WRITE(8,'()')
        ENDIF
        ! Find real KINDs
        num_rkinds = 0
        lastkind=SELECTED_REAL_KIND(1)
	max_decimal_prec = 1
        DO ik=2,36
             k = SELECTED_REAL_KIND(ik)
             IF (k .NE. lastkind) THEN
                  num_rkinds = num_rkinds + 1
                  WRITE(8,'(I0)',ADVANCE='NO') lastkind
                  lastkind = k
             	  IF(k.GT.0) WRITE(8,'(A)',ADVANCE='NO') ','
	          max_decimal_prec = ik
             ENDIF
             IF (k .LE. 0) EXIT
        ENDDO
        IF (lastkind.NE.-1)THEN
	    num_rkinds = num_rkinds + 1
            WRITE(8,'(I0)') lastkind
	ELSE
           WRITE(8,'()')
        ENDIF
	WRITE(8,'(I0)') max_decimal_prec
	WRITE(8,'(I0)') num_ikinds
	WRITE(8,'(I0)') num_rkinds
    END
    ])
],[
    if test -s pac_fconftest.out ; then
	
     dnl The output from the above program will be:
     dnl    -- LINE 1 --  valid integer kinds (comma seperated list)
     dnl    -- LINE 2 --  valid real kinds (comma seperated list)
     dnl    -- LINE 3 --  max decimal precision for reals
     dnl    -- LINE 4 --  number of valid integer kinds
     dnl    -- LINE 5 --  number of valid real kinds

        pac_validIntKinds="`perl -ne '$. == 1 && print && exit' pac_fconftest.out`"
	pac_validRealKinds="`perl -ne '$. == 2 && print && exit' pac_fconftest.out`"
        PAC_FC_MAX_REAL_PRECISION="`perl -ne '$. == 3 && print && exit' pac_fconftest.out`"
        AC_DEFINE_UNQUOTED([PAC_FC_MAX_REAL_PRECISION], $PAC_FC_MAX_REAL_PRECISION, [Define Fortran Maximum Real Decimal Precision])

        PAC_FC_ALL_INTEGER_KINDS="{`echo $pac_validIntKinds`}"
        PAC_FC_ALL_REAL_KINDS="{`echo $pac_validRealKinds`}"

	H5CONFIG_F_NUM_IKIND="INTEGER, PARAMETER :: num_ikinds = `perl -ne '$. == 4 && print && exit' pac_fconftest.out`"
	H5CONFIG_F_IKIND="INTEGER, DIMENSION(1:num_ikinds) :: ikind = (/`echo $pac_validIntKinds`/)"
	H5CONFIG_F_NUM_RKIND="INTEGER, PARAMETER :: num_rkinds = `perl -ne '$. == 5 && print && exit' pac_fconftest.out`"
	H5CONFIG_F_RKIND="INTEGER, DIMENSION(1:num_rkinds) :: rkind = (/`echo $pac_validRealKinds`/)"

	AC_DEFINE_UNQUOTED([H5CONFIG_F_NUM_RKIND], $H5CONFIG_F_NUM_RKIND, [Define number of valid Fortran REAL KINDs])
	AC_DEFINE_UNQUOTED([H5CONFIG_F_NUM_IKIND], $H5CONFIG_F_NUM_IKIND, [Define number of valid Fortran INTEGER KINDs])
	AC_DEFINE_UNQUOTED([H5CONFIG_F_RKIND], $H5CONFIG_F_RKIND, [Define valid Fortran REAL KINDs])
	AC_DEFINE_UNQUOTED([H5CONFIG_F_IKIND], $H5CONFIG_F_IKIND, [Define valid Fortran INTEGER KINDs])

        AC_MSG_CHECKING([for Fortran INTEGER KINDs])
        AC_MSG_RESULT([$PAC_FC_ALL_INTEGER_KINDS])
	AC_MSG_CHECKING([for Fortran REAL KINDs])
	AC_MSG_RESULT([$PAC_FC_ALL_REAL_KINDS])
	AC_MSG_CHECKING([for Fortran REALs maximum decimal precision])
	AC_MSG_RESULT([$PAC_FC_MAX_REAL_PRECISION])
    else
        AC_MSG_RESULT([Error])
        AC_MSG_WARN([No output from test program!])
    fi
    rm -f pac_fconftest.out
],[
    AC_MSG_RESULT([Error])
    AC_MSG_WARN([Failed to run program to determine available KINDs])
],[])

AC_LANG_POP([Fortran])
])
AC_DEFUN([PAC_FC_SIZEOF_INT_KINDS],[
AC_REQUIRE([PAC_FC_AVAIL_KINDS])
AC_MSG_CHECKING([sizeof of available INTEGER KINDs])
AC_LANG_PUSH([Fortran])
pack_int_sizeof=""
rm -f pac_fconftest.out

for kind in `echo $pac_validIntKinds | perl -pe 's/,/ /g'`; do
  AC_LANG_CONFTEST([
      AC_LANG_SOURCE([
                PROGRAM main
                USE ISO_C_BINDING
                IMPLICIT NONE
                INTEGER (KIND=$kind) a
                OPEN(8, FILE='pac_fconftest.out', FORM='formatted')
                WRITE(8,'(I0)') $FC_SIZEOF_A
                CLOSE(8)
                END
            ])
        ])
        AC_RUN_IFELSE([],[
            if test -s pac_fconftest.out ; then
                sizes="`cat pac_fconftest.out`"
                pack_int_sizeof="$pack_int_sizeof $sizes,"
            else
                AC_MSG_WARN([No output from test program!])
            fi
            rm -f pac_fconftest.out
        ],[
            AC_MSG_WARN([Fortran program fails to build or run!])
        ],[
            pack_int_sizeof="$2"
        ])
done
PAC_FC_ALL_INTEGER_KINDS_SIZEOF="{`echo $pack_int_sizeof | perl -pe 's/,$//' | perl -pe 's/ //g'`}"
AC_MSG_RESULT([$PAC_FC_ALL_INTEGER_KINDS_SIZEOF])
AC_LANG_POP([Fortran])
])

AC_DEFUN([PAC_FC_SIZEOF_REAL_KINDS],[
AC_REQUIRE([PAC_FC_AVAIL_KINDS])
AC_MSG_CHECKING([sizeof of available REAL KINDs])
AC_LANG_PUSH([Fortran])
pack_real_sizeof=""
rm -f pac_fconftest.out
for kind in `echo  $pac_validRealKinds| perl -pe 's/,/ /g'`; do
  AC_LANG_CONFTEST([
      AC_LANG_SOURCE([
                PROGRAM main
                USE ISO_C_BINDING
                IMPLICIT NONE
                REAL (KIND=$kind) :: a
                OPEN(8, FILE='pac_fconftest.out', FORM='formatted')
                WRITE(8,'(I0)') $FC_SIZEOF_A
                CLOSE(8)
                END
            ])
        ])
        AC_RUN_IFELSE([],[
            if test -s pac_fconftest.out ; then
                sizes="`cat pac_fconftest.out`"
                pack_real_sizeof="$pack_real_sizeof $sizes,"
            else
                AC_MSG_WARN([No output from test program!])
            fi
            rm -f pac_fconftest.out
        ],[
            AC_MSG_WARN([Fortran program fails to build or run!])
        ],[
            pack_real_sizeof="$2"
        ])
done
PAC_FC_ALL_REAL_KINDS_SIZEOF="{`echo $pack_real_sizeof | perl -pe 's/,$//' | perl -pe 's/ //g'`}"
AC_MSG_RESULT([$PAC_FC_ALL_REAL_KINDS_SIZEOF])
AC_LANG_POP([Fortran])
])

AC_DEFUN([PAC_FC_NATIVE_INTEGER],[
AC_REQUIRE([PAC_FC_AVAIL_KINDS])
AC_MSG_CHECKING([sizeof of native KINDS])
AC_LANG_PUSH([Fortran])
pack_int_sizeof=""
rm -f pac_fconftest.out
  AC_LANG_CONFTEST([
      AC_LANG_SOURCE([
                PROGRAM main
                USE ISO_C_BINDING
                IMPLICIT NONE
                INTEGER a
                REAL b
                DOUBLE PRECISION c
                OPEN(8, FILE='pac_fconftest.out', FORM='formatted')
                WRITE(8,*) $FC_SIZEOF_A
	        WRITE(8,*) kind(a)
	        WRITE(8,*) $FC_SIZEOF_B
	        WRITE(8,*) kind(b)
                WRITE(8,*) $FC_SIZEOF_C
                WRITE(8,*) kind(c)
                CLOSE(8)
                END
            ])
        ])
        AC_RUN_IFELSE([],[
            if test -s pac_fconftest.out ; then
                PAC_FORTRAN_NATIVE_INTEGER_KIND="`perl -ne '$. == 1 && print && exit' pac_fconftest.out`"
                PAC_FORTRAN_NATIVE_INTEGER_SIZEOF="`perl -ne '$. == 2 && print && exit' pac_fconftest.out`"
                PAC_FORTRAN_NATIVE_REAL_KIND="`perl -ne '$. == 3 && print && exit' pac_fconftest.out`"
                PAC_FORTRAN_NATIVE_REAL_SIZEOF="`perl -ne '$. == 4 && print && exit' pac_fconftest.out`"
                PAC_FORTRAN_NATIVE_DOUBLE_KIND="`perl -ne '$. == 5 && print && exit' pac_fconftest.out`"
                PAC_FORTRAN_NATIVE_DOUBLE_SIZEOF="`perl -ne '$. == 6 && print && exit' pac_fconftest.out`"
            else
                AC_MSG_WARN([No output from test program!])
            fi
            rm -f pac_fconftest.out
        ],[
            AC_MSG_WARN([Fortran program fails to build or run!])
        ],[
            pack_int_sizeof="$2"
        ])
AC_MSG_RESULT([$pack_int_sizeof])
AC_LANG_POP([Fortran])
])

AC_DEFUN([PAC_LDBL_DIG],[
AC_MSG_CHECKING([maximum decimal precision for C])
rm -f pac_Cconftest.out
  AC_LANG_CONFTEST([
      AC_LANG_PROGRAM([
                #include <float.h>
                #include <stdio.h>
                #define CHECK_FLOAT128 $ac_cv_sizeof___float128
                #if CHECK_FLOAT128!=0
                # if $HAVE_QUADMATH!=0
                #include <quadmath.h>
                # endif
                # ifdef FLT128_DIG
                #define C_FLT128_DIG FLT128_DIG
                # else
                #define C_FLT128_DIG 0
                # endif
                #else
                #define C_FLT128_DIG 0
                #endif
                #if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
                #define C_LDBL_DIG DECIMAL_DIG 
                #else
                #define C_LDBL_DIG LDBL_DIG
                #endif
                ],[[
                  FILE * pFile;
                  pFile = fopen("pac_Cconftest.out","w");
                  fprintf(pFile, "%d\n%d\n", C_LDBL_DIG, C_FLT128_DIG);
                ]])
        ])
        AC_RUN_IFELSE([],[
            if test -s pac_Cconftest.out ; then
	        LDBL_DIG="`perl -ne '$. == 1 && print && exit'  pac_Cconftest.out`" 
	        FLT128_DIG="`perl -ne '$. == 2 && print && exit' pac_Cconftest.out`"
            else
                AC_MSG_WARN([No output from test program!])
            fi
            rm -f pac_Cconftest.out
        ],[
            AC_MSG_ERROR([C program fails to build or run!])
        ],[])
])


