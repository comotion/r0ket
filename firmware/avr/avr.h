/*
  library to interface the 4quar1us m0dul
  which is an AVR mcu hooked up to the SPI bus
*/

#include "basic/basic.h"
#include "ssp/ssp.h"

// GPIO connected to the AVR RESET pin
#define AVR_RESET RB_SPI_SS0

// Clock of the AVR
#define AVR_CLK 8000000L

// SPI bus speed should not be 72Mhz
// therefore, clock dividing has to be done
// attiny2313 with internal 8Mhz could be interfaced
// with a divider of 0x10 (1/16 of 72Mhz = 4.5Mhz)
#define AVR_DIVIDER 0x10