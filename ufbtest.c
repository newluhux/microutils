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

#define DATA_SRC_DEF "/dev/urandom"
static char *data_src_fn = NULL;
static int data_src_fd = -1;

int data_src_init(void) {
	data_src_fn = getenv("DATA_SRC");
	if (data_src_fn == NULL)
		data_src_fn = DATA_SRC_DEF;
	if (data_src_fd == -1) {
		data_src_fd = open(data_src_fn,O_RDONLY);
		if (data_src_fd == -1) {
			fprintf(stderr,
				"can't open %s, reason: %s\n",
				data_src_fn,strerror(errno));
			return -1;
		}
	}
	return data_src_fd;
}

void fbdev_fill(useconds_t delay, unsigned long long count) {
	while(count--) {
		if (pread(data_src_fd,fbdev_mem,fbdev_finfo.smem_len,0) <
		    fbdev_finfo.smem_len) {
			fprintf(stderr,
				"can't read %s, reason: %s\n",
				data_src_fn,strerror(errno));
		}
		usleep(delay);
	}
	return;
}

void data_src_uninit(void) {
	close(data_src_fd);
}

void fbdev_uninit(void) {
	if (fbdev_fd != -1)
		close(fbdev_fd);
	if ((void *)fbdev_mem != (void *)-1)
		munmap(fbdev_mem,fbdev_finfo.smem_len);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr,"Usage: %s update_delay update_count\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	if (fbdev_init() == -1)
		exit(EXIT_FAILURE);
	if (data_src_init() == -1)
		exit(EXIT_FAILURE);
	fbdev_blank();
	fbdev_fill(atoll(argv[1]),atoll(argv[2]));
	data_src_uninit();
	fbdev_uninit();
	return 0;
}
