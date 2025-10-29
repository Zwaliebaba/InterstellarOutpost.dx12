cmake_minimum_required(VERSION 3.20)

# Project configuration
project(InterstellarOutpost 
    VERSION 1.0.0
    DESCRIPTION "An interstellar outpost management system"
    LANGUAGES CXX
)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Visual Studio specific settings
add_compile_options(/W4)
# Enable C++20++ features for Visual Studio
add_compile_options(/std:c++latest)
add_compile_options(/permissive)

# Build configuration
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")

# Add libraries
add_subdirectory(libs/Neuron)
add_subdirectory(libs/GameLogic)

# Add main executable
add_subdirectory(src)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Print configuration summary
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")