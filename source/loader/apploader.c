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
#include "fst.h"

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(void (**init)(void (*report)(const char *fmt, ...)), int (**main)(), void *(**final)());

/* Apploader pointers */
static u8 *appldr = (u8 *) 0x81200000;

/* Constants */
#define APPLDR_OFFSET	0x2440

/* Variables */
static u32 buffer[0x20] ATTRIBUTE_ALIGN(32);

void maindolpatches(void *dst, int len, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio);
void PatchCountryStrings(void *Address, int Size);
bool Remove_001_Protection(void *Address, int Size);
bool PrinceOfPersiaPatch();
bool NewSuperMarioBrosPatch();
bool hookpatched = false;

s32 Apploader_Run(entry_point *entry, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio)
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
	if (ret < 0)
		return ret;

	/* Calculate apploader length */
	appldr_len = buffer[5] + buffer[6];

	/* Read apploader code */
	ret = WDVD_Read(appldr, appldr_len, APPLDR_OFFSET + 0x20);
	if (ret < 0)
		return ret;

	/* Set apploader entry function */
	app_entry appldr_entry = (app_entry)buffer[4];

	/* Call apploader entry */
	appldr_entry(&appldr_init, &appldr_main, &appldr_final);

	/* Initialize apploader */
	appldr_init(gprintf);

	while (appldr_main(&dst, &len, &offset))
	{
		/* Read data from DVD */
		WDVD_Read(dst, len, (u64)(offset << 2));
		maindolpatches(dst, len, vidMode, vmode, vipatch, countryString, patchVidModes, aspectRatio);
	}

	free_wip();
	if (hooktype != 0)
	{
		if(hookpatched)
			ocarina_do_code();
		else
			gprintf("Error: Could not patch the hook, Ocarina and debugger won't work\n");
	}

	/* Set entry point from apploader */
	*entry = appldr_final();

	/* ERROR 002 fix (WiiPower) */
	*(u32 *)0x80003140 = *(u32 *)0x80003188;

	DCFlushRange((void*)0x80000000, 0x3f00);

	return 0;
}

void maindolpatches(void *dst, int len, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio)
{
	PrinceOfPersiaPatch();
	NewSuperMarioBrosPatch();

	patchVideoModes(dst, len, vidMode, vmode, patchVidModes);

	if(hooktype != 0 && dogamehooks(dst, len, false))
		hookpatched = true;
	if(vipatch)
		vidolpatcher(dst, len);
	if(configbytes[0] != 0xCD)
		langpatcher(dst, len);
	if(countryString)
		PatchCountryStrings(dst, len); // Country Patch by WiiPower
	if(aspectRatio != -1)
		PatchAspectRatio(dst, len, aspectRatio);

	Remove_001_Protection(dst, len);

	do_wip_code((u8 *)dst, len);

	DCFlushRange(dst, len);
	ICInvalidateRange(dst, len);
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

bool PrinceOfPersiaPatch()
{
	if (memcmp("SPX", (char *) 0x80000000, 3) != 0 && memcmp("RPW", (char *) 0x80000000, 3) != 0)
		return false;

	WIP_Code * CodeList = MEM2_alloc(5 * sizeof(WIP_Code));
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
		MEM2_free(CodeList);
		CodeList = NULL;
		return false;
	}

	return true;
}

bool NewSuperMarioBrosPatch()
{
	WIP_Code * CodeList = NULL;

	if (memcmp("SMNE01", (char *) 0x80000000, 6) == 0)
	{
		CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
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
		CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
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
		CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
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
		MEM2_free(CodeList);
		CodeList = NULL;
		return false;
	}
	return CodeList != NULL;
}

bool Remove_001_Protection(void *Address, int Size)
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
