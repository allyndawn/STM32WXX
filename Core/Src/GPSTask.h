/*
 * GPSTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_GPSTASK_H_
#define SRC_GPSTASK_H_

#include "stdint.h"
#include "cmsis_os.h"

typedef struct {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} gps_time;

typedef struct {
	int8_t degrees;
	uint8_t minutes;
	uint8_t seconds;
} gps_angle;

typedef struct {
	gps_angle latitude;
	gps_angle longitude;
} gps_location;

#include "stm32f4xx_hal.h"

#define GPSTASK_MAX_BUFFER_LENGTH 96

class GPSTask {
	public:
		GPSTask( UART_HandleTypeDef *huart, osMessageQueueId_t queue_handle );
		virtual ~GPSTask();

		void runTask();

	private:
		UART_HandleTypeDef *m_huart;
		osMessageQueueId_t m_queue_handle;

		char m_buffer[ GPSTASK_MAX_BUFFER_LENGTH ];
		gps_time m_time;
		gps_location m_location;

		int8_t intFromString( char *buffer, int8_t offset, int8_t length );
		bool processBuffer();
};

#endif /* SRC_GPSTASK_H_ */
