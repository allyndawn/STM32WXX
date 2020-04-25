/*
 * THPTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "THPTask.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

THPTask::THPTask(const bme280_dev &dev) {
	m_dev = dev;
	m_state = THPTASK_SEARCHING;
}

THPTask::~THPTask() {
}

void THPTask::runTask() {
	int8_t rslt = BME280_OK;

	if ( THPTASK_SEARCHING == m_state ) {
		rslt = bme280_init( &m_dev );
		if ( BME280_OK == rslt ) {
			m_state = THPTASK_READY;

			// Set the blue LED on solid to indicate success
			HAL_GPIO_WritePin( GPIOB, GPIO_PIN_7, GPIO_PIN_SET );
		}
	}

	// If we haven't initialized successfully, blink the blue LED
	if ( THPTASK_SEARCHING == m_state ) {
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_7 );
		osDelay( 500 );
	}

}
