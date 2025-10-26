# Tests CMakeLists.txt

# Find or include a testing framework
# For simplicity, we'll use basic C++ assertions

# Test for Neuron library
add_executable(test_neuron
    test_neuron.cpp
)

target_include_directories(test_neuron
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/Neuron>
)

target_link_libraries(test_neuron 
    PRIVATE 
        Neuron
)

target_compile_features(test_neuron PRIVATE cxx_std_20)

# Test for GameLogic library  
add_executable(test_gamelogic
    test_gamelogic.cpp
)

target_include_directories(test_gamelogic
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/GameLogic>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/Neuron>
)

target_link_libraries(test_gamelogic 
    PRIVATE 
        GameLogic
        Neuron
)

target_compile_features(test_gamelogic PRIVATE cxx_std_20)

# Integration test
add_executable(test_integration
    test_integration.cpp
)

target_include_directories(test_integration
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/GameLogic>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/Neuron>
)

target_link_libraries(test_integration 
    PRIVATE 
        GameLogic
        Neuron
)

target_compile_features(test_integration PRIVATE cxx_std_20)

# Register tests with CTest
add_test(NAME NeuronLibraryTest COMMAND test_neuron)
add_test(NAME GameLogicLibraryTest COMMAND test_gamelogic)
add_test(NAME IntegrationTest COMMAND test_integration)

# Set test properties
set_tests_properties(NeuronLibraryTest PROPERTIES
    TIMEOUT 30
)

set_tests_properties(GameLogicLibraryTest PROPERTIES
    TIMEOUT 30
)

set_tests_properties(IntegrationTest PROPERTIES
    TIMEOUT 60
)

# Visual Studio specific settings
if(MSVC)
    target_compile_options(test_neuron PRIVATE /W4)
    target_compile_options(test_gamelogic PRIVATE /W4)
    target_compile_options(test_integration PRIVATE /W4)
else()
    target_compile_options(test_neuron PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(test_gamelogic PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(test_integration PRIVATE -Wall -Wextra -Wpedantic)
endif()