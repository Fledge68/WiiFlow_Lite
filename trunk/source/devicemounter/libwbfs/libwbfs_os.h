
#ifndef LIBWBFS_GLUE_H
#define LIBWBFS_GLUE_H

#include <gctypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "mem2.hpp"
#include "gecko/gecko.h"

#define wbfs_fatal(x)		do { gprintf(x); wd_last_error = 1; } while(0)
#define wbfs_error(x)		do { gprintf(x); wd_last_error = 2; } while(0)

#define wbfs_malloc(x)		MEM2_alloc(x)
#define wbfs_free(x)		MEM2_free(x)

#define wbfs_ioalloc(x)		MEM2_alloc(((x) + 31) & ~31)
#define wbfs_iofree(x)		MEM2_free(x)

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
