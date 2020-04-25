/*
 * THPTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_THPTASK_H_
#define SRC_THPTASK_H_

#include "bme280.h"

enum THPState {
	THPTASK_SEARCHING,
	THPTASK_READY
};

class THPTask {
	public:
		THPTask(const bme280_dev &dev);
		virtual ~THPTask();

		void runTask();

	private:
		THPState m_state;
		bme280_dev m_dev;

};

#endif /* SRC_THPTASK_H_ */
