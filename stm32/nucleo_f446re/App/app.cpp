#include "app.h"
#include "chip8_state.hpp"
#include "roms.hpp"
#include "main.h"
#include <array>

namespace {
    auto state = Chip8State();
}

constexpr std::size_t row_size = 66;
constexpr std::uint32_t delta_time = 1000 / 60;

void app_main(UART_HandleTypeDef* huart2) {
    state.load_rom(rom);
    std::array<std::uint8_t, row_size> screen_buffer = {};
    auto last_frame = HAL_GetTick();
    while (true) {
        if (last_frame - HAL_GetTick() > delta_time) {
            for (int i = 0; i < 100; ++i) {
                (void) state.cycle();
            }
            state.tick_timers();
            last_frame = HAL_GetTick();
        }
        if (state.screen_dirty) {
            HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
            HAL_UART_Transmit(huart2, reinterpret_cast<const uint8_t *>("\x1b[H"), 3, HAL_MAX_DELAY);
            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 64; ++x) {
                    const bool pixel_on = (state.screen[y] >> x) & 1;
                    screen_buffer[x] = pixel_on ? '#' : '.';
                }
                screen_buffer[64] = '\r';
                screen_buffer[65] = '\n';
                HAL_UART_Transmit(huart2, screen_buffer.data(), row_size, HAL_MAX_DELAY);
            }
            state.screen_dirty = false;
        }
    }
}