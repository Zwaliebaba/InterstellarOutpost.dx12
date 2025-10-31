cmake_minimum_required(VERSION 3.20)

# Sane global defaults
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project configuration
project(InterstellarOutpost 
    VERSION 1.0.0
    DESCRIPTION "An interstellar Outpost"
    LANGUAGES CXX
)

# Enforce Windows + MSVC toolchain only
if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "This project supports Windows with MSVC only.")
endif()

# Prefer explicit MSVC runtime selection via CMP0091 (picked up by presets or cache)
    cmake_policy(SET CMP0091 NEW)
    # Default to static runtime; adjust via presets if needed
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Enforce supported architectures: x64 and ARM64 only (Visual Studio generator)
if(DEFINED CMAKE_GENERATOR_PLATFORM AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
    string(TOUPPER "${CMAKE_GENERATOR_PLATFORM}" _GEN_PLATFORM)
    if(NOT _GEN_PLATFORM MATCHES "^(X64|ARM64)$")
        message(FATAL_ERROR "Unsupported generator platform '${CMAKE_GENERATOR_PLATFORM}'. Supported: x64, ARM64.")
    endif()
else()
    # Fallback: ensure 64-bit address size
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Unsupported platform: 64-bit build required (x64 or ARM64).")
    endif()
endif()

# Prevent in-source builds
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. Please create a separate build directory and run cmake from there.\n"
    "Remove CMakeCache.txt and CMakeFiles/ if they were created in the source directory.")
endif()

# Add cmake modules to the path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Include standard project settings
include(StandardProjectSettings)

# Include compiler warnings configuration
include(CompilerWarnings)

# Add libraries
add_subdirectory(libs/NeuronCore)
add_subdirectory(libs/NeuronClient)
add_subdirectory(libs/GameLogic)

# Add main executable
add_subdirectory(src)

# Standard CTest integration; enable with -DBUILD_TESTING=ON or via presets
include(CTest)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation and packaging
include(InstallConfig)

# Auto-copy gamedata into the config-specific runtime output (required for runtime and tests)
set(GAMEDATA_SRC_DIR ${CMAKE_SOURCE_DIR}/gamedata)
add_custom_target(copy_gamedata ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/gamedata
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${GAMEDATA_SRC_DIR}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/gamedata
    COMMENT "Copying gamedata to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/gamedata"
)