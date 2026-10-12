#pragma once

#include <SDL3/SDL.h>
#include "Chip8State.hpp"

class AppState;

struct DebugUI {
    DebugUI(const AppState* appstate);
    ~DebugUI();

    void update(AppState *appstate);
    void show_message(const char* message);
private:
    bool m_show_message = false;
    const char* m_message = nullptr;
};
