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
#include "video_tinyload.h"
#include "apploader.h"
#include "patchcode.h"
#include "memory.h"
#include "utils.h"
#include "disc.h"
#include "fst.h"
#include "wdvd.h"
#include "gecko.h"

using namespace std;
IOS_Info CurrentIOS;

/* Boot Variables */
u32 GameIOS = 0;
u32 vmode_reg = 0;
GXRModeObj *vmode = NULL;

u32 AppEntrypoint = 0;

extern "C" {
extern void __exception_closeall();
extern s32 wbfsDev;
extern u32 wbfs_part_idx;
extern FragList *frag_list;
}

the_CFG normalCFG;
int main()
{
	InitGecko();
	gprintf("WiiFlow External Booter by FIX94\n");
	memcpy(&normalCFG, (void*)0x93100000, sizeof(the_CFG));
	VIDEO_Init();
	video_init();
	prog10();

	configbytes[0] = normalCFG.configbytes[0];
	configbytes[1] = normalCFG.configbytes[1];
	hooktype = normalCFG.hooktype;
	debuggerselect = normalCFG.debugger;
	CurrentIOS = normalCFG.IOS;
	set_wip_list(normalCFG.wip_list, normalCFG.wip_count);
	app_gameconfig_set(normalCFG.gameconf, normalCFG.gameconfsize);
	ocarina_set_codes(normalCFG.codelist, normalCFG.codelistend, normalCFG.cheats, normalCFG.cheatSize);
	frag_list = normalCFG.fragments;
	wbfsDev = normalCFG.wbfsDevice;
	wbfs_part_idx = normalCFG.wbfsPart;
	prog10();

	/* Setup Low Memory */
	Disc_SetLowMemPre();

	if(normalCFG.BootType == TYPE_WII_GAME)
	{
		WDVD_Init();
		if(CurrentIOS.Type == IOS_TYPE_D2X)
		{
			s32 ret = BlockIOSReload();
			gprintf("Block IOS Reload using d2x %s.\n", ret < 0 ? "failed" : "succeeded");
		}
		if(normalCFG.GameBootType == TYPE_WII_DISC)
		{
			Disc_SetUSB(NULL, false);
			if(CurrentIOS.Type == IOS_TYPE_HERMES)
				Hermes_Disable_EHC();
		}
		else
		{
			Disc_SetUSB((u8*)normalCFG.gameID, normalCFG.GameBootType == TYPE_WII_WBFS_EXT);
			if(CurrentIOS.Type == IOS_TYPE_HERMES)
				Hermes_shadow_mload();
		}
		prog(20);
		Disc_Open();
		u32 offset = 0;
		Disc_FindPartition(&offset);
		WDVD_OpenPartition(offset, &GameIOS);
		vmode = Disc_SelectVMode(normalCFG.vidMode, &vmode_reg);
		AppEntrypoint = Apploader_Run(normalCFG.vidMode, vmode, normalCFG.vipatch, normalCFG.countryString,
						normalCFG.patchVidMode, normalCFG.aspectRatio, normalCFG.returnTo);
		WDVD_Close();
	}
	else if(normalCFG.BootType == TYPE_CHANNEL)
	{
		ISFS_Initialize();
		*Disc_ID = TITLE_LOWER(normalCFG.title);
		vmode = Disc_SelectVMode(normalCFG.vidMode, &vmode_reg);
		AppEntrypoint = LoadChannel(normalCFG.title, &GameIOS);
		PatchChannel(normalCFG.vidMode, vmode, normalCFG.vipatch, normalCFG.countryString, 
					normalCFG.patchVidMode, normalCFG.aspectRatio);
		ISFS_Deinitialize();
	}
	gprintf("Entrypoint: %08x, Requested Game IOS: %i\n", AppEntrypoint, GameIOS);
	setprog(320);

	/* Setup Low Memory */
	Disc_SetLowMem(GameIOS);

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
 	else if(hooktype)
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
