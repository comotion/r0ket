#include <sysinit.h>

#include "basic/basic.h"
#include "lcd/lcd.h"
#include "basic/config.h"
#include "usetable.h"


void w_render(bool bit);
void w_reset(char *data);
void cb_render(char *cb_data[7], int bit);

// 7                                                  xD #l
// 6                                                  yD ##                 
// 5                                                  yD ##                 
// 4  D# l#                xl #l xD xl xl l#    xl #l yD ## x5 #l #l xl #l D# ##          xl #l
// 3  D# ##       #d       U# ## #D ##          U# ## #D ##          U# ## D# ##          U# ##
// 2  r# r# r# r# ux r# r# U# ## #r r# r# r# r# U# ## #r r# r# r# r# U# ## r# r# r# r# r# U# ##
// 1
// 0  x  y4

char *wb_ptr;
int size = 3;
int begin = 42;

int cur_x, cur_y;
int cnt;

enum State {
    None,
    Up,
    Down,
    Wait,
    New,
};

void ram(void) {
    getInputWaitRelease();

    char *wb_data = "xyyy""plDDrrrrudxrrUUxll""xxxxxx""plllDDrrrrrUUxll""xxyyyxx""plDDDDDrrrrrUUxllll""xxxxxx""plDDrrrrrUUxll";
    char *cb_data[7] = 
    {"          #                      ",
     "          #                      ",
     " ##       # ##   ###   ##    ### ",
     "#         #   # #   # #     #   #",
     "#   # ### #   # #   # # # # # # #",
     "#   #     #   # #   #     # #    ",
     " ###       ###   ##     ##   ##  ",};


    lcdClear();
    w_reset(wb_data);

    int state = Up, next;
    int wait = 0;

    while(getInputRaw()==BTN_NONE) {
        next = None;
        switch (state) {
            case Up:
                w_render(1);
                if (! *wb_ptr)
                    next = Wait;
                break;
            case Down:
                w_render(0);
                if (! *wb_ptr)
                    next = New;
                break;
            case New:
                if (--wait == 0)
                    next = Up;
                break;
            case Wait:
                if (--wait == 0)
                    next = Down;
        }

        switch (next) {
            case New:
                cb_render(cb_data, 0);
                wait = 10;
                break;
            case Wait:
                cb_render(cb_data, 1);
                wait = 400;
                break;
            case Down:
                w_reset(wb_data);
                break;
            case Up:
                w_reset(wb_data);
                break;
        }
        if (next != None)
            state = next;

        lcdRefresh();
        delayms_queue_plus(15,0);
    }
    return;
}

void w_reset(char *data) {
    wb_ptr = data;
    cur_x = 0;
    cur_y = begin;
    cnt=0;
}

void w_render(bool bit) {
    int len;
    goto code;
inc:
    wb_ptr++;
    cnt=0;
code:
    len = size;
    switch (*wb_ptr) {
    case 'p':
        cur_x++;
        goto inc;
    case 'x': 
        cur_x+=size; 
        goto inc;
    case 'X':
        cur_x-=size; 
        goto inc;
    case 'y':
        cur_y-=size; 
        goto inc;
    case 'Y':
        cur_y+=size; 
        goto inc;

    case 'R':
        len += size;
    case 'r': 
        if (cnt++ == size)
            goto inc;
        for (int y=cur_y; y<cur_y+len; y++)
            lcdSetPixel(cur_x, y, bit);
        cur_x++;
        break;

    case 'L':
        len += size;
    case 'l':
        if (cnt++ == size)
            goto inc;
        for (int y=cur_y; y<cur_y+len; y++)
            lcdSetPixel(cur_x+size-1, y, bit);
        cur_x--;
        break;

    case 'D':
        len += size;
    case 'd':
        if (cnt++ == size)
            goto inc;
        for (int x=cur_x; x<cur_x+len; x++)
            lcdSetPixel(x, cur_y, bit);
        cur_y++;
        break;

    case 'U':
        len += size;
    case 'u':
        if (cnt++ == size)
            goto inc;
        for (int x=cur_x; x<cur_x+len; x++)
            lcdSetPixel(x, cur_y+size-1, bit);
        cur_y--;
        break;
    }
}

void cb_render(char *cb_data[7], int bit) {
    int sx=60, sy=begin+1;
    for (int y=0; y<7; y++) {
        char *c = cb_data[y];
        for (int x=0; *c!=0; c++, x++) {
            switch (*c) {
                case ' ':
                    break;
                default:
                    lcdSetPixel(sx+x,sy+y,bit);
                    break;
            }
        }
    }
}


