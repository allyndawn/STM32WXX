/*
 * THPTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "THPTask.h"
#include "cmsis_os.h"

#define BME280_ADDRESS 0x76

#define BME280_REGISTER_CHIPID 0xD0
#define BME280_REGISTER_SOFTRESET 0xE0
#define BME280_REGISTER_STATUS 0xF3

#define BME280_REGISTER_DIG_T1 0x88
#define BME280_REGISTER_DIG_T2 0x8A
#define BME280_REGISTER_DIG_T3 0x8C

#define BME280_REGISTER_DIG_P1 0x8E
#define BME280_REGISTER_DIG_P2 0x90
#define BME280_REGISTER_DIG_P3 0x92
#define BME280_REGISTER_DIG_P4 0x94
#define BME280_REGISTER_DIG_P5 0x96
#define BME280_REGISTER_DIG_P6 0x98
#define BME280_REGISTER_DIG_P7 0x9A
#define BME280_REGISTER_DIG_P8 0x9C
#define BME280_REGISTER_DIG_P9 0x9E

#define BME280_REGISTER_DIG_H1 0xA1,
#define BME280_REGISTER_DIG_H2 0xE1
#define BME280_REGISTER_DIG_H3 0xE3
#define BME280_REGISTER_DIG_H4 0xE4
#define BME280_REGISTER_DIG_H5 0xE5
#define BME280_REGISTER_DIG_H6 0xE7

THPTask::THPTask(I2C_HandleTypeDef *hi2c) {
	m_hi2c = hi2c;
	m_state = THPTASK_SEARCHING;
}

THPTask::~THPTask() {
}

void THPTask::runTask() {
	uint8_t buffer;

	// If we've not yet spoken to a device
	// attempt to read its chip ID
	if ( THPTASK_SEARCHING == m_state ) {
		// First, make sure this is the device we are expecting
		HAL_I2C_Mem_Read( m_hi2c, BME280_ADDRESS << 1, BME280_REGISTER_CHIPID, I2C_MEMADD_SIZE_8BIT, &buffer, sizeof(uint8_t), 100 );
		if ( 0x60 == buffer ) {
			// If so, send a reset
			buffer = 0xB6;
			//HAL_I2C_Mem_Write( m_hi2c, BME280_ADDRESS << 1, BME280_REGISTER_SOFTRESET, &buffer, sizeof(uint8_t), 100 );

			// Wait for calibration to be loaded
			//do {
			//	buffer = 0;
			//	halStatus = HAL_I2C_Mem_Read( m_hi2c, BME280_ADDRESS << 1, BME280_REGISTER_STATUS, I2C_MEMADD_SIZE_8BIT, &buffer, sizeof(uint8_t), 100 );
			//} while ( buffer & 0x01 );

			// Read the factory set trimming parameters


			// Update state to ready
			m_state = THPTASK_READY;
		}
	}

	// HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

	// HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

	// If it doesn't respond, return and
	// we will try again later

	// If we receive the expected chip ID
	// set the flag that we have heard from
	// the chip

	// If we haven't yet initialized the device
	// do a soft reset, read all the coefficients,
	// and setup the sampling, including forced mode

	// Set a flag that init has been completed

	// Request a forced measurement

	// Capture the temperature
	// Capture the humidity
	// Capture the pressure

	if ( THPTASK_SEARCHING == m_state ) {
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_7 ); // Blink the Blue LED
	}

	if ( THPTASK_READY == m_state ) {
		HAL_GPIO_WritePin( GPIOB, GPIO_PIN_7, GPIO_PIN_SET ); // Blue LED on solid
	}

	osDelay( 500 );

}
