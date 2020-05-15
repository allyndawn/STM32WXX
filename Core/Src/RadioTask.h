/*
 * RadioTask.h
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#ifndef SRC_RADIOTASK_H_
#define SRC_RADIOTASK_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

enum RadioState {
	RADIOTASK_SEARCHING,
	RADIOTASK_RADIO_NOT_FOUND,
	RADIOTASK_READY
};

enum RadioBand {
	RADIOTASK_TWOMETER_BAND,
	RADIOTASK_70CM_BAND
};

enum RadioClockMode {
	RADIOTASK_12_14_MHZ,
	RADIOTASK_24_28_MHZ
};
class RadioTask {
	public:
		RadioTask( osMessageQueueId_t queue_handle );
		virtual ~RadioTask();

		void setTimerHandle( TIM_HandleTypeDef *htim );
		void setDACHandle( DAC_HandleTypeDef *hdac );

		void runTask();

	private:
		TIM_HandleTypeDef *m_htim;
		DAC_HandleTypeDef *m_hdac;

		osMessageQueueId_t m_queue_handle;
		RadioState m_state;

		bool m_rx_on;
		bool m_tx_on;
		uint32_t m_frequency;

		bool testConnection();

		bool transmitTest();

		bool initialize();
		bool softReset();

		bool setClockMode( RadioClockMode mode );
		bool setNarrowBand();
		bool setWideBand();
		bool setFrequency( uint32_t frequencyKHZ );
		bool setRxOn( bool rxOn );

		bool setVolume( uint8_t volume1, uint8_t volume2 );
		bool setSquelchLowThreshold( int16_t threshold );
		bool setSquelchOn( bool squelchOn );

		bool setTxOn( bool txOn );
		bool setTxSourceMic();
		bool setTxPower( uint8_t power );
		bool setTxBand( RadioBand band );

		bool setGPIOLow( uint8_t pin );
		bool setGPIOHigh( uint8_t pin );

		void setSDIOForOutput();
		void setSDIOForInput();

		void setSDIO( bool on );
		bool getSDIO();

		void setSCLK( bool on );
		void setNCS( bool on );

		bool writeWord( uint8_t reg, uint16_t data );
		bool readWord( uint8_t reg, uint16_t *data );
};

#endif /* SRC_RADIOTASK_H_ */
