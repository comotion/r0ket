#include <sysinit.h>
#include <string.h>
#include <stdlib.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "funk/nrf24l01p.h"
#include "funk/filetransfer.h"
#include "funk/rftransfer.h"

#include "lcd/render.h"
#include "lcd/print.h"

#include "usetable.h"

/**************************************************************************/

void sendreset(void);

uint8_t mac[5] = {1,2,3,5,5};

struct NRF_CFG config = 
{
   .channel= 81,
   .txmac= "\x1\x2\x3\x5\x5",
   .nrmacs=1,
   .mac0=  "\x1\x2\x3\x5\x5",
   .maclen ="\x20",
};

                                                



                                                
void ram(void) 
{
  unsigned int lasttime = 0;
  unsigned int time = 0;
  unsigned int seconds = 0;
  unsigned char buf[32];
  int n;
  uint8_t key;
  uint8_t updatenow = 1;
  
  do
  {
    nrf_config_set(&config);
    n = nrf_rcv_pkt_time(100, 32, buf);

    if ( (n == 32) 
          && (buf [0] == 'R')
          && (buf [1] == 'E') 
          && (buf [2] == 'S') 
          && (buf [3] == 'E')
          && (buf [4] == 'T') 
          && (buf [5] == '_') 
          && (buf [6] == 'S') 
          && (buf [7] == 'I')
          && (buf [8] == 'G') 
          && (buf [9] == 'N') )
    {
      seconds = 0;
      updatenow = 1;
    } 
            
    time = systickGetTicks()/100;
    key = getInput();

    if (key == BTN_LEFT)
    {
      return;
    }
    else if(key == BTN_DOWN)
    {
      sendreset();
      seconds = 0;
    }

    if ( (time != lasttime)
          || (updatenow != 0) )
    {
      updatenow = 0;
      lasttime = time;
        
      lcdClear();

      lcdNl();
      lcdPrint("  ");
      lcdPrint(IntToStr(seconds, 5, 0));

      lcdPrintln("s since ");        
      lcdPrintln("   somebody   ");        
      lcdPrintln("    reset     ");
      lcdPrintln("  this sign!  ");
      lcdPrintln("");
      lcdPrintln("");
      lcdPrintln(GLOBAL(nickname));
      lcdRefresh();

      seconds++;        
     
    }
         
  } while (1);

}



void sendreset(void)
{
  char buf[32];
    
  memset (buf, 0, 32);
  strcpy (buf, "RESET_SIGN");
             
  nrf_config_set(&config);
  nrf_snd_pkt_crc(32, (uint8_t *)buf);

  return;
}
