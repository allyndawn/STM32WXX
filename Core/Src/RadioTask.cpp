/*
 * RadioTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "RadioTask.h"
#include "TaskMessages.h"

#define A1846S_CTL_REG 0x30

RadioTask::RadioTask( SPI_HandleTypeDef *hspi, osMessageQueueId_t queue_handle ) {
	m_hspi = hspi;
	m_queue_handle = queue_handle;
	m_state = RADIOTASK_SEARCHING;
}

RadioTask::~RadioTask() {
	// TODO Auto-generated destructor stub
}

void RadioTask::runTask() {
	uint8_t tx_buffer[3];
	uint8_t rx_buffer[3];

	HAL_StatusTypeDef halStatus;

	if ( RADIOTASK_SEARCHING == m_state ) {
		HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_RESET ); // nCS LOW

		tx_buffer[0] = 0x80;
		tx_buffer[1] = 0;
		tx_buffer[2] = 0;
		rx_buffer[0] = 0;
		rx_buffer[1] = 0;
		rx_buffer[2] = 0;
		halStatus = HAL_SPI_Transmit( m_hspi, &tx_buffer[0], 1, 1 );
		halStatus = HAL_SPI_Receive( m_hspi, &rx_buffer[0], 2, 1 );

		HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_SET ); // nCS HIGH

		// this->softReset();
		//m_state = RADIOTASK_READY;
	}

	osDelay( 500 );
}

void RadioTask::softReset() {
	uint8_t data;

	//HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_RESET ); // nCS LOW
	osDelay(10);

	data = 0x01;
	//HAL_I2C_Mem_Write( m_hi2c, RADIO_ADDRESS << 1, A1846S_CTL_REG, sizeof(uint8_t), &data, sizeof(uint8_t), 100 );
	osDelay(100);

	data = 0x04;
	//HAL_I2C_Mem_Write( m_hi2c, RADIO_ADDRESS << 1, A1846S_CTL_REG, sizeof(uint8_t), &data, sizeof(uint8_t), 100 );

	//HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_SET ); // nCS HIGH
	osDelay(10);
}
