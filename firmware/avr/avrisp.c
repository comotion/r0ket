#include "basic/basic.h"
#include "filesystem/ff.h"
#include "lcd/print.h"

#include "avr/avrisp.h"


uint8_t avrisp_chk_prog_mode(void) {
	int failed = 0;
	uint8_t loBuf = 0;
	while (failed < 5) {
		avrspi_tx(0xAC);
        	avrspi_tx(0x53);
		loBuf = avrspi_rx();
		avrspi_tx(0x00);
		if(loBuf  == 0x53) {
			return loBuf;
		}
		else {
			failed++;
			// toggle reset to power AVR back up
        		avrspi_setReset(1);
        		delayms(100);
        		avrspi_setReset(0);
			delayms(400);
		}
	}
	return 0;
}

void avrisp_sync(void) {
	uint8_t check = 1;
	while(check == 1) {
        	avrspi_tx(0xF0);
        	avrspi_tx(0x00);
        	avrspi_tx(0x00);
        	check = avrspi_txrx(0x00);
	}
}

void avrisp_erase_chip(void) {

        avrspi_tx(0xAC);
        avrspi_tx(0x80);
        avrspi_tx(0x00);
        avrspi_tx(0x00);
        avrisp_sync();
        
        // toggle reset to power AVR back up
        avrspi_setReset(1);
        delayms(100);
        avrspi_setReset(0);
}

void avrisp_write_lock_bit(uint8_t bits) {
    bits = bits & 0xFF;
    avrspi_txrx(0xAC);
    avrspi_txrx(0xE0);
    avrspi_txrx(0x00);
    avrspi_txrx(bits);
    avrisp_sync();
}

void avrisp_write_fuse_low(uint8_t bits) {
    bits = bits & 0xFF;
    avrspi_txrx(0xAC);
    avrspi_txrx(0xA0);
    avrspi_txrx(0x00);
    avrspi_txrx(bits);
    avrisp_sync();
}

void avrisp_write_fuse_high(uint8_t bits) {
    bits = bits & 0xFF;
    avrspi_txrx(0xAC);
    avrspi_txrx(0xA8);
    avrspi_txrx(0x00);
    avrspi_txrx(bits);
    avrisp_sync();
}

void avrisp_write_fuse_ext(uint8_t bits) {
    bits = bits & 0xFF;
    avrspi_txrx(0xAC);
    avrspi_txrx(0xA4);
    avrspi_txrx(0x00);
    avrspi_txrx(bits);
    avrisp_sync();
}

uint8_t avrisp_read_hex_byte(char *buffer, uint16_t *curPos) {
    char ascii_char[2];
    ascii_char[0] = buffer[*curPos];
    ascii_char[1] = buffer[*curPos+1];
    *curPos += 2;
    return   (((ascii_char[0] < 65) ? (ascii_char[0]-48) : (ascii_char[0]-55)) << 4)
            | ((ascii_char[1] < 65) ? (ascii_char[1]-48) : (ascii_char[1]-55));
}

void avrisp_load_prog_page(uint16_t addr, uint8_t low_data, uint8_t high_data) {
	// low byte before high byte (see datasheet)
    	avrspi_tx(0x40);
    	avrspi_tx(0x00);
    	// first 4 MSB will be ignored
    	avrspi_tx((uint8_t)(addr & 0xFF));
    	avrspi_tx(low_data);
    	avrisp_sync();
    
    	// high byte
    	avrspi_tx(0x48);
    	avrspi_tx(0x00);
	// first 4 MSB will be ignored
    	avrspi_tx((uint8_t)(addr & 0xFF));
    	avrspi_tx(high_data);
    	avrisp_sync();   
   
    	// if page buffer is full, write to flash
    	if(addr % AVR_PAGE_SIZE == (AVR_PAGE_SIZE-1)) {
        	avrisp_write_prog_page(addr);
    	}
}

void avrisp_load_prog_page_dry(uint16_t addr, uint8_t low_data, uint8_t high_data) {
    //load program memory low byte (little endian)
    lcdPrintln("loadprogpage:");
    lcdPrint("addr:");
    lcdPrintln(IntToStrX(addr,4));
    lcdPrint("low:");
    lcdPrintln(IntToStrX(low_data,2));
    					
    
    //load program memory high byte
    lcdPrint("high:");
    lcdPrintln(IntToStrX(high_data,2));
    lcdRefresh(); 
  
   
    // write page if 32 words have now been written
    if(addr % AVR_PAGE_SIZE == (AVR_PAGE_SIZE-1)) {
    	lcdPrintln("writing page");
    	lcdRefresh();
    	avrisp_write_prog_page_dry(addr);
    }   				
    getInputWait();
    getInputWaitRelease();
}

void avrisp_write_prog_page_dry(uint16_t addr)
{
    lcdPrintln("wrtPage: ");
    lcdPrintln(IntToStrX(addr,6));
    lcdRefresh(); getInputWait();getInputWaitRelease();
}

void avrisp_write_prog_page(uint16_t addr)
{
    avrspi_tx(0x4C);
    //avrspi_tx(MSB(addr));
    //avrspi_tx(LSB(addr));
    // bit 5 and 6 go here
    avrspi_tx((addr & 0x30) >> 4);
    // bit 1-4 are used here
    avrspi_tx((addr & 0x0F));
    avrspi_tx(0x00);
    avrisp_sync();
}

uint8_t avrisp_flash_dry(char *filename) {
	// was flashing succesful?
	AI_RESULT success = AI_ERROR;
	
	// filesystem vars
	FIL fhandle;
	FILINFO finfo;
	FRESULT res;
	UINT readbytes;
	char progBuffer[PROG_BUFFER_SIZE];
	
	// get info on file
	f_stat(filename,&finfo);
	lcdPrint("flen:");
    	lcdPrintln(IntToStr(finfo.fsize,4,0));
	lcdRefresh(); getInputWait();getInputWaitRelease();
	
	// only continue if file has contents
	if( finfo.fsize > 0 && finfo.fsize < PROG_BUFFER_SIZE) {	
		res = f_open(&fhandle, filename, FA_OPEN_EXISTING|FA_READ);

		if(res == FR_OK) {
			// read complete file into ram
			res = f_read(&fhandle, &progBuffer, finfo.fsize, &readbytes);
			if(res == FR_OK) {
				// some error during reading
				if(readbytes < finfo.fsize) {
					f_lseek(&fhandle, 0);
					f_close(&fhandle);
					return AI_FILE_READ_INCOMPLETE;
				}
			}
			else {
				f_close(&fhandle);
				return AI_FILE_NOT_READ;
			}			
			f_close(&fhandle);
		}
		else {
			return AI_FILEHANDLE;
		}	
	}
	else {
		return AI_FILESIZE;
	}
	
	// file is in ram, lets program it to the avr
	
	// avr flashing algo helpers
	int flag = 0;
	uint16_t address = 0;
	uint16_t hexPosition = 0;
	uint8_t length = 0;
	uint8_t i;
	uint8_t low_data = 0;
	uint8_t high_data = 0;
	
	
	
	// use the SPI bus
	avrspi_select();

	// set RST low (active) so AVR goes into SPI prog mode
	avrspi_setReset(0);	
	
	
	// check if progmode can be entered
	if(avrisp_chk_prog_mode() != 0)
	{
		avrspi_tx(0x30);
    		avrspi_tx(0x00);
    		avrspi_tx(0x00);
    		avrspi_tx(0x00);
		
		// perform an chip erase cycle
		avrisp_erase_chip();
		if(avrisp_chk_prog_mode() != 0)
		{
		
			lcdPrintln("erased!");
			lcdRefresh(); getInputWait();getInputWaitRelease();
			
			// write default fuses
			// TODO: needed? dangerous?
			//avrisp_write_lock_bit(0xFF);
    			//avrisp_write_fuse_low(0x64);
    			//avrisp_write_fuse_high(0xDF);
    			//avrisp_write_fuse_ext(0xFF);
    			
    			
    			while( flag == 0 ) {
    				// seems legit?
    				lcdPrint("pos:");
    				lcdPrintln(IntToStr(hexPosition,4,0));
    				lcdPrint("buf[pos]:");
    				lcdPrintln(IntToStrX(progBuffer[hexPosition],2));
				lcdRefresh(); getInputWait();getInputWaitRelease();
				
    				if( progBuffer[hexPosition] == ':' ) {
    					hexPosition++;
    					length = avrisp_read_hex_byte(&progBuffer, &hexPosition);
    					
    					lcdPrint("len:");
    					lcdPrintln(IntToStr(length,4,0));
					lcdRefresh(); getInputWait();getInputWaitRelease();
    					
    					if ( length == 0 ) {
    						// EOF LINE
    						lcdPrint("len==0");
						lcdRefresh(); getInputWait();getInputWaitRelease();
    						flag = 1;
    					}
    					else {
    						// skip beginning of line
    						hexPosition += 6;
    						for( i=0; i<length; i+=2) {
    							low_data = avrisp_read_hex_byte(&progBuffer, &hexPosition);
                        				high_data = avrisp_read_hex_byte(&progBuffer, &hexPosition);
                        				
                        				lcdPrint("lo:");
    							lcdPrintln(IntToStrX(low_data,2));
    							lcdPrint("hi:");
    							lcdPrintln(IntToStrX(high_data,2));
							lcdRefresh(); getInputWait();getInputWaitRelease();
                        				
                        				avrisp_load_prog_page_dry(address, low_data, high_data);
                        
                        				address++;
    						}
    						
    						// search for next ':'
    						while(hexPosition < finfo.fsize) {
    							hexPosition++;
    							if(progBuffer[hexPosition] == ':') {
    								break;
    							}    							
    						}    						
    					}
    				}
    				else {
    					flag = 1;
    				}
    			}
    			
    			avrisp_write_prog_page_dry(address);
			success = AI_OK;
		}
		else {
			success = AI_NO_PROG_MODE;
		}
	}
	else {
		success = AI_NO_PROG_MODE;
	}


	// all done, free SPI bus
	avrspi_deselect();
	
	// set RST hi (inactive) so AVR starts running
	avrspi_setReset(1);
	delayms(300);
	avrspi_setReset(0);
	delayms(300);
	avrspi_setReset(1);
	
	return success;
}

uint8_t avrisp_flash(char *filename) {
	// was flashing succesful?
	AI_RESULT success = AI_ERROR;
	
	// filesystem vars
	FIL fhandle;
	FILINFO finfo;
	FRESULT res;
	UINT readbytes;
	char progBuffer[PROG_BUFFER_SIZE];
	
	// get info on file
	f_stat(filename,&finfo);
	
	// only continue if file has contents
	if( finfo.fsize > 0 && finfo.fsize < PROG_BUFFER_SIZE) {	
		res = f_open(&fhandle, filename, FA_OPEN_EXISTING|FA_READ);

		if(res == FR_OK) {
			// read complete file into ram
			res = f_read(&fhandle, &progBuffer, finfo.fsize, &readbytes);
			if(res == FR_OK) {
				// some error during reading
				if(readbytes < finfo.fsize) {
					f_lseek(&fhandle, 0);
					f_close(&fhandle);
					return AI_FILE_READ_INCOMPLETE;
				}
			}
			else {
				f_close(&fhandle);
				return AI_FILE_NOT_READ;
			}			
			f_close(&fhandle);
		}
		else {
			return AI_FILEHANDLE;
		}	
	}
	else {
		return AI_FILESIZE;
	}
	
	// file is in ram, lets program it to the avr
	
	// avr flashing algo helpers
	int flag = 0;
	uint16_t address = 0;
	uint16_t hexPosition = 0;
	uint8_t length = 0;
	uint8_t i;
	uint8_t low_data = 0;
	uint8_t high_data = 0;
	
	
	
	// use the SPI bus
	avrspi_select();

	// set RST low (active) so AVR goes into SPI prog mode
	avrspi_setReset(0);	
	
	
	// check if progmode can be entered
	if(avrisp_chk_prog_mode() != 0)
	{
		avrspi_tx(0x30);
    		avrspi_tx(0x00);
    		avrspi_tx(0x00);
    		avrspi_tx(0x00);
		
		// perform an chip erase cycle
		avrisp_erase_chip();
		if(avrisp_chk_prog_mode() != 0)
		{
			// write default fuses
			// TODO: needed? dangerous?
			avrisp_write_lock_bit(0xFF);
    			avrisp_write_fuse_low(0x64);
    			avrisp_write_fuse_high(0xDF);
    			avrisp_write_fuse_ext(0xFF);
    			
    			
    			while( flag == 0 ) {
    				// seems legit?
    				if( progBuffer[hexPosition] == ':' ) {
    					hexPosition++;
    					length = avrisp_read_hex_byte(&progBuffer, &hexPosition);
    					
    					if ( length == 0 ) {
    						// EOF LINE
    						flag = 1;
    					}
    					else {
    						// skip beginning of line
    						// TODO: put in address parsing here
    						hexPosition += 6;
    						for( i=0; i<length; i+=2) {
    							low_data = avrisp_read_hex_byte(&progBuffer, &hexPosition);
                        				high_data = avrisp_read_hex_byte(&progBuffer, &hexPosition);
                        				
                        				avrisp_load_prog_page(address, low_data, high_data);
                        
                        				address++;
    						}
    						
    						// search for next ':'
    						while(hexPosition < finfo.fsize) {
    							hexPosition++;
    							if(progBuffer[hexPosition] == ':') {
    								break;
    							}    							
    						}    						
    					}
    				}
    				else {
    					flag = 1;
    				}
    			}
    			
    			avrisp_write_prog_page(address);
			success = AI_OK;
		}
		else {
			success = AI_NO_PROG_MODE;
		}
	}
	else {
		success = AI_NO_PROG_MODE;
	}


	// all done, free SPI bus
	avrspi_deselect();
	
	// set RST hi (inactive) so AVR starts running
	avrspi_setReset(1);
	delayms(300);
	avrspi_setReset(0);
	delayms(300);
	avrspi_setReset(1);
	
	return success;
}