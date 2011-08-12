#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/lcd.h"
#include "lcd/print.h"
#include "filesystem/ff.h"

#include "funk/mesh.h"

#include "usetable.h"

#define IMAGEFILE "nyan.lcd"

void ram(void) {
    FIL file;
    int res;
    UINT readbytes;
    uint8_t state = 0;
    int dx, dy;

    uint32_t framems = 100;

    res = f_open(&file, IMAGEFILE, FA_OPEN_EXISTING|FA_READ);
    if(res)
            return;

    /* calculate height */
    setExtFont(GLOBAL(nickfont));
    dx = DoString(0, 0, GLOBAL(nickname));
    dy = (RESY - getFontHeight())/2;

    getInputWaitRelease();
    while(!getInputRaw()) {
        lcdFill(0x55);

        res = f_read(&file, (char *)lcdBuffer, RESX*RESY_B, &readbytes);
        if (res)
            return;

        if (readbytes < RESX*RESY_B) {
            f_lseek(&file, 0);
            continue;
        }

        setExtFont(GLOBAL(nickfont));
        DoString(5, dy, GLOBAL(nickname));

        if (GLOBAL(newmsgcount) > 0) {
            setExtFont("Font_8x8");
            //lcdSetCrsr(0,0);
            lcdPrint("new msgs: ");
            lcdPrintln(IntToStr(GLOBAL(newmsgcount),2,0));
        }

        lcdDisplay();

        if(framems < 100) {
            state = delayms_queue_plus(framems, 0);
        } else {
            getInputWaitTimeout(framems);
        }
    }

    if (getInputRaw()==BTN_UP && GLOBAL(newmsgcount) > 0) {
        char *msg;
        for(int z=0;z<MESHBUFSIZE;z++) {
            if (MO_TYPE(meshbuffer[z].pkt) == GLOBAL(newmsg)) {
                msg = (char *)MO_BODY(meshbuffer[z].pkt);
                break;
            }
        };
		setExtFont("Font_8x8");
        lcdPrint(GLOBAL(newmsg));
        lcdPrintln(": ");
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
        GLOBAL(newmsgcount) = 0;
    }

    if(state)
        work_queue();
}
