# InterstellarOutpost executable and runtime wiring

file(GLOB app_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB app_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(${APP_CLIENT} WIN32 ${app_sources} ${app_headers})

target_link_libraries(${APP_CLIENT}
    PRIVATE
        InterstellarOutpost::NeuronCore
        InterstellarOutpost::NeuronClient
        InterstellarOutpost::GameLogic
)

target_include_directories(${APP_CLIENT}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

ApplyTargetDefaults(${APP_CLIENT})
EnablePch(${APP_CLIENT} "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

set(_io_client_output_dir "${PROJECT_SOURCE_DIR}/bin/Client")
set_target_properties(${APP_CLIENT} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${_io_client_output_dir}"
)

if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${config}" config_upper)
        set_target_properties(${APP_CLIENT} PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${_io_client_output_dir}/${config}"
        )
    endforeach()
endif()

if(MSVC)
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(config ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${config}" config_upper)
            set_property(TARGET ${APP_CLIENT} PROPERTY
                VS_DEBUGGER_WORKING_DIRECTORY_${config_upper} "${_io_client_output_dir}/${config}")
        endforeach()
    else()
        set_property(TARGET ${APP_CLIENT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${_io_client_output_dir}")
    endif()
endif()

set_target_properties(${APP_CLIENT} PROPERTIES
    OUTPUT_NAME "${APP_CLIENT_OUTPUT_NAME}"
    FOLDER "Applications"
)

install(TARGETS ${APP_CLIENT}
    EXPORT ${INTERSTELLAROUTPOST_EXPORT_NAME}
    RUNTIME DESTINATION ${INTERSTELLAROUTPOST_INSTALL_CLIENT_BINDIR}
    COMPONENT Client
)
