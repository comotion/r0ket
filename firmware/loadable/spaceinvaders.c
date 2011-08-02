#include <sysinit.h>

#include "basic/basic.h"

#include "lcd/render.h"
#include "lcd/display.h"
#include "lcd/allfonts.h"

void ReinvokeISP(void);
void EnableWatchdog(uint32_t ms);
void delayms(uint32_t ms);

/**************************************************************************/
#define POS_PLAYER_Y 60
#define POS_UFO_Y 0
#define ENEMY_ROWS 3
#define ENEMY_COLUMNS 6
#define DISABLED 255

#define UFO_PROB 100

#define TYPE_PLAYER 1
#define TYPE_ENEMY_A 2
#define TYPE_ENEMY_B 3
#define TYPE_ENEMY_C 4
#define TYPE_UFO 5

#define BUNKERS 3
#define BUNKER_WIDTH  10
static const BUNKER_X[] = {15, RESX/2-BUNKER_WIDTH/2,RESX-BUNKER_WIDTH-15};
static const ENEMY_WIDTHS[] = {10,11,8};

struct gamestate {
    char player;
	char ufo;
    char shot_x, shot_y;
	char shots_x[ENEMY_COLUMNS];
	char shots_y[ENEMY_COLUMNS];
    char alive;
    char move, direction, lastcol;
    bool killed; 
	bool step;
	uint32_t score;
	char level;
	char rokets;
    char enemy_x[ENEMY_ROWS][ENEMY_COLUMNS];
    char enemy_row_y[ENEMY_ROWS];
    uint8_t bunker[BUNKERS][BUNKER_WIDTH]; 
} game; 
char key;

void init_game();
void check_end();
void move_ufo();
void move_shots();
void move_player();
void move_enemay();
void draw_score();
void draw_bunker();
void draw_player();
void draw_enemy();
void draw_shots();
void draw_ufo();

void ram(void) {
    gpioSetValue (RB_LED1, CFG_LED_OFF); 
    backlightInit();
    
	init_game();
    game.rokets = 3; 
    while (1) {
        ////checkISP();
        lcdFill(0);
		check_end();
		move_ufo();
		move_shot();
		move_shots();
        move_player();
        move_enemy();
        draw_score();
		draw_ufo();
		draw_bunker();
		draw_player();
        draw_enemy();
        draw_shots();
//        draw_status();
        lcdDisplay();
        delayms(12);
    }
    return;
}
void init_game(void) {
	game.player = RESX/2-4;
	game.shot_x = DISABLED;
	game.shot_y = 0;
	game.alive = ENEMY_ROWS*ENEMY_COLUMNS;
	game.move = 0;
	game.direction = -1;
	game.lastcol = ENEMY_COLUMNS-1;
	game.killed = 0;
	game.step = false;
	game.ufo = DISABLED;
	init_enemy();
	
	for (char col=0; col<ENEMY_COLUMNS; col++){
	   game.shots_x[col] = DISABLED;
   	}

	for (int b=0; b<BUNKERS; b++){
		//for (int slice=0; slice<BUNKER_WIDTH; slice++){
		//	game.bunker[b][slice] = 255<<2;
		//}
		game.bunker[b][0] = 0b00111100;
		game.bunker[b][1] = 0b01111100; 
		game.bunker[b][2] = 0b11111100;
		game.bunker[b][3] = 0b11100000;
		game.bunker[b][4] = 0b11100000;
		game.bunker[b][5] = 0b11100000;
		game.bunker[b][6] = 0b11100000;
		game.bunker[b][7] = 0b11111100;
		game.bunker[b][8] = 0b01111100;
		game.bunker[b][9] = 0b00111100;
	}
}

void init_enemy() {
    for (int row = 0; row<ENEMY_ROWS; row++) {
        game.enemy_row_y[row] = 10 + (40/ENEMY_ROWS)*row;
        for (int col = 0; col<ENEMY_COLUMNS; col++) {
            game.enemy_x[row][col] = 5+(86/ENEMY_COLUMNS)*col;
        }
    }
}

void move_shot() {
    //No shot, do nothing
    if(game.shot_x == DISABLED) {
        return;
    }
    
    //moving out of top, end shot
    if (game.shot_y <= 0) {
            game.shot_x = DISABLED;
            return;
    } 
    
    //check for collision with bunker
	for (int b=0; b<BUNKERS; b++) {
		if (game.shot_x>BUNKER_X[BUNKERS-1-b] &&
			game.shot_x<BUNKER_X[BUNKERS-1-b]+BUNKER_WIDTH &&
			game.shot_y<RESY-8 &&
			game.shot_y>RESY-16) {
			int offset = BUNKER_WIDTH - (game.shot_x-BUNKER_X[BUNKERS-1-b]);	
			if (game.bunker[b][offset]!=0) {
				game.bunker[b][offset]&=game.bunker[b][offset]<<1;
				game.shot_x=DISABLED;
			}
		}
	}

    //check for collision with enemy, kill enemy if
    for (int row=0; row<ENEMY_ROWS; row++) {
        if (game.enemy_row_y[row]+6 >= game.shot_y && game.enemy_row_y[row]+6 < game.shot_y+7) {
            for(int col = 0; col<ENEMY_COLUMNS; col++) {
                if(game.shot_x >= game.enemy_x[row][col] && game.shot_x < game.enemy_x[row][col]+ENEMY_WIDTHS[row]) {
                    game.enemy_x[row][col]=DISABLED;
                    game.shot_x = DISABLED;
                    game.alive--;
					game.score++;
                    return;
                }
            }
        }
    }
    //check for collision with ufo
   	if (game.ufo != DISABLED &&
		game.shot_x>game.ufo &&
		game.shot_x<game.ufo + 16 &&
		game.shot_y<8) {

		game.ufo = DISABLED;
		game.score += 5;
	}

    game.shot_y -= 2;
}

void move_shots() {
    for (char col = 0; col<ENEMY_COLUMNS; col++){
		//No shot, maybe generate
		if (game.shots_x[col] == DISABLED) {
			for (char row = 0; row<ENEMY_ROWS; row++) {
				if (game.enemy_x[row][col] != DISABLED) {	
					if(getRandom()%(game.alive*5)==0) {
						game.shots_x[col] = game.enemy_x[row][col]+5;
						game.shots_y[col] = game.enemy_row_y[row]+0;
					}
				}
			}
			continue;
		}

		//moving out of bottm, end shot
		if (game.shots_y[col] >= RESY) {
			game.shots_x[col] = DISABLED;
			return;
		} 
		//check for collision with bunker
		for (int b=0; b<BUNKERS; b++) {
			if (game.shots_x[col]>BUNKER_X[BUNKERS-1-b] &&
					game.shots_x[col]<BUNKER_X[BUNKERS-1-b]+BUNKER_WIDTH &&
					game.shots_y[col]<RESY-8 &&
					game.shots_y[col]>RESY-16) {
				int offset = BUNKER_WIDTH - (game.shots_x[col]-BUNKER_X[BUNKERS-1-b])-1;	
				if (game.bunker[b][offset]!=0) {
					game.bunker[b][offset]&=game.bunker[b][offset]>>1;
					game.shots_x[col]=DISABLED;
				}
			}
		}
		//check for collision with player
		
		if (game.shots_y[col] >= RESY-13 &&
			game.shots_x[col] > game.player+1 &&
			game.shots_x[col] < game.player+6) {

			game.killed = true;
		}
		
		//move shots down
		game.shots_y[col] += 1;
	}
}

void move_ufo() {
	if (game.ufo == DISABLED) {
		if ((getRandom()%UFO_PROB)==0) {
			game.ufo = 0;
		}
		return;
	}
	if (game.ufo >= RESX){
		game.ufo = DISABLED;
		return;
	}
	game.ufo++;
}

void move_player() {
    if(gpioGetValue(RB_BTN0)==0 && game.player > 0 ){
        game.player-=1;
    }
    
    if(gpioGetValue(RB_BTN1)==0 && game.player < RESX-8){
        game.player+=1;
    }
    
     if(gpioGetValue(RB_BTN4)==0 && game.shot_x == 255){
        game.shot_x = game.player+4;
        game.shot_y = POS_PLAYER_Y;
    }
}

void move_enemy() {
    if(game.move > 0){
        game.move--;
        return;
    }
    
	game.step = !game.step;
    for (int col = 0; col < ENEMY_COLUMNS; col++) {
        for (int row = 0; row < ENEMY_ROWS; row++) {
            char pos = game.enemy_x[row][(game.direction==1)?(ENEMY_COLUMNS-(col+1)):col];
            if (pos != DISABLED) {
                //Check collision with player
                if(game.enemy_row_y[row]+8 >= POS_PLAYER_Y && pos+8 >= game.player && pos < game.player+8){
                    game.killed = true;
                }

				//check for collision with bunker, dirty
				for (int b=0; b<BUNKERS; b++) {
					if (pos>=BUNKER_X[BUNKERS-1-b] &&
							pos<=BUNKER_X[BUNKERS-1-b]+BUNKER_WIDTH &&
							game.enemy_row_y[row]+8<RESY-8 &&
							game.enemy_row_y[row]+8>RESY-16) {
						int offset = BUNKER_WIDTH - (pos-BUNKER_X[BUNKERS-1-b]);	
						if (game.bunker[b][offset]!=0) {
							game.bunker[b][offset]&=game.bunker[b][offset]>>2;
						}
					}
				}


			    //Are we at the beginning or end? Direction change	
                if((pos <=0 && game.direction != 1) ||
                   (pos >=RESX-11-1 && game.direction == 1)){
                    game.direction = (game.direction==1)?-1:1;
                    for (char r = 0; r<ENEMY_ROWS; r++) {
                        game.enemy_row_y[r]+=2;
                    }
                    return;
                }
                game.enemy_x[row][(game.direction==1)?(ENEMY_COLUMNS-(col+1)):col] += game.direction;
            }
        }
    }
    
    game.move = game.alive-1;
}

void draw_player() {
    //draw_sprite(50, 20);
    draw_sprite(TYPE_PLAYER, game.player, POS_PLAYER_Y);
}

void draw_ufo() {
	if (game.ufo!=DISABLED)
		draw_sprite(TYPE_UFO, game.ufo, POS_UFO_Y);
}

void draw_enemy() {
    for (int row = 0; row<ENEMY_ROWS; row++) {
        for (int col = 0; col<ENEMY_COLUMNS; col++) {
            if (game.enemy_x[row][col] != DISABLED) {
                draw_sprite(TYPE_ENEMY_A+row,game.enemy_x[row][col],game.enemy_row_y[row]);
            }
        }
    }
}
    
void draw_bunker() {
	for (int b=0; b<BUNKERS; b++) {
		memcpy(lcdBuffer+(RESX*1+BUNKER_X[b]),game.bunker+b,BUNKER_WIDTH);
	}
}

void draw_shots() {
    if (game.shot_x != 255) {
        for (int length=0; length<=5; length++) {
            lcdSetPixel(game.shot_x, game.shot_y+length, true);
        }
    }

	for (char col = 0; col < ENEMY_COLUMNS; col++) {
		if (game.shots_x[col] != DISABLED) {
			for (int length=0; length<=5; length++) {
				lcdSetPixel(game.shots_x[col], game.shots_y[col]+length,true);
			}
		}
	}

}

void draw_status() {
    for (int p = 0; p<game.alive; p++){
        lcdSetPixel(p+1,1,true);
    }
}

void draw_sprite(char type, char x, char y) {
	font = &Font_Invaders;
	switch(type) {
		case TYPE_PLAYER:
			DoChar(x,y-1,'P');
			break;
		case TYPE_ENEMY_A:
			DoChar(x,y-1,game.step?'a':'A');
			break;
		case TYPE_ENEMY_B:
			DoChar(x,y-1,game.step?'b':'B');
			break;
		case TYPE_ENEMY_C:
			DoChar(x,y-1,game.step?'c':'C');
			break;
		case TYPE_UFO:
			DoChar(x,y-1,'U');
			break;
	}
}

void draw_score() {
	font = &Font_7x8;
	DoInt(0,0,game.score);
    
	DoInt(RESX-8,0,game.rokets);
    font = &Font_Invaders;
	DoChar(RESX-16, 0, 'P');
}


void check_end() {
    if (game.killed) {
		game.rokets--;
		delayms(500);
        game.player = RESX/2+4;
        for(int row=0; row<ENEMY_ROWS; row++) {
            game.enemy_row_y[row] = 10 + (40/ENEMY_ROWS)*row;
        }
		for(int col=0; col<ENEMY_COLUMNS; col++) {
			game.shots_x[col] = DISABLED;
		}
        game.killed = false;
    }
    if (game.alive == 0) {
    }
}

void checkISP() {
	if(gpioGetValue(RB_BTN0)==0 && gpioGetValue(RB_BTN4)==0){
		DoString(0,0,"Enter ISP!");
		lcdDisplay();
		ISPandReset();
	}
}


