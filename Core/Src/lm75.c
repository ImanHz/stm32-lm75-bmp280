#include "main.h"
uint8_t temp_data[2] = { 0, 0 };
int16_t read_lm75(void) {

	uint8_t reg = 0x00; // Temperature register
	HAL_I2C_Master_Transmit(I2C, (LM75_ADDR << 1), &reg, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(I2C, (LM75_ADDR << 1) | 0x01, temp_data, 2,
	HAL_MAX_DELAY);

	// Convert to temperature (LM75 uses 9-bit resolution)
	uint16_t temp_raw = (uint16_t) ((temp_data[0] << 8)) | (temp_data[1]);
	temp_raw = ((temp_raw) >> 7) & 255; // Right shift for 9-bit data
	return temp_raw;
}
