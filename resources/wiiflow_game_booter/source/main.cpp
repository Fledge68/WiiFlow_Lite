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

#define EXT_ADDR_CFG	((vu32*)0x90100000)
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
	memcpy(&normalCFG, ((void*)*EXT_ADDR_CFG), sizeof(the_CFG));
	VIDEO_Init();// libogc
	video_init();// tinyload progress bar
	prog10();

	configbytes[0] = normalCFG.configbytes[0];// game language 0 - 9 or 0xCD = not patched
	configbytes[1] = normalCFG.configbytes[1];// not used
	hooktype = normalCFG.hooktype;
	debuggerselect = normalCFG.debugger;
	CurrentIOS = normalCFG.IOS;
	set_wip_list(normalCFG.wip_list, normalCFG.wip_count);
	app_gameconfig_set(normalCFG.gameconf, normalCFG.gameconfsize);
	ocarina_set_codes(normalCFG.codelist, normalCFG.codelistend, normalCFG.cheats, normalCFG.cheatSize);
	copy_frag_list(normalCFG.fragments);
	wbfsDev = normalCFG.wbfsDevice;
	wbfs_part_idx = normalCFG.wbfsPart;
	prog10();

	if(normalCFG.BootType == TYPE_WII_GAME)
	{
		WDVD_Init();
		if(CurrentIOS.Type == IOS_TYPE_D2X)
		{
			// only for wii games - no known channels require block IOS Reload
			s32 ret = BlockIOSReload();
			gprintf("Block IOS Reload using d2x %s.\n", ret < 0 ? "failed" : "succeeded");
		}
		if(normalCFG.GameBootType == TYPE_WII_DISC)
		{
			Disc_SetUSB(NULL, false);
			if(CurrentIOS.Type == IOS_TYPE_HERMES)
				Hermes_Disable_EHC();
			if(normalCFG.vidMode > 1) //forcing a video mode
				normalCFG.patchVidMode = 1; //always normal patching
		}
		else
		{
			Disc_SetUSB((u8*)normalCFG.gameID, normalCFG.GameBootType == TYPE_WII_WBFS_EXT);
			if(CurrentIOS.Type == IOS_TYPE_HERMES)
				Hermes_shadow_mload();
		}
		prog(20);
		/* Clear Disc ID */
		memset((u8*)Disc_ID, 0, 32);
		Disc_Open(normalCFG.GameBootType);// sets Disc_ID
		u32 offset = 0;
		Disc_FindPartition(&offset);
		WDVD_OpenPartition(offset, &GameIOS);
		Disc_SetLowMem();
		if(normalCFG.vidMode == 5)
			normalCFG.patchVidMode = 1; //progressive mode requires this
		vmode = Disc_SelectVMode(normalCFG.vidMode, &vmode_reg);// requires Disc_ID[3]
		AppEntrypoint = Apploader_Run(normalCFG.vidMode, vmode, normalCFG.vipatch, normalCFG.countryString, normalCFG.patchVidMode, normalCFG.aspectRatio, normalCFG.returnTo,  
						normalCFG.patchregion, normalCFG.private_server, normalCFG.server_addr, normalCFG.videoWidth, normalCFG.patchFix480p, normalCFG.deflicker, normalCFG.BootType);
		WDVD_Close();
	}
	else if(normalCFG.BootType == TYPE_CHANNEL)
	{
		ISFS_Initialize();
		AppEntrypoint = LoadChannel(normalCFG.title, normalCFG.use_dol, &GameIOS);// sets Disc_ID
		ISFS_Deinitialize();
		vmode = Disc_SelectVMode(normalCFG.vidMode, &vmode_reg);// requires Disc_ID[3]
		PatchChannel(normalCFG.vidMode, vmode, normalCFG.vipatch, normalCFG.countryString, normalCFG.patchVidMode, normalCFG.aspectRatio,
					normalCFG.returnTo, normalCFG.private_server, normalCFG.server_addr, normalCFG.videoWidth, normalCFG.patchFix480p, normalCFG.deflicker, normalCFG.BootType);
	}
	gprintf("Entrypoint: %08x, Requested Game IOS: %i\n", AppEntrypoint, GameIOS);
	setprog(320);

	/* Set Disc ID for WiiRD - must be set after ocarina stuff is done */
	memcpy((void*)0x80001800, (void*)Disc_ID, 8);
	
	/* Error 002 Fix (thanks WiiPower and uLoader) */
	*Current_IOS = (GameIOS << 16) | 0xffff;
	if(!isForwarder)
		*Apploader_IOS = (GameIOS << 16) | 0xffff;
	
	/* Flush everything */
	DCFlushRange((void*)0x80000000, 0x3f00);
	
	/* Enable front LED if requested */
	if(normalCFG.use_led) *HW_GPIOB_OUT |= 0x20;

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
