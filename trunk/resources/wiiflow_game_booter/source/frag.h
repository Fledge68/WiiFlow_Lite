
// worst case wbfs fragmentation scenario:
// 9GB (dual layer) / 2mb (wbfs sector size) = 4608
#define MAX_FRAG 20000
// max that ehcmodule_frag will allow at the moment is about:
// 40000/4/3-1 = 21844

#ifdef __cplusplus
extern "C" {
#endif

#include "gctypes.h"

typedef struct
{
	u32 offset; // file offset, in sectors unit
	u32 sector;
	u32 count;
} Fragment;

typedef struct
{
	u32 size; // num sectors
	u32 num;  // num fragments
	u32 maxnum;
	Fragment frag[MAX_FRAG];
} FragList;

#ifdef __cplusplus
}
#endif
