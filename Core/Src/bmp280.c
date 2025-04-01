#include "main.h"

uint8_t check_bmp280(void) {
	uint8_t reg = BMP280_CHIP_ID_REGISTER;
	uint8_t chip_id = 0;
	HAL_I2C_Master_Transmit(I2C, (BMP280_ADDR << 1), &reg, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(I2C, (BMP280_ADDR << 1) | 0x01, &chip_id, 1,
	HAL_MAX_DELAY);
	return chip_id == BMP280_CHIP_ID;
}

void read_bmp280(bmp280_raw_t *bmp280_data) {
	uint8_t reg = BMP280_DATA_REGISTER;
	uint8_t data[6];
	HAL_I2C_Master_Transmit(I2C, (BMP280_ADDR << 1), &reg, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(I2C, (BMP280_ADDR << 1) | 0x01, data, 6,
	HAL_MAX_DELAY);

	uint32_t u_temp = ((uint32_t) (data[0] << 12) | ((uint16_t) (data[1] << 4))
			| (data[2] >> 4));
	bmp280_data->temperature = (int32_t) u_temp;
	uint32_t p_pressure = ((uint32_t) ((data[3] << 12))
			| (uint16_t) ((data[4] << 4)) | (data[5] >> 4));
	bmp280_data->pressure = (int32_t) p_pressure;

}

void read_bmp280_calib_values(uint8_t *calib_regs, uint8_t len) {
	uint8_t reg = BMP280_CALIB_REGISTER;
	uint8_t data[24];
	HAL_I2C_Master_Transmit(I2C, (BMP280_ADDR << 1), &reg, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(I2C, (BMP280_ADDR << 1) | 0x01, data, 24,
	HAL_MAX_DELAY);

	if (len < 24) {
		return;
	}

	memcpy(calib_regs, data, 24);

}
// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
// t_fine carries fine temperature as global value

void parse_temp_press_calib_data(const uint8_t *reg_data,
		bmp280_calib_data_t *calib_data) {

	calib_data->dig_t1 = BMP280_CONCAT_BYTES(reg_data[1], reg_data[0]);
	calib_data->dig_t2 = (int16_t) BMP280_CONCAT_BYTES(reg_data[3],
			reg_data[2]);
	calib_data->dig_t3 = (int16_t) BMP280_CONCAT_BYTES(reg_data[5],
			reg_data[4]);
	calib_data->dig_p1 = BMP280_CONCAT_BYTES(reg_data[7], reg_data[6]);
	calib_data->dig_p2 = (int16_t) BMP280_CONCAT_BYTES(reg_data[9],
			reg_data[8]);
	calib_data->dig_p3 = (int16_t) BMP280_CONCAT_BYTES(reg_data[11],
			reg_data[10]);
	calib_data->dig_p4 = (int16_t) BMP280_CONCAT_BYTES(reg_data[13],
			reg_data[12]);
	calib_data->dig_p5 = (int16_t) BMP280_CONCAT_BYTES(reg_data[15],
			reg_data[14]);
	calib_data->dig_p6 = (int16_t) BMP280_CONCAT_BYTES(reg_data[17],
			reg_data[16]);
	calib_data->dig_p7 = (int16_t) BMP280_CONCAT_BYTES(reg_data[19],
			reg_data[18]);
	calib_data->dig_p8 = (int16_t) BMP280_CONCAT_BYTES(reg_data[21],
			reg_data[20]);
	calib_data->dig_p9 = (int16_t) BMP280_CONCAT_BYTES(reg_data[23],
			reg_data[22]);
}

void init_bmp280(uint8_t *calib_regs, bmp280_calib_data_t *bme_calib_data) {

	// check bmp280 device id
	// TODO: add timeout!

	uint8_t cnt = 0;
	while ( (check_bmp280() != 1) && (cnt < 100)) {
		cnt++;
		HAL_Delay(10);
	}

	if (cnt >= 100) {
		return;
	}

	// setting default configuration
	uint8_t config_data[2] = { BMP280_CONTROL_REGISTER, 0x27 }; // Control register
	HAL_I2C_Master_Transmit(I2C, (BMP280_ADDR << 1), config_data, 2,
	HAL_MAX_DELAY);

	uint8_t config_data2[2] = { BMP280_CONFIG_REGISTER, 0xA0 }; // Config register
	HAL_I2C_Master_Transmit(I2C, (BMP280_ADDR << 1), config_data2, 2,
	HAL_MAX_DELAY);

	// read and parse the calibration registers
	read_bmp280_calib_values(calib_regs, 24);
	parse_temp_press_calib_data(calib_regs, bme_calib_data);
}

// from the official bmp280 api
// see here: https://github.com/boschsensortec/BME280_SensorAPI
int32_t compensate_temperature(const bmp280_raw_t *uncomp_data,
		bmp280_calib_data_t *calib_data) {
	int32_t var1;
	int32_t var2;
	int32_t temperature;
	int32_t temperature_min = -4000;
	int32_t temperature_max = 8500;

	var1 = (int32_t) ((uncomp_data->temperature / 8)
			- ((int32_t) calib_data->dig_t1 * 2));
	var1 = (var1 * ((int32_t) calib_data->dig_t2)) / 2048;
	var2 = (int32_t) ((uncomp_data->temperature / 16)
			- ((int32_t) calib_data->dig_t1));
	var2 = (((var2 * var2) / 4096) * ((int32_t) calib_data->dig_t3)) / 16384;
	calib_data->t_fine = var1 + var2;
	temperature = (calib_data->t_fine * 5 + 128) / 256;

	if (temperature < temperature_min) {
		temperature = temperature_min;
	} else if (temperature > temperature_max) {
		temperature = temperature_max;
	}

	return temperature;
}

// from the official bmp280 api
// see here: https://github.com/boschsensortec/BME280_SensorAPI
uint32_t compensate_pressure(const bmp280_raw_t *uncomp_data,
		bmp280_calib_data_t *calib_data) {
	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t var4;
	uint32_t var5;
	uint32_t pressure;
	uint32_t pressure_min = 30000;
	uint32_t pressure_max = 110000;

	var1 = (((int32_t) calib_data->t_fine) / 2) - (int32_t) 64000;
	var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t) calib_data->dig_p6);
	var2 = var2 + ((var1 * ((int32_t) calib_data->dig_p5)) * 2);
	var2 = (var2 / 4) + (((int32_t) calib_data->dig_p4) * 65536);
	var3 = (calib_data->dig_p3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8;
	var4 = (((int32_t) calib_data->dig_p2) * var1) / 2;
	var1 = (var3 + var4) / 262144;
	var1 = (((32768 + var1)) * ((int32_t) calib_data->dig_p1)) / 32768;

	/* Avoid exception caused by division by zero */
	if (var1) {
		var5 = (uint32_t) ((uint32_t) 1048576) - uncomp_data->pressure;
		pressure = ((uint32_t) (var5 - (uint32_t) (var2 / 4096))) * 3125;

		if (pressure < 0x80000000) {
			pressure = (pressure << 1) / ((uint32_t) var1);
		} else {
			pressure = (pressure / (uint32_t) var1) * 2;
		}

		var1 = (((int32_t) calib_data->dig_p9)
				* ((int32_t) (((pressure / 8) * (pressure / 8)) / 8192)))
				/ 4096;
		var2 = (((int32_t) (pressure / 4)) * ((int32_t) calib_data->dig_p8))
				/ 8192;
		pressure = (uint32_t) ((int32_t) pressure
				+ ((var1 + var2 + calib_data->dig_p7) / 16));

		if (pressure < pressure_min) {
			pressure = pressure_min;
		} else if (pressure > pressure_max) {
			pressure = pressure_max;
		}
	} else {
		pressure = pressure_min;
	}

	return pressure;
}

