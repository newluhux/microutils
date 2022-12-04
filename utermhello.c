#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "textui.h"

#define TTY_DEFAULT "/dev/tty"
char *ttyfn = TTY_DEFAULT;
int ttyfd = -1;
struct termios termattr_save;

int term_restore(void)
{
	return tcsetattr(ttyfd, TCSAFLUSH, &termattr_save);
}

void exit_hook(void)
{
	term_restore();
}

void signal_handle(int signum)
{
	(void)signum;
	exit_hook();
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	ttyfd = open(ttyfn, O_RDWR);
	if (ttyfd < 0) {
		fprintf(stderr, "can't open tty %s,REASON: %s\n",
			ttyfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	// use for restore origin terminal
	if (tcgetattr(ttyfd, &termattr_save) < 0) {
		fprintf(stderr, "can't get term attr %s,REASON: %s\n",
			ttyfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	signal(SIGTERM, signal_handle);
	signal(SIGKILL, signal_handle);
	signal(SIGQUIT, signal_handle);
	signal(SIGSTOP, signal_handle);
	signal(SIGABRT, signal_handle);
	atexit(exit_hook);

	// get terminal size
	struct winsize wsz;
	memset(&wsz, 0, sizeof(wsz));
	if (ioctl(ttyfd, TIOCGWINSZ, &wsz) < 0) {
		fprintf(stderr, "can't get term size %s,REASON: %s\n",
			ttyfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	// init textui
	if (textui_init(ttyfd) < 0) {
		fprintf(stderr, "can't init textui %s,REASON: %s\n",
			ttyfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	// draw string
	textui_drawstr(ttyfd, wsz.ws_col / 2, wsz.ws_row / 2, "Hello World");
	textui_drawstr(ttyfd, wsz.ws_col / 2, wsz.ws_row / 2 + 1,
		       "PRESS ANY KEY EXIT");
	getchar();
	textui_clearterm(ttyfd);

	exit(EXIT_SUCCESS);
}
