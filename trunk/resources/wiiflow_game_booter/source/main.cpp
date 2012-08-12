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

#include "apploader.h"
#include "wdvd.h"
#include "patchcode.h"
#include "cios.h"
#include "disc.h"
#include "fst.h"
#include "videopatch.h"

using namespace std;
IOS_Info CurrentIOS;

typedef struct _the_CFG {
	u8 vidMode;
	bool vipatch;
	bool countryString;
	u8 patchVidMode;
	int aspectRatio;
	u32 returnTo;
	u8 configbytes[2];
	IOS_Info IOS;
	void *codelist;
	u8 *codelistend;
	u8 *cheats;
	u32 cheatSize;
	u32 hooktype;
	u8 debugger;
	u32 *gameconf;
	u32 gameconfsize;
	u8 BootType;
	/* needed for channels */
	void *dolchunkoffset[18];
	u32	dolchunksize[18];
	u32	dolchunkcount;
	u32 startPoint;
} the_CFG;

static the_CFG *conf = (the_CFG*)0x90000000;

/* Boot Variables */
u32 vmode_reg = 0;
entry_point p_entry;
GXRModeObj *vmode = NULL;

u32 AppEntrypoint;

extern "C" { extern void __exception_closeall(); }

void PatchChannel(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio)
{
	for(u32 i = 0; i < conf->dolchunkcount; i++)
	{		
		patchVideoModes(conf->dolchunkoffset[i], conf->dolchunksize[i], vidMode, vmode, patchVidModes);
		if(vipatch) vidolpatcher(conf->dolchunkoffset[i], conf->dolchunksize[i]);
		if(configbytes[0] != 0xCD) langpatcher(conf->dolchunkoffset[i], conf->dolchunksize[i]);
		if(countryString) PatchCountryStrings(conf->dolchunkoffset[i], conf->dolchunksize[i]);
		if(aspectRatio != -1) PatchAspectRatio(conf->dolchunkoffset[i], conf->dolchunksize[i], aspectRatio);

		if(hooktype != 0)
			dogamehooks(conf->dolchunkoffset[i], conf->dolchunksize[i], true);
	}
}

int main()
{
	VIDEO_Init();
	configbytes[0] = conf->configbytes[0];
	configbytes[1] = conf->configbytes[1];
	hooktype = conf->hooktype;
	debuggerselect = conf->debugger;
	CurrentIOS = conf->IOS;
	app_gameconfig_set(conf->gameconf, conf->gameconfsize);
	ocarina_set_codes(conf->codelist, conf->codelistend, conf->cheats, conf->cheatSize);

	/* Set low memory */
	Disc_SetLowMem();

	/* Select an appropriate video mode */
	vmode = Disc_SelectVMode(conf->vidMode, &vmode_reg);

	if(conf->BootType == TYPE_WII_GAME)
	{
		WDVD_Init();

		/* Find Partition */
		u64 offset = 0;
		Disc_FindPartition(&offset);

		/* Open Partition */
		WDVD_OpenPartition(offset);

		/* Run apploader */
		Apploader_Run(&p_entry, conf->vidMode, vmode, conf->vipatch, conf->countryString, conf->patchVidMode, 
					conf->aspectRatio, conf->returnTo);
		AppEntrypoint = (u32)p_entry;
	}
	else if(conf->BootType == TYPE_CHANNEL)
	{
		if(hooktype != 0)
			ocarina_do_code();

		PatchChannel(conf->vidMode, vmode, conf->vipatch, conf->countryString, 
					conf->patchVidMode, conf->aspectRatio);
		AppEntrypoint = conf->startPoint;
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
