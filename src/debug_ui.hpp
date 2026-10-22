#pragma once

#include <SDL3/SDL.h>
#include "chip8_state.hpp"
#include <string>

class AppState;

struct DebugUI {
    DebugUI(const AppState* appstate);
    ~DebugUI();

    void update(AppState *appstate);
    void show_message(const char *message);
private:
    bool m_show_message = false;
    std::string m_message;
};
