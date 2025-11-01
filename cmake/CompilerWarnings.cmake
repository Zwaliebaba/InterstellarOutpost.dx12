# Shared warning configuration for every target.

include_guard(GLOBAL)

option(IO_WARNINGS_AS_ERRORS "Treat compiler warnings as fatal" OFF)

function(io_enable_warnings target)
  if(NOT TARGET "${target}")
    message(FATAL_ERROR "io_enable_warnings: target '${target}' does not exist")
  endif()

    set(project_warnings
      /W4
      /w14242
      /w14254
      /w14263
      /w14265
      /w14287
      /we4289
      /w14296
      /w14311
      /w14545
      /w14546
      /w14547
      /w14549
      /w14555
      /w14619
      /w14640
      /w14826
      /w14905
      /w14906
      /w14928
    )

    if(IO_WARNINGS_AS_ERRORS)
      list(APPEND project_warnings /WX)
    endif()

  if(project_warnings)
    target_compile_options(${target} PRIVATE ${project_warnings})
  endif()
endfunction()
