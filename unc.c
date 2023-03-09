#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

pid_t child_pid = -1;

ssize_t eread(int fd, void *buf, size_t count)
{
	int ret;
	while ((ret = read(fd, buf, count)) < 0) {
		if (errno == EINTR) {
			continue;
		}
		fprintf(stderr, "can't read fd %d, REASON: %s\n", fd,
			strerror(errno));
		_exit(EXIT_FAILURE);
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
		_exit(EXIT_FAILURE);
	}
	return ret;
}

void handle_output(int fd)
{
	ssize_t nr;
	ssize_t nw;
	unsigned char buf[BUFSIZ];
	while ((nr = eread(fd, buf, BUFSIZ)) > 0) {
		while ((nw = ewrite(STDOUT_FILENO, buf, nr)) != nr) {
		}
	}
}

void handle_input(int fd)
{
	ssize_t nr;
	ssize_t nw;
	char c;
	while ((nr = eread(STDIN_FILENO, &c, sizeof(c))) > 0) {
		while ((nw = ewrite(fd, &c, sizeof(c)) != nr)) {
		}
	}
}

/*
this function copy from: http://git.suckless.org/ii/file/ii.c.html#l381
 (C)opyright 2014-2022 Hiltjo Posthuma <hiltjo at codemadness dot org>
 (C)opyright 2005-2006 Anselm R. Garbe <garbeam@wmii.de>
 (C)opyright 2005-2011 Nico Golde <nico at ngolde dot de>
*/
static int tcpopen(const char *host, const char *service)
{
	struct addrinfo hints, *res = NULL, *rp;
	int fd = -1, e;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	/* allow IPv4 or IPv6 */
	hints.ai_flags = AI_NUMERICSERV;	/* avoid name lookup for port */
	hints.ai_socktype = SOCK_STREAM;

	if ((e = getaddrinfo(host, service, &hints, &res))) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
		exit(EXIT_FAILURE);
	}

	for (rp = res; rp; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd == -1)
			continue;
		if (connect(fd, rp->ai_addr, rp->ai_addrlen) == -1) {
			close(fd);
			fd = -1;
			continue;
		}
		break;		/* success */
	}
	if (fd == -1) {
		fprintf(stderr, "could not connect to %s:%s: %s\n", host,
			service, strerror(errno));
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);
	return fd;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s host port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int socket_fd = tcpopen(argv[1], argv[2]);

	child_pid = fork();
	switch (child_pid) {
	case 0:
		close(STDIN_FILENO);
		handle_output(socket_fd);
		break;
	case -1:
		fprintf(stderr, "can't fork, REASON: %s\n", strerror(errno));
		break;
	default:
		close(STDOUT_FILENO);
		handle_input(socket_fd);
		break;
	}
	kill(child_pid, SIGTERM);
	exit(EXIT_SUCCESS);
}
