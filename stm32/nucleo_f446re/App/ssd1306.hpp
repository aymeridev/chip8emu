#pragma once
#include "main.h"
constexpr std::uint16_t oled_address = 0x3C << 1;

bool init_ssd1306(I2C_HandleTypeDef &hi2c1);