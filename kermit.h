#ifndef _KERMIT_H_
#define _KERMIT_H_

/*
reference:
https://www.kermitproject.org/kproto.pdf
https://ece.northeastern.edu/courses/eceu629/2004sp/labkermit3.pdf
*/

#include <stdint.h>
#include <string.h>

#define kermit_tochar(x) ((x) + 32)
#define kermit_unchar(x) ((x)-32)
#define kermit_ctl(x) ((x) ^ 64)
#define kermit_checksum(x) (kermit_tochar((x + ((x & 192) / 64)) & 63))

#define KERMIT_BUF_MAXSIZE 96

enum {
	KERMIT_TYPE_D = 'D',
	KERMIT_TYPE_Y = 'Y',
	KERMIT_TYPE_N = 'N',
	KERMIT_TYPE_S = 'S',
	KERMIT_TYPE_B = 'B',
	KERMIT_TYPE_F = 'F',
	KERMIT_TYPE_Z = 'Z',
	KERMIT_TYPE_E = 'E',
	KERMIT_TYPE_Q = 'Q',
	KERMIT_TYPE_T = 'T',
};

enum {
	KERMIT_PARAM_MAXLEN = 0x00,
	KERMIT_PARAM_TIME = 0x01,
	KERMIT_PARAM_NPAD = 0x02,
	KERMIT_PARAM_PADC = 0x03,
	KERMIT_PARAM_EOL = 0x04,
	KERMIT_PARAM_QCTL = 0x05,
	KERMIT_PARAM_QBIN = 0x06,
	KERMIT_PARAM_CHKT = 0x07,
	KERMIT_PARAM_REPT = 0x08,
	KERMIT_PARAM_CAPA = 0x09,
	KERMIT_PARAM_WINDO = 0x0A,
	KERMIT_PARAM_MAXL1 = 0x0B,
	KERMIT_PARAM_MAXL2 = 0x0C,
};

#define KERMIT_PARAM_SIZE (KERMIT_PARAM_MAXL2 + 1)

uint8_t kermit_param_def[KERMIT_PARAM_SIZE] = {
	[KERMIT_PARAM_MAXLEN] = kermit_tochar(80),
	[KERMIT_PARAM_TIME] = kermit_tochar(5),
	[KERMIT_PARAM_NPAD] = kermit_tochar(0),
	[KERMIT_PARAM_PADC] = kermit_ctl(0),
	[KERMIT_PARAM_EOL] = kermit_tochar(0x0D),
	[KERMIT_PARAM_QCTL] = '#',
	[KERMIT_PARAM_QBIN] = ' ',
	[KERMIT_PARAM_CHKT] = '1',
	[KERMIT_PARAM_REPT] = ' ',
	[KERMIT_PARAM_CAPA] = kermit_tochar(0),
	[KERMIT_PARAM_WINDO] = kermit_tochar(0),
	[KERMIT_PARAM_MAXL1] = kermit_tochar(0),
	[KERMIT_PARAM_MAXL2] = kermit_tochar(0),
};

enum {
	KERMIT_HDR_MARK = 0x00,
	KERMIT_HDR_LEN  = 0x01,
	KERMIT_HDR_SEQ  = 0x02,
	KERMIT_HDR_TYPE = 0x03,
};

#define KERMIT_MARK_CHAR 0x01 /* SOH */

int kermit_char_is_prefix(uint8_t c)
{
	if ((c >= 0x21 && c <= 0x3E) || (c >= 0x60 && c <= 0x7E)) {
		return 1;
	}
	return 0;
}

int kermit_char_is_control(uint8_t c)
{
	c &= 0b01111111;
	if ((c <= 0x1F) || (c == 0x7F)) {
		return 1;
	}
	return 0;
}

int kermit_char_have_bit8(uint8_t c)
{
	return ((c & 0b10000000) >> 7);
}

/* out bufsize must >= 3 bytes */
int kermit_data_encode(uint8_t *param, uint8_t data, uint8_t *out)
{
	int i = 0;

	if (kermit_char_have_bit8(data)
	    && (kermit_char_is_prefix(param[KERMIT_PARAM_QBIN]))) {
		out[i] = param[KERMIT_PARAM_QBIN];
		i++;
		data &= 0b01111111;
	}

	if (kermit_char_is_control(data)
	    || (data == param[KERMIT_PARAM_QCTL])
	    || (data == param[KERMIT_PARAM_QBIN])) {
		out[i] = param[KERMIT_PARAM_QCTL];
		i++;
		if ((data == param[KERMIT_PARAM_QBIN]) ||
			(data == param[KERMIT_PARAM_QCTL])) {
			out[i] = data;
		} else {
			out[i] = kermit_ctl(data);
		}
		i++;
		return i;
	}

	out[i] = data;
	i++;
	return i;
}

void kermit_make_checksum(uint8_t *out) {
	out++; /* skip MARK */
	uint8_t checksum = 0x0;
	while (*out != 0x0) {
		checksum += *out;
		out++;
	}
	*out = kermit_checksum(checksum);
}

int kermit_make_init(uint8_t *param, int seq, uint8_t *out) {
	out[KERMIT_HDR_MARK] = KERMIT_MARK_CHAR;
	out[KERMIT_HDR_LEN] = kermit_tochar(1 + 1 + KERMIT_PARAM_SIZE + 1);
	out[KERMIT_HDR_SEQ] = kermit_tochar(seq);
	out[KERMIT_HDR_TYPE] = KERMIT_TYPE_S;
	out += 4;
	memcpy(out, param, KERMIT_PARAM_SIZE);
	out += KERMIT_PARAM_SIZE;
	*out = 0x0; /* init checksum */
	out -= KERMIT_PARAM_SIZE;
	out -= 4;
	kermit_make_checksum(out);
	return (kermit_unchar(out[KERMIT_HDR_LEN]) + 2);
}

int kermit_make_data(uint8_t *param, int seq, uint8_t *out,
			uint8_t *data, int datasize, int type) {
	uint8_t *outp = out;
	out[KERMIT_HDR_MARK] = KERMIT_MARK_CHAR;
	out[KERMIT_HDR_LEN] = kermit_tochar(1 + 1 + (datasize << 2) + 1);
	if (kermit_unchar(out[KERMIT_HDR_LEN]) > param[KERMIT_PARAM_MAXLEN]) {
		return -1;
	}
	out[KERMIT_HDR_SEQ] = kermit_tochar(seq);
	out[KERMIT_HDR_TYPE] = type;
	outp += 4;
	uint8_t buf[3];
	int i;
	int es;

	for (i = 0; i < datasize; i++) {
		es = kermit_data_encode(param, data[i], buf);
		memcpy(outp, buf, es);
		outp += es;
	}
	out[KERMIT_HDR_LEN] = kermit_tochar(1+1+(outp-out-4)+1);

	*outp = 0x0; /* init checksum */
	kermit_make_checksum(out);
	return (kermit_unchar(out[KERMIT_HDR_LEN]) + 2);
}

#endif
