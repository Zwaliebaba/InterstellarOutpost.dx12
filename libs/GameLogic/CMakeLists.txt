# GameLogic Library CMakeLists.txt

file(GLOB CPP_SOURCES *.cpp Interface/*.cpp WorldObject/*.cpp)
file(GLOB H_SOURCES *.h Interface/*.h WorldObject/*.h)

# Create GameLogic library
add_library(GameLogic STATIC
    ${CPP_SOURCES}
    ${H_SOURCES} 
)

target_precompile_headers(GameLogic 
    PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/pch.h>"
)

# Set target properties
target_include_directories(GameLogic 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/Neuron>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libs/GameLogic>
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