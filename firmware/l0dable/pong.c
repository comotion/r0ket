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
#define WALL_HEIGHT 67
#define WALL_WIDTH 2
#define MAX_HEIGHT 66
#define MAX_WIDTH 96
#define POINT_HEIGHT 4
#define POINT_WIDTH 4
#define STEP_PER_LOOP 1
#define BUMPER_HEIGHT 10
#define BUMPER_WIDTH 2
#define BUMPER_STEP 3

void o_rectangle (int x0, int y0, int width, int height);
void createWall();
void calcPoint();
void calcBumperPos();

int pointPos[2]; //0:x, 1:y
int vector[2]; //0:vector to x, 1:vector to y
int bumperPos[2];
int resetFlag;

void ram(void)
{
	//int input;
	char test[512]; /* scratch space */
	int ord[2]; //0:x, 1:y
	
	o_init (test, sizeof(test));
	lcdClear();
	o_identity (); /* reset tranforms */
	
	o_path_new ();
	ord[0] = 20;
	ord[1] = 20;
	o_move_to (0, 0);

	pointPos[0] = 1;
	pointPos[1] = 1;
	vector[0] = 1;
	vector[1] = 1;
	
	resetFlag = 0;
	
	bumperPos[0] = MAX_WIDTH - BUMPER_WIDTH;
	bumperPos[1] = (MAX_HEIGHT - BUMPER_HEIGHT) / 2;
	
	o_set_gray (255);
		
	while(1){
		lcdClear();
		
		//get current pressed key
		if(getInputRaw() == BTN_ENTER){
			resetFlag = 0;
		}else if (getInputRaw() ==  BTN_LEFT){
			//exit the game
			return;
		}else if (getInputRaw() ==  BTN_UP){
			//decrese the position of the bumper
			bumperPos[1] = bumperPos[1] - BUMPER_STEP;
		}else if (getInputRaw() ==  BTN_DOWN){
			//increse the position of the bumper
			bumperPos[1] = bumperPos[1] + BUMPER_STEP;
		}
		
		//o_rectangle (ord[0] , ord[1] , LINE_WIDTH, LINE_HEIGHT);
		
		//check if point was outside of the game
		if(resetFlag == 1){
			pointPos[0] = WALL_WIDTH;
			pointPos[1] = WALL_WIDTH;
		}
		
		//draw and calc all elements
		calcPoint();
		drawWall();
		calcBumperPos();
	
		lcdRefresh();
		lcdDisplay();
		delayms(23);	
	}
}

//===========================================
//= draw the wall
//===========================================
//create the mirror wall
void drawWall(){
	o_rectangle (0 , 0, WALL_WIDTH, WALL_HEIGHT);
}

//===========================================
//= calculate the position and the vector of the point and draw them
//===========================================
//calculate position of the ball
void calcPoint(){
	//calcualte point x
	//if((pointPos[0] + vector[0]) > MAX_WIDTH)
		//pointPos[0] = MAX_WIDTH - (pointPos[0] + vector[0] - MAX_WIDTH);
	//else
	pointPos[0] += vector[0];
		
	//calcualte point y
	//if((pointPos[1] + vector[1]) > MAX_HEIGHT)
		//pointPos[1] = MAX_HEIGHT - (pointPos[1] + vector[1] - MAX_HEIGHT);
	//else
	pointPos[1] += vector[1];
	
	//check if point is in front of the bumper
	if(pointPos[0] >= bumperPos[0] && 
	   pointPos[1] > bumperPos[1] && 
	   pointPos[1] < (bumperPos[1] + BUMPER_HEIGHT)){
		vector[1] = -1 * vector[1];
	}
	
	//reflect the point on the y-axis 
	if(pointPos[1] < 0 || 
	   pointPos[1] > MAX_HEIGHT - POINT_HEIGHT){
		vector[1] = -1 * vector[1];
	}
	
	//reflect the point on the x-axis 
	if(pointPos[0] < WALL_WIDTH){
		  vector[0] = -1 * vector[0]; 	
	}
	
	//point is out if game...restart the game
	if(pointPos[0] > (MAX_WIDTH - BUMPER_WIDTH)){
		resetFlag = 1;
	}
	
	//check if point is in front of the bumper
	if(pointPos[0] + POINT_WIDTH >= bumperPos[0] && 
	   pointPos[1] >= bumperPos[1] && 
	   pointPos[1] <= (bumperPos[1] + BUMPER_HEIGHT)){
		vector[0] = -1 * vector[0];
		//vector[1] = -1 * vector[1];
		resetFlag = 0;
	}
	
	o_rectangle (pointPos[0] * STEP_PER_LOOP, pointPos[1] * STEP_PER_LOOP, POINT_WIDTH, POINT_HEIGHT);
}

//===========================================
//= calculate and draw the bumper
//===========================================
void calcBumperPos(){
	//chek if bumper is in the display range
	if(bumperPos[1] < 0 && (bumperPos[1] + BUMPER_HEIGHT) > MAX_HEIGHT)
		if(bumperPos[1] < 0)
			bumperPos[1] = 0;
		if(bumperPos[1] > (bumperPos[1] + BUMPER_HEIGHT) < MAX_HEIGHT)
			bumperPos[1] = 0;
	else
		
	
	o_rectangle (bumperPos[0], bumperPos[1], BUMPER_WIDTH, BUMPER_HEIGHT);
}

//===========================================
//= draw a rectangle
//===========================================
void o_rectangle (int x0, int y0, int width, int height)
{
  o_path_new ();
  o_move_to (x0, y0);
  o_line_to (x0 + width, y0);
  o_line_to (x0 + width, y0+height);
  o_line_to (x0, y0+height);
  o_close ();
  o_fill ();
}

