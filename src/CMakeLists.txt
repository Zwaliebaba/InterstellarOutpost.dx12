# InterstellarOutpost executable and runtime wiring

file(GLOB app_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB app_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(${PROJECT_NAME} WIN32 ${app_sources} ${app_headers})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        InterstellarOutpost::NeuronCore
        InterstellarOutpost::NeuronClient
        InterstellarOutpost::GameLogic
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

io_apply_target_defaults(${PROJECT_NAME})
io_enable_warnings(${PROJECT_NAME})
io_enable_pch(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

if(MSVC)
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(config ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${config}" config_upper)
            set_property(TARGET ${PROJECT_NAME} PROPERTY
                VS_DEBUGGER_WORKING_DIRECTORY_${config_upper} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}}")
        endforeach()
    else()
        set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "InterstellarOutpost"
    DEBUG_POSTFIX "_d"
    FOLDER "Applications"
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Applications
)

if(EXISTS "${GAMEDATA_SRC_DIR}")
    install(DIRECTORY "${GAMEDATA_SRC_DIR}"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Data
    )
endif()