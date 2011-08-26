#include <sysinit.h>
#include "basic/basic.h"
#include "lcd/render.h"

#define CFG_USBHID 1

#ifdef CFG_USBHID
  #include "usb/usbhid.h"
#endif

void main_kerosin(void) {

	lcdInit();
    DoString(10,5,"USB plug ");
    lcdDisplay();
	// Initialise USB HID
	#ifdef CFG_USBHID
		usbHIDInit();
	#endif


    while (1) {}
}
