#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

struct bitmap {
	uint8_t *data;
	unsigned int w;
	unsigned int h;
};

int bitmap_check(struct bitmap *bm)
{
	if (bm == NULL)
		return -EINVAL;
	if (bm->data == NULL)
		return -EINVAL;
	if (bm->w == 0 || bm->h == 0)
		return -EINVAL;
	return 0;
}

int bitmap_view(struct bitmap *bm, char fg, char bg)
{
	int ret = 0;
	ret = bitmap_check(bm);
	if (ret < 0)
		return ret;
	unsigned int x, y;
	uint8_t *bmdata = bm->data;
	for (y = 0; y < bm->h; y++) {
		for (x = 0; x < bm->w; x++) {
			if (*(bmdata + y) & 1 << x) {
				putchar(fg);
			} else {
				putchar(bg);
			}
		}
		putchar('\n');
	}
	return ret;
}

#endif
