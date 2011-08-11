#include "basic/basic.h"
#include "usetable.h"

void ram(void)
{
	int key;
	int score;
	int x=42-5;
	int y=23;
	//############################
	lcdClear();
        for(int i=0;i<68;i++)
                lcdSetPixel(0,i,1);
        for(int i=0;i<68;i++)
                lcdSetPixel(95,i,1);
        for(int i=0;i<96;i++)
                lcdSetPixel(i,0,1);
        for(int i=0;i<96;i++)
                lcdSetPixel(i,67,1);
	score=0;
	//############################
	while (1) {
		if(getInputRaw()!=key&&getInputRaw()!=BTN_NONE)
			key=getInputRaw();
		if(lcdGetPixel(x,y)){
			//###################################
			if(key!=BTN_NONE){
			lcdClear();
        		for(int i=0;i<68;i++)
                		lcdSetPixel(0,i,1);
        		for(int i=0;i<68;i++)
                		lcdSetPixel(95,i,1);
        		for(int i=0;i<96;i++)
                		lcdSetPixel(i,0,1);
        		for(int i=0;i<96;i++)
                		lcdSetPixel(i,67,1);
			score=0;
			}
			//#######################
                	x=42-5;
                        y=23;
                }
		else{
			DoInt(2,2,score);
			DoString(42,58,"@ CTHN");
			lcdSetPixel(x,y,1);
			if(key==BTN_LEFT)
				x--;
			if(key==BTN_RIGHT)
				x++;
			if(key==BTN_UP)
                       		y--;
                	if(key==BTN_DOWN)
                        	y++;
			if(key==BTN_ENTER){
				lcdClear();
				break;
			}
			score++;
		}
		lcdRefresh();
		delayms(23*5-42);
	}
}
