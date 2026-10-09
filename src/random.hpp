#pragma once
#include <cstdint>

namespace detail {
    static uint32_t state = 2463534242u;
}

inline std::uint32_t xorshift32() {
    uint32_t x = detail::state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    detail::state = x;
    return x;
}

inline float random_float() {
    return (xorshift32() >> 8) * (1.0f / 16777216.0f);
}