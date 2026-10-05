#pragma once

#include <array>
#include <string>
#include <span>
#include <cstdint>

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

    void reset();
    bool load_rom(const std::string &file_path);
    bool load_rom(std::span<uint8_t> &rom_data);


    /**
     *
     * @return return true if the ROM has reached the end of the memory.
     */
    bool cycle();

    // fetch 4 bits from memory
    std::uint16_t fetch();
    void decode_and_execute(std::uint16_t instruction);


private:
    void clear_screen();
    void run_math(std::uint8_t n, std::uint8_t x, std::uint8_t y);
};
