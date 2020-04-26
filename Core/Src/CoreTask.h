/*
 * CoreTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_CORETASK_H_
#define SRC_CORETASK_H_

#include "cmsis_os.h"

class CoreTask {
	public:
		CoreTask(
			osMessageQueueId_t gps_queue_handle,
			osMessageQueueId_t lcd_queue_handle,
			osMessageQueueId_t radio_queue_handle,
			osMessageQueueId_t thp_queue_handle
		);
		virtual ~CoreTask();

		void runTask();

	private:
		osMessageQueueId_t m_gps_queue_handle;
		osMessageQueueId_t m_lcd_queue_handle;
		osMessageQueueId_t m_radio_queue_handle;
		osMessageQueueId_t m_thp_queue_handle;
};

#endif /* SRC_CORETASK_H_ */
