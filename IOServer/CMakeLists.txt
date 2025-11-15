# InterstellarOutpost Server executable and runtime wiring

file(GLOB app_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB app_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(${APP_SERVER} ${app_sources} ${app_headers})

target_link_libraries(${APP_SERVER}
    PRIVATE
        InterstellarOutpost::NeuronCore
        InterstellarOutpost::NeuronServer
        InterstellarOutpost::GameLogic
)

target_include_directories(${APP_SERVER}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

ApplyTargetDefaults(${APP_SERVER})
EnablePch(${APP_SERVER} "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

if(MSVC)
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(config ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${config}" config_upper)
            set_property(TARGET ${APP_SERVER} PROPERTY
                VS_DEBUGGER_WORKING_DIRECTORY_${config_upper} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}}")
        endforeach()
    else()
        set_property(TARGET ${APP_SERVER} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
endif()

set_target_properties(${APP_SERVER} PROPERTIES
    OUTPUT_NAME "${APP_SERVER_OUTPUT_NAME}"
    FOLDER "Applications"
)

install(TARGETS ${APP_SERVER}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Applications
)
