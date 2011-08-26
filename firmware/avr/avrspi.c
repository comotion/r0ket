#include "avr/avrspi.h"

void avrspi_select(void) {
	// init the ssp on SPI port 0 (SPI1 would be awesome)
	sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);

	// set ssp clock divider so avr can handle bus speed (see avr.h)
	SCB_SSP0CLKDIV = AVR_DIVIDER;
	
	// set direction of RESET to output and bring hi (off)
	avrspi_setReset(1);
	gpioSetDir(AVR_RESET, gpioDirection_Output);
	avrspi_setReset(1);
}

void avrspi_deselect(void) {
	// set ssp clock divider back to fullspeed
	SCB_SSP0CLKDIV = SCB_SSP0CLKDIV_DIV1;
}

void avrspi_setReset(uint8_t state) {
	gpioSetValue(AVR_RESET, state);
	delayms(25);
}

uint8_t avrspi_txrx(uint8_t byteToSend) {
	uint8_t spiBuf = byteToSend;
	
	//sspSendReceive(0, (uint8_t *)&spiBuf, 1);
	avrspi_tx(byteToSend);
	spiBuf = avrspi_rx();

	return spiBuf;
}

void avrspi_tx(uint8_t byteToSend) {
	uint8_t spiBuf = byteToSend;
	
	gpioSetValue(RB_LED0, 1);
	sspSend(0, (uint8_t *)&spiBuf, 1);
	gpioSetValue(RB_LED0, 0);
}

uint8_t avrspi_rx(void) {
	uint8_t spiBuf;

	gpioSetValue(RB_LED2, 1);
	sspReceive(0, (uint8_t *)&spiBuf, 1);
	gpioSetValue(RB_LED2, 0);

	return spiBuf;
}