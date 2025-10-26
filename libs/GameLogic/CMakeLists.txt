# GameLogic Library CMakeLists.txt

# Create GameLogic library
add_library(GameLogic STATIC
    GameLogic.cpp
)

# Set target properties
target_include_directories(GameLogic 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Neuron>
)

# Set C++20 for this target
target_compile_features(GameLogic PUBLIC cxx_std_20)

# Link with Neuron library
target_link_libraries(GameLogic PUBLIC Neuron)

# Visual Studio specific settings
if(MSVC)
    target_compile_options(GameLogic PRIVATE /W4)
else()
    target_compile_options(GameLogic PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Export target
set_target_properties(GameLogic PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)