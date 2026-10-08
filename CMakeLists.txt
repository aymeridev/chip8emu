cmake_minimum_required(VERSION 4.3)
project(chip8emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_SANITIZERS "enable to use ASan/UBSan" ON)

include_directories(tests)

include(FetchContent)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9b)

FetchContent_MakeAvailable(imgui)

find_package(SDL3 REQUIRED CONFIG)

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

add_library(chip8core src/Chip8State.cpp
        src/Chip8State.hpp
        src/utils_color.hpp
        src/Disassembler.hpp
)


add_executable(chip8emu src/main.cpp
        src/DebugUI.cpp
        src/DebugUI.hpp
        src/AppState.hpp
        src/example_roms.hpp)

add_executable(chip8tests
        tests/test_main.cpp
        tests/test_utils.hpp
        tests/test_timers.cpp
        tests/test_font.cpp
        tests/test_save_load.cpp)

target_link_directories(chip8tests PRIVATE chip8core)
target_link_libraries(chip8tests PRIVATE chip8core)
target_include_directories(chip8tests PRIVATE external)

if(ENABLE_SANITIZERS)
    message(STATUS "enabling sanitizers")
endif ()

if (MSVC)
    target_compile_options(chip8emu PRIVATE /W4) # no UBSan with MSVC
    if(ENABLE_SANITIZERS)
        target_compile_options(chip8emu PRIVATE /fsanitize=address)
        target_link_options(chip8emu PRIVATE -fsanitize=address)
    endif ()
else ()
    target_compile_options(chip8emu PRIVATE -Wall -Wextra -Wpedantic)
    if(ENABLE_SANITIZERS)
        target_compile_options(chip8emu PRIVATE -fsanitize=address,undefined)
        target_link_options(chip8emu PRIVATE -fsanitize=address,undefined)
    endif ()
endif ()

target_link_libraries(chip8emu PRIVATE SDL3::SDL3)
target_link_libraries(chip8emu PRIVATE imgui)
target_link_libraries(chip8emu PRIVATE chip8core)

add_custom_command(TARGET chip8emu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/roms"
        "$<TARGET_FILE_DIR:chip8emu>/roms"
        COMMENT "chip8emu: copying roms into build folder"
)
