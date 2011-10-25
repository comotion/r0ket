#include <stdint.h>
#include <string.h>

#include "lpc134x.h"
#include "usb/usb.h"
#include "usbdesc.h"
#include "rom_drivers.h"


#include <sysinit.h>
#include "basic/basic.h"
#include "basic/config.h"
#include "core/i2c/i2c.h"
#include "core/timer32/timer32.h"
#include "lcd/allfonts.h"
#include "lcd/backlight.h"
#include "lcd/render.h"
#include "lcd/print.h"

/* hardware dependent: */
#define LED0 RB_SPI_SS3
#define LED1 RB_SPI_SS4
#define LED2 RB_SPI_SS0
#define LED3 RB_SPI_SS1
#define LED4 RB_SPI_SS2
#define LED5 RB_SPI_SS5

/* ADXL345 addresses */
#define ADXL345_I2C_WRITE		0xA6
#define ADXL345_I2C_READ		0xA7

/* ADXL345 registers */
#define ADXL345_I2C_R_DEVID		0x0
#define ADXL345_I2C_R_BW_RATE		0x2C
#define ADXL345_I2C_R_POWER		0x2D
#define ADXL345_I2C_R_INT_ENABLE	0x2E
#define ADXL345_I2C_R_INT_MAP		0x2F
#define ADXL345_I2C_R_INT_SOURCE	0x30
#define ADXL345_I2C_R_DATA_FORMAT	0x31
#define ADXL345_I2C_R_DATAX0		0x32
#define ADXL345_I2C_R_DATAX1		0x33
#define ADXL345_I2C_R_DATAY0		0x34
#define ADXL345_I2C_R_DATAY1		0x35
#define ADXL345_I2C_R_DATAZ0		0x36
#define ADXL345_I2C_R_DATAZ1		0x37

/* ADXL345 constants */
#define ADXL345_DEVID			0xE5 /* octal 0345 */

/* I2C communication with ADXL345 */
uint32_t adxl345SetByte(uint8_t cr, uint8_t value) {
	I2CMasterBuffer[0] = ADXL345_I2C_WRITE;
	I2CMasterBuffer[1] = cr;
	I2CMasterBuffer[2] = value;
	I2CWriteLength = 3;
	I2CReadLength = 0;
	return i2cEngine();
}

uint32_t adxl345GetBytes(uint8_t cr, uint8_t count) {
	I2CMasterBuffer[0] = ADXL345_I2C_WRITE;
	I2CMasterBuffer[1] = cr;
	I2CWriteLength = 2;
	I2CReadLength = 0;
	i2cEngine();

	I2CMasterBuffer[0] = ADXL345_I2C_READ;
	I2CWriteLength = 1;
	I2CReadLength = count;

	return i2cEngine();
}

uint32_t adxl345GetByte(uint8_t cr) {
	return adxl345GetBytes(cr, 1);
}

/* our globals: */

uint32_t interrupts = 0;
uint32_t ticks = 0;

/* defined as weak in basic/basic.c, we'll override it here */
void businterrupt(void) {
//	interrupts++;
}

#define INVERT 0

void timerhandler(void){
	TMR_TMR32B0IR = TMR_TMR32B0IR_MR0;
	ticks++;
}

#define PIXELS_PER_ROTATION 200
void led_loop(uint8_t show[], uint16_t show_len) {
	int32_t avg = 0;
	int32_t oldavg = 0;
	int32_t diffavg = 0;
	int32_t olddiffavg = 0;
	uint32_t oldticks = 0;
	uint32_t ticksteps = 2000; // 2 msecs
	int16_t offset = 0;

	// we don't need no backlight when displaying via LEDs
	backlightSetBrightness(0);

	offset = 32 - (show_len / 2);
	if(offset < 0) offset = 0;

	timer32Callback0 = timerhandler;
	timer32Init(0, (CFG_CPU_CCLK / 1000000) * ticksteps);
	timer32Enable(0);

	adxl345SetByte(ADXL345_I2C_R_POWER, 1<<3); // start measuring

	// loop untin ENTER is pressed:
	while(gpioGetValue(RB_BTN4) != 0) {
		if(oldticks != ticks) {
			oldticks = ticks;
			if(ticks >= offset && ticks < (show_len + offset)) {
				gpioSetValue(LED5, (show[ticks - offset] & 0x01) ? 1 - INVERT : 0 + INVERT);
				gpioSetValue(LED4, (show[ticks - offset] & 0x02) ? 1 - INVERT : 0 + INVERT);
				gpioSetValue(LED3, (show[ticks - offset] & 0x04) ? 1 - INVERT : 0 + INVERT);
				gpioSetValue(LED2, (show[ticks - offset] & 0x08) ? 1 - INVERT : 0 + INVERT);
				gpioSetValue(LED1, (show[ticks - offset] & 0x10) ? 1 - INVERT : 0 + INVERT);
				gpioSetValue(LED0, (show[ticks - offset] & 0x20) ? 1 - INVERT : 0 + INVERT);
			}

			/* we need to read from INT_SOURCE to reset interrupts on ADXL345's side */
			adxl345GetBytes(ADXL345_I2C_R_INT_SOURCE, 8);

			oldavg = avg;
			olddiffavg = diffavg;

			avg = (avg * 63 + (*((int16_t*) &I2CSlaveBuffer[2]) * 128)) / 64;
			diffavg = (diffavg * 31 + ((avg - oldavg) * 128)) / 32;
			
			if(ticks > 80 && olddiffavg < 0 && diffavg >= 0) {
				// from negative to positive difference: we're at the bottom
				timer32Disable(0);
				ticksteps = (ticksteps + 3*((ticks * ticksteps) / PIXELS_PER_ROTATION)) / 4;
				if(ticksteps < 1000) ticksteps = 1000;
				timer32Init(0, (CFG_CPU_CCLK / 1000000) * ticksteps);
				ticks = 0;
				timer32Enable(0);
			}
		}
	}

	timer32Disable(0);

	adxl345SetByte(ADXL345_I2C_R_POWER, 0); // stop

	backlightSetBrightness(GLOBAL(lcdbacklight));
}

void main_adxlupndown(void)
{
	uint8_t show[] = {
		0x00, 0x3f, 0x08, 0x08, 0x3f, 0x00, 0x00, 0x21, 0x3f, 0x21,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x3f, 0x20,
		0x00, 0x3f, 0x08, 0x08, 0x3f, 0x00, 0x3f, 0x29, 0x29, 0x21,
		0x00, 0x3f, 0x24, 0x26, 0x19, 0x00, 0x3f, 0x29, 0x29, 0x21,
		0x00, 0x00, 0x00, 0x3d, 0x00};
#define SHOW_LEN 45

	/* INIT */

	/* switch off LEDs */
	gpioSetValue(LED0, 0);
	gpioSetValue(LED1, 0);
	gpioSetValue(LED2, 0);
	gpioSetValue(LED3, 0);
	gpioSetValue(LED4, 0);
	gpioSetValue(LED5, 0);

	/* I2C communications for ADXL345: */
	i2cInit(I2CMASTER);

	/* check presence: */
	adxl345GetByte(ADXL345_I2C_R_DEVID);
	if(I2CSlaveBuffer[0] != ADXL345_DEVID) {
		lcdPrintln("ERROR:");
		lcdPrintln("==========");
		lcdPrintln("can't find");
		lcdPrintln("ADXL345");
		DoInt(64,0,I2CSlaveBuffer[0]);
		lcdRefresh();
		while(1) {
			// endless loop
		}
	}
	/* configure: */
	adxl345SetByte(ADXL345_I2C_R_DATA_FORMAT, 1<<5 | 1<<3 | 1<<1 | 1<<0); // full res at +-16g, interrupts are active low
	adxl345SetByte(ADXL345_I2C_R_BW_RATE, 0x0D); // 800 Hz sampling rate

	/*
	// prepare BUSINT interrupt
	gpioSetDir(RB_BUSINT, gpioDirection_Input);
	gpioSetPullup (&RB_BUSINT_IO, gpioPullupMode_PullUp);
	gpioSetInterrupt(RB_BUSINT, gpioInterruptSense_Edge, gpioInterruptEdge_Single, gpioInterruptEvent_ActiveLow);
	gpioIntEnable(RB_BUSINT);
	adxl345SetByte(ADXL345_I2C_R_INT_ENABLE, 1<<7); // enable data ready interrupt
	*/

	/* MAIN LOOP: */

	while(1) {
		lcdClear();
		DoInt(0,0,interrupts);
		DoInt(0,8,ticks);
		lcdRefresh();
		while(gpioGetValue(RB_BTN0) != 0) {
			/* wait for left button */
		}
		led_loop(show, SHOW_LEN);
	}
}
