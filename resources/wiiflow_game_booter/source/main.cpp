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
#include "fs.h"
#include "fst.h"
#include "lz77.h"
#include "utils.h"
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
	u64 title;
} the_CFG;

static the_CFG *conf = (the_CFG*)0x90000000;

/* Boot Variables */
u32 vmode_reg = 0;
entry_point p_entry;
GXRModeObj *vmode = NULL;

u32 AppEntrypoint;

extern "C" { extern void __exception_closeall(); }

typedef struct _dolheader
{
	u32 section_pos[18];
	u32 section_start[18];
	u32 section_size[18];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
	u32 padding[7];
} __attribute__((packed)) dolheader;

void *dolchunkoffset[18];
u32	dolchunksize[18];
u32	dolchunkcount;

void PatchChannel(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio)
{
	for(u32 i = 0; i < dolchunkcount; i++)
	{		
		patchVideoModes(dolchunkoffset[i], dolchunksize[i], vidMode, vmode, patchVidModes);
		if(vipatch) vidolpatcher(dolchunkoffset[i], dolchunksize[i]);
		if(configbytes[0] != 0xCD) langpatcher(dolchunkoffset[i], dolchunksize[i]);
		if(countryString) PatchCountryStrings(dolchunkoffset[i], dolchunksize[i]);
		if(aspectRatio != -1) PatchAspectRatio(dolchunkoffset[i], dolchunksize[i], aspectRatio);

		if(hooktype != 0)
			dogamehooks(dolchunkoffset[i], dolchunksize[i], true);
	}
}

static u8 *GetDol(u32 bootcontent)
{
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	sprintf(filepath, "/title/%08x/%08x/content/%08x.app", TITLE_UPPER(conf->title), TITLE_LOWER(conf->title), bootcontent);

	u32 contentSize = 0;

	u8 *data = ISFS_GetFile((u8 *) &filepath, &contentSize, -1);
	if(data != NULL && contentSize != 0)
	{
		if(isLZ77compressed(data))
		{
			u8 *decompressed;
			u32 size = 0;
			if(decompressLZ77content(data, contentSize, &decompressed, &size) < 0)
			{
				free(data);
				return NULL;
			}
			free(data);
			data = decompressed;
		}	
		return data;
	}
	return NULL;
}

static bool GetAppNameFromTmd(char *app, bool dol, u32 *bootcontent)
{
	bool ret = false;

	char tmd[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	sprintf(tmd, "/title/%08x/%08x/content/title.tmd", TITLE_UPPER(conf->title), TITLE_LOWER(conf->title));

	u32 size;
	u8 *data = ISFS_GetFile((u8 *) &tmd, &size, -1);
	if (data == NULL || size < 0x208)
		return ret;

	_tmd *tmd_file = (_tmd *)SIGNATURE_PAYLOAD((u32 *)data);
	u16 i;
	for(i = 0; i < tmd_file->num_contents; ++i)
	{
		if(tmd_file->contents[i].index == (dol ? tmd_file->boot_index : 0))
		{
			*bootcontent = tmd_file->contents[i].cid;
			sprintf(app, "/title/%08x/%08x/content/%08x.app", TITLE_UPPER(conf->title), TITLE_LOWER(conf->title), *bootcontent);
			ret = true;
			break;
		}
	}

	free(data);

	return ret;
}

static u32 MoveDol(u8 *buffer)
{
	dolchunkcount = 0;
	dolheader *dolfile = (dolheader *)buffer;

	if(dolfile->bss_start)
	{
		if(!(dolfile->bss_start & 0x80000000))
			dolfile->bss_start |= 0x80000000;

		memset((void *)dolfile->bss_start, 0, dolfile->bss_size);
		DCFlushRange((void *)dolfile->bss_start, dolfile->bss_size);
		ICInvalidateRange((void *)dolfile->bss_start, dolfile->bss_size);
	}

	int i;
	for(i = 0; i < 18; i++)
	{
		if(!dolfile->section_size[i]) 
			continue;
		if(dolfile->section_pos[i] < sizeof(dolheader)) 
			continue;
		if(!(dolfile->section_start[i] & 0x80000000)) 
			dolfile->section_start[i] |= 0x80000000;

		dolchunkoffset[dolchunkcount] = (void *)dolfile->section_start[i];
		dolchunksize[dolchunkcount] = dolfile->section_size[i];			

		//gprintf("Moving section %u from offset %08x to %08x-%08x...\n", i, dolfile->section_pos[i], dolchunkoffset[dolchunkcount], (u32)dolchunkoffset[dolchunkcount]+dolchunksize[dolchunkcount]);
		memmove(dolchunkoffset[dolchunkcount], buffer + dolfile->section_pos[i], dolchunksize[dolchunkcount]);
		DCFlushRange(dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);
		ICInvalidateRange(dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);

		dolchunkcount++;
	}
	return dolfile->entry_point;
}

u32 LoadChannel()
{
	char app[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	u32 bootcontent;
	u32 entry = 0;

	if(!GetAppNameFromTmd(app, true, &bootcontent))
		return entry;

	u8 *data = GetDol(bootcontent);
	entry = MoveDol(data);
	free(data);
	return entry;
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
		ISFS_Initialize();
		AppEntrypoint = LoadChannel();
		PatchChannel(conf->vidMode, vmode, conf->vipatch, conf->countryString, 
					conf->patchVidMode, conf->aspectRatio);
		if(hooktype != 0)
			ocarina_do_code(conf->title);
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
