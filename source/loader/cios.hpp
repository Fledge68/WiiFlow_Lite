#ifndef _CIOSINFO_H_
#define _CIOSINFO_H_

#include <gccore.h>

typedef struct _iosinfo_t
{
	u32 magicword;					//0x1ee7c105
	u32 magicversion;				// 1
	u32 version;					// Example: 5
	u32 baseios;					// Example: 56
	char name[0x10];				// Example: d2x
	char versionstring[0x10];		// Example: beta2
} __attribute__((packed)) iosinfo_t;

class cIOSInfo
{
	public:
		static bool D2X(u8 ios, u8 *base);
		static iosinfo_t *GetInfo(u8 ios);
};

#endif