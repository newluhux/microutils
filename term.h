#ifndef _TERM_H_
#define _TERM_H_

#include <stdint.h>
#include <string.h>

struct term_info {
	uint8_t *mem;
	uint8_t *mem_end;
	uint8_t *curp;
	unsigned long mem_length;
	unsigned long line_length;
	unsigned long line_nums;
};

void term_scrollup(struct term_info *term)
{
	// move up one line
	memmove(term->mem, term->mem + term->line_length,
		term->mem_length - term->line_length);
	term->curp -= term->line_length;
	// fill white after curp
	int fill = term->line_length;
	if ((term->curp + fill) > term->mem_end)
		fill = term->mem_end - term->curp;
	memset(term->curp, '\0', term->mem_end - term->curp);
}

void term_putc(char c, struct term_info *term)
{
	if (term->curp <= term->mem)
		term->curp = term->mem;
	while (term->curp >= term->mem_end)
		term_scrollup(term);
	*(term->curp) = c;
	if (c == '\n' || c == '\r') {
		term->curp += term->line_length -
		    ((term->curp - term->mem) % term->line_length);
	} else if (c == '\t') {
		term->curp += 8;
	} else if (c == '\b') {
		term->curp -= 1;
	} else {
		term->curp += 1;
	}
}

#endif
