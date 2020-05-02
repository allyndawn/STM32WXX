/*
 * GPSTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "GPSTask.h"
#include "TaskMessages.h"
#include "string.h"
#include "stdlib.h"

GPSTask::GPSTask( UART_HandleTypeDef *huart, osMessageQueueId_t queue_handle ) {
	m_huart = huart;
	m_queue_handle = queue_handle;
	m_buffer_bytes = 0;
	m_time = { 0 };
	m_location = { 0 };
}

GPSTask::~GPSTask() {
	// TODO Auto-generated destructor stub
}

int8_t GPSTask::intFromString( int8_t index, int8_t offset, int8_t length ) {
	int8_t value = 0;
	int8_t digit = 0;
	for ( int8_t i=0; i < length; i++ ) {
		value *= 10;
		digit = m_scratchpad[index][offset + i];
		if ( digit >= '0' && digit <= '9' ) {
			value += digit - '0'; // Convert ASCII to number
		}
	}
	return value;
}

bool GPSTask::processBuffer() {
	// Is the buffer between 16 and 66 chars?
	if ( ( m_buffer_bytes < 16 ) | ( m_buffer_bytes > 66 ) ) {
		return false;
	}

	// Is this a $GPRMRC sentence?
	if ( strncmp( "$GPRMC", m_buffer, 6 ) != 0 ) {
		return false;
	}

	// Does it have exactly 11 commas?
	uint8_t comma_count = 0;
	for ( uint8_t i=0; i < m_buffer_bytes; i++ ) {
		if ( ',' == m_buffer[i] ) {
			comma_count++;
		}
	}
	if ( 12 != comma_count ) {
		return false;
	}

	// Does it have a checksum delimeter 3 characters from the end?
	if ( '*' != m_buffer[ m_buffer_bytes - 3 ] ) {
		return false;
	}

	// Calculate the checksum
	// A NEMA checksum is all the characters between the $ and the *
	// XOR'd with each other and then turned into a two digit hex value
	uint8_t calculated_checksum = 0;
	for ( uint8_t i=1; i < m_buffer_bytes - 3; i++ ) {
		calculated_checksum ^= (uint8_t) m_buffer[i];
	}

	// Check the checksum
	unsigned long checksum = strtoul( m_buffer + m_buffer_bytes - 2, NULL, 16 );
	if ( checksum != calculated_checksum ) {
		return false;
	}

	// If we have gotten this far, we're ready to parse the tokens
	// First, clear the tokens
	for ( int8_t i=0; i < GPSTASK_GPRMC_TOKENS; i++ ) {
		m_scratchpad[i][0] = 0;
	}

	int8_t tokenIndex = 0;

	// Now, tokenize into the scratchpad
	char dataChar[2];
	dataChar[0] = 0;
	dataChar[1] = 0;

	for ( uint8_t i=0; i < m_buffer_bytes; i++ ) {
		if ( ',' == m_buffer[i] ) {
			tokenIndex++;
		} else if (strlen( m_scratchpad[tokenIndex] ) < GPSTASK_GPRMC_MAX_TOKEN_LENGTH - 1 ) {
			dataChar[0] = m_buffer[i];
			strcat( m_scratchpad[tokenIndex], dataChar );
		}
	}

	// Lastly, extract all the values we want
	// Scratchpad Entry #2: Validity
	if ( 'A' != m_scratchpad[2][0] ) {
		return false;
	}

	gps_time new_time = { 0 };
	gps_location new_location = { 0 };

	// Scratchpad Entry #1: Time stamp (hhmmss.xx)
	if ( strlen( m_scratchpad[1] ) < 6 ) {
		return false;
	}
	new_time.hour = this->intFromString( 1, 0, 2 );
	new_time.minutes = this->intFromString( 1, 2, 2 );
	new_time.seconds = this->intFromString( 1, 4, 2 );

	// Scratchpad Entry #9: Date stamp (ddmmyy)
	if ( strlen( m_scratchpad[9] ) != 6 ) {
		return false;
	}
	new_time.year = this->intFromString( 9, 4, 2 );
	new_time.month = this->intFromString( 9, 2, 2 );
	new_time.day = this->intFromString( 9, 0, 2 );

	uint16_t seconds = 0;

	// Scratchpad Entry #3: Latitude (ddmm.xxxxx)
	if ( strlen( m_scratchpad[3] ) < 7 ) {
		return false;
	}
	if ( '.' != m_scratchpad[3][4] ) {
		return false;
	}
	new_location.latitude.degrees = this->intFromString( 3, 0, 2 );
	new_location.latitude.minutes = this->intFromString( 3, 2, 2 );
	seconds = this->intFromString( 3, 5, 2 );
	seconds = seconds * 60 / 100;
	new_location.latitude.seconds = seconds;

	// Scratchpad Entry #4: North/South (N or S)
	if ( 'N' != m_scratchpad[4][0] && 'S' != m_scratchpad[4][0] ) {
		return false;
	}
	new_location.latitude.hem = m_scratchpad[4][0];

	// Scratchpad Entry #5: Longitude (dddmm.xxxxx)
	if ( strlen( m_scratchpad[5] ) < 8 ) {
		return false;
	}
	if ( '.' != m_scratchpad[5][5] ) {
		return false;
	}
	new_location.longitude.degrees = this->intFromString( 5, 0, 3 );
	new_location.longitude.minutes = this->intFromString( 5, 3, 2 );
	seconds = this->intFromString( 5, 6, 2 );
	seconds = seconds * 60 / 100;
	new_location.longitude.seconds = seconds;

	// Scratchpad Entry #6: East/West (E or W)
	if ( 'E' != m_scratchpad[6][0] && 'W' != m_scratchpad[6][0] ) {
		return false;
	}
	new_location.longitude.hem = m_scratchpad[6][0];

	// All is well - persist the new time and location
	m_time = new_time;
	m_location = new_location;

	return true;
}

void GPSTask::runTask() {
	HAL_StatusTypeDef hal_status;
	uint8_t uc_received;

	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_SET );

	// Get as many characters as we can from the device, adding them to the buffer
	do {
		// Read any error flags so reception doesn't stall
		// (Don't care if we get errors - we can just wait for the next line)
		m_dr_flags = m_huart->Instance->DR;
		m_sr_flags = m_huart->Instance->SR;

		// Receive any bits
		hal_status = HAL_UART_Receive( m_huart, &uc_received, 1, 50 );
		if ( HAL_OK == hal_status ) {

			// End of line? Try to process it
			if ( m_buffer_bytes > 0 && ( 0x0A == uc_received || 0x0D == uc_received ) ) {
				if ( this->processBuffer() ) {
					this->enqueueData();
				}
				m_buffer_bytes = 0;
			} else {
				if ( '$' == uc_received ) {
					// First character of any sentence is always $
					// If we receive a $, throw away anything we have and start over
					m_buffer[ 0 ] = '$';
					m_buffer_bytes = 1;
				} else {
					// If we have a $ already in the buffer, keep appending
					// while space permits
					if ( ( m_buffer[ 0 ] == '$' ) && ( m_buffer_bytes < GPSTASK_MAX_BUFFER_LENGTH - 1 ) ) {
						m_buffer[ m_buffer_bytes ] = uc_received;
						m_buffer[ m_buffer_bytes + 1 ] = 0;
						m_buffer_bytes++;
					}
				}
			}
		}
	} while ( HAL_OK == hal_status );

	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_RESET );

	osDelay( 1000 );
}

void GPSTask::enqueueData() {
	gps_location_message gps_loc_msg = { 0 };
	gps_loc_msg.id = 'L';

	gps_loc_msg.lat_deg = m_location.latitude.degrees;
	gps_loc_msg.lat_min = m_location.latitude.minutes;
	gps_loc_msg.lat_sec = m_location.latitude.seconds;
	gps_loc_msg.lat_hem = m_location.latitude.hem;

	gps_loc_msg.long_deg = m_location.longitude.degrees;
	gps_loc_msg.long_min = m_location.longitude.minutes;
	gps_loc_msg.long_sec = m_location.longitude.seconds;
	gps_loc_msg.long_hem = m_location.longitude.hem;
	osMessageQueuePut( m_queue_handle, (void *) &gps_loc_msg, 0, 0 );

	gps_datetime_message gps_dt_msg = { 0 };
	gps_dt_msg.id = 'D';
	gps_dt_msg.year = m_time.year;
	gps_dt_msg.month = m_time.month;
	gps_dt_msg.day = m_time.day;
	gps_dt_msg.hour = m_time.hour;
	gps_dt_msg.minutes = m_time.minutes;
	gps_dt_msg.seconds = m_time.seconds;
	osMessageQueuePut( m_queue_handle, (void *) &gps_dt_msg, 0, 0 );
}
