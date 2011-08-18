#include <sysinit.h>

#include "basic/basic.h"

#include "usetable.h"

void ram(void){
	for (int x=0;x<20;x++){
		gpioSetValue (RB_LED0, x%2);
		delayms(100);
		gpioSetValue (RB_LED2, (x-1)%2);
		delayms(100);
	};
	gpioSetValue (RB_LED0, 0);
	gpioSetValue (RB_LED2, 0);
};
