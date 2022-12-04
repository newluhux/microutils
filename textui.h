#ifndef _TEXTUI_H_
#define _TEXTUI_H_

#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "termcap.h"

int textui_setcur(int ttyfd, unsigned int col, unsigned int row)
{
	char buf[32];
	snprintf(buf, 32, ESC_SET_CURSOR_POS, row, col);
	return write(ttyfd, buf, strlen(buf));
}

int textui_clearterm(int ttyfd)
{
	int ret;
	ret = textui_setcur(ttyfd, 1, 1);
	if (ret < 0)
		return ret;
	char buf[32];
	snprintf(buf, 32, ESC_CLEAR2EOS);
	return write(ttyfd, buf, strlen(buf));
}

int textui_drawstr(int ttyfd, unsigned int col, unsigned int row, char *s)
{
	int ret;
	ret = textui_setcur(ttyfd, col, row);
	if (ret < 0)
		return ret;
	return write(ttyfd, s, strlen(s));
}

int textui_init(int ttyfd)
{
	int ret;
	// setup terminal attr
	struct termios termattr;
	ret = tcgetattr(ttyfd, &termattr);
	if (ret < 0)
		return ret;
	cfmakeraw(&termattr);
	termattr.c_cc[VMIN] = 1;
	termattr.c_cc[VTIME] = 0;
	ret = tcsetattr(ttyfd, TCSAFLUSH, &termattr);
	if (ret < 0)
		return ret;

	// blank terminal;
	ret = textui_clearterm(ttyfd);
	if (ret < 0)
		return ret;
	return 0;
}

#endif
