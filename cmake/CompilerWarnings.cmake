# ==============================================================================
# Compiler Warning Configuration
# ==============================================================================
# 
# This module provides standardized MSVC compiler warning settings for the 
# InterstellarOutpost project. It ensures consistent warning levels across
# all targets for Windows development with Visual Studio.
#
# Key Features:
# - High warning levels for catching potential issues early
# - Optional warnings-as-errors mode for CI/CD pipelines
# - MSVC-optimized warning configuration
# - Per-target configuration flexibility
#
# Usage:
#   include(CompilerWarnings)
#   set_project_warnings(my_target)
#
# Options:
#   WARNINGS_AS_ERRORS - Set to ON to treat warnings as compilation errors
#                        (default: OFF, recommended ON for CI builds)
#
# ==============================================================================

#[[
Set comprehensive warning flags for a target

This function applies a standardized set of compiler warnings to the specified
target. The warnings are designed to catch common C++ issues while avoiding
overly pedantic warnings that would hinder development productivity.

Arguments:
  target_name - Name of the CMake target to apply warnings to

MSVC Warning Flags:
  /W4           - Enable level 4 warnings (high warning level)
  /permissive-  - Disable non-conforming code extensions
  /WX           - Treat warnings as errors (when WARNINGS_AS_ERRORS is ON)
  
Additional specific warnings target common C++ issues like data loss,
virtual function safety, and modern C++ compliance.

Example:
  add_executable(game_engine main.cpp)
  set_project_warnings(game_engine)
]]
function(set_project_warnings target_name)
    # Validation
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR 
            "set_project_warnings(): Target '${target_name}' does not exist. "
            "Make sure to call add_executable() or add_library() first.")
    endif()

    # Global option to control warnings-as-errors behavior
    # Recommended: OFF for development, ON for CI/CD builds
    option(WARNINGS_AS_ERRORS 
           "Treat compiler warnings as compilation errors" 
           FALSE)

    # MSVC warning configuration
    set(PROJECT_WARNINGS
        /W4              # Enable level 4 warnings (comprehensive)
        /permissive-     # Disable Microsoft extensions for better standards compliance
        /w14242          # 'identifier': conversion from 'type1' to 'type2', possible data loss
        /w14254          # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits'
        /w14263          # 'function': member function does not override any base class virtual member function
        /w14265          # 'classname': class has virtual functions, but destructor is not virtual
        /w14287          # 'operator': unsigned/negative constant mismatch
        /we4289          # nonstandard extension used: 'variable': loop control variable is used outside the for-loop scope
        /w14296          # 'operator': expression is always 'boolean_value'
        /w14311          # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545          # expression before comma evaluates to a function which is missing an argument list
        /w14546          # function call before comma missing argument list
        /w14547          # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549          # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555          # expression has no effect; expected expression with side-effect
        /w14619          # pragma warning: there is no warning number 'number'
        /w14640          # Enable warning on thread un-safe static member initialization
        /w14826          # Conversion from 'type1' to 'type2' is sign-extended
        /w14905          # wide string literal cast to 'LPSTR'
        /w14906          # string literal cast to 'LPWSTR'
        /w14928          # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
    )

    # Add warnings-as-errors flag if requested
    if(WARNINGS_AS_ERRORS)
        list(APPEND PROJECT_WARNINGS /WX)
    endif()

    # Apply the warning flags to the target
    target_compile_options(${target_name} PRIVATE ${PROJECT_WARNINGS})

    # Log the applied warnings for debugging
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS 
            "Applied warning flags to '${target_name}': ${PROJECT_WARNINGS}")
    endif()
endfunction()
