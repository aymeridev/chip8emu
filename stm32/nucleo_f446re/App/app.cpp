#include "app.h"
#include "chip8_state.hpp"
#include "roms.hpp"
#include "main.h"
#include "ssd1306.hpp"
#include <array>
#include <string_view>
#include "menu.hpp"

namespace {
    auto state = Chip8State();
    std::array<std::uint8_t, 1024> screen_buffer = {};
    bool show_menu = true;
    Menu menu = {};
    bool invert_screen = false;

    struct KeyButton {
        GPIO_TypeDef* gpio {};
        std::uint16_t pin {};
    };

    struct Button {
        GPIO_TypeDef* gpio {};
        std::uint16_t pin {};
        bool was_pressed = false;
    };

    std::array menu_buttons = {
        Button {.gpio = BTN_UP_GPIO_Port,    .pin = BTN_UP_Pin    },
        Button {.gpio = BTN_DOWN_GPIO_Port,  .pin = BTN_DOWN_Pin  },
        Button {.gpio = BTN_RIGHT_GPIO_Port, .pin = BTN_RIGHT_Pin },
    };

    std::uint32_t start = 0;
    std::uint64_t next_frame = 0;
    std::uint64_t n = 0;
}

constexpr std::uint8_t BTN_UP_INDEX = 0;
constexpr std::uint8_t BTN_DOWN_INDEX = 1;
constexpr std::uint8_t BTN_RIGHT_INDEX = 2;


constexpr std::uint32_t fps = 60;

const std::array keysMap = {
    KeyButton {.gpio = BTN_UP_GPIO_Port,    .pin = BTN_UP_Pin    },
    KeyButton {.gpio = BTN_DOWN_GPIO_Port,  .pin = BTN_DOWN_Pin  },
    KeyButton {.gpio = BTN_LEFT_GPIO_Port,  .pin = BTN_LEFT_Pin  },
    KeyButton {.gpio = BTN_RIGHT_GPIO_Port, .pin = BTN_RIGHT_Pin },
};



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

bool just_pressed(Button& button) {
    const bool now = HAL_GPIO_ReadPin(button.gpio, button.pin) == GPIO_PIN_RESET;
    if (now && !button.was_pressed) {
        state.sound_timer = 5;
        button.was_pressed = now;
        return true;
    }
    button.was_pressed = now;
    return false;
}

void reset() {
    start = HAL_GetTick();
    next_frame = 0;
    n = 0;
    menu.dirty = true;
}

void app_main(UART_HandleTypeDef* huart2, I2C_HandleTypeDef* hi2c1, TIM_HandleTypeDef* htim3) {

    bool error = false;

    if (HAL_I2C_IsDeviceReady(hi2c1, oled_address, 3, 100) != HAL_OK || !ssd1306_init(*hi2c1)) {
        error = true;
    }

    bool sound_timer_active = false;

    reset();
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
            if (show_menu) {

                if (just_pressed(menu_buttons[BTN_UP_INDEX])) {
                    menu.previous();
                }

                if (just_pressed(menu_buttons[BTN_DOWN_INDEX])) {
                    menu.next();
                }

                if (just_pressed(menu_buttons[BTN_RIGHT_INDEX])) {
                    switch (menu.action()) {
                        case MenuAction::LOAD_ROM:
                            reset();
                            state.load_rom(roms[menu.selected()].data);
                            show_menu = false;
                            break;
                        case MenuAction::INVERT:
                            invert_screen = !invert_screen;
                            if (!ssd1306_invert_screen(*hi2c1, invert_screen)) error = true;
                            break;
                    }
                }


                if (menu.dirty) {
                    menu.draw_menu(screen_buffer);
                    if (ssd1306_draw(*hi2c1, screen_buffer)) {
                        menu.dirty = false;
                    } else {
                        error = true;
                    }
                }
            } else {
                for (int i = 0; i < 4; ++i) {
                    const auto&[gpio, pin] = keysMap[i];
                    const auto key = roms[menu.selected()].keys[i];
                    if (HAL_GPIO_ReadPin(gpio, pin) == GPIO_PIN_RESET) {
                        if (!state.keys[key]) state.update_key(key, true);
                    } else if (state.keys[key]) {
                        state.update_key(key, false);
                    }
                }

                if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET) {
                    screen_buffer.fill(0);
                    reset();
                    show_menu = true;
                }

                for (int i = 0; i < 100; ++i) {
                    if (state.cycle() != Chip8Error::None) {
                        error = true;
                        break;
                    }
                }
                if (state.screen_dirty) {
                    screen_to_array(screen_buffer, state.screen);
                    if (!ssd1306_draw(*hi2c1, screen_buffer)) error = true;
                    state.screen_dirty = false;
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
    }
}