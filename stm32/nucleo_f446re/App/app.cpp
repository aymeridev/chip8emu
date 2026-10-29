#include "app.h"
#include "chip8_state.hpp"
#include "roms.hpp"
#include "main.h"
#include "ssd1306.hpp"
#include <array>

namespace {
    auto state = Chip8State();
    std::array<std::uint8_t, 1024> screen_buffer = {};

    struct Key {
        GPIO_TypeDef* gpio {};
        std::uint16_t pin {};
        std::uint8_t key = 0;
    };
}



const std::array<Key, 4> keysMap = {
    Key {.gpio = BTN_UP_GPIO_Port,    .pin = BTN_UP_Pin,    .key = 5 },
    Key {.gpio = BTN_DOWN_GPIO_Port,  .pin = BTN_DOWN_Pin,  .key = 8 },
    Key {.gpio = BTN_LEFT_GPIO_Port,  .pin = BTN_LEFT_Pin,  .key = 7 },
    Key {.gpio = BTN_RIGHT_GPIO_Port, .pin = BTN_RIGHT_Pin, .key = 9 },
};

constexpr std::size_t row_size = 66;
constexpr std::uint32_t fps = 60;


void i2c_bus_recover() {
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin | OLED_SDA_Pin, GPIO_PIN_SET);
    GPIO_InitTypeDef init = {};
    init.Pin = OLED_SCL_Pin | OLED_SDA_Pin;
    init.Mode = GPIO_MODE_OUTPUT_OD;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(OLED_SCL_GPIO_Port, &init);
    for (int i = 0; i < 9; ++i) {
        if (HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin) == GPIO_PIN_SET) break;
        HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
}

void app_main(UART_HandleTypeDef* huart2, I2C_HandleTypeDef* hi2c1, TIM_HandleTypeDef* htim3) {
    state.load_rom(rom);
    const auto start = HAL_GetTick();
    std::uint64_t next_frame = 0;
    std::uint64_t n = 0;
    bool error = false;

    if (HAL_I2C_IsDeviceReady(hi2c1, oled_address, 3, 100) != HAL_OK || !ssd1306_init(*hi2c1)) {
        error = true;
    }

    bool sound_timer_active = false;

    while (true) {
        if (error) {
            if (sound_timer_active) {
                HAL_TIM_PWM_Stop(htim3, TIM_CHANNEL_2);
                sound_timer_active = false;
            }
            HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
            HAL_Delay(100);
            continue;
        }
        if (HAL_GetTick() - start >= next_frame) {
            for (const auto &[gpio, pin, key]: keysMap) {
                if (HAL_GPIO_ReadPin(gpio, pin) == GPIO_PIN_RESET) {
                    if (!state.keys[key]) state.update_key(key, true);
                } else if (state.keys[key]) {
                    state.update_key(key, false);
                }
            }
            for (int i = 0; i < 100; ++i) {
                if (state.cycle() != Chip8Error::None) {
                    error = true;
                    break;
                }
            }

            if (!sound_timer_active && state.sound_timer > 0) {
                HAL_TIM_PWM_Start(htim3, TIM_CHANNEL_2);
                sound_timer_active = true;
            } else if (sound_timer_active && state.sound_timer == 0) {
                HAL_TIM_PWM_Stop(htim3, TIM_CHANNEL_2);
                sound_timer_active = false;
            }

            state.tick_timers();
            n++;
            next_frame = n * 1000 / fps;
        }
        if (state.screen_dirty) {

            // convert chip-8 screen to an uint8 array
            screen_buffer.fill(0x00);
            for (int page = 0; page < 8; ++page) {
                for (int c = 0; c < 128; ++c) {
                    for (int b = 0; b < 8; ++b) {
                        if ((state.screen[(page * 8 + b) / 2] >> static_cast<std::uint64_t>(c / 2)) & 1) { // pixel on
                            screen_buffer[128 * page + c] |= 1 << b;
                        }
                    }
                }
            }
            if (!ssd1306_draw(*hi2c1, screen_buffer)) error = true;
            state.screen_dirty = false;
        }
    }
}