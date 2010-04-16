# runTest.cmake executes a command and captures the output in a file. File is then compared
# against a reference file. Exit status of command can also be compared.

# arguments checking
IF (NOT TEST_PROGRAM)
  MESSAGE (FATAL_ERROR "Require TEST_PROGRAM to be defined")
ENDIF (NOT TEST_PROGRAM)
IF (NOT TEST_ARGS)
  MESSAGE (FATAL_ERROR "Require TEST_ARGS to be defined")
ENDIF (NOT TEST_ARGS)
IF (NOT TEST_FOLDER)
  MESSAGE ( FATAL_ERROR "Require TEST_FOLDER to be defined")
ENDIF (NOT TEST_FOLDER)
IF (NOT TEST_OUTPUT)
  MESSAGE (FATAL_ERROR "Require TEST_OUTPUT to be defined")
ENDIF (NOT TEST_OUTPUT)
#IF (NOT TEST_EXPECT)
#  MESSAGE (STATUS "Require TEST_EXPECT to be defined")
#ENDIF (NOT TEST_EXPECT)
IF (NOT TEST_REFERENCE)
  MESSAGE (FATAL_ERROR "Require TEST_REFERENCE to be defined")
ENDIF (NOT TEST_REFERENCE)

SET (arg_list)
FOREACH (arg ${TEST_ARGS})
  SET (arg_list ${arg_list} ${arg})
ENDFOREACH (arg ${TEST_ARGS})

MESSAGE (STATUS "COMMAND: ${TEST_PROGRAM} ${arg_list}")

# run the test program, capture the stdout/stderr and the result var
EXECUTE_PROCESS (
    COMMAND ${TEST_PROGRAM} ${arg_list}
    WORKING_DIRECTORY ${TEST_FOLDER}
    RESULT_VARIABLE TEST_RESULT
    OUTPUT_FILE ${TEST_OUTPUT}
    ERROR_FILE ${TEST_OUTPUT}
    OUTPUT_VARIABLE TEST_ERROR
    ERROR_VARIABLE TEST_ERROR
)

MESSAGE (STATUS "COMMAND Result: ${TEST_RESULT}")

# if the return value is !=0 bail out
IF (NOT ${TEST_RESULT} STREQUAL ${TEST_EXPECT})
  MESSAGE ( FATAL_ERROR "Failed: Test program ${TEST_PROGRAM} exited != 0.\n${TEST_ERROR}")
ENDIF (NOT ${TEST_RESULT} STREQUAL ${TEST_EXPECT})

MESSAGE (STATUS "COMMAND Error: ${TEST_ERROR}")

# now compare the output with the reference
EXECUTE_PROCESS (
    COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_OUTPUT} ${TEST_REFERENCE}
    RESULT_VARIABLE TEST_RESULT
)

# again, if return value is !=0 scream and shout
IF (TEST_RESULT)
  MESSAGE (FATAL_ERROR "Failed: The output of ${TEST_PROGRAM} did not match ${TEST_REFERENCE}")
ENDIF (TEST_RESULT)

# everything went fine...
MESSAGE ("Passed: The output of ${TEST_PROGRAM} matches ${TEST_REFERENCE}")

