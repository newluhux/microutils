#ifndef _BIT_OPS_
#define _BIT_OPS_

/*
 * Bitops Module
 *
 * Copyright (C) 2010 Corentin Chary <corentin.chary@gmail.com>
 *
 * Mostly inspired by (stolen from) linux/bitmap.h and linux/bitops.h
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 */


#include <assert.h>

static inline uint32_t extract32(uint32_t value, int start, int length)
{
	assert(start >= 0 && length > 0 && length <= 32 - start);
	return (value >> start) & (~0U >> (32 - length));
}

#endif
