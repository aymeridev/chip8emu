#pragma once

#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "Chip8State.hpp"

class DebugUI {
public:
    DebugUI(Chip8State& state, SDL_Window* window, SDL_Renderer* renderer);
    void Update();
private:
    Chip8State& state;
    SDL_Window* window;
    SDL_Renderer* renderer;
};
