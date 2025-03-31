/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern I2C_HandleTypeDef hi2c2;

#define I2C &hi2c2

#define LM75_ADDR 0x48
#define BMP280_ADDR 0x76
#define BMP280_CHIP_ID 0x58

#define BMP280_CHIP_ID_REGISTER 0xD0
#define BMP280_CONTROL_REGISTER 0xF4
#define BMP280_CONFIG_REGISTER 0xF5
#define BMP280_CALIB_REGISTER 0x88
#define BMP280_DATA_REGISTER 0xF7

#define BME280_CONCAT_BYTES(msb, lsb)  (((uint16_t)msb << 8) | (uint16_t)lsb)


typedef struct {
	uint16_t dig_t1;

	int16_t dig_t2;

	int16_t dig_t3;

	uint16_t dig_p1;

	int16_t dig_p2;

	int16_t dig_p3;

	int16_t dig_p4;

	int16_t dig_p5;

	int16_t dig_p6;

	int16_t dig_p7;

	int16_t dig_p8;

	int16_t dig_p9;

	/*! Variable to store the intermediate temperature coefficient */
	int32_t t_fine;
} bme280_calib_data_t;

typedef struct {
	int32_t pressure;
	int32_t temperature;
} bmp280_raw_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
int16_t read_lm75(void);

uint8_t check_bmp280(void);
void read_bmp280(bmp280_raw_t *bmp280_data);
void read_bmp280_calib_values(uint8_t *calib_regs, uint8_t len);
void parse_temp_press_calib_data(const uint8_t *reg_data,
		bme280_calib_data_t *calib_data);
void init_bmp280(uint8_t* calib_regs, bme280_calib_data_t* bme_calib_data);
int32_t compensate_temperature(const bmp280_raw_t *uncomp_data,
		bme280_calib_data_t *calib_data);
uint32_t compensate_pressure(const bmp280_raw_t *uncomp_data,
		bme280_calib_data_t *calib_data);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
