#pragma once
#include <algorithm>
#include <span>
// ReSharper disable once CppUnusedIncludeDirective
#include <cstdint>

inline uint32_t color_to_uint32(const std::span<const float, 3> color) {
    auto float_to_int = [](const float c) {
        return static_cast<uint32_t>(std::clamp(c, 0.0f, 1.0f) * 255.999f);
    };

    return (float_to_int(color[0]) << 24)
        | (float_to_int(color[1]) << 16)
        | (float_to_int(color[2]) << 8)
        | 0xFF;
}