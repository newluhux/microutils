#ifndef _FBDRAW_H_
#define _FBDRAW_H_

#include <errno.h>
#include <stdint.h>
#include "misc.h"

struct fbdraw_info {
	uint8_t *mem;
	unsigned long size;
	unsigned int xres;
	unsigned int yres;
	unsigned int bits_per_pixel;
	unsigned int line_length;
};

inline void fbdraw_draw_pixel(unsigned x, unsigned int y,
			    uint32_t color, struct fbdraw_info *fb)
{
	if ((x >= fb->xres) || (y >= fb->yres))
		return;
	uint8_t *fbp8 = fb->mem;
	int pos = y * fb->line_length + x * fb->bits_per_pixel / 8;
	fbp8 += pos;
	uint16_t *fbp16 = (uint16_t *) fbp8;
	uint32_t *fbp32 = (uint32_t *) fbp8;
	switch (fb->bits_per_pixel) {
	case 32:
		*fbp32 = color;
		break;
	case 16:
		*fbp16 = color;
		break;
	default:
		*fbp8 = color;
		break;
	}
}

inline void fbdraw_draw_xline(unsigned x0, unsigned x1, unsigned int y,
			    uint32_t color, struct fbdraw_info *fb)
{
	unsigned int xmin = MIN(x0, x1);
	unsigned int xmax = MAX(x0, x1);
	for (; xmin <= xmax; xmin++)
		fbdraw_draw_pixel(xmin, y, color, fb);
}

inline void fbdraw_draw_yline(unsigned y0, unsigned y1, unsigned int x,
			    uint32_t color, struct fbdraw_info *fb)
{
	unsigned int ymin = MIN(y0, y1);
	unsigned int ymax = MAX(y0, y1);
	for (; ymin <= ymax; ymin++)
		fbdraw_draw_pixel(x, ymin, color, fb);
}

inline void fbdraw_draw_rect(unsigned int x, unsigned int y,
			   unsigned int w, unsigned int h,
			   uint32_t color, struct fbdraw_info *fb)
{
	// draw up
	fbdraw_draw_xline(x, x + w, y, color, fb);
	// draw down
	fbdraw_draw_xline(x, x + w, y + h, color, fb);
	// draw left
	fbdraw_draw_yline(y, y + h, x, color, fb);
	// draw right
	fbdraw_draw_yline(y, y + h, x + w, color, fb);
}

inline void fbdraw_draw_rect_solid(unsigned int x, unsigned int y,
				 unsigned int w, unsigned int h,
				 uint32_t color, struct fbdraw_info *fb)
{
	unsigned int ycur;
	for (ycur = y; ycur <= (y + h); ycur++)
		fbdraw_draw_xline(x, x + w, ycur, color, fb);
}

#ifdef _BITMAP_H_
inline void fbdraw_draw_bitmap(unsigned int x, unsigned int y,
			     struct bitmap *bm, uint32_t * colorfg,
			     uint32_t * colorbg, struct fbdraw_info *fb)
{
	uint8_t *bmdata = bm->data;
	unsigned int bx, by;
	for (by = 0; by < bm->h; by++) {
		for (bx = 0; bx < bm->w; bx++) {
			if ((*(bmdata + by) & 1 << bx) && colorfg != NULL) {
				fbdraw_draw_pixel(bx + x, by + y,
						       *colorfg, fb);
			} else if (colorbg != NULL) {
				fbdraw_draw_pixel(bx + x, by + y,
						       *colorbg, fb);
			}
		}
	}
}
#endif

#endif
