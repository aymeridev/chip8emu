#include "app.h"
#include "chip8_state.hpp"
#include "roms.hpp"
#include "main.h"

#include <cstdio>
#include <cstdlib>

namespace {
    auto state = Chip8State();
}

void app_main() {
    state.load_rom(rom);
    while (true) {
        for (int i = 0; i < 100; ++i) {
            (void) state.cycle();
        }
        state.tick_timers();
        if (state.screen_dirty) {
            HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
            state.screen_dirty = false;
        }
        printf("%d", HAL_GetTick());
        // auto t = HAL_GetTick();
    }
}