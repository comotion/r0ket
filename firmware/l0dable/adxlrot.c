/*
 * adxlrot.c
 *
 * Rotational display m0dul
 *
 * copyright (c) 2011 by Hans-Werner Hilse <hilse@web.de>
 *
 * uses an LED driver on the SPI_SS pins and an ADXL345
 * accelerometer on the I2C bus pins on the hardware side.
 *
 * this l0dable will read animations from files and will
 * display them in an endless loop.
 *
 * the files look like this:
 *
 * rotfile = header + banner [ + banner [ + banner ... ] ]
 * header = number_of_banners(1 byte)
 * banner = show_for_rotations(1 byte) + banner_length(1 byte) + banner_data
 * banner_data = column1(1 byte) + column2(1 byte) + column3(1 byte) + ...
 */

#include <stdint.h>
#include <sysinit.h>
#include "basic/basic.h"
#include "basic/config.h"
#include "core/i2c/i2c.h"
#include "core/timer32/timer32.h"
#include "lcd/allfonts.h"
#include "lcd/backlight.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "filesystem/ff.h"

#include "usetable.h"

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

void adxl345SetByte(uint8_t cr, uint8_t value);
void adxl345GetBytes(uint8_t cr, uint8_t count);
void led_loop(char *filename);
void led_set(uint8_t display);

void ram(void)
{
	char filename[13];

	/* INIT */

	/* switch off LEDs */
	led_set(0);

	/* I2C communications for ADXL345: */
	i2cInit(I2CMASTER);

	/* check presence: */
	adxl345GetBytes(ADXL345_I2C_R_DEVID, 1);
	if(I2CSlaveBuffer[0] != ADXL345_DEVID) {
		lcdPrintln("ERROR:");
		lcdPrintln("==========");
		lcdPrintln("can't find");
		lcdPrintln("ADXL345");
		lcdRefresh();
		while(gpioGetValue(RB_BTN0) != 0) {
			/* wait for left button */
		}
		return;
	}

	/* configure: */
	adxl345SetByte(ADXL345_I2C_R_DATA_FORMAT, 1<<5 | 1<<3 | 1<<1 | 1<<0); // full res at +-16g, interrupts are active low
	adxl345SetByte(ADXL345_I2C_R_BW_RATE, 0x0D); // 800 Hz sampling rate

	if(selectFile(filename, "ROT") == 0){
		led_loop(filename);
	}
}

/* I2C communication with ADXL345 */
void adxl345SetByte(uint8_t cr, uint8_t value) {
	I2CMasterBuffer[0] = ADXL345_I2C_WRITE;
	I2CMasterBuffer[1] = cr;
	I2CMasterBuffer[2] = value;
	I2CWriteLength = 3;
	I2CReadLength = 0;
	i2cEngine();
}

void adxl345GetBytes(uint8_t cr, uint8_t count) {
	I2CMasterBuffer[0] = ADXL345_I2C_WRITE;
	I2CMasterBuffer[1] = cr;
	I2CWriteLength = 2;
	I2CReadLength = 0;
	i2cEngine();

	I2CMasterBuffer[0] = ADXL345_I2C_READ;
	I2CWriteLength = 1;
	I2CReadLength = count;
	i2cEngine();
}

void led_set(uint8_t display) {
	gpioSetValue(LED5, display & 0x01);
	gpioSetValue(LED4, (display >> 1) & 0x01);
	gpioSetValue(LED3, (display >> 2) & 0x01);
	gpioSetValue(LED2, (display >> 3) & 0x01);
	gpioSetValue(LED1, (display >> 4) & 0x01);
	gpioSetValue(LED0, (display >> 5) & 0x01);
}

#define PIXELS_PER_ROTATION 200
#define BUFSIZE 32
#define VALSTEP ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 800)
#define MINROT ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 15)

void led_loop(char *filename) {
	FIL file;
	int res;
	UINT readbytes;
	uint8_t banners = 0;
	uint8_t current_banner = 0;
	uint8_t current_banner_duration = 0;
	uint8_t show_len = 0;
	uint8_t value_ptr = 0; // shared current buffer index for circular buffers
	uint8_t i;
	uint8_t *show;

	uint32_t position_trigger = 1 << 31;
	uint32_t position_step = TIMER32_CCLK_1MS * 5; // 5ms
	uint32_t position_step_target = TIMER32_CCLK_1MS * 5; // 5ms
	uint32_t value_trigger = 0;
	uint32_t old_backlight_mr0;
	uint16_t position = 0;
	int16_t offset = 0;
	int16_t *acceleration_values; // will point to circular buffer
	int16_t *acceleration_difference_values; // will point to circular buffer

	int32_t acceleration = 0;
	int32_t acceleration_old = 0;
	int32_t acceleration_difference = 0;
	int32_t acceleration_difference_old = 0;
	int32_t timer_top_position = 0;

	res = f_open(&file, filename, FA_OPEN_EXISTING|FA_READ);
	if(res) return;

	// we use the lcdBuffer as value cache
	acceleration_values = (int16_t*) lcdBuffer;
	acceleration_difference_values = &acceleration_values[BUFSIZE];
	memset(acceleration_values, 0, sizeof(int16_t) * BUFSIZE);
	memset(acceleration_difference_values, 0, sizeof(int16_t) * BUFSIZE);
	show = (uint8_t*) &acceleration_difference_values[BUFSIZE];

	// we don't need no backlight when displaying via LEDs
	gpioSetDir(RB_PWR_LCDBL, gpioDirection_Input);
	old_backlight_mr0 = TMR_TMR16B1MR0;
	TMR_TMR16B1MR0 = 0xFFFF;

	// read number of banners
	res = f_read(&file, (char*)&banners, 1, &readbytes);
	if(res || banners == 0) goto quit;

	adxl345SetByte(ADXL345_I2C_R_POWER, 1<<3); // start measuring

	SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_CT32B0); // enable 32bit timer #0
	TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERRESET_ENABLED; // and reset it
	TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_ENABLED;

	// loop until LEFT is pressed or our timer reaches 2^30 ticks:
	while((gpioGetValue(RB_BTN0) != 0) && (TMR_TMR32B0TC < (1 << 30))) {
		// this will be a dynamic interval:
		if(TMR_TMR32B0TC > position_trigger) {
			// lazy step time correction:
			position_step = (position_step * 3 + position_step_target) / 4;
			// next trigger value
			position_trigger += position_step;

			// display:
			if(position >= offset && position < (show_len + offset)) led_set(show[position - offset]);
			position++;
		}
		// this is a static interval:
		if(TMR_TMR32B0TC > value_trigger) {
			value_trigger += VALSTEP;

			// buffer old values
			acceleration_old = acceleration;
			acceleration_difference_old = acceleration_difference;

			// get values
			adxl345GetBytes(ADXL345_I2C_R_DATAX0, 2);

			acceleration_values[value_ptr] = I2CSlaveBuffer[0] + (I2CSlaveBuffer[1] << 8);

			// average acceleration over BUFSIZE single values
			acceleration = 0;
			for(i=0; i<BUFSIZE; i++) acceleration += acceleration_values[i];
			acceleration /= BUFSIZE;

			// the same for the acceleration differences (with which we'll actually work)
			acceleration_difference_values[value_ptr] = acceleration - acceleration_old;
			acceleration_difference = 0;
			for(i=0; i<BUFSIZE; i++) acceleration_difference += acceleration_difference_values[i];
			acceleration_difference /= BUFSIZE;

			// increment buffer index
			value_ptr++;
			if(value_ptr == BUFSIZE) value_ptr = 0;

			// check values for rotation events:

			if(acceleration_difference_old < 0 && acceleration_difference >= 0) {
				// from negative to positive difference: we're at the bottom of the rotation
				if(TMR_TMR32B0TC > MINROT) {
					position_step_target = TMR_TMR32B0TC / PIXELS_PER_ROTATION;
					timer_top_position = TMR_TMR32B0TC - timer_top_position;

					// take averaging time into account and re-calculate offset:
					offset = 64 - ((BUFSIZE * VALSTEP) / position_step_target);
					offset = offset + 40 - (show_len / 2);
					if(offset < 0) offset = 0;

					// reset triggers and position counter
					position = 0;
					position_trigger = position_step_target;
					value_trigger = VALSTEP;

					if(current_banner_duration == 0) {
						if(current_banner == banners) {
							current_banner = 0;
							f_lseek(&file, 1);
						}
						current_banner++;
						res = f_read(&file, (char*) &current_banner_duration, 1, &readbytes);
						if(res) goto quit;
						res = f_read(&file, (char*) &show_len, 1, &readbytes);
						if(res) goto quit;
						res = f_read(&file, (char*) show, show_len, &readbytes);
						if(res) goto quit;
					}
					current_banner_duration--;

					// reset timer
					TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERRESET_ENABLED;
					TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_ENABLED;
				} else {
					position_trigger = 1 << 31; // effectively disable display (trigger value won't be reached)
				}
			}

			if(acceleration_difference_old > 0 && acceleration_difference <= 0) {
				// at the top of the rotation
				if(TMR_TMR32B0TC + timer_top_position > MINROT) {
					position_step_target = (TMR_TMR32B0TC + timer_top_position) / PIXELS_PER_ROTATION;
				}
				timer_top_position = TMR_TMR32B0TC;
			}
		}
	}

quit:

	f_close(&file);
	TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_DISABLED; // disable timer32 #0

	adxl345SetByte(ADXL345_I2C_R_POWER, 0); // stop

	lcdClear(); // clear lcd buffer, we used it to store data

	// activate backlight:
	gpioSetDir(RB_PWR_LCDBL, gpioDirection_Output);
	gpioSetValue(RB_PWR_LCDBL, 0);
	TMR_TMR16B1MR0 = old_backlight_mr0;

	// just to be sure, reset LEDs
	led_set(0);
}
