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
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minutes;
	uint8_t seconds;
} gps_time;

typedef struct {
	uint8_t degrees;
	uint8_t minutes;
	uint8_t seconds;
	char hem;
} gps_angle;

typedef struct {
	gps_angle latitude;
	gps_angle longitude;
} gps_location;

#include "stm32f4xx_hal.h"

enum GPSTaskState {
	GPSTASK_POWER_ON,
	GPSTASK_INIT,
	GPSTASK_READY
};

#define GPSTASK_MAX_BUFFER_LENGTH 96
#define GPSTASK_GPRMC_TOKENS 12
#define GPSTASK_GPRMC_MAX_TOKEN_LENGTH 12

class GPSTask {
	public:
		GPSTask( UART_HandleTypeDef *huart, osMessageQueueId_t queue_handle );
		virtual ~GPSTask();

		void runTask();

	private:
		UART_HandleTypeDef *m_huart;
		osMessageQueueId_t m_queue_handle;

		char m_buffer[ GPSTASK_MAX_BUFFER_LENGTH ];
		uint8_t m_buffer_bytes;

		char m_scratchpad[GPSTASK_GPRMC_TOKENS][GPSTASK_GPRMC_MAX_TOKEN_LENGTH];
		gps_time m_time;
		gps_location m_location;

		int8_t intFromString( int8_t index, int8_t offset, int8_t length );
		bool processBuffer();
		void enqueueData();

		uint32_t m_sr_flags;
		uint32_t m_dr_flags;
};

#endif /* SRC_GPSTASK_H_ */
