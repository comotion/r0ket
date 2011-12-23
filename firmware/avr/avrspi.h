/*
  basic functions for the spi interaction with the avr
*/

#ifndef __AVRSPI_H_
#define __AVRSPI_H_

#include "basic/basic.h"
#include "ssp/ssp.h"

#include "avr/avr.h"

void avrspi_select(void);
void avrspi_deselect(void);
void avrspi_setReset(uint8_t state);

// send one byte and return the answer
uint8_t avrspi_txrx(uint8_t byteToSend);

// send one byte
void avrspi_tx(uint8_t byteToSend);

// receive one byte
uint8_t avrspi_rx(void);

#endif