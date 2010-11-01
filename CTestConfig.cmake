## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)
set(CTEST_PROJECT_NAME "HDF5.1.8_EXT")
set(CTEST_NIGHTLY_START_TIME "20:00:00 CST")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "nei.hdfgroup.uiuc.edu")
set(CTEST_DROP_LOCATION "/cdash/submit.php?project=HDF5.1.8_EXT")
set(CTEST_DROP_SITE_CDASH TRUE)

set(UPDATE_TYPE svn)

set(VALGRIND_COMMAND "/usr/bin/valgrind")
set(VALGRIND_COMMAND_OPTIONS "--tool=memcheck")
set(CTEST_MEMORYCHECK_COMMAND "/usr/bin/valgrind")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--tool=memcheck")
set(CTEST_TESTING_TIMEOUT 3600)
