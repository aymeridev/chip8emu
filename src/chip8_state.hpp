#pragma once

#include <array>
#include <span>
#include <optional>
#include <cstdint>

constexpr int screen_width = 64;
constexpr int screen_height = 32;
constexpr int memory_size = 0x1000; // 4096
constexpr int memory_data_start = 0x200; // 512
constexpr int font_start = 0x50; // 512

enum class Chip8Error : std::uint8_t {
    None,
    UnknownOpcode,
    StackOverflow,
    StackUnderflow,
    PcOutOfBounds,
};


class Chip8State {
public:
    Chip8State();

    std::array<std::uint8_t, 16> v = {}; // general registers
    uint16_t i_reg = 0; // store memory address
    uint16_t pc = memory_data_start; // program counter

    // input
    std::array<bool, 16> keys = {};

    std::optional<std::uint8_t> register_listening_for_key;
    bool waiting_for_vblank = false;

    std::array<std::uint16_t, 16> stack = {};

    // stack pointer
    std::uint8_t sp = 0;

    std::uint8_t delay_timer = 0;
    std::uint8_t sound_timer = 0;
    bool rom_loaded = false;
    bool screen_dirty = false;

    void tick_timers();

    std::array<std::uint8_t, memory_size> memory = {};
    std::array<std::uint64_t, screen_height> screen = {};

    void reset();
    bool load_rom(std::span<const std::uint8_t> rom_data);



    void update_key(std::uint8_t index, bool enabled);

    // fetch 2 bytes from memory
    std::uint16_t fetch();

    [[nodiscard]] Chip8Error cycle();
    Chip8Error decode_and_execute(uint16_t instruction);


private:
    [[nodiscard]] Chip8Error run_math(std::uint8_t n, std::uint8_t x, std::uint8_t y);
};
