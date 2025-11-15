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

target_link_options(${APP_CLIENT} PRIVATE ${LINKER_SWITCHES})

if(MSVC)
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(config ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${config}" config_upper)
            set_property(TARGET ${APP_CLIENT} PROPERTY
                VS_DEBUGGER_WORKING_DIRECTORY_${config_upper} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}}")
        endforeach()
    else()
        set_property(TARGET ${APP_CLIENT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
endif()

set_target_properties(${APP_CLIENT} PROPERTIES
    OUTPUT_NAME "InterstellarOutpost"
    DEBUG_POSTFIX "_d"
    FOLDER "Applications"
)

install(TARGETS ${APP_CLIENT}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Applications
)

if(EXISTS "${GAMEDATA_SRC_DIR}")
    install(DIRECTORY "${GAMEDATA_SRC_DIR}"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Data
    )
endif()
