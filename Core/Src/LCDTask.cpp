/*
 * LCDTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "LCDTask.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"

LCDTask::LCDTask( UART_HandleTypeDef *huart, osMessageQueueId_t queue_handle ) {
	m_huart = huart;
	m_queue_handle = queue_handle;

	m_rtc_handle = NULL;
	m_last_rtc_status = HAL_OK;
	m_time = { 0 };
	m_date = { 0 };

	m_state = LCDTASK_POWER_ON;

	m_has_location = false;
	m_has_thp = false;

	m_os_ticks_per_second = osKernelGetTickFreq(); // Kernel ticks per second
	m_os_ticks_next_update = osKernelGetTickCount();
}

LCDTask::~LCDTask() {
	// TODO Auto-generated destructor stub
}

void LCDTask::setRTCHandle( RTC_HandleTypeDef *hrtc ) {
	m_rtc_handle = hrtc;
}

void LCDTask::getRTCDateTime() {
	if ( ! m_rtc_handle ) {
		return;
	}

	m_last_rtc_status = HAL_RTC_GetTime( m_rtc_handle, &m_time, RTC_FORMAT_BIN );
	if ( m_last_rtc_status != HAL_OK ) {
		return;
	}

	m_last_rtc_status = HAL_RTC_GetDate( m_rtc_handle, &m_date, RTC_FORMAT_BIN );
	if ( m_last_rtc_status != HAL_OK ) {
		return;
	}
}

void LCDTask::processQueue() {
	char msg[9];

	osStatus_t status;

	do {
		status = osMessageQueueGet( m_queue_handle, (void *) &(msg[0]), NULL, 0U );
		if ( osOK == status ) {
			// Handle THP (E) messages
			if ( 'E' == msg[0] ) {
				this->processTHPMsg( (thp_message *) &(msg[0]) );
			}
			// Handle Location (L) messages
			if ( 'L' == msg[0] ) {
				this->processLocationMsg( (gps_location_message *) &(msg[0]) );
			}
		}
	} while ( osOK == status );
}

void LCDTask::processTHPMsg( thp_message *msg ) {
	m_has_thp = true;
	m_temperature = msg->temperature;
	m_humidity = msg->humidity;
	m_pressure = msg->pressure;
}

void LCDTask::processLocationMsg( gps_location_message *msg ) {
	m_has_location = true;
	m_lat_deg = msg->lat_deg;
	m_lat_min = msg->lat_min;
	m_lat_sec = msg->lat_sec;
	m_lat_hem = msg->lat_hem;
	m_long_deg = msg->long_deg;
	m_long_min = msg->long_min;
	m_long_sec = msg->long_sec;
	m_long_hem = msg->long_hem;
}

void LCDTask::prepareTimeDisplay() {
	//if ( ! m_has_location ) { // Use location as a proxy for having time
		strcpy( m_buffer, "Waiting for GPS No time data" );
	//} else {
		strcpy( m_buffer,  "   " );
		if ( m_date.Month < 10 ) {
			strcat( m_buffer, "0" );
		}
		itoa( m_date.Month, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "/" );
		if ( m_date.Date < 10 ) {
			strcat( m_buffer, "0" );
		}
		itoa( m_date.Date, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "/" );
		itoa( 2000 + m_date.Year, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer,  "   " );

		strcat( m_buffer,  "  " );
		if ( m_time.Hours < 10 ) {
			strcat( m_buffer, "0" );
		}
		itoa( m_time.Hours, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, ":" );
		if ( m_time.Minutes < 10 ) {
			strcat( m_buffer, "0" );
		}
		itoa( m_time.Minutes, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, ":" );
		if ( m_time.Seconds < 10 ) {
			strcat( m_buffer, "0" );
		}
		itoa( m_time.Seconds, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, " UTC" );
	//}
}

void LCDTask::prepareWeatherDisplay() {
	if ( ! m_has_thp ) {
		strcpy( m_buffer, "Waiting for THP No weather data" );
	} else {
		strcpy( m_buffer, "   " );
		itoa( m_temperature, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "\xDF" );
		strcat( m_buffer, "C " );

		itoa( m_humidity, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "%RH" );
		strcat( m_buffer, "   ");

		strcat( m_buffer, "     ");
		itoa( m_pressure, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "hPa" );
	}
}

void LCDTask::prepareLocationDisplay() {
	if ( ! m_has_location ) {
		strcpy( m_buffer, "Waiting for GPS No location data" );
	} else {
		strcpy( m_buffer, "   " );
		itoa( m_lat_deg, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "\xDF" );

		if ( m_lat_min < 10 ) {
			strcat( m_buffer, " " );
		}
		itoa( m_lat_min, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "'" );

		if ( m_lat_sec < 10 ) {
			strcat( m_buffer, " " );
		}
		itoa( m_lat_sec, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "\"" );

		strcat( m_buffer, " " );
		strcat( m_buffer, "N" ); // TODO m_lat_hem

		strcat( m_buffer, "    " );

		itoa( m_long_deg, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "\xDF" );

		if ( m_long_min < 10 ) {
			strcat( m_buffer, " " );
		}
		itoa( m_long_min, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "'" );

		if ( m_long_sec < 10 ) {
			strcat( m_buffer, " " );
		}
		itoa( m_long_sec, m_temp_value, 10 );
		strcat( m_buffer, m_temp_value );
		strcat( m_buffer, "\"" );

		strcat( m_buffer, " " );
		strcat( m_buffer, "W" ); // TODO m_long_hem
	}
}

void LCDTask::runTask() {
	// Update ourselves with any inbound messages
	this->processQueue();

	// If we've not yet spoken to the LCD
	// Set it up the way we want it
	if ( LCDTASK_POWER_ON == m_state ) {
		m_buffer[0] = 0x7c;
		m_buffer[1] = 0x2d;
		HAL_UART_Transmit( m_huart, (uint8_t *) m_buffer, 2, 40 );
		m_state = LCDTASK_READY;
	}

	// If it isn't time to update yet, bail
	uint32_t m_os_ticks_now = osKernelGetTickCount();
	if ( m_os_ticks_next_update > m_os_ticks_now ) {
		return;
	}

	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_14, GPIO_PIN_SET );

	// Don't bother updating the LCD more than once every two seconds
	m_os_ticks_next_update = m_os_ticks_now + m_os_ticks_per_second;

	if ( LCDTASK_READY == m_state ) {
		// Fetch the current time from the RTC
		this->getRTCDateTime();

		uint32_t ticks_as_seconds = m_os_ticks_now / m_os_ticks_per_second;

		// Clear the LCD
		m_buffer[0] = 0x7c;
		m_buffer[1] = 0x2d;
		HAL_UART_Transmit( m_huart, (uint8_t *) m_buffer, 2, 40 );

		// Use modulus to rotate to one of three displays each second
		uint8_t ticks_mod = ticks_as_seconds % 9;

		// 0, 1, 2: Date and time
		if ( ticks_mod < 3 ) {
			this->prepareTimeDisplay();
		}

		// 3, 4, 5: Temp, Humidity, Press
		if ( ticks_mod == 3 || ticks_mod == 4 || ticks_mod == 5 ) {
			this->prepareWeatherDisplay();
		}

		// 6, 7, 8: Location
		if ( ticks_mod == 6 || ticks_mod == 7 || ticks_mod == 8 ) {
			this->prepareLocationDisplay();
		}


		HAL_UART_Transmit( m_huart, (uint8_t *) m_buffer, strlen( m_buffer ), 40 );
	}

	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_14, GPIO_PIN_RESET );
}
