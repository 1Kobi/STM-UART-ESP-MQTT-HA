#include "bmp280.h"

static uint8_t bmp280_addr = (0x76 << 1);
static uint8_t calib_addr = 0x88;
static uint8_t config_byte = 0x23;
static uint8_t config_addr = 0xF4;
static uint8_t data_addr = 0xFA;
static uint16_t dig_T1;
static int16_t dig_T2;
static int16_t dig_T3;

void BMP280_Init(I2C_HandleTypeDef *hi2c){
	uint8_t calib_data[6];

	HAL_I2C_Mem_Read(hi2c, bmp280_addr, calib_addr, 1, calib_data, 6, 100);

	dig_T1 = (calib_data[1] << 8) | calib_data[0];
	dig_T2 = (calib_data[3] << 8) | calib_data[2];
	dig_T3 = (calib_data[5] << 8) | calib_data[4];

	HAL_I2C_Mem_Write(hi2c, bmp280_addr, config_addr, 1, &config_byte, 1, 100);
}

float BMP280_ReadTemperature(I2C_HandleTypeDef *hi2c){
	uint8_t raw_temp_data[3] = {0, 0, 0};

	HAL_I2C_Mem_Read(hi2c, bmp280_addr, data_addr, 1, raw_temp_data, 3, 100);
	int32_t raw_temp = (raw_temp_data[0] << 12) | (raw_temp_data[1] << 4) | (raw_temp_data[2] >> 4);

	int32_t var1, var2, t_fine, T;
	var1 = ((((raw_temp >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((raw_temp >> 4) - ((int32_t)dig_T1)) * ((raw_temp >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;

	return T / 100.0f;
}
