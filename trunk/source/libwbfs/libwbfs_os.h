
#ifndef LIBWBFS_GLUE_H
#define LIBWBFS_GLUE_H

#include <gctypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gccore.h>
#include <malloc.h>

#include "gecko/gecko.hpp"
#include "loader/disc.h"
#include "loader/utils.h"
#include "memory/mem2.hpp"

#define wbfs_fatal(x)		do { gprintf(x); wd_last_error = 1; } while(0)
#define wbfs_error(x)		do { gprintf(x); wd_last_error = 2; } while(0)

/* Thanks to cfg-loader */
#define wbfs_malloc(x)		calloc(x, 1)
#define wbfs_free(x)		free(x)

static inline void *wbfs_ioalloc(size_t x)
{
	void *p = memalign(32, x);
	if(p)
		memset(p, 0, x);
	return p;
}
#define wbfs_iofree(x)		free(x)

#define wbfs_be16(x)		(*((u16*)(x)))
#define wbfs_be32(x)		(*((u32*)(x)))
#define wbfs_ntohl(x)		(x)
#define wbfs_htonl(x)		(x)
#define wbfs_ntohs(x)		(x)
#define wbfs_htons(x)		(x)

#define wbfs_memcmp(x,y,z)	memcmp(x,y,z)
#define wbfs_memcpy(x,y,z)	memcpy(x,y,z)
#define wbfs_memset(x,y,z)	memset(x,y,z)

#endif
