
#include <ogc/machine/processor.h>

#include "alt_ios.h"
#include "cios.h"
#include "disc.h"
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

#define HAVE_AHBPROT ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)

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

bool loadIOS(int ios, bool launch_game, bool emu_channel)
{
#ifndef DOLPHIN
	gprintf("Reloading into IOS %i from %i (AHBPROT: %u)...\n", ios, IOS_GetVersion(), HAVE_AHBPROT);

	Close_Inputs();
	DeviceHandler::Instance()->UnMountAll();
	WDVD_Close();
	USBStorage2_Deinit();
	mload_close();

	ISFS_Deinitialize();
	bool iosOK = IOS_ReloadIOS(ios) == 0;
	ISFS_Initialize();

	gprintf("%s, Current IOS: %i\n", iosOK ? "OK" : "FAILED!", IOS_GetVersion());
	if(is_ios_type(IOS_TYPE_HERMES, IOS_GetVersion()))
	{
		load_ehc_module_ex();
		gprintf("Hermes cIOS Base IOS%i\n", get_ios_base());
	}
	else if(is_ios_type(IOS_TYPE_WANIN, IOS_GetVersion()) && IOS_GetRevision() >= 18)
	{
		load_dip_249();
		gprintf("Waninkoko cIOS Base IOS%i\n", get_ios_base());
	}
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

u32 get_ios_base()
{
	u32 revision = IOS_GetRevision();
	if (is_ios_type(IOS_TYPE_WANIN, IOS_GetVersion()) && revision >= 17)
		return wanin_mload_get_IOS_base();
	
	else if (is_ios_type(IOS_TYPE_HERMES, IOS_GetVersion()) && revision >= 4)
		return mload_get_IOS_base();

	return 0;
}
