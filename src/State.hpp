#pragma once
#include <array>
#include <cstdint>


constexpr int screen_width = 64;
constexpr int screen_height = 32;

class State {
public:
    std::array<std::uint8_t, 4096> memory;
    bool screen[screen_height][screen_width];

    bool load_rom(const std::string &file_path);
};
