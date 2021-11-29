/***************************************************************************
 * Copyright (C) 2010
 * by dude
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * Channel Launcher Class
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <malloc.h>

#include "ChannelHandler.hpp"
#include "patchcode.h"
#include "cios.h"
#include "fs.h"
#include "fst.h"
#include "lz77.h"
#include "utils.h"
#include "videopatch.h"
#include "video_tinyload.h"
#include "apploader.h"
#include "memory.h"

void *dolchunkoffset[18];
u32	dolchunksize[18];
u32	dolchunkcount;
u32 bootcontent;
bool isForwarder = false;

char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);

static u8 *GetDol(u32 bootcontent, u64 title)
{
	snprintf(filepath, ISFS_MAXPATH, "/title/%08x/%08x/content/%08x.app", TITLE_UPPER(title), TITLE_LOWER(title), bootcontent);

	u32 contentSize = 0;

	u8 *data = ISFS_GetFile(filepath, &contentSize, -1);
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

static bool GetAppNameFromTmd(bool dol, u32 *bootcontent, u64 title, u32 *IOS)
{
	static const u8 dolsign[6] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    static u8 dolhead[32] ATTRIBUTE_ALIGN(32);
	bool found = false;
	snprintf(filepath, ISFS_MAXPATH, "/title/%08x/%08x/content/title.tmd", TITLE_UPPER(title), TITLE_LOWER(title));

	u32 size;
	u8 *data = ISFS_GetFile(filepath, &size, -1);
	if(data == NULL || size < 0x208)
		return found;
	*IOS = data[0x18B];

	_tmd *tmd_file = (_tmd *)SIGNATURE_PAYLOAD((u32 *)data);
	
	if(dol)
	{
		// check for dol signature - channels and vc
        for(u32 i = 0; i < tmd_file->num_contents; ++i)
        {
            if(tmd_file->contents[i].index == tmd_file->boot_index)
                continue; // Skip app loader

            snprintf(filepath, ISFS_MAXPATH, "/title/%08x/%08x/content/%08x.app", TITLE_UPPER(title), TITLE_LOWER(title), tmd_file->contents[i].cid);

            s32 fd = ISFS_Open(filepath, ISFS_OPEN_READ);
            if(fd < 0)
                continue;

            s32 ret = ISFS_Read(fd, dolhead, 32);
            ISFS_Close(fd);

            if(ret != 32)
                continue;

            if(memcmp(dolhead, dolsign, sizeof(dolsign)) == 0)
            {
                // Normal channels and VC use 1
                if(tmd_file->contents[i].index != 1)// forwarder channel
                    isForwarder = true;
				*bootcontent = tmd_file->contents[i].cid;
				found = true;
                break;
            }
        }
	}
	if(!found) // WiiWare not matching a dol signature or apploader selected
	{
		for(u32 i = 0; i < tmd_file->num_contents; ++i)
		{
			if(tmd_file->contents[i].index == (dol ? 0x01 : tmd_file->boot_index))
			{
				*bootcontent = tmd_file->contents[i].cid;
				found = true;
				break;
			}
		}
	}
	//fall back to app loader if main dol wanted but not found
	if(!found && dol)
	{
		*bootcontent = tmd_file->contents[tmd_file->boot_index].cid;
		found = true;
	}
	free(data);
	return found;
}

static u32 MoveDol(u8 *buffer)
{
	dolchunkcount = 0;
	dolheader *dolfile = (dolheader *)buffer;

	if(dolfile->bss_start)// if start is not zero
	{
		if(!(dolfile->bss_start & 0x80000000))// if start isn't >=80000000 
			dolfile->bss_start |= 0x80000000;// set it to 80000000 or greater

		memset((void *)dolfile->bss_start, 0, dolfile->bss_size);
		DCFlushRange((void *)dolfile->bss_start, dolfile->bss_size);
		ICInvalidateRange((void *)dolfile->bss_start, dolfile->bss_size);
	}

	for(u8 i = 0; i < 18; i++)
	{
		if(!dolfile->section_size[i])// if section size is zero don't move 
			continue;
		if(dolfile->section_pos[i] < sizeof(dolheader)) // if section position is within the header don't move
			continue;
		if(!(dolfile->section_start[i] & 0x80000000)) // maker sure section start is 80000000 or greater
			dolfile->section_start[i] |= 0x80000000;

		dolchunkoffset[dolchunkcount] = (void *)dolfile->section_start[i];
		dolchunksize[dolchunkcount] = dolfile->section_size[i];

		memmove(dolchunkoffset[dolchunkcount], buffer + dolfile->section_pos[i], dolchunksize[dolchunkcount]);
		dolchunkcount++;
		prog(5);
	}
	return dolfile->entry_point;
}

u32 LoadChannel(u64 title, bool dol, u32 *IOS)
{
	u32 entry = 0;

	GetAppNameFromTmd(dol, &bootcontent, title, IOS);
	u8 *data = GetDol(bootcontent, title);
	entry = MoveDol(data);
	free(data);

	// Preparations
    memset((void *)Disc_ID, 0, 6);
    *Disc_ID = TITLE_LOWER(title);                	  // Game ID
    *Arena_H = 0;                                     // Arena High, the apploader does this too
    *BI2 = 0x817FE000;                                // BI2, the apploader does this too
    *Bus_Speed = 0x0E7BE2C0;                          // bus speed
    *CPU_Speed = 0x2B73A840;                          // cpu speed
    *GameID_Address = 0x81000000;                     // Game id address, while there's all 0s at 0x81000000 when using the apploader...
    memcpy((void *)Online_Check, (void *)Disc_ID, 4); // online check

    memset((void *)0x817FE000, 0, 0x2000); // Clearing BI2
    DCFlushRange((void *)0x817FE000, 0x2000);

	return entry;
}

void PatchChannel(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, 
				u32 returnTo, u8 private_server, const char *server_addr, bool patchFix480p, u8 deflicker, u8 bootType)
{
	u8 vfilter_off[7] = {0, 0, 21, 22, 21, 0, 0};
	u8 vfilter_low[7] = {4, 4, 16, 16, 16, 4, 4};
	u8 vfilter_medium[7] = {4, 8, 12, 16, 12, 8, 4};
	u8 vfilter_high[7] = {8, 8, 10, 12, 10, 8, 8};

	bool hookpatched = false;
	for(u8 i = 0; i < dolchunkcount; i++)
	{		
		patchVideoModes(dolchunkoffset[i], dolchunksize[i], vidMode, vmode, patchVidModes, bootType);
		if(vipatch)
			vidolpatcher(dolchunkoffset[i], dolchunksize[i]);
		if(configbytes[0] != 0xCD)
			langpatcher(dolchunkoffset[i], dolchunksize[i]);
		if(countryString)
			PatchCountryStrings(dolchunkoffset[i], dolchunksize[i]);
		if(aspectRatio != -1)
			PatchAspectRatio(dolchunkoffset[i], dolchunksize[i], aspectRatio);
		if(returnTo)
			PatchReturnTo(dolchunkoffset[i], dolchunksize[i], returnTo);
		if(private_server)
			PrivateServerPatcher(dolchunkoffset[i], dolchunksize[i], private_server, server_addr);	
		if(hooktype != 0 && hookpatched == false)
			hookpatched = dogamehooks(dolchunkoffset[i], dolchunksize[i], true);

		if (deflicker == DEFLICKER_ON_LOW)
		{
			patch_vfilters(dolchunkoffset[i], dolchunksize[i], vfilter_low);
			patch_vfilters_rogue(dolchunkoffset[i], dolchunksize[i], vfilter_low);
		}
		else if (deflicker == DEFLICKER_ON_MEDIUM)
		{
			patch_vfilters(dolchunkoffset[i], dolchunksize[i], vfilter_medium);
			patch_vfilters_rogue(dolchunkoffset[i], dolchunksize[i], vfilter_medium);
		}
		else if (deflicker == DEFLICKER_ON_HIGH)
		{
			patch_vfilters(dolchunkoffset[i], dolchunksize[i], vfilter_high);
			patch_vfilters_rogue(dolchunkoffset[i], dolchunksize[i], vfilter_high);
		}
		else if (deflicker != DEFLICKER_NORMAL) // Either safe or extended
		{
			patch_vfilters(dolchunkoffset[i], dolchunksize[i], vfilter_off);
			patch_vfilters_rogue(dolchunkoffset[i], dolchunksize[i], vfilter_off);
			// This might break fade and brightness effects
			if (deflicker == DEFLICKER_OFF_EXTENDED)
				deflicker_patch(dolchunkoffset[i], dolchunksize[i]);
		}

		DCFlushRange(dolchunkoffset[i], dolchunksize[i]);
		ICInvalidateRange(dolchunkoffset[i], dolchunksize[i]);
		prog10();
	}
	if(hookpatched)
		ocarina_do_code();
	
	if(patchFix480p)
		PatchFix480p();
		
	if(private_server == PRIVSERV_WIIMMFI)
		do_new_wiimmfi_nonMKWii();
}
