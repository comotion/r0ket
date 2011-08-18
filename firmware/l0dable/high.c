#include "basic/basic.h"
#include "basic/config.h"
//#include "basic/random.h"
#include "lcd/render.h"
#include "lcd/display.h"
#include "funk/mesh.h"

#include "usetable.h"


static uint32_t highscore_get(char nick[]) {
    MPKT * mpkt= meshGetMessage('i');

    strcpy(nick,(char*)MO_BODY(mpkt->pkt));

	return MO_TIME(mpkt->pkt);
}

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

static int strcmp(char *s1, char *s2) {
  char *p1 = s1, *p2 = s2;
  for(p1 = s1, p2 = s2; *p1 && *p2 && *p1 == *p2; p1++, p2++);
  return *p1 != 0 || *p2 != 0;
}

void ram() {
  char *nick = "Pentagon V1LLAG3";
  char nickbuf[32];  // 16?

  do {
    lcdFill(0);
    DoString(0, 0, "Getting");
    lcdDisplay();
    uint32_t score = highscore_get(nickbuf);

    gpioSetValue (RB_LED2, 1);
    lcdFill(0);
    DoString(0, 0, "Highscore:");
    DoString(0, 16, nickbuf);
    DoInt(0, 32, score);
    lcdDisplay();

    if (strcmp(nick, nickbuf) != 0) {
      lcdFill(0);
      DoString(0, 48, "Setting");
      lcdDisplay();
      delayms_queue(500);
      highscore_set(score+1, nick);
    }

  } while(getInputWaitTimeout(500) != BTN_UP);

}
