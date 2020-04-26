/*
 * TaskMessages.h
 *
 *  Created on: Apr 26, 2020
 *      Author: allensnook
 */

#ifndef SRC_TASKMESSAGES_H_
#define SRC_TASKMESSAGES_H_

// Used for inter task communication
// All structures should be 9 bytes long (1 byte id + 8 data bytes)

typedef struct {
	int8_t id; // always 'E'
	int16_t temperature; // deg C
	uint8_t humidity; // %
	uint16_t pressure; // hPa
	uint8_t reserved[3];
} thp_message;

typedef struct {
	int8_t id; // always 'D'
	int16_t year;
	uint8_t month; // 1 - 12
	uint8_t day; // 1 - 31
	uint8_t hour; // 0 - 23
	uint8_t minutes; // 0 - 59
	uint8_t seconds; // 0 - 59
	uint8_t reserved;
} gps_datetime_message;

typedef struct {
	int8_t id; // always 'L'
	uint8_t lat_deg; // 0 - 90
	uint8_t lat_min; // 0 - 59
	uint8_t lat_sec; // 0 - 59
	char lat_hem; // 'N' or 'S'
	uint8_t long_deg; // 0 - 90
	uint8_t long_min; // 0 - 59
	uint8_t long_sec; // 0 - 59
	char long_hem; // 'W' or 'E'
} gps_location_message;

#endif /* SRC_TASKMESSAGES_H_ */
