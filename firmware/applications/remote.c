#include <sysinit.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "funk/nrf24l01p.h"

void main_remote(void) {
    char key;
    char oldkey = BTN_NONE;
    uint8_t buf[32];
    nrf_init();
	nrf_set_strength(10);
    struct NRF_CFG config = {
     .channel= 82,
     .txmac= "\x1\x2\x3\x2\x1",
     .nrmacs=1,
     .mac0=  "\x1\x2\x3\x2\x1",
     .maclen ="\x20",
    };
    nrf_config_set(&config);
    buf[0] = 1;
    buf[1] = 1;
    buf[2] = 150;
    buf[3] = 150;
	uint8_t speed=150, dir=150;
    while (1) {
        lcdClear();
        delayms(20);
        key = getInputRaw();
        if(key & BTN_LEFT){
            DoString(0,8,"LEFT");
		dir--;
        }
        if(key & BTN_UP){
            DoString(0,18,"UP");
		speed++;
        }
        if(key & BTN_DOWN){
            DoString(0,28,"DOWN");
		speed--;
        }
        if(key & BTN_RIGHT){
            DoString(0,38,"RIGHT");
		dir++;
        }
        if(key & BTN_ENTER){
            DoString(0,48,"ENTER");
		speed=150;
		dir=150;
        }
/*	if (key ==BTN_NONE){
		if(speed<150){
			speed++;
		}
		if(speed>150){
			speed--;
		}
		if(dir<150){
			dir++;
		}
		if(dir>150){
			dir--;
		}
	}
*/		
        if((oldkey != key) && (oldkey != BTN_NONE) && (oldkey != BTN_ENTER)) {
               gpioSetValue(RB_LED1, !gpioGetValue(RB_LED1));
		
        }
	lcdPrintInt(dir);
	lcdNl();
	lcdPrintInt(speed);
        lcdDisplay();
        oldkey = key;
	#define MAX 220
	#define MIN 100
	if (dir>MAX){
		dir=MAX;
	}
	if(dir<MIN){
		dir=MIN;
	}
	
	if(speed>MAX){
		speed=MAX;
	}
	if(speed<MIN){
		speed=MIN;
	}
	buf[2]=speed;
	buf[3]=dir;
        nrf_snd_pkt_crc(32, buf);
    }
}

