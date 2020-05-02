/*
 * LCDTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_LCDTASK_H_
#define SRC_LCDTASK_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "TaskMessages.h"

enum LCDState {
	LCDTASK_POWER_ON,
	LCDTASK_READY
};

class LCDTask {
	public:
		LCDTask( UART_HandleTypeDef *huart, osMessageQueueId_t queue_handle );
		virtual ~LCDTask();

		void runTask();

	private:
		UART_HandleTypeDef *m_huart;
		osMessageQueueId_t m_queue_handle;
		LCDState m_state;

		char m_buffer[64];

		bool m_has_date;
		uint16_t m_year;
		uint8_t m_month;
		uint8_t m_day;
		uint8_t m_hour;
		uint8_t m_minutes;
		uint8_t m_seconds;

		bool m_has_thp;
		int16_t m_temperature;
		uint8_t m_humidity;
		uint16_t m_pressure;

		bool m_has_location;
		uint8_t m_lat_deg;
		uint8_t m_lat_min;
		uint8_t m_lat_sec;
		char m_lat_hem;
		uint8_t m_long_deg;
		uint8_t m_long_min;
		uint8_t m_long_sec;
		char m_long_hem;

		uint32_t m_os_ticks_per_second;
		uint32_t m_os_ticks_next_update;

		void processQueue();
		void processTHPMsg( thp_message *msg );
		void processDateMsg( gps_datetime_message *msg );
		void processLocationMsg( gps_location_message *msg );
};

#endif /* SRC_LCDTASK_H_ */
