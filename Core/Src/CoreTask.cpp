/*
 * CoreTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "CoreTask.h"
#include "stm32f4xx_hal.h"

CoreTask::CoreTask(
	osMessageQueueId_t gps_queue_handle,
	osMessageQueueId_t lcd_queue_handle,
	osMessageQueueId_t radio_queue_handle,
	osMessageQueueId_t thp_queue_handle
) {
	m_gps_queue_handle = gps_queue_handle;
	m_lcd_queue_handle = lcd_queue_handle;
	m_radio_queue_handle = radio_queue_handle;
	m_thp_queue_handle = thp_queue_handle;

	m_rtc_handle = NULL;
	m_last_rtc_status = HAL_OK;
	m_rtc_set = false;
}

CoreTask::~CoreTask() {
	// TODO Auto-generated destructor stub
}

void CoreTask::setRTCHandle( RTC_HandleTypeDef *hrtc ) {
	m_rtc_handle = hrtc;
}

void CoreTask::runTask() {
	//HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_SET );
	this->handleGPSInbound();
	this->handleTHPInbound();
	//HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_RESET );

	osDelay(100);
}

void CoreTask::handleGPSInbound() {
	if ( ! m_gps_queue_handle ) {
		return;
	}

	// Forward GPS location messages to the LCD and Radio
	// Update the RTC with GPS datetime messages
	m_os_status = osMessageQueueGet( m_gps_queue_handle, (void *) &(m_inbound_msg[0]), NULL, 0U );
	if ( m_os_status == osOK ) {
		// Is this a datetime (D) message? If so, set the clock
		if ( 'D' == m_inbound_msg[0] ) {
			this->setRTC();
		}

		// Is this a location (L) message? If so, forward it
		if ( 'L' == m_inbound_msg[0] ) {
			osMessageQueuePut( m_lcd_queue_handle, (void *) &(m_inbound_msg[0]), 0U, 0U );
			//osMessageQueuePut( m_radio_queue_handle, (void *) &(m_inbound_msg[0]), 0U, 0U );
		}
	}
}

void CoreTask::setRTC() {
	// Already set? Don't bother doing it again
	// TODO: Look at clock drift and consider re syncing periodically
	if ( m_rtc_set ) {
		return;
	}

	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	gps_datetime_message *gps_dt = (gps_datetime_message *) (&m_inbound_msg);

	sTime.Hours = gps_dt->hour;
	sTime.Minutes = gps_dt->minutes;
	sTime.Seconds = gps_dt->seconds;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	m_last_rtc_status = HAL_RTC_SetTime( m_rtc_handle, &sTime, RTC_FORMAT_BIN );
	if ( m_last_rtc_status != HAL_OK ) {
		return;
	}
	sDate.WeekDay = RTC_WEEKDAY_MONDAY; // TODO
	sDate.Month = gps_dt->month;
	sDate.Date = gps_dt->day;
	sDate.Year = gps_dt->year;

	m_last_rtc_status = HAL_RTC_SetDate( m_rtc_handle, &sDate, RTC_FORMAT_BIN );
	if ( m_last_rtc_status != HAL_OK ) {
		return;
	}

	m_rtc_set = true;
}

void CoreTask::handleTHPInbound() {
	if ( ! m_thp_queue_handle ) {
		return;
	}

	// Forward THP messages to the LCD and Radio
	m_os_status = osMessageQueueGet( m_thp_queue_handle, &(m_inbound_msg[0]), NULL, 0U );
	if ( m_os_status == osOK ) {
		osMessageQueuePut( m_lcd_queue_handle, (void *) &(m_inbound_msg[0]), 0U, 0U );
		//osMessageQueuePut( m_radio_queue_handle, (void *) &(m_inbound_msg[0]), 0U, 0U );
	}
}
