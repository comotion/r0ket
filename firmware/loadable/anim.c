#include <sysinit.h>
#include <string.h>

#include "basic/basic.h"

#include "lcd/lcd.h"
#include "lcd/display.h"

#include "filesystem/ff.h"
#include "filesystem/diskio.h"

#include "minilzo/minilzo.h"
#include "minilzo/minilzo.c"

/**************************************************************************/

//#define SIMULAT0R

#ifdef SIMULAT0R
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#endif

uint8_t lcdShowLZOAnim(char *fname, uint32_t framems) {
	unsigned char img[864];
	UINT readbytes;
	FRESULT ret;
	FIL file;
	uint16_t sz;
	lzo_uint len;
	uint8_t state=0;

	ret=f_open(&file, fname, FA_OPEN_EXISTING|FA_READ);
	if(ret)
		return 1;

	ret = lzo_init();
	if(ret)
		return 2;

	getInputWaitRelease();

	while(!getInputRaw()) {
		//lcdFill(0x55);

		ret = f_read(&file, &sz, 2, &readbytes);

		if(sz == 0) {
			// uncompressed
			ret = f_read(&file, (unsigned char *)lcdBuffer, 864, &readbytes);
			if(ret || readbytes != 864) {
				f_lseek(&file, 0);
				continue;
			}
		} else {
			ret = f_read(&file, img, sz, &readbytes);
			if(ret || readbytes != sz) {
				f_lseek(&file, 0);
				continue;
			}

			lzo1x_decompress(img, sz, (unsigned char *)lcdBuffer, &len, NULL);
		}

		lcdDisplay();
#ifndef SIMULAT0R
		if(framems<100){
#endif
			state=delayms_queue_plus(framems,0);
#ifdef SIMULAT0R
			usleep(framems*1000);
#endif
#ifndef SIMULAT0R
		}else{
			getInputWaitTimeout(framems);
		}
#endif
	}

	if(state)
		work_queue();

	f_close(&file);
	return 0;
}

int lcdLoadLZOImage(char *fname) {
	unsigned char img[864];
	UINT readbytes;
	FRESULT ret;
	FIL file;
	uint16_t sz;
	lzo_uint len;

	ret=f_open(&file, fname, FA_OPEN_EXISTING|FA_READ);
	if(ret)
		return 1;

	ret = lzo_init();
	if(ret)
		return 2;

	ret = f_read(&file, &sz, 2, &readbytes);

	if(ret || readbytes != 2) {
		return 3;
	}

	if(sz == 0) {
		// uncompressed
		ret = f_read(&file, (unsigned char *)lcdBuffer, 864, &readbytes);
		if(ret)
			return 4;
	} else {
		ret = f_read(&file, img, sz, &readbytes);

		if(ret || readbytes != sz) {
			return 5;
		}

		lzo1x_decompress(img, sz, (unsigned char *)lcdBuffer, &len, NULL);
	}

	f_close(&file);

	return 0;
}

void ram(void) {
	FRESULT ret;
	FATFS FatFs[1];          /* File system object for logical drive */

	//gpioSetValue (RB_LED1, CFG_LED_OFF); 
	//backlightInit();
	//
	lcdFill(0);

	ret=f_mount(0, &FatFs[0]);
	
	if(ret) {
		return;
	}

//	lcdLoadLZOImage("TESTLZO.ILZ");
//	lcdDisplay();
	ret = lcdShowLZOAnim("LZOANIM.VLZ", 80);
#ifdef SIMULAT0R
	fprintf(stderr, "ret=%d\n", ret);
#endif
	while(!getInputRaw()){
		/* nothing Yet */
            getInputWaitTimeout(1000);
	}
	return;
}
