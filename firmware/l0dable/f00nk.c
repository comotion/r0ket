#include "basic/basic.h"
#include "basic/config.h"
#include "basic/random.h"
#include "lcd/render.h"
#include "lcd/display.h"
#include "funk/mesh.h"

#include "usetable.h"


static bool highscore_set(uint32_t score, char nick[]) {
    MPKT * mpkt= meshGetMessage('i');
    if(MO_TIME(mpkt->pkt)>score)
        return false;

    MO_TIME_set(mpkt->pkt,score);
    strcpy((char*)MO_BODY(mpkt->pkt),nick);
    if(GLOBAL(privacy)==0){
        uint32touint8p(GetUUID32(),mpkt->pkt+26);
        mpkt->pkt[25]=0;
    };
	return true;
}

static void foo_set(char *s) {
  MPKT *mpkt = meshGetMessage('G');
  MO_TIME_set(mpkt->pkt, 1313803870);
  strcpy((char*)MO_BODY(mpkt->pkt),s);
}

static void bar_set(char *s) {
  MPKT *mpkt = meshGetMessage('b');
  MO_TIME_set(mpkt->pkt, 2342);
  strcpy((char*)MO_BODY(mpkt->pkt),s);
}

static void time_set(time_t time) {
  MPKT *mpkt = meshGetMessage('T');
  MO_TIME_set(mpkt->pkt, time);
}

void ram() {
  for(int x = 63; x >= 0; x--) {
    lcdFill(0);
    DoString(x, 0, "Setting Highscore");
    lcdDisplay();
  }
  highscore_set(23542, "Pentagon V1LLAG3");

  delayms_queue(500);

  for(int x = 63; x >= 0; x--) {
    lcdFill(0);
    DoString(x, 0, "Setting ...");
    lcdDisplay();
  }
  foo_set("PwnedPwnedPwnedPwnedPwnedPwned  ");
  bar_set("http://chaosbay.camp.ccc.de/    ");

  delayms_queue(500);
  time_set(1313803870);
}
