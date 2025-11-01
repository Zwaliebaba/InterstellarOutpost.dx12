# Tests for runtime subsystems and gameplay layers

set(TEST_TIMEOUT_SHORT 30)
set(TEST_TIMEOUT_LONG 60)

function(add_project_test TEST_NAME)
  set(options)
  set(oneValueArgs TIMEOUT)
  set(multiValueArgs SOURCES LIBRARIES)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${TEST_NAME} ${ARG_SOURCES})
  target_link_libraries(${TEST_NAME} PRIVATE ${ARG_LIBRARIES})
  target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  io_apply_target_defaults(${TEST_NAME})
  io_enable_warnings(${TEST_NAME})

  set_target_properties(${TEST_NAME} PROPERTIES FOLDER "Tests")

  add_test(NAME ${TEST_NAME}
    COMMAND ${CMAKE_COMMAND} -E chdir "$<TARGET_FILE_DIR:${TEST_NAME}>" "$<TARGET_FILE:${TEST_NAME}>"
  )

  if(ARG_TIMEOUT)
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT ${ARG_TIMEOUT})
  else()
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT ${TEST_TIMEOUT_SHORT})
  endif()
endfunction()

add_project_test(test_neuron_core
  SOURCES test_neuron_core.cpp
  LIBRARIES InterstellarOutpost::NeuronCore
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

add_project_test(test_neuron_client
  SOURCES test_neuron_client.cpp
  LIBRARIES InterstellarOutpost::NeuronClient
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

add_project_test(test_gamelogic
  SOURCES test_gamelogic.cpp
  LIBRARIES
    InterstellarOutpost::GameLogic
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

add_project_test(test_integration
  SOURCES test_integration.cpp
  LIBRARIES
    InterstellarOutpost::GameLogic
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
  TIMEOUT ${TEST_TIMEOUT_LONG}
)

add_custom_target(run_tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel ${CMAKE_BUILD_PARALLEL_LEVEL} -C $<CONFIG>
  DEPENDS test_neuron_core test_neuron_client test_gamelogic test_integration
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running all tests"
)

set_target_properties(run_tests PROPERTIES FOLDER "Tests")