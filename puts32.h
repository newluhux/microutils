#ifndef _PUTS32_H_
#define _PUTS32_H_

/* my embedded board need this, because printf is toooo big */

#include <stdint.h>
#include <stdio.h>

#define putc putchar

void putstr(char *s)
{
	while (*s) {
		putc(*s);
		s++;
	}
}

void puthex(uint32_t hex)
{
	uint32_t temp;
	int i;
	for (i = (sizeof(hex) * 2) - 1; i >= 0; i--) {
		temp = hex;
		temp &= 0xF << (i * 4);
		temp >>= i * 4;
		if (temp >= 0xA) {
			temp -= 0xA;
			temp += 'A';
		} else {
			temp += '0';
		}
		putc(temp);
	}
}

#endif
