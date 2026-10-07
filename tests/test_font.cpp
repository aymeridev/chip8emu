#include "../external/doctest.h"
#include "../src/Chip8State.hpp"

TEST_SUITE_BEGIN("font and bcd");

TEST_CASE("Fx33 stores the decimal digits of Vx at I") {
    const auto [value, hundreds, tens, units] = GENERATE(
        std::tuple<int, int, int, int>{156, 1, 5, 6},
        std::tuple<int, int, int, int>{0,   0, 0, 0},
        std::tuple<int, int, int, int>{9,   0, 0, 9},
        std::tuple<int, int, int, int>{255, 2, 5, 5});

    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 6> {
        0x60, static_cast<std::uint8_t>(value), // V0 = value
        0xA3, 0x00,                             // I = 0x300
        0xF0, 0x33,                             // BCD of V0 at I
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    for (int c = 0; c < 3; ++c) chip.cycle();

    CHECK(chip.memory[0x300] == hundreds);
    CHECK(chip.memory[0x301] == tens);
    CHECK(chip.memory[0x302] == units);
}

TEST_CASE("Fx29 points I at the font sprite of the low digit of Vx") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 4> {
        0x60, 0x1A, // V0 = 0x1A (low digit is A)
        0xF0, 0x29, // I = font sprite for V0
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    chip.cycle();
    chip.cycle();

    CHECK(chip.i == 0x050 + 5 * 0xA);
    CHECK(chip.memory[chip.i] == 0xF0); // first row of "A"
    CHECK(chip.memory[chip.i + 1] == 0x90); // fails if "A" has the wrong byte
}

TEST_SUITE_END;