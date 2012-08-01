#! /bin/sh
#
# Copyright by The HDF Group.
# Copyright by the Board of Trustees of the University of Illinois.
# All rights reserved.
#
# This file is part of HDF5.  The full HDF5 copyright notice, including
# terms governing use, modification, and redistribution, is contained in
# the files COPYING and Copyright.html.  COPYING can be found at the root
# of the source code distribution tree; Copyright.html can be found at the
# root level of an installed copy of the electronic HDF5 document set and
# is linked from the top-level documents page.  It can also be found at
# http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have
# access to either file, you may request a copy from help@hdfgroup.org.
#
# Tests for the h5diff tool
#
# Modification:
#   Albert Cheng, 2005/08/17
#   Added the SKIP feature.
#   Albert Cheng, 2005/2/3
#   Added -p option for parallel h5diff tests.
#   Pedro Vicente Nunes:
#    10/25/2005: Added test #9
#    11/27/2006: Added test #10, #11
#   Jonathan Kim:
#    Improved to use single line
#    Improved to check exit code (only serial mode, not necessary for parallel)
#    Added test 400 - 425  (links with --follow-symlinks option)
#    Added test 450 - 459  (dangling links)



TESTNAME=h5diff
EXIT_SUCCESS=0
EXIT_FAILURE=1

H5DIFF=h5diff               # The tool name
H5DIFF_BIN=`pwd`/$H5DIFF    # The path of the tool binary

CMP='cmp -s'
DIFF='diff -c'
CP='cp'
DIRNAME='dirname'
LS='ls'
AWK='awk'

nerrors=0
verbose=yes
h5haveexitcode=yes	    # default is yes
pmode=			    # default to run h5diff tests
mydomainname=`domainname 2>/dev/null`

# The build (current) directory might be different than the source directory.
if test -z "$srcdir"; then
   srcdir=.
fi

# source dirs
SRC_TOOLS="$srcdir/.."
SRC_TOOLS_TESTFILES="$SRC_TOOLS/testfiles"
# testfiles source dirs for tools
SRC_H5LS_TESTFILES="$SRC_TOOLS_TESTFILES"
SRC_H5DUMP_TESTFILES="$SRC_TOOLS_TESTFILES"
SRC_H5DIFF_TESTFILES="$SRC_TOOLS/h5diff/testfiles"
SRC_H5COPY_TESTFILES="$SRC_TOOLS/h5copy/testfiles"
SRC_H5REPACK_TESTFILES="$SRC_TOOLS/h5repack/testfiles"
SRC_H5JAM_TESTFILES="$SRC_TOOLS/h5jam/testfiles"
SRC_H5STAT_TESTFILES="$SRC_TOOLS/h5stat/testfiles"
SRC_H5IMPORT_TESTFILES="$SRC_TOOLS/h5import/testfiles"

TESTDIR=./testfiles
test -d $TESTDIR || mkdir $TESTDIR

######################################################################
# test files
# --------------------------------------------------------------------
# All the test files copy from source directory to test directory
# NOTE: Keep this framework to add/remove test files.
#       Any test files from other tools can be used in this framework.
#       This list are also used for checking exist.
#       Comment '#' without space can be used.
# --------------------------------------------------------------------
LIST_HDF5_TEST_FILES="
$SRC_H5DIFF_TESTFILES/h5diff_basic1.h5
$SRC_H5DIFF_TESTFILES/h5diff_basic2.h5
$SRC_H5DIFF_TESTFILES/h5diff_types.h5
$SRC_H5DIFF_TESTFILES/h5diff_dtypes.h5
$SRC_H5DIFF_TESTFILES/h5diff_attr1.h5
$SRC_H5DIFF_TESTFILES/h5diff_attr2.h5
$SRC_H5DIFF_TESTFILES/h5diff_dset1.h5
$SRC_H5DIFF_TESTFILES/h5diff_dset2.h5
$SRC_H5DIFF_TESTFILES/h5diff_hyper1.h5
$SRC_H5DIFF_TESTFILES/h5diff_hyper2.h5
$SRC_H5DIFF_TESTFILES/h5diff_empty.h5
$SRC_H5DIFF_TESTFILES/h5diff_links.h5
$SRC_H5DIFF_TESTFILES/h5diff_softlinks.h5
$SRC_H5DIFF_TESTFILES/h5diff_linked_softlink.h5
$SRC_H5DIFF_TESTFILES/h5diff_extlink_src.h5
$SRC_H5DIFF_TESTFILES/h5diff_extlink_trg.h5
$SRC_H5DIFF_TESTFILES/h5diff_ext2softlink_src.h5
$SRC_H5DIFF_TESTFILES/h5diff_ext2softlink_trg.h5
$SRC_H5DIFF_TESTFILES/h5diff_dset_zero_dim_size1.h5
$SRC_H5DIFF_TESTFILES/h5diff_dset_zero_dim_size2.h5
$SRC_H5DIFF_TESTFILES/h5diff_danglelinks1.h5
$SRC_H5DIFF_TESTFILES/h5diff_danglelinks2.h5
$SRC_H5DIFF_TESTFILES/h5diff_grp_recurse1.h5
$SRC_H5DIFF_TESTFILES/h5diff_grp_recurse2.h5
$SRC_H5DIFF_TESTFILES/h5diff_grp_recurse_ext1.h5
$SRC_H5DIFF_TESTFILES/h5diff_grp_recurse_ext2-1.h5
$SRC_H5DIFF_TESTFILES/h5diff_grp_recurse_ext2-2.h5
$SRC_H5DIFF_TESTFILES/h5diff_grp_recurse_ext2-3.h5
$SRC_H5DIFF_TESTFILES/h5diff_exclude1-1.h5
$SRC_H5DIFF_TESTFILES/h5diff_exclude1-2.h5
$SRC_H5DIFF_TESTFILES/h5diff_exclude2-1.h5
$SRC_H5DIFF_TESTFILES/h5diff_exclude2-2.h5
$SRC_H5DIFF_TESTFILES/h5diff_exclude3-1.h5
$SRC_H5DIFF_TESTFILES/h5diff_exclude3-2.h5
$SRC_H5DIFF_TESTFILES/h5diff_comp_vl_strs.h5
$SRC_H5DIFF_TESTFILES/compounds_array_vlen1.h5
$SRC_H5DIFF_TESTFILES/compounds_array_vlen2.h5
$SRC_H5DIFF_TESTFILES/h5diff_attr_v_level1.h5
$SRC_H5DIFF_TESTFILES/h5diff_attr_v_level2.h5
$SRC_H5DIFF_TESTFILES/h5diff_enum_invalid_values.h5
$SRC_H5DIFF_TESTFILES/non_comparables1.h5
$SRC_H5DIFF_TESTFILES/non_comparables2.h5
"

LIST_OTHER_TEST_FILES="
$SRC_H5DIFF_TESTFILES/h5diff_10.txt
$SRC_H5DIFF_TESTFILES/h5diff_100.txt
$SRC_H5DIFF_TESTFILES/h5diff_101.txt
$SRC_H5DIFF_TESTFILES/h5diff_102.txt
$SRC_H5DIFF_TESTFILES/h5diff_103.txt
$SRC_H5DIFF_TESTFILES/h5diff_104.txt
$SRC_H5DIFF_TESTFILES/h5diff_11.txt
$SRC_H5DIFF_TESTFILES/h5diff_12.txt
$SRC_H5DIFF_TESTFILES/h5diff_13.txt
$SRC_H5DIFF_TESTFILES/h5diff_14.txt
$SRC_H5DIFF_TESTFILES/h5diff_15.txt
$SRC_H5DIFF_TESTFILES/h5diff_16_1.txt
$SRC_H5DIFF_TESTFILES/h5diff_16_2.txt
$SRC_H5DIFF_TESTFILES/h5diff_16_3.txt
$SRC_H5DIFF_TESTFILES/h5diff_17.txt
$SRC_H5DIFF_TESTFILES/h5diff_171.txt
$SRC_H5DIFF_TESTFILES/h5diff_172.txt
$SRC_H5DIFF_TESTFILES/h5diff_18_1.txt
$SRC_H5DIFF_TESTFILES/h5diff_18.txt
$SRC_H5DIFF_TESTFILES/h5diff_20.txt
$SRC_H5DIFF_TESTFILES/h5diff_200.txt
$SRC_H5DIFF_TESTFILES/h5diff_201.txt
$SRC_H5DIFF_TESTFILES/h5diff_202.txt
$SRC_H5DIFF_TESTFILES/h5diff_203.txt
$SRC_H5DIFF_TESTFILES/h5diff_204.txt
$SRC_H5DIFF_TESTFILES/h5diff_205.txt
$SRC_H5DIFF_TESTFILES/h5diff_206.txt
$SRC_H5DIFF_TESTFILES/h5diff_207.txt
$SRC_H5DIFF_TESTFILES/h5diff_208.txt
$SRC_H5DIFF_TESTFILES/h5diff_220.txt
$SRC_H5DIFF_TESTFILES/h5diff_221.txt
$SRC_H5DIFF_TESTFILES/h5diff_222.txt
$SRC_H5DIFF_TESTFILES/h5diff_223.txt
$SRC_H5DIFF_TESTFILES/h5diff_224.txt
$SRC_H5DIFF_TESTFILES/h5diff_21.txt
$SRC_H5DIFF_TESTFILES/h5diff_22.txt
$SRC_H5DIFF_TESTFILES/h5diff_23.txt
$SRC_H5DIFF_TESTFILES/h5diff_24.txt
$SRC_H5DIFF_TESTFILES/h5diff_25.txt
$SRC_H5DIFF_TESTFILES/h5diff_26.txt
$SRC_H5DIFF_TESTFILES/h5diff_27.txt
$SRC_H5DIFF_TESTFILES/h5diff_28.txt
$SRC_H5DIFF_TESTFILES/h5diff_30.txt
$SRC_H5DIFF_TESTFILES/h5diff_300.txt
$SRC_H5DIFF_TESTFILES/h5diff_400.txt
$SRC_H5DIFF_TESTFILES/h5diff_401.txt
$SRC_H5DIFF_TESTFILES/h5diff_402.txt
$SRC_H5DIFF_TESTFILES/h5diff_403.txt
$SRC_H5DIFF_TESTFILES/h5diff_404.txt
$SRC_H5DIFF_TESTFILES/h5diff_405.txt
$SRC_H5DIFF_TESTFILES/h5diff_406.txt
$SRC_H5DIFF_TESTFILES/h5diff_407.txt
$SRC_H5DIFF_TESTFILES/h5diff_408.txt
$SRC_H5DIFF_TESTFILES/h5diff_409.txt
$SRC_H5DIFF_TESTFILES/h5diff_410.txt
$SRC_H5DIFF_TESTFILES/h5diff_411.txt
$SRC_H5DIFF_TESTFILES/h5diff_412.txt
$SRC_H5DIFF_TESTFILES/h5diff_413.txt
$SRC_H5DIFF_TESTFILES/h5diff_414.txt
$SRC_H5DIFF_TESTFILES/h5diff_415.txt
$SRC_H5DIFF_TESTFILES/h5diff_416.txt
$SRC_H5DIFF_TESTFILES/h5diff_417.txt
$SRC_H5DIFF_TESTFILES/h5diff_418.txt
$SRC_H5DIFF_TESTFILES/h5diff_419.txt
$SRC_H5DIFF_TESTFILES/h5diff_420.txt
$SRC_H5DIFF_TESTFILES/h5diff_421.txt
$SRC_H5DIFF_TESTFILES/h5diff_422.txt
$SRC_H5DIFF_TESTFILES/h5diff_423.txt
$SRC_H5DIFF_TESTFILES/h5diff_424.txt
$SRC_H5DIFF_TESTFILES/h5diff_425.txt
$SRC_H5DIFF_TESTFILES/h5diff_450.txt
$SRC_H5DIFF_TESTFILES/h5diff_451.txt
$SRC_H5DIFF_TESTFILES/h5diff_452.txt
$SRC_H5DIFF_TESTFILES/h5diff_453.txt
$SRC_H5DIFF_TESTFILES/h5diff_454.txt
$SRC_H5DIFF_TESTFILES/h5diff_455.txt
$SRC_H5DIFF_TESTFILES/h5diff_456.txt
$SRC_H5DIFF_TESTFILES/h5diff_457.txt
$SRC_H5DIFF_TESTFILES/h5diff_458.txt
$SRC_H5DIFF_TESTFILES/h5diff_459.txt
$SRC_H5DIFF_TESTFILES/h5diff_465.txt
$SRC_H5DIFF_TESTFILES/h5diff_466.txt
$SRC_H5DIFF_TESTFILES/h5diff_467.txt
$SRC_H5DIFF_TESTFILES/h5diff_468.txt
$SRC_H5DIFF_TESTFILES/h5diff_469.txt
$SRC_H5DIFF_TESTFILES/h5diff_471.txt
$SRC_H5DIFF_TESTFILES/h5diff_472.txt
$SRC_H5DIFF_TESTFILES/h5diff_473.txt
$SRC_H5DIFF_TESTFILES/h5diff_474.txt
$SRC_H5DIFF_TESTFILES/h5diff_475.txt
$SRC_H5DIFF_TESTFILES/h5diff_480.txt
$SRC_H5DIFF_TESTFILES/h5diff_481.txt
$SRC_H5DIFF_TESTFILES/h5diff_482.txt
$SRC_H5DIFF_TESTFILES/h5diff_483.txt
$SRC_H5DIFF_TESTFILES/h5diff_484.txt
$SRC_H5DIFF_TESTFILES/h5diff_485.txt
$SRC_H5DIFF_TESTFILES/h5diff_486.txt
$SRC_H5DIFF_TESTFILES/h5diff_487.txt
$SRC_H5DIFF_TESTFILES/h5diff_50.txt
$SRC_H5DIFF_TESTFILES/h5diff_51.txt
$SRC_H5DIFF_TESTFILES/h5diff_52.txt
$SRC_H5DIFF_TESTFILES/h5diff_53.txt
$SRC_H5DIFF_TESTFILES/h5diff_54.txt
$SRC_H5DIFF_TESTFILES/h5diff_55.txt
$SRC_H5DIFF_TESTFILES/h5diff_56.txt
$SRC_H5DIFF_TESTFILES/h5diff_57.txt
$SRC_H5DIFF_TESTFILES/h5diff_58.txt
$SRC_H5DIFF_TESTFILES/h5diff_500.txt
$SRC_H5DIFF_TESTFILES/h5diff_501.txt
$SRC_H5DIFF_TESTFILES/h5diff_502.txt
$SRC_H5DIFF_TESTFILES/h5diff_503.txt
$SRC_H5DIFF_TESTFILES/h5diff_504.txt
$SRC_H5DIFF_TESTFILES/h5diff_505.txt
$SRC_H5DIFF_TESTFILES/h5diff_506.txt
$SRC_H5DIFF_TESTFILES/h5diff_507.txt
$SRC_H5DIFF_TESTFILES/h5diff_508.txt
$SRC_H5DIFF_TESTFILES/h5diff_509.txt
$SRC_H5DIFF_TESTFILES/h5diff_510.txt
$SRC_H5DIFF_TESTFILES/h5diff_511.txt
$SRC_H5DIFF_TESTFILES/h5diff_512.txt
$SRC_H5DIFF_TESTFILES/h5diff_513.txt
$SRC_H5DIFF_TESTFILES/h5diff_514.txt
$SRC_H5DIFF_TESTFILES/h5diff_515.txt
$SRC_H5DIFF_TESTFILES/h5diff_516.txt
$SRC_H5DIFF_TESTFILES/h5diff_517.txt
$SRC_H5DIFF_TESTFILES/h5diff_518.txt
$SRC_H5DIFF_TESTFILES/h5diff_530.txt
$SRC_H5DIFF_TESTFILES/h5diff_540.txt
$SRC_H5DIFF_TESTFILES/h5diff_600.txt
$SRC_H5DIFF_TESTFILES/h5diff_601.txt
$SRC_H5DIFF_TESTFILES/h5diff_603.txt
$SRC_H5DIFF_TESTFILES/h5diff_604.txt
$SRC_H5DIFF_TESTFILES/h5diff_605.txt
$SRC_H5DIFF_TESTFILES/h5diff_606.txt
$SRC_H5DIFF_TESTFILES/h5diff_607.txt
$SRC_H5DIFF_TESTFILES/h5diff_608.txt
$SRC_H5DIFF_TESTFILES/h5diff_609.txt
$SRC_H5DIFF_TESTFILES/h5diff_610.txt
$SRC_H5DIFF_TESTFILES/h5diff_612.txt
$SRC_H5DIFF_TESTFILES/h5diff_613.txt
$SRC_H5DIFF_TESTFILES/h5diff_614.txt
$SRC_H5DIFF_TESTFILES/h5diff_615.txt
$SRC_H5DIFF_TESTFILES/h5diff_616.txt
$SRC_H5DIFF_TESTFILES/h5diff_617.txt
$SRC_H5DIFF_TESTFILES/h5diff_618.txt
$SRC_H5DIFF_TESTFILES/h5diff_619.txt
$SRC_H5DIFF_TESTFILES/h5diff_621.txt
$SRC_H5DIFF_TESTFILES/h5diff_622.txt
$SRC_H5DIFF_TESTFILES/h5diff_623.txt
$SRC_H5DIFF_TESTFILES/h5diff_624.txt
$SRC_H5DIFF_TESTFILES/h5diff_625.txt
$SRC_H5DIFF_TESTFILES/h5diff_626.txt
$SRC_H5DIFF_TESTFILES/h5diff_627.txt
$SRC_H5DIFF_TESTFILES/h5diff_628.txt
$SRC_H5DIFF_TESTFILES/h5diff_629.txt
$SRC_H5DIFF_TESTFILES/h5diff_630.txt
$SRC_H5DIFF_TESTFILES/h5diff_631.txt
$SRC_H5DIFF_TESTFILES/h5diff_640.txt
$SRC_H5DIFF_TESTFILES/h5diff_641.txt
$SRC_H5DIFF_TESTFILES/h5diff_642.txt
$SRC_H5DIFF_TESTFILES/h5diff_643.txt
$SRC_H5DIFF_TESTFILES/h5diff_644.txt
$SRC_H5DIFF_TESTFILES/h5diff_645.txt
$SRC_H5DIFF_TESTFILES/h5diff_646.txt
$SRC_H5DIFF_TESTFILES/h5diff_70.txt
$SRC_H5DIFF_TESTFILES/h5diff_700.txt
$SRC_H5DIFF_TESTFILES/h5diff_701.txt
$SRC_H5DIFF_TESTFILES/h5diff_702.txt
$SRC_H5DIFF_TESTFILES/h5diff_703.txt
$SRC_H5DIFF_TESTFILES/h5diff_704.txt
$SRC_H5DIFF_TESTFILES/h5diff_705.txt
$SRC_H5DIFF_TESTFILES/h5diff_706.txt
$SRC_H5DIFF_TESTFILES/h5diff_707.txt
$SRC_H5DIFF_TESTFILES/h5diff_708.txt
$SRC_H5DIFF_TESTFILES/h5diff_709.txt
$SRC_H5DIFF_TESTFILES/h5diff_710.txt
$SRC_H5DIFF_TESTFILES/h5diff_80.txt
$SRC_H5DIFF_TESTFILES/h5diff_90.txt
"

#
# copy test files and expected output files from source dirs to test dir
#
COPY_TESTFILES="$LIST_HDF5_TEST_FILES $LIST_OTHER_TEST_FILES $LIST_HDF5_TEST_FILES_XML $LIST_OTHER_TEST_FILES_XML"

COPY_TESTFILES_TO_TESTDIR()
{
    # copy test files. Used -f to make sure get a new copy
    for tstfile in $COPY_TESTFILES
    do
        # ignore '#' comment
        echo $tstfile | tr -d ' ' | grep '^#' > /dev/null
        RET=$?
        if [ $RET -eq 1 ]; then
            # skip cp if srcdir is same as destdir
            # this occurs when build/test performed in source dir and
            # make cp fail
            SDIR=`$DIRNAME $tstfile`
            INODE_SDIR=`$LS -i -d $SDIR | $AWK -F' ' '{print $1}'`
            INODE_DDIR=`$LS -i -d $TESTDIR | $AWK -F' ' '{print $1}'`
            if [ "$INODE_SDIR" != "$INODE_DDIR" ]; then
    	        $CP -f $tstfile $TESTDIR
                if [ $? -ne 0 ]; then
                    echo "Error: FAILED to copy $tstfile ."
                
                    # Comment out this to CREATE expected file
                    exit $EXIT_FAILURE
                fi
            fi
        fi
    done
}

# Parse option
#   -p   run ph5diff tests
#   -h   print help page
while [ $# -gt 0 ]; do
    case "$1" in
    -p)	# reset the tool name and bin to run ph5diff tests
	TESTNAME=ph5diff
	H5DIFF=ph5diff               # The tool name
	H5DIFF_BIN=`pwd`/$H5DIFF
	pmode=yes
	shift
	;;
    -h) # print help page
	echo "$0 [-p] [-h]"
	echo "    -p   run ph5diff tests"
	echo "    -h   print help page"
	shift
	exit 0
	;;
    *)  # unknown option
        echo "$0: Unknown option ($1)"
	exit 1
	;;
    esac
done

# RUNSERIAL is used. Check if it can return exit code from executalbe correctly.
if [ -n "$RUNSERIAL_NOEXITCODE" ]; then
    echo "***Warning*** Serial Exit Code is not passed back to shell corretly."
    echo "***Warning*** Exit code checking is skipped."
    h5haveexitcode=no
fi

# Print a line-line message left justified in a field of 70 characters
# beginning with the word "Testing".
#
TESTING() {
   SPACES="                                                               "
   echo "Testing $* $SPACES" | cut -c1-70 | tr -d '\012'
}

# Source in the output filter function definitions.
. $srcdir/../../bin/output_filter.sh

# Run a test and print PASS or *FAIL*.  If a test fails then increment
# the `nerrors' global variable and (if $verbose is set) display the
# difference between the actual output and the expected output. The
# expected output is given as the first argument to this function and
# the actual output file is calculated by replacing the `.ddl' with
# `.out'.  The actual output is not removed if $HDF5_NOCLEANUP has a
# non-zero value.
#
# Need eval before the RUNCMD command because some machines like
# AIX, has RUNPARALLEL in the style as
#   MP_PROCS=3 MP_TASKS_PER_NODE=3 poe ./a.out
# that throws the shell script off.
#
TOOLTEST() {
    expect="$TESTDIR/$1"
    actual="$TESTDIR/`basename $1 .txt`.out"
    actual_err="$TESTDIR/`basename $1 .txt`.err"
    actual_sav=${actual}-sav
    actual_err_sav=${actual_err}-sav
    shift
    if test -n "$pmode"; then
        RUNCMD=$RUNPARALLEL
    else
        RUNCMD=$RUNSERIAL
    fi

    # Run test.
    TESTING $H5DIFF $@
    (
	#echo "#############################"
	#echo "Expected output for '$H5DIFF $@'" 
	#echo "#############################"
	cd $TESTDIR
	eval $RUNCMD $H5DIFF_BIN "$@"
    ) >$actual 2>$actual_err
    EXIT_CODE=$?
    # save actual and actual_err in case they are needed later.
    cp $actual $actual_sav
    STDOUT_FILTER $actual
    cp $actual_err $actual_err_sav
    STDERR_FILTER $actual_err
    cat $actual_err >> $actual
    # don't add exit code check in pmode, as it causes failure. (exit code 
    # is from mpirun not tool)
    # if any problem occurs relate to an exit code, it will be caught in 
    # serial mode, so the test is fullfilled.
    if test $h5haveexitcode = 'yes' -a -z "$pmode"; then
      echo "EXIT CODE: $EXIT_CODE" >> $actual
    fi

    if [ ! -f $expect ]; then
        # Create the expect file if it doesn't yet exist.
        echo " CREATED"
        cp $actual $expect
    elif $CMP $expect $actual; then
        echo " PASSED"
    elif test $h5haveexitcode = 'yes' -a -z "$pmode"; then
        echo "*FAILED*"
        echo "    Expected result ($expect) differs from actual result ($actual)"
        nerrors="`expr $nerrors + 1`"
        test yes = "$verbose" && $DIFF $expect $actual |sed 's/^/    /'
    else
	    # parallel mode output are often of different ordering from serial
        # output.  If the sorted expected and actual files compare the same,
        # it is safe to assume the actual output match the expected file.
        expect_sorted=expect_sorted
        actual_sorted=actual_sorted
        sort $expect -o $expect_sorted
        sort $actual -o $actual_sorted
        # remove "EXIT CODE:" line from expect file. test for exit code
        # is done by serial mode.
        grep -v "EXIT CODE:" $expect_sorted > $expect_sorted.noexit
        mv $expect_sorted.noexit $expect_sorted
	if $CMP $expect_sorted $actual_sorted; then
	    echo " PASSED"
	else
	    echo "*FAILED*"
	    nerrors="`expr $nerrors + 1`"
	    if test yes = "$verbose"; then
		echo "====Expected result ($expect_sorted) differs from actual result ($actual_sorted)"
		$DIFF $expect_sorted $actual_sorted |sed 's/^/    /'
		echo "====The actual output ($actual_sav)"
		sed 's/^/    /' < $actual_sav 
		echo "====The actual stderr ($actual_err_sav)"
		sed 's/^/    /' < $actual_err_sav 
		echo "====End of actual stderr ($actual_err_sav)"
		echo ""
	    fi
	fi
    fi

    # Clean up output file
    if test -z "$HDF5_NOCLEANUP"; then
	    rm -f $actual $actual_err $actual_sav $actual_err_sav
    	rm -f $actual_sorted $expect_sorted
    fi
}


# Print a "SKIP" message
SKIP() {
	 TESTING $H5DIFF $@
	  echo  " -SKIP-"
}



##############################################################################
# The tests 
# To avoid the printing of the complete full path of the test file, that hides
# all the other parameters for long paths, the printing of the command line 
# is done first in
# TESTING with the name only of the test file $TOOL, not its full path $TESTFILE
##############################################################################
# prepare for test
COPY_TESTFILES_TO_TESTDIR

# ############################################################################
# # Common usage
# ############################################################################

# 1.0
TOOLTEST h5diff_10.txt -h

# 1.1 normal mode
TOOLTEST h5diff_11.txt  h5diff_basic1.h5 h5diff_basic2.h5 

# 1.2 normal mode with objects
TOOLTEST h5diff_12.txt  h5diff_basic1.h5 h5diff_basic2.h5  g1/dset1 g1/dset2

# 1.3 report mode
TOOLTEST h5diff_13.txt -r h5diff_basic1.h5 h5diff_basic2.h5 

# 1.4 report  mode with objects
TOOLTEST h5diff_14.txt  -r h5diff_basic1.h5 h5diff_basic2.h5 g1/dset1 g1/dset2

# 1.5 with -d
TOOLTEST h5diff_15.txt --report --delta=5 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 1.6.1 with -p (int)
TOOLTEST h5diff_16_1.txt -v -p 0.02 h5diff_basic1.h5 h5diff_basic1.h5 g1/dset5 g1/dset6

# 1.6.2 with -p (unsigned long_long)
TOOLTEST h5diff_16_2.txt --verbose --relative=0.02 h5diff_basic1.h5 h5diff_basic1.h5 g1/dset7 g1/dset8

# 1.6.3 with -p (double)
TOOLTEST h5diff_16_3.txt -v -p 0.02 h5diff_basic1.h5 h5diff_basic1.h5 g1/dset9 g1/dset10

# 1.7 verbose mode
TOOLTEST h5diff_17.txt -v h5diff_basic1.h5 h5diff_basic2.h5   

# 1.7 test 32-bit INFINITY
TOOLTEST h5diff_171.txt -v h5diff_basic1.h5 h5diff_basic1.h5 /g1/fp19 /g1/fp19_COPY

# 1.7 test 64-bit INFINITY
TOOLTEST h5diff_172.txt -v h5diff_basic1.h5 h5diff_basic1.h5 /g1/fp20 /g1/fp20_COPY

# 1.8 quiet mode 
TOOLTEST h5diff_18.txt -q h5diff_basic1.h5 h5diff_basic2.h5 

# 1.8 -v and -q
TOOLTEST h5diff_18_1.txt -v -q h5diff_basic1.h5 h5diff_basic2.h5


# ##############################################################################
# # not comparable types
# ##############################################################################

# 2.0
TOOLTEST h5diff_20.txt -v h5diff_types.h5 h5diff_types.h5  dset g1

# 2.1
TOOLTEST h5diff_21.txt -v h5diff_types.h5 h5diff_types.h5 dset l1

# 2.2
TOOLTEST h5diff_22.txt -v  h5diff_types.h5 h5diff_types.h5 dset t1

# ##############################################################################
# # compare groups, types, links (no differences and differences)
# ##############################################################################

# 2.3
TOOLTEST h5diff_23.txt -v h5diff_types.h5 h5diff_types.h5 g1 g1

# 2.4
TOOLTEST h5diff_24.txt -v h5diff_types.h5 h5diff_types.h5 t1 t1

# 2.5
TOOLTEST h5diff_25.txt -v h5diff_types.h5 h5diff_types.h5 l1 l1 

# 2.6
TOOLTEST h5diff_26.txt -v h5diff_types.h5 h5diff_types.h5 g1 g2

# 2.7
TOOLTEST h5diff_27.txt -v h5diff_types.h5 h5diff_types.h5 t1 t2

# 2.8
TOOLTEST h5diff_28.txt -v h5diff_types.h5 h5diff_types.h5 l1 l2


# ##############################################################################
# # Enum value tests (may become more comprehensive in the future)
# ##############################################################################

# 3.0
# test enum types which may have invalid values
TOOLTEST h5diff_30.txt -v h5diff_enum_invalid_values.h5 h5diff_enum_invalid_values.h5 dset1 dset2




# ##############################################################################
# # Dataset datatypes
# ##############################################################################

# 5.0
TOOLTEST h5diff_50.txt -v h5diff_dtypes.h5 h5diff_dtypes.h5 dset0a dset0b

# 5.1
TOOLTEST h5diff_51.txt -v h5diff_dtypes.h5 h5diff_dtypes.h5 dset1a dset1b

# 5.2
TOOLTEST h5diff_52.txt -v h5diff_dtypes.h5 h5diff_dtypes.h5 dset2a dset2b

# 5.3
TOOLTEST h5diff_53.txt -v h5diff_dtypes.h5 h5diff_dtypes.h5 dset3a dset4b

# 5.4
TOOLTEST h5diff_54.txt -v h5diff_dtypes.h5 h5diff_dtypes.h5 dset4a dset4b

# 5.5
TOOLTEST h5diff_55.txt -v h5diff_dtypes.h5 h5diff_dtypes.h5 dset5a dset5b

# 5.6
TOOLTEST h5diff_56.txt -v h5diff_dtypes.h5 h5diff_dtypes.h5 dset6a dset6b

# 5.7
TOOLTEST h5diff_57.txt -v h5diff_dtypes.h5 h5diff_dtypes.h5 dset7a dset7b

# 5.8 (region reference)
TOOLTEST h5diff_58.txt -v h5diff_dset1.h5 h5diff_dset2.h5 refreg

# ##############################################################################
# # Error messages
# ##############################################################################


# 6.0: Check if the command line number of arguments is less than 3
TOOLTEST h5diff_600.txt h5diff_basic1.h5 

# 6.1: Check if non-exist object name is specified 
TOOLTEST h5diff_601.txt h5diff_basic1.h5 h5diff_basic1.h5 nono_obj


# ##############################################################################
# # -d 
# ##############################################################################


# 6.3: negative value
TOOLTEST h5diff_603.txt -d -4 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.4: zero
TOOLTEST h5diff_604.txt -d 0 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.5: non number
TOOLTEST h5diff_605.txt -d u h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.6: hexadecimal
TOOLTEST h5diff_606.txt -d 0x1 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.7: string
TOOLTEST h5diff_607.txt -d "1" h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.8: use system epsilon 
TOOLTEST h5diff_608.txt --use-system-epsilon h5diff_basic1.h5 h5diff_basic2.h5  g1/dset3 g1/dset4

# 6.9: number larger than biggest difference
TOOLTEST h5diff_609.txt -d 200 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.10: number smaller than smallest difference
TOOLTEST h5diff_610.txt -d 1 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4


# ##############################################################################
# # -p
# ##############################################################################


# 6.12: negative value
TOOLTEST h5diff_612.txt -p -4 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.13: zero
TOOLTEST h5diff_613.txt -p 0 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.14: non number
TOOLTEST h5diff_614.txt -p u h5diff_basic1.h5 h5diff_basic2.h5  g1/dset3 g1/dset4

# 6.15: hexadecimal
TOOLTEST h5diff_615.txt -p 0x1 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.16: string
TOOLTEST h5diff_616.txt -p "0.21" h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.17: repeated option
TOOLTEST h5diff_617.txt -p 0.21 -p 0.22 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.18: number larger than biggest difference
TOOLTEST h5diff_618.txt -p 2 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.19: number smaller than smallest difference
TOOLTEST h5diff_619.txt -p 0.005 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4



# ##############################################################################
# # -n
# ##############################################################################

# 6.21: negative value
TOOLTEST h5diff_621.txt -n -4 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.22: zero
TOOLTEST h5diff_622.txt -n 0 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.23: non number
TOOLTEST h5diff_623.txt -n u h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.24: hexadecimal
TOOLTEST h5diff_624.txt -n 0x1 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.25: string
TOOLTEST h5diff_625.txt -n "2" h5diff_basic1.h5 h5diff_basic2.h5  g1/dset3 g1/dset4

# 6.26: repeated option
TOOLTEST h5diff_626.txt -n 2 -n 3 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.27: number larger than biggest difference
TOOLTEST h5diff_627.txt --count=200 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# 6.28: number smaller than smallest difference
TOOLTEST h5diff_628.txt -n 1 h5diff_basic1.h5 h5diff_basic2.h5 g1/dset3 g1/dset4

# Disabling this test as it hangs - LRK 20090618
# 6.29  non valid files
#TOOLTEST h5diff_629.txt file1.h6 file2.h6

# ##############################################################################
# # NaN
# ##############################################################################
# 6.30: test (NaN == NaN) must be true based on our documentation -- XCAO
TOOLTEST h5diff_630.txt -v -d "0.0001" h5diff_basic1.h5 h5diff_basic1.h5 g1/fp18 g1/fp18_COPY
TOOLTEST h5diff_631.txt -v --use-system-epsilon h5diff_basic1.h5 h5diff_basic1.h5 g1/fp18 g1/fp18_COPY


# ##############################################################################
# 7.  attributes
# ##############################################################################
TOOLTEST h5diff_70.txt -v h5diff_attr1.h5 h5diff_attr2.h5 

# ##################################################
#  attrs with verbose option level
# ##################################################

TOOLTEST h5diff_700.txt -v1 h5diff_attr1.h5 h5diff_attr2.h5 
TOOLTEST h5diff_701.txt -v2 h5diff_attr1.h5 h5diff_attr2.h5 
TOOLTEST h5diff_702.txt --verbose=1 h5diff_attr1.h5 h5diff_attr2.h5 
TOOLTEST h5diff_703.txt --verbose=2 h5diff_attr1.h5 h5diff_attr2.h5 

# same attr number , all same attr name
TOOLTEST h5diff_704.txt -v2 h5diff_attr_v_level1.h5 h5diff_attr_v_level2.h5 /g

# same attr number , some same attr name
TOOLTEST h5diff_705.txt -v2 h5diff_attr_v_level1.h5 h5diff_attr_v_level2.h5 /dset

# same attr number , all different attr name
TOOLTEST h5diff_706.txt -v2 h5diff_attr_v_level1.h5 h5diff_attr_v_level2.h5 /ntype

# different attr number , same attr name (intersected)
TOOLTEST h5diff_707.txt -v2 h5diff_attr_v_level1.h5 h5diff_attr_v_level2.h5 /g2

# different attr number , all different attr name 
TOOLTEST h5diff_708.txt -v2 h5diff_attr_v_level1.h5 h5diff_attr_v_level2.h5 /g3

# when no attributes exist in both objects
TOOLTEST h5diff_709.txt -v2 h5diff_attr_v_level1.h5 h5diff_attr_v_level2.h5 /g4

# file vs file
TOOLTEST h5diff_710.txt -v2 h5diff_attr_v_level1.h5 h5diff_attr_v_level2.h5

# ##############################################################################
# 8.  all dataset datatypes
# ##############################################################################
TOOLTEST h5diff_80.txt -v h5diff_dset1.h5 h5diff_dset2.h5 

# 9. compare a file with itself
TOOLTEST h5diff_90.txt -v h5diff_basic2.h5 h5diff_basic2.h5

# 10. read by hyperslab, print indexes
if test -n "$pmode" -a "$mydomainname" = hdfgroup.uiuc.edu; then
    # skip this test which sometimes hangs in some THG machines
    SKIP -v h5diff_hyper1.h5 h5diff_hyper2.h5
else
    TOOLTEST h5diff_100.txt -v h5diff_hyper1.h5 h5diff_hyper2.h5 
fi

# 11. floating point comparison
# double value
TOOLTEST h5diff_101.txt -v h5diff_basic1.h5 h5diff_basic1.h5 g1/d1  g1/d2 

# float value
TOOLTEST h5diff_102.txt -v h5diff_basic1.h5 h5diff_basic1.h5 g1/fp1 g1/fp2 

# with --use-system-epsilon for double value 
TOOLTEST h5diff_103.txt -v --use-system-epsilon h5diff_basic1.h5 h5diff_basic1.h5 g1/d1  g1/d2 

# with --use-system-epsilon for float value
TOOLTEST h5diff_104.txt -v --use-system-epsilon h5diff_basic1.h5 h5diff_basic1.h5 g1/fp1 g1/fp2 


# not comparable -c flag
TOOLTEST h5diff_200.txt h5diff_basic2.h5 h5diff_basic2.h5 g2/dset1  g2/dset2 

TOOLTEST h5diff_201.txt -c h5diff_basic2.h5 h5diff_basic2.h5 g2/dset1  g2/dset2 

TOOLTEST h5diff_202.txt -c h5diff_basic2.h5 h5diff_basic2.h5 g2/dset2  g2/dset3

TOOLTEST h5diff_203.txt -c h5diff_basic2.h5 h5diff_basic2.h5 g2/dset3  g2/dset4

TOOLTEST h5diff_204.txt -c h5diff_basic2.h5 h5diff_basic2.h5 g2/dset4  g2/dset5

TOOLTEST h5diff_205.txt -c h5diff_basic2.h5 h5diff_basic2.h5 g2/dset5  g2/dset6

# not comparable in compound
TOOLTEST h5diff_206.txt -c h5diff_basic2.h5 h5diff_basic2.h5 g2/dset7  g2/dset8

TOOLTEST h5diff_207.txt -c h5diff_basic2.h5 h5diff_basic2.h5 g2/dset8  g2/dset9

# not comparable in dataspace of zero dimension size
TOOLTEST h5diff_208.txt -c h5diff_dset_zero_dim_size1.h5 h5diff_dset_zero_dim_size2.h5 

# non-comparable dataset with comparable attribute, and other comparable datasets. 
# Also test non-compatible attributes with different type, dimention, rank.
# All the comparables should display differences.
TOOLTEST h5diff_220.txt -c non_comparables1.h5 non_comparables2.h5 /g1

# comparable dataset with non-comparable attribute and other comparable attributes.
# All the comparables should display differences.
TOOLTEST h5diff_221.txt -c non_comparables1.h5 non_comparables2.h5 /g2

# entire file
# All the comparables should display differences.
if test -n "$pmode"; then
    # parallel mode: 
    # skip due to ph5diff hangs on koala (linux64-LE) and ember intermittently.
    # (HDFFV-8003 - TBD)
    SKIP -c non_comparables1.h5 non_comparables2.h5
else
    TOOLTEST h5diff_222.txt -c non_comparables1.h5 non_comparables2.h5
fi    

# non-comparable test for common objects (same name) with different object types
# (HDFFV-7644)
TOOLTEST h5diff_223.txt -c non_comparables1.h5 non_comparables2.h5 /diffobjtypes
# swap files
TOOLTEST h5diff_224.txt -c non_comparables2.h5 non_comparables1.h5 /diffobjtypes
    
# ##############################################################################
# # Links compare without --follow-symlinks nor --no-dangling-links
# ##############################################################################
# test for bug1749
TOOLTEST h5diff_300.txt -v h5diff_links.h5 h5diff_links.h5 /link_g1 /link_g2

# ##############################################################################
# # Links compare with --follow-symlinks Only
# ##############################################################################
# soft links file to file
TOOLTEST h5diff_400.txt --follow-symlinks -v h5diff_softlinks.h5 h5diff_softlinks.h5

# softlink vs dset"
TOOLTEST h5diff_401.txt --follow-symlinks -v h5diff_softlinks.h5 h5diff_softlinks.h5 /softlink_dset1_1 /target_dset2

# dset vs softlink"
TOOLTEST h5diff_402.txt --follow-symlinks -v h5diff_softlinks.h5 h5diff_softlinks.h5 /target_dset2 /softlink_dset1_1

# softlink vs softlink"
TOOLTEST h5diff_403.txt --follow-symlinks -v h5diff_softlinks.h5 h5diff_softlinks.h5 /softlink_dset1_1 /softlink_dset2

# extlink vs extlink (FILE)"
TOOLTEST h5diff_404.txt --follow-symlinks -v h5diff_extlink_src.h5 h5diff_extlink_src.h5

# extlink vs dset"
TOOLTEST h5diff_405.txt --follow-symlinks -v h5diff_extlink_src.h5 h5diff_extlink_trg.h5 /ext_link_dset1 /target_group2/x_dset

# dset vs extlink"
TOOLTEST h5diff_406.txt --follow-symlinks -v h5diff_extlink_trg.h5 h5diff_extlink_src.h5 /target_group2/x_dset /ext_link_dset1

# extlink vs extlink"
TOOLTEST h5diff_407.txt --follow-symlinks -v h5diff_extlink_src.h5 h5diff_extlink_src.h5 /ext_link_dset1 /ext_link_dset2

# softlink vs extlink"
TOOLTEST h5diff_408.txt --follow-symlinks -v h5diff_softlinks.h5 h5diff_extlink_src.h5 /softlink_dset1_1 /ext_link_dset2

# extlink vs softlink "
TOOLTEST h5diff_409.txt --follow-symlinks -v h5diff_extlink_src.h5 h5diff_softlinks.h5 /ext_link_dset2 /softlink_dset1_1

# linked_softlink vs linked_softlink (FILE)"
TOOLTEST h5diff_410.txt --follow-symlinks -v h5diff_linked_softlink.h5 h5diff_linked_softlink.h5

# dset2 vs linked_softlink_dset1"
TOOLTEST h5diff_411.txt --follow-symlinks -v h5diff_linked_softlink.h5 h5diff_linked_softlink.h5 /target_dset2 /softlink1_to_slink2

# linked_softlink_dset1 vs dset2"
TOOLTEST h5diff_412.txt --follow-symlinks -v h5diff_linked_softlink.h5 h5diff_linked_softlink.h5 /softlink1_to_slink2 /target_dset2

# linked_softlink_to_dset1 vs linked_softlink_to_dset2"
TOOLTEST h5diff_413.txt --follow-symlinks -v h5diff_linked_softlink.h5 h5diff_linked_softlink.h5 /softlink1_to_slink2 /softlink2_to_slink2

# group vs linked_softlink_group1"
TOOLTEST h5diff_414.txt --follow-symlinks -v h5diff_linked_softlink.h5 h5diff_linked_softlink.h5 /target_group /softlink3_to_slink2

# linked_softlink_group1 vs group"
TOOLTEST h5diff_415.txt --follow-symlinks -v h5diff_linked_softlink.h5 h5diff_linked_softlink.h5 /softlink3_to_slink2 /target_group

# linked_softlink_to_group1 vs linked_softlink_to_group2"
TOOLTEST h5diff_416.txt --follow-symlinks -v h5diff_linked_softlink.h5 h5diff_linked_softlink.h5 /softlink3_to_slink2 /softlink4_to_slink2

# non-exist-softlink vs softlink"
TOOLTEST h5diff_417.txt --follow-symlinks -v h5diff_softlinks.h5 h5diff_softlinks.h5 /softlink_noexist /softlink_dset2

# softlink vs non-exist-softlink"
TOOLTEST h5diff_418.txt --follow-symlinks -v h5diff_softlinks.h5 h5diff_softlinks.h5 /softlink_dset2 /softlink_noexist

# non-exist-extlink_file vs extlink"
TOOLTEST h5diff_419.txt --follow-symlinks -v h5diff_extlink_src.h5 h5diff_extlink_src.h5 /ext_link_noexist2 /ext_link_dset2

# exlink vs non-exist-extlink_file"
TOOLTEST h5diff_420.txt --follow-symlinks -v h5diff_extlink_src.h5 h5diff_extlink_src.h5 /ext_link_dset2 /ext_link_noexist2

# extlink vs non-exist-extlink_obj"
TOOLTEST h5diff_421.txt --follow-symlinks -v h5diff_extlink_src.h5 h5diff_extlink_src.h5 /ext_link_dset2 /ext_link_noexist1

# non-exist-extlink_obj vs extlink"
TOOLTEST h5diff_422.txt --follow-symlinks -v h5diff_extlink_src.h5 h5diff_extlink_src.h5 /ext_link_noexist1 /ext_link_dset2

# extlink_to_softlink_to_dset1 vs dset2"
TOOLTEST h5diff_423.txt --follow-symlinks -v h5diff_ext2softlink_src.h5 h5diff_ext2softlink_trg.h5 /ext_link_to_slink1 /dset2

# dset2 vs extlink_to_softlink_to_dset1"
TOOLTEST h5diff_424.txt --follow-symlinks -v h5diff_ext2softlink_trg.h5 h5diff_ext2softlink_src.h5 /dset2 /ext_link_to_slink1

# extlink_to_softlink_to_dset1 vs extlink_to_softlink_to_dset2"
TOOLTEST h5diff_425.txt --follow-symlinks -v h5diff_ext2softlink_src.h5 h5diff_ext2softlink_src.h5 /ext_link_to_slink1 /ext_link_to_slink2


# ##############################################################################
# # Dangling links compare (--follow-symlinks and --no-dangling-links)
# ##############################################################################
# dangling links --follow-symlinks (FILE to FILE)
TOOLTEST h5diff_450.txt  --follow-symlinks -v h5diff_danglelinks1.h5 h5diff_danglelinks2.h5

# dangling links --follow-symlinks and --no-dangling-links (FILE to FILE)
TOOLTEST h5diff_451.txt  --follow-symlinks -v --no-dangling-links  h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 

# try --no-dangling-links without --follow-symlinks options
TOOLTEST h5diff_452.txt  --no-dangling-links  h5diff_softlinks.h5 h5diff_softlinks.h5

# dangling link found for soft links (FILE to FILE)
TOOLTEST h5diff_453.txt  --follow-symlinks -v --no-dangling-links  h5diff_softlinks.h5 h5diff_softlinks.h5  

# dangling link found for soft links (obj to obj)
TOOLTEST h5diff_454.txt  --follow-symlinks -v --no-dangling-links  h5diff_softlinks.h5 h5diff_softlinks.h5 /softlink_dset2 /softlink_noexist 

# dangling link found for soft links (obj to obj) Both dangle links
TOOLTEST h5diff_455.txt  --follow-symlinks -v --no-dangling-links  h5diff_softlinks.h5 h5diff_softlinks.h5 /softlink_noexist /softlink_noexist 

# dangling link found for ext links (FILE to FILE)
TOOLTEST h5diff_456.txt  --follow-symlinks -v --no-dangling-links  h5diff_extlink_src.h5 h5diff_extlink_src.h5 

# dangling link found for ext links (obj to obj). target file exist
TOOLTEST h5diff_457.txt  --follow-symlinks -v --no-dangling-links  h5diff_extlink_src.h5 h5diff_extlink_src.h5 /ext_link_dset1 /ext_link_noexist1 

# dangling link found for ext links (obj to obj). target file NOT exist
TOOLTEST h5diff_458.txt  --follow-symlinks -v --no-dangling-links  h5diff_extlink_src.h5 h5diff_extlink_src.h5 /ext_link_dset1 /ext_link_noexist2  

# dangling link found for ext links (obj to obj). Both dangle links
TOOLTEST h5diff_459.txt  --follow-symlinks -v --no-dangling-links  h5diff_extlink_src.h5 h5diff_extlink_src.h5 /ext_link_noexist1 /ext_link_noexist2

# dangling link --follow-symlinks (obj vs obj)
# (HDFFV-7836)
TOOLTEST h5diff_465.txt --follow-symlinks h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /soft_link1
# (HDFFV-7835)
# soft dangling vs. soft dangling
TOOLTEST h5diff_466.txt -v --follow-symlinks h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /soft_link1
# soft link  vs. soft dangling
TOOLTEST h5diff_467.txt -v --follow-symlinks h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /soft_link2
# ext dangling vs. ext dangling
TOOLTEST h5diff_468.txt -v --follow-symlinks h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /ext_link4 
# ext link vs. ext dangling
TOOLTEST h5diff_469.txt -v --follow-symlinks h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /ext_link2

#----------------------------------------
# dangling links without follow symlink 
# (HDFFV-7998)
# test - soft dangle links (same and different paths), 
#      - external dangle links (same and different paths)
TOOLTEST h5diff_471.txt -v h5diff_danglelinks1.h5 h5diff_danglelinks2.h5
TOOLTEST h5diff_472.txt -v h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /soft_link1
TOOLTEST h5diff_473.txt -v h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /soft_link4
TOOLTEST h5diff_474.txt -v h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /ext_link4
TOOLTEST h5diff_475.txt -v h5diff_danglelinks1.h5 h5diff_danglelinks2.h5 /ext_link1

# ##############################################################################
# # test for group diff recursivly
# ##############################################################################
# root 
TOOLTEST h5diff_500.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 / /
TOOLTEST h5diff_501.txt -v --follow-symlinks h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 / /

# root vs group
TOOLTEST h5diff_502.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 / /grp1/grp2/grp3

# group vs group (same name and structure)
TOOLTEST h5diff_503.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /grp1 /grp1

# group vs group (different name and structure)
TOOLTEST h5diff_504.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /grp1/grp2 /grp1/grp2/grp3

# groups vs soft-link
TOOLTEST h5diff_505.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /grp1 /slink_grp1
TOOLTEST h5diff_506.txt -v --follow-symlinks h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /grp1/grp2 /slink_grp2

# groups vs ext-link
TOOLTEST h5diff_507.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /grp1 /elink_grp1
TOOLTEST h5diff_508.txt -v --follow-symlinks h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /grp1 /elink_grp1

# soft-link vs ext-link
TOOLTEST h5diff_509.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /slink_grp1 /elink_grp1
TOOLTEST h5diff_510.txt -v --follow-symlinks h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /slink_grp1 /elink_grp1

# circled ext links
TOOLTEST h5diff_511.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /grp10 /grp11
TOOLTEST h5diff_512.txt -v --follow-symlinks h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /grp10 /grp11

# circled soft2ext-link vs soft2ext-link
TOOLTEST h5diff_513.txt -v h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /slink_grp10 /slink_grp11
TOOLTEST h5diff_514.txt -v --follow-symlinks h5diff_grp_recurse1.h5 h5diff_grp_recurse2.h5 /slink_grp10 /slink_grp11

###############################################################################
# Test for group recursive diff via multi-linked external links 
# With follow-symlinks, file h5diff_grp_recurse_ext1.h5 and h5diff_grp_recurse_ext2-1.h5 should
# be same with the external links.
###############################################################################
# file vs file
TOOLTEST h5diff_515.txt -v h5diff_grp_recurse_ext1.h5 h5diff_grp_recurse_ext2-1.h5
TOOLTEST h5diff_516.txt -v --follow-symlinks h5diff_grp_recurse_ext1.h5 h5diff_grp_recurse_ext2-1.h5
# group vs group
TOOLTEST h5diff_517.txt -v h5diff_grp_recurse_ext1.h5 h5diff_grp_recurse_ext2-1.h5 /g1
TOOLTEST h5diff_518.txt -v --follow-symlinks h5diff_grp_recurse_ext1.h5 h5diff_grp_recurse_ext2-1.h5 /g1

# ##############################################################################
# # Exclude objects (--exclude-path)
# ##############################################################################
#
# Same structure, same names and different value.
#
# Exclude the object with different value. Expect return - same
TOOLTEST h5diff_480.txt -v --exclude-path /group1/dset3 h5diff_exclude1-1.h5 h5diff_exclude1-2.h5
# Verify different by not excluding. Expect return - diff
TOOLTEST h5diff_481.txt -v h5diff_exclude1-1.h5 h5diff_exclude1-2.h5

#
# Different structure, different names. 
#
# Exclude all the different objects. Expect return - same
TOOLTEST h5diff_482.txt -v --exclude-path "/group1" --exclude-path "/dset1" h5diff_exclude2-1.h5 h5diff_exclude2-2.h5
# Exclude only some different objects. Expect return - diff
TOOLTEST h5diff_483.txt -v --exclude-path "/group1" h5diff_exclude2-1.h5 h5diff_exclude2-2.h5

# Exclude from group compare
TOOLTEST h5diff_484.txt -v --exclude-path "/dset3" h5diff_exclude1-1.h5 h5diff_exclude1-2.h5 /group1

#
# Only one file contains unique objs. Common objs are same.
# (HDFFV-7837)
#
TOOLTEST h5diff_485.txt -v --exclude-path "/group1" h5diff_exclude3-1.h5 h5diff_exclude3-2.h5
TOOLTEST h5diff_486.txt -v --exclude-path "/group1" h5diff_exclude3-2.h5 h5diff_exclude3-1.h5
TOOLTEST h5diff_487.txt -v --exclude-path "/group1/dset" h5diff_exclude3-1.h5 h5diff_exclude3-2.h5


# ##############################################################################
# # diff various multiple vlen and fixed strings in a compound type dataset
# ##############################################################################
TOOLTEST h5diff_530.txt -v  h5diff_comp_vl_strs.h5 h5diff_comp_vl_strs.h5 /group /group_copy

# ##############################################################################
# # Test container types (array,vlen) with multiple nested compound types
# # Complex compound types in dataset and attribute
# ##############################################################################
TOOLTEST h5diff_540.txt -v compounds_array_vlen1.h5 compounds_array_vlen2.h5

# ##############################################################################
# # Test mutually exclusive options 
# ##############################################################################
# Test with -d , -p and --use-system-epsilon. 
TOOLTEST h5diff_640.txt -v -d 5 -p 0.05 --use-system-epsilon h5diff_basic1.h5 h5diff_basic2.h5 /g1/dset3 /g1/dset4
TOOLTEST h5diff_641.txt -v -d 5 -p 0.05 h5diff_basic1.h5 h5diff_basic2.h5 /g1/dset3 /g1/dset4
TOOLTEST h5diff_642.txt -v -p 0.05 -d 5 h5diff_basic1.h5 h5diff_basic2.h5 /g1/dset3 /g1/dset4
TOOLTEST h5diff_643.txt -v -d 5 --use-system-epsilon h5diff_basic1.h5 h5diff_basic2.h5 /g1/dset3 /g1/dset4
TOOLTEST h5diff_644.txt -v --use-system-epsilon -d 5 h5diff_basic1.h5 h5diff_basic2.h5 /g1/dset3 /g1/dset4
TOOLTEST h5diff_645.txt -v -p 0.05 --use-system-epsilon h5diff_basic1.h5 h5diff_basic2.h5 /g1/dset3 /g1/dset4
TOOLTEST h5diff_646.txt -v --use-system-epsilon -p 0.05 h5diff_basic1.h5 h5diff_basic2.h5 /g1/dset3 /g1/dset4


# ##############################################################################
# # END
# ##############################################################################

if test $nerrors -eq 0 ; then
    echo "All $TESTNAME tests passed."
    exit $EXIT_SUCCESS
else
    echo "$TESTNAME tests failed with $nerrors errors."
    exit $EXIT_FAILURE
fi
