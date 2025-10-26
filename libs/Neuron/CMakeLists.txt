# Neuron Library CMakeLists.txt

# Create Neuron library
add_library(Neuron STATIC
    Neuron.cpp
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