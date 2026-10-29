#include <array>
#include <cstdint>
#include <span>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest.h>

#include "chip8_state.hpp"


TEST_SUITE_BEGIN("basic operations");
TEST_CASE("add") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 4> {
        0x60, 0x02, // set v[0] to 2
        0x70, 0x03, // add 3 to v[0]
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    CHECK(chip.cycle() == Chip8Error::None);
    CHECK(chip.cycle() == Chip8Error::None);
    CHECK(chip.v[0] == 5);
}
TEST_SUITE_END;

TEST_SUITE_BEGIN("subroutines");
TEST_CASE("subroutines work") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 10> {
        0x22, 0x06, // create subroutine and call 0x206
        0x61, 0x05, // set v[1] to 5
        0x12, 0x04, // jump to 0x204
        0x60, 0x07, // v[0] = 7
        0x00, 0xEE  // return
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);

    for (int i = 0; i < 10; ++i)
        CHECK(chip.cycle() == Chip8Error::None);

    CHECK(chip.v[0] == 7);
    CHECK(chip.v[1] == 5);
}
TEST_SUITE_END;



TEST_SUITE_BEGIN("stack overflow/underflow");

TEST_CASE("stack overflow") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 2> {
        0x22, 0x00
    };
    chip.load_rom(mem);

    for (int i = 0; i < 16; ++i) CHECK(chip.cycle() == Chip8Error::None);
    CHECK(chip.cycle() == Chip8Error::StackOverflow);
}

TEST_CASE("stack underflow") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 2> {
        0x00, 0xEE
    };
    chip.load_rom(mem);

    CHECK(chip.cycle() == Chip8Error::StackUnderflow);
}

TEST_SUITE_END;