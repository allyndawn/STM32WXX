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

	m_has_date = false;
	m_has_location = false;
	m_has_thp = false;

	m_os_ticks_per_second = osKernelGetTickFreq(); // Kernel ticks per second
	m_os_ticks_next_update = osKernelGetTickCount();
}

LCDTask::~LCDTask() {
	// TODO Auto-generated destructor stub
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
			// Handle Date (D) messages
			if ( 'D' == msg[0] ) {
				this->processDateMsg( (gps_datetime_message *) &(msg[0]) );
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

void LCDTask::processDateMsg( gps_datetime_message *msg ) {
	m_has_date = true;
	m_year = msg->year;
	m_month = msg->month;
	m_day = msg->day;
	m_hour = msg->hour;
	m_minutes = msg->minutes;
	m_seconds = msg->seconds;
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

void LCDTask::runTask() {
	char buffer[64];

	// Update ourselves with any inbound messages
	this->processQueue();

	// If we've not yet spoken to the device
	// Set it up the way we want it
	if ( LCDTASK_POWER_ON == m_state ) {
		buffer[0] = 0x7c;
		buffer[1] = 0x2d;
		buffer[2] = 0;
		HAL_UART_Transmit( m_huart, (uint8_t *) buffer, strlen( buffer ), 40 );
		m_state = LCDTASK_READY;
	}

	// If it isn't time to update yet, bail
	uint32_t m_os_ticks_now = osKernelGetTickCount();
	if ( m_os_ticks_next_update > m_os_ticks_now ) {
		return;
	}

	HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_14 ); // Toggle the red LED

	// Don't bother updating the LCD more than once every two seconds
	m_os_ticks_next_update = m_os_ticks_now + m_os_ticks_per_second;

	if ( LCDTASK_READY == m_state ) {
		uint32_t ticks_as_seconds = m_os_ticks_now / m_os_ticks_per_second;

		// Clear the LCD
		buffer[0] = 0x7c;
		buffer[1] = 0x2d;
		buffer[2] = 0;
		HAL_UART_Transmit( m_huart, (uint8_t *) buffer, strlen( buffer ), 40 );

		// Use modulus to rotate to one of three displays each second
		uint8_t ticks_mod_3 = ticks_as_seconds % 3;

		// 0: Date
		if ( 0 == ticks_mod_3 ) {
			if ( ! m_has_date ) {
				strcpy( buffer, "Waiting for GPS No time data" );
			} else {
				//sprintf(
				//	buffer,
				//	"   %2hu/%2hu/%4hu     %2hu:%2hu:%2hu UTC",
				//	m_month,
				//	m_day,
				//	m_year,
				//	m_hour,
				//	m_minutes,
				//	m_seconds
				//);
				sprintf(
					buffer,
					"  %2hu:%2hu:%2hu UTC",
					m_hour,
					m_minutes,
					m_seconds
				);
			}
		}

		// 1: Location
		if ( 1 == ticks_mod_3 ) {
			if ( ! m_has_location ) {
				strcpy( buffer, "Waiting for GPS No location data" );
			} else {
				//sprintf(
				//	buffer,
				//	"%3hu %2hu'%2hu\" %c    %3hu %2hu'%2hu\" %c",
				//	m_lat_deg,
				//	m_lat_min,
				//	m_lat_sec,
				//	m_lat_hem,
				//	m_long_deg,
				//	m_long_min,
				//	m_long_sec,
				//	m_long_hem
				//);
				strcpy( buffer, "Loc data ready" );
			}
		}

		// 2: Temp, Humidity, Press
		if ( 2 == ticks_mod_3 ) {
			if ( ! m_has_thp ) {
				strcpy( buffer, "Waiting for THP No weather data" );
			} else {
				//sprintf(
				//	buffer,
				//	"%3hu F %2hu%%RH     %4uhPa",
				//	m_temperature,
				//	m_humidity,
				//	m_pressure
				//);
				sprintf(
					buffer,
					"%3hu C %2hu%% RH",
					m_temperature,
					m_humidity
				);
			}
		}

		HAL_UART_Transmit( m_huart, (uint8_t *) buffer, strlen( buffer ), 40 );
	}
}
