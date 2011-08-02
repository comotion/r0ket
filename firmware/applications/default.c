#include <sysinit.h>
#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/lcd.h"
#include "lcd/fonts/smallfonts.h"
#include "lcd/print.h"
#include "filesystem/ff.h"
#include "usb/usbmsc.h"
#include "basic/random.h"

/**************************************************************************/

void main_default(void) {
    systickInit(SYSTICKSPEED);

    switch(getInputRaw()){
        case BTN_ENTER:
            ISPandReset();
            break;
        case BTN_DOWN:
            usbMSCInit();
            while(1)
                delayms_power(100);
            break;
    };

    readConfig();
    randomInit();

    return;
};



void queue_setinvert(void){
    lcdSetInvert(1);
};
void queue_unsetinvert(void){
    lcdSetInvert(0);
};

#define EVERY(x,y) if((ctr+y)%(x/SYSTICKSPEED)==0)

// every 10 ms
void tick_default(void) {
    static int ctr;
    ctr++;
    incTimer();

    EVERY(1000,0){
        if(!adcMutex){
            VoltageCheck();
            LightCheck();
        }else{
            ctr--;
        };
    };

    static char night=0;
    EVERY(100,2){
        if(night!=isNight()){
            night=isNight();
            if(night){
                backlightSetBrightness(GLOBAL(lcdbacklight));
                push_queue(queue_unsetinvert);
            }else{
                backlightSetBrightness(0);
                push_queue(queue_setinvert);
            };
        };
    };


    EVERY(50,0){
        if(GetVoltage()<3600){
            IOCON_PIO1_11 = 0x0;
            gpioSetDir(RB_LED3, gpioDirection_Output);
            if( (ctr/(50/SYSTICKSPEED))%10 == 1 )
                gpioSetValue (RB_LED3, 1);
            else
                gpioSetValue (RB_LED3, 0);
        };
    };
    return;
};
