#include <sysinit.h>

#include "basic/basic.h"
#include "lcd/lcd.h"
#include "lcd/print.h"
#include "filesystem/ff.h"

#include "usetable.h"

uint8_t lcdShowAnimation(char *fname);

void ram(void)
{
    char filename[13];
    selectFile(filename,"ANI");
    lcdShowAnimation(filename);
}


// format: uint32_t frame_delay
//         char lcdBuffer[864]

uint8_t lcdShowAnimation(char *fname) {
    FIL file;            /* File object */
	int res;
	uint32_t framems;
    UINT readbytes;
	uint8_t state=0;

	res=f_open(&file, fname, FA_OPEN_EXISTING|FA_READ);
	if(res)
		return 1;

	getInputWaitRelease();
	while(!getInputRaw()){
        lcdFill(0x55);
   		res = f_read(&file, (char *) &framems, 4, &readbytes);
		if(res) return -1;

		res = f_read(&file, (char *)lcdBuffer, RESX*RESY_B, &readbytes);
        if(res)
            return -1;
		if(readbytes<RESX*RESY_B){
			f_lseek(&file,0);
            continue;
        };
		lcdDisplay();
        if(framems<100){
            state=delayms_queue_plus(framems,0);
        }else{
            getInputWaitTimeout(framems);
        };
	}

	if(state)
		work_queue();

    return 0;
};

