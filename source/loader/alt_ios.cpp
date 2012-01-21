#include "DeviceHandler.hpp"
#include "wdvd.h"
#include "disc.h"
#include "usbstorage.h"
#include "mem2.hpp"
#include "alt_ios.h"
#include "sys.h"
#include "wbfs.h"

#include <malloc.h>
#include <ogc/machine/processor.h>

#include "gecko.h"

extern "C" {extern u8 currentPartition;}
extern int __Arena2Lo;

#define HAVE_AHBPROT ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)
#define MEM_REG_BASE 0xd8b4000
#define MEM_PROT (MEM_REG_BASE + 0x20a)

static void disable_memory_protection()
{
	gprintf("Disable memory protection...");
	int mem_prot = read32(MEM_PROT);
	gprintf("current value: %08X...", mem_prot);
    write32(MEM_PROT, mem_prot & 0x0000FFFF);
	gprintf("done\n");
}

static u32 apply_patch(const u8 *pattern, u32 pattern_size, const u8 *patch, u32 patch_size, u32 patch_offset)
{
	//gprintf("Applying AHBPROT patch...");
    u8 *ptr_start = (u8*)*((u32*)0x80003134), *ptr_end = (u8*)0x94000000;
    u32 found = 0;
    u8 *location = NULL;
    while (ptr_start < (ptr_end - patch_size))
	{
        if (!memcmp(ptr_start, pattern, pattern_size))
		{
            found++;
            location = ptr_start + patch_offset;
            u8 *start = location;
            u32 i;
            for (i = 0; i < patch_size; i++)
                *location++ = patch[i];

            DCFlushRange((u8 *)(((u32)start) >> 5 << 5), (patch_size >> 5 << 5) + 64);
            ICInvalidateRange((u8 *)(((u32)start) >> 5 << 5), (patch_size >> 5 << 5) + 64);
        }
        ptr_start++;
    }
	//gprintf("done\n");
    return found;
}

const u8 es_set_ahbprot_pattern[] = { 0x68, 0x5B, 0x22, 0xEC, 0x00, 0x52, 0x18, 0x9B, 0x68, 0x1B, 0x46, 0x98, 0x07, 0xDB };
const u8 es_set_ahbprot_patch[]   = { 0x01 };

u32 IOSPATCH_AHBPROT()
{
    if (HAVE_AHBPROT)
	{
        disable_memory_protection();
        return apply_patch((const u8 *) es_set_ahbprot_pattern, sizeof(es_set_ahbprot_pattern), (const u8 *) es_set_ahbprot_patch, sizeof(es_set_ahbprot_patch), 25);
    }
    return 0;
}

bool loadIOS(int ios, bool launch_game)
{
	gprintf("Reloading into IOS %i from %i (AHBPROT: %u)...", ios, IOS_GetVersion(), HAVE_AHBPROT);

	Close_Inputs();
	DeviceHandler::Instance()->UnMountAll();

	WDVD_Close();
	USBStorage_Deinit();

	//gprintf("AHBPROT state before reloading: %s\n", HAVE_AHBPROT ? "enabled" : "disabled");
	//IOSPATCH_AHBPROT();

/* 	void *backup = MEM1_alloc(0x200000);	// 0x126CA0 bytes were needed last time i checked. But take more just in case.
	if (backup != 0)
	{
		memcpy(backup, &__Arena2Lo, 0x200000);
		DCFlushRange(backup, 0x200000);
	} */

	bool iosOK = IOS_ReloadIOS(ios) == 0;

/* 	if (backup != 0)
	{
		memcpy(&__Arena2Lo, backup, 0x200000);
		DCFlushRange(&__Arena2Lo, 0x200000);
		free(backup);
	} */

	gprintf("%s, Current IOS: %i\n", iosOK ? "OK" : "FAILED!", IOS_GetVersion());

	//IOSPATCH_AHBPROT();
	//gprintf("Current AHBPROT state: %s\n", HAVE_AHBPROT ? "enabled" : "disabled");

 	if (launch_game)
	{
		DeviceHandler::Instance()->MountAll();
		DeviceHandler::Instance()->Open_WBFS(currentPartition);
		Disc_Init();
	}
	else Open_Inputs();

	return iosOK;
}