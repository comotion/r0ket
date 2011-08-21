#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/print.h"

#include "avr/avrspi.h"


#include "usetable.h"





void ram(void) {
	uint8_t spiBuf=0;
	int failed = 0, progmode = 0;
	uint8_t sigbyte = 0, partfam = 0, partnum = 0;

	lcdClear();
	lcdPrintln("avrisp + spi");
	lcdRefresh();
	
	
	getInputWait();
	getInputWaitRelease();
};