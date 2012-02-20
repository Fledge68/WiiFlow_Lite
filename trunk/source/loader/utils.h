#ifndef _UTILS_H_
#define _UTILS_H_

#include <gctypes.h>
/* Constants */
#define KB_SIZE		1024.0
#define MB_SIZE		1048576.0
#define GB_SIZE		1073741824.0

#define MAX_FAT_PATH   1024

/* Macros */
#define round_up(x,n)	(-(-(x) & -(n)))

#define ALIGN(x) (((x) + 3) & ~3)
#define ALIGN32(x) (((x) + 31) & ~31)
#define ALIGNED(x) __attribute__((aligned(x)))

#define SMART_FREE(P)		{if(!!P)P.release();}
#define SAFE_FREE(P)		{if(P != NULL){free(P);P = NULL;}}
#define MEM2_SAFE_FREE(P)	{if(P){MEM2_free(P);P = NULL;}}
#define SAFE_DELETE(P)		{if(P != NULL){delete P;P = NULL;}}
#define SAFE_CLOSE(P)		{if(P != NULL){fclose(P);P = NULL; }}

#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
#define TITLE_UPPER(x)		((u32)((x) >> 32))
#define TITLE_LOWER(x)		((u32)(x) & 0xFFFFFFFF)

/* Macros */
#define Write8(addr, val) *(u8 *)addr = val; DCFlushRange((void *)addr, sizeof(u8));
#define Write16(addr, val) *(u16 *)addr = val; DCFlushRange((void *)addr, sizeof(u16));
#define Write32(addr, val) *(u32 *)addr = val; DCFlushRange((void *)addr, sizeof(u32));

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* Prototypes */
u64 le64(u64);
u32 le32(u32);
u16 le16(u16);
int makedir(char *newdir);

typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;

bool str_replace(char *str, const char *olds, const char *news, int size);
bool str_replace_all(char *str, const char *olds, const char *news, int size);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif

