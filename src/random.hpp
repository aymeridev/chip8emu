#pragma once
#include <cstdint>

namespace detail {
    inline static std::uint32_t state = 2463534242u;
}


inline bool set_seed(std::uint32_t new_seed) {
    if (new_seed < 0) return false;
    detail::state = new_seed;
    return true;
}

inline std::uint32_t xorshift32() {
    std::uint32_t x = detail::state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    detail::state = x;
    return x;
}

inline float random_float() {
    return static_cast<float>(xorshift32() >> 8) * (1.0f / 16777216.0f);
}