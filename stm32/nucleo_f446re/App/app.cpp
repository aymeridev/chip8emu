#include "app.h"
#include "chip8_state.hpp"
#include "roms.hpp"
#include "main.h"
#include "ssd1306.hpp"
#include <array>
#include <string_view>
#include "font5x7.hpp"

namespace {
    auto state = Chip8State();
    std::array<std::uint8_t, 1024> screen_buffer = {};
    bool menu = true;
    std::uint8_t menu_index = 0;

    struct KeyButton {
        GPIO_TypeDef* gpio {};
        std::uint16_t pin {};
    };

    bool audio_beep = false;
}



const std::array<KeyButton, 4> keysMap = {
    KeyButton {.gpio = BTN_UP_GPIO_Port,    .pin = BTN_UP_Pin    },
    KeyButton {.gpio = BTN_DOWN_GPIO_Port,  .pin = BTN_DOWN_Pin  },
    KeyButton {.gpio = BTN_LEFT_GPIO_Port,  .pin = BTN_LEFT_Pin  },
    KeyButton {.gpio = BTN_RIGHT_GPIO_Port, .pin = BTN_RIGHT_Pin },
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

void screen_to_array(std::array<std::uint8_t, 1024>& buffer, const std::array<std::uint64_t, screen_height>& screen) {
    buffer.fill(0x00);
    for (int page = 0; page < 8; ++page) {
        for (int c = 0; c < 128; ++c) {
            for (int b = 0; b < 8; ++b) {
                if ((screen[(page * 8 + b) / 2] >> static_cast<std::uint64_t>(c / 2)) & 1) { // pixel on
                    buffer[128 * page + c] |= 1 << b;
                }
            }
        }
    }
}

void draw_text(std::array<std::uint8_t, 1024>& buffer, int page, int column, std::string_view text) {
    for (int ch = 0; ch < text.size(); ++ch) {
        for (int x = 0; x < 5; ++x) {
            const std::uint8_t buf_col = (column + ch) * 6 + x;
            if (buf_col >= 128) return;
            buffer[128 * page + buf_col] = font[static_cast<std::uint8_t>(text[ch]) * 5 + x];
        }
    }
}

void draw_menu() {

}

bool is_button_released(GPIO_TypeDef* gpio, std::uint16_t pin) {
    if (HAL_GPIO_ReadPin(gpio, pin) == GPIO_PIN_RESET) {
        while (HAL_GPIO_ReadPin(gpio, pin) == GPIO_PIN_RESET) {}
        audio_beep = true;
        return true;
    }
    return false;
}

void app_main(UART_HandleTypeDef* huart2, I2C_HandleTypeDef* hi2c1, TIM_HandleTypeDef* htim3) {
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
        if (audio_beep) {
            HAL_TIM_PWM_Start(htim3, TIM_CHANNEL_2);
            HAL_Delay(50);
            HAL_TIM_PWM_Stop(htim3, TIM_CHANNEL_2);
            audio_beep = false;
        }
        if (menu) {

            if (is_button_released(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
                menu_index = (menu_index + 1) % roms.size();
            }

            if (is_button_released(BTN_UP_GPIO_Port, BTN_UP_Pin)) {
                menu_index = (menu_index - 1) % roms.size();
            }

            if (is_button_released(BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin) || BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET) {
                while (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET) {}
                audio_beep = true;
                state.load_rom(roms[menu_index].data);
                menu = false;
            }

            screen_buffer.fill(0);
            draw_text(screen_buffer, 0, 1, "===== chip8emu =====");
            for (int i = 0; i < roms.size(); ++i) {
                draw_text(screen_buffer, i + 1, 1, roms[i].name);
            }
            for (int i = 128 * (menu_index + 1); i < 128 * (menu_index + 2); ++i) {
                screen_buffer[i] = ~screen_buffer[i];
            }
            if (!ssd1306_draw(*hi2c1, screen_buffer)) error = true;
        } else {
            if (HAL_GetTick() - start >= next_frame) {
                for (int i = 0; i < 4; ++i) {
                    const auto&[gpio, pin] = keysMap[i];
                    const auto key = roms[menu_index].keys[i];
                    if (HAL_GPIO_ReadPin(gpio, pin) == GPIO_PIN_RESET) {
                        if (!state.keys[key]) state.update_key(key, true);
                    } else if (state.keys[key]) {
                        state.update_key(key, false);
                    }
                }

                if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET) {
                    while (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET) {}
                    audio_beep = true;
                    menu = true;
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
                screen_to_array(screen_buffer, state.screen);
                if (!ssd1306_draw(*hi2c1, screen_buffer)) error = true;
                state.screen_dirty = false;
            }
        }
    }
}