#include <sysinit.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "lcd/display.h"
#include "lcd/allfonts.h"
#include "usetable.h"

#define O_DRAWSTEP 3
#define MAX_PIXLE 55
#define LINE_WIDTH 1
#define LINE_HEIGHT 2

void o_rectangle (int x0, int y0, int width, int height);

void ram(void)
{
	//int input;
	char test[512]; /* scratch space */
	int ord[2]; //0:x, 1:y
	int enterFlag = 0;
	
	o_init (test, sizeof(test));
	lcdClear();
	o_identity (); /* reset tranforms */
	
	o_path_new ();
	ord[0] = 20;
	ord[1] = 20;
	o_move_to (0, 0);
	//o_move_to (ord[0], ord[1]);
	
	//while((inpt = getInputRaw()) != BTN_ENTER){
		
		 o_set_gray (255);
		
	while(1){	
		//get current pressed key
		//input = getInputRaw();
		if(getInputRaw() == BTN_ENTER){
			enterFlag ^= 0x01;
		}else if (enterFlag && getInputRaw() == BTN_LEFT){
			ord[0] --;
		}else if (enterFlag && getInputRaw() ==  BTN_RIGHT){
			ord[0] ++;
		}else if (enterFlag && getInputRaw() ==  BTN_UP){
			ord[1] --;
		}else if (enterFlag && getInputRaw() ==  BTN_DOWN){
			ord[1] ++;
		}
		
		o_rectangle (ord[0] , ord[1] , LINE_WIDTH, LINE_HEIGHT);
		
		o_fill ();
		lcdRefresh();
		lcdDisplay();
		delayms(23);	
	}
}

void o_rectangle (int x0, int y0, int width, int height)
{
  o_path_new ();
  o_move_to (x0, y0);
  o_line_to (x0 + width, y0);
  o_line_to (x0 + width, y0+height);
  o_line_to (x0, y0+height);
  o_close ();
}

