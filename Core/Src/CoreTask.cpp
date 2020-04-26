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
  // TODO - read queues, forward messages to appropriate devices
  HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_0 ); // Blink the Green LED
  osDelay(500);
}
