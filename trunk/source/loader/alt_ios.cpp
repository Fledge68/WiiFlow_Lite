
#include <ogc/machine/processor.h>

#include "alt_ios.h"
#include "cios.h"
#include "disc.h"
#include "nk.h"
#include "sys.h"
#include "wbfs.h"
#include "wdvd.h"
#include "devicemounter/DeviceHandler.hpp"
#include "devicemounter/usbstorage.h"
#include "gecko/gecko.h"
#include "memory/mem2.hpp"
#include "types.h"

// mload from uloader by Hermes
#include "mload.h"
#include "ehcmodule_5.h"
#include "dip_plugin_249.h"
#include "odip_frag.h"
#include "mload_modules.h"

extern "C" {extern u8 currentPartition;}
extern int __Arena2Lo;
u8 use_port1 = 0;

#define HAVE_AHBPROT	((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)

static int load_ehc_module_ex(void)
{
	ehcmodule = ehcmodule_5;
	size_ehcmodule = size_ehcmodule_5;
	dip_plugin = odip_frag;
	size_dip_plugin = size_odip_frag;

	u8 *ehc_cfg = search_for_ehcmodule_cfg((u8 *)ehcmodule, size_ehcmodule);
	if (ehc_cfg)
	{
		ehc_cfg += 12;
		ehc_cfg[0] = use_port1;
		gprintf("EHC Port info = %i\n", ehc_cfg[0]);
		DCFlushRange((void *) (((u32)ehc_cfg[0]) & ~31), 32);
	}
	if(use_port1)   // release port 0 and use port 1
	{
		u32 dat=0;
		u32 addr;

		// get EHCI base registers
		mload_getw((void *) 0x0D040000, &addr);

		addr&=0xff;
		addr+=0x0D040000;

		mload_getw((void *) (addr+0x44), &dat);
		if((dat & 0x2001)==1) 
			mload_setw((void *) (addr+0x44), 0x2000);

		mload_getw((void *) (addr+0x48), &dat);

		if((dat & 0x2000)==0x2000)
			mload_setw((void *) (addr+0x48), 0x1001);
	}
	load_ehc_module();

	return 0;
}

void load_dip_249()
{
	gprintf("Starting mload\n");
	if(mload_init() < 0)
		return;

	gprintf("Loading 249 dip...");
	int ret = mload_module((void *) dip_plugin_249, size_dip_plugin_249);
	gprintf("%d\n", ret);
	mload_close();
}

/* Thanks to postloader for that patch */
#define MEM2_PROT		0x0D8B420A
#define ES_MODULE_START	(u16*)0x939F0000

static const u16 ticket_check[] = {
    0x685B,          // ldr r3,[r3,#4] ; get TMD pointer
    0x22EC, 0x0052,  // movls r2, 0x1D8
    0x189B,          // adds r3, r3, r2; add offset of access rights field in TMD
    0x681B,          // ldr r3, [r3]   ; load access rights (haxxme!)
    0x4698,          // mov r8, r3     ; store it for the DVD video bitcheck later
    0x07DB           // lsls r3, r3, #31; check AHBPROT bit
};

static void PatchAHB()
{
	// Disable memory protection
	write16(MEM2_PROT, 2);

	for(u16 *patchme = ES_MODULE_START; patchme < ES_MODULE_START + 0x4000; patchme++) 
	{
		if(!memcmp(patchme, ticket_check, sizeof(ticket_check))) 
		{
			// write16/uncached poke doesn't work for this. Go figure.
			patchme[4] = 0x23FF; // li r3, 0xFF
			DCFlushRange(patchme + 4, 2);
			break;
		}
	}
}

bool loadIOS(int ios, bool launch_game, bool emu_channel)
{
#ifndef DOLPHIN
	if(neek2o())
	{
		memset(&CurrentIOS, 0, sizeof(IOS_Info));
		CurrentIOS.Version = 254;
		CurrentIOS.Type = IOS_TYPE_D2X;
		CurrentIOS.Base = 254;
		CurrentIOS.Revision = 999;
		DCFlushRange(&CurrentIOS, sizeof(IOS_Info));
		return true;
	}

	Close_Inputs();
	DeviceHandler::Instance()->UnMountAll();
	WDVD_Close();
	USBStorage2_Deinit();
	mload_close();

	gprintf("Reloading into IOS %i from %i...\n", ios, IOS_GetVersion());
	if(HAVE_AHBPROT && ios == 58) //IOS58 with AHBPROT patched out for Homebrew
		PatchAHB();

	ISFS_Deinitialize();
	bool iosOK = IOS_ReloadIOS(ios) == 0;
	ISFS_Initialize();

	IOS_GetCurrentIOSInfo();
	if(CurrentIOS.Type == IOS_TYPE_HERMES)
		load_ehc_module_ex();
	else if(CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision >= 18)
		load_dip_249();

	gprintf("AHBPROT after IOS Reload: %u\n", HAVE_AHBPROT);
	if(!emu_channel)
	{
		if(launch_game)
		{
			DeviceHandler::Instance()->Mount(currentPartition);
			DeviceHandler::Instance()->Open_WBFS(currentPartition);
			Disc_Init();
		}
		else
			Open_Inputs();
	}

	return iosOK;
#else
	return true;
#endif
}
