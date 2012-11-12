
#ifndef _CIOSINFO_H_
#define _CIOSINFO_H_

#include <gccore.h>
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
	u32 SubRevision;
	u8 Version;
	u8 Type;
	u8 Base;
} IOS_Info;

extern IOS_Info CurrentIOS;
void IOS_GetCurrentIOSInfo();

bool IOS_D2X(u8 ios, u8 *base);
u8 IOS_GetType(u8 slot);

bool Hermes_shadow_mload();
void Hermes_Disable_EHC();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
