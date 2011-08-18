#include "basic/basic.h"
#include "lcd/lcd.h"
#include "usetable.h"

void ram(void)
{
  lcdClear();
  lcdDisplay();
  lcdLoadImage("troll.lcd");
  lcdRefresh();
  getInputWait();
}
