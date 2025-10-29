# Main Application
# InterstellarOutpost executable

# Collect source files
file(GLOB APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB APP_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Create main executable with WIN32 flag for Windows GUI subsystem
add_executable(${PROJECT_NAME} WIN32
    ${APP_SOURCES}
    ${APP_HEADERS}
)

# Link libraries using alias targets
target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        InterstellarOutpost::Neuron
        InterstellarOutpost::GameLogic
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}
)

# Precompiled headers
if(ENABLE_PCH AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
    target_precompile_headers(${PROJECT_NAME}
 PRIVATE
      "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>"
    )
endif()

# Set C++20 standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Apply compiler warnings
set_project_warnings(${PROJECT_NAME})

# Platform-specific settings
if(MSVC)
    # Set subsystem for Windows application (GUI)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup"
    )
    
    # Set Visual Studio debugging working directory to the output binary directory
# For multi-config generators, this will be automatically adjusted per configuration
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    # For each configuration, set the specific working directory
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIG_UPPER)
      set_target_properties(${PROJECT_NAME} PROPERTIES
      VS_DEBUGGER_WORKING_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin/${CONFIG}"
        )
    endforeach()
endif()

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "InterstellarOutpost"
    DEBUG_POSTFIX "_d"
    FOLDER "Applications"
)

# Copy gamedata directory to output directory after build
set(GAMEDATA_SOURCE_DIR "${CMAKE_SOURCE_DIR}/gamedata")
if(EXISTS "${GAMEDATA_SOURCE_DIR}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
   "${GAMEDATA_SOURCE_DIR}"
   "$<TARGET_FILE_DIR:${PROJECT_NAME}>/gamedata"
        COMMENT "Copying gamedata directory to output..."
        VERBATIM
    )
else()
    message(WARNING "gamedata directory not found at ${GAMEDATA_SOURCE_DIR}")
endif()

# IDE source grouping
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PREFIX "Source Files" 
    FILES ${APP_SOURCES}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PREFIX "Header Files" 
    FILES ${APP_HEADERS}
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