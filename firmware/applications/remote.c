#include <sysinit.h>
#include <string.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "funk/nrf24l01p.h"
#include "remote.h"

char send_remote(struct remote_t *rcdata)
{
	uint8_t buf[32];

	memcpy(buf, rcdata, sizeof(*rcdata));
	return nrf_snd_pkt_crc(32, buf);
}

void main_remote(void) {
    char key;
    char oldkey = BTN_NONE;
    struct remote_t rcdata = {
	    .valid = RC_ALL,
	    .x = 0,
	    .y = 0,
	    .z = 0
    };

    nrf_init();
    struct NRF_CFG config = {
     .channel= 82,
     .txmac= "\x1\x2\x3\x2\x1",
     .nrmacs=1,
     .mac0=  "\x1\x2\x3\x2\x1",
     .maclen ="\x20",
    };
    nrf_config_set(&config);
    while (1) {
        lcdClear();
        delayms(20);
        key = getInputRaw();
        if(key & BTN_LEFT){
            lcdPrintln("LEFT");
		rcdata.x--;
        }
        if(key & BTN_UP){
            lcdPrintln("UP");
		rcdata.y++;
        }
        if(key & BTN_DOWN){
            lcdPrintln("DOWN");
		rcdata.y--;
        }
        if(key & BTN_RIGHT){
            lcdPrintln("RIGHT");
		rcdata.x++;
        }
        if(key & BTN_ENTER){
            lcdPrintln("ENTER");
		rcdata.z = 1;
		rcdata.x = 0;
		rcdata.y = 0;
        }
	if (key ==BTN_NONE){
	    rcdata.z = 0;
	    lcdPrintln("NONE");
/*		if(speed<150){
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
*/	}

        if((oldkey != key) && (oldkey != BTN_NONE) && (oldkey != BTN_ENTER)) {
               gpioSetValue(RB_LED1, !gpioGetValue(RB_LED1));
	}
	lcdPrint("X ");
	lcdPrintInt(rcdata.x);
	lcdNl();
	lcdPrint("Y ");
	lcdPrintInt(rcdata.y);
	lcdNl();
	lcdPrint("Z ");
	lcdPrintInt(rcdata.z);
        lcdDisplay();
        oldkey = key;

	if (rcdata.x < RC_X_MIN)
		rcdata.x = RC_X_MIN;
	if (rcdata.x > RC_X_MAX)
		rcdata.x = RC_X_MAX;
	if (rcdata.y < RC_Y_MIN)
		rcdata.y = RC_Y_MIN;
	if (rcdata.y > RC_Y_MAX)
		rcdata.y = RC_Y_MAX;
	if (rcdata.z < RC_Z_MIN)
		rcdata.z = RC_Z_MIN;
	if (rcdata.z > RC_X_MAX)
		rcdata.z = RC_Z_MAX;

	send_remote(&rcdata);
    }
}

