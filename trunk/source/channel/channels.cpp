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
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "channel_launcher.h"
#include "channels.h"
#include "banner.h"
#include "nand.hpp"
#include "config/config.hpp"
#include "gecko/gecko.hpp"
#include "gui/fmt.h"
#include "gui/text.hpp"
#include "loader/fs.h"
#include "loader/nk.h"
#include "loader/sys.h"
#include "memory/mem2.hpp"
#include "wstringEx/wstringEx.hpp"

#define DOWNLOADED_CHANNELS	0x00010001
#define SYSTEM_CHANNELS		0x00010002
#define GAME_CHANNELS		0x00010004

#define RF_NEWS_CHANNEL		0x48414741
#define RF_FORECAST_CHANNEL	0x48414641

Channels ChannelHandle;

void Channels::Init(string lang)
{
	this->langCode = lang;
	this->clear();
	Search();
}

void Channels::Cleanup()
{
	this->clear();
}

u8 Channels::GetRequestedIOS(u64 title)
{
	u8 IOS = 0;
	u32 size = 0;
	u8 *titleTMD = NULL;
	if(NANDemuView)
		titleTMD = NandHandle.GetTMD(title, &size);
	else
	{
		char tmd[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
		strncpy(tmd, fmt("/title/%08x/%08x/content/title.tmd", TITLE_UPPER(title), TITLE_LOWER(title)), ISFS_MAXPATH);
		titleTMD = ISFS_GetFile(tmd, &size, -1);
	}
	if(titleTMD == NULL)
		return 0;

	if(size > 0x18B)
		IOS = titleTMD[0x18B];

	free(titleTMD);
	gprintf("Requested Game IOS: %i\n", IOS);
	return IOS;
}

u64 *Channels::GetChannelList(u32 *count)
{
	*count = 0;
	u32 countall;
	if(ES_GetNumTitles(&countall) < 0 || countall == 0)
		return NULL;

	u64 *titles = (u64*)MEM2_alloc(countall*sizeof(u64));
	if(titles == NULL)
		return NULL;

	if(ES_GetTitles(titles, countall) < 0)
	{
		MEM2_free(titles);
		return NULL;
	}
	*count = countall;
	return titles;
}

bool Channels::GetAppNameFromTmd(u64 title, char *app, u32 *bootcontent)
{
	bool ret = false;
	u32 size = 0;
	u8 *data = NULL;
	if(NANDemuView)
		data = NandHandle.GetTMD(title, &size);
	else
	{
		char tmd[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
		strncpy(tmd, fmt("/title/%08x/%08x/content/title.tmd", TITLE_UPPER(title), TITLE_LOWER(title)), ISFS_MAXPATH);
		data = ISFS_GetFile(tmd, &size, -1);
	}
	if(data == NULL || size < 0x208)
	{
		if(data != NULL)
			free(data);
		return ret;
	}
	_tmd *tmd_file = (_tmd *)SIGNATURE_PAYLOAD((u32 *)data);
	u16 i;
	for(i = 0; i < tmd_file->num_contents; ++i)
	{
		if(tmd_file->contents[i].index == 0)
		{
			*bootcontent = tmd_file->contents[i].cid;
			strncpy(app, fmt("/title/%08x/%08x/content/%08x.app", 
					TITLE_UPPER(title), TITLE_LOWER(title), *bootcontent), ISFS_MAXPATH);
			ret = true;
			break;
		}
	}

	free(data);

	return ret;
}

void Channels::GetBanner(u64 title, bool imetOnly)
{
	u32 cid = 0;
	CurrentBanner.ClearBanner();
	char app[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	if(!GetAppNameFromTmd(title, app, &cid))
		return;
	CurrentBanner.GetBanner(app, imetOnly);
}

bool Channels::GetChannelNameFromApp(u64 title, wchar_t* name, int language)
{
	bool ret = false;

	if (language > CONF_LANG_KOREAN)
		language = CONF_LANG_ENGLISH;

	GetBanner(title, true);
	if(CurrentBanner.IsValid())
	{
		ret = CurrentBanner.GetName(name, language);
		CurrentBanner.ClearBanner();
	}

	return ret;
}

int Channels::GetLanguage(const char *lang)
{
	if (strncmp(lang, "JP", 2) == 0) return CONF_LANG_JAPANESE;
	else if (strncmp(lang, "EN", 2) == 0) return CONF_LANG_ENGLISH;
	else if (strncmp(lang, "DE", 2) == 0) return CONF_LANG_GERMAN;
	else if (strncmp(lang, "FR", 2) == 0) return CONF_LANG_FRENCH;
	else if (strncmp(lang, "ES", 2) == 0) return CONF_LANG_SPANISH;
	else if (strncmp(lang, "IT", 2) == 0) return CONF_LANG_ITALIAN;
	else if (strncmp(lang, "NL", 2) == 0) return CONF_LANG_DUTCH;
	else if (strncmp(lang, "ZHTW", 4) == 0) return CONF_LANG_TRAD_CHINESE;
	else if (strncmp(lang, "ZH", 2) == 0) return CONF_LANG_SIMP_CHINESE;
	else if (strncmp(lang, "KO", 2) == 0) return CONF_LANG_KOREAN;
	
	return CONF_LANG_ENGLISH; // Default to EN
}

void Channels::Search()
{
	u32 count;
	u64 *list = NULL;
	if(!neek2o() && NANDemuView)
		list = NandHandle.GetChannels(&count);
	else
		list = GetChannelList(&count);
	if(list == NULL)
		return;

	int language = langCode.size() == 0 ? CONF_GetLanguage() : GetLanguage(langCode.c_str());

	for(u32 i = 0; i < count; i++)
	{
		u32 Type = TITLE_UPPER(list[i]);
		if(Type == SYSTEM_CHANNELS || Type == DOWNLOADED_CHANNELS || Type == GAME_CHANNELS)
		{
			u32 Title = TITLE_LOWER(list[i]);
			if(Title == RF_NEWS_CHANNEL || Title == RF_FORECAST_CHANNEL)
				continue; //skip region free news and forecast channel
			Channel CurrentChan;
			memset(&CurrentChan, 0, sizeof(Channel));
			if(GetChannelNameFromApp(list[i], CurrentChan.name, language))
			{
				CurrentChan.title = list[i];
				memcpy(CurrentChan.id, &Title, sizeof(CurrentChan.id));
				this->push_back(CurrentChan);
			}
		}
	}
	free(list);
}

wchar_t * Channels::GetName(int index)
{
	if (index < 0 || index > (int)Count() - 1)
	{
		return (wchar_t *) "";
	}
	return this->at(index).name;
}

u32 Channels::Count() 
{ 
	return this->size(); 
}

const char *Channels::GetId(int index)
{
	if(index < 0 || index > (int)Count() - 1)
		return "";
	return this->at(index).id;
}

u64 Channels::GetTitle(int index)
{
	if (index < 0 || index > (int)Count() - 1) return 0;
	return this->at(index).title;
}

Channel * Channels::GetChannel(int index)
{
	if (index < 0 || index > (int)Count() - 1) return NULL;
	return &this->at(index);
}
