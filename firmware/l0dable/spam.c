#include "basic/basic.h"
#include "basic/config.h"
#include "basic/random.h"
#include "lcd/render.h"
#include "lcd/display.h"
#include "funk/mesh.h"
#include "funk/nrf24l01p.h"

#include "SECRETS"
#include "usetable.h"

static int mesh_sanity(uint8_t * pkt){
    if(MO_TYPE(pkt)>='A' && MO_TYPE(pkt)<='Z'){
        if(MO_TIME(pkt)>1313803870)
            return 1;
        if(MO_TIME(pkt)<1312075898)
            return 1;
    }else if(MO_TYPE(pkt)>='a' && MO_TYPE(pkt)<='z'){
        if(MO_TIME(pkt)>16777216)
            return 1;
        if(MO_TIME(pkt)<0)
            return 1;
    };
    if(MO_TYPE(pkt)>0x7f || MO_TYPE(pkt)<0x20)
        return 1;
    return 0;
};

void ram() {
    lcdClear();

    lcdPrintln("SPAM:");
    while(getInputRaw() != BTN_LEFT) {
        char c[2] = {0, 0};
        c[0] = (char)getRandom();

        uint8_t pkt[32];
        MO_TYPE_set(pkt, c[0]);
        MO_GEN_set(pkt, 0xFF);
        if (c[0] >= 'A' && c[0] <= 'Z')
            MO_TIME_set(pkt, 1313803870);
        else if (c[0] >= 'a' && c[0] <= 'z')
            MO_TIME_set(pkt, getRandom() % 16777216);
        else
            MO_TIME_set(pkt, (uint32_t)getRandom());
        strcpy((char *)MO_BODY(pkt), "chaosbay.camp.ccc.de");

        if (!mesh_sanity(pkt)) {
            lcdPrint("Sending: ");
            lcdPrintln(c);
            lcdRefresh();

            MPKT *mpkt = meshGetMessage(c[0]);
            mpkt->flags = MF_USED;
            memcpy(mpkt->pkt, pkt, 32);
        }
    }
}
