/*
 * CoreTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_CORETASK_H_
#define SRC_CORETASK_H_

#include "cmsis_os.h"
#include "TaskMessages.h"
#include "stm32f4xx_hal.h"

class CoreTask {
	public:
		CoreTask(
			osMessageQueueId_t gps_queue_handle,
			osMessageQueueId_t lcd_queue_handle,
			osMessageQueueId_t radio_queue_handle,
			osMessageQueueId_t thp_queue_handle
		);
		virtual ~CoreTask();

		void setRTCHandle( RTC_HandleTypeDef *hrtc );
		void runTask();

	private:
		osMessageQueueId_t m_gps_queue_handle;
		osMessageQueueId_t m_lcd_queue_handle;
		osMessageQueueId_t m_radio_queue_handle;
		osMessageQueueId_t m_thp_queue_handle;

		RTC_HandleTypeDef *m_rtc_handle;
		HAL_StatusTypeDef m_last_rtc_status;
		bool m_rtc_set;

		uint8_t m_os_status;
		uint8_t m_inbound_msg[TASKMESSAGES_MESSAGE_SIZE_BYTES];

		void handleGPSInbound();
		void setRTC();
		void handleTHPInbound();
};

#endif /* SRC_CORETASK_H_ */
