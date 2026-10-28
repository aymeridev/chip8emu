#pragma once
#include "main.h"
constexpr std::uint16_t oled_address = 0x3C << 1;

bool ssd1306_init(I2C_HandleTypeDef &hi2c1);

bool ssd1306_draw(I2C_HandleTypeDef &hi2c1, std::array<std::uint8_t, 1024> &buffer);