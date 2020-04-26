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
			m_dev.settings.osr_h = BME280_OVERSAMPLING_1X;
			m_dev.settings.osr_p = BME280_OVERSAMPLING_16X;
			m_dev.settings.osr_t = BME280_OVERSAMPLING_2X;
			m_dev.settings.filter = BME280_FILTER_COEFF_16;

			uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

			rslt = bme280_set_sensor_settings(settings_sel, &m_dev);
			if (BME280_OK == rslt) {
				uint32_t req_delay = bme280_cal_meas_delay(&(m_dev.settings));

				// TODO read and use min delay from sensor
				m_state = THPTASK_READY;
				// Set the blue LED on solid to indicate success
				HAL_GPIO_WritePin( GPIOB, GPIO_PIN_7, GPIO_PIN_SET );
			}
		}
	}

	if ( THPTASK_READY == m_state ) {
		// Set the sensor to forced mode to trigger a measurement
		rslt = bme280_set_sensor_mode( BME280_FORCED_MODE, &m_dev );
		if ( BME280_OK == rslt ) {
			osDelay( 100 );
			m_data = { 0 };
			rslt = bme280_get_sensor_data( BME280_ALL, &m_data, &m_dev );
		}

		osDelay( 1000 ); // Get updated temperature once per second
	}

	// If we haven't initialized successfully, blink the blue LED
	if ( THPTASK_SEARCHING == m_state ) {
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_7 );
		osDelay( 500 );
	}

}
