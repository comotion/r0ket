#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/lcd.h"
#include "lcd/print.h"

#include "funk/mesh.h"

#include "usetable.h"

//extern MPKT meshbuffer[MESHBUFSIZE];

void ram(void) {
    int dx=0;
    int dy=0;
    int msgcount=0;
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
        if (GLOBAL(newmsgcount) > msgcount) {
            msgcount = GLOBAL(newmsgcount);
            setExtFont("Font_3x6");
			lcdSetCrsr(0,0);
            lcdPrint("new msgs: ");
            lcdPrintln(IntToStr(GLOBAL(newmsgcount),2,0));
            lcdRefresh();
        }
        delayms_queue_plus(10,0);
    };

    if (getInputRaw()==BTN_UP && GLOBAL(newmsgcount) > 0) {
        char *msg;
        for(int z=0;z<MESHBUFSIZE;z++) {
            if (MO_TYPE(meshbuffer[z].pkt) == GLOBAL(newmsg))
                msg = (char *)MO_BODY(meshbuffer[z].pkt);
		};
        while(strlen(msg)>13){
            int q;
            for(q=0;q<13;q++){
                if(msg[q]==' ')
                    break;
            };
            msg[q]=0;
            lcdPrintln(msg);
            msg[q]=' ';
            msg+=q+1;
        };
        lcdPrintln(msg);
        lcdRefresh();

        getInputWaitRelease();
        //GLOBAL(newmsg) =
        GLOBAL(newmsgcount) = 0;
    }
    return;
}
