#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// is it corrent? I don't know.

#define ERR_MSG "detect memory error!!!\n"

int main(int argc ,char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s bytes\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	size_t mem_size = atoll(argv[1]) / 2;

	unsigned char *mem_a = (unsigned char *)malloc(mem_size);
	unsigned char *mem_b = (unsigned char *)malloc(mem_size);
	if ((mem_a == NULL) || (mem_b == NULL)) {
		fprintf(stderr,"malloc() failed: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	FILE *random_fp = fopen("/dev/urandom","r");
	if (random_fp == NULL) {
		fprintf(stderr,"fopen() failed: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	while (fread(mem_a,mem_size,1,random_fp) != 1) {
		fprintf(stderr,"read() failed: %s\n",strerror(errno));
		fprintf(stderr,"retry...\n");
	}
	memcpy(mem_b,mem_a,mem_size);

loop:   if (memcmp(mem_a,mem_b,mem_size) != 0) {
		write(STDERR_FILENO,ERR_MSG,strlen(ERR_MSG));
		exit(EXIT_FAILURE);
	}
	goto loop;
}
