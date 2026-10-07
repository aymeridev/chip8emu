#include "../external/doctest.h"
#include "../src/Chip8State.hpp"

TEST_SUITE_BEGIN("timers");

TEST_CASE("Fx15 sets the delay timer from Vx") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 4> {
        0x60, 0x0A, // V0 = 10
        0xF0, 0x15, // delay_timer = V0
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    chip.cycle();
    chip.cycle();
    CHECK(chip.delay_timer == 10);
}

TEST_CASE("Fx07 reads the delay timer into Vx") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 6> {
        0x60, 0x0A, // V0 = 10
        0xF0, 0x15, // delay_timer = V0
        0xF1, 0x07, // V1 = delay_timer
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    for (int i = 0; i < 3; ++i) chip.cycle();
    CHECK(chip.v[1] == 10);
}

TEST_CASE("Fx18 sets the sound timer from Vx") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 4> {
        0x60, 0x05, // V0 = 5
        0xF0, 0x18, // sound_timer = V0
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    chip.cycle();
    chip.cycle();
    CHECK(chip.sound_timer == 5);
}

TEST_CASE("tick_timers decrements both timers by one") {
    auto chip = Chip8State {};
    chip.delay_timer = 3;
    chip.sound_timer = 2;
    chip.tick_timers();
    CHECK(chip.delay_timer == 2);
    CHECK(chip.sound_timer == 1);
}

TEST_CASE("timers stop at 0 and never underflow") {
    auto chip = Chip8State {};
    chip.delay_timer = 1;
    chip.sound_timer = 0;
    chip.tick_timers();
    chip.tick_timers();
    CHECK(chip.delay_timer == 0);
    CHECK(chip.sound_timer == 0); // would be 255 if it wrapped
}

TEST_CASE("reset clears the timers") {
    auto chip = Chip8State {};
    chip.delay_timer = 9;
    chip.sound_timer = 9;
    chip.reset();
    CHECK(chip.delay_timer == 0);
    CHECK(chip.sound_timer == 0);
}

TEST_SUITE_END;