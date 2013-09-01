
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

static inline void *wbfs_malloc(size_t size)
{
	void *p = MEM2_memalign(32, size);
	if(p) memset(p, 0, size);
	return p;
}

static inline void wbfs_free(void *ptr)
{
	MEM2_free(ptr);
}

#define wbfs_ioalloc(x)		wbfs_malloc(x)
#define wbfs_iofree(x)		wbfs_free(x)

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
