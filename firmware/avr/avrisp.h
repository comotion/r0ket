/*
  lib to flash an avr via the arm
*/

#ifndef __AVRISP_H_
#define __AVRISP_H_

#include "filesystem/ff.h"
#include "avr/avrspi.h"

#define LSB(I) ((I) & 0xFF)
#define MSB(I) (((I) & 0xF00) >> 8)

typedef enum {
	// avrisp status is OK
	AI_OK = 0,
	// unspecified error
	AI_ERROR,
	// filesize of avr firmware not ok
	AI_FILESIZE,
	// problems with the filehandle
	AI_FILEHANDLE,
	// problems while reading hexfile
	AI_FILE_NOT_READ,
	// reading too little from file
	AI_FILE_READ_INCOMPLETE,
	// cant enter prog mode
	AI_NO_PROG_MODE			
} AI_RESULT;

// check if AVR is in programming mode
uint8_t avrisp_chk_prog_mode(void);

// wait for all data to be written to avr
void avrisp_sync(void);

// perform an erase chip cycle
void avrisp_erase_chip(void);

// write fuses and locks
void avrisp_write_lock_bit(uint8_t bits);
void avrisp_write_fuse_low(uint8_t bits);
void avrisp_write_fuse_high(uint8_t bits);
void avrisp_write_fuse_ext(uint8_t bits);

uint8_t avrisp_read_hex_byte(char *buffer, uint16_t *curPos);
void avrisp_load_prog_page(uint16_t addr, uint8_t low_data, uint8_t high_data);
void avrisp_load_prog_page_dry(uint16_t addr, uint8_t low_data, uint8_t high_data);
void avrisp_write_prog_page_dry(uint16_t addr);
void avrisp_write_prog_page(uint16_t addr);

// flash an image from filesystem
uint8_t avrisp_flash(char *filename);
uint8_t avrisp_flash_dry(filename);

#endif