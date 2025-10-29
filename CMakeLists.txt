cmake_minimum_required(VERSION 3.20)

# Project configuration
project(InterstellarOutpost 
    VERSION 1.0.0
    DESCRIPTION "An interstellar outpost management system"
    LANGUAGES CXX
)

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
add_subdirectory(libs/Neuron)
add_subdirectory(libs/GameLogic)

# Add main executable
add_subdirectory(src)

# Enable testing if requested
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation and packaging
include(InstallConfig)