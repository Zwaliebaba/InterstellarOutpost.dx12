# Standard Project Settings
# Common CMake settings and options for the project

# compile_commands.json is enabled in the root CMakeLists.txt

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    # Set possible values for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Enable folders in IDE for better organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Make output directories configuration-aware for multi-config generators
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

# Project options
option(ENABLE_PCH "Enable precompiled headers" ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Platform detection
add_compile_definitions(PLATFORM_WINDOWS)

    add_compile_options(/permissive-)

    # Use Windows Unicode APIs by default and NOT MBCS/UTF-8 char API mappings
    add_compile_definitions(UNICODE _UNICODE)

    # Ensure resource compiler also uses Unicode
    string(APPEND CMAKE_RC_FLAGS " /DUNICODE /D_UNICODE")

message(STATUS "=================================================")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")
message(STATUS "  PCH: ${ENABLE_PCH}")
message(STATUS "=================================================")
