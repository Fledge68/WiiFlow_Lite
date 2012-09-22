
#include <ogc/machine/processor.h>

#include "alt_ios.h"
#include "cios.h"
#include "disc.h"
#include "nk.h"
#include "sys.h"
#include "wbfs.h"
#include "wdvd.h"
#include "external_booter.hpp"
#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "devicemounter/sdhc.h"
#include "devicemounter/usbstorage.h"
#include "gecko/gecko.h"
#include "memory/mem2.hpp"
#include "memory/memory.h"
#include "types.h"

// mload from uloader by Hermes
#include "mload.h"
#include "ehcmodule_5.h"
#include "dip_plugin_249.h"
#include "odip_frag.h"
#include "mload_modules.h"

extern "C" { extern u8 currentPartition; }
u8 use_port1 = 0;

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

bool loadIOS(int ios, bool MountDevices)
{
	int CurIOS = IOS_GetVersion();
	bool ret = true;

	if(ios != CurIOS)
	{
		WDVD_Close();
		Close_Inputs();
		gprintf("Reloading into IOS %i from %i...\n", ios, CurIOS);
		ShutdownBeforeExit();
		ret = IOS_ReloadIOS(ios) == 0;
		Nand::Instance()->Init_ISFS();
		gprintf("AHBPROT after IOS Reload: %u\n", AHBRPOT_Patched());
		WDVD_Init();
	}

	IOS_GetCurrentIOSInfo();
	if(CurrentIOS.Type == IOS_TYPE_HERMES)
		load_ehc_module_ex();
	else if(CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision >= 18)
		load_dip_249();
	DeviceHandle.SetModes();
	if(MountDevices && ios != CurIOS)
		DeviceHandle.MountAll();

	return ret;
}
