# Main Application CMakeLists.txt

file(GLOB CPP_SOURCES *.cpp)
file(GLOB H_SOURCES *.h)

# Create main executable
add_executable(InterstellarOutpost
    ${CPP_SOURCES}
    ${H_SOURCES}
)

# Link libraries
target_link_libraries(InterstellarOutpost 
    PRIVATE 
        Neuron 
        GameLogic
)

# Set target properties
target_include_directories(InterstellarOutpost 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/Neuron>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/GameLogic>
)

target_precompile_headers(InterstellarOutpost
    PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/pch.h>"
)

# Set C++20 for this target
target_compile_features(InterstellarOutpost PRIVATE cxx_std_20)

# Visual Studio specific settings
if(MSVC)
    target_compile_options(InterstellarOutpost PRIVATE /W4)
    # Set subsystem for console application
    set_target_properties(InterstellarOutpost PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
else()
    target_compile_options(InterstellarOutpost PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set output directory
set_target_properties(InterstellarOutpost PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)