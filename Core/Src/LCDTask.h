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
		uint32_t m_uloopcount;
};

#endif /* SRC_LCDTASK_H_ */
