#ifndef CHIP8EMU_APP_H
#define CHIP8EMU_APP_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_main(UART_HandleTypeDef* huart2, I2C_HandleTypeDef* hi2c1, TIM_HandleTypeDef* htim3);

void i2c_bus_recover(void);

#ifdef __cplusplus
}
#endif

#endif