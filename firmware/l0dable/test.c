#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "usb/usbmsc.h"

#include "lcd/print.h"

#include "funk/nrf24l01p.h"
#include "funk/mesh.h"


#include "usetable.h"

/*Global Communication Config*/
uint8_t mac[5] = {1,2,3,5,5};
struct NRF_CFG config = 
{
   .channel= 81,
   .txmac= "\x1\x2\x3\x5\x5",
   .nrmacs=1,
   .mac0=  "\x1\x2\x3\x5\x5",
   .maclen ="\x20",
};



void ram(void){
	int mv_s=0, mv_e=0;
	unsigned char buf_rx[32];
	char buf_tx[32];
	int tries=0;
	int key;
	int n,i;
	int chrg = 0;
	int funk=0;
	
	// get voltage
	mv_s = GetVoltage();
	
	int jogdial=0;

	// DISPLAY text 
	lcdClear();
	lcdPrintln("testing...");
	lcdRefresh();
	
	// LEDs
	IOCON_PIO1_11 = 0x0;
	gpioSetDir(RB_LED3, gpioDirection_Output);
	for (int x=0;x<30;x++){
		gpioSetValue (RB_LED0, x%2);
		gpioSetValue (RB_LED1, x%2);
		gpioSetValue (RB_LED2, x%2);		
		gpioSetValue (RB_LED3, x%2);
		delayms(100);
	};
	
	// until all buttons have been pushed
	while(jogdial < 31)
	{
		// turn off all
		gpioSetValue (RB_LED0, 0);
		gpioSetValue (RB_LED1, 0);
		gpioSetValue (RB_LED2, 0);
		gpioSetValue (RB_LED3, 0);
		
		switch(getInputRaw()) {
	        case BTN_ENTER:
	        	jogdial |= (1 << 0);
	        	gpioSetValue (RB_LED0, 1);
			gpioSetValue (RB_LED1, 1);
			gpioSetValue (RB_LED2, 1);
			gpioSetValue (RB_LED3, 1);
	        	break;
	        case BTN_UP: // Reset config
	        	jogdial |= (1 << 1);
	        	gpioSetValue (RB_LED1, 1);
	        	break;
	        case BTN_DOWN:
	        	jogdial |= (1 << 2);
	        	gpioSetValue (RB_LED2, 1);
	        	break;
	        case BTN_LEFT:
	        	jogdial |= (1 << 3);
	        	gpioSetValue (RB_LED0, 1);
	        	break;
	        case BTN_RIGHT:
	        	jogdial |= (1 << 4);
	        	gpioSetValue (RB_LED3, 1);
	        	break;
    		};
    		
    		lcdClear();
		lcdPrintln(IntToStr(jogdial,10,0));
		lcdRefresh();
    		
    		delayms(50);
    	}
    	
    	// turn off all
	gpioSetValue (RB_LED0, 0);
	gpioSetValue (RB_LED1, 0);
	gpioSetValue (RB_LED2, 0);
	gpioSetValue (RB_LED3, 0);
	
	// set LED3 back to light sensor mode
	gpioSetDir(RB_LED3, gpioDirection_Input);
	IOCON_PIO1_11 = 0x41;
	
	getInputWait();
	getInputWaitRelease();
	
	// DISPLAY 
	lcdClear();
	lcdLoadImage("white.lcd");
	lcdRefresh();
	getInputWait();
	getInputWaitRelease();
	
	lcdClear();
	lcdLoadImage("black.lcd");
	lcdRefresh();
	getInputWait();
	getInputWaitRelease();
	
	// BATTERY
	key = 0;
	while (tries < 500 && key != BTN_DOWN) {
		chrg = !gpioGetValue(RB_PWR_CHRG);
		key = getInput();
		// get voltage again
		mv_e = GetVoltage();
		lcdClear();
		lcdPrintln("start volt:");
		lcdPrintln(IntToStr(mv_s,10,0));
		lcdPrintln("end volt:");
		lcdPrintln(IntToStr(mv_e,10,0));
		lcdPrintln("charging:");
		lcdPrintln(IntToStr(chrg,10,0));
		if(chrg) {
			gpioSetValue (RB_LED1, 1);
		}
		else {
			gpioSetValue (RB_LED0, 1);
		}
		lcdRefresh();
		getInputWaitTimeout(100);
		gpioSetValue (RB_LED0, 0);
		gpioSetValue (RB_LED1, 0);
	}	
	
	// FUNK
	key = 0;
	// wait for ping from master
	// interrupt and continue with button down
	nrf_config_set(&config);
	while (tries < 500 && key != BTN_DOWN) {
		gpioSetValue (RB_LED1, 1);
		key = getInput();	
		n = nrf_rcv_pkt_time(100,32,buf_rx);
		getInputWaitTimeout(100);
		gpioSetValue (RB_LED1, 0);
		
		
		lcdClear();		
		// full package received
		if(n == 32) {
			lcdPrintln("received!");
			lcdPrint("try: ");
			lcdPrintln(IntToStr(tries,4,0));
			lcdPrint("msg: ");
			lcdPrintln((char *)buf_rx);
			if(buf_rx[0] == '4' && buf_rx[1]=='2') {
				// package ok
				tries = 500;
				funk = 1;
				lcdPrintln("paket ok");
			}
		}
		else {
			lcdPrintln("listening...");
			lcdPrint("try: ");
			lcdPrintln(IntToStr(tries,4,0));		
		}
		lcdRefresh();
		delayms(30);
		
		tries++;	
	}
	
	getInputWait();
	getInputWaitRelease();
	
	
	// tests done
	lcdClear();
	lcdPrintln("test done");
	gpioSetValue (RB_LED0, 1);	
	
	getInputWait();
	getInputWaitRelease();
	
	lcdPrintln("sending");
	lcdPrintln("like a bau5");
	lcdRefresh();
	
	// start sending like a boss
	// setup packet
	memset(buf_tx,0,32);
	buf_tx[0]='4';
	buf_tx[1]='2';

	getInputWait();
	getInputWaitRelease();
	
	tries = 0;
	key = 0;
	while(key != BTN_DOWN) {
		gpioSetValue (RB_LED1, 1);
		key = getInput();
		nrf_snd_pkt_crc(32,(uint8_t *)buf_tx);
		lcdClear();
		lcdPrintln("sending ping");
		lcdRefresh();
		delayms(100);
		gpioSetValue (RB_LED1, 0);
		delayms(50);
	}
	
	lcdClear();
	lcdPrintln("EOP.");
	lcdRefresh();

};

/* to test:
- leds an/aus
- lichtsensor
- display on all lines
- 5 way durchklicken
- battery: charging (gpioGetValue(RB_PWR_CHRG)) und voltage ende-anfang testen? POWER_GOOD
- dd check if flashing works (in flasher)
- funk
*/