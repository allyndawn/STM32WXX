/*
 * RadioTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_RADIOTASK_H_
#define SRC_RADIOTASK_H_

#include "stm32f4xx_hal.h"

enum RadioState {
	RADIOTASK_SEARCHING,
	RADIOTASK_READY
};

class RadioTask {
	public:
		RadioTask(I2C_HandleTypeDef *hi2c);
		virtual ~RadioTask();

		void runTask();
		void softReset();

	private:
		RadioState m_state;
		I2C_HandleTypeDef *m_hi2c;};

#endif /* SRC_RADIOTASK_H_ */
