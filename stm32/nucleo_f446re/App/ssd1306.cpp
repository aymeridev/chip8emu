#include <array>
#include <cstdint>

#include "ssd1306.hpp"

bool ssd1306_init(I2C_HandleTypeDef &hi2c1) {
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


    return HAL_I2C_Mem_Write(&hi2c1, oled_address, 0x00,
        I2C_MEMADD_SIZE_8BIT, screen.data(), screen.size(), HAL_MAX_DELAY) == HAL_OK;
}


bool ssd1306_draw(I2C_HandleTypeDef &hi2c1, const std::array<std::uint8_t, 1024> &buffer) {
    auto screen = std::to_array<std::uint8_t>({
    0x21, 0x00, 0x7F, // reset column
    0x22, 0x00, 0x07 // reset page
});
    if (HAL_I2C_Mem_Write(&hi2c1, oled_address, 0x00,
        I2C_MEMADD_SIZE_8BIT, screen.data(), screen.size(), HAL_MAX_DELAY) != HAL_OK) return false;

    if (HAL_I2C_Mem_Write(&hi2c1, oled_address, 0x40,
                    I2C_MEMADD_SIZE_8BIT, const_cast<std::uint8_t*>(buffer.data()), buffer.size(), HAL_MAX_DELAY) != HAL_OK) {
        return false;
                    }
    return true;
}

bool ssd1306_invert_screen(I2C_HandleTypeDef &hi2c1, bool invert) {

    std::uint8_t screen = invert ? 0xA7 : 0xA6;

    if (HAL_I2C_Mem_Write(&hi2c1, oled_address, 0x00,
        I2C_MEMADD_SIZE_8BIT, &screen, 1, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }
    return true;
}
