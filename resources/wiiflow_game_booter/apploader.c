
#include "apploader.h"
#include "patchcode.h"
#include "cios.h"
#include "fst.h"
#include "wip.h"
#include "debug.h"
#include "utils.h"
#include "di.h"
#include "cache.h"
#include "config.h"
#include "video.h"

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(void (**init)(void (*report)(const char *fmt, ...)), int (**main)(), void *(**final)());

/* Apploader pointers */
static u8 *appldr = (u8 *)0x81200000;

/* Variables */
static u32 buffer[0x20] __attribute__((aligned(32)));

void maindolpatches(void *dst, int len, u8 vidMode, u8 vipatch, u8 countryString, u8 patchVidModes, int aspectRatio, u32 returnTo);
static void patch_NoDiscinDrive(void *buffer, u32 len);
static void Anti_002_fix(void *Address, int Size);
static u8 Remove_001_Protection(void *Address, int Size);
static u8 PrinceOfPersiaPatch();
static u8 NewSuperMarioBrosPatch();
u8 hookpatched = 0;
u8 wip_needed = 0;

static void simple_report(const char *fmt, ...)
{
	debug_string(fmt);
}

u32 Apploader_Run(u8 vidMode, u8 vipatch, u8 countryString, u8 patchVidModes, int aspectRatio, u32 returnTo)
{
	/* Check if WIP patches are needed */
	if(PrinceOfPersiaPatch() == 1)
	{
		debug_string("PoP patch\n");
		wip_needed = 1;
	}
	else if(NewSuperMarioBrosPatch() == 1)
	{
		debug_string("NSMBW patch\n");
		wip_needed = 1;
	}
	else
	{
		debug_string("No WIP patch\n");
		wip_needed = 0;
	}
	/* Variables */
	s32 ret;
	u32 appldr_len;
	app_init  appldr_init;
	app_main  appldr_main;
	app_final appldr_final;

	/* Read apploader header */
	ret = di_read(buffer, 0x20, 0x910);
	if(ret < 0)
		return ret;

	/* Calculate apploader length */
	appldr_len = buffer[5] + buffer[6];

	/* Read apploader code */
	ret = di_read(appldr, appldr_len, 0x918);
	if(ret < 0)
		return ret;

	/* Flush into memory */
	sync_after_write(appldr, appldr_len);

	/* Set apploader entry function */
	app_entry appldr_entry = (app_entry)buffer[4];

	/* Call apploader entry */
	appldr_entry(&appldr_init, &appldr_main, &appldr_final);

	/* Initialize apploader */
	appldr_init(simple_report);

	void *dst = NULL;
	int len = 0;
	int offset = 0;
	while(appldr_main(&dst, &len, &offset))
	{
		/* Read data from DVD */
		di_read(dst, len, offset);
		maindolpatches(dst, len, vidMode, vipatch, countryString, patchVidModes, aspectRatio, returnTo);
		sync_after_write(dst, len);
		prog10();
	}
	if(wip_needed == 1)
		free_wip();
	if(hooktype != 0 && hookpatched == 1)
		ocarina_do_code(0);
	/* Set entry point from apploader */
	return (u32)appldr_final();
}

void maindolpatches(void *dst, int len, u8 vidMode, u8 vipatch, u8 countryString, u8 patchVidModes, int aspectRatio, u32 returnTo)
{
	// Patch NoDiscInDrive only for IOS 249 < rev13 or IOS 222/223/224
	if((CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision < 13) || CurrentIOS.Type == IOS_TYPE_HERMES)
		patch_NoDiscinDrive(dst, len);
	if(CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision < 13)
		Anti_002_fix(dst, len);
	Remove_001_Protection(dst, len);
	//patchVideoModes(dst, len, vidMode, vmode, patchVidModes);
	if(hooktype != 0 && dogamehooks(dst, len, 0))
		hookpatched = 1;
	if(vipatch == 1)
		vidolpatcher(dst, len);
	if(configbytes[0] != 0xCD)
		langpatcher(dst, len);
	//if(conf->countryString)
	//	PatchCountryStrings(dst, len); // Country Patch by WiiPower
	if(aspectRatio != -1)
		PatchAspectRatio(dst, len, aspectRatio);
	if(returnTo > 0)
		PatchReturnTo(dst, len, returnTo);
	if(wip_needed == 1)
		do_wip_code((u8*)dst, len);
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


static u8 PrinceOfPersiaPatch()
{
	if(memcmp("SPX", (char *)0x80000000, 3) != 0 && memcmp("RPW", (char *) 0x80000000, 3) != 0)
		return 0;
	WIP_Code CodeList[5*sizeof(WIP_Code)];
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
	set_wip_list(CodeList, 5);
	return 1;
}

static u8 NewSuperMarioBrosPatch()
{
	WIP_Code CodeList[3 * sizeof(WIP_Code)];

	if(memcmp("SMNE01", (char *) 0x80000000, 6) == 0)
	{
		CodeList[0].offset = 0x001AB610;
		CodeList[0].srcaddress = 0x9421FFD0;
		CodeList[0].dstaddress = 0x4E800020;
		CodeList[1].offset = 0x001CED53;
		CodeList[1].srcaddress = 0xDA000000;
		CodeList[1].dstaddress = 0x71000000;
		CodeList[2].offset = 0x001CED6B;
		CodeList[2].srcaddress = 0xDA000000;
		CodeList[2].dstaddress = 0x71000000;
		set_wip_list(CodeList, 3);
		return 1;
	}
	else if(memcmp("SMNP01", (char *) 0x80000000, 6) == 0)
	{
		CodeList[0].offset = 0x001AB750;
		CodeList[0].srcaddress = 0x9421FFD0;
		CodeList[0].dstaddress = 0x4E800020;
		CodeList[1].offset = 0x001CEE90;
		CodeList[1].srcaddress = 0x38A000DA;
		CodeList[1].dstaddress = 0x38A00071;
		CodeList[2].offset = 0x001CEEA8;
		CodeList[2].srcaddress = 0x388000DA;
		CodeList[2].dstaddress = 0x38800071;
		set_wip_list(CodeList, 3);
		return 1;
	}
	else if(memcmp("SMNJ01", (char *) 0x80000000, 6) == 0)
	{
		CodeList[0].offset = 0x001AB420;
		CodeList[0].srcaddress = 0x9421FFD0;
		CodeList[0].dstaddress = 0x4E800020;
		CodeList[1].offset = 0x001CEB63;
		CodeList[1].srcaddress = 0xDA000000;
		CodeList[1].dstaddress = 0x71000000;
		CodeList[2].offset = 0x001CEB7B;
		CodeList[2].srcaddress = 0xDA000000;
		CodeList[2].dstaddress = 0x71000000;
		set_wip_list(CodeList, 3);
		return 1;
	}
	return 0;
}

static u8 Remove_001_Protection(void *Address, int Size)
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
			return 1;
		}
	}
	return 0;
}
