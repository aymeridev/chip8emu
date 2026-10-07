#pragma once

#include <SDL3/SDL.h>

#include <array>
#include "DebugUI.hpp"


constexpr float frame_total_timer = 1.0f / 20.0f;

class AppState {
public:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    Chip8State chip8_state = {};
    std::unique_ptr<DebugUI> debug_ui = nullptr;
    std::array<float, 3> background_color { 0.573f, 0.412f, 0.129f };
    std::array<float, 3> foreground_color { 0.969f, 0.808f, 0.274f };
    SDL_Texture *texture = nullptr;
    Uint32 pixels[screen_width * screen_height] = {};

    float pixel_scale = 24.0f;
    float frame_timer = frame_total_timer;
    float last_tick;

    bool pause = false;
};
