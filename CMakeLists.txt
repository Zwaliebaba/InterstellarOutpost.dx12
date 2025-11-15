<<<<<<< HEAD
﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(INTERSTELLAROUTPOST_VERSION 1.0.0)

project ("Interstellar Outpost"
  VERSION ${INTERSTELLAROUTPOST_VERSION}
  DESCRIPTION "Interstellar Outpost"
  LANGUAGES CXX)

# https://devblogs.microsoft.com/cppblog/spectre-mitigations-in-msvc/
option(ENABLE_SPECTRE_MITIGATION "Build using /Qspectre for MSVC" OFF)

# Disable iterator debugging in Debug configurations with the MSVC CRT
option(DISABLE_MSVC_ITERATOR_DEBUGGING "Disable iterator debugging in Debug configurations with the MSVC CRT" OFF)

# Enable Static Code Analysis
option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

# Enable Code Coverage
option(ENABLE_CODE_COVERAGE "Build with code-coverage" OFF)

# Enable Code Profiling
option(ENABLE_CODE_PROFILING "Build for profiling" OFF)

# Enable Fuzz Testing
option(BUILD_FUZZING "Build for fuzz testing" OFF)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories for binaries and libraries.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(build/CompilerAndLinker.cmake)

# Include sub-projects.
add_subdirectory ("IOClient")
add_subdirectory ("IOServer")
add_subdirectory ("Lib/NeuronCore")
add_subdirectory ("Lib/NeuronClient")
add_subdirectory ("Lib/NeuronServer")



=======
cmake_minimum_required(VERSION 3.21)

project(InterstellarOutpost
    VERSION 1.0.0
    DESCRIPTION "An interstellar Outpost"
    LANGUAGES CXX
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "InterstellarOutpost currently builds with MSVC on Windows only.")
endif()

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(DEFINED CMAKE_GENERATOR_PLATFORM AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
    string(TOUPPER "${CMAKE_GENERATOR_PLATFORM}" _io_gen_platform)
    if(NOT _io_gen_platform MATCHES "^(X64|ARM64)$")
        message(FATAL_ERROR "Unsupported generator platform '${CMAKE_GENERATOR_PLATFORM}'. Supported: x64 or ARM64.")
    endif()
else()
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Unsupported target: a 64-bit build (x64 or ARM64) is required.")
    endif()
endif()

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not supported. Configure the project in a separate build directory.\n"
        "Remove CMakeCache.txt and the CMakeFiles directory if they were generated in the source tree.")
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(StandardProjectSettings)
include(CompilerWarnings)

# Testing Framework Configuration
# ================================
# This project uses a lightweight custom test harness (tests/common/TestHarness.h)
# for unit and integration testing. Tests are registered with CTest and labeled
# appropriately for filtering (unit vs integration).
#
# Enable/disable tests with: -DIO_BUILD_TESTS=ON/OFF (default: ON in debug builds)
# Run unit tests only: ctest -L unit
# Run integration tests: ctest -L integration
# Run all tests: ctest --output-on-failure

include(CTest)
option(IO_BUILD_TESTS "Build unit and integration test targets" ${BUILD_TESTING})

if(IO_BUILD_TESTS)
    set(BUILD_TESTING ON CACHE BOOL "Enable CTest targets" FORCE)
else()
    set(BUILD_TESTING OFF CACHE BOOL "Enable CTest targets" FORCE)
endif()

message(STATUS "=================================================")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  PCH enabled: ${IO_ENABLE_PCH}")
message(STATUS "  Warnings as errors: ${IO_WARNINGS_AS_ERRORS}")
message(STATUS "  Tests enabled: ${BUILD_TESTING}")
message(STATUS "=================================================")

add_subdirectory(libs/NeuronCore)
add_subdirectory(libs/NeuronClient)
add_subdirectory(libs/GameLogic)
add_subdirectory(src)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

set(GAMEDATA_SRC_DIR "${CMAKE_SOURCE_DIR}/gamedata")
if(CMAKE_CONFIGURATION_TYPES)
    set(_io_gamedata_dst "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
else()
    set(_io_gamedata_dst "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

add_custom_target(copy_gamedata ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_io_gamedata_dst}/gamedata"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GAMEDATA_SRC_DIR}" "${_io_gamedata_dst}/gamedata"
    COMMENT "Mirroring gamedata assets to ${_io_gamedata_dst}/gamedata"
)

# Installation and packaging configuration (must appear after defining install() rules).
include(InstallConfig)
>>>>>>> e23be089ba864cfa6ba9b2af265d0fb4d033df9d
