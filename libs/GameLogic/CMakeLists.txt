# GameLogic Library
# Game logic and world object management

# Collect source files by subdirectory
file(GLOB GAMELOGIC_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB GAMELOGIC_CORE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB GAMELOGIC_INTERFACE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.cpp")
file(GLOB GAMELOGIC_INTERFACE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h")
file(GLOB GAMELOGIC_WORLDOBJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/WorldObject/*.cpp")
file(GLOB GAMELOGIC_WORLDOBJECT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/WorldObject/*.h")

# Combine all sources
set(GAMELOGIC_SOURCES
    ${GAMELOGIC_CORE_SOURCES}
    ${GAMELOGIC_INTERFACE_SOURCES}
    ${GAMELOGIC_WORLDOBJECT_SOURCES}
)

set(GAMELOGIC_HEADERS
    ${GAMELOGIC_CORE_HEADERS}
    ${GAMELOGIC_INTERFACE_HEADERS}
    ${GAMELOGIC_WORLDOBJECT_HEADERS}
)

# Create GameLogic static library
add_library(GameLogic STATIC
    ${GAMELOGIC_SOURCES}
    ${GAMELOGIC_HEADERS}
)

# Create library alias for consistent namespacing
add_library(InterstellarOutpost::GameLogic ALIAS GameLogic)

# Public include directories
target_include_directories(GameLogic
  PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/GameLogic>
    PRIVATE
      # GameLogic needs access to src headers
    ${CMAKE_SOURCE_DIR}/src
)

# Precompiled headers
if(ENABLE_PCH AND EXISTS "${CMAKE_SOURCE_DIR}/src/pch.h")
    target_precompile_headers(GameLogic
        PRIVATE
            "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/pch.h>"
    )
endif()

# Set C++20 standard
target_compile_features(GameLogic PUBLIC cxx_std_20)

# Link dependencies using alias
target_link_libraries(GameLogic 
    PUBLIC 
      InterstellarOutpost::Neuron
)

# Apply compiler warnings
set_project_warnings(GameLogic)

# Set target properties
set_target_properties(GameLogic PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    FOLDER "Libraries"
    POSITION_INDEPENDENT_CODE ON
)

# IDE source grouping for better organization
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PREFIX "Source Files" 
    FILES ${GAMELOGIC_SOURCES}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PREFIX "Header Files" 
    FILES ${GAMELOGIC_HEADERS}
)