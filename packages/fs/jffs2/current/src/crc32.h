#ifndef CRC32_H
#define CRC32_H

/* $Id$ */

#ifdef __ECOS
#include "jffs2port.h"
#else
#include <linux/types.h>
#endif

extern const uint32_t crc32_table[256];

/* Return a 32-bit CRC of the contents of the buffer. */

static inline uint32_t 
crc32(uint32_t val, const void *ss, int len)
{
	return cyg_crc32_accumulate(val,(void *)ss,len);
}

#endif
