cmake_minimum_required(VERSION 4.3)
project(chip8emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(SDL3 REQUIRED CONFIG)

add_executable(chip8emu main.cpp)

target_link_libraries(chip8emu PRIVATE SDL3::SDL3)