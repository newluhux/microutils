#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr,
			"usage: %s [-l] host port \n",
			argv[0]);
		exit(EXIT_FAILURE);
	}
	
	
}
