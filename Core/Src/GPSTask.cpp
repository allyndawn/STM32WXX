/*
 * GPSTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "GPSTask.h"
#include "cmsis_os.h"
#include "string.h"

GPSTask::GPSTask( UART_HandleTypeDef *huart ) {
	m_huart = huart;
	m_buffer[0] = 0;
}

GPSTask::~GPSTask() {
	// TODO Auto-generated destructor stub
}

void GPSTask::processBuffer() {
	// TODO process the buffer

	// Clear the buffer
	m_buffer[0] = 0;
}

void GPSTask::runTask() {
	HAL_StatusTypeDef hal_status;
	uint8_t uc_received;

	// Get as many characters as we can from the device, adding them to the buffer
	do {
		hal_status = HAL_UART_Receive( m_huart, &uc_received, 1, 50 );
		if ( HAL_OK == hal_status ) {
			if ( 0x0A == uc_received || 0x0D == uc_received ) {
				this->processBuffer();
			} else {
				size_t c_buflen = strlen( m_buffer );
				if ( GPSTASK_MAX_BUFFER_LENGTH - 1 > c_buflen ) {
					m_buffer[ c_buflen ] = uc_received;
					m_buffer[ c_buflen + 1 ] = 0;
				}
			}
		}
	} while ( HAL_OK == hal_status );

	osDelay( 1000 );
}
