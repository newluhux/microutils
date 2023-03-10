#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tftp.h"

void string2sockaddr(char *ip, int port, struct sockaddr *addr)
{
	struct sockaddr_in *in = (struct sockaddr_in *)addr;
	in->sin_family = AF_INET;
	in->sin_port = htons(port);
	inet_aton(ip, &in->sin_addr);
}

ssize_t udpsend(int sockfd, void *buf, size_t len, struct sockaddr *dest_addr)
{
	ssize_t ret;
	socklen_t addrlen = sizeof(*dest_addr);
	while ((ret = sendto(sockfd, buf, len, 0, dest_addr, addrlen)) < 0) {
		if (errno == EINTR) {
			continue;
		}
		fprintf(stderr, "sendto failed, REASON: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	return ret;
}

ssize_t udprecv(int sockfd, void *buf, size_t len, struct sockaddr *src_addr)
{
	ssize_t ret;
	socklen_t addrlen = sizeof(*src_addr);
	while ((ret = recvfrom(sockfd, buf, len, 0, src_addr, &addrlen)) < 0) {
		if (errno == EINTR) {
			continue;
		}
		fprintf(stderr, "recvfrom failed, REASON: %s\n",
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

void tftp_download(char *serverip, int port, char *filename)
{
	int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (file_fd < 0) {
		fprintf(stderr, "can't open file %s, REASON: %s\n",
			filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		fprintf(stderr, "can't create socket, REASON: %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct sockaddr server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	string2sockaddr(serverip, port, &server_addr);

	unsigned char buf[TFTP_BUFSIZE];

	/* send tftp read request */
	int send_len;
	send_len = tftp_make_rwq(buf, TFTP_RRQ, filename, "octet");
	if (send_len < 0) {
		fprintf(stderr, "filename is too long\n");
		exit(EXIT_FAILURE);
	}
	udpsend(sock_fd, buf, send_len, &server_addr);

	uint16_t opcode;
	uint16_t block;		/* block that report from server */
	uint16_t block_local = 1;
	/* recv from server */
	ssize_t recv_len;
	while ((recv_len =
		udprecv(sock_fd, buf, TFTP_BUFSIZE, &server_addr)) > 0) {
		memcpy(&opcode, buf, sizeof(opcode));
		opcode = ntohs(opcode);
		if (recv_len < TFTP_HDRSIZE) {
			fprintf(stderr, "bad recv data length %ld, drop it.\n",
				recv_len);
			continue;
		}
		if (opcode == TFTP_DATA) {
			memcpy(&block, buf + 2, sizeof(block));
			block = ntohs(block);
			if (block != block_local) {
				fprintf(stderr,
					"server send a bad block number: %d\n",
					block);
				fprintf(stderr,
					"local block number: %d\n",
					block_local);
				exit(EXIT_FAILURE);
			}
			/* save to file */
			ewrite(file_fd, buf + TFTP_HDRSIZE,
			       recv_len - TFTP_HDRSIZE);
			/* send ack to server */
			send_len = tftp_make_ack(buf, block);
			udpsend(sock_fd, buf, send_len, &server_addr);
			/* block recv count */
			block_local++;
			/* pkt is less than TFTP_BUFSIZE, transfer over */
			if (recv_len < TFTP_BUFSIZE) {
				close(file_fd);
				exit(EXIT_SUCCESS);
			}
		} else if (opcode == TFTP_ERR) {
			/* don't access out of buffer */
			buf[recv_len] = '\0';
			fprintf(stderr, "server: %s\n", buf + TFTP_HDRSIZE);
			exit(EXIT_FAILURE);
		} else {
			fprintf(stderr, "unknown packet, drop it.\n");
		}
	}

	close(file_fd);
	close(sock_fd);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s host filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	tftp_download(argv[1], TFTP_PORT, argv[2]);

	exit(EXIT_SUCCESS);
}
