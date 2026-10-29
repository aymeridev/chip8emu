#include <array>
#include <cstdint>
#include <span>

#include <doctest.h>

#include "chip8_state.hpp"

TEST_SUITE_BEGIN("display");

TEST_CASE("Dxyn clipped at the bottom still waits for vblank") {
    auto chip = Chip8State {};
    auto mem = std::array<std::uint8_t, 17> {
        0x60, 0x00, // 0x200: V0 = 0  (x)
        0x61, 0x1E, // 0x202: V1 = 30 (y), so rows 32 to 34 fall off the screen
        0xA2, 0x0C, // 0x204: I = 0x20C, the sprite below
        0xD0, 0x15, // 0x206: draw 5 rows at (V0, V1)
        0x62, 0x05, // 0x208: V2 = 5, must wait for the next vblank
        0x12, 0x0A, // 0x20A: jump to itself (never run the sprite bytes)
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 0x20C: sprite, 5 rows
    };
    std::span<std::uint8_t> sp_mem = mem;
    chip.load_rom(sp_mem);
    for (int c = 0; c < 4; ++c) CHECK(chip.cycle() == Chip8Error::None);

    // the 2 visible rows (0xF0, 0x90) are drawn, the 3 others are clipped
    CHECK(((chip.screen[30] >> 0) & 1));
    CHECK(((chip.screen[30] >> 3) & 1));
    CHECK_FALSE(((chip.screen[30] >> 4) & 1));
    CHECK(((chip.screen[31] >> 0) & 1));
    CHECK_FALSE(((chip.screen[31] >> 1) & 1));
    CHECK(chip.v[0xF] == 0);

    CHECK(chip.waiting_for_vblank);
    (void)chip.cycle(); // blocked: V2 = 5 must not run yet
    CHECK(chip.v[2] == 0);

    chip.tick_timers(); // vblank
    (void)chip.cycle();
    CHECK(chip.v[2] == 5);
}

TEST_SUITE_END;
