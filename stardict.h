#ifndef _STARDICT_H_
#define _STARDICT_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#define STARDICT_WORDLEN_MAX 255

#define stardict_offset uint32_t
#define stardict_size uint32_t

uint8_t *stardict_index_next(uint8_t * indexp, unsigned long size)
{
	uint8_t *endp = indexp;
	if (size > STARDICT_WORDLEN_MAX)
		size = STARDICT_WORDLEN_MAX;
	endp += size;
	endp -= sizeof(stardict_offset);
	endp -= sizeof(stardict_size);
	while (indexp < endp) {
		if (*indexp == '\0')
			break;
		indexp++;
	}
	if (*indexp != '\0')
		return NULL;
	indexp++;
	indexp += sizeof(stardict_offset);
	indexp += sizeof(stardict_size);
	return indexp;
}

#endif
