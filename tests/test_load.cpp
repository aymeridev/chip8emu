#include <array>
#include <cstdint>
#include <doctest.h>
#include <span>
#include <vector>

#include "chip8_state.hpp"

TEST_SUITE_BEGIN("font and rom loading");

TEST_CASE("reset copies the font to 0x050..0x09F") {
    auto chip = Chip8State {};
    chip.reset();

    CHECK(chip.memory[0x050] == 0xF0); // first row of "0"
    CHECK(chip.memory[0x055] == 0x20); // first row of "1"
    CHECK(chip.memory[0x09F] == 0x80); // last row of "F"
    CHECK(chip.memory[0x04F] == 0x00); // nothing written before the font
    CHECK(chip.memory[0x0A0] == 0x00); // nothing written after the font
}

TEST_CASE("the font is already there right after construction") {
    const auto chip = Chip8State {};

    CHECK(chip.memory[0x050] == 0xF0);
    CHECK(chip.memory[0x09F] == 0x80);
}

TEST_CASE("load_rom copies the bytes at 0x200 and keeps the font") {
    auto chip = Chip8State {};
    auto rom = std::array<std::uint8_t, 3> { 0x12, 0x34, 0x56 };

    CHECK(chip.load_rom(rom));
    CHECK(chip.rom_loaded);
    CHECK(chip.pc == 0x200);
    CHECK(chip.memory[0x1FF] == 0x00); // nothing written before the ROM
    CHECK(chip.memory[0x200] == 0x12);
    CHECK(chip.memory[0x201] == 0x34);
    CHECK(chip.memory[0x202] == 0x56);
    CHECK(chip.memory[0x203] == 0x00); // nothing written after the ROM
    CHECK(chip.memory[0x050] == 0xF0); // the font survived
}

TEST_CASE("loading a second ROM leaves nothing from the first one") {
    auto chip = Chip8State {};
    auto big = std::array<std::uint8_t, 4> { 0xAA, 0xBB, 0xCC, 0xDD };
    auto small = std::array<std::uint8_t, 2> { 0x11, 0x22 };

    chip.load_rom(big);
    chip.v[3] = 42; // some state left over by the first game
    chip.load_rom(small);

    CHECK(chip.memory[0x200] == 0x11);
    CHECK(chip.memory[0x201] == 0x22);
    CHECK(chip.memory[0x202] == 0x00); // was 0xCC
    CHECK(chip.memory[0x203] == 0x00); // was 0xDD
    CHECK(chip.v[3] == 0);
}

TEST_CASE("a ROM of exactly 3584 bytes fills memory up to 0xFFF") {
    auto chip = Chip8State {};
    auto rom = std::vector<std::uint8_t>(memory_size - memory_data_start, 0x00);
    rom.back() = 0xEE;

    CHECK(chip.load_rom(rom));
    CHECK(chip.memory[0xFFF] == 0xEE);
}

TEST_CASE("a ROM one byte too big is rejected") {
    auto chip = Chip8State {};
    auto rom = std::vector<std::uint8_t>(memory_size - memory_data_start + 1, 0xAB);

    CHECK_FALSE(chip.load_rom(rom));
    CHECK_FALSE(chip.rom_loaded);
    CHECK(chip.memory[0x200] == 0x00); // nothing was copied
}

TEST_SUITE_END;
