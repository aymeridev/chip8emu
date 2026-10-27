#include "app.h"
#include "chip8_state.hpp"
#include "roms.hpp"
#include "main.h"
#include "ssd1306.hpp"
#include <array>

namespace {
    auto state = Chip8State();
}

constexpr std::size_t row_size = 66;
constexpr std::uint32_t fps = 60;

void app_main(UART_HandleTypeDef* huart2, I2C_HandleTypeDef* hi2c1) {
    state.load_rom(rom);
    std::array<std::uint8_t, row_size> screen_buffer = {};
    auto start = HAL_GetTick();
    std::uint64_t next_frame = 0;
    std::uint64_t n = 0;
    bool error = false;

    if (HAL_I2C_IsDeviceReady(hi2c1, 0x3C << 1, 3, 100) == HAL_OK) {
        if (init_ssd1306(*hi2c1)) {
        std::array<std::uint8_t, 1024> screen = {};
        screen.fill(0x80);
        HAL_I2C_Mem_Write(hi2c1, 0x3C << 1, 0x40,
            I2C_MEMADD_SIZE_8BIT, screen.data(), screen.size(), HAL_MAX_DELAY);
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