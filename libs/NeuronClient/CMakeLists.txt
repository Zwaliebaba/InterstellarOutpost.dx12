# NeuronClient – presentation, audio, and input subsystems

file(GLOB neuronclient_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB neuronclient_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(NeuronClient ${neuronclient_sources} ${neuronclient_headers})
add_library(InterstellarOutpost::NeuronClient ALIAS NeuronClient)

target_include_directories(NeuronClient
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/libs/NeuronCore
    PRIVATE
        ${PROJECT_SOURCE_DIR}/libs/GameLogic
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(NeuronClient
    PUBLIC
        InterstellarOutpost::NeuronCore
)

io_apply_target_defaults(NeuronClient)
io_enable_warnings(NeuronClient)
io_enable_pch(NeuronClient "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

set_target_properties(NeuronClient PROPERTIES FOLDER "Libraries")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${neuronclient_sources})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${neuronclient_headers})