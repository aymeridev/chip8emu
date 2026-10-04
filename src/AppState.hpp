#pragma once

#include <SDL3/SDL.h>

#include <array>
#include "DebugUI.hpp"


struct AppState {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    Chip8State chip8_state = {};
    std::unique_ptr<DebugUI> debug_ui = nullptr;
    std::array<float, 3> background_color { 0, 0, 0 };
    std::array<float, 3> foreground_color { 255, 255, 255 };
};
