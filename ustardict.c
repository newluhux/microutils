#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "stardict.h"

#define STARDICT_INDEX_DEFAULT "/usr/share/stardict/default.idx"
#define STARDICT_DICT_DEFAULT "/usr/share/stardict/default.idx"

#define STARDICT_OUTPUT_HEADER "==============================================="
#define STARDICT_OUTPUT_FOOTER "-----------------------------------------------"

unsigned long long stardict_wordcount(uint8_t * idxp, unsigned long size)
{
	uint8_t *stardict_idx_end = idxp + size;
	uint8_t *new_idxp = NULL;
	unsigned long long wordcount = 0;
	while (idxp < stardict_idx_end) {
		wordcount++;
		new_idxp = stardict_index_next(idxp, stardict_idx_end - idxp);
		if (new_idxp == NULL)
			break;
		idxp = new_idxp;
	}
	return wordcount;
}

int stardict_strcmp(char *s1, char *s2)
{
	int r;
	r = strcasecmp(s1, s2);
	if (r == 0)
		return (strcmp(s1, s2));
	else
		return r;
}

uint8_t *stardict_word_search(uint8_t * idxp, unsigned long size, char *word)
{
	uint8_t *stardict_idx_end = idxp + size;
	uint8_t *new_idxp = NULL;
	while (idxp < stardict_idx_end) {
		if (stardict_strcmp(word, (char *)idxp) == 0)
			return idxp;
		new_idxp = stardict_index_next(idxp, stardict_idx_end - idxp);
		if (new_idxp == NULL)
			break;
		idxp = new_idxp;
	}
	return NULL;
}

int stardict_dump_dict(uint8_t * dictp, unsigned long dict_size,
		       unsigned long offset, unsigned long size, FILE * out)
{
	if ((dictp + offset + size) > (dictp + dict_size))
		return -EINVAL;
	return fwrite(dictp + offset, 1, size, out);
}

int stardict_print(char *word,
		   uint8_t * idxp, unsigned long idxsize,
		   uint8_t * dictp, unsigned long dictsize, FILE * out)
{
	stardict_offset dict_data_offset;
	stardict_size dict_data_size;
	printf("%s\n", STARDICT_OUTPUT_HEADER);
	idxp = stardict_word_search(idxp, idxsize, word);
	if (idxp == NULL) {
		printf("NOTFOUND %s\n", word);
		printf("\n%s\n", STARDICT_OUTPUT_FOOTER);
		return -1;
	}
	idxp += strlen((char *)idxp) + 1;
	dict_data_offset = ntohl(*((stardict_offset *) idxp));
	idxp += sizeof(stardict_offset);
	dict_data_size = ntohl(*((stardict_size *) idxp));
	printf("FOUND %s DATAOFFSET 0x%x DATASIZE: 0x%x\n",
	       word, dict_data_offset, dict_data_size);
	stardict_dump_dict(dictp, dictsize,
			   dict_data_offset, dict_data_size, out);
	printf("\n%s\n", STARDICT_OUTPUT_FOOTER);
	return 0;
}

int getword(char *word, unsigned long size, FILE * in)
{
	int c;
	// skip space 
	while (1) {
		c = getc(in);
		if (c == EOF)
			exit(EXIT_SUCCESS);
		if (!isspace(c))
			break;
	}
	ungetc(c, in);

	// copy input to word
	unsigned long i = 0;
	for (; i < (size - 1); i++) {
		c = getc(in);
		if (c == EOF)
			exit(EXIT_SUCCESS);
		if (isspace(c))
			break;
		word[i] = c;
	}
	word[i] = '\0';
	return 1;
}

int main(int argc, char *argv[])
{
	char *stardict_idxfn = getenv("STARDICT_INDEX");
	if (stardict_idxfn == NULL)
		stardict_idxfn = STARDICT_INDEX_DEFAULT;
	int stardict_idxfd = open(stardict_idxfn, O_RDONLY);
	if (stardict_idxfd < 0) {
		fprintf(stderr, "can't open index file %s,REASON: %s\n",
			stardict_idxfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	struct stat stbuf;
	if (stat(stardict_idxfn, &stbuf) < 0) {
		fprintf(stderr, "can't get index file size %s,REASON: %s\n",
			stardict_idxfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	unsigned long stardict_idxsize = stbuf.st_size;
	uint8_t *stardict_idxmem = mmap(NULL, stardict_idxsize,
					PROT_READ, MAP_SHARED,
					stardict_idxfd, 0);
	if (stardict_idxmem == MAP_FAILED) {
		fprintf(stderr, "can't mmap index file %s,REASON: %s\n",
			stardict_idxfn, strerror(errno));
		exit(EXIT_FAILURE);
	}

	char *stardict_dictfn = getenv("STARDICT_DICT");
	if (stardict_dictfn == NULL)
		stardict_dictfn = STARDICT_DICT_DEFAULT;
	int stardict_dictfd = open(stardict_dictfn, O_RDONLY);
	if (stardict_dictfd < 0) {
		fprintf(stderr, "can't open dict file %s,REASON: %s\n",
			stardict_dictfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (stat(stardict_dictfn, &stbuf) < 0) {
		fprintf(stderr, "can't get dict file size %s,REASON: %s\n",
			stardict_dictfn, strerror(errno));
		exit(EXIT_FAILURE);
	}
	unsigned long stardict_dictsize = stbuf.st_size;
	uint8_t *stardict_dictmem = mmap(NULL, stardict_dictsize,
					 PROT_READ, MAP_SHARED,
					 stardict_dictfd, 0);
	if (stardict_dictmem == MAP_FAILED) {
		fprintf(stderr, "can't mmap dict file %s,REASON: %s\n",
			stardict_dictfn, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int i;
	// detect options
	for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-wc") == 0) {
				unsigned long long wordcount;
				wordcount = stardict_wordcount(stardict_idxmem,
							       stardict_idxsize);
				printf("%llu\n", wordcount);
				exit(EXIT_FAILURE);
			}
		}
	}

	char word[STARDICT_WORDLEN_MAX];
	while (argc == 1) {
		getword(word, STARDICT_WORDLEN_MAX, stdin);
		stardict_print(word, stardict_idxmem, stardict_idxsize,
			       stardict_dictmem, stardict_dictsize, stdout);
	}

	for (i = 1; i < argc; i++) {
		stardict_print(argv[i], stardict_idxmem, stardict_idxsize,
			       stardict_dictmem, stardict_dictsize, stdout);
	}
	exit(EXIT_SUCCESS);
}
