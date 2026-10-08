#include "../external/doctest.h"
#include "../src/Chip8State.hpp"

TEST_SUITE_BEGIN("register save and load");

TEST_CASE("Fx55 saves V0..Vx inclusive and advances I") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 10> {
        0x60, 0x11, // V0 = 0x11
        0x61, 0x22, // V1 = 0x22
        0x62, 0x33, // V2 = 0x33
        0xA3, 0x00, // I = 0x300
        0xF1, 0x55, // save V0..V1
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    for (int c = 0; c < 5; ++c) chip.cycle();

    CHECK(chip.memory[0x300] == 0x11);
    CHECK(chip.memory[0x301] == 0x22);
    CHECK(chip.memory[0x302] == 0x00); // V2 must NOT be saved
    CHECK(chip.i_reg == 0x302);            // COSMAC: I += x + 1
}

TEST_CASE("Fx55 with x = 0 still saves V0") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 6> {
        0x60, 0x7F, // V0 = 0x7F
        0xA3, 0x00, // I = 0x300
        0xF0, 0x55, // save V0
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    for (int c = 0; c < 3; ++c) chip.cycle();

    CHECK(chip.memory[0x300] == 0x7F);
}

TEST_CASE("Fx65 loads V0..Vx inclusive and advances I") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 4> {
        0xA3, 0x00, // I = 0x300
        0xF2, 0x65, // load V0..V2
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    chip.memory[0x300] = 0xAA;
    chip.memory[0x301] = 0xBB;
    chip.memory[0x302] = 0xCC;
    chip.memory[0x303] = 0xDD;
    chip.cycle();
    chip.cycle();

    CHECK(chip.v[0] == 0xAA);
    CHECK(chip.v[1] == 0xBB);
    CHECK(chip.v[2] == 0xCC);
    CHECK(chip.v[3] == 0x00); // V3 must NOT be loaded
    CHECK(chip.i_reg == 0x303);
}

TEST_SUITE_END;