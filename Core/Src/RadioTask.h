/*
 * RadioTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_RADIOTASK_H_
#define SRC_RADIOTASK_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

enum RadioState {
	RADIOTASK_SEARCHING,
	RADIOTASK_READY
};

class RadioTask {
	public:
		RadioTask( SPI_HandleTypeDef *hspi, osMessageQueueId_t queue_handle );
		virtual ~RadioTask();

		void runTask();
		void softReset();

	private:
		SPI_HandleTypeDef *m_hspi;
		osMessageQueueId_t m_queue_handle;
		RadioState m_state;
};

#endif /* SRC_RADIOTASK_H_ */
