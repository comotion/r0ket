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
#include "lcd/render.h"
#include "lcd/allfonts.h"
#include "basic/ecc.h"
#include "filesystem/ff.h"
#include "filesystem/diskio.h"
#include "funk/filetransfer.h"
#include "lcd/print.h"
#include "funk/nrf24l01p.h"
#include "funk/filetransfer.h"
#include "funk/rftransfer.h"
#include "basic/basic.h"
#include "basic/xxtea.h"
#include "filesystem/ff.h"
#include "lcd/print.h"
#include "funk/mesh.h"

#include "usetable.h"
#include "funk/nrf24l01p.h"

/*
struct NRF_CFG config = {
    .channel= 81,
    .txmac= "\x1\x2\x3\x2\x1",
    .nrmacs=1,
    .mac0=  "\x1\x2\x3\x2\x1",
    .maclen ="\x20",
};
*/
struct NRF_CFG config = {
    .channel= 83,
    .txmac= "MESHB",
    .nrmacs=1,
    .mac0=  "MESHB",
    .maclen ="\x20",
};

void ram(void) {
    uint8_t key;
    uint8_t done = 0;

    uint8_t buf[32]; 
    uint8_t n;

    lcdClear();

    MPKT * mpkt= meshGetMessage('i');

    for (int i=0; i<32; i++) {
        lcdPrint(IntToStrX(mpkt->pkt[i], 2));
        if (!(i % 6))
            lcdPrintln("");
    }
    lcdPrintln("");
    lcdRefresh();

    while (!done) {
        key = getInput();
        if( key == BTN_DOWN ) {
            lcdPrintln("abort");
            done=1;
        }
        delayms(500);
    }

    lcdPrintln("finished");
    lcdRefresh();
}

// eof.
