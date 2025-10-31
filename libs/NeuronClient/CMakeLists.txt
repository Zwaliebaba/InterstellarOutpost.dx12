# NeuronClient Library
# Core neuron system and input/output handling

# Collect source files explicitly by subdirectory for better maintainability
set(NEURON_SOURCES
    # Core sources (list explicitly when possible, or use GLOB as fallback)
)

set(NEURON_HEADERS
    # Core headers
)

# Use GLOB for subdirectories (consider listing files explicitly for production)
file(GLOB NEURON_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB NEURON_CORE_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Combine all sources
list(APPEND NEURON_SOURCES
    ${NEURON_CORE_SOURCES}
)

list(APPEND NEURON_HEADERS
    ${NEURON_CORE_HEADERS}
)

# Create NeuronClient library (respects BUILD_SHARED_LIBS)
add_library(NeuronClient
    ${NEURON_SOURCES}
    ${NEURON_HEADERS}
)

# Create library alias for consistent namespacing
add_library(InterstellarOutpost::NeuronClient ALIAS NeuronClient)

# Public include directories
target_include_directories(NeuronClient
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>

    ${CMAKE_SOURCE_DIR}/libs/NeuronCore
     
   # NeuronClient needs access to src and GameLogic headers due to cross-dependencies
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_SOURCE_DIR}/libs/GameLogic
)

# Precompiled headers
if(ENABLE_PCH AND EXISTS "${CMAKE_SOURCE_DIR}/src/pch.h")
    target_precompile_headers(NeuronClient
        PRIVATE
         "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/pch.h>"
    )
endif()

# Set C++23 standard
target_compile_features(NeuronClient PUBLIC cxx_std_23)

# Apply compiler warnings
set_project_warnings(NeuronClient)

# Set target properties
set_target_properties(NeuronClient PROPERTIES
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