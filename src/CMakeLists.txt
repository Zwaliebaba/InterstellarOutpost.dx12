# Main Application CMakeLists.txt

# Create main executable
add_executable(InterstellarOutpost
    main.cpp
)

# Link libraries
target_link_libraries(InterstellarOutpost 
    PRIVATE 
        Neuron 
        GameLogic
)

# Set target properties
target_include_directories(GameLogic 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/Neuron>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/GameLogic>
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