#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/lcd.h"
#include "lcd/print.h"

#include "usetable.h"

void ram(void) {
    int dx=0;
    int dy=0;
    int msgdx=0;
    int msgdy=0;
    bool msgshow=1;
    static uint32_t ctr=0;
    ctr++;

    setExtFont(GLOBAL(nickfont));
    dx=DoString(0,0,GLOBAL(nickname));
    dx=(RESX-dx)/2;
    if(dx<0)
        dx=0;
    dy=(RESY-getFontHeight())/2;

    lcdClear();
    lcdSetPixel(1,1,1);
    DoString(dx,dy,GLOBAL(nickname));
    lcdRefresh();

    while(getInputRaw()==BTN_NONE){
        if (GLOBAL(newmsgflag) && msgshow) {
            msgshow = 0;
            DoString(msgdx,msgdy,"New Message");
            lcdRefresh();
        }
        delayms_queue_plus(10,0);
    };

	if (getInputRaw()==BTN_UP && GLOBAL(newmsgflag)) {
		char *foo = GLOBAL(newmsg);
		while(strlen(foo)>13){
	        int q;
	        for(q=0;q<13;q++){
	            if(foo[q]==' ')
	                break;
	        };
	        foo[q]=0;
	        lcdPrintln(foo);
	        foo[q]=' ';
	        foo+=q+1;
	    };
	    lcdPrintln(foo);
	    lcdRefresh();

		getInputWaitRelease();
		//GLOBAL(newmsg) = 
		GLOBAL(newmsgflag) = 0;
	}
    return;
}
