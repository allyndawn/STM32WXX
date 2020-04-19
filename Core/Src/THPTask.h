/*
 * THPTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_THPTASK_H_
#define SRC_THPTASK_H_

#include "stm32f4xx_hal.h"

enum THPState {
	THPTASK_SEARCHING,
	THPTASK_READY
};

class THPTask {
	public:
		THPTask(I2C_HandleTypeDef *hi2c);
		virtual ~THPTask();

		void runTask();

	private:
		THPState m_state;
		I2C_HandleTypeDef *m_hi2c;

};

#endif /* SRC_THPTASK_H_ */
