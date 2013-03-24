
#include <ogc/machine/processor.h>

#include "alt_ios.h"
#include "cios.h"
#include "disc.h"
#include "nk.h"
#include "sys.h"
#include "wbfs.h"
#include "wdvd.h"
#include "booter/external_booter.hpp"
#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "devicemounter/sdhc.h"
#include "devicemounter/usbstorage.h"
#include "gecko/gecko.hpp"
#include "memory/mem2.hpp"
#include "memory/memory.h"
#include "types.h"

// mload from uloader by Hermes
#include "mload.h"
#include "ehcmodule_5.h"
#include "dip_plugin_249.h"
#include "mload_modules.h"

static int load_ehc_module_ex(void)
{
	u8 *ehc_cfg = search_for_ehcmodule_cfg((u8 *)ehcmodule_5, size_ehcmodule_5);
	if(ehc_cfg)
	{
		ehc_cfg += 12;
		ehc_cfg[0] = currentPort;
		gprintf("EHC Port info = %i\n", ehc_cfg[0]);
		DCFlushRange((void *) (((u32)ehc_cfg[0]) & ~31), 32);
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
	int ret = mload_module((void *)dip_plugin_249, size_dip_plugin_249);
	gprintf("%d\n", ret);
	mload_close();
}

bool loadIOS(int ios, bool MountDevices)
{
	int CurIOS = IOS_GetVersion();
	bool ret = true;

	if(ios != CurIOS && IOS_GetType(ios) != IOS_TYPE_STUB)
	{
		WDVD_Close();
		gprintf("Reloading into IOS %i from %i...\n", ios, CurIOS);
		ShutdownBeforeExit();
		NandHandle.Patch_AHB(); //No AHBPROT for the next IOS
		ret = IOS_ReloadIOS(ios) == 0;
		gprintf("AHBPROT after IOS Reload: %u\n", AHBRPOT_Patched());
		NandHandle.Init_ISFS();
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
