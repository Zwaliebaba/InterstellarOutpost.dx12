# Main Application
# InterstellarOutpost executable

# Collect source files
file(GLOB APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB APP_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Create main executable with WIN32 flag for Windows GUI subsystem
add_executable(${PROJECT_NAME} WIN32
    ${APP_SOURCES}
    ${APP_HEADERS}
)

# Link libraries using alias targets
target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        InterstellarOutpost::NeuronCore
        InterstellarOutpost::NeuronClient
        InterstellarOutpost::GameLogic
)

# PIX setup is provided via NeuronCore (PUBLIC link + HAVE_PIX)

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_SOURCE_DIR}/libs/NeuronCore
      ${CMAKE_SOURCE_DIR}/libs/NeuronClient
      ${CMAKE_SOURCE_DIR}/libs/GameLogic
)

# Precompiled headers
if(ENABLE_PCH AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
    target_precompile_headers(${PROJECT_NAME}
 PRIVATE
      "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>"
    )
endif()

# Set C++23 standard per target
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

# Apply compiler warnings
set_project_warnings(${PROJECT_NAME})

# Platform-specific settings
    # For each configuration, set the specific VS working directory to the target output dir
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIG_UPPER)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin/${CONFIG}"
        )
    endforeach()

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "InterstellarOutpost"
    DEBUG_POSTFIX "_d"
    FOLDER "Applications"
)

# Installation rules (optional)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT Applications
)

if(EXISTS "${GAMEDATA_SOURCE_DIR}")
    install(DIRECTORY "${GAMEDATA_SOURCE_DIR}"
        DESTINATION bin
        COMPONENT Data
    )
endif()