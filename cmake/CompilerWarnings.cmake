# Compiler Warning Configuration
# Sets comprehensive warning levels for different compilers

function(set_project_warnings target_name)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" FALSE)

    set(MSVC_WARNINGS
        /W4      # Baseline reasonable warnings
        # Removed /permissive- to allow legacy code patterns
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
    )

    if(WARNINGS_AS_ERRORS)
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
   set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
    endif()

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
    )

    if(MSVC)
 set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
     set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else()
        message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif()

    target_compile_options(${target_name} PRIVATE ${PROJECT_WARNINGS})
endfunction()
