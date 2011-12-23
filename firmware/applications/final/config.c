#include <sysinit.h>

#include "filesystem/ff.h"
#include "filesystem/execute.h"

/**************************************************************************/
//# MENU config
void changer(void) {
    // now external to save space here
    if(execute_file("config.int")){
        lcdClear();
        lcdPrintln("config");
        lcdPrintln("l0dable");
        lcdPrintln("not found");
        lcdRefresh();
        getInputWait();
        getInputWaitRelease();
    }
}

