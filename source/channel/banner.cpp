/***************************************************************************
 * Copyright (C) 2010
 * by r-win
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
 * Banner Handling Class
 *
 * for wiiflow 2010
 ***************************************************************************/

#include <malloc.h>
#include <ogcsys.h>
#include <string.h>
#include <stdio.h>
#include <ogc/conf.h>
#include <ogc/isfs.h>

#include "memory/smartptr.hpp"
#include "banner.h"
#include "MD5.h"
#include "loader/fs.h"
#include "loader/utils.h"
#include "gecko.h"
#include "U8Archive.h"

#define IMET_OFFSET			0x40
#define IMET_SIGNATURE		0x494d4554

Banner::Banner(u8 *bnr, u64 title)
{
	this->title = title;
	opening = bnr;
	imet = NULL;
	
	if (opening == NULL) return;
	
	IMET *imet = (IMET *) opening;
	if (imet->sig != IMET_SIGNATURE)
	{
		imet = (IMET *) (opening + IMET_OFFSET);
	}
	
	if (imet->sig == IMET_SIGNATURE)
	{
		unsigned char md5[16];
		unsigned char imetmd5[16];

		memcpy(imetmd5, imet->md5, 16);
		memset(imet->md5, 0, 16);
		
		MD5(md5, (unsigned char*)(imet), sizeof(IMET));
		if (memcmp(imetmd5, md5, 16) == 0)
		{
			this->imet = imet;
		}
		else
		{
			gprintf("Invalid md5, banner not valid for title %08x\n", title);
		}
	}
	else
	{
		gprintf("Invalid signature found, banner not valid for title %08x\n", title);
	}
}

Banner::~Banner()
{
	SAFE_FREE(opening);
}

bool Banner::IsValid()
{
	return imet != NULL;
}

u16 *Banner::GetName(int language)
{
	if (imet == NULL) return NULL;
	
	if (imet->name_japanese[language*IMET_MAX_NAME_LEN] == 0) // Requested language is not found
	{
		if (imet->name_english[0] == 0) // And the channel name is not available in english
		{
			return NULL;
		}
		language = CONF_LANG_ENGLISH;
	}
	
	if (language >= 0)
	{
		return &imet->name_japanese[language * IMET_MAX_NAME_LEN]; // Return a pointer to the start of the channel name
	}
	return NULL;
}

bool Banner::GetName(u8 *name, int language)
{
	if (imet == NULL) return false;

	u16 *channelname = GetName(language);
	if (channelname)
	{
		memcpy(name, channelname, IMET_MAX_NAME_LEN * sizeof(u16));
		return true;
	}
	return false;
}

bool Banner::GetName(wchar_t *name, int language)
{
	if (imet == NULL) return false;

	u16 *channelname = GetName(language);
	if (channelname)
	{
		for (int i = 0; i < IMET_MAX_NAME_LEN; i++)
		{
			name[i] = channelname[i];
		}						
		return true;
	}
	return false;
}

const u8 *Banner::GetFile(char *name, u32 *size)
{
	const u8 *bnrArc = (const u8 *)(((u8 *) imet) + sizeof(IMET));
	return u8_get_file(bnrArc, name, size);
}

Banner * Banner::GetBanner(u64 title, char *appname, bool isfs, bool imetOnly)
{
	void *buf = NULL;
	if (isfs)
	{
		u32 size = 0;
		
		buf = ISFS_GetFile((u8 *) appname, &size, imetOnly ? sizeof(IMET) + IMET_OFFSET : 0);
		if (size == 0) 
		{
			SAFE_FREE(buf);
			return NULL;
		}
	}
	else
	{
		FILE *fp = fopen(appname, "rb");
		if (fp == NULL) return NULL;
		
		u32 size = sizeof(IMET) + IMET_OFFSET;
		if (!imetOnly)
		{
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
		}
		
		buf = malloc(size);

		fread(buf, size, 1, fp);
		SAFE_CLOSE(fp);
	}
	
	return new Banner((u8 *) buf, title);
}