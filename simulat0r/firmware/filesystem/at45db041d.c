#define wait_for_ready _hideaway_wait_for_ready
#define dataflash_ioctl _hideaway_dataflash_ioctl
#define dataflash_powerdown _hideaway_dataflash_powerdown
#define dataflash_resume _hideaway_dataflash_resume
#define dataflash_initialize _hideaway_dataflash_initialize
#define dataflash_random_write _hideaway_dataflash_random_write
#define dataflash_write _hideaway_dataflash_write
#define dataflash_random_read _hideaway_dataflash_random_read
#define dataflash_read _hideaway_dataflash_read
#define dataflash_ioctl _hideaway_dataflash_ioctl
#include "../../../firmware/filesystem/at45db041d.c"
#undef wait_for_ready
#undef dataflash_ioctl
#undef dataflash_powerdown
#undef dataflash_resume
#undef dataflash_initialize
#undef dataflash_random_write
#undef dataflash_write
#undef dataflash_random_read
#undef dataflash_read
#undef dataflash_ioctl

#include "string.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int dataflash_file_fd=-2;
void* dataflash_mapped=NULL;

#define DATAFLASH_SIZE 512*1024
#define DATAFLASH_IMAGE "dataflash.img"

static void wait_for_ready() {
    return;
}

static void dataflash_powerdown() {
    return;
}

static void dataflash_resume() {
    return;
}

DSTATUS dataflash_initialize() {
    if(dataflash_file_fd < 0) {
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        dataflash_file_fd = open(DATAFLASH_IMAGE, O_RDWR | O_CREAT, mode);
        if(dataflash_file_fd == -1) {
            perror("open flash image");
            exit(1);
        }
    }
    if(dataflash_mapped == NULL) {
        dataflash_mapped = mmap(0, DATAFLASH_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dataflash_file_fd, 0);
        if(dataflash_mapped == MAP_FAILED) {
            perror("mmap flash image");
            exit(1);
        }
    }

    status &= ~STA_NOINIT;
    return status;
}

DRESULT dataflash_random_read(BYTE *buff, DWORD offset, DWORD length) {
    if (!length) return RES_PARERR;
    if (status & STA_NOINIT) return RES_NOTRDY;
    if (offset+length > MAX_PAGE*256) return RES_PARERR;

    memcpy(buff, dataflash_mapped+offset, length);

    return RES_OK;
}

DRESULT dataflash_read(BYTE *buff, DWORD sector, BYTE count) {
    return dataflash_random_read(buff, sector*512, count*512);
}

#if _READONLY == 0
DRESULT dataflash_random_write(const BYTE *buff, DWORD offset, DWORD length) {
    if (!length) return RES_PARERR;
    if (status & STA_NOINIT) return RES_NOTRDY;
    if (offset+length > MAX_PAGE*256) return RES_PARERR;

    memcpy(dataflash_mapped+offset, buff, length);

    return RES_OK;
}
DRESULT dataflash_write(const BYTE *buff, DWORD sector, BYTE count) {
    return dataflash_random_write(buff, sector*512, count*512);
}
#endif /* _READONLY */

#if _USE_IOCTL != 0
DRESULT dataflash_ioctl(BYTE ctrl, void *buff) {
    DRESULT res;
    BYTE *ptr = buff;

    res = RES_ERROR;


    if (ctrl == CTRL_POWER) {
        switch (*ptr) {
            case 0: /* Sub control code == 0 (POWER_OFF) */
                res = RES_OK;
                break;
            case 1: /* Sub control code == 1 (POWER_ON) */
                res = RES_OK;
                break;
            case 2: /* Sub control code == 2 (POWER_GET) */
                *(ptr+1) = (BYTE)1;
                res = RES_OK;
                break;
            default :
                res = RES_PARERR;
            }
    } else {
        if (status & STA_NOINIT) return RES_NOTRDY;

        switch (ctrl) {
            case CTRL_SYNC:
                res = RES_OK;
                break;
            case GET_SECTOR_COUNT:
                *(WORD*)buff = MAX_PAGE/2;
                res = RES_OK;
                break;
            case GET_SECTOR_SIZE:
                *(WORD*)buff = 512;
                res = RES_OK;
                break;
            case GET_BLOCK_SIZE:
                *(WORD*)buff = 1;
                res = RES_OK;
                break;
            default:
                res = RES_PARERR;
        }
    }

    return res;
}
#endif /* _USE_IOCTL */
