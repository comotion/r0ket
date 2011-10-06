#include <stdlib.h>
#include <sysinit.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "funk/nrf24l01p.h"
#include "core/ssp/ssp.h"

#define CS_LOW()    gpioSetValue(RB_SPI_NRF_CS, 0)
#define CS_HIGH()   gpioSetValue(RB_SPI_NRF_CS, 1)
#define CE_LOW()    gpioSetValue(RB_NRF_CE, 0)
#define CE_HIGH()   gpioSetValue(RB_NRF_CE, 1)

// modified functions to get pakets
// original from source of r0ket firmware
void startPkt(void){

    nrf_write_reg(R_CONFIG,
            R_CONFIG_PRIM_RX| // Receive mode
            R_CONFIG_PWR_UP   // Power on
            );

    nrf_cmd(C_FLUSH_RX);
    nrf_write_reg(R_STATUS,0);

    CE_HIGH();
};

void endPkt(void){
    CE_LOW();
	nrf_cmd(C_FLUSH_RX);
    nrf_write_reg(R_STATUS,R_STATUS_RX_DR);
};

int pollPkt(uint8_t maxsize, uint8_t * pkt){
    uint8_t length = 0;
    uint8_t status = 0;

	delayms(8);
    status = nrf_cmd_status(C_NOP);

	if ((status & R_STATUS_RX_DR) != R_STATUS_RX_DR) {
		return 0;
	}

    if ((status & R_STATUS_RX_P_NO) == R_STATUS_RX_FIFO_EMPTY) {
#ifdef USB_CDC
        puts("FIFO empty, but RX?\r\n");
#endif
        nrf_write_reg(R_STATUS,R_STATUS_RX_DR);
        nrf_cmd(C_FLUSH_RX);
        delayms(1);
        nrf_write_reg(R_STATUS, 0);
    } else {
		nrf_read_long(C_R_RX_PL_WID, 1 ,&length);

		nrf_write_reg(R_STATUS,R_STATUS_RX_DR);
		if (length > 32 || length == 0){
		    return -1;							// no packet error
		}

		if (length > maxsize){
		    length = maxsize;
		}

		nrf_read_pkt(length, pkt);
	}

    return length;
};

// scan for packets on a given frequency, bitrate and preamble
uint16_t detect(uint8_t bitrate, uint8_t freqChannel, uint8_t noise, uint16_t dataLength) {
	#define BUFFER_SIZE 16

	const int16_t START_BYTE = 3;					// number of evaluated bytes
	uint8_t buffer[BUFFER_SIZE];					// buffer for packets
	uint32_t data[dataLength];						// variable for data being evaluated later
	int16_t length = 0;								// length of received packet
	uint32_t factor = 1;
	uint32_t mostCommon = 0;
	uint16_t mostHits = 0;
	uint16_t recentHits = 0;
	uint32_t recentNumber = 0;

	nrf_write_reg(0x05, freqChannel);				// set frequency
	nrf_write_reg(0x06, bitrate);					// set rate
	nrf_write_reg(0x0A, noise);						// the preamble on which we receive
	nrf_write_reg(0x10, noise);

	for (uint16_t k = 0; k < dataLength; ++k) {

		// initialize
		for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
			buffer[i] = 0x00;
		}

		// receive
		startPkt();
		length = pollPkt(BUFFER_SIZE, buffer);
		endPkt();

		// this shows registers for debug purposes
		/*for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
			buffer[i] = nrf_read_reg(i);
		}*/

		if (nrf_read_reg(0x07) & 0x40) {
			nrf_write_reg(0x07, 0x4E);
		}

		// write buffer to data array
		factor = 1;
		data[k] = 0;
		for(int16_t j = START_BYTE - 1; j >= 0; --j) {
			data[k] += factor * buffer[j];
			if (factor <= 0x00FFFF) {				
				factor *= 0x000100;
			}
		}

		// show data on the screen (too fast to read without a delay function)
		/*lcdClear();
		lcdPrintln("Receiving...");
		DoInt(75, 0, length);
		for (uint8_t i = 0; i < 4; ++i) {
			DoInt(20 * i, 15, buffer[i]);
		}
		for (uint8_t i = 0; i < 4; ++i) {
			DoInt(20 * i, 30, buffer[4 + i]);
		}
		for (uint8_t i = 0; i < 4; ++i) {
			DoInt(20 * i, 45, buffer[8 + i]);
		}
		for (uint8_t i = 0; i < 4; ++i) {
			DoInt(20 * i, 60, buffer[12 + i]);
		}
		lcdRefresh();
		delayms_queue(500);*/
	}
	
	// evaluation starts here
	lcdClear();
	lcdPrintln("Auswertung");
	lcdRefresh();
	// sorting
	for (uint16_t i = 0; i < dataLength - 1; ++i) {
		for (uint16_t j = 0; j < dataLength - i - 1; ++j) {
			if (data[j] > data[j + 1]) {
				uint32_t tmp = data[j];
				data[j] = data[j + 1];
				data[j + 1] = tmp;
			}
		}
	}
	
	// searching for most common bytes
	mostCommon = 0;
	mostHits = 0;
	recentHits = 0;
	recentNumber = 0;
	for (uint16_t i = 0; i < dataLength; ++i) {
		if (data[i] == 0) {
			continue;
		}
		// activate to evaluate ohne leading Cs of 3 bytes
		/*if ((data[i] & 0xF00000) != 0xC00000) {
			if (recentHits > mostHits) {
				mostCommon = recentNumber;
				mostHits = recentHits;
			}

			recentNumber = data[i];
			recentHits = 1;
			continue;
		}*/
		if (recentNumber == data[i]) {
			recentHits++;
		} else {
			if (recentHits > mostHits) {
				mostCommon = recentNumber;
				mostHits = recentHits;
			}

			recentNumber = data[i];
			recentHits = 1;
		}
	}
	if (recentHits > mostHits) {
		mostCommon = recentNumber;
		mostHits = recentHits;
	}

	DoInt(0, 15, mostHits);
	DoInt(0, 30, (mostCommon >> 16) & 0xFF);
	DoInt(20, 30, (mostCommon >> 8) & 0xFF);
	DoInt(40, 30, mostCommon & 0xFF);
	lcdRefresh();
	//delayms_queue(500);

	return mostHits;
}

void main_radi0(void) {
	const uint16_t DATA_LENGTH = 64;			// number of evaluated packets per call of detect
	uint32_t maxI = 0;
	uint16_t max = 0;
	uint16_t freqHits;

	lcdClear();
	lcdRefresh();
	nrf_init();
	// initialize register
	nrf_write_reg(0x00, 0x00);					// power off
	nrf_write_reg(0x01, 0x00);					// necessary to disable CRC
	nrf_write_reg(0x02, 0x01);					// open pipe 0
	nrf_write_reg(0x1C, 0x00);					// disabled dynamic payload length
	nrf_write_reg(0x1D, 0x00);					// disable several features

	nrf_write_reg(0x03, 0x00);					// set adress length to 2
	nrf_write_reg(0x07, 0x78);					// initialise status register
	// buffer size is fake decimal
	nrf_write_reg(0x11, 0x16);					// paket length
	nrf_write_reg(0x00, 0x70 | 0x03);			// power on, receive on

	for (uint16_t i = 0x0000; i < 0x0080; ++i) {
		/*freqHits = detect(0x00, i, 0x55, DATA_LENGTH);
		if (freqHits > max) {
			max = freqHits;
			maxI = 4 * i;
		}
		freqHits = detect(0x00, i, 0xAA, DATA_LENGTH);
		if (freqHits > max) {
			max = freqHits;
			maxI = 4 * i + 1;
		}*/
		freqHits = detect(0x0A, i, 0x55, DATA_LENGTH);
		if (freqHits > max) {
			max = freqHits;
			maxI = 4 * i + 2;
		}
		freqHits = detect(0x0A, i, 0xAA, DATA_LENGTH);
		if (freqHits > max) {
			max = freqHits;
			maxI = 4 * i + 3;
		}
	}

	lcdClear();
	lcdPrintln("Result:");
	DoString(0, 25, "max gleich:");
	DoInt(76, 25, max);
	DoString(0, 40, "bei Konfig:");
	DoInt(76, 40, maxI);
	lcdRefresh();
}
