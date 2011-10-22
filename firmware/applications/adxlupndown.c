#include <stdint.h>
#include <string.h>

#include "lpc134x.h"
#include "usb/usb.h"
#include "usbdesc.h"
#include "rom_drivers.h"


#include <sysinit.h>
#include "basic/basic.h"
#include "core/i2c/i2c.h"
#include "systick/systick.h"
#include "lcd/backlight.h"

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

/* defined as weak in basic/basic.c, we'll override it here */
void businterrupt(void) {
	interrupts++;
}

void main_adxlupndown(void)
{
	uint16_t cur = 0;
	uint16_t duration = 0;
	uint16_t pxstep = 0;
	int32_t avg = 0;
	int32_t oldavg = 0;
	int32_t diffavg = 0;
	int32_t olddiffavg = 0;
	uint32_t oldsystick = 0;
	uint32_t lasttrans = 0;

	uint8_t show[] = {
		0x00, 0x3f, 0x08, 0x08, 0x3f, 0x00, 0x00, 0x21, 0x3f, 0x21,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x3f, 0x20,
		0x00, 0x3f, 0x08, 0x08, 0x3f, 0x00, 0x3f, 0x29, 0x29, 0x21,
		0x00, 0x3f, 0x24, 0x26, 0x19, 0x00, 0x3f, 0x29, 0x29, 0x21,
		0x00, 0x00, 0x00, 0x3d, 0x00};
#define SHOW_LEN 45
#define PIXSTEP 2
#define INVERT 0

	/* INIT */

	backlightSetBrightness(0);
	i2cInit(I2CMASTER); // Init I2C
	adxl345SetByte(ADXL345_I2C_R_DATA_FORMAT, 1<<5 | 1<<3 | 1<<1 | 1<<0); // full res at +-16g, interrupts are active low
	adxl345SetByte(ADXL345_I2C_R_INT_ENABLE, 1<<7); // enable data ready interrupt
	adxl345SetByte(ADXL345_I2C_R_BW_RATE, 0x0D); // 800 Hz sampling rate
	adxl345SetByte(ADXL345_I2C_R_POWER, 1<<3); // start measuring
	systickInit(1); // 1000 Hz systicks

	// prepare BUSINT interrupt
	gpioSetDir(RB_BUSINT, gpioDirection_Input);
	gpioSetPullup (&RB_BUSINT_IO, gpioPullupMode_PullUp);
	gpioSetInterrupt(RB_BUSINT, gpioInterruptSense_Edge, gpioInterruptEdge_Single, gpioInterruptEvent_ActiveLow);
	gpioIntEnable(RB_BUSINT);

	/* switch off LEDs */
	gpioSetValue(LED0, 0);
	gpioSetValue(LED1, 0);
	gpioSetValue(LED2, 0);
	gpioSetValue(LED3, 0);
	gpioSetValue(LED4, 0);
	gpioSetValue(LED5, 0);

	while(1){
		oldavg = avg;
		olddiffavg = diffavg;
		oldsystick = systickGetTicks();

		/* we need to read from INT_SOURCE to reset interrupts on ADXL345's side */
		adxl345GetBytes(ADXL345_I2C_R_INT_SOURCE, 8);

		avg *= 127;
		avg += *((int16_t*) &I2CSlaveBuffer[2]) * 128;
		avg /= 128;

		diffavg *= 63;
		diffavg += (avg - oldavg) * 128;
		diffavg /= 64;
		
		if(olddiffavg < 0 && diffavg >= 0) {
			// from negative to positive difference: we're at the bottom
			if(oldsystick > lasttrans) {
				duration = oldsystick - lasttrans;
			}
			lasttrans = oldsystick;
			if(duration > 200) {
				cur=SHOW_LEN;
				pxstep=PIXSTEP;
			}
		}
		if(pxstep == PIXSTEP && cur > 0) {
			uint8_t px = show[SHOW_LEN - cur];
			gpioSetValue(LED5, (px & 0x01) ? 1 - INVERT : 0 + INVERT);
			gpioSetValue(LED4, (px & 0x02) ? 1 - INVERT : 0 + INVERT);
			gpioSetValue(LED3, (px & 0x04) ? 1 - INVERT : 0 + INVERT);
			gpioSetValue(LED2, (px & 0x08) ? 1 - INVERT : 0 + INVERT);
			gpioSetValue(LED1, (px & 0x10) ? 1 - INVERT : 0 + INVERT);
			gpioSetValue(LED0, (px & 0x20) ? 1 - INVERT : 0 + INVERT);
			cur--;
		}
		pxstep--;
		if(pxstep == 0) pxstep=PIXSTEP;
		while(systickGetTicks() <= (oldsystick + 1)) {}
	}
}
