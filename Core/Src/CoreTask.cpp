/*
 * CoreTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "CoreTask.h"
#include "stm32f4xx_hal.h"

CoreTask::CoreTask(
	osMessageQueueId_t gps_queue_handle,
	osMessageQueueId_t lcd_queue_handle,
	osMessageQueueId_t radio_queue_handle,
	osMessageQueueId_t thp_queue_handle
) {
	m_gps_queue_handle = gps_queue_handle;
	m_lcd_queue_handle = lcd_queue_handle;
	m_radio_queue_handle = radio_queue_handle;
	m_thp_queue_handle = thp_queue_handle;
}

CoreTask::~CoreTask() {
	// TODO Auto-generated destructor stub
}

void CoreTask::runTask() {
	osStatus_t status;
	uint8_t msg[9];

	//HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_SET );

	// Forward GPS messages to the LCD and Radio
	status = osMessageQueueGet( m_gps_queue_handle, (void *) &(msg[0]), NULL, 0U );
	if ( status == osOK ) {
		osMessageQueuePut( m_lcd_queue_handle, (void *) &(msg[0]), 0U, 0U );
		//osMessageQueuePut( m_radio_queue_handle, (void *) &(msg[0]), 0U, 0U );
	}

	// Forward THP messages to the LCD and Radio
	status = osMessageQueueGet( m_thp_queue_handle, &(msg[0]), NULL, 0U );
	if ( status == osOK ) {
		osMessageQueuePut( m_lcd_queue_handle, (void *) &(msg[0]), 0U, 0U );
		//osMessageQueuePut( m_radio_queue_handle, (void *) &(msg[0]), 0U, 0U );
	}

	//HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_RESET );
	osDelay(100);
}
