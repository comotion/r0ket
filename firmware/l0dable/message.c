#include <sysinit.h>
#include <string.h>

#include "basic/basic.h"
//#include "basic/config.h"
//#include "basic/random.h"

#include "funk/mesh.h"

#include "usetable.h"

static bool set_msg(char m[]);

void ram(void) {
	char m[] = "...set sail for fail";
	set_msg(*m);
}

static bool set_msg(char m[]) {
    MPKT * mpkt = meshGetMessage('D');

    MO_TIME_set(mpkt->pkt,1313803870);
    strcpy((char*)MO_BODY(mpkt->pkt),m);
//    if(GLOBAL(privacy)==0){
//        uint32touint8p(GetUUID32(),mpkt->pkt+26);
//        mpkt->pkt[25]=0;
//    };
	return true;
}
