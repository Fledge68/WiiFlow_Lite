#include <stdio.h>
#include <ogcsys.h>
#include <string.h>

#include "apploader.h"
#include "wdvd.h"
#include "patchcode.h"
#include "disc.h"
#include "videopatch.h"
#include "wip.h"
#include "wbfs.h"
#include "sys.h"
#include "gecko.h"

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(void (**init)(void (*report)(const char *fmt, ...)), int (**main)(), void *(**final)());

/* Constants */
#define APPLDR_OFFSET	0x2440

/* Variables */
static u32 buffer[0x20] ATTRIBUTE_ALIGN(32);

static bool maindolpatches(void *dst, int len, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes);
static bool Remove_001_Protection(void *Address, int Size);
static bool PrinceOfPersiaPatch();

static void __noprint(const char *fmt, ...)
{
}

s32 Apploader_Run(entry_point *entry, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes)
{
	void *dst = NULL;
	int len = 0;
	int offset = 0;
	app_init  appldr_init;
	app_main  appldr_main;
	app_final appldr_final;

	/* Read apploader header */
	s32 ret = WDVD_Read(buffer, 0x20, APPLDR_OFFSET);
	if (ret < 0) return ret;

	/* Calculate apploader length */
	u32 appldr_len = buffer[5] + buffer[6];

	SYS_SetArena1Hi(APPLOADER_END);

	/* Read apploader code */
	// Either you limit memory usage or you don't touch the heap after that, because this is writing at 0x1200000
	ret = WDVD_Read(APPLOADER_START, appldr_len, APPLDR_OFFSET + 0x20);
	if (ret < 0) return ret;

	DCFlushRange(APPLOADER_START, appldr_len);

	/* Set apploader entry function */
	app_entry appldr_entry = (app_entry)buffer[4];

	/* Call apploader entry */
	appldr_entry(&appldr_init, &appldr_main, &appldr_final);

	/* Initialize apploader */
	appldr_init(__noprint);
	
	bool hookpatched = false;

	while (appldr_main(&dst, &len, &offset))
	{
		/* Read data from DVD */
		WDVD_Read(dst, len, (u64)(offset << 2));
		if(maindolpatches(dst, len, vidMode, vmode, vipatch, countryString, patchVidModes))
			hookpatched = true;
	}

	if (hooktype != 0 && !hookpatched)
	{
		gprintf("Error: Could not patch the hook\n");
		gprintf("Ocarina and debugger won't work\n");
	}
	
	PrinceOfPersiaPatch();

	/* Set entry point from apploader */
	*entry = appldr_final();
	
	IOSReloadBlock(IOS_GetVersion());
	*(vu32 *)0x80003140 = *(vu32 *)0x80003188; // IOS Version Check
	*(vu32 *)0x80003180 = *(vu32 *)0x80000000; // Game ID Online Check
	*(vu32 *)0x80003184 = 0x80000000;

	DCFlushRange((void*)0x80000000, 0x3f00);

	return 0;
}

void PatchCountryStrings(void *Address, int Size)
{
	u8 SearchPattern[4] = {0x00, 0x00, 0x00, 0x00};
	u8 PatchData[4] = {0x00, 0x00, 0x00, 0x00};
	u8 *Addr = (u8*)Address;
	int wiiregion = CONF_GetRegion();

	switch (wiiregion)
	{
		case CONF_REGION_JP:
			SearchPattern[0] = 0x00;
			SearchPattern[1] = 'J';
			SearchPattern[2] = 'P';
			break;
		case CONF_REGION_EU:
			SearchPattern[0] = 0x02;
			SearchPattern[1] = 'E';
			SearchPattern[2] = 'U';
			break;
		case CONF_REGION_KR:
			SearchPattern[0] = 0x04;
			SearchPattern[1] = 'K';
			SearchPattern[2] = 'R';
			break;
		case CONF_REGION_CN:
			SearchPattern[0] = 0x05;
			SearchPattern[1] = 'C';
			SearchPattern[2] = 'N';
			break;
		case CONF_REGION_US:
		default:
			SearchPattern[0] = 0x01;
			SearchPattern[1] = 'U';
			SearchPattern[2] = 'S';
	}
	switch (((const u8 *)0x80000000)[3])
	{
		case 'J':
			PatchData[1] = 'J';
			PatchData[2] = 'P';
			break;
		case 'D':
		case 'F':
		case 'P':
		case 'X':
		case 'Y':
			PatchData[1] = 'E';
			PatchData[2] = 'U';
			break;

		case 'E':
		default:
			PatchData[1] = 'U';
			PatchData[2] = 'S';
	}
	while (Size >= 4)
		if (Addr[0] == SearchPattern[0] && Addr[1] == SearchPattern[1] && Addr[2] == SearchPattern[2] && Addr[3] == SearchPattern[3])
		{
			//*Addr = PatchData[0];
			Addr += 1;
			*Addr = PatchData[1];
			Addr += 1;
			*Addr = PatchData[2];
			Addr += 1;
			//*Addr = PatchData[3];
			Addr += 1;
			Size -= 4;
		}
		else
		{
			Addr += 4;
			Size -= 4;
		}
}

static bool PrinceOfPersiaPatch()
{
    if (memcmp("SPX", (char *)0x80000000, 3) == 0 || memcmp("RPW", (char *)0x80000000, 3) == 0)
	{
        u8 *p = (u8 *)0x807AEB6A;
        *p++ = 0x6F;
        *p++ = 0x6A;
        *p++ = 0x7A;
        *p++ = 0x6B;
        p = (u8 *)0x807AEB75;
        *p++ = 0x69;
        *p++ = 0x39;
        *p++ = 0x7C;
        *p++ = 0x7A;
        p = (u8 *)0x807AEB82;
        *p++ = 0x68;
        *p++ = 0x6B;
        *p++ = 0x73;
        *p++ = 0x76;
        p = (u8 *)0x807AEB92;
        *p++ = 0x75;
        *p++ = 0x70;
        *p++ = 0x80;
        *p++ = 0x71;
        p = (u8 *)0x807AEB9D;
        *p++ = 0x6F;
        *p++ = 0x3F;
        *p++ = 0x82;
        *p++ = 0x80;
        return true;
	}
    return false;
}

bool NewSuperMarioBrosPatch(void *Address, int Size)
{
	if (memcmp("SMN", (char *)0x80000000, 3) == 0)
	{
		u8 SearchPattern1[32] = {// PAL
			0x94, 0x21, 0xFF, 0xD0, 0x7C, 0x08, 0x02, 0xA6,
			0x90, 0x01, 0x00, 0x34, 0x39, 0x61, 0x00, 0x30,
			0x48, 0x12, 0xD9, 0x39, 0x7C, 0x7B, 0x1B, 0x78,
			0x7C, 0x9C, 0x23, 0x78, 0x7C, 0xBD, 0x2B, 0x78};
		u8 SearchPattern2[32] = {// NTSC
			0x94, 0x21, 0xFF, 0xD0, 0x7C, 0x08, 0x02, 0xA6,
			0x90, 0x01, 0x00, 0x34, 0x39, 0x61, 0x00, 0x30,
			0x48, 0x12, 0xD7, 0x89, 0x7C, 0x7B, 0x1B, 0x78,
			0x7C, 0x9C, 0x23, 0x78, 0x7C, 0xBD, 0x2B, 0x78};
		u8 PatchData[4] = {0x4E, 0x80, 0x00, 0x20};
	
		void *Addr = Address;
		void *Addr_end = Address+Size;
		while (Addr <= Addr_end-sizeof(SearchPattern1))
		{
			if (  memcmp(Addr, SearchPattern1, sizeof(SearchPattern1))==0
				|| memcmp(Addr, SearchPattern2, sizeof(SearchPattern2))==0)
			{
				memcpy(Addr,PatchData,sizeof(PatchData));
				return true;
			}
			Addr += 4;
		}
	}
	return false;
}

static bool maindolpatches(void *dst, int len, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes)
{
	bool ret = false;

	DCFlushRange(dst, len);

	patchVideoModes(dst, len, vidMode, vmode, patchVidModes);

	if (hooktype != 0) ret = dogamehooks(dst, len, false);
	if (vipatch) vidolpatcher(dst, len);
	if (configbytes[0] != 0xCD) langpatcher(dst, len);
	if (countryString) PatchCountryStrings(dst, len); // Country Patch by WiiPower

	Remove_001_Protection(dst, len);
	
	// NSMB Patch by WiiPower
	NewSuperMarioBrosPatch(dst,len);

	do_wip_code((u8 *) dst, len);
	
	DCFlushRange(dst, len);

	return ret;
}

static bool Remove_001_Protection(void *Address, int Size)
{
	static const u8 SearchPattern[] = {0x40, 0x82, 0x00, 0x0C, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18};
	static const u8 PatchData[] = {0x40, 0x82, 0x00, 0x04, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18};
	u8 *Addr_end = Address + Size;
	u8 *Addr;

	for (Addr = Address; Addr <= Addr_end - sizeof SearchPattern; Addr += 4)
		if (memcmp(Addr, SearchPattern, sizeof SearchPattern) == 0) 
		{
			memcpy(Addr, PatchData, sizeof PatchData);
			return true;
		}
	return false;
}