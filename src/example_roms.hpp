#pragma once
#include <array>
#include <string_view>

constexpr std::array<std::string_view, 8> test_roms = {
    "1-chip8-logo.ch8",
    "2-ibm-logo.ch8",
    "3-corax+.ch8",
    "4-flags.ch8",
    "5-quirks.ch8",
    "6-keypad.ch8",
    "7-beep.ch8",
    "8-scrolling.ch8",
};

constexpr std::array<std::string_view, 0> example_roms = {
};