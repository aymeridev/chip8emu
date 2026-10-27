#include <array>
#include <cstdint>

#include "ssd1306.hpp"

bool init_ssd1306(I2C_HandleTypeDef &hi2c1) {
    auto screen = std::to_array<std::uint8_t>({
        0xAE, // turn off display
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0x8D, 0x14,
        0x20, 0x00,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0xCF,
        0xD9, 0xF1,
        0xDB, 0x40,
        0xA4,
        0xA6,
        0x2E,
        0xAF
    });


    return HAL_I2C_Mem_Write(&hi2c1, 0x3C << 1, 0x00,
        I2C_MEMADD_SIZE_8BIT, screen.data(), screen.size(), HAL_MAX_DELAY) == HAL_OK;
}