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
#include <sys/sysinfo.h>
#include <time.h>
#include <linux/fb.h>

#include "font.h"
#include "bitmap.h"
#include "framebuffer.h"

#define FBDEV_DEFAULT "/dev/fb0"

char *progname = NULL;

unsigned char default_font_w = 8;
unsigned char default_font_h = 8;

int draw_string(unsigned int x, unsigned int y,
		uint32_t colorfg, uint32_t colorbg,
		char *s, struct framebuffer_info *fb)
{
	struct bitmap bm;
	memset(&bm, 0, sizeof(bm));
	bm.w = default_font_w;
	bm.h = default_font_h;
	while (*s) {
		bm.data = vga_font_8x8[(int)*s];
		framebuffer_draw_bitmap(x, y, &bm, &colorfg, &colorbg, fb);
		x += bm.w;
		s++;
	}
	return 0;
}

int draw_progress_bar(unsigned int x, unsigned int y,
		      unsigned int w, unsigned int h,
		      uint32_t colorfg, uint32_t colorbg,
		      unsigned short progress, struct framebuffer_info *fb)
{
	framebuffer_draw_rect(x, y, w, h, colorfg, fb);
	float fw = w;
	fw -= (float)x;
	if (fw > 0) {
		fw /= (float)100;
		fw *= (float)progress;
	}
	if (progress >= 100)
		fw = w;
	unsigned int pw = (unsigned int)fw;
	framebuffer_draw_rect_solid(x + 1, y + 1, pw - 2, h - 2, colorfg, fb);
	framebuffer_draw_rect_solid(x + pw + 1, y + 1, w - pw - 2, h - 2,
				    colorbg, fb);
	return 0;
}

void get_hostname(char *s, unsigned long size)
{
	static int fd = -1;
	if (fd < 0)
		fd = open("/proc/sys/kernel/hostname", O_RDONLY);
	char hostname[64];
	memset(hostname, ' ', 64);
	if (pread(fd, hostname, 64, 0) < 0)
		hostname[0] = '\0';
	hostname[63] = '\0';
	snprintf(s, size, "HOST:   %s", hostname);
}

void get_uptime(char *s, unsigned long size)
{
	struct sysinfo info;
	sysinfo(&info);
	unsigned long days, hours, mins;
	days = info.uptime / (24 * 60 * 60);
	mins = info.uptime / 60;
	hours = (mins / 60) % 24;
	mins %= 60;
	snprintf(s, size, "UPTIME: %2lu DAYS %2lu HOURS %2lu MINS",
		 days, hours, mins);
}

unsigned short get_cpu_usage(void)
{
	static int fd = -1;
	if (fd == -1)
		fd = open("/proc/stat", O_RDONLY);
	char buf[128];
	memset(buf, 0, 128);
	pread(fd, buf, 128, 0);
	strtok(buf, " ");
	unsigned long user_now = atol(strtok(NULL, " "));
	unsigned long nice_now = atol(strtok(NULL, " "));
	unsigned long system_now = atol(strtok(NULL, " "));
	unsigned long idle_now = atol(strtok(NULL, " "));
	unsigned long iowait_now = atol(strtok(NULL, " "));
	unsigned long irq_now = atoll(strtok(NULL, " "));
	unsigned long softirq_now = atol(strtok(NULL, " "));
	unsigned long steal_now = atol(strtok(NULL, " "));
	unsigned long guest_now = atol(strtok(NULL, " "));
	unsigned long guest_nice_now = atol(strtok(NULL, " "));
	unsigned long total_now = user_now + nice_now + system_now +
	    idle_now + iowait_now + irq_now + softirq_now +
	    steal_now + guest_now + guest_nice_now;
	unsigned long used_now = user_now + nice_now + system_now +
	    irq_now + softirq_now + guest_now + guest_nice_now;
	static unsigned long used_old = 0;
	static unsigned long total_old = 0;
	if (used_old == 0 && total_old == 0) {
		used_old = used_now;
		total_old = total_now;
		return 0;
	}
	float used = (float)(used_now - used_old);
	float total = (float)(total_now - total_old);
	used_old = used_now;
	total_old = total_now;
	return (unsigned short)(used / total * 100.0);
}

unsigned short get_mem_usage(void)
{
	static FILE *fp = NULL;
	if (fp == NULL)
		fp = fopen("/proc/meminfo", "r");
	rewind(fp);
	char line[64];

	unsigned long available, total, used;
	while (fgets(line, 64, fp) != NULL) {
		sscanf(line, "MemAvailable: %lu %*s\n", &available);
		sscanf(line, "MemTotal: %lu %*s\n", &total);
	}
	used = total - available;
	float fu, ft;
	fu = (float)used;
	ft = (float)total;
	return (unsigned short)(fu / ft * 100);
}

int main(int argc, char *argv[])
{
	char *fbdev_pathname = getenv("FRAMEBUFFER");
	if (fbdev_pathname == NULL)
		fbdev_pathname = FBDEV_DEFAULT;
	int fbdev_fd = open(fbdev_pathname, O_RDWR);
	if (fbdev_fd < 0) {
		fprintf(stderr, "can't open framebuffer %s,REASON: %s\n",
			fbdev_pathname, strerror(errno));
		exit(EXIT_FAILURE);
	}
	struct framebuffer_info fb;
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
	useconds_t delay = 100000;
	if (argc > 1)
		delay = atol(argv[1]);
	uint32_t bg = 0xFFFFFFFF;
	uint32_t fg = 0x0;

	char hostname_str[128];
	unsigned int hostname_str_x = 10;
	unsigned int hostname_str_y = 20;

	char uptime_str[128];
	unsigned int uptime_str_x = 10;
	unsigned int uptime_str_y = 30;

	char *cpu_str = "CPU:    ";
	unsigned int cpu_str_x = 10;
	unsigned int cpu_str_y = 40;
	unsigned int cpu_progress = 0;
	unsigned int cpu_progress_x = cpu_str_x + strlen(cpu_str) *
	    default_font_w;
	unsigned int cpu_progress_y = cpu_str_y;

	char *mem_str = "MEM:    ";
	unsigned int mem_str_x = 10;
	unsigned int mem_str_y = 50;
	unsigned int mem_progress = 0;
	unsigned int mem_progress_x = mem_str_x + strlen(mem_str) *
	    default_font_w;
	unsigned int mem_progress_y = mem_str_y;

	// fill bg
	framebuffer_draw_rect_solid(0, 0, fb.xres, fb.yres, bg, &fb);
	while (1) {
		// get hostname
		get_hostname(hostname_str, 128);
		// draw hostname
		draw_string(hostname_str_x, hostname_str_y, fg, bg,
			    hostname_str, &fb);
		// get uptime
		get_uptime(uptime_str, 128);
		// draw uptime
		draw_string(uptime_str_x, uptime_str_y, fg, bg,
			    uptime_str, &fb);
		// get cpu usage
		cpu_progress = get_cpu_usage();
		// draw cpu usage
		draw_string(cpu_str_x, cpu_str_y, fg, bg, cpu_str, &fb);
		draw_progress_bar(cpu_progress_x, cpu_progress_y,
				  fb.xres - cpu_progress_x - 10, 8,
				  fg, bg, cpu_progress, &fb);
		// get mem usage
		mem_progress = get_mem_usage();
		// draw mem usage
		draw_string(mem_str_x, mem_str_y, fg, bg, mem_str, &fb);
		draw_progress_bar(mem_progress_x, mem_progress_y,
				  fb.xres - mem_progress_x - 10, 8,
				  fg, bg, mem_progress, &fb);
		// sleep some time
		if (delay > 0)
			usleep(delay);
	}

	exit(EXIT_SUCCESS);
}
