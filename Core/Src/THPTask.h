/*
 * THPTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_THPTASK_H_
#define SRC_THPTASK_H_

#include "bme280.h"
#include "cmsis_os.h"

enum THPState {
	THPTASK_SEARCHING,
	THPTASK_READY
};

class THPTask {
	public:
		THPTask(const bme280_dev &dev, osMessageQueueId_t queue_handle);
		virtual ~THPTask();

		void runTask();

	private:
		bme280_dev m_dev;
		osMessageQueueId_t m_queue_handle;

		THPState m_state;
		bme280_data m_data;

		void enqueueData();
};

#endif /* SRC_THPTASK_H_ */
