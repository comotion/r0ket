#ifndef __REMOTE_H__
#define __REMOTE_H__

#include <stdint.h>

#define RC_X_MAX  200
#define RC_X_MIN -200
#define RC_Y_MAX  200
#define RC_Y_MIN -200
#define RC_Z_MAX  200
#define RC_Z_MIN -200

#define RC_NONE	0
#define RC_X	1
#define RC_Y	2
#define RC_Z	4
#define RC_ALL	7

struct remote_t {
	uint8_t valid;
	int16_t x;
	int16_t y;
	int16_t z;
} __attribute__((__packed__));

#endif

