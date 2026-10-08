#pragma once

#include <array>
#include <string>
#include <span>
#include <random>
#include <cstdint>
#include <initializer_list>

constexpr int screen_width = 64;
constexpr int screen_height = 32;

class Chip8State {
public:

    std::string rom_file_path;
    std::array<std::uint8_t, 16> v; // general registers
    uint16_t i_reg; // store memory address
    uint16_t pc = 0x200; // program counter
    std::mt19937 rng = std::mt19937(std::random_device{}());

    // input
    std::array<bool, 16> keys = {};

    std::optional<uint8_t> register_listening_for_key;
    bool waiting_for_vblank;

    std::array<std::uint16_t, 16> stack;

    // stack pointer
    uint8_t sp = 0;

    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    void tick_timers();

    std::array<std::uint8_t, 4096> memory;
    bool screen[screen_height][screen_width];

    void reset();
    bool load_rom(const std::string &file_path);
    bool load_rom(std::span<uint8_t> &rom_data);


    /**
     *
     * @return return true while the run has not reached the end of memory
     */
    bool cycle();

    void update_key(std::uint8_t index, bool enabled);

    // fetch 4 bits from memory
    std::uint16_t fetch();
    void decode_and_execute(std::uint16_t instruction);


private:
    void clear_screen();
    void run_math(std::uint8_t n, std::uint8_t x, std::uint8_t y);
};
