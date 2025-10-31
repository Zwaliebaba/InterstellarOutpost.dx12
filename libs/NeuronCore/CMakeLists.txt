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
file(GLOB NEURON_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB NEURON_CORE_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Combine all sources
list(APPEND NEURON_SOURCES
    ${NEURON_CORE_SOURCES}
)

list(APPEND NEURON_HEADERS
    ${NEURON_CORE_HEADERS}
)

# Create Neuron library (respects BUILD_SHARED_LIBS)
add_library(NeuronCore
    ${NEURON_SOURCES}
    ${NEURON_HEADERS}
)

# Create library alias for consistent namespacing
add_library(InterstellarOutpost::NeuronCore ALIAS NeuronCore)

# Public include directories
target_include_directories(NeuronCore
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}

   # Neuron needs access to src and GameLogic headers due to cross-dependencies
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libs/GameLogic
    ${CMAKE_SOURCE_DIR}/libs/NeuronClient
)

# JSON dependency (header-only via vcpkg)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(NeuronCore PUBLIC nlohmann_json::nlohmann_json)

# Precompiled headers
if(ENABLE_PCH AND EXISTS "${CMAKE_SOURCE_DIR}/src/pch.h")
    target_precompile_headers(NeuronCore
        PRIVATE
         "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/pch.h>"
    )
endif()

# Set C++23 standard
target_compile_features(NeuronCore PUBLIC cxx_std_23)

# Apply compiler warnings
set_project_warnings(NeuronCore)

# Set target properties
set_target_properties(NeuronCore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    FOLDER "Libraries"
    POSITION_INDEPENDENT_CODE ON
)

# PIX GPU event markers via vcpkg (winpixevent port)
find_package(winpixevent CONFIG REQUIRED)
# Link publicly so include directories propagate to dependents that include NeuronCore headers
if(TARGET Microsoft::WinPixEventRuntime)
    target_link_libraries(NeuronCore PUBLIC Microsoft::WinPixEventRuntime)
elseif(TARGET WinPixEventRuntime::WinPixEventRuntime)
    target_link_libraries(NeuronCore PUBLIC WinPixEventRuntime::WinPixEventRuntime)
elseif(TARGET winpixevent::winpixevent)
    target_link_libraries(NeuronCore PUBLIC winpixevent::winpixevent)
elseif(TARGET winpixevent)
    target_link_libraries(NeuronCore PUBLIC winpixevent)
else()
    message(FATAL_ERROR "winpixevent package was found but no known CMake targets were exported. Please update vcpkg.")
endif()
# Propagate HAVE_PIX so users of NeuronCore can conditionally compile PIX markers
target_compile_definitions(NeuronCore PUBLIC HAVE_PIX)

# IDE source grouping for better organization
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PREFIX "Source Files" 
    FILES ${NEURON_SOURCES}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PREFIX "Header Files" 
    FILES ${NEURON_HEADERS}
)
