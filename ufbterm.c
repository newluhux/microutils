#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>

#include "bitmap.h"
#include "term.h"
#include "font.h"
#include "fbdraw.h"

#define FBDEV_DEFAULT "/dev/fb0"

void draw_term(int x, int y,
	       uint32_t colorfg, uint32_t colorbg,
	       struct fbdraw_info *fb, struct term_info *term)
{
	struct bitmap bm;
	memset(&bm, 0, sizeof(bm));
	bm.w = 8;
	bm.h = 8;

	unsigned int term_row;
	unsigned int term_col;
	unsigned char c;

	for (term_row = 0; term_row < term->line_nums; term_row++) {
		for (term_col = 0; term_col < term->line_length; term_col++) {
			c = term->mem[term_row * term->line_length + term_col];
			bm.data = vga_font_8x8[c];
			fbdraw_bitmap(x + term_col * bm.w, y + term_row * bm.h,
					&bm, &colorfg, &colorbg, fb);
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 5) {
		fprintf(stderr, "usage: %s rows cols x y\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	struct term_info term;
	memset(&term, 0, sizeof(term));
	term.line_length = atoi(argv[1]);
	term.line_nums = atoi(argv[2]);
	term.mem_length = term.line_length * term.line_nums;
	term.mem = (uint8_t *) malloc(term.mem_length);
	memset(term.mem, '\0', term.mem_length);
	if (term.mem == NULL) {
		fprintf(stderr, "can't alloc memory %ld bytes,REASON: %s\n",
			term.mem_length, strerror(errno));
		exit(EXIT_FAILURE);
	}
	term.mem_end = term.mem + term.mem_length;
	term.curp = term.mem;

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

	uint32_t bg = 0xFFFFFFFF;
	uint32_t fg = 0x0;

	int x = atoi(argv[3]);
	int y = atoi(argv[4]);

	int i;
	char linebuf[BUFSIZ];
	while (fgets(linebuf, BUFSIZ, stdin) != NULL) {
		for (i = 0; linebuf[i] != '\0'; i++) {
			term_putc(linebuf[i], &term);
		}
		draw_term(x, y, fg, bg, &fb, &term);
	}

	free(term.mem);
	exit(EXIT_SUCCESS);
}
