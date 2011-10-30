#include <sysinit.h>
#include <string.h>

#include "basic/basic.h"

#include "lcd/lcd.h"
#include "lcd/display.h"
#include "lcd/render.h"

#include "filesystem/ff.h"

#include "usetable.h"

#define LBUFSIZE 26
#define LBACKLOG 6
#define LDISPLAY 6

#define IDLE_MSECS 120   // idle loop time
#define KEY_REPEAT_DELAY 300     // initial delay
#define KEY_REPEAT_INTERVAL 100  // delay when repeating

#define DEBUG(C) { DoChar(0, 60, C); lcdDisplay(); }
#define DEBUG_INT(C) { DoInt(0, 60, C); lcdDisplay(); }
#define DEBUG_INTX(C) { DoIntX(0, 60, C); lcdDisplay(); }

/******************* KEYBOARD STUFF: ********************/

#ifndef INPUTSIZE
#define INPUTSIZE 24
#endif

#define INPUT_NONE 0
#define INPUT_CONT 1
#define INPUT_DONE 2
#define INPUT_QUIT 3

void getLineReset(void);
int getLineHandleButton(int button);

BYTE inputString[INPUTSIZE];

/* private API: */

#define KCOLS 12
#define KROWS 6

#define KNONE 0
#define KQUIT 1
#define KBACKSPACE 2
#define KXMIT 3

int inputPos = 0;
int inputOffs = 0;
int k_row = 2;
int k_col = 7;
int k_last = 2;

/********************************************************/

void ram(void) {
	int keyPressedSince = 0;
	int keyRepeating = 0;
	char keyPressed = BTN_NONE;
	char keyCheck = BTN_NONE;

	int inputStatus;

	int run = 1;

	setExtFont("PX6X6.F0N");

	getLineReset();

	while (run) {
		keyCheck = getInputRaw();

		inputStatus = INPUT_NONE;

		if(keyCheck != keyPressed) {
			if(keyCheck == BTN_NONE) {
				// key released -- handle as keypress
				if(keyRepeating == 0) {
					// ignore when we are in key repeat mode
					inputStatus = getLineHandleButton(keyPressed);
				}
			} else {
				// key pressed -- we do nothing.
			}
			keyPressed = keyCheck;
			keyPressedSince = 0;
			keyRepeating = 0;
		} else if(keyCheck != BTN_NONE) {
			// key hold
			
			keyPressedSince += IDLE_MSECS;
			if(keyPressedSince >= KEY_REPEAT_DELAY) {
				// key repetition
				keyPressedSince = KEY_REPEAT_DELAY - KEY_REPEAT_INTERVAL;
				keyRepeating = 1;
				inputStatus = getLineHandleButton(keyPressed);
			}
		}

		if(inputStatus == INPUT_QUIT) {
			// quit l0dable
			return;
		} else if(inputStatus == INPUT_DONE) {
			// OK was pressed


			delayms_queue(200);
			getLineReset();
		}

		if(inputStatus != INPUT_NONE) {
			lcdDisplay();
		}

		delayms_queue(IDLE_MSECS);
	}
}

char getCharAt(int y, int x) {
	char *charTable = "\3 \2\1\0\0\0\0\0\0\0\0zxcvbnm,./?~asdfghjkl;:\"qwertzuiop\\|1234567890-=!@#$%^&*()_+";
	/*
		KXMIT, ' ', KBACKSPACE, KQUIT, 0, 0, 0, 0, 0, 0, 0, 0,
		'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '?', '~',
		'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', ':', '"',
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\\', '|',
		'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
		'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+'
	};
	*/
	return charTable[y*12+x];
}

void keybPrint(void) {
	int c;
	int i;
	int x;

	if(k_row == 0) {
		DoString(0,60,"OK");
	      	DoString(4*7,60,"[_]");
		DoString(8*7,60,"DEL");
	       	DoChar(12*7,60,'Q');
		if(k_last == 3) {
			c = 12;
			i = 12;
		} else {
			c = k_last*4;
			i = (k_last+1)*4 - 2;
		}
	} else {
		for(c=0; c < KCOLS; c++) {
			DoChar(c*7, 60, getCharAt(k_row, c));
		}
		c = k_col;
		i = k_col;
	}
	i = (12-i)*7 + 5;
	c = (12+1-c)*7 + 5;
	for(x=0; x < 96; x++) {
		if((x < i) || (x > c)) {
			lcdBuffer[x] ^= 0xFF;
		}
	}
}

void getLineReset(void) {
	int i;

	for(i=0; i < INPUTSIZE; i++) inputString[i] = '\0';
	for(i=0; i < 96*2; i++) lcdBuffer[i]=0;
	
	inputPos = 0;
	inputOffs = 0;
	k_row = 2;
	k_col = 3;
	k_last = 1;

	keybPrint();
	lcdDisplay();
}

int getLineHandleButton(int button) {
	int ret;
	int i;
	BYTE kchar;


	kchar = KNONE;
	ret = INPUT_NONE;

	switch(button) {
	case BTN_LEFT:
		if(k_row == 0) {
			// special line
			k_last--;
			if(k_last < 0) k_last = 3;
		} else {
			k_col--;
			if(k_col < 0) k_col = (KCOLS-1);
		}
		break;
	case BTN_RIGHT:
		if(k_row == 0) {
			// special line
			k_last++;
			if(k_last >= 4) k_last = 0;
		} else {
			k_col++;
			if(k_col >= KCOLS) k_col = 0;
		}
		break;
	case BTN_DOWN:
		if(k_row > 0) k_row--;
		break;
	case BTN_UP:
		if(k_row < (KROWS-1)) k_row++;
		break;
	case BTN_ENTER:
		kchar = (k_row == 0) ? getCharAt(0, k_last) : getCharAt(k_row, k_col);
		break;
	}

	if(kchar == KBACKSPACE) {
		if(inputPos > 0) {
			inputPos--;
			inputString[inputPos] = '\0';
		}
		ret = INPUT_CONT;
	} else if(kchar == KXMIT) {
		ret = INPUT_DONE;
	} else if(kchar == KQUIT) {
		ret = INPUT_QUIT;
	} else if(kchar == KNONE) {
		ret = INPUT_NONE;
	} else {
		if(inputPos < (INPUTSIZE - 1)) {
			inputString[inputPos] = kchar;
			inputPos++;
		}
		ret = INPUT_CONT;
	}

	for(i=0; i < 96*2; i++) lcdBuffer[i]=0;

	keybPrint();

	inputOffs = inputPos - 13;
	if(inputOffs < 0) inputOffs = 0;
	DoString(0, 52, (const char*) &(inputString[inputOffs]));
	lcdDisplay();

	return ret;
}

