# CMakeList.txt : Top-level CMake project file, do global configuration
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



