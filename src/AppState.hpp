#pragma once

#include <SDL3/SDL.h>

#include <array>
#include "DebugUI.hpp"


class AppState {
public:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    Chip8State chip8_state = {};
    std::unique_ptr<DebugUI> debug_ui = nullptr;
    std::array<float, 3> background_color { 0, 0, 0 };
    std::array<float, 3> foreground_color { 1.0f, 1.0f, 1.0f };
};
