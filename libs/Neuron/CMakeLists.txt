# Neuron Library CMakeLists.txt

file(GLOB CPP_SOURCES *.cpp Canvas/*.cpp Input/*.cpp Netlib/*.cpp Network/*.cpp Sound/*.cpp)
file(GLOB H_SOURCES *.h Canvas/*.h Input/*.h Netlib/*.h Network/*.h Sound/*.h)

# Create Neuron library
add_library(Neuron STATIC
    ${CPP_SOURCES}  
    ${H_SOURCES}
)

target_include_directories(Neuron
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libs/Neuron>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libs/GameLogic>
)

target_precompile_headers(Neuron
    PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/pch.h>"
)

# Set target properties
#target_include_directories(Neuron 
#    PUBLIC 
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#        $<INSTALL_INTERFACE:include>
#)

# Set C++20 for this target
target_compile_features(Neuron PUBLIC cxx_std_20)

# Visual Studio specific settings
if(MSVC)
    target_compile_options(Neuron PRIVATE /W4)
else()
    target_compile_options(Neuron PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Export target
set_target_properties(Neuron PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)