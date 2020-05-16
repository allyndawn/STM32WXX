/*
 * RadioTask.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: allensnook
 */

#include "RadioTask.h"
#include "TaskMessages.h"

#define RADIOTASK_DEFAULT_FREQ 144390

#define A1846S_CTL_REG 0x30

#define A1846S_CLK_MODE_REG 0x04
#define A1846S_PABIAS_REG 0x0A
#define A1846S_GPIO_MODE_REG 0x1F
#define A1846S_FREQ_HI_REG 0x29
#define A1846S_FREQ_LO_REG 0x2A
#define A1846S_TX_SOURCE_REG 0x3A
#define A1846S_RX_VOLUME_REG 0x44
#define A1846S_SQ_SHUT_THRESH_REG 0x49

const uint16_t sine_wave_array[32] = {2047, 1648, 1264, 910, 600, 345,
	156, 39,  0,  39,  156,  345,
	600, 910, 1264, 1648, 2048, 2447,
	2831, 3185, 3495, 3750, 3939, 4056,
	4095, 4056, 3939, 3750, 3495, 3185,
	2831, 2447
};

RadioTask::RadioTask( osMessageQueueId_t queue_handle ) {
	m_htim = NULL;
	m_hdac = NULL;

	m_queue_handle = queue_handle;
	m_state = RADIOTASK_SEARCHING;

	m_rx_on = false;
	m_tx_on = false;
	m_frequency = RADIOTASK_DEFAULT_FREQ;

	// Initial state for the bus
	this->setSDIOForOutput();
	this->setSDIO( true );
	this->setNCS( true );
	this->setSCLK( true );

	// User LED (red) off
	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_14, GPIO_PIN_RESET );
}

RadioTask::~RadioTask() {
	// TODO Auto-generated destructor stub
}

void RadioTask::setTimerHandle( TIM_HandleTypeDef *htim ) {
	m_htim = htim;
}

void RadioTask::setDACHandle( DAC_HandleTypeDef *hdac ) {
	m_hdac = hdac;
}

/**
 * Outputs a sinusoid continuously on PA4 (DAC_OUT1)
 * Requires DMA_NORMAL -> DMA_CIRCULAR in Core/Src/stm32f4xx_hal_msp.c
 *
 * Props Artyom Sinitsin https://elastic-notes.blogspot.com/p/blog-page_1.html
 */
bool RadioTask::transmitTest() {
	if ( ! m_htim ) {
		return false;
	}

	if ( ! m_hdac ) {
		return false;
	}

	HAL_TIM_Base_Start( m_htim );
	HAL_DAC_Start( m_hdac, DAC_CHANNEL_1 );
	HAL_DAC_Start_DMA( m_hdac, DAC_CHANNEL_1, (uint32_t*)sine_wave_array, 32, DAC_ALIGN_12B_R );

	// TODO

	return true;
}

bool RadioTask::testConnection() {
	uint16_t data = 0;
	if ( ! this->readWord( 0x00, &data ) ) {
		return false;
	}

	return (data == 0x1846);
}

bool RadioTask::initialize() {
	this->softReset();
	this->setClockMode( RADIOTASK_12_14_MHZ );

	this->writeWord( 0x09, 0x03AC );	// Set GPIO voltage to 3.3V

	this->writeWord( 0x0A, 0x47E0 );	// Set PGA gain [10:6]
	this->writeWord( 0x13, 0xA100 );
	this->writeWord( 0x1F, 0x5001 );	// GPIO7->VOX, GPIO0->CTC/DCS

	this->writeWord( 0x31, 0x0031 );
	this->writeWord( 0x33, 0x0AF2 );	// AGC

	this->writeWord( 0x41, 0x067F );	// Voice gain TX [6:0]
	this->writeWord( 0x44, 0x02FF );
	this->writeWord( 0x47, 0x7F2F );
	this->writeWord( 0x4F, 0x2C62 );
	this->writeWord( 0x53, 0x0094 );	// Compressor update time [6:0] /5.12 ms
	this->writeWord( 0x54, 0x2A18 );
	this->writeWord( 0x55, 0x0081 );
	this->writeWord( 0x56, 0x0B22 );	// SQ detect time
	this->writeWord( 0x57, 0x1C00 );
	this->writeWord( 0x58, 0x800D );
	this->writeWord( 0x5A, 0x0EDB );	// SQ and noise detect times
	this->writeWord( 0x63, 0x3FFF );	// Pre-emphasis bypass

	// Calibration
	this->writeWord( A1846S_CTL_REG, 0x00A4 );
	HAL_Delay( 100 );
	this->writeWord( A1846S_CTL_REG, 0x00A6 );
	HAL_Delay( 100 );
	this->writeWord( A1846S_CTL_REG, 0x0006 );
	HAL_Delay( 100 );

	this->setNarrowBand();
	HAL_Delay( 100 );

	// this->setFrequency( 144390 );
	this->setFrequency( 162550 );
	this->setVolume( 0x3, 0x3 );
	this->setRxOn( true );

	this->setTxSourceMic();
	this->setTxPower( 0 );
	this->setSquelchLowThreshold( -80 );
	this->setSquelchOn( false );

	return true;
}

bool RadioTask::softReset() {
	this->writeWord( A1846S_CTL_REG, 0x1 );
	HAL_Delay( 100 );
	this->writeWord( A1846S_CTL_REG, 0x4 );

	return true;
}


bool RadioTask::setClockMode( RadioClockMode mode ) {
	if ( mode == RADIOTASK_12_14_MHZ ) {
		return this->writeWord( A1846S_CLK_MODE_REG, 0x0FD1 );
	}

	return this->writeWord( A1846S_CLK_MODE_REG, 0x0FD0 );
}

bool RadioTask::setNarrowBand() {
	this->writeWord( 0x11, 0x3D37 );
	this->writeWord( 0x12, 0x0100 );
	this->writeWord( 0x15, 0x1100 );
	this->writeWord( 0x32, 0x4495 );
	this->writeWord( 0x34, 0x2B8E );
	this->writeWord( 0x3A, 0x40C3 );
	this->writeWord( 0x3C, 0x0F1E );
	this->writeWord( 0x3F, 0x28D0 );
	this->writeWord( 0x48, 0x20BE );
	this->writeWord( 0x60, 0x1BB7 );
	this->writeWord( 0x62, 0x0A10 );
	this->writeWord( 0x65, 0x2494 );
	this->writeWord( 0x66, 0xEB2E );

	this->writeWord( 0x7F, 0x0001 );
	this->writeWord( 0x05, 0x000C );
	this->writeWord( 0x06, 0x020C );
	this->writeWord( 0x07, 0x030C );
	this->writeWord( 0x08, 0x0324 );
	this->writeWord( 0x09, 0x1344 );
	this->writeWord( 0x0A, 0x3F44 );
	this->writeWord( 0x0B, 0x3F44 );
	this->writeWord( 0x0C, 0x3F44 );
	this->writeWord( 0x0D, 0x3F44 );
	this->writeWord( 0x0E, 0x3F44 );
	this->writeWord( 0x0F, 0x3F44 );
	this->writeWord( 0x12, 0xE0ED );
	this->writeWord( 0x13, 0xF2FE );
	this->writeWord( 0x14, 0x0A16 );
	this->writeWord( 0x15, 0x2424 );
	this->writeWord( 0x16, 0x2424 );
	this->writeWord( 0x17, 0x2424 );
	this->writeWord( 0x7F, 0x0000 );

	return true;
}

bool RadioTask::setWideBand() {
	this->writeWord( 0x11, 0x3D37 );
	this->writeWord( 0x12, 0x0100 );
	this->writeWord( 0x15, 0x1F00 );
	this->writeWord( 0x32, 0x7564 );
	this->writeWord( 0x34, 0x2B8E );
	this->writeWord( 0x3A, 0x44C3 );
	this->writeWord( 0x3C, 0x1930 );
	this->writeWord( 0x3F, 0x29D2 );
	this->writeWord( 0x48, 0x21C0 );
	this->writeWord( 0x60, 0x101E );
	this->writeWord( 0x62, 0x3767 );
	this->writeWord( 0x65, 0x248A );
	this->writeWord( 0x66, 0xFFAE );

	this->writeWord( 0x7F, 0x0001 );
	this->writeWord( 0x05, 0x000C );
	this->writeWord( 0x06, 0x0024 );
	this->writeWord( 0x07, 0x0214 );
	this->writeWord( 0x08, 0x0224 );
	this->writeWord( 0x09, 0x0314 );
	this->writeWord( 0x0A, 0x0324 );
	this->writeWord( 0x0B, 0x0344 );
	this->writeWord( 0x0C, 0x0384 );
	this->writeWord( 0x0D, 0x1384 );
	this->writeWord( 0x0E, 0x1B84 );
	this->writeWord( 0x0F, 0x3F84 );
	this->writeWord( 0x12, 0xE0EB );
	this->writeWord( 0x13, 0xF2FE );
	this->writeWord( 0x14, 0x0A16 );
	this->writeWord( 0x15, 0x2424 );
	this->writeWord( 0x16, 0x2424 );
	this->writeWord( 0x17, 0x2424 );
	this->writeWord( 0x7F, 0x0000 );

	return true;
}

bool RadioTask::setFrequency( uint32_t frequency ) {
	uint32_t freq_raw = frequency << 4;
	bool tx_on = m_tx_on;
	bool rx_on = m_rx_on;

	this->setRxOn( false );
	this->setTxOn( false );

	// TODO - handle special frequency exceptions
	this->writeWord( 0x05, 0x8763 );

	uint16_t top_half = (uint16_t) ( 0x3FFF & (freq_raw >> 16 ) );
	this->writeWord( A1846S_FREQ_HI_REG, top_half );

	uint16_t bottom_half = (uint16_t) ( freq_raw & 0xFFFF );
	this->writeWord( A1846S_FREQ_LO_REG, bottom_half );

	m_frequency = frequency;

	this->setRxOn( rx_on );
	this->setTxOn( tx_on );

	return true;
}

bool RadioTask::setTxOn( bool tx_on ) {
	uint16_t data = 0;

	if ( tx_on == m_tx_on ) {
		return true;
	}

	if ( tx_on ) {
		this->setRxOn( false );
	}

	if ( m_frequency >= 134000 && m_frequency <= 174000 ) {
		this->setTxBand( RADIOTASK_TWOMETER_BAND );
	} else if ( m_frequency >= 400000 && m_frequency <= 520000 ) {
		this->setTxBand( RADIOTASK_70CM_BAND );
	} else {
		return false;
	}

	HAL_Delay( 50 );

	if ( ! this->readWord( A1846S_CTL_REG, &data ) ) {
		return false;
	}

	// The TX bit is bit 6 (xxxx xxxx xTxx xxxx)
	if ( tx_on ) {
		data |= 0x0040;
	} else {
		data &= 0xFFBF;
	}

	if ( ! this->writeWord( A1846S_CTL_REG, data ) ) {
		return false;
	}

	m_tx_on = tx_on;

	return true;
}

bool RadioTask::setRxOn( bool rx_on ) {
	uint16_t data = 0;

	if ( rx_on == m_rx_on ) {
		return true;
	}

	if ( rx_on ) {
		this->setTxOn( false );
		this->setGPIOLow( 4 );
		this->setGPIOLow( 5 );
		HAL_Delay( 50 );
	}

	if ( ! this->readWord( A1846S_CTL_REG, &data ) ) {
		return false;
	}

	// The RX bit is bit 5 (xxxx xxxx xxTx xxxx)
	if ( rx_on ) {
		data |= 0x0020;
	} else {
		data &= 0xFFDF;
	}

	if ( ! this->writeWord( A1846S_CTL_REG, data ) ) {
		return false;
	}

	m_rx_on = rx_on;

	return true;
}

bool RadioTask::setVolume( uint8_t volume1, uint8_t volume2 ) {
	uint16_t data = 0;
	data = ( volume1 & 0xFF ) << 4;
	data |= (volume2 & 0xFF );

	return this->writeWord( A1846S_RX_VOLUME_REG, data );
}

bool RadioTask::setSquelchLowThreshold( int16_t threshold ) {
	uint16_t squelch = 137 + threshold;
	uint16_t data = 0;

	if ( ! this->readWord( A1846S_SQ_SHUT_THRESH_REG, &data ) ) {
		return false;
	}

	// The squelch shut bits are bits 6:0 ( xxxx xxxx xSSS SSSS )

	// Clear the bits
	data &= squelch & 0xFF80;

	// Set the bits
	data |= (squelch & 0x7F);

	return this->writeWord( A1846S_SQ_SHUT_THRESH_REG, data );
}

bool RadioTask::setSquelchOn( bool squelchOn ) {
	uint16_t data = 0;

	if ( ! this->readWord( A1846S_CTL_REG, &data ) ) {
		return false;
	}

	// The squelch bit is bit 3 ( xxxx xxxx xxxx Sxxx )

	if ( squelchOn ) {
		// Set the bit
		data |= 0x40;
	} else {
		// Clear the bit
		data &= 0xFFF7;
	}

	return this->writeWord( A1846S_CTL_REG, data );
}

bool RadioTask::setTxSourceMic() {
	uint16_t data = 0;

	if ( ! this->readWord( A1846S_TX_SOURCE_REG, &data ) ) {
		return false;
	}

	// The selection bits are bits 14:12 (xSSS xxxx xxxx xxxx)

	// Clear the selection bits
	data &= 0x8FFF;

	// Set for Mic, source 4 (100 0x4)
	data |= 0x4000;

	return this->writeWord( A1846S_TX_SOURCE_REG, data );
}

bool RadioTask::setTxPower( uint8_t power ) {
	uint16_t data = 0;
	uint16_t raw_power = power;

	// Cap at 0xF
	if ( raw_power > 0xF ) {
		raw_power = 0xF;
	}

	// Turn TX and RX off while we do this
	bool rx_on = m_rx_on;
	bool tx_on = m_tx_on;

	this->setRxOn( false );
	this->setTxOn( false );

	if ( ! this->readWord( A1846S_PABIAS_REG, &data ) ) {
		return false;
	}

	// The power bits are bits 14:11 (xPPP Pxxx xxxx xxxx)

	// Clear the bits
	data &= 0x87FF;

	// Set the power
	data |= raw_power << 11;

	if ( ! this->writeWord( A1846S_PABIAS_REG, data ) ) {
		return false;
	}

	this->setRxOn( rx_on );
	this->setTxOn( tx_on );

	return true;
}

bool RadioTask::setTxBand( RadioBand band ) {
	uint16_t data = 0;

	if ( band == RADIOTASK_TWOMETER_BAND ) {
		this->setGPIOLow( 4 );
		this->setGPIOHigh( 5 );
		return true;
	}

	if ( band == RADIOTASK_70CM_BAND ) {
		if ( ! this->readWord( A1846S_GPIO_MODE_REG, &data ) ) {
			return false;
		}

		// The mode bits are bits 11:8 (xxxx MMMM xxxx xxxx)

		// TODO - why not use a setGPIO___ call?

		// Set the bits
		data |= 0x0F00;

		return this->writeWord( A1846S_GPIO_MODE_REG, data );
	}

	return false;
}

bool RadioTask::setGPIOLow( uint8_t pin ) {
	uint16_t data = 0;
	uint16_t clear = 0xFF;
	uint16_t value = 0x10;

	// Pin:  7 6  5 4  3 2  1 0
	// Bits: xxxx xxxx xxxx xxxx

	// For low, need to set the pin pair to 0x10

	if ( ! this->readWord( A1846S_GPIO_MODE_REG, &data ) ) {
			return false;
		}

	data &= ~( clear << ( pin * 2 ) );
	data |= value << ( pin * 2 );

	return this->writeWord( A1846S_GPIO_MODE_REG, data );
}

bool RadioTask::setGPIOHigh( uint8_t pin ) {
	uint16_t data = 0;
	uint16_t clear = 0xFF;
	uint16_t value = 0x11;

	// Pin:  7 6  5 4  3 2  1 0
	// Bits: xxxx xxxx xxxx xxxx

	// For high, need to set the pin pair to 0x11

	if ( ! this->readWord( A1846S_GPIO_MODE_REG, &data ) ) {
			return false;
		}

	data &= ~( clear << ( pin * 2 ) );
	data |= value << ( pin * 2 );

	return this->writeWord( A1846S_GPIO_MODE_REG, data );
}

/**
 * Sets SDIO (PB5) for output (Master to Slave)
 */
void RadioTask::setSDIOForOutput() {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * Sets SDIO (PB5) for input (Slave to Master)
 */
void RadioTask::setSDIOForInput() {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * Set the SDIO pin (PB5) to on (high) or off (low).
 * Call setSDIOForOutput before calling this.
 */
void RadioTask::setSDIO( bool on ) {
	if ( on ) {
		HAL_GPIO_WritePin( GPIOB, GPIO_PIN_5, GPIO_PIN_SET );
	} else {
		HAL_GPIO_WritePin( GPIOB, GPIO_PIN_5, GPIO_PIN_RESET );
	}
}

/**
 * Get the SDIO pin (PB5) state.
 * Call setSDIOForInput before calling this.
 */
bool RadioTask::getSDIO() {
	GPIO_PinState pin_state = HAL_GPIO_ReadPin( GPIOB, GPIO_PIN_5 );
	return ( pin_state == GPIO_PIN_SET );
}

/**
 * Set the CLK pin (PA5) on (high) or off (low)
 */
void RadioTask::setSCLK( bool on ) {
	if ( on ) {
		HAL_GPIO_WritePin( GPIOA, GPIO_PIN_5, GPIO_PIN_SET );
	} else {
		HAL_GPIO_WritePin( GPIOA, GPIO_PIN_5, GPIO_PIN_RESET );
	}
}

/**
 * Set the NCS pin (PG9) on (slave not selected) or off (slave selected)
 */
void RadioTask::setNCS( bool on ) {
	if ( on ) {
		HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_SET );
	} else {
		HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_RESET );
	}
}

bool RadioTask::writeWord( uint8_t reg, uint16_t data ) {
	int8_t bit = 0;

	// Set SDIO for output
	this->setSDIOForOutput();
	// Set SDIO high
	this->setSDIO( true );
	// Set SCLK high
	this->setSCLK( true );

	// Take NCS low to get slave to listen
	this->setNCS( false );

	// Send the register address to write to
	// For each bit in the register address, starting with the MSb
	for ( bit = 7; bit >= 0; bit-- ) {
		// Take SCLK low
		this->setSCLK( false );
		// Set SDIO to the bit value
		this->setSDIO( reg & ( 1 << bit ) );
		// Sleep 1 us
		HAL_Delay( 1 ); // 1 ms for now
		// Take the SCLK high to prompt slave to read
		this->setSCLK( true );
		// Sleep 1 us
		HAL_Delay( 1 ); // 1 ms for now
	}

	// For each bit in the data
	for ( bit = 15; bit >= 0; bit-- ) {
		// Take SCLK low
		this->setSCLK( false );
		// Set SDIO to the bit value
		this->setSDIO( data & ( 1 << bit ) );
		// Sleep 1 us
		HAL_Delay( 1 ); // 1 ms for now
		// Take the SCLK high to prompt slave to read
		this->setSCLK( true );
		// Sleep 1 us
		HAL_Delay( 1 ); // 1 ms for now
	}

	// We're done, take NCS high
	this->setNCS( true );

	return true;
}

bool RadioTask::readWord( uint8_t reg, uint16_t *data ) {
	int8_t bit = 0;
	uint16_t rx_data = 0;

	// Set SDIO for output
	this->setSDIOForOutput();
	// Set SDIO high
	this->setSDIO( true );
	// Set SCLK high
	this->setSCLK( true );

	// Take NCS low to get slave to listen
	this->setNCS( false );

	// To read, we need to force the MSb on
	reg |= 0x80;

	// Send the register address to write to
	// For each bit in the register address, starting with the MSb
	for ( bit = 7; bit >= 0; bit-- ) {
		// Take SCLK low
		this->setSCLK( false );
		// Set SDIO to the bit value
		this->setSDIO( reg & ( 1 << bit ) );
		// Sleep 1 us
		HAL_Delay( 1 ); // 1 ms for now
		// Take the SCLK high to prompt slave to read
		this->setSCLK( true );
		// Sleep 1 us
		HAL_Delay( 1 ); // 1 ms for now
	}

	// Set SDIO for input
	this->setSDIOForInput();

	// For each bit in the data
	for ( bit = 15; bit >= 0; bit-- ) {
		// Take SCLK low
		this->setSCLK( false );
		// Sleep 1 us
		HAL_Delay( 1 ); // 1 ms for now
		// Take the SCLK high to prompt slave to write
		this->setSCLK( true );
		// TODO why not delay here to give slave time to set bit?
		// Get the bit
		if ( this->getSDIO() ) {
			rx_data |= 1 << bit;
		}
		// Sleep 1 us
		HAL_Delay( 1 ); // 1 ms for now
	}

	// We're done, take NCS high
	this->setNCS( true );

	// Return our data
	*data = rx_data;

	return true;
}

void RadioTask::runTask() {
	uint16_t delay = 250;

	if ( RADIOTASK_SEARCHING == m_state ) {
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_14 ); // Toggle user (red) LED
		if ( this->testConnection() ) {
			if ( this->initialize() ) { // Fast blinking - initialization failing
				m_state = RADIOTASK_READY;
				HAL_GPIO_WritePin( GPIOB, GPIO_PIN_14, GPIO_PIN_SET ); // User (red) LED on - test + initialize succeeded
			}
		} else {
			// Slow blinking : test connection failing
			delay = 1000;
			//m_state = RADIOTASK_RADIO_NOT_FOUND;
		}
	}


	osDelay( delay );
}

