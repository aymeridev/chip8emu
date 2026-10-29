cmake_minimum_required(VERSION 3.31)
project(chip8emu VERSION 1.0.0
        DESCRIPTION "A CHIP-8 Emulator made with C++20 with an SDL3 Frontend."
        HOMEPAGE_URL "https://github.com/aymeridev/chip8emu"
        LANGUAGES CXX)

enable_testing()

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_SANITIZERS "enable to use ASan/UBSan" ON)
option(ENABLE_COVERAGE "enable coverage for tests" OFF)
option(CHIP8_BUILD_STM32 "build for the stm32 instead of desktop" OFF)

add_library(chip8core src/chip8_state.cpp
        src/chip8_state.hpp
        src/random.hpp
        src/logo.hpp
)

target_include_directories(chip8core PUBLIC src)

add_library(chip8options INTERFACE)


if (CHIP8_BUILD_STM32)
    add_subdirectory(stm32/nucleo_f446re)
else ()
    include(FetchContent)

    FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.9b)
    FetchContent_MakeAvailable(imgui)

    FetchContent_Declare(SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.4.18)

    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL3)


    FetchContent_Declare(doctest
            GIT_REPOSITORY https://github.com/doctest/doctest.git
            GIT_TAG v2.5.3)

    set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(doctest)


    add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
            ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
    )
    target_include_directories(imgui PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC SDL3::SDL3)


    add_library(chip8tools INTERFACE
            src/disassembler.hpp
    )

    target_include_directories(chip8tools INTERFACE src)

    add_executable(chip8emu src/main.cpp
            src/debug_ui.cpp
            src/debug_ui.hpp
            src/app_state.cpp
            src/app_state.hpp
            src/utils_color.hpp
    )
    install(TARGETS chip8emu DESTINATION . COMPONENT chip8emu)
    install(DIRECTORY roms DESTINATION . COMPONENT chip8emu)

    target_link_libraries(chip8emu PRIVATE
            SDL3::SDL3
            imgui
            chip8core
            chip8tools
    )

    add_executable(chip8tests
            tests/test_main.cpp
            tests/test_timers.cpp
            tests/test_font.cpp
            tests/test_save_load.cpp
            tests/test_disassembler.cpp
            tests/test_display.cpp
            tests/test_load.cpp
    )

    target_link_libraries(chip8tests PRIVATE
            chip8core
            chip8tools
            doctest::doctest
    )

    include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")
    doctest_discover_tests(chip8tests)


    if(ENABLE_SANITIZERS)
        message(STATUS "enabling sanitizers")
    endif ()


    target_link_libraries(chip8tests PRIVATE chip8options)
    target_link_libraries(chip8emu PRIVATE chip8options)


    add_custom_command(TARGET chip8emu POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/roms"
            "$<TARGET_FILE_DIR:chip8emu>/roms"
            COMMENT "chip8emu: copying roms into build folder"
    )

    if (NOT MSVC)
        if (ENABLE_COVERAGE)
            target_compile_options(chip8core PRIVATE --coverage)
            target_compile_options(chip8tests PRIVATE --coverage)

            target_link_options(chip8core INTERFACE --coverage)
        endif ()
    endif ()

endif ()

target_link_libraries(chip8core PRIVATE chip8options)

if (MSVC)
    target_compile_options(chip8options INTERFACE /W4) # no UBSan with MSVC
    if(NOT CHIP8_BUILD_STM32 AND ENABLE_SANITIZERS)
        target_compile_options(chip8options INTERFACE /fsanitize=address)
    endif ()
else ()

    target_compile_options(chip8core PRIVATE -fno-exceptions -fno-rtti)
    target_compile_options(chip8options INTERFACE -Wconversion -Wsign-conversion -Wall -Wextra -Wpedantic)
    if(NOT CHIP8_BUILD_STM32 AND ENABLE_SANITIZERS)
        target_compile_options(chip8options INTERFACE -fsanitize=address,undefined)
        target_link_options(chip8options INTERFACE -fsanitize=address,undefined)
    endif ()
endif ()
