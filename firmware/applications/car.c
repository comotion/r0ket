#include <sysinit.h>
#include <string.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "funk/nrf24l01p.h"
#include "filesystem/diskio.h"
#include "core/timer32/timer32.h"
#include "remote.h"

//#define SERV RB_LED2
//#define MOT RB_LED1
#define GP_STEERING RB_HB5
#define GP_MOT1		RB_HB1
#define GP_MOT2		RB_HB4
#define GP_MOT3		RB_HB3
#define GP_MOT4		RB_HB0

#define X_MAX 200
#define X_MIN 100
#define Y_MAX 200
#define Y_MIN 100

enum motor_mode {
	MOT_FWD,
	MOT_REV,
	MOT_COAST,
	MOT_STOP
};

uint16_t max=2000;

uint32_t dutyx, dutyy, val=0;
struct remote_t rcdata;

inline void setSteering(uint8_t on)
{
	gpioSetValue(GP_STEERING, on);
}

inline void setMotor(enum motor_mode mode)
{
	switch(mode) {
	case MOT_FWD:
		gpioSetValue(GP_MOT1, 1);
		gpioSetValue(GP_MOT2, 0);
		gpioSetValue(GP_MOT3, 0);
		gpioSetValue(GP_MOT4, 1);
		break;
	case MOT_REV:
		gpioSetValue(GP_MOT1, 0);
		gpioSetValue(GP_MOT2, 1);
		gpioSetValue(GP_MOT3, 1);
		gpioSetValue(GP_MOT4, 0);
		break;
	case MOT_COAST:
		gpioSetValue(GP_MOT1, 0);
		gpioSetValue(GP_MOT2, 0);
		gpioSetValue(GP_MOT3, 0);
		gpioSetValue(GP_MOT4, 0);
		break;
	case MOT_STOP:
		gpioSetValue(GP_MOT1, 1);
		gpioSetValue(GP_MOT2, 0);
		gpioSetValue(GP_MOT3, 1);
		gpioSetValue(GP_MOT4, 0);
		break;
	}
}


void TIMER32_0_callb(void){

	TMR_TMR32B0IR = TMR_TMR32B0IR_MR0;

	if (val==max+1){
		val=0;
		setSteering(1);
		gpioSetValue(GP_MOT1, 1);
//		if (rcdata.y > 0)
//			setMotor(MOT_FWD);
//		else if (rcdata.y < 0)
//			setMotor(MOT_REV);
//		else
//			setMotor(MOT_COAST);
	}

	if (val==dutyx){
		setSteering(0);
	}
	if (val==dutyy) {
		gpioSetValue(GP_MOT1, 0);
	}
//	if (rcdata.z == 1) {
//		setMotor(MOT_STOP);
//	} else if (val == dutyy) {
//		setMotor(MOT_COAST);
//	}

	val++;
};

void main_car(void) {

	RB_HB0_IO = 0x81;
	RB_HB1_IO = 0x01;
	RB_HB5_IO = 0x81;

	gpioSetValue(GP_MOT1, 0);
	gpioSetValue(GP_MOT2, 0);
	gpioSetValue(GP_MOT3, 0);
	gpioSetValue(GP_MOT4, 0);
	gpioSetValue(GP_STEERING, 0);
	gpioSetDir(GP_STEERING, gpioDirection_Output);
	gpioSetDir(GP_MOT1, gpioDirection_Output);
	gpioSetDir(GP_MOT2, gpioDirection_Output);
	gpioSetDir(GP_MOT3, gpioDirection_Output);
	gpioSetDir(GP_MOT4, gpioDirection_Output);

	timer32Callback0= TIMER32_0_callb;

	timer32Init(0, (72E6/100E3));
	timer32Enable(0);

    DoString(10,20,"Hello World");
    lcdDisplay();
    int i=0;
    int invalx = 0, invaly = 0;
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
		memcpy(&rcdata, buf, sizeof(rcdata));
		lcdClear();
		i=1;
//		if (n>0){
//			if(buf[0]==1){
//				duty0=buf[2];
//				lcdPrintInt(buf[2]);
//			}
//			lcdNl();
//			if(buf[1]==1){
//				duty1=buf[3];
//				lcdPrintInt(buf[3]);
//			}
//			lcdDisplay();
//		}

		if (!(rcdata.valid & RC_X)) {
			invalx++;
			if (invalx > 5)
				dutyx = (X_MAX + X_MIN) / 2;
		} else {
			invalx = 0;
		if (rcdata.x >= 0)
			dutyx = (X_MIN + X_MAX)/2 + rcdata.x * ((X_MAX - X_MIN)/2) / RC_X_MAX;
		else if (rcdata.x < 0)
			dutyx = (X_MIN + X_MAX)/2 - rcdata.x * ((X_MAX - X_MIN)/2) / RC_X_MIN;
		}

		if (!(rcdata.valid & RC_Y)) {
			invaly++;
			if (invaly > 5)
				dutyy = (Y_MAX + Y_MIN) / 2;
//			dutyy = 0;
		} else {
			invaly = 0;
		if (rcdata.y >= 0)
			dutyy = (Y_MIN + Y_MAX)/2 + rcdata.y * ((Y_MAX - Y_MIN)/2) / RC_Y_MAX;
		else if (rcdata.y < 0)
			dutyy = (Y_MIN + Y_MAX)/2 - rcdata.y * ((Y_MAX - Y_MIN)/2) / RC_Y_MIN;
		}

//		if (rcdata.y > 0)
//			dutyy = rcdata.y * Y_MAX / RC_Y_MAX;
//		else if (rcdata.y < 0)
//			dutyy = -rcdata.y * Y_MAX / RC_Y_MAX;
//		else
//			dutyy = 0;


		if (n==0 || n== -2){
			rcdata.valid = RC_NONE;
			lcdClear();
			lcdPrintln("Error in");
			lcdPrint("recv(): ");
			lcdPrintInt(n);
			lcdNl();
			lcdRefresh();
		} else {
			lcdClear();
			lcdPrint("Valid: ");
			lcdPrintInt(rcdata.valid);
			lcdNl();
			lcdPrint("Speed: ");
			lcdPrintInt(rcdata.y);
			lcdNl();
			lcdPrint("Brake: ");
			lcdPrintInt(rcdata.z);
			lcdNl();
			lcdPrint("Dir:   ");
			lcdPrintInt(rcdata.x);
			lcdNl();
			lcdPrint("Ret = ");
			lcdPrintInt(n);
			lcdNl();
			lcdPrint("Dutyx =  ");
			lcdPrintInt(dutyx);
			lcdNl();
			lcdPrint("Dutyy =  ");
			lcdPrintInt(dutyy);
			lcdRefresh();
		}


	//}


    }
}
