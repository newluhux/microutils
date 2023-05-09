#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include "kermit.h"

uint8_t param[KERMIT_PARAM_SIZE];
uint8_t txbuf[KERMIT_BUF_MAXSIZE];

ssize_t eread(int fd, void *buf, size_t count)
{
	int ret;
	while ((ret = read(fd, buf, count)) < 0) {
		if (errno == EINTR) {
			continue;
		}
		fprintf(stderr, "can't read fd %d, REASON: %s\n", fd,
			strerror(errno));
		exit(EXIT_FAILURE);
	}
	return ret;
}

ssize_t ewrite(int fd, const void *buf, size_t count)
{
	int ret;
	while ((ret = write(fd, buf, count)) < 0) {
		if (errno == EINTR) {
			continue;
		}
		fprintf(stderr, "can't write fd %d, REASON: %s\n", fd,
			strerror(errno));
		exit(EXIT_FAILURE);
	}
	return ret;
}

void timer_handle(int i)
{
	(void)i;

	uint8_t txlen = kermit_unchar(txbuf[KERMIT_HDR_LEN]) + 2;
	ewrite(STDOUT_FILENO, txbuf, txlen);
        uint8_t eol;
        eol = kermit_unchar(param[KERMIT_PARAM_EOL]);
        ewrite(STDOUT_FILENO, &eol, sizeof(eol));
	alarm(kermit_unchar(param[KERMIT_PARAM_TIME]));
}

int kermit_trans(uint8_t * param, uint8_t * txbuf, uint8_t * rxbuf)
{
	uint8_t txlen = kermit_unchar(txbuf[KERMIT_HDR_LEN]) + 2;
	if (ewrite(STDOUT_FILENO, txbuf, txlen) != txlen) {
		return -1;
	}
	uint8_t eol;
	eol = kermit_unchar(param[KERMIT_PARAM_EOL]);
	if (ewrite(STDOUT_FILENO, &eol, sizeof(eol)) != sizeof(eol)) {
		return -1;
	}
	alarm(kermit_unchar(param[KERMIT_PARAM_TIME]));

	uint8_t *rxbufp = rxbuf;
	/* detect MARK */
	do {
		if (eread(STDIN_FILENO, rxbufp, 1) != 1) {
			return -1;
		}
	} while (rxbuf[KERMIT_HDR_MARK] != KERMIT_MARK_CHAR);
	rxbufp++;

	if (eread(STDIN_FILENO, rxbufp, 3) != 3) {
		return -1;
	}
	rxbufp += 3;
	if ((rxbuf[KERMIT_HDR_LEN] > param[KERMIT_PARAM_MAXLEN]) ||
	    (kermit_unchar(rxbuf[KERMIT_HDR_LEN]) < 3)) {
		fprintf(stderr, "bad len\n");
		return -1;
	}
	if (rxbuf[KERMIT_HDR_SEQ] != txbuf[KERMIT_HDR_SEQ]) {
		fprintf(stderr, "bad seq\n");
		return -1;
	}
	if (rxbuf[KERMIT_HDR_TYPE] != KERMIT_TYPE_Y) {
		fprintf(stderr, "bad ack\n");
		return -1;
	}

	uint8_t needrx = kermit_unchar(rxbuf[KERMIT_HDR_LEN]) - 2;
	if (eread(STDIN_FILENO, rxbufp, needrx) != needrx) {
		return -1;
	}
	rxbufp += needrx;
	rxbufp--;
	uint8_t remote_sum = *rxbufp;
	*rxbufp = 0x00;
	kermit_make_checksum(rxbuf);
	uint8_t local_sum = *rxbufp;

	if (remote_sum != local_sum) {
		fprintf(stderr, "bad sum: %02x\n", *rxbufp);
		return -1;
	}
	return kermit_unchar(rxbuf[KERMIT_HDR_LEN]) + 2;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s file\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	char *fn = argv[1];
	int fd = open(fn, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "can't open file %s, REASON: %s\n", fn,
			strerror(errno));
		exit(EXIT_FAILURE);
	}
	memcpy(param, kermit_param_def, KERMIT_PARAM_SIZE);
	param[KERMIT_PARAM_QBIN] = '&';	/* for 7bit serial port */

	int bufsize;
	bufsize = (param[KERMIT_PARAM_MAXLEN] - 3) >> 2;
	uint8_t *buf = malloc(bufsize);
	if (buf == NULL) {
		fprintf(stderr, "can't alloc memory, REASON: %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	uint8_t rxbuf[KERMIT_BUF_MAXSIZE];

	int seq = 0;

	/* timer init */
	signal(SIGALRM, timer_handle);

	/* send init */
	kermit_make_init(param, seq, txbuf);
	while (kermit_trans(param, txbuf, rxbuf) < 0) ;
	seq = ((seq + 1) % 64);

	/* send filename */
	kermit_make_data(param, seq, txbuf, (uint8_t *)fn,
			strlen(fn), KERMIT_TYPE_F);
	while (kermit_trans(param, txbuf, rxbuf) < 0) ;
	seq = ((seq + 1) % 64);

	ssize_t filesize;
	filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	/* send file data */
	ssize_t nr;
	ssize_t readed = 0;
	while (readed < filesize) {
		if ((nr = read(fd, buf, bufsize)) < 0) {
			fprintf(stderr, "can't read file %s, REASON: %s\n", fn,
				strerror(errno));
			exit(EXIT_FAILURE);
		}
		kermit_make_data(param, seq, txbuf, buf, nr, KERMIT_TYPE_D);
		while (kermit_trans(param, txbuf, rxbuf) < 0) ;
		seq = ((seq + 1) % 64);
		readed += nr;
		fprintf(stderr, "sended: %ld/%ld\n", readed, filesize);
	}

	kermit_make_data(param, seq, txbuf, NULL, 0, KERMIT_TYPE_Z);
	while (kermit_trans(param, txbuf, rxbuf) < 0) ;
	seq = ((seq + 1) % 64);
	kermit_make_data(param, seq, txbuf, NULL, 0, KERMIT_TYPE_B);
	while (kermit_trans(param, txbuf, rxbuf) < 0) ;
	seq = ((seq + 1) % 64);

	exit(EXIT_SUCCESS);
}
