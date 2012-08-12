
#include <stdio.h>
#include <ogcsys.h>
#include <string.h>
#include <malloc.h>
#include "apploader.h"
#include "wdvd.h"
#include "patchcode.h"
#include "videopatch.h"
#include "cios.h"
#include "fst.h"
#include "wip.h"

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(void (**init)(void (*report)(const char *fmt, ...)), int (**main)(), void *(**final)());

/* Apploader pointers */
static u8 *appldr = (u8 *)0x81200000;

/* Constants */
#define APPLDR_OFFSET	0x2440

/* Variables */
static u32 buffer[0x20] ATTRIBUTE_ALIGN(32);

void maindolpatches(void *dst, int len, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, u32 returnTo);
static void patch_NoDiscinDrive(void *buffer, u32 len);
static void Anti_002_fix(void *Address, int Size);
static bool Remove_001_Protection(void *Address, int Size);
static bool PrinceOfPersiaPatch();
static bool NewSuperMarioBrosPatch();
bool hookpatched = false;

static void no_print(const char*a __attribute__((unused)),...){}

s32 Apploader_Run(entry_point *entry, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, u32 returnTo)
{
	void *dst = NULL;
	int len = 0;
	int offset = 0;
	u32 appldr_len;
	s32 ret;
	app_init  appldr_init;
	app_main  appldr_main;
	app_final appldr_final;

	/* Read apploader header */
	ret = WDVD_Read(buffer, 0x20, APPLDR_OFFSET);
	if(ret < 0)
		return ret;

	/* Calculate apploader length */
	appldr_len = buffer[5] + buffer[6];

	/* Read apploader code */
	ret = WDVD_Read(appldr, appldr_len, APPLDR_OFFSET + 0x20);
	if(ret < 0)
		return ret;

	/* Flush into memory */
	DCFlushRange(appldr, appldr_len);
	ICInvalidateRange(appldr, appldr_len);

	/* Set apploader entry function */
	app_entry appldr_entry = (app_entry)buffer[4];

	/* Call apploader entry */
	appldr_entry(&appldr_init, &appldr_main, &appldr_final);

	/* Initialize apploader */
	appldr_init(no_print);

	while(appldr_main(&dst, &len, &offset))
	{
		/* Read data from DVD */
		WDVD_Read(dst, len, (u64)(offset << 2));
		maindolpatches(dst, len, vidMode, vmode, vipatch, countryString, patchVidModes, aspectRatio, returnTo);
	}

	free_wip();
	if(hooktype != 0 && hookpatched)
		ocarina_do_code();

	/* Set entry point from apploader */
	*entry = appldr_final();

	/* ERROR 002 fix (WiiPower) */
	*(u32 *)0x80003140 = *(u32 *)0x80003188;
	DCFlushRange((void*)0x80000000, 0x3f00);

	return 0;
}

void maindolpatches(void *dst, int len, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, u32 returnTo)
{
	PrinceOfPersiaPatch();
	NewSuperMarioBrosPatch();
	// Patch NoDiscInDrive only for IOS 249 < rev13 or IOS 222/223/224
	if((CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision < 13) || CurrentIOS.Type == IOS_TYPE_HERMES)
		patch_NoDiscinDrive(dst, len);
	patchVideoModes(dst, len, vidMode, vmode, patchVidModes);
	if(hooktype != 0 && dogamehooks(dst, len, false))
		hookpatched = true;
	if(vipatch)
		vidolpatcher(dst, len);
	if(configbytes[0] != 0xCD)
		langpatcher(dst, len);
	if(CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision < 13)
		Anti_002_fix(dst, len);
	if(countryString)
		PatchCountryStrings(dst, len); // Country Patch by WiiPower
	if(aspectRatio != -1)
		PatchAspectRatio(dst, len, aspectRatio);
	if(returnTo)
		PatchReturnTo(dst, len, returnTo);

	Remove_001_Protection(dst, len);

	do_wip_code((u8 *)dst, len);

	DCFlushRange(dst, len);
	ICInvalidateRange(dst, len);
}

static void patch_NoDiscinDrive(void *buffer, u32 len)
{
	static const u8 oldcode[] = {0x54, 0x60, 0xF7, 0xFF, 0x40, 0x82, 0x00, 0x0C, 0x54, 0x60, 0x07, 0xFF, 0x41, 0x82, 0x00, 0x0C};
	static const u8 newcode[] = {0x54, 0x60, 0xF7, 0xFF, 0x40, 0x82, 0x00, 0x0C, 0x54, 0x60, 0x07, 0xFF, 0x48, 0x00, 0x00, 0x0C};
	u32 n;

/* Patch cover register */
	for(n = 0; n < len - sizeof oldcode; n += 4) // n is not 4 aligned here, so you can get an out of buffer thing
	{
		if (memcmp(buffer + n, (void *)oldcode, sizeof oldcode) == 0)
			memcpy(buffer + n, (void *)newcode, sizeof newcode);
	}
}

static void Anti_002_fix(void *Address, int Size)
{
	static const u8 SearchPattern[] = {0x2C, 0x00, 0x00, 0x00, 0x48, 0x00, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00};
	static const u8 PatchData[] =   {0x2C, 0x00, 0x00, 0x00, 0x40, 0x82, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00};
	void *Addr = Address;
	void *Addr_end = Address + Size;

	while(Addr <= Addr_end - sizeof SearchPattern)
	{
		if(memcmp(Addr, SearchPattern, sizeof SearchPattern) == 0) 
			memcpy(Addr, PatchData, sizeof PatchData);
		Addr += 4;
	}
}

static bool PrinceOfPersiaPatch()
{
	if (memcmp("SPX", (char *) 0x80000000, 3) != 0 && memcmp("RPW", (char *) 0x80000000, 3) != 0)
		return false;

	WIP_Code * CodeList = malloc(5 * sizeof(WIP_Code));
	CodeList[0].offset = 0x007AAC6A;
	CodeList[0].srcaddress = 0x7A6B6F6A;
	CodeList[0].dstaddress = 0x6F6A7A6B;
	CodeList[1].offset = 0x007AAC75;
	CodeList[1].srcaddress = 0x7C7A6939;
	CodeList[1].dstaddress = 0x69397C7A;
	CodeList[2].offset = 0x007AAC82;
	CodeList[2].srcaddress = 0x7376686B;
	CodeList[2].dstaddress = 0x686B7376;
	CodeList[3].offset = 0x007AAC92;
	CodeList[3].srcaddress = 0x80717570;
	CodeList[3].dstaddress = 0x75708071;
	CodeList[4].offset = 0x007AAC9D;
	CodeList[4].srcaddress = 0x82806F3F;
	CodeList[4].dstaddress = 0x6F3F8280;

	if (set_wip_list(CodeList, 5) == false)
	{
		free(CodeList);
		CodeList = NULL;
		return false;
	}

	return true;
}

static bool NewSuperMarioBrosPatch()
{
	WIP_Code * CodeList = NULL;

	if (memcmp("SMNE01", (char *) 0x80000000, 6) == 0)
	{
		CodeList = malloc(3 * sizeof(WIP_Code));
		if(!CodeList)
			return false;
		CodeList[0].offset = 0x001AB610;
		CodeList[0].srcaddress = 0x9421FFD0;
		CodeList[0].dstaddress = 0x4E800020;
		CodeList[1].offset = 0x001CED53;
		CodeList[1].srcaddress = 0xDA000000;
		CodeList[1].dstaddress = 0x71000000;
		CodeList[2].offset = 0x001CED6B;
		CodeList[2].srcaddress = 0xDA000000;
		CodeList[2].dstaddress = 0x71000000;
	}
	else if (memcmp("SMNP01", (char *) 0x80000000, 6) == 0)
	{
		CodeList = malloc(3 * sizeof(WIP_Code));
		if(!CodeList)
			return false;
		CodeList[0].offset = 0x001AB750;
		CodeList[0].srcaddress = 0x9421FFD0;
		CodeList[0].dstaddress = 0x4E800020;
		CodeList[1].offset = 0x001CEE90;
		CodeList[1].srcaddress = 0x38A000DA;
		CodeList[1].dstaddress = 0x38A00071;
		CodeList[2].offset = 0x001CEEA8;
		CodeList[2].srcaddress = 0x388000DA;
		CodeList[2].dstaddress = 0x38800071;
	}
	else if (memcmp("SMNJ01", (char *) 0x80000000, 6) == 0)
	{
		CodeList = malloc(3 * sizeof(WIP_Code));
		if(!CodeList)
			return false;
		CodeList[0].offset = 0x001AB420;
		CodeList[0].srcaddress = 0x9421FFD0;
		CodeList[0].dstaddress = 0x4E800020;
		CodeList[1].offset = 0x001CEB63;
		CodeList[1].srcaddress = 0xDA000000;
		CodeList[1].dstaddress = 0x71000000;
		CodeList[2].offset = 0x001CEB7B;
		CodeList[2].srcaddress = 0xDA000000;
		CodeList[2].dstaddress = 0x71000000;
	}
	if (CodeList && set_wip_list(CodeList, 3) == false)
	{
		free(CodeList);
		CodeList = NULL;
		return false;
	}
	return CodeList != NULL;
}

static bool Remove_001_Protection(void *Address, int Size)
{
	static const u8 SearchPattern[] = {0x40, 0x82, 0x00, 0x0C, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18};
	static const u8 PatchData[] = {0x40, 0x82, 0x00, 0x04, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18};
	u8 *Addr_end = Address + Size;
	u8 *Addr;

	for (Addr = Address; Addr <= Addr_end - sizeof SearchPattern; Addr += 4)
	{
		if (memcmp(Addr, SearchPattern, sizeof SearchPattern) == 0) 
		{
			memcpy(Addr, PatchData, sizeof PatchData);
			return true;
		}
	}
	return false;
}