#pragma once
#include <array>
#include <iostream>
#include <cstdint>
#include "../src/Chip8State.hpp"

inline void print_registers(const std::array<std::uint8_t, 16> &registers) {
    for (const auto r : registers) {
        std::cout << static_cast<unsigned>(r) << ' ';
    }
    std::cout << '\n';
}