
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
static char* emptyChar = (char*)" ";

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
	for(u8 pos = 0; pos < Plugins.size(); pos++)
	{
		if(Plugins[pos].BannerSound != NULL)
			MEM2_free(Plugins[pos].BannerSound);
	}
}

bool Plugin::AddPlugin(Config &plugin)
{
	if(!adding)
		return false;

	PluginOptions NewPlugin;
	NewPlugin.DolName = plugin.getString("PLUGIN","dolFile","");
	NewPlugin.coverFolder = plugin.getString("PLUGIN","coverFolder","");
	sscanf(plugin.getString("PLUGIN","magic","").c_str(), "%08x", &NewPlugin.magicWord);
	sscanf(plugin.getString("PLUGIN","coverColor","").c_str(), "%08x", &NewPlugin.caseColor);
	NewPlugin.ReturnLoader = plugin.getBool("PLUGIN","ReturnLoader");

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
		NewPlugin.BannerSound = (u8*)FileReadBuffer;
		NewPlugin.BannerSoundSize = size;
		Plugins.push_back(NewPlugin);
		return true;
	}
	else
	{
		NewPlugin.BannerSound = 0;
		NewPlugin.BannerSoundSize = 0;
		Plugins.push_back(NewPlugin);
	}
	return false;
}

s8 Plugin::GetPluginPosition(u32 magic)
{
	for(u8 pos = 0; pos < Plugins.size(); pos++)
	{
		if(magic == Plugins[pos].magicWord)
			return pos;
	}
	return -1;
}

bool Plugin::UseReturnLoader(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return Plugins[Plugin_Pos].ReturnLoader;
	return false;
}

u8* Plugin::GetBannerSound(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return Plugins[Plugin_Pos].BannerSound;
	return NULL;
}

u32 Plugin::GetBannerSoundSize()
{
	//We call that directly after GetBannerSound, so no need to search for the magic again
	if(Plugin_Pos >= 0)
		return Plugins[Plugin_Pos].BannerSoundSize;
	return 0;
}

char* Plugin::GetDolName(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return (char*)Plugins[Plugin_Pos].DolName.c_str();
	return emptyChar;
}

char* Plugin::GetCoverFolderName(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return (char*)Plugins[Plugin_Pos].coverFolder.c_str();
	return emptyChar;
}

safe_vector<dir_discHdr> Plugin::ParseScummvmINI(Config &ini, string Device)
{
	gprintf("Parsing scummvm.ini\n");
	safe_vector<dir_discHdr> gameHeader;
	if(!ini.loaded())
		return gameHeader;
	string game(ini.firstDomain());
	dir_discHdr tmp;
	while(1)
	{
		if(game == emptyString)
			break;
		if(strncasecmp(game.c_str(), "/", 1) == 0 || 
		strncasecmp(ini.getString(game,"description").c_str(), "/", 1) == 0 ||
		strncasecmp(ini.getWString(game, "path").toUTF8().c_str(), Device.c_str(), 3) != 0)
		{
			game = ini.nextDomain();
			continue;
		}
		memset(&tmp, 0, sizeof(dir_discHdr));
		tmp.hdr.casecolor = Plugins.back().caseColor;
		wstringEx tmpString;
		tmpString.fromUTF8(ini.getString(game,"description").c_str());
		wcsncpy(tmp.title, tmpString.c_str(), 64);
		strncpy(tmp.path, game.c_str(), sizeof(tmp.path));
		gprintf("Found: %ls\n", tmp.title);
		tmp.hdr.magic = Plugins.back().magicWord;
		tmp.hdr.gc_magic = 0x4c4f4c4f;
		gameHeader.push_back(tmp);
		game = ini.nextDomain();
	}
	return gameHeader;
}
