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

#include "patchcode.h"
#include "cios.h"
#include "fs.h"
#include "fst.h"
#include "lz77.h"
#include "utils.h"
#include "videopatch.h"

using namespace std;

void *dolchunkoffset[18];
u32	dolchunksize[18];
u32	dolchunkcount;
u32 bootcontent;

char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);

static u8 *GetDol(u32 bootcontent)
{
	memset(filepath, 0, ISFS_MAXPATH);
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

static bool GetAppNameFromTmd(bool dol, u32 *bootcontent)
{
	bool ret = false;

	memset(filepath, 0, ISFS_MAXPATH);
	sprintf(filepath, "/title/%08x/%08x/content/title.tmd", TITLE_UPPER(conf->title), TITLE_LOWER(conf->title));

	u32 size;
	u8 *data = ISFS_GetFile((u8 *) &filepath, &size, -1);
	if(data == NULL || size < 0x208)
		return ret;

	_tmd *tmd_file = (_tmd *)SIGNATURE_PAYLOAD((u32 *)data);
	for(u16 i = 0; i < tmd_file->num_contents; ++i)
	{
		if(tmd_file->contents[i].index == (dol ? tmd_file->boot_index : 0))
		{
			*bootcontent = tmd_file->contents[i].cid;
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

	for(u8 i = 0; i < 18; i++)
	{
		if(!dolfile->section_size[i]) 
			continue;
		if(dolfile->section_pos[i] < sizeof(dolheader)) 
			continue;
		if(!(dolfile->section_start[i] & 0x80000000)) 
			dolfile->section_start[i] |= 0x80000000;

		dolchunkoffset[dolchunkcount] = (void *)dolfile->section_start[i];
		dolchunksize[dolchunkcount] = dolfile->section_size[i];			

		memmove(dolchunkoffset[dolchunkcount], buffer + dolfile->section_pos[i], dolchunksize[dolchunkcount]);
		DCFlushRange(dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);
		ICInvalidateRange(dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);

		dolchunkcount++;
	}
	return dolfile->entry_point;
}

u32 LoadChannel()
{
	u32 entry = 0;

	/* Re-Init ISFS */
	ISFS_Initialize();

	GetAppNameFromTmd(true, &bootcontent);
	u8 *data = GetDol(bootcontent);
	entry = MoveDol(data);
	free(data);

	/* De-Init ISFS */
	ISFS_Deinitialize();

	return entry;
}

void PatchChannel(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio)
{
	bool hook = false;
	for(u32 i = 0; i < dolchunkcount; i++)
	{		
		patchVideoModes(dolchunkoffset[i], dolchunksize[i], vidMode, vmode, patchVidModes);
		if(vipatch) vidolpatcher(dolchunkoffset[i], dolchunksize[i]);
		if(configbytes[0] != 0xCD) langpatcher(dolchunkoffset[i], dolchunksize[i]);
		if(countryString) PatchCountryStrings(dolchunkoffset[i], dolchunksize[i]);
		if(aspectRatio != -1) PatchAspectRatio(dolchunkoffset[i], dolchunksize[i], aspectRatio);
		if(hooktype != 0 && dogamehooks(dolchunkoffset[i], dolchunksize[i], true))
			hook = true;
	}
	if(hook)
		ocarina_do_code(conf->title);
}
