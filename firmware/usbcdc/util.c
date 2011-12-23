#include <sysinit.h>
#include <string.h>

#include "usbcdc/usb.h"
#include "usbcdc/usbcore.h"
#include "usbcdc/cdc_buf.h"
#include "usbcdc/usbhw.h"
#include "usbcdc/cdcuser.h"

#include "basic/basic.h"

#define TO_HEX(i) ((i <= 9) ? '0' + i : 'A' - 10 + i)

volatile unsigned int lastTick;

//static char to_hex(uint8_t in) {
//    return ((in <= 9) : ('0' + in) ? ('A' - 10 + in));
//}

// There must be at least 1ms between USB frames (of up to 64 bytes)
// This buffers all data and writes it out from the buffer one frame
// and one millisecond at a time
int puts(const char * str){
    if(!USB_Configuration)
        return -1;
 
    int len = strlen(str);
    CDC_WrInBuf(str, &len);
    return 0;
}

int write_bytes(const uint8_t *bytes, const uint8_t len){
    uint8_t i = 0;
    char nibbles[3];

    if(!USB_Configuration)
        return -1;
    
    nibbles[2] = '\0';

    for (i=0; i<len; i++) {
        nibbles[0] = TO_HEX(((bytes[i] & 0XF0) >> 4));
        nibbles[1] = TO_HEX((bytes[i] & 0X0F));
        puts_plus(nibbles);
        puts_plus(" ");
    }

    return puts("\r\n");

}

int puts_plus(const char * str){
    return puts(str);
}

void usbCDCInit(){
    CDC_Init();                     // Initialise VCOM
    USB_Init();                     // USB Initialization
    USB_Connect(TRUE);              // USB Connect

    /* You could wait until serial is connected. */
#if 0 // We don't
    uint32_t usbTimeout = 0; 

    while ( usbTimeout < CFG_USBCDC_INITTIMEOUT / 10 ) {
      if (USB_Configuration) break;
      delayms(10);             // Wait 10ms
      usbTimeout++;
    }
#endif
}

void usbCDCOff(void){
    USB_Connect(FALSE);
}
