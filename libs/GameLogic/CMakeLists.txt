
# GameLogic – world simulation and gameplay entities

file(GLOB gamelogic_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB gamelogic_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(GameLogic ${gamelogic_sources} ${gamelogic_headers})
add_library(InterstellarOutpost::GameLogic ALIAS GameLogic)

target_include_directories(GameLogic
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/libs/NeuronCore
)

target_link_libraries(GameLogic
    PUBLIC
        InterstellarOutpost::NeuronCore
)

ApplyTargetDefaults(GameLogic)
EnablePch(GameLogic "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

set_target_properties(GameLogic PROPERTIES FOLDER "Libraries")
