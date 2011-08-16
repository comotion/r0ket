#include <sysinit.h>
#include <string.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "funk/nrf24l01p.h"
#include "remote.h"
#include "core/adc/adc.h"

char send_remote(struct remote_t *rcdata)
{
	uint8_t buf[32];

	memcpy(buf, rcdata, sizeof(*rcdata));
	return nrf_snd_pkt_crc(32, buf);
}

void main_acc_remote(void) {
    char key;
    char oldkey = BTN_NONE;
    uint32_t x=0,y=0;
    uint32_t ox=0,oy=0;
    uint32_t oox=0,ooy=0;
    int foox=0,fooy=0;
	adcInit();

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
    uint32_t xmax=0,xmin=-1,ymax=0,ymin=-1,xmean=0,ymean=0;

    for (int i=0; i<1000; i++){
	x=adcRead(3);
	y=adcRead(2);
    }
    key = getInputRaw();
    while(!(key & BTN_ENTER)){
        key = getInputRaw();
    }
    while(key & BTN_ENTER){
        key = getInputRaw();
    	x=adcRead(3);
	y=adcRead(2);
	if (x>xmax){
		xmax=x;
	}
	if(y>ymax){
		ymax=y;
	}
	if(y<ymin){
		ymin=y;
	}
	if(x<xmin){
		xmin=x;
	}
	xmean=(xmean+x)/2;
	ymean=(ymean+y)/2;
    }
   // xmax=510;
   // xmin=500;
   // ymax=610;
   // ymin=600;


    
    while (1) {
	lcdClear();
        delayms(20);
        key = getInputRaw();
        oox=ox;
        ooy=oy;
        ox=x;
        oy=y;
        x=adcRead(3);
        y=adcRead(2);
        x=(ox+x)/2;
        y=(oy+y)/2;
        x=(((ox-oox)+ox)+x)/2;
        y=(((oy-ooy)+oy)+y)/2;

        lcdClear();
	foox=0;
	fooy=0;
	if (x>xmean-10&& x<xmean+10){
            lcdPrintln("X");
		rcdata.x=0;		
		foox=1;
	}
	if(y>ymean-10 && y<ymean+10){
            lcdPrintln("Y");
		rcdata.y=0;
		fooy=1;
	}
	if(y>600|| y<400){
		rcdata.y=0;
		fooy=1;
	}	
	if (0==fooy){
		rcdata.y= (y-ymean)*(100/70);
	}
	if (0==foox) {
		rcdata.x= (x-xmean)*(300/70);
	}
        delayms(20);
        key = getInputRaw();
        if(key & BTN_ENTER){
            lcdPrintln("ENTER");
		rcdata.z = 1;
		rcdata.x = 0;
		rcdata.y = 0;
        }
	if (key ==BTN_NONE){
	    rcdata.z = 0;
	    lcdPrintln("NONE");
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

