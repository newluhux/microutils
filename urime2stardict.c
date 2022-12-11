#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	if (argc < 4) {
		fprintf(stderr,
			"usage: %s rime.table.txt stardict.idx stardict.dict [-s]\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}

	char *rimefn = argv[1];
	char *idxfn = argv[2];
	char *dictfn = argv[3];
	int swap = 0;
	if ((argc >= 5) && (strcmp(argv[4], "-s") == 0))
		swap = 1;

	FILE *rimefp = fopen(rimefn, "r");
	if (rimefp == NULL) {
		fprintf(stderr, "can't fopen %s,REASON: %s\n",
			rimefn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	FILE *idxfp = fopen(idxfn, "w+");
	if (idxfp == NULL) {
		fprintf(stderr, "can't fopen %s,REASON: %s\n",
			idxfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	FILE *dictfp = fopen(dictfn, "w+");
	if (dictfp == NULL) {
		fprintf(stderr, "can't fopen %s,REASON: %s\n",
			dictfn, strerror(errno));
		exit(EXIT_FAILURE);
	}

	char line[512];
	char key[512];
	char word[512];
	uint32_t offset;
	uint32_t size;
	int ret;
	while (fgets(line, 512, rimefp) != NULL) {
		if (line[0] == '#')
			continue;
		if (line[0] == '\n')
			continue;
		if (swap == 1) {
			strncpy(word, strtok(line, "\t"), 512);
			strncpy(key, strtok(NULL, "\t"), 512);
		} else {
			strncpy(key, strtok(line, "\t"), 512);
			strncpy(word, strtok(NULL, "\t"), 512);
		}
		ret = fprintf(idxfp, "%s", key);
		if (fputc('\0', idxfp) == '\0')
			ret++;
		offset = ftell(dictfp);
		size = strlen(word);
		offset = htonl(offset);
		size = htonl(size);
		ret += fwrite(&offset, sizeof(offset), 1, idxfp);
		ret += fwrite(&size, sizeof(size), 1, idxfp);
		if (ret != strlen(key) + 1 + 1 + 1) {
			fprintf(stderr, "can't write to %s,REASON: %s\n",
				idxfn, strerror(errno));
			exit(EXIT_FAILURE);
		}
		ret = fprintf(dictfp, "%s", word);
		if (ret != strlen(word)) {
			fprintf(stderr, "can't write to %s,REASON: %s\n",
				dictfn, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	fclose(rimefp);
	fclose(idxfp);
	fclose(dictfp);
	exit(EXIT_SUCCESS);
}
