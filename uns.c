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

void rx(int sockfd)
{
	ssize_t nr;
	ssize_t nw;
	unsigned char buf[BUFSIZ];
	while ((nr = eread(sockfd, buf, BUFSIZ)) > 0) {
		while ((nw = ewrite(STDOUT_FILENO, buf, nr)) != nr) {
		}
	}
}

void tx(int sockfd)
{
	ssize_t nr;
	ssize_t nw;
	unsigned char buf[BUFSIZ];
	while ((nr = eread(STDIN_FILENO, buf, BUFSIZ)) > 0) {
		while ((nw = ewrite(sockfd, buf, nr) != nr)) {
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int sock_fd, conn_fd;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		fprintf(stderr, "can't create socket, REASON: %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	if (bind(sock_fd, (struct sockaddr *)&server_addr,
		 sizeof(server_addr)) < 0) {
		fprintf(stderr, "can't bind, REASON: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (listen(sock_fd, 1) < 0) {
		fprintf(stderr, "can't listen, REASON: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	conn_fd = accept(sock_fd, NULL, NULL);
	if (conn_fd < 0) {
		fprintf(stderr, "can't accept, REASON: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	child_pid = fork();
	switch (child_pid) {
	case 0:
		close(STDIN_FILENO);
		rx(conn_fd);
		break;
	case -1:
		fprintf(stderr, "can't fork, REASON: %s\n", strerror(errno));
		break;
	default:
		close(STDOUT_FILENO);
		tx(conn_fd);
		break;
	}
	kill(child_pid, SIGTERM);
	exit(EXIT_SUCCESS);
}
