#!/bin/sh

h5dump=h5dump		# a relative name
cmp='cmp -s'
diff='diff -c'

nerrors=0
verbose=yes

# Print a line-line message left justified in a field of 70 characters
# beginning with the word "Testing".
TESTING()
{
    SPACES="                                                               "
    echo "Testing $* $SPACES" |cut -c1-70 |tr -d '\n'
}

# Run a test and print PASS or *FAIL*.  If a test fails then increment
# the `nerrors' global variable and (if $verbose is set) display the
# difference between the actual output and the expected output. The
# expected output is given as the first argument to this function and
# the actual output file is calculated by replacing the `.ddl' with
# `.out'.  The actual output is not removed if $HDF5_NOCLEANUP has a
# non-zero value.
DUMP()
{
    expect=testfiles/$1
    actual="testfiles/`basename $1 .ddl`.out"
    shift

    # Run test.
    TESTING $h5dump $@
    (
	echo "#############################"
	echo "Expected output for '$h5dump $@'" 
	echo "#############################"
	cd testfiles
        ../$h5dump "$@" 2>/dev/null
    ) >$actual
    
    # Results. We normalize the result to account for different output 
    # widths.  That is, the test should succeed if the only
    # differences are in white space. We have to do this the hard way
    # because diff isn't always smart enough.
    tr '\n' ' ' <$actual |tr -s ' \t' |fold >$actual-norm
    tr '\n' ' ' <$expect |tr -s ' \t' |fold >$expect-norm

    if $cmp $expect-norm $actual-norm; then
	echo " PASSED"
    else
	echo "*FAILED*"
	echo "    Actual result (*.out) differs from expected result (*.ddl)"
	nerrors="`expr $nerrors + 1`"
	test yes = "$verbose" && $diff $expect $actual |sed 's/^/    /'
    fi

    # Clean up output file
    rm -f $expect-norm $actual-norm
    if [ X = ${HDF5_NOCLEANUP:-X} ]; then
	rm -f $actual
    fi
}



##############################################################################
##############################################################################
###			  T H E   T E S T S                                ###
##############################################################################
##############################################################################

DUMP tgroup-1.ddl tgroup.h5
DUMP tgroup-2.ddl -g / tgroup.h5
DUMP tgroup-3.ddl -g /g2 /y tgroup.h5

DUMP tdset-1.ddl tdset.h5
DUMP tdset-2.ddl -d dset1 /dset2 tdset.h5
DUMP tdset-3.ddl -d /dset1 -header tdset.h5
DUMP tdset-4.ddl -d dset3 tdset.h5

DUMP tattr-1.ddl tattr.h5
DUMP tattr-2.ddl -a attr1 attr3 tattr.h5
DUMP tattr-3.ddl -header -a attr2 tattr.h5
DUMP tattr-4.ddl -a attr4 tattr.h5

DUMP tslink-1.ddl tslink.h5
DUMP tslink-2.ddl -l slink2 tslink.h5

DUMP thlink-1.ddl thlink.h5
DUMP thlink-2.ddl -d /g1/link2 /dset /g1/link1/link3 thlink.h5
DUMP thlink-3.ddl -d /dset /g1/link1/link3 /g1/link2 thlink.h5
DUMP thlink-4.ddl -g /g1 thlink.h5
DUMP thlink-5.ddl -d /dset -g /g2 -d /g1/link2 thlink.h5

DUMP tcomp-1.ddl tcompound.h5
DUMP tcomp-2.ddl -t /type1 /type2 /group1/type3 tcompound.h5
DUMP tcomp-3.ddl -d /group2/dset5 -g /group1 tcompound.h5
DUMP tcomp-4.ddl -t /#3432:0 -g /group2 tcompound.h5

DUMP tall-1.ddl tall.h5
DUMP tall-2.ddl -header -g /g1/g1.1 -a attr2 tall.h5
DUMP tall-3.ddl -d /g2/dset2.1 -l /g1/g1.2/g1.2.1/slink tall.h5

