#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include "stm32f7xx_hal.h"

void BMP280_Init(I2C_HandleTypeDef *hi2c);
float BMP280_ReadTemperature(I2C_HandleTypeDef *hi2c);


#endif /* INC_BMP280_H_ */
