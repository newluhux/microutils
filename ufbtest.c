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

#define FBDEV_DEF "/dev/fb0"
static char *fbdev_fn = NULL;
static int fbdev_fd = -1;
static uint8_t *fbdev_mem = NULL;
static struct fb_fix_screeninfo fbdev_finfo;
static struct fb_var_screeninfo fbdev_vinfo;

int fbdev_init(void)
{
	fbdev_fn = getenv("FRAMEBUFFER");
	if (fbdev_fn == NULL)
		fbdev_fn = FBDEV_DEF;
	fbdev_fd = open(fbdev_fn, O_RDWR);
	if (fbdev_fd == -1) {
		fprintf(stderr,
			"can't open %s , reason: %s\n",
			fbdev_fn,strerror(errno));
		return -1;
	}
	if (ioctl(fbdev_fd, FBIOGET_FSCREENINFO, &fbdev_finfo) == -1) {
		fprintf(stderr,
			"can't get %s FSCREENINFO, reason: %s\n",
			fbdev_fn,strerror(errno));
		return -1;
	}
	if (ioctl(fbdev_fd, FBIOGET_VSCREENINFO, &fbdev_vinfo) == -1) {
		fprintf(stderr,
			"can't get %s VSCREENINFO, reason: %s\n",
			fbdev_fn,strerror(errno));
	}
	fbdev_mem = (uint8_t *)mmap(NULL,fbdev_finfo.smem_len,
				    PROT_READ | PROT_WRITE, MAP_SHARED,
				    fbdev_fd,0);
	if ((void *)fbdev_mem == (void *)-1) {
		fprintf(stderr,
			"can't mmap %s, reason: %s\n",
			fbdev_fn,strerror(errno));
		return -1;
	}

	return 1;
}

#define fbdev_blank() memset(fbdev_mem, 0, fbdev_finfo.smem_len);

void fbdev_draw_pixel(unsigned int x, unsigned int y, uint32_t color) {
	int pos = y * fbdev_finfo.line_length +
		x * sizeof(fbdev_vinfo.bits_per_pixel);
	uint8_t *fbp8 = (uint8_t *)fbdev_mem + pos;
	uint16_t *fbp16 = (uint16_t *)fbp8;
	uint32_t *fbp32 = (uint32_t *)fbp8;
	switch(fbdev_vinfo.bits_per_pixel) {
	case 32:
		*fbp32 = color;
		break;
	case 16:
		*fbp16 = color;
		break;
	case 8:
		*fbp8 = color;
		break;
	default:
		*fbp8 = color;
		break;
	}
	return;
}

void fbdev_draw_rect(unsigned int pos_x, unsigned int pos_y,
		     unsigned int rect_h, unsigned int rect_w,
		     uint32_t color) {
	unsigned int i;

	// up
	for (i=pos_x;i<=pos_x+rect_w;i++) {
		fbdev_draw_pixel(i,pos_y,color);
	}
	// down
	for (i=pos_x;i<=pos_x+rect_w;i++) {
		fbdev_draw_pixel(i,pos_y+rect_h,color);
	}
	// left
	for (i=pos_y;i<=pos_y+rect_h;i++) {
		fbdev_draw_pixel(pos_x,i,color);
	}
	// right
	for (i=pos_y;i<=pos_y+rect_h;i++) {
		fbdev_draw_pixel(pos_x+rect_w,i,color);
	}
	return;
}

void fbdev_draw_rect_solid(unsigned int pos_x, unsigned int pos_y,
                     unsigned int rect_h, unsigned int rect_w,
                     uint32_t color) {
	unsigned int x,y;
	for (y=pos_y;y<=(pos_y+rect_h);y++) {
		for (x=pos_x;x<=(pos_x+rect_w);x++) {
			fbdev_draw_pixel(x,y,color);
		}
	}
	return;
}

void fbdev_fill_random_pixel(useconds_t delay, unsigned long long count,
			     uint32_t *color) {
	srand(time(NULL));
	unsigned int x;
	unsigned int y;
        uint32_t random_color;
        if (color == NULL)
                color = &random_color;
	while(count--) {
		random_color = rand();
		x = rand() % fbdev_vinfo.xres;
		y = rand() % fbdev_vinfo.yres;
		fbdev_draw_pixel(x,y,*color);
		usleep(delay);
	}
	return;
}

void fbdev_fill_random_rect(useconds_t delay, unsigned long long count,
			    uint32_t *color) {
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
	srand(time(NULL));
        uint32_t random_color;
        if (color == NULL)
                color = &random_color;
	while(count--) {
		random_color = rand();
		x = rand() % fbdev_vinfo.xres;
		y = rand() % fbdev_vinfo.yres;
		w = rand() % (fbdev_vinfo.xres - x);
		h = rand() % (fbdev_vinfo.yres - y);
		fbdev_draw_rect(x,y,h,w,*color);
		usleep(delay);
	}
	return;
}

void fbdev_fill_random_rect_solid(useconds_t delay, unsigned long long count,
                            uint32_t *color) {
        unsigned int x;
        unsigned int y;
        unsigned int w;
        unsigned int h;
        srand(time(NULL));
	uint32_t random_color;
	if (color == NULL)
		color = &random_color;
        while(count--) {
		random_color = rand();
                x = rand() % fbdev_vinfo.xres;
                y = rand() % fbdev_vinfo.yres;
                w = rand() % (fbdev_vinfo.xres - x);
                h = rand() % (fbdev_vinfo.yres - y);
                fbdev_draw_rect_solid(x,y,h,w,*color);
                usleep(delay);
        }
        return;
}

void fbdev_uninit(void) {
	if (fbdev_fd != -1)
		close(fbdev_fd);
	if ((void *)fbdev_mem != (void *)-1)
		munmap(fbdev_mem,fbdev_finfo.smem_len);
}

int main(int argc, char *argv[])
{
	if (argc < 4) {
		fprintf(stderr,
			"Usage: %s mode delay count [hex]\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}
	if (fbdev_init() == -1)
		exit(EXIT_FAILURE);
	fbdev_blank();

	useconds_t delay = atoll(argv[2]);
	unsigned long long count = strtoull(argv[3],NULL,10);
	uint32_t *colorp = NULL;
	uint32_t color;
	if (argc >= 5) {
		color = strtoll(argv[4],NULL,16);
		colorp = &color;
	}

	if (strcmp(argv[1],"pixel") == 0) {
		fbdev_fill_random_pixel(delay,count,colorp);
	} else if (strcmp(argv[1],"rect") == 0) {
		fbdev_fill_random_rect(delay,count,colorp);
	} else if (strcmp(argv[1],"rect_solid") == 0) {
		fbdev_fill_random_rect_solid(delay,count,colorp);
	}
	fbdev_uninit();
	return 0;
}
