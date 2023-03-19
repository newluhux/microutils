#ifndef _TFTP_H_
#define _TFTP_H_

#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

/*
https://datatracker.ietf.org/doc/html/rfc1350
*/

/* tftp port */
#define TFTP_PORT 69

/* max of tftp block size */
#define TFTP_BLOCKSIZE 512
#define TFTP_HDRSIZE   4
#define TFTP_BUFSIZE   (TFTP_HDRSIZE + TFTP_BLOCKSIZE)

/* tftp opcode */
enum {
	TFTP_RRQ = 1,
	TFTP_WRQ = 2,
	TFTP_DATA = 3,
	TFTP_ACK = 4,
	TFTP_ERR = 5,
};

/* tftp errcode */
enum {
	TFTP_ERR_NOSPEC = 0,
	TFTP_ERR_NOFILE = 1,
	TFTP_ERR_ACCESS = 2,
	TFTP_ERR_WRITE = 3,
	TFTP_ERR_OP = 4,
	TFTP_ERR_ID = 5,
	TFTP_ERR_EXIST = 6,
	TFTP_ERR_USER = 7,
};

/* tftp error message */
char *tftp_error[] = {
	"No defined, see error message",
	"File not found.",
	"Access violation.",
	"Disk full or allocation exceeded.",
	"Illegal TFTP operation.",
	"Unknown transfer ID.",
	"File already exists.",
	"No such user.",
};

/* make tftp pkt */
int tftp_make_rwq(unsigned char *buf, uint16_t type, char *filename, char *mode)
{
	int fn_len, mode_len, data_len;
	uint16_t opcode = type;

	fn_len = strlen(filename) + 1;	/* with '\0' */
	mode_len = strlen(mode) + 1;	/* with '\0' */
	data_len = sizeof(type) + fn_len + mode_len;
	if (data_len > TFTP_BUFSIZE)
		return -1;
	opcode = htons(type);
	memcpy(buf, &opcode, sizeof(opcode));
	buf += sizeof(type);
	memcpy(buf, filename, fn_len);
	buf += fn_len;
	memcpy(buf, mode, mode_len);
	buf += mode_len;

	return data_len;
}

int tftp_make_data(unsigned char *buf, uint16_t block,
		   unsigned char *data, int data_size)
{
	uint16_t opcode = htons(TFTP_DATA);
	if (data_size > (int)(TFTP_BUFSIZE - sizeof(opcode) - sizeof(block)))
		return -1;

	memcpy(buf, &opcode, sizeof(opcode));
	buf += sizeof(opcode);
	memcpy(buf, &block, sizeof(block));
	buf += sizeof(block);
	memcpy(buf, data, data_size);
	buf += data_size;

	return (data_size + sizeof(opcode) + sizeof(block));
}

int tftp_make_ack(unsigned char *buf, uint16_t block)
{
	uint16_t opcode = htons(TFTP_ACK);
	memcpy(buf, &opcode, sizeof(opcode));
	buf += sizeof(opcode);
	block = htons(block);
	memcpy(buf, &block, sizeof(block));
	buf += sizeof(block);

	return (sizeof(opcode) + sizeof(block));
}

int tftp_make_err(unsigned char *buf, uint16_t errcode, char *errmsg)
{
	uint16_t opcode = htons(TFTP_ERR);
	char *msg = tftp_error[errcode];
	if (errmsg) {
		msg = errmsg;
	}
	if (errcode > 7)
		return -1;

	int data_len, msg_len;
	msg_len = strlen(msg) + 1;	/* with '\0' */
	data_len = sizeof(opcode) + sizeof(errcode) + msg_len;
	if (data_len > TFTP_BUFSIZE) {
		return -1;
	}

	memcpy(buf, &opcode, sizeof(opcode));
	buf += sizeof(opcode);
	memcpy(buf, &errcode, sizeof(errcode));
	buf += sizeof(errcode);
	memcpy(buf, msg, msg_len);
	buf += msg_len;

	return data_len;
}

#endif
