# Neuron Library
# Core neuron system and input/output handling

# Collect source files explicitly by subdirectory for better maintainability
set(NEURON_SOURCES
    # Core sources (list explicitly when possible, or use GLOB as fallback)
)

set(NEURON_HEADERS
    # Core headers
)

# Use GLOB for subdirectories (consider listing files explicitly for production)
file(GLOB NEURON_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB NEURON_CORE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB NEURON_CANVAS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Canvas/*.cpp")
file(GLOB NEURON_CANVAS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Canvas/*.h")
file(GLOB NEURON_INPUT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Input/*.cpp")
file(GLOB NEURON_INPUT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Input/*.h")
file(GLOB NEURON_NETLIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Netlib/*.cpp")
file(GLOB NEURON_NETLIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Netlib/*.h")
file(GLOB NEURON_NETWORK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Network/*.cpp")
file(GLOB NEURON_NETWORK_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Network/*.h")
file(GLOB NEURON_SOUND_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Sound/*.cpp")
file(GLOB NEURON_SOUND_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Sound/*.h")

# Combine all sources
list(APPEND NEURON_SOURCES
    ${NEURON_CORE_SOURCES}
    ${NEURON_CANVAS_SOURCES}
    ${NEURON_INPUT_SOURCES}
    ${NEURON_NETLIB_SOURCES}
    ${NEURON_NETWORK_SOURCES}
    ${NEURON_SOUND_SOURCES}
)

list(APPEND NEURON_HEADERS
    ${NEURON_CORE_HEADERS}
    ${NEURON_CANVAS_HEADERS}
    ${NEURON_INPUT_HEADERS}
    ${NEURON_NETLIB_HEADERS}
    ${NEURON_NETWORK_HEADERS}
    ${NEURON_SOUND_HEADERS}
)

# Create Neuron static library
add_library(Neuron STATIC
    ${NEURON_SOURCES}
    ${NEURON_HEADERS}
)

# Create library alias for consistent namespacing
add_library(InterstellarOutpost::Neuron ALIAS Neuron)

# Public include directories
target_include_directories(Neuron
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/Neuron>
    PRIVATE
   # Neuron needs access to src and GameLogic headers due to cross-dependencies
    ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs/GameLogic
)

# Precompiled headers
if(ENABLE_PCH AND EXISTS "${CMAKE_SOURCE_DIR}/src/pch.h")
    target_precompile_headers(Neuron
        PRIVATE
         "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/pch.h>"
    )
endif()

# Set C++20 standard
target_compile_features(Neuron PUBLIC cxx_std_20)

# Apply compiler warnings
set_project_warnings(Neuron)

# Set target properties
set_target_properties(Neuron PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    FOLDER "Libraries"
    POSITION_INDEPENDENT_CODE ON
)

# IDE source grouping for better organization
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PREFIX "Source Files" 
    FILES ${NEURON_SOURCES}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PREFIX "Header Files" 
    FILES ${NEURON_HEADERS}
)