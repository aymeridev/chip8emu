cmake_minimum_required(VERSION 4.3)
project(chip8emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9b)

FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL3::SDL3)


find_package(SDL3 REQUIRED CONFIG)

add_executable(chip8emu src/main.cpp
        src/DebugUI.cpp
        src/DebugUI.hpp
        src/State.cpp
        src/State.hpp)


target_link_libraries(chip8emu PRIVATE SDL3::SDL3)
target_link_libraries(chip8emu PRIVATE imgui)

add_custom_command(TARGET chip8emu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "$<TARGET_FILE_DIR:chip8emu>/tests"
        COMMENT "chip8emu: copying tests folder into build folder"
)
