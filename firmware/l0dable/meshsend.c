/*
 * r0ket mesh send demo
 *
 * 2011 Benjamin Schweizer.
 *
 */

#include <sysinit.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "basic/basic.h"
#include "basic/ecc.h"
#include "funk/mesh.h"
#include "funk/nrf24l01p.h"
#include "funk/rftransfer.h"
#include "lcd/print.h"
#include "usetable.h"

struct NRF_CFG config = {
    .channel= 81,
    .txmac= "\x1\x2\x3\x2\x1",
    .nrmacs=1,
    .mac0=  "\x1\x2\x3\x2\x1",
    .maclen ="\x20",
};

void ram(void) {
    uint8_t key;
    uint8_t done = 0;

    uint8_t n;

    lcdClear();
    lcdPrintln("init");
    lcdRefresh();

    nrf_config_set(&config);

    lcdPrintln("meshsend");
    lcdRefresh();

    MPKT * mpkt= meshGetMessage('*');
    //MO_TIME_set(mpkt->pkt,16777216);
    MO_TIME_set(mpkt->pkt,getSeconds());
    strcpy((char*)MO_BODY(mpkt->pkt),"http://bit.ly/q38WFn");

    getInputWaitTimeout(2000);
}

// eof.
