/****************************************************************************
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <malloc.h>

#include "Config.hpp"
#include "ChannelHandler.hpp"

#include "apploader.h"
#include "patchcode.h"
#include "disc.h"
#include "fst.h"
#include "wdvd.h"
#include "gecko.h"

using namespace std;
IOS_Info CurrentIOS;

/* Boot Variables */
u32 vmode_reg = 0;
entry_point p_entry;
GXRModeObj *vmode = NULL;

u32 AppEntrypoint;

extern "C" {
extern void __exception_closeall();
extern s32 wbfsDev;
extern u32 wbfs_part_idx;
extern FragList *frag_list;
}

int main()
{
	VIDEO_Init();
	InitGecko();
	gprintf("WiiFlow External Booter by FIX94\n");

	configbytes[0] = conf->configbytes[0];
	configbytes[1] = conf->configbytes[1];
	hooktype = conf->hooktype;
	debuggerselect = conf->debugger;
	CurrentIOS = conf->IOS;
	app_gameconfig_set(conf->gameconf, conf->gameconfsize);
	ocarina_set_codes(conf->codelist, conf->codelistend, conf->cheats, conf->cheatSize);
	frag_list = conf->fragments;
	wbfsDev = conf->wbfsDevice;
	wbfs_part_idx = conf->wbfsPart;

	if(conf->BootType == TYPE_WII_GAME)
	{
		WDVD_Init();
		if(conf->GameBootType == TYPE_WII_DISC)
			Disc_SetUSB(NULL, false);
		else
			Disc_SetUSB((u8*)conf->gameID, conf->GameBootType == TYPE_WII_WBFS_EXT);
		Disc_Open();
		Disc_SetLowMem();
		u64 offset = 0;
		Disc_FindPartition(&offset);
		gprintf("Partition Offset: %08x\n", offset);
		WDVD_OpenPartition(offset);
		vmode = Disc_SelectVMode(conf->vidMode, &vmode_reg);
		Apploader_Run(&p_entry, conf->vidMode, vmode, conf->vipatch, conf->countryString, conf->patchVidMode, 
					conf->aspectRatio, conf->returnTo);
		AppEntrypoint = (u32)p_entry;
		if(CurrentIOS.Type == IOS_TYPE_HERMES)
		{
			if(conf->GameBootType == TYPE_WII_DISC)
				Hermes_Disable_EHC();
			else
				Hermes_shadow_mload(conf->mload_rev);
		}
		WDVD_Close();
	}
	else if(conf->BootType == TYPE_CHANNEL)
	{
		ISFS_Initialize();
		AppEntrypoint = LoadChannel();
		Disc_SetLowMem();
		vmode = Disc_SelectVMode(conf->vidMode, &vmode_reg);
		PatchChannel(conf->vidMode, vmode, conf->vipatch, conf->countryString, 
					conf->patchVidMode, conf->aspectRatio);
		ISFS_Deinitialize();
	}

	/* Set time */
	Disc_SetTime();

	/* Set an appropriate video mode */
	Disc_SetVMode(vmode, vmode_reg);

	/* Shutdown IOS subsystems */
	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();

	/* Originally from tueidj - taken from NeoGamma (thx) */
	*(vu32*)0xCC003024 = 1;

 	if(AppEntrypoint == 0x3400)
	{
 		if(hooktype)
 		{
			asm volatile (
				"lis %r3, returnpoint@h\n"
				"ori %r3, %r3, returnpoint@l\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"nop\n"
				"mtctr %r3\n"
				"bctr\n"
				"returnpoint:\n"
				"bl DCDisable\n"
				"bl ICDisable\n"
				"li %r3, 0\n"
				"mtsrr1 %r3\n"
				"lis %r4, AppEntrypoint@h\n"
				"ori %r4,%r4,AppEntrypoint@l\n"
				"lwz %r4, 0(%r4)\n"
				"mtsrr0 %r4\n"
				"rfi\n"
			);
 		}
 		else
 		{
 			asm volatile (
 				"isync\n"
				"lis %r3, AppEntrypoint@h\n"
				"ori %r3, %r3, AppEntrypoint@l\n"
 				"lwz %r3, 0(%r3)\n"
 				"mtsrr0 %r3\n"
 				"mfmsr %r3\n"
 				"li %r4, 0x30\n"
 				"andc %r3, %r3, %r4\n"
 				"mtsrr1 %r3\n"
 				"rfi\n"
 			);
 		}
	}
 	else if (hooktype)
	{
		asm volatile (
				"lis %r3, AppEntrypoint@h\n"
				"ori %r3, %r3, AppEntrypoint@l\n"
				"lwz %r3, 0(%r3)\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"nop\n"
				"mtctr %r3\n"
				"bctr\n"
		);
	}
	else
	{
		asm volatile (
				"lis %r3, AppEntrypoint@h\n"
				"ori %r3, %r3, AppEntrypoint@l\n"
				"lwz %r3, 0(%r3)\n"
				"mtlr %r3\n"
				"blr\n"
		);
	}

	IRQ_Restore(level);

	return 0;
}
