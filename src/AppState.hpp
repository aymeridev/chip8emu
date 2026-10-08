#pragma once

#include <SDL3/SDL.h>

#include <array>
#include "DebugUI.hpp"


constexpr float cycle_total_timer = 1.0f / 60.0f;
constexpr float tick_total_timer = 1.0f / 60.0f;

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
    static constexpr std::array<SDL_Keycode, 16> keys = {
        SDLK_X, SDLK_1, SDLK_2, SDLK_3,
        SDLK_A, SDLK_Z, SDLK_E, SDLK_Q,
        SDLK_S, SDLK_D, SDLK_W, SDLK_C,
        SDLK_4, SDLK_R, SDLK_F, SDLK_C,
     };

    // static constexpr std::array<SDL_Keycode, 16> keys = {
    //     SDLK_1, SDLK_2, SDLK_3, SDLK_4,
    //     SDLK_A, SDLK_Z, SDLK_E, SDLK_R,
    //     SDLK_Q, SDLK_S, SDLK_D, SDLK_F,
    //     SDLK_W, SDLK_X, SDLK_C, SDLK_V,
    //  };

    float pixel_scale = 24.0f;
    float cycle_timer = cycle_total_timer;
    float tick_timer = tick_total_timer;
    float last_tick;

    bool pause = false;
};
