#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/print.h"

#include "avr/avrisp.h"

#include "filesystem/ff.h"


void ReinvokeISP(void);

/**************************************************************************/

void main_avrisp(void) {
	uint8_t loBuf = 0;
	char *filename = "main.hex";

	lcdClear();
	lcdPrintln("avrisp + spi");
	lcdRefresh();

	lcdPrintln("RST hi");
	lcdRefresh();
	avrspi_setReset(1);

	delayms(150);

	lcdPrintln("RST lo");
	lcdRefresh();
	avrspi_setReset(0);

	delayms(150);

	lcdPrintln("RST hi");
	lcdRefresh();
	avrspi_setReset(1);

	delayms(150);
	
	lcdPrintln("flash?");
	lcdRefresh();
	
	getInputWait();
	getInputWaitRelease();

	lcdClear();
	lcdPrintln("flashing");
	lcdRefresh();
	
	//loBuf = avrisp_flash(filename);	
	loBuf = avrisp_flash_dry(filename);

	lcdPrintln(IntToStr(loBuf,10,0));
	lcdRefresh();	

	lcdPrintln("done.");
	lcdRefresh();
	
	
	getInputWait();
	getInputWaitRelease();
}