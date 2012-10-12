
#ifndef _CIOSINFO_H_
#define _CIOSINFO_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _iosinfo_t {
	u32 magicword;					//0x1ee7c105
	u32 magicversion;				// 1
	u32 version;					// Example: 5
	u32 baseios;					// Example: 56
	char name[0x10];				// Example: d2x
	char versionstring[0x10];		// Example: beta2
} iosinfo_t;

typedef struct _IOS_Info {
	u32 Revision;
	u8 Version;
	u8 Type;
	u8 Base;
} IOS_Info;
extern IOS_Info CurrentIOS;
u8 Hermes_shadow_mload(int mload_rev);
void Hermes_Disable_EHC();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
