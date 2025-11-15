# NeuronCore – core engine utilities and shared systems

file(GLOB neuroncore_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB neuroncore_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(NeuronCore ${neuroncore_sources} ${neuroncore_headers})
add_library(InterstellarOutpost::NeuronCore ALIAS NeuronCore)

target_include_directories(NeuronCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/libs/GameLogic
        ${PROJECT_SOURCE_DIR}/libs/NeuronClient
        ${PROJECT_SOURCE_DIR}/src
)

io_apply_target_defaults(NeuronCore)
io_enable_warnings(NeuronCore)
io_enable_pch(NeuronCore "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(NeuronCore PUBLIC nlohmann_json::nlohmann_json)

find_package(winpixevent CONFIG REQUIRED)
if(TARGET Microsoft::WinPixEventRuntime)
    target_link_libraries(NeuronCore PUBLIC Microsoft::WinPixEventRuntime)
elseif(TARGET WinPixEventRuntime::WinPixEventRuntime)
    target_link_libraries(NeuronCore PUBLIC WinPixEventRuntime::WinPixEventRuntime)
elseif(TARGET winpixevent::winpixevent)
    target_link_libraries(NeuronCore PUBLIC winpixevent::winpixevent)
elseif(TARGET winpixevent)
    target_link_libraries(NeuronCore PUBLIC winpixevent)
else()
    message(FATAL_ERROR "winpixevent package was found but exported no recognised CMake targets.")
endif()

target_compile_definitions(NeuronCore PUBLIC HAVE_PIX)

set_target_properties(NeuronCore PROPERTIES FOLDER "Libraries")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${neuroncore_sources})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${neuroncore_headers})
