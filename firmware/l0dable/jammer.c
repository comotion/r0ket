/*
 * r0ket packet sniffer
 *
 * (c) 2011 Benjamin Schweizer.
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
#include "funk/nrf24l01p.h"
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

    uint8_t buf[32] = "deadbeefdeadbeefdeadbeefdeadbeef"; 
    uint8_t n;

    lcdClear();
    lcdPrintln("init");
    lcdRefresh();

    nrf_config_set(&config);

    lcdPrintln("jamming");
    lcdRefresh();

    while (!done) {
        key = getInput();
        if( key == BTN_DOWN )
            done=1;
        if( key == BTN_LEFT ) {
            config.channel--;
            if (config.channel == 0)
                config.channel = 254;
            lcdPrint("channel=");
            lcdPrintInt(config.channel);
            lcdPrintln("");
            lcdRefresh();
        }
        if( key == BTN_RIGHT ) {
            config.channel++;
            if (config.channel == 255)
                config.channel = 1;
            lcdPrint("channel=");
            lcdPrintInt(config.channel);
            lcdPrintln("");
            lcdRefresh();
        }

        buf[0] = getRandom();
        buf[4] = getRandom();
        buf[8] = getRandom();
        buf[12] = getRandom();
        buf[16] = getRandom();
        buf[20] = getRandom();
        buf[24] = getRandom();
        buf[28] = getRandom();
        nrf_snd_pkt_crc(32, buf); 
        delayms(500);
    }
}

// eof.
