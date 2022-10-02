#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr,"usage: %s pid\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	pid_t target_pid;
	target_pid = atoi(argv[1]);

	char maps_fn[PATH_MAX];
	snprintf(maps_fn,PATH_MAX,"/proc/%d/maps",target_pid);

	FILE *maps_fp = NULL;
	maps_fp = fopen(maps_fn,"r");
	if (maps_fp == NULL) {
		fprintf(stderr,"can't open %s : %s\n",maps_fn,strerror(errno));
		exit(EXIT_FAILURE);
	}

        char mem_fn[PATH_MAX];
        snprintf(mem_fn,PATH_MAX,"/proc/%d/mem",target_pid);

	int mem_fd = -1;
	mem_fd = open(mem_fn,O_RDONLY);
	if (mem_fd < 0) {
		fprintf(stderr,"can't open %s : %s\n",mem_fn,strerror(errno));
		exit(EXIT_FAILURE);
	}

	unsigned long long region_start;
	unsigned long long region_end;

	char line[PATH_MAX];
	char *strtemp;

	long pagesize = sysconf(_SC_PAGESIZE);

	unsigned char buf[pagesize];
	ssize_t nr;

	off_t offset;

	int is_vvar;
	int is_vsyscall;

	while (fgets(line,PATH_MAX,maps_fp) != NULL) {
		if (strstr(line,"[vvar]") != NULL)
			is_vvar = 1;
		else
			is_vvar = 0;
		if (strstr(line,"[vsyscall]") != NULL)
			is_vsyscall = 1;
		else
			is_vsyscall = 0;

		strtemp = strtok(line,"-");
		region_start = strtoull(strtemp,NULL,16);
		strtemp = strtok(NULL," ");
		region_end = strtoull(strtemp,NULL,16);
		offset = region_start;
		while (offset < (off_t)region_end) {
			nr = pread(mem_fd,buf,pagesize,offset);
			if (nr != pagesize) {
				if (is_vvar) {
					fprintf(stderr,
					"in [vvar] region,skip it\n");
					goto next_region;
				}
				if (is_vsyscall) {
					fprintf(stderr,
					"in [vsyscall] region,skip it\n");
					goto next_region;
				}
				fprintf(stderr,
					"can't read %s offset %ld: %s\n",
					mem_fn,offset,strerror(errno));
				exit(EXIT_FAILURE);
			}
			if (fwrite(buf,pagesize,1,stdout) != 1) {
				fprintf(stderr,"can't write to stdout: %s\n",
					strerror(errno));
				exit(EXIT_FAILURE);
			}
			offset += pagesize;
		}
next_region:
		;
	}
	exit(EXIT_SUCCESS);
}
