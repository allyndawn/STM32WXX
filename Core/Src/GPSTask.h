/*
 * GPSTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_GPSTASK_H_
#define SRC_GPSTASK_H_

#include "stm32f4xx_hal.h"

#define GPSTASK_MAX_BUFFER_LENGTH 200

class GPSTask {
	public:
		GPSTask( UART_HandleTypeDef *huart );
		virtual ~GPSTask();

		void processBuffer();
		void runTask();

	private:
		UART_HandleTypeDef *m_huart;
		char m_buffer[ GPSTASK_MAX_BUFFER_LENGTH ];
};

#endif /* SRC_GPSTASK_H_ */
