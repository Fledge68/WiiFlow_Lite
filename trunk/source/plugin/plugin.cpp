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
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "plugin.hpp"
#include "gui/text.hpp"
#include "gecko/gecko.h"
#include "devicemounter/PartitionHandle.h"
#include "devicemounter/DeviceHandler.hpp"
#include "types.h"
#include "crc32.h"

static const string emptyString;
static const string emptyString2("/");
static char* emptyChar = (char*)" ";
u32 ScummVM_magic;

void Plugin::init(string m_pluginsDir)
{
	ScummVM_magic = 0;
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
			free(Plugins[pos].BannerSound);
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

	string PluginName = plugin.getString("PLUGIN","displayname","");
	if(PluginName == emptyString || PluginName == emptyString2)
	{
		PluginName = NewPlugin.DolName;
		PluginName.erase(PluginName.end() - 4, PluginName.end());
	}
	NewPlugin.DisplayName.fromUTF8(PluginName.c_str());
	NewPlugin.consoleCoverID = plugin.getString("PLUGIN","consoleCoverID","");

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
		char* FileReadBuffer = (char*)malloc(size);
		infile.read(FileReadBuffer, size);
		NewPlugin.BannerSound = (u8*)FileReadBuffer;
		NewPlugin.BannerSoundSize = size;
		Plugins.push_back(NewPlugin);
		infile.close();
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

bool Plugin::PluginExist(u8 pos)
{
	if(pos < Plugins.size())
		return true;
	return false;
}

wstringEx Plugin::GetPluginName(u8 pos)
{
	return Plugins[pos].DisplayName;
}

void Plugin::SetEnablePlugin(Config &cfg, u8 pos, u8 ForceMode)
{
	if(pos < Plugins.size())
	{
		char PluginMagicWord[9];
		snprintf(PluginMagicWord, sizeof(PluginMagicWord), "%08x", Plugins[pos].magicWord);
		if(ForceMode == 1)
			cfg.setBool("PLUGIN", PluginMagicWord, false);
		else if(ForceMode == 2)
			cfg.setBool("PLUGIN", PluginMagicWord, true);
		else
			cfg.setBool("PLUGIN", PluginMagicWord, cfg.getBool("PLUGIN", PluginMagicWord) ? false : true);
	}
}

vector<bool> Plugin::GetEnabledPlugins(Config &cfg)
{
	vector<bool> enabledPlugins;
	char PluginMagicWord[9];
	u8 enabledPluginsNumber = 0;
	for(u8 i = 0; i < Plugins.size(); i++)
	{
		snprintf(PluginMagicWord, sizeof(PluginMagicWord), "%08x", Plugins[i].magicWord);
		if(cfg.getBool("PLUGIN", PluginMagicWord, true))
		{
			enabledPluginsNumber++;
			enabledPlugins.push_back(true);
		}
		else
			enabledPlugins.push_back(false);
	}
	if(enabledPluginsNumber == Plugins.size())
		enabledPlugins.clear();
	return enabledPlugins;
}

u32 Plugin::getPluginMagic(u8 pos)
{
	return Plugins[pos].magicWord;
}

vector<dir_discHdr> Plugin::ParseScummvmINI(Config &ini, string Device)
{
	gprintf("Parsing scummvm.ini\n");
	vector<dir_discHdr> gameHeader;
	if(!ini.loaded())
		return gameHeader;
	ScummVM_magic = Plugins[Plugins.size()-1].magicWord;

	string game(ini.firstDomain());
	string GameName;
	dir_discHdr tmp;
	while(1)
	{
		if(game == emptyString || game == emptyString2)
			break;
		GameName = ini.getString(game, "description");
		if(GameName == emptyString || GameName == emptyString2 ||
		strncasecmp(ini.getWString(game, "path").toUTF8().c_str(), Device.c_str(), 2) != 0)
		{
			game = ini.nextDomain();
			continue;
		}
		memset(&tmp, 0, sizeof(dir_discHdr));
		strncpy((char*)tmp.id, "PLUGIN", sizeof(tmp.id));
		tmp.casecolor = Plugins.back().caseColor;
		wstringEx tmpString;
		tmpString.fromUTF8(GameName.c_str());
		wcsncpy(tmp.title, tmpString.c_str(), 64);
		strncpy(tmp.path, game.c_str(), sizeof(tmp.path));
		gprintf("Found: %ls\n", tmp.title);
		tmp.settings[0] = Plugins.back().magicWord;
		tmp.type = TYPE_PLUGIN;
		gameHeader.push_back(tmp);
		game = ini.nextDomain();
	}
	return gameHeader;
}

/* Thanks to dimok for this */
vector<string> Plugin::CreateMplayerCEArguments(const char *filepath)
{
	vector<string> args;
	char dst[1024];

	int i = 0;
	char device[10];

	while(filepath[i] != ':')
	{
		device[i] = filepath[i];
		device[i+1] = 0;
		i++;
	}

	char * ptr = (char *) &filepath[i];

	while(ptr[0] != '/' || ptr[1] == '/')
		ptr++;

	if(strncmp(DeviceHandle.PathToFSName(filepath), "NTF", 3) == 0)
	{
		sprintf(dst, "ntfs:%s", ptr);
	}
	else if(strncmp(device, "usb", 3) == 0)
	{
		sprintf(dst, "usb:%s", ptr);
	}
	else
	{
		sprintf(dst, "%s:%s", device, ptr);
	}

	args.push_back(dst);
	return args;
}

bool Plugin::isMplayerCE(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return (Plugins[Plugin_Pos].magicWord == 0x4D504345);
	return false;
}

bool Plugin::isScummVM(u32 magic)
{
	return (magic == ScummVM_magic);
}

string Plugin::GenerateCoverLink(dir_discHdr gameHeader, string url, Config &Checksums)
{
	Plugin_Pos = GetPluginPosition(gameHeader.settings[0]);

	if(url.find(TAG_LOC) != url.npos)
 		url.replace(url.find(TAG_LOC), strlen(TAG_LOC), "EN");

	if(url.find(TAG_CONSOLE) != url.npos)
		url.replace(url.find(TAG_CONSOLE), strlen(TAG_CONSOLE), (Plugins[Plugin_Pos].consoleCoverID.size() ? Plugins[Plugin_Pos].consoleCoverID.c_str() : "nintendo"));	

	char gamePath[256];
	if(string(gameHeader.path).find_last_of("/") != string::npos)
		strncpy(gamePath, &gameHeader.path[string(gameHeader.path).find_last_of("/")+1], sizeof(gamePath));
	else
		strncpy(gamePath, gameHeader.path, sizeof(gamePath));
	string cachedCRC = Checksums.getString("CHECKSUMS", gamePath, emptyString);
	char crc_string[9];
	if(cachedCRC != emptyString)
	{
		gprintf("CRC32 of %s is cached\n", gamePath);
		snprintf(crc_string, sizeof(crc_string), "%s", cachedCRC.c_str());
	}
	else
	{
		gprintf("Generating CRC32 for %s\n", gamePath);
		u32 buffer;
		ifstream infile;
		if(strstr(gameHeader.path, ".zip") != NULL)
		{
			infile.open(gameHeader.path, ios::binary);
			infile.seekg(0x0e, ios::beg);
			infile.read((char*)&buffer, 8);
			infile.close();
			snprintf(crc_string, sizeof(crc_string), "%08x", (u32)__builtin_bswap32(buffer));
		}
		else if(strstr(gameHeader.path, ".7z") != NULL)
		{
			infile.open(gameHeader.path, ios::binary);
			infile.seekg(-8, ios::end);
			while(infile.tellg())
			{
				infile.read((char*)&buffer, 8);
				if(buffer == 0x00050111)
					break;
				infile.seekg(-9, ios::cur);
			}
			infile.seekg(-13, ios::cur);
			infile.read((char*)&buffer, 8);
			infile.close();
			snprintf(crc_string, sizeof(crc_string), "%08x", (u32)__builtin_bswap32(buffer));
		}
		else
			snprintf(crc_string, sizeof(crc_string), "%08x", crc32file(gameHeader.path));
		Checksums.setString("CHECKSUMS", gamePath, crc_string);
		Checksums.save();
	}
	url.replace(url.find(TAG_GAME_ID), strlen(TAG_GAME_ID), upperCase(crc_string).c_str());
	gprintf("URL: %s\n", url.c_str());
	return url;
}
