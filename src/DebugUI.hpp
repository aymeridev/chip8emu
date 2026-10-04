#pragma once

#include <SDL3/SDL.h>


#include "Chip8State.hpp"

struct AppState;

class DebugUI {
public:
    DebugUI(AppState* appstate);
    void update();
private:
    AppState* appstate;
    Chip8State& chip8_state;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};
