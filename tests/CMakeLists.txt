# ======================================================================
# Test Configuration for Interstellar Outpost
# ======================================================================
# This file defines the test infrastructure for the project.
#
# Test Organization:
#   - tests/unit/         : Fast, isolated tests of individual classes/functions
#   - tests/integration/  : Multi-component tests requiring resources/subsystems
#   - tests/common/       : Shared test utilities (TestHarness.h)
#
# Test Labels:
#   - unit         : Quick unit tests (run on every PR/push)
#   - integration  : Slower integration tests (may require GPU, file I/O)
#   - <module>     : Module-specific label (e.g., neuroncore, gamelogic)
#
# Usage:
#   add_project_test(test_name
#     SOURCES test.cpp
#     LIBRARIES InterstellarOutpost::NeuronCore
#     LABELS unit neuroncore
#     TIMEOUT 30
#   )
# ======================================================================

set(TEST_TIMEOUT_SHORT 30)
set(TEST_TIMEOUT_LONG 120)
set(TEST_TIMEOUT_INTEGRATION 180)

# Helper function to add a test executable and register it with CTest.
#
# Parameters:
#   TEST_NAME   : Name of the test executable and CTest test
#   SOURCES     : List of source files for the test
#   LIBRARIES   : List of libraries to link against
#   LABELS      : List of CTest labels (e.g., "unit" "integration" "neuroncore")
#   TIMEOUT     : Test timeout in seconds (default: TEST_TIMEOUT_SHORT)
#
# The test executable is built with the same compiler settings as the main
# project and runs with its working directory set to the target output directory
# (where gamedata/ is mirrored) to ensure asset availability.
function(add_project_test TEST_NAME)
  set(options)
  set(oneValueArgs TIMEOUT)
  set(multiValueArgs SOURCES LIBRARIES LABELS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT ARG_SOURCES)
    message(FATAL_ERROR "add_project_test(${TEST_NAME}) requires SOURCES")
  endif()

  # Create test executable
  add_executable(${TEST_NAME} ${ARG_SOURCES})
  
  # Link against test utilities and specified libraries
  # Note: Using PROJECT_SOURCE_DIR to ensure correct path from any subdirectory
  target_include_directories(${TEST_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/tests/common
  )
  target_link_libraries(${TEST_NAME} PRIVATE ${ARG_LIBRARIES})

  # Apply standard project settings (C++23, warnings, etc.)
  io_apply_target_defaults(${TEST_NAME})
  io_enable_warnings(${TEST_NAME})

  # Organize in IDE
  set_target_properties(${TEST_NAME} PROPERTIES FOLDER "Tests")

  # Register with CTest - run from target output directory so gamedata/ resolves
  add_test(NAME ${TEST_NAME}
    COMMAND ${CMAKE_COMMAND} -E chdir "$<TARGET_FILE_DIR:${TEST_NAME}>" "$<TARGET_FILE:${TEST_NAME}>"
  )

  # Set timeout
  if(ARG_TIMEOUT)
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT ${ARG_TIMEOUT})
  else()
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT ${TEST_TIMEOUT_SHORT})
  endif()

  # Apply labels for filtering
  if(ARG_LABELS)
    set_tests_properties(${TEST_NAME} PROPERTIES LABELS "${ARG_LABELS}")
  endif()
endfunction()

# Add test subdirectories
add_subdirectory(common)
add_subdirectory(unit)
add_subdirectory(integration)