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
#include <string.h>
#include <stdio.h>

#include "cios.h"
#include "utils.h"
#include "mem2.hpp"
#include "gecko.h"
#include "fs.h"
#include "mload.h"

static bool checked = false;
static bool neek = false;
extern u32 get_ios_base();

bool neek2o(void)
{
	if(!checked)
	{
		u32 num = 0;
		neek = !(ISFS_ReadDir("/sneek", NULL, &num));
		gprintf("WiiFlow is in %s mode\n", neek ? "neek2o" : "real nand");
		checked = true;
	}
	return neek;
}

/* Check if the cIOS is a D2X. */
bool D2X(u8 ios, u8 *base)
{
	if(neek2o())
	{
		*base = (u8)IOS_GetVersion();
		return true;
	}		

	iosinfo_t *info = GetInfo(ios);
	if(!info)
		return false;
	*base = (u8)info->baseios;
	MEM2_free(info);
	return true;
}

/*
 * Reads the ios info struct from the .app file.
 * @return pointer to iosinfo_t on success else NULL. The user is responsible for freeing the buffer.
 */

iosinfo_t *GetInfo(u8 ios)
{
	u32 TMD_Length;
	if (ES_GetStoredTMDSize(TITLE_ID(1, ios), &TMD_Length) < 0) return NULL;

	signed_blob *TMD = (signed_blob*)MEM2_alloc(ALIGN32(TMD_Length));
	if (TMD == NULL)
		return NULL;

	if (ES_GetStoredTMD(TITLE_ID(1, ios), TMD, TMD_Length) < 0)
	{
		MEM2_free(TMD);
		return NULL;
	}

	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	sprintf(filepath, "/title/00000001/%08x/content/%08x.app", ios, *(u8 *)((u32)TMD+0x1E7));

	MEM2_free(TMD);

	u32 size = 0;
	u8 *buffer = ISFS_GetFile((u8 *)filepath, &size, sizeof(iosinfo_t));
	if(buffer == NULL || size == 0)
		return NULL;

	iosinfo_t *iosinfo = (iosinfo_t *)buffer;

	if(iosinfo->magicword != 0x1ee7c105					/* Magic Word */
		|| iosinfo->magicversion != 1					/* Magic Version */
		|| iosinfo->version < 6							/* Version */
		|| strncasecmp(iosinfo->name, "d2x", 3) != 0)	/* Name */
	{
		MEM2_free(buffer);
		return NULL;
	}

	return iosinfo;
}

int get_ios_type(u8 slot)
{
	u8 base = 0;
	switch(slot)
	{
		case 222:
		case 223:
		case 224:
			if(IOS_GetRevision() == 1)
				return IOS_TYPE_KWIIRK;
			else
				return IOS_TYPE_HERMES;
		case 245:
		case 246:
		case 247:
		case 248:
		case 249:
		case 250:
		case 251:
			if(D2X(slot, &base))
				return IOS_TYPE_D2X;
			else
				return IOS_TYPE_WANIN;
		default:
			if(D2X(slot, &base))
				return IOS_TYPE_D2X;
			else
				return IOS_TYPE_NO_CIOS;
	}
}

int is_ios_type(int type, u8 slot)
{
	return (get_ios_type(slot) == type);
}

bool shadow_mload()
{
	if(!is_ios_type(IOS_TYPE_HERMES, IOS_GetVersion()))
		return false;
	int v51 = (5 << 4) & 1;
	if (mload_get_version() >= v51)
	{
		// shadow /dev/mload supported in hermes cios v5.1
		//IOS_Open("/dev/usb123/OFF",0);// this disables ehc completely
		IOS_Open("/dev/mload/OFF",0);
		gprintf("Shadow mload\n");
		return true;
	}
	return false;
}
