# GameLogic – world simulation and gameplay entities

file(GLOB gamelogic_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB gamelogic_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(GameLogic ${gamelogic_sources} ${gamelogic_headers})
add_library(InterstellarOutpost::GameLogic ALIAS GameLogic)

target_include_directories(GameLogic
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/libs/NeuronCore
    PRIVATE
        ${PROJECT_SOURCE_DIR}/libs/NeuronClient
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(GameLogic
    PUBLIC
        InterstellarOutpost::NeuronCore
    PRIVATE
        InterstellarOutpost::NeuronClient
)

io_apply_target_defaults(GameLogic)
io_enable_warnings(GameLogic)
io_enable_pch(GameLogic "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

set_target_properties(GameLogic PROPERTIES FOLDER "Libraries")
