# NeuronServer – server core functionality

file(GLOB neuronserver_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB neuronserver_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(NeuronServer ${neuronserver_sources} ${neuronserver_headers})
add_library(InterstellarOutpost::NeuronServer ALIAS NeuronServer)

target_include_directories(NeuronServer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/libs/NeuronCore
)

target_link_libraries(NeuronServer
    PUBLIC
        InterstellarOutpost::NeuronCore
)

ApplyTargetDefaults(NeuronServer)
EnablePch(NeuronServer "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

set_target_properties(NeuronServer PROPERTIES FOLDER "Libraries")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${neuronserver_sources})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${neuronserver_headers})
