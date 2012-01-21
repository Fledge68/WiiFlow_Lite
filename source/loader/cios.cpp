/***************************************************************************
 * Copyright (C) 2011
 * by Dimok
 * Modifications by xFede
 * Wiiflowized and heavily improved by Miigotu
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
 ***************************************************************************/


#include <gctypes.h>
#include <malloc.h>
#include <cstdio>
#include <cstring>

#include "cios.hpp"
#include "utils.h"
#include "mem2.hpp"
#include "fs.h"

#define ARRAY_SIZE(a)		(sizeof a / sizeof a[0])

static u32 allowedBases[] = { 37, 38, 53, 55, 56, 57, 58 };

/* Check if the cIOS is a D2X. */
bool cIOSInfo::D2X(u8 ios, u8 *base)
{
	bool ret = false;

	iosinfo_t *info = GetInfo(ios);
	if(info != NULL)
	{
		*base = (u8)info->baseios;
		SAFE_FREE(info);
		ret = true;
	}
	return ret;
}

/*
 * Reads the ios info struct from the .app file.
 * @return pointer to iosinfo_t on success else NULL. The user is responsible for freeing the buffer.
 */

iosinfo_t *cIOSInfo::GetInfo(u8 ios)
{
	u32 TMD_Length;
	if (ES_GetStoredTMDSize(TITLE_ID(1, ios), &TMD_Length) < 0) return NULL;

	signed_blob *TMD = (signed_blob*) MEM2_alloc(ALIGN32(TMD_Length));
	if (!TMD) return NULL;

	if (ES_GetStoredTMD(TITLE_ID(1, ios), TMD, TMD_Length) < 0)
	{
		SAFE_FREE(TMD);
		return NULL;
	}

	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	sprintf(filepath, "/title/%08x/%08x/content/%08x.app", 0x00000001, ios, *(u8 *)((u32)TMD+0x1E7));

	SAFE_FREE(TMD);

	u32 size = 0;
	u8 *buffer = ISFS_GetFile((u8 *) filepath, &size, sizeof(iosinfo_t));
	if(!buffer) return NULL;

	iosinfo_t *iosinfo = (iosinfo_t *) buffer;

	bool baseMatch = false;
	for(u8 i = 0; i < ARRAY_SIZE(allowedBases); i++)
		if(iosinfo->baseios == allowedBases[i])
		{
			baseMatch = true;
			break;
		}

	if(iosinfo->magicword != 0x1ee7c105					/* Magic Word */
		|| iosinfo->magicversion != 1					/* Magic Version */
		|| iosinfo->version < 6							/* Version */
		|| !baseMatch									/* Base */
		|| strncasecmp(iosinfo->name, "d2x", 3) != 0)	/* Name */
	{
		SAFE_FREE(buffer);
		return NULL;
	}
	return iosinfo;
}