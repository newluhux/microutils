#ifndef _PUTS32_H_
#define _PUTS32_H_

/* my embedded board need this, because printf is toooo big */

#include <stdint.h>
#include <stdio.h>

#define putc putchar

void putstr(char *s) {
	while (*s) {
		putc(*s);
		s++;
	}
}

void puthex(uint32_t n)
{
        uint32_t val;
        uint8_t hex[sizeof(val) * 2 + 1];
        uint32_t i;

        // split 8 bit to 4 bit, such as : 0xAF to 0x0A, 0x0F
        for (i = 0; i < (sizeof(val) * 2); i++) {
                val = n;
                val >>= i * 4;
                hex[i] = val;
                hex[i] <<= 4; // shiftout high 4 bit
                hex[i] >>= 4; // shiftout low  4 bit
        }

        // convert 4bit to ascii char.
        for (i = 0; i < (sizeof(val) * 2); i++) {
                if (hex[i] < 0xA) {
                        hex[i] += '0';
                } else {
			hex[i] -= 0xA;
                        hex[i] += 'A';
                }
        }

	// human readable
        uint32_t j;
        uint8_t temp;
        i = 0; // start of string
        j = sizeof(val) * 2 - 1; // end of string
        while (i < j) {
                temp = hex[i];
                hex[i] = hex[j];
                hex[j] = temp;
                i++;
                j--;
        }
        hex[sizeof(val) * 2] = '\0'; // end of NUL
        putstr((char *)hex);
}

#endif
