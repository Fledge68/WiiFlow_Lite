#ifndef _CIOSINFO_H_
#define _CIOSINFO_H_

#include <gccore.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _iosinfo_t
{
	u32 magicword;					//0x1ee7c105
	u32 magicversion;				// 1
	u32 version;					// Example: 5
	u32 baseios;					// Example: 56
	char name[0x10];				// Example: d2x
	char versionstring[0x10];		// Example: beta2
} __attribute__((packed)) iosinfo_t;

bool neek2o(void);
bool D2X(u8 ios, u8 *base);
iosinfo_t *GetInfo(u8 ios);
signed_blob *GetTMD(u8 ios, u32 *TMD_Length);
int is_ios_type(int type, u8 slot);
int get_ios_type(u8 slot);
bool shadow_mload();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif