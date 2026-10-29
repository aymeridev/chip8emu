#include "app_state.hpp"
#include <iostream>
#include <string>
#include <format>

void AppState::run_frame() {
    if (!chip8_state.rom_loaded) return;
    for (int i = 0; i < cycles_per_tick; ++i) {
        if (const auto err = chip8_state.cycle(); err != Chip8Error::None) {
            handle_cycle_error(err);
            break;
        }
    }
    chip8_state.tick_timers();
}


void AppState::handle_cycle_error(Chip8Error err) {
    if (err != Chip8Error::PcOutOfBounds) {
        pause = true;
        const auto msg = std::format(
            "Error Occured (Code {})",
            static_cast<int>(err));
        if (debug_ui) {
            debug_ui->show_message(msg.data());
        }
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", msg.data());
    }
}
