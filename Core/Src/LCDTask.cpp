/*
 * LCDTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "LCDTask.h"
#include "string.h"
#include "stdio.h"

LCDTask::LCDTask( UART_HandleTypeDef *huart, osMessageQueueId_t queue_handle ) {
	m_huart = huart;
	m_queue_handle = queue_handle;
	m_state = LCDTASK_POWER_ON;
	m_uloopcount = 0;
}

LCDTask::~LCDTask() {
	// TODO Auto-generated destructor stub
}

void LCDTask::runTask() {
	char buffer[64];

	m_uloopcount++;

	// If we've not yet spoken to the device
	// Set it up the way we want it
	if ( LCDTASK_POWER_ON == m_state ) {
		buffer[0] = 0x7c;
		buffer[1] = 0x2d;
		buffer[2] = 0;
		HAL_UART_Transmit( m_huart, (uint8_t *) buffer, strlen( buffer ), 40 );
		m_state = LCDTASK_READY;
	}

	if ( LCDTASK_READY == m_state ) {
		// Clear the LCD
		buffer[0] = 0x7c;
		buffer[1] = 0x2d;
		buffer[2] = 0;
		HAL_UART_Transmit( m_huart, (uint8_t *) buffer, strlen( buffer ), 40 );

		// Write the loop count
		sprintf( buffer, "%lu", m_uloopcount );
		HAL_UART_Transmit( m_huart, (uint8_t *) buffer, strlen( buffer ), 40 );
	}

	osDelay( 800 );
}
