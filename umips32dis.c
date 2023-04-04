#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 4096
#define LINEMAX 256

#include "mips32.h"

void mips32_instr2asm(uint32_t instr, char *out, int outsize)
{
	if (instr == 0) {
		snprintf(out, outsize, "nop");
		return;
	}

	unsigned op = MIPS32_GET_OP(instr);
	unsigned rs = MIPS32_GET_RS(instr);
	unsigned rt = MIPS32_GET_RT(instr);
	unsigned rd = MIPS32_GET_RD(instr);
	unsigned sh = MIPS32_GET_SH(instr);
	unsigned fn = MIPS32_GET_FN(instr);
	unsigned dt = MIPS32_GET_DT(instr);
	unsigned im = MIPS32_GET_IM(instr);

	char temp[LINEMAX];
	temp[0] = '\0';
	if (op == 0) {
		if (fn <= 7) {
			snprintf(temp, LINEMAX, "0x%02x", sh);
		}
		snprintf(out, outsize,
			 "%s %s %s %s %s",
			 mips32_special_func[fn],
			 mips32_regname[rd], mips32_regname[rs],
			 mips32_regname[rt], temp);
		return;
	}

	if (op == 28) {
		snprintf(out, outsize,
			 "%s %s %s %s",
			 mips32_special2_func[fn],
			 mips32_regname[rd], mips32_regname[rs],
			 mips32_regname[rt]);
		return;
	}
	if (op == 31) {
		snprintf(out, outsize,
			 "%s %s %s %s",
			 mips32_special3_func[fn],
			 mips32_regname[rd],
			 mips32_regname[rs], mips32_regname[rt]);
		return;
	}

	if (op == 1) {
		snprintf(out, outsize, "%s %s 0x%04x",
			 mips32_regimm_rt[rt], mips32_regname[rs], im);
		return;
	}

	if (mips32_opcode[op][0] == 'j') {
		snprintf(out, outsize, "%s 0x%08x", mips32_opcode[op], dt << 2);
		return;
	}

	if (mips32_opcode[op][0] == 'b') {
		snprintf(out, outsize, "%s %s %s 0x%04x",
			 mips32_opcode[op], mips32_regname[rs],
			 mips32_regname[rt], im << 2);
		return;
	}

	if (op >= 8 && op <= 15) {
		snprintf(out, outsize, "%s %s %s 0x%04x",
			 mips32_opcode[op], mips32_regname[rt],
			 mips32_regname[rs], im);
		return;
	}

	if (op >= 32 && op <= 63) {
		snprintf(out, outsize, "%s %s 0x%04x(0x%02x)",
			 mips32_opcode[op], mips32_regname[rt], im, rs);
		return;
	}

	/* for cop0 */
	unsigned sel = MIPS32_GET_SEL(instr);
	if (op == 16) {
		snprintf(out, outsize, "%s %s %s 0x%01x",
			 mips32_cop0_rs[rs], mips32_regname[rt],
			 mips32_regname[rd], sel);
		return;
	}

	snprintf(out, outsize, "unknown instruction, op:0x%02x", op);
	return;
}

int main(void)
{
	int i;
	uint32_t buffer[BUFSIZE];
	char asmstr[LINEMAX];
	ssize_t nr;
	uint32_t offset = 0;
	while ((nr = read(STDIN_FILENO, buffer, BUFSIZE)) > 0) {
		for (i = 0; i < (nr / 4); i++) {
			printf("%08X\t%08X\t", offset, buffer[i]);
			mips32_instr2asm(buffer[i], asmstr, LINEMAX);
			printf("%s\n", asmstr);
			offset += 4;
			fflush(stdout);
		}
	}
	if (nr < 0) {
		fprintf(stderr, "can't read from stdin, REASON: %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
