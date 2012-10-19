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

#include <ogcsys.h>
#include <string.h>
#include <stdio.h>
#include <ogc/conf.h>
#include <ogc/isfs.h>

#include "banner.h"
#include "MD5.h"
#include "gecko/gecko.h"
#include "loader/fs.h"
#include "memory/smartptr.hpp"
#include "unzip/U8Archive.h"

#define IMET_OFFSET			0x40
#define IMET_SIGNATURE		0x494d4554

Banner CurrentBanner;

Banner::Banner()
{
	opening = NULL;
	opening_size = 0;
	title = 0;
	imet = NULL;
}

void Banner::SetBanner(u8 *bnr, u32 bnr_size, u64 title, bool custom)
{
	ClearBanner();
	if(bnr == NULL || bnr_size == 0)
		return;

	this->title = title;
	opening = bnr;
	opening_size = bnr_size;
	imet = (IMET *)opening;
	if(imet->sig != IMET_SIGNATURE)
		imet = (IMET *) (opening + IMET_OFFSET);

	if(imet->sig == IMET_SIGNATURE)
	{
		DCFlushRange(opening, opening_size);
		unsigned char md5[16];
		unsigned char imetmd5[16];

		memcpy(imetmd5, imet->md5, 16);
		memset(imet->md5, 0, 16);
		MD5(md5, (unsigned char*)(imet), sizeof(IMET));
		memcpy(imet->md5, imetmd5, 16);

		if(memcmp(imetmd5, md5, 16) == 0 || custom)
			this->imet = imet;
		else
			gprintf("Invalid md5, banner not valid for title %08x\n", title);
	}
	else
		gprintf("Invalid signature found, banner not valid for title %08x\n", title);
}

void Banner::ClearBanner()
{
	if(opening != NULL)
		free(opening);
	opening = NULL;
	opening_size = 0;
	title = 0;
	imet = NULL;
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
	if(channelname)
	{
		for(int i = 0; i < IMET_MAX_NAME_LEN; i++)
			name[i] = channelname[i];
		return true;
	}
	return false;
}

u8 *Banner::GetFile(char *name, u32 *size)
{
	const u8 *bnrArc = (const u8 *)(((u8 *) imet) + sizeof(IMET));
	const u8* curfile = u8_get_file(bnrArc, name, size);
	if(curfile == NULL || *size == 0)
		return NULL;

	//I like to have stuff in a separate memory region
	u8 *file = (u8*)malloc(*size);
	if(file == NULL)
		return NULL;

	memcpy(file, curfile, (*size));
	return file;
}

void Banner::GetBanner(u64 title, char *appname, bool isfs, bool imetOnly)
{
	void *buf = NULL;
	u32 size = 0;
	if (isfs)
	{
		buf = ISFS_GetFile((u8 *)appname, &size, imetOnly ? sizeof(IMET) + IMET_OFFSET : 0);
		if (size == 0) 
		{
			if(buf != NULL)
				free(buf);
			return;
		}
	}
	else
	{
		FILE *fp = fopen(appname, "rb");
		if(fp == NULL)
			return;

		u32 size = sizeof(IMET) + IMET_OFFSET;
		if (!imetOnly)
		{
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
		}

		buf = malloc(size);
		if(!buf)
			return;

		fread(buf, size, 1, fp);
		fclose(fp);
	}
	SetBanner((u8 *)buf, size, title);
}
