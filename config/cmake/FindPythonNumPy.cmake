# - Find NumPy
# Find the native numpy includes
# This module defines
#  PYTHON_NUMPY_INCLUDE_DIR, where to find numpy/arrayobject.h, etc.
#  PYTHON_NUMPY_FOUND, If false, do not try to use numpy headers.

IF (PYTHON_NUMPY_INCLUDE_DIR)
  # in cache already
  SET (PYTHON_NUMPY_FIND_QUIETLY TRUE)
ENDIF (PYTHON_NUMPY_INCLUDE_DIR)

INCLUDE (FindPythonInterp)

IF (PYTHON_EXECUTABLE)
  EXEC_PROGRAM ("${PYTHON_EXECUTABLE}"
    ARGS "-c \"import numpy; print numpy.get_include()\""
    OUTPUT_VARIABLE PYTHON_NUMPY_INCLUDE_PATH
    RETURN_VALUE PYTHON_NUMPY_NOT_FOUND)

  IF (NOT ${PYTHON_NUMPY_INCLUDE_PATH} MATCHES "Traceback")
    SET (PYTHON_NUMPY_INCLUDE_DIR ${PYTHON_NUMPY_INCLUDE_PATH} CACHE STRING "Numpy include path.")
  ENDIF (NOT ${PYTHON_NUMPY_INCLUDE_PATH} MATCHES "Traceback")
ENDIF(PYTHON_EXECUTABLE)

INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (PYTHON_NUMPY DEFAULT_MSG PYTHON_NUMPY_INCLUDE_DIR)

MARK_AS_ADVANCED (PYTHON_NUMPY_INCLUDE_DIR)
