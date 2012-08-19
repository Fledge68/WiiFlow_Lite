
#ifndef _UTILS_H_
#define _UTILS_H_

#include <gctypes.h>
#include "defines.h"

#define KB_SIZE				1024.0
#define MB_SIZE				1048576.0
#define GB_SIZE				1073741824.0

#define MAX_FAT_PATH		1024

#define round_up(x,n)		(-(-(x) & -(n)))

#define ALIGN(n, x)			(((x) + (n - 1)) & ~(n - 1))
#define ALIGN32(x)			(((x) + 31) & ~31)

#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
#define TITLE_UPPER(x)		((u32)((x) >> 32))
#define TITLE_LOWER(x)		((u32)(x) & 0xFFFFFFFF)

#define Write8(addr, val)	*(u8 *)addr = val; DCFlushRange((void *)addr, sizeof(u8));
#define Write16(addr, val)	*(u16 *)addr = val; DCFlushRange((void *)addr, sizeof(u16));
#define Write32(addr, val)	*(u32 *)addr = val; DCFlushRange((void *)addr, sizeof(u32));

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* Prototypes */
u64 le64(u64);
u32 le32(u32);
u16 le16(u16);

bool str_replace(char *str, const char *olds, const char *news, int size);
bool str_replace_all(char *str, const char *olds, const char *news, int size);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif
