#pragma once

#include <array>
#include <string>


constexpr int screen_width = 64;
constexpr int screen_height = 32;

class Chip8State {
public:

    std::string rom_file_path;
    std::array<std::uint8_t, 16> v; // general registers
    uint16_t i; // store memory address
    uint16_t pc = 0x200; // program counter

    std::array<std::uint16_t, 16> stack;

    // stack pointer
    uint8_t sp = 0;


    std::array<std::uint8_t, 4096> memory;
    bool screen[screen_height][screen_width];

    bool load_rom(const std::string &file_path);
    void fetch();
    void decode(std::uint16_t instruction);

private:
    void clear_screen();
    void run_math(std::uint8_t n, std::uint8_t x, std::uint8_t y);
};
