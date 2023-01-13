#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdint.h>

struct bitmap {
	uint8_t *data;
	unsigned int w;
	unsigned int h;
};

#endif
