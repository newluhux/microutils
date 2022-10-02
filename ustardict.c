#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define STARDICT_WORDLENMAX 255
#define STARDICT_DEF "/usr/share/stardict/default"

static char dict_path[PATH_MAX];

static char idx_fn[PATH_MAX];
static int idx_fd = -1;
static size_t idx_size = 0;
static uint8_t *idx_mmap = NULL;

static char dict_fn[PATH_MAX];
static int dict_fd = -1;

static int stardict_init(void) {
	char *temp = getenv("stardict");
	if (temp == NULL)
		temp = STARDICT_DEF;
	snprintf(dict_path,PATH_MAX,"%s",temp);
	snprintf(idx_fn,PATH_MAX,"%s.idx",dict_path);
	snprintf(dict_fn,PATH_MAX,"%s.dict",dict_path);

	idx_fd = open(idx_fn,O_RDONLY);
	dict_fd = open(dict_fn,O_RDONLY);
	if (idx_fd < 0 || dict_fd < 0)
		return -1;

	struct stat stbuf;
	if (stat(idx_fn,&stbuf) == -1)
		return -1;
	idx_size = stbuf.st_size;

	idx_mmap = mmap(NULL,idx_size,PROT_READ,MAP_SHARED,
			idx_fd,0);
	if (idx_mmap == NULL)
		return -1;

	return 1;
}

static int stardict_strcmp(char *s1, char *s2)
{
        int a;
        a = strcasecmp(s1, s2);
        if (a == 0)
                return (strcmp(s1, s2));
        else
                return a;
}

static int stardict_search(char *word, off_t *offset, size_t *size) {
	uint8_t *idxp = idx_mmap;
	int found = 0;
	
	uint32_t *temp32;

	while (idxp < (idx_mmap + idx_size)) {
		if (stardict_strcmp(word, (char *)idxp) == 0) {
			found = 1;
		}
		while (*idxp != '\0')
			idxp++;
		idxp++;

		if (found == 1) {
			temp32 = (uint32_t *)idxp;
			*offset = (off_t)ntohl(*temp32);
		}
		idxp += sizeof(uint32_t);
		
		if (found == 1) {
			temp32 = (uint32_t *)idxp;
			*size = (size_t)ntohl(*temp32);
			return found;
		}
		idxp += sizeof(uint32_t);
	}
	return found;
}

static int stardict_data_dump(off_t offset, size_t size) {
	uint8_t buf[BUFSIZ];
	ssize_t nr;
	size_t readed = 0;
	if (lseek(dict_fd,offset,SEEK_SET) != offset) {
		return -1;
	}
	while ((nr = read(dict_fd,buf,BUFSIZ)) > 0) {
		if ((readed + nr) > size)
			nr = size - readed;
		if (write(STDOUT_FILENO,buf,nr) != nr) {
			return -1;
		}
		readed += nr;
		if (readed == size)
			break;
	}
	return 1;
}

static int search_and_print(char *word) {
	off_t offset;
	size_t size;
	int ret;
	
	if (stardict_search(word,&offset,&size) != 1) {
		fprintf(stderr,"NO RESULT FOR '%s'\n",word);
		ret = -1;
	} else {
		printf("RESULT FOR '%s': \n",word);
		ret = 1;
	}
	printf("\n");
	if (ret == 1) {
		fflush(stdout);
		stardict_data_dump(offset,size);
		ret = 1;
	}
	printf("\n---------------------------\n");
	return ret;
}

static int getword(char *s, int size, FILE *in) {
	char c;
	s[0] = '\0';
	// skip space before word
	while ((c = fgetc(in)) != EOF && isspace(c));
	if (c == EOF) return c;
	ungetc(c,in);

	int i = 0;
	while(((c=fgetc(in)) != EOF)) {
		if (isspace(c)) break;
		if (i == size-1) break;
		s[i++] = c;
	}
	s[i] = '\0';
	return s[0];
}

int main(int argc, char *argv[]) {
	if (stardict_init() == -1) {
		fprintf(stderr,"ustardict can't init\n");
		exit(EXIT_FAILURE);
	}

	char word[STARDICT_WORDLENMAX];
	int i;

	int hitcount = 0;
	
	if (argc == 1) {
		while(getword(word,STARDICT_WORDLENMAX,stdin) != EOF) {
			if (search_and_print(word) == 1) {
				hitcount++;
			}
		}
	} else {
		for (i=1;i<argc;i++) {
			if (search_and_print(argv[i]) == 1) {
				hitcount++;
			}
		}
	}

	return hitcount;
}
