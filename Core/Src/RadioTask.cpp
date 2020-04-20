/*
 * RadioTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "RadioTask.h"
#include "cmsis_os.h"

// 21: NAK, 2E: NAK, 71: SCK Only, 72: SCK Only, 77: SCK Only

#define RADIO_ADDRESS 0x21
#define A1846S_CTL_REG 0x30

RadioTask::RadioTask(I2C_HandleTypeDef *hi2c) {
	m_hi2c = hi2c;
	m_state = RADIOTASK_SEARCHING;
}

RadioTask::~RadioTask() {
	// TODO Auto-generated destructor stub
}

void RadioTask::runTask() {
	uint8_t buffer[2];
	HAL_StatusTypeDef halStatus;

	if ( RADIOTASK_SEARCHING == m_state ) {
		HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_RESET ); // nCS LOW
		osDelay(10);

		halStatus = HAL_I2C_Mem_Read( m_hi2c, RADIO_ADDRESS << 1, 0x0, sizeof(uint8_t), &buffer[0], 2 * sizeof(uint8_t), 100 );
		osDelay(10);

		HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_SET ); // nCS HIGH
		osDelay(10);

		// this->softReset();
		//m_state = RADIOTASK_READY;
	}

	osDelay( 1000 );
}

void RadioTask::softReset() {
	uint8_t data;

	HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_RESET ); // nCS LOW
	osDelay(10);

	data = 0x01;
	HAL_I2C_Mem_Write( m_hi2c, RADIO_ADDRESS << 1, A1846S_CTL_REG, sizeof(uint8_t), &data, sizeof(uint8_t), 100 );
	osDelay(100);

	data = 0x04;
	HAL_I2C_Mem_Write( m_hi2c, RADIO_ADDRESS << 1, A1846S_CTL_REG, sizeof(uint8_t), &data, sizeof(uint8_t), 100 );

	HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_SET ); // nCS HIGH
	osDelay(10);
}
