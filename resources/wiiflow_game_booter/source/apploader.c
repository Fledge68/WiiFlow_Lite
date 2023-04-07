
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
#include "gecko.h"
#include "memory.h"
#include "video_tinyload.h"
#include "kirbypatch.h"

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(void (**init)(void (*report)(const char *fmt, ...)), int (**main)(), void *(**final)());

/* pointers */
static u8 *appldr = (u8*)0x81200000;
static const char *GameID = (const char*)0x80000000;

/* Constants */
#define APPLDR_OFFSET	0x910// usblgx uses 0x2440 -huh?
#define APPLDR_CODE		0x918// usblgx uses APPLDR_OFFSET + 0x20 - huh?

void maindolpatches(void *dst, int len, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, 
					u32 returnTo, bool patchregion, u8 private_server, const char *server_addr, u8 deflicker, u8 bootType);
static void patch_NoDiscinDrive(void *buffer, u32 len);
static void Anti_002_fix(void *Address, int Size);
static bool Remove_001_Protection(void *Address, int Size);
static void PrinceOfPersiaPatch();
static void NewSuperMarioBrosPatch();
static void Patch_23400_and_MKWii_vulnerability();
bool hookpatched = false;

// wiiflow uses a struct to hold the appldr hdr and usblgx uses a u32 buffer[32] array to hold it.
/* Thanks Tinyload */
static struct
{
	char revision[16];//[0-3] 16 divided by 4 bytes = 4
	void *entry;// [4] 4 bytes
	s32 size;//[5] 4 bytes
	s32 trailersize;// [6] 4 bytes
	s32 padding;// [7] 4 bytes
} apploader_hdr ATTRIBUTE_ALIGN(32);// 16+4+4+4+4=32 bytes

u32 Apploader_Run(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, u32 returnTo, 
					bool patchregion , u8 private_server, const char *server_addr, u8 videoWidth, bool patchFix480p, u8 deflicker, u8 bootType)
{
	//! Disable private server for games that still have official servers.
	if(memcmp(GameID, "SC7", 3) == 0 || memcmp(GameID, "RJA", 3) == 0 ||
		memcmp(GameID, "SM8", 3) == 0 || memcmp(GameID, "SZB", 3) == 0 || memcmp(GameID, "R9J", 3) == 0)
	{
		private_server = PRIVSERV_OFF; // Private server patching causes error 20100
	}
	
	// if either of these 2 games - adds internal wip codes before do_wip_code() is called in maindolpatches()
	// note: using external .wip codes for these games will prevent their internal codes.
	PrinceOfPersiaPatch();
	NewSuperMarioBrosPatch();

	void *dst = NULL;
	int len = 0;
	int offset = 0;
	u32 appldr_len;
	s32 ret;

	app_entry appldr_entry;
	app_init  appldr_init;
	app_main  appldr_main;
	app_final appldr_final;

	/* Read apploader header */
	ret = WDVD_Read(&apploader_hdr, 0x20, APPLDR_OFFSET);
	if(ret < 0)
		return 0;

	/* Calculate apploader length */
	appldr_len = apploader_hdr.size + apploader_hdr.trailersize;

	/* Read apploader code */
	ret = WDVD_Read(appldr, appldr_len, APPLDR_CODE);
	if(ret < 0)
		return 0;

	/* Flush into memory */
	DCFlushRange(appldr, appldr_len);
	ICInvalidateRange(appldr, appldr_len);

	/* Set apploader entry function */
	appldr_entry = apploader_hdr.entry;

	/* Call apploader entry */
	appldr_entry(&appldr_init, &appldr_main, &appldr_final);

	/* Initialize apploader */
	appldr_init(gprintf);

	while(appldr_main(&dst, &len, &offset))
	{
		/* Read data from DVD */
		WDVD_Read(dst, len, offset);
		// if server is wiimmfi and game is mario kart wii don't patch private server here, do_new_wiimfi() patches it below.
		if(private_server == PRIVSERV_WIIMMFI && memcmp("RMC", GameID, 3) == 0)
			maindolpatches(dst, len, vidMode, vmode, vipatch, countryString, patchVidModes, aspectRatio, returnTo, patchregion, 
							0, NULL, deflicker, bootType);
		else
			maindolpatches(dst, len, vidMode, vmode, vipatch, countryString, patchVidModes, aspectRatio, returnTo, patchregion, 
							private_server,  server_addr, deflicker, bootType);
			
		DCFlushRange(dst, len);
		ICInvalidateRange(dst, len);
		prog(20);
	}
	free_wip();

	patch_kirby((u8 *)0x80000000);// can't be done during maindolpatches.
	
	if(videoWidth == WIDTH_FRAMEBUFFER)
		patch_width((void*)0x80000000, 0x900000);
	
	if(hooktype != 0 && hookpatched)
		ocarina_do_code();
	
	//! Apply the 480p fix.
	//! This needs to be done after the call to maindolpatches(), after loading any code handler.
	//! Can (and should) be done before Wiimmfi patching, can't be done in maindolpatches() itself.
	if(patchFix480p)
		PatchFix480p();

	//! If we're NOT on Wiimmfi, patch the known Remote Code Execution (RCE) vulnerability in MKWii. 
	//! Wiimmfi will handle that on its own through the update payload.
	//! This will also patch error 23400 for a couple games that still have official servers.
	if(private_server != PRIVSERV_WIIMMFI)
		Patch_23400_and_MKWii_vulnerability();
	
	else //PRIVSERV_WIIMMFI
	{
		if(memcmp("RMC", GameID, 3) != 0)// This isn't MKWii, perform the patch for other games.
			do_new_wiimmfi_nonMKWii();// does not patch the server address - done in maindolpatches()
		else // This is MKWii, perform the known patch from 2018.
			do_new_wiimmfi();// includes patching the server address
	}

	/* Set entry point from apploader */
	return (u32)appldr_final();
}

void maindolpatches(void *dst, int len, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, 
					u32 returnTo, bool patchregion , u8 private_server, const char *serverAddr, u8 deflicker, u8 bootType)
{
	u8 vfilter_off[7] = {0, 0, 21, 22, 21, 0, 0};
	u8 vfilter_low[7] = {4, 4, 16, 16, 16, 4, 4};
	u8 vfilter_medium[7] = {4, 8, 12, 16, 12, 8, 4};
	u8 vfilter_high[7] = {8, 8, 10, 12, 10, 8, 8};

	patchVideoModes(dst, len, vidMode, vmode, patchVidModes, bootType);

	if(debuggerselect == 2)
		Patch_fwrite(dst, len);
	if(hooktype != 0 && hookpatched == false)
		hookpatched = dogamehooks(dst, len, false);
	if(vipatch)
		vidolpatcher(dst, len);
	if(patchVidModes > 0)
		PatchVideoSneek(dst, len);
	if(configbytes[0] != 0xCD)
		langpatcher(dst, len);
	if(countryString)
		PatchCountryStrings(dst, len); // Country Patch by WiiPower
	do_wip_code((u8 *)dst, len);
	Remove_001_Protection(dst, len);
	if(CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision < 13)
		Anti_002_fix(dst, len);
	if((CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision < 13) || CurrentIOS.Type == IOS_TYPE_HERMES)
		patch_NoDiscinDrive(dst, len);
	if(patchregion)
		PatchRegion(dst, len);
	if(deflicker == DEFLICKER_ON_LOW)
	{
		patch_vfilters(dst, len, vfilter_low);
		patch_vfilters_rogue(dst, len, vfilter_low);
	}
	else if(deflicker == DEFLICKER_ON_MEDIUM)
	{
		patch_vfilters(dst, len, vfilter_medium);
		patch_vfilters_rogue(dst, len, vfilter_medium);
	}
	else if(deflicker == DEFLICKER_ON_HIGH)
	{
		patch_vfilters(dst, len, vfilter_high);
		patch_vfilters_rogue(dst, len, vfilter_high);
	}
	else if(deflicker != DEFLICKER_NORMAL) // Either safe or extended
	{
		patch_vfilters(dst, len, vfilter_off);
		patch_vfilters_rogue(dst, len, vfilter_off);
		// This might break fade and brightness effects
		if (deflicker == DEFLICKER_OFF_EXTENDED)
			deflicker_patch(dst, len);
	}
	if(returnTo)
		PatchReturnTo(dst, len, returnTo);
	if(aspectRatio != -1)
		PatchAspectRatio(dst, len, aspectRatio);
	if(private_server)
		PrivateServerPatcher(dst, len, private_server, serverAddr);
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

static void PrinceOfPersiaPatch()// Prince of Persia: The Forgotten Sands
{
	if(memcmp("SPX", GameID, 3) != 0 && memcmp("RPW", GameID, 3) != 0)
		return;

	WIP_Code CodeList[5];
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
}

static void NewSuperMarioBrosPatch()
{
	WIP_Code CodeList[3];
	if(memcmp("SMNE01", GameID, 6) == 0)
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
	}
	else if(memcmp("SMNP01", GameID, 6) == 0)
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
	}
	else if(memcmp("SMNJ01", GameID, 6) == 0)
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
	}
}

static bool Remove_001_Protection(void *Address, int Size)
{
	static const u8 SearchPattern[] = {0x40, 0x82, 0x00, 0x0C, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18};
	static const u8 PatchData[] = {0x40, 0x82, 0x00, 0x04, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18};
	u8 *Addr_end = Address + Size;
	u8 *Addr;

	for(Addr = Address; Addr <= Addr_end - sizeof SearchPattern; Addr += 4)
	{
		if(memcmp(Addr, SearchPattern, sizeof SearchPattern) == 0) 
		{
			memcpy(Addr, PatchData, sizeof PatchData);
			return true;
		}
	}
	return false;
}

static void Patch_23400_and_MKWii_vulnerability()
{
	// Thanks to Seeky for the MKWii gecko codes
	// Thanks to InvoxiPlayGames for the gecko codes for the 23400 fix.
	// Reimplemented by Leseratte without the need for a code handler.

	u32 * patch_addr = 0;
	char * patched = 0; 

	// Patch error 23400 for CoD (Black Ops, Reflex, MW3) and Rock Band 3 / The Beatles

	if (memcmp(GameID, "SC7", 3) == 0) 
	{
		gprintf("Patching error 23400 for game %s\n", GameID);
		*(u32 *)0x8023c954 = 0x41414141;
	}

	else if (memcmp(GameID, "RJA", 3) == 0) 
	{
		gprintf("Patching error 23400 for game %s\n", GameID);
		*(u32 *)0x801b838c = 0x41414141;
	}

	else if (memcmp(GameID, "SM8", 3) == 0) 
	{
		gprintf("Patching error 23400 for game %s\n", GameID);
		*(u32 *)0x80238c74 = 0x41414141;
	}

	else if (memcmp(GameID, "SZB", 3) == 0) 
	{
		gprintf("Patching error 23400 for game %s\n", GameID);
		*(u32 *)0x808e3b20 = 0x41414141;
	}

	else if (memcmp(GameID, "R9J", 3) == 0) 
	{
		gprintf("Patching error 23400 for game %s\n", GameID);
		*(u32 *)0x808d6934 = 0x41414141;
	}

	// Patch RCE vulnerability in MKWii.
	else if (memcmp(GameID, "RMC", 3) == 0) 
	{
		switch (GameID[3]) {

			case 'P':
				patched = (char *)0x80276054;
				patch_addr = (u32 *)0x8089a194;
				break; 
			
			case 'E': 
				patched = (char *)0x80271d14;
				patch_addr = (u32 *)0x80895ac4;
				break; 

			case 'J': 
				patched = (char *)0x802759f4;
				patch_addr = (u32 *)0x808992f4;
				break;
			
			case 'K': 
				patched = (char *)0x80263E34; 
				patch_addr = (u32 *)0x808885cc; 
				break; 

			default:
				gprintf("NOT patching RCE vulnerability due to invalid game ID: %s\n", GameID);
				return;
		}

		if (*patched != '*')
		{
			gprintf("Game is already Wiimmfi-patched, don't apply the RCE fix\n");
		}
		else
		{
			gprintf("Patching RCE vulnerability for game ID %s\n", GameID);

			for (int i = 0; i < 7; i++)
				*patch_addr++ = 0xff; 
		}
	}
}
