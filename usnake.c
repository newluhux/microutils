#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>
#include "textui.h"

#define TTY_DEFAULT "/dev/tty"
#define BOARD_W 80
#define BOARD_H 23
#define SNAKE_MAXLEN (BOARD_H * BOARD_W)
#define FG_CHAR '%'
#define SN_CHAR 'S'
#define BG_CHAR '.'

#define SNAKE_UP 0
#define SNAKE_DOWN 1
#define SNAKE_LEFT 2
#define SNAKE_RIGHT 3
#define GAME_SPEED 1

int ttyfd = -1;
struct termios termattr;

struct {
	int ori;
	int start;
	int length;
	int x[SNAKE_MAXLEN];
	int y[SNAKE_MAXLEN];
} snake;

struct {
	int x;
	int y;
} food;

unsigned long long score;

void reset_score(void)
{
	score = 0;
}

void reset_snake(void)
{
	snake.ori = SNAKE_LEFT;
	snake.start = 0;
	snake.length = 1;
	snake.x[snake.start] = rand() % BOARD_W;
	snake.y[snake.start] = rand() % BOARD_H;
}

int is_snake(int x, int y)
{
	int i = snake.start;
	int len = snake.length;
	while (len) {
		if (i >= SNAKE_MAXLEN) {
			i = 0;
		}
		if ((x == snake.x[i]) && (y == snake.y[i])) {
			return 1;
		}
		i++;
		len--;
	}
	return 0;
}

void reset_food(void)
{
	while (snake.length < (SNAKE_MAXLEN - 2)) {
		food.x = rand() % BOARD_W;
		food.y = rand() % BOARD_H;
		if (!is_snake(food.x, food.y)) {
			break;
		}
	}
}

int is_food(int x, int y)
{
	if ((x == food.x) && (y == food.y)) {
		return 1;
	}
	return 0;
}

void redraw(void)
{
	int y;
	char line[BOARD_W + 1];
	line[BOARD_W] = '\0';
	int i;
	for (i = 0; i < BOARD_W; i++) {
		line[i] = BG_CHAR;
	}
	for (y = 0; y < BOARD_H; y++) {
		textui_drawstr(ttyfd, 1, y + 1, line);
	}
	int len = snake.length;
	i = snake.start;
	static char snstr[2] = { SN_CHAR, '\0' };
	while (len) {
		if (i >= SNAKE_MAXLEN) {
			i = 0;
		}
		textui_drawstr(ttyfd, snake.x[i] + 1, snake.y[i] + 1, snstr);
		i++;
		len--;
	}
	static char foodstr[2] = { FG_CHAR, '\0' };
	textui_drawstr(ttyfd, food.x + 1, food.y + 1, foodstr);
	snprintf(line, BOARD_W + 1,
		 "food (%04d,%04d) | snake (%04d, %04d) | score %08lld       ",
		 food.x, food.y, snake.x[snake.start], snake.y[snake.start],
		 score);
	textui_drawstr(ttyfd, 1, BOARD_H + 1, line);
}

void snake_move(void)
{
	int old_start = snake.start;
	int new_start = snake.start;
	new_start = snake.start - 1;
	if (new_start < 0) {
		new_start = SNAKE_MAXLEN - 1;
	}
	switch (snake.ori) {
	case SNAKE_UP:
		snake.x[new_start] = snake.x[old_start];
		snake.y[new_start] = snake.y[old_start] - 1;
		break;
	case SNAKE_DOWN:
		snake.x[new_start] = snake.x[old_start];
		snake.y[new_start] = snake.y[old_start] + 1;
		break;
	case SNAKE_LEFT:
		snake.x[new_start] = snake.x[old_start] - 1;
		snake.y[new_start] = snake.y[old_start];
		break;
	case SNAKE_RIGHT:
		snake.x[new_start] = snake.x[old_start] + 1;
		snake.y[new_start] = snake.y[old_start];
		break;
	}
	if (snake.x[snake.start] < 0) {
		snake.x[new_start] = BOARD_W - 1;
	}
	if (snake.y[snake.start] < 0) {
		snake.y[new_start] = BOARD_H - 1;
	}
	snake.x[new_start] %= BOARD_W;
	snake.y[new_start] %= BOARD_H;
	if (is_food(snake.x[new_start], snake.y[new_start])) {
		/* eat a food */
		score++;
		snake.length++;
		reset_food();
	}
	if (is_snake(snake.x[new_start], snake.y[new_start])) {
		/* eat self */
		redraw();
		exit(EXIT_SUCCESS);
	}
	snake.start = new_start;
}

void timer_handle(int i)
{
	(void)i;
	snake_move();
	redraw();
	alarm(GAME_SPEED);
}

void init_term(void)
{
	/* init terminal */
	char *ttyfn = TTY_DEFAULT;
	ttyfd = open(ttyfn, O_RDWR);
	if (ttyfd < 0) {
		fprintf(stderr, "can't open tty %s,REASON: %s\n",
			ttyfn, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* use for restore origin terminal */
	if (tcgetattr(ttyfd, &termattr) < 0) {
		fprintf(stderr, "can't get term attr %s,REASON: %s\n",
			ttyfn, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* init textui */
	if (textui_init(ttyfd) < 0) {
		fprintf(stderr, "can't init textui %s,REASON: %s\n",
			ttyfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void restore_term(void)
{
	/* reset terminal */
	tcsetattr(ttyfd, TCSANOW, &termattr);
}

int main(void)
{
	/* setup terminal */
	init_term();

	/* setup exit hook */
	atexit(restore_term);

	/* game */
	int c;
	srand(time(NULL));
	reset_snake();
	reset_food();
	reset_score();
	redraw();
	signal(SIGALRM, timer_handle);
	alarm(GAME_SPEED);
	while ((c = getchar()) != 'q') {
		switch (c) {
		case 'k':
			snake.ori = SNAKE_UP;
			break;
		case 'j':
			snake.ori = SNAKE_DOWN;
			break;
		case 'h':
			snake.ori = SNAKE_LEFT;
			break;
		case 'l':
			snake.ori = SNAKE_RIGHT;
			break;
		}
		snake_move();
		redraw();
	}

	exit(EXIT_SUCCESS);
}
