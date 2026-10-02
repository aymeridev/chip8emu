#pragma once
#include <array>
#include <cstdint>


constexpr int screen_width = 64;
constexpr int screen_height = 32;

class State {
public:

    std::array<std::uint8_t, 16> v; // general registers
    uint8_t sp; // stack pointer
    uint16_t i; // store memory address
    uint16_t pc; // program counter



    std::array<std::uint8_t, 4096> memory;
    bool screen[screen_height][screen_width];

    bool load_rom(const std::string &file_path);
    void fetch();
    void decode(std::uint16_t instruction);
};
