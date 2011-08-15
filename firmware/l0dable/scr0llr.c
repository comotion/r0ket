#include "basic/basic.h"
#include "basic/config.h"
#include "basic/random.h"
#include "lcd/render.h"
#include "lcd/display.h"
#include "funk/mesh.h"

#include "usetable.h"

void ram() {
  char key;
  int x = 0;
  for(key = 0; key = getInputRaw(), key != BTN_DOWN; ) {
    x = (x + 1) % 64;
    lcdFill(0);
    DoString(x, 0, "scr0llr");
    lcdDisplay();
    delayms_queue(20);
  }
}
