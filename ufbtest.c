#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <linux/fb.h>

#include "font.h"
#include "bitmap.h"
#include "fbdraw.h"

#define FBDEV_DEFAULT "/dev/fb0"

char *progname = NULL;

void help(void)
{
	fprintf(stderr, "usage: %s action delay\n",progname);
}

void do_fill(struct fbdraw_info *fb, useconds_t delay)
{
	uint32_t color;
	while (1) {
		if (delay > 0)
			usleep(delay);
		color = rand();
		fbdraw_draw_rect_solid(0, 0, fb->xres, fb->yres, color,
					    fb);
	}
}

void do_pixel(struct fbdraw_info *fb, useconds_t delay)
{
	uint32_t color;
	unsigned int x, y;
	while (1) {
		if (delay > 0)
			usleep(delay);
		color = rand();
		x = rand() % fb->xres;
		y = rand() % fb->yres;
		fbdraw_draw_pixel(x, y, color, fb);
	}
}

void do_rect(struct fbdraw_info *fb, useconds_t delay)
{
	uint32_t color;
	unsigned int x, y, w, h;
	while (1) {
		if (delay > 0)
			usleep(delay);
		color = rand();
		x = rand() % fb->xres;
		y = rand() % fb->yres;
		w = rand() % (fb->xres - x);
		h = rand() % (fb->yres - y);
		fbdraw_draw_rect_solid(x, y, w, h, color, fb);
	}
}

void do_line(struct fbdraw_info *fb, useconds_t delay)
{
	uint32_t color;
	unsigned int x, y;
	while (1) {
		if (delay > 0)
			usleep(delay);
		color = rand();
		x = rand() % fb->xres;
		y = rand() % fb->yres;
		fbdraw_draw_xline(0, fb->xres, y, color, fb);
		fbdraw_draw_yline(0, fb->yres, x, color, fb);
	}
}
#ifdef _BITMAP_H_
#ifdef _FONT_H_
void do_char(struct fbdraw_info *fb, useconds_t delay)
{
	uint32_t colorfg;
	unsigned int x, y;
	unsigned char c;
	struct bitmap bm;
	memset(&bm,0,sizeof(bm));
	bm.w = 8;
	bm.h = 8;
	while (1) {
		if (delay > 0)
			usleep(delay);
		colorfg = rand();
                x = rand() % fb->xres;
                y = rand() % fb->yres;
		c = rand() % 128;
		bm.data = vga_font_8x8[c];
		fbdraw_draw_bitmap(x,y,&bm,&colorfg,NULL,fb);
	}
	return;
}
#endif
#endif

int main(int argc, char *argv[])
{
	progname = argv[0];
	if (argc < 3) {
		help();
		exit(EXIT_FAILURE);
	}
	char *fbdev_pathname = getenv("FRAMEBUFFER");
	if (fbdev_pathname == NULL)
		fbdev_pathname = FBDEV_DEFAULT;
	int fbdev_fd = open(fbdev_pathname, O_RDWR);
	if (fbdev_fd < 0) {
		fprintf(stderr, "can't open framebuffer %s,REASON: %s\n",
			fbdev_pathname, strerror(errno));
		exit(EXIT_FAILURE);
	}
	struct fbdraw_info fb;
	memset(&fb, 0, sizeof(fb));
	struct fb_var_screeninfo fb_vinfo;
	if (ioctl(fbdev_fd, FBIOGET_VSCREENINFO, &fb_vinfo) < 0) {
		fprintf(stderr,
			"can't get framebuffer %s var_screeninfo,REASON: %s\n",
			fbdev_pathname, strerror(errno));
		exit(EXIT_FAILURE);
	}
	struct fb_fix_screeninfo fb_finfo;
	if (ioctl(fbdev_fd, FBIOGET_FSCREENINFO, &fb_finfo) < 0) {
		fprintf(stderr,
			"can't get framebuffer %s fix_screeninfo,REASON: %s\n",
			fbdev_pathname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fb.xres = fb_vinfo.xres;
	fb.yres = fb_vinfo.yres;
	fb.bits_per_pixel = fb_vinfo.bits_per_pixel;
	fb.size = fb_finfo.smem_len;
	fb.line_length = fb_finfo.line_length;
	fb.mem = mmap(NULL, fb.size, PROT_READ | PROT_WRITE,
		      MAP_SHARED, fbdev_fd, 0);

	printf("xres = %u\n", fb.xres);
	printf("yres = %u\n", fb.yres);
	printf("bits_per_pixel = %u\n", fb.bits_per_pixel);
	printf("size = %lu\n", fb.size);
	printf("line_length = %u\n", fb.line_length);
	if (fb.mem == MAP_FAILED) {
		fprintf(stderr,
			"can't mmap framebuffer %s,REASON: %s\n",
			fbdev_pathname, strerror(errno));
		exit(EXIT_FAILURE);
	}
	srand(time(NULL));
	char *action = argv[1];
	useconds_t delay = atol(argv[2]);
	if (strcmp(action, "fill") == 0) {
		do_fill(&fb, delay);
	} else if (strcmp(action, "pixel") == 0) {
		do_pixel(&fb, delay);
	} else if (strcmp(action, "rect") == 0) {
		do_rect(&fb, delay);
	} else if (strcmp(action, "line") == 0) {
		do_line(&fb, delay);
#ifdef _BITMAP_H_
#ifdef _FONT_H_
	} else if (strcmp(action, "char") == 0) {
		do_char(&fb, delay);
#endif
#endif
	} else {
		help();
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
