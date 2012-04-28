
//============================================================================
// Name        : plugin.cpp
// Copyright   : 2012 FIX94
//============================================================================

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include "plugin.hpp"
#include "memory/mem2.hpp"
#include "gui/text.hpp"
#include "gecko/gecko.h"

static const std::string emptyString;

void Plugin::init(string m_pluginsDir)
{
	pluginsDir = m_pluginsDir;
	//Ready to add plugins
	adding = true;
}

void Plugin::EndAdd()
{
	adding = false;
}

void Plugin::Cleanup()
{
	for(banner_pos = 0; banner_pos < magicWords.size(); banner_pos++)
		MEM2_free(BannerSound[banner_pos]);
	BannerSound.clear();
	BannerSoundSize.clear();
	magicWords.clear();
	DolName.clear();
	banner_pos = 0;
	pluginsDir.erase(0, pluginsDir.size());
}

bool Plugin::AddPlugin(Config &plugin)
{
	if(!adding)
		return false;

	DolName.push_back(plugin.getString("PLUGIN","dolFile",""));
	u32 magic, caseColor;
	sscanf(plugin.getString("PLUGIN","magic","").c_str(), "%08x", &magic);
	magicWords.push_back(magic);
	sscanf(plugin.getString("PLUGIN","coverColor","").c_str(), "%08x", &caseColor);
	caseColors.push_back(caseColor);
	ReturnLoader.push_back(plugin.getBool("PLUGIN","ReturnLoader"));

	string bannerfilepath = sfmt("%s/%s", pluginsDir.c_str(), plugin.getString("PLUGIN","bannerSound","").c_str());
	ifstream infile;
	infile.open(bannerfilepath.c_str(), ios::binary);
	if(infile.is_open())
	{
		int size;
		infile.seekg(0, ios::end);
		size = infile.tellg();
		infile.seekg(0, ios::beg);
		//Don't free that, otherwise you would delete the sound
		char* FileReadBuffer = (char*)MEM2_alloc(size);
		infile.read(FileReadBuffer, size);
		BannerSound.push_back((u8*)FileReadBuffer);
		BannerSoundSize.push_back(size);
		return true;
	}
	else
	{
		BannerSound.push_back(0);
		BannerSoundSize.push_back(0);
	}
	return false;
}

bool Plugin::UseReturnLoader(u32 magic)
{
	for(u8 pos = 0; pos < magicWords.size(); pos++)
	{
		if(magic == magicWords[pos])
			return ReturnLoader[pos];
	}
	return false;
}

u8* Plugin::GetBannerSound(u32 magic)
{
	for(banner_pos = 0; banner_pos < magicWords.size(); banner_pos++)
	{
		if(magic == magicWords[banner_pos])
			return BannerSound[banner_pos];
	}
	return NULL;
}

u32 Plugin::GetBannerSoundSize()
{
	//We call that directly after GetBannerSound, so no need to search for the magic again
	if(BannerSoundSize[banner_pos] > 0)
		return BannerSoundSize[banner_pos];
	return 0;
}

char* Plugin::GetDolName(u32 magic)
{
	char *null = (char*)" ";
	for(banner_pos = 0; banner_pos < magicWords.size(); banner_pos++)
		if(magic == magicWords[banner_pos])
			return (char*)DolName[banner_pos].c_str();
	return null;
}

safe_vector<dir_discHdr> Plugin::ParseScummvmINI(Config &ini, string Device)
{
	gprintf("Parsing scummvm.ini\n");
	safe_vector<dir_discHdr> gameHeader;
	if(!ini.loaded())
		return gameHeader;
	string game = ini.firstDomain().c_str();
	if(Device.rfind("usb") != string::npos)
		Device.erase(3, 1);
	dir_discHdr tmp;
	while(1)
	{
		if(game == emptyString)
			break;
		if(strncasecmp(game.c_str(), "/", 1) == 0 || 
		strncasecmp(lowerCase(ini.getString(game,"description")).c_str(), "/", 1) == 0 ||
		lowerCase(ini.getStrings(game, "path", '/')[0]).rfind(Device.c_str()) == string::npos)
		{
			game = ini.nextDomain();
			continue;
		}
		memset(&tmp, 0, sizeof(dir_discHdr));
		tmp.hdr.casecolor = caseColors.back();
		wstringEx tmpString;
		tmpString.fromUTF8(ini.getString(game,"description").c_str());
		wcsncpy(tmp.title, tmpString.c_str(), 64);
		strncpy(tmp.path, game.c_str(), sizeof(tmp.path));
		gprintf("Found: %s\n", tmpString.c_str());
		tmp.hdr.magic = magicWords.back();
		tmp.hdr.gc_magic = 0x4c4f4c4f;
		gameHeader.push_back(tmp);
		game = ini.nextDomain();
	}
	return gameHeader;
}
