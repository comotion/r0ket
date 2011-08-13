#include <sysinit.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "funk/nrf24l01p.h"
#include "filesystem/diskio.h"
#include "core/timer32/timer32.h"
//#define SERV RB_LED2
//#define MOT RB_LED1
#define SERV RB_HB4
#define MOT RB_HB3
uint16_t max=2000;
uint16_t duty1=150,duty0=150,val=0;

void TIMER32_0_IRQHandler(void){
	TMR_TMR32B0IR = TMR_TMR32B0IR_MR0;
	if (val==duty0){
		gpioSetValue(SERV,0);
	}
	if (val== duty1){
		gpioSetValue(MOT,0);
	}
	val++;
	if (val==max+1){
		val=0;
		gpioSetValue(SERV,1);
		gpioSetValue(MOT,1);
	}
};
		




void main_car(void) {
	timer32Init(0, (72E6/100E3));
	timer32Enable(0);
	gpioSetDir(SERV, gpioDirection_Output);
	gpioSetDir(MOT, gpioDirection_Output);
    DoString(10,20,"Hello World");
    lcdDisplay();
    int i=0;	
    nrf_init();
	struct NRF_CFG config = {
        .channel= 82,
        .txmac= "\x1\x2\x3\x2\x1",
        .nrmacs=1,
        .mac0=  "\x1\x2\x3\x2\x1",
        .maclen ="\x20",
	};
	nrf_config_set(&config);
	uint8_t buf[32];
    while (1) {
    //delayms(1000);
	//if(i){
	//	buf[0]="D";
	//	nrf_snd_pkt_crc(32, buf);
	//	lcdClear();
    	//	DoString(10,20,"Hello World");
	//	lcdDisplay();
	//	i=0;
	//	buf[0]="H";
	//}else{
		uint8_t n;
		n = nrf_rcv_pkt_time(300, 32, buf);
		lcdClear();
		i=1;
		if (n>0){
			if(buf[0]==1){
				duty0=buf[2];
				lcdPrintInt(buf[2]);
			}
			lcdNl();
			if(buf[1]==1){
				duty1=buf[3];
				lcdPrintInt(buf[3]);
			}
			lcdDisplay();
		}
		if (n==0 || n== -2){
			duty0=150;
			duty1=150;
			lcdPrintln("TIMEOUT!!");
			lcdPrintInt(duty0);
			lcdNl();
			lcdPrintInt(duty1);
			lcdDisplay();
		}


	//}	
		
	
    }
}
