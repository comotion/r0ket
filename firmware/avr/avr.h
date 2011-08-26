/*
  library to interface the 4quar1us m0dul
  which is an AVR mcu hooked up to the SPI bus
*/

#ifndef __AVR_H_
#define __AVR_H_

#include "core/projectconfig.h"

// GPIO connected to the AVR RESET pin
#define AVR_RESET RB_SPI_SS0

// Clock of the AVR
#define AVR_CLK 8000000L

// SPI bus speed should not be 72Mhz
// therefore, clock dividing has to be done
// attiny2313 with internal 8Mhz could be interfaced
// with a divider of 0x10 (1/16 of 72Mhz = 4.5Mhz)
//#define AVR_DIVIDER (CFG_CPU_CCLK/(AVR_CLK/4))+1
#define AVR_DIVIDER 0x10

// page size (see datasheet)
// for an attiny2313 its 16 words per page
#define AVR_PAGE_SIZE 16

// how much space to reserve for avr program (hexfile)
// if hexfile is larger, programming is not possible
// hexfile needs to be loaded to ram, so dataflash-spi doesnt
// interrupt avr-spi
#define PROG_BUFFER_SIZE 1024


#endif