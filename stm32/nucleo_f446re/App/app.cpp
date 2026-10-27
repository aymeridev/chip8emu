#include "app.h"
#include "chip8_state.hpp"
#include "roms.hpp"
#include "main.h"
#include "ssd1306.hpp"
#include <array>

namespace {
    auto state = Chip8State();
    std::array<std::uint8_t, 1024> screen_buffer = {};
}

constexpr std::size_t row_size = 66;
constexpr std::uint32_t fps = 60;


void screen_to_array(const std::array<std::uint64_t, screen_height>& screen, std::array<std::uint8_t, 1024>& out) {
    out.fill(0x00);
    for (int page = 0; page < 8; ++page) {
        for (int c = 0; c < 128; ++c) {
            for (int b = 0; b < 8; ++b) {
                if ((screen[(page * 8 + b) / 2] >> static_cast<std::uint64_t>(c / 2)) & 1) { // pixel on
                    out[128 * page + c] |= 1 << b;
                }
            }
        }
    }
}

void app_main(UART_HandleTypeDef* huart2, I2C_HandleTypeDef* hi2c1) {
    state.load_rom(rom);
    auto start = HAL_GetTick();
    std::uint64_t next_frame = 0;
    std::uint64_t n = 0;
    bool error = false;

    if (HAL_I2C_IsDeviceReady(hi2c1, oled_address, 3, 100) == HAL_OK) {
        if (init_ssd1306(*hi2c1)) {
        } else {
            error = true;
        }
    } else {
        error = true;
    }

    while (true) {
        if (error) {
            HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
            HAL_Delay(100);
            continue;
        }
        if (HAL_GetTick() - start >= next_frame) {
            for (int i = 0; i < 100; ++i) {
                if (state.cycle() != Chip8Error::None) {
                    error = true;
                    break;
                }
            }
            state.tick_timers();
            n++;
            next_frame = n * 1000 / fps;
        }
        if (state.screen_dirty) {
            screen_to_array(state.screen, screen_buffer);
            if (HAL_I2C_Mem_Write(hi2c1, oled_address, 0x40,
                I2C_MEMADD_SIZE_8BIT, screen_buffer.data(), screen_buffer.size(), HAL_MAX_DELAY) != HAL_OK) {
                error = true;
            }
            state.screen_dirty = false;
        }
    }
}