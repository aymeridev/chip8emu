#pragma once

#include <SDL3/SDL.h>
#include "Chip8State.hpp"

class AppState;

struct DebugUI {
    DebugUI(const AppState* appstate);
    ~DebugUI();

    static void update(AppState *appstate);
};
