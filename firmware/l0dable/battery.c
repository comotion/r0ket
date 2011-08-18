#include <sysinit.h>
#include <string.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/render.h"
#include "usetable.h"

// This is a team project from
// * Juna (the complicated mathematical computation)
// * nougad (bit shift magic)
// * fu86 (graphical goodness)

void hLine(int y, int x1, int x2, bool pixel);
void vLine(int x, int y1, int y2, bool pixel);
int calculateLoadLevel();

void ram(void) {
  int v, mv;
  // TODO: Do something with gpioGetValue(RB_PWR_CHRG);
  //       Draw a flash or something else on connected USB cable.

  do {
    lcdClear();
    mv = GetVoltage();
    v = mv/1000;

    // Draw the frame
    hLine(16, 14, 74, true);
    hLine(51, 14, 74, true);
    vLine(14, 16, 51, true);
    vLine(74, 16, 24, true);
    vLine(74, 43, 51, true);
    hLine(24, 74, 82, true);
    hLine(43, 74, 82, true);
    vLine(82, 24, 43, true);

    // Fill the battery
    for (int i=16; i<=calculateLoadLevel(); ++i) {
      if (i%8 != 0) vLine(i, 18, 49, true);
    }

    lcdRefresh();
  } while (getInputWaitTimeout(242) == BTN_NONE);
}

void hLine(int y, int x1, int x2, bool pixel) {
  for (int i=x1; i<=x2; ++i) {
    lcdSetPixel(i, y, pixel);
  }
}

void vLine(int x, int y1, int y2, bool pixel) {
  for (int i=y1; i<=y2; ++i) {
    lcdSetPixel(x, i, pixel);
  }
}

int calculateLoadLevel() {
  int mw = GetVoltage() - 3550;
  mw = mw >> 6;
  mw = mw * 3;
  return mw + 16;
}
