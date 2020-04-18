/*
 * CoreTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "CoreTask.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

CoreTask::CoreTask() {
	// TODO Auto-generated constructor stub

}

CoreTask::~CoreTask() {
	// TODO Auto-generated destructor stub
}

void CoreTask::runTask() {
  HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_0 );
  osDelay(100);
}
