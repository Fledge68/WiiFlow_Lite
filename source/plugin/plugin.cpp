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
#include "gecko/gecko.hpp"
#include "devicemounter/PartitionHandle.h"
#include "devicemounter/DeviceHandler.hpp"
#include "types.h"
#include "crc32.h"

Plugin m_plugin;
void Plugin::init(const string& m_pluginsDir)
{
	PluginMagicWord[8] = '\0';
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
	Plugins.clear();
	adding = true;
}

bool Plugin::AddPlugin(Config &plugin)
{
	if(!adding)
		return false;

	PluginOptions NewPlugin;
	NewPlugin.DolName = plugin.getString(PLUGIN_INI_DEF, "dolFile");
	NewPlugin.coverFolder = plugin.getString(PLUGIN_INI_DEF, "coverFolder");
	NewPlugin.magicWord = strtoul(plugin.getString(PLUGIN_INI_DEF, "magic").c_str(), NULL, 16);
	NewPlugin.caseColor = strtoul(plugin.getString(PLUGIN_INI_DEF, "coverColor").c_str(), NULL, 16);
	NewPlugin.Args = plugin.getStrings(PLUGIN_INI_DEF, "arguments", '|');
	string PluginName = plugin.getString(PLUGIN_INI_DEF, "displayname");
	if(PluginName.size() < 2)
	{
		PluginName = NewPlugin.DolName;
		PluginName.erase(PluginName.end() - 4, PluginName.end());
	}
	NewPlugin.DisplayName.fromUTF8(PluginName.c_str());
	NewPlugin.consoleCoverID = plugin.getString(PLUGIN_INI_DEF,"consoleCoverID");

	const char *bannerfilepath = fmt("%s/%s", pluginsDir.c_str(), plugin.getString(PLUGIN_INI_DEF,"bannerSound").c_str());
	FILE *fp = fopen(bannerfilepath, "rb");
	if(fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		NewPlugin.BannerSound = string(bannerfilepath);
		NewPlugin.BannerSoundSize = ftell(fp);
		rewind(fp);
		fclose(fp);
	}
	else
		NewPlugin.BannerSoundSize = 0;
	Plugins.push_back(NewPlugin);
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

u8* Plugin::GetBannerSound(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
	{
		u8 *FileReadBuffer = NULL;
		FILE *fp = fopen(Plugins[Plugin_Pos].BannerSound.c_str(), "rb");
		if(fp)
		{
			FileReadBuffer = (u8*)MEM2_alloc(Plugins[Plugin_Pos].BannerSoundSize);
			fread(FileReadBuffer, 1, Plugins[Plugin_Pos].BannerSoundSize, fp);
			fclose(fp);
		}
		return FileReadBuffer;
	}
	return NULL;
}

u32 Plugin::GetBannerSoundSize()
{
	//We call that directly after GetBannerSound, so no need to search for the magic again
	if(Plugin_Pos >= 0)
		return Plugins[Plugin_Pos].BannerSoundSize;
	return 0;
}

const char *Plugin::GetDolName(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return Plugins[Plugin_Pos].DolName.c_str();
	return NULL;
}

const char *Plugin::GetCoverFolderName(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return Plugins[Plugin_Pos].coverFolder.c_str();
	return NULL;
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
		strncpy(PluginMagicWord, fmt("%08x", Plugins[pos].magicWord), 8);
		if(ForceMode == 1)
			cfg.setBool(PLUGIN_INI_DEF, PluginMagicWord, false);
		else if(ForceMode == 2)
			cfg.setBool(PLUGIN_INI_DEF, PluginMagicWord, true);
		else
			cfg.setBool(PLUGIN_INI_DEF, PluginMagicWord, cfg.getBool(PLUGIN_INI_DEF, PluginMagicWord) ? false : true);
	}
}

const vector<bool> &Plugin::GetEnabledPlugins(Config &cfg)
{
	enabledPlugins.clear();
	u8 enabledPluginsNumber = 0;
	for(u8 i = 0; i < Plugins.size(); i++)
	{
		strncpy(PluginMagicWord, fmt("%08x", Plugins[i].magicWord), 8);
		if(cfg.getBool(PLUGIN_INI_DEF, PluginMagicWord, true))
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

vector<dir_discHdr> Plugin::ParseScummvmINI(Config &ini, const char *Device, u32 MagicWord)
{
	gprintf("Parsing scummvm.ini\n");
	vector<dir_discHdr> gameHeader;
	if(!ini.loaded())
		return gameHeader;

	const char *GameDomain = ini.firstDomain().c_str();
	dir_discHdr ListElement;
	while(1)
	{
		if(strlen(GameDomain) < 2)
			break;
		const char *GameName = ini.getString(GameDomain, "description").c_str();
		if(strlen(GameName) < 2 || strncasecmp(Device, ini.getString(GameDomain, "path").c_str(), 2) != 0)
		{
			GameDomain = ini.nextDomain().c_str();
			continue;
		}
		memset((void*)&ListElement, 0, sizeof(dir_discHdr));
		strncpy(ListElement.id, PLUGIN_INI_DEF, 6);
		ListElement.casecolor = Plugins.back().caseColor;
		mbstowcs(ListElement.title, GameName, 63);
		strncpy(ListElement.path, GameDomain, sizeof(ListElement.path));
		gprintf("Found: %s\n", GameDomain);
		ListElement.settings[0] = MagicWord;
		ListElement.type = TYPE_PLUGIN;
		gameHeader.push_back(ListElement);
		GameDomain = ini.nextDomain().c_str();
	}
	return gameHeader;
}

vector<string> Plugin::CreateArgs(const char *device, const char *path, 
						const char *title, const char *loader, u32 magic)
{
	vector<string> args;
	Plugin_Pos = GetPluginPosition(magic);
	if(Plugin_Pos < 0)
		return args;
	for(vector<string>::const_iterator arg = Plugins[Plugin_Pos].Args.begin();
								arg != Plugins[Plugin_Pos].Args.end(); ++arg)
	{
		string Argument(*arg);
		if(Argument.find(PLUGIN_DEV) != string::npos)
			Argument.replace(Argument.find(PLUGIN_DEV), strlen(PLUGIN_DEV), device);
		if(Argument.find(PLUGIN_PATH) != string::npos)
			Argument.replace(Argument.find(PLUGIN_PATH), strlen(PLUGIN_PATH), path);
		if(Argument.find(PLUGIN_NAME) != string::npos)
			Argument.replace(Argument.find(PLUGIN_NAME), strlen(PLUGIN_NAME), title);
		if(Argument.find(PLUGIN_LDR) != string::npos)
			Argument.replace(Argument.find(PLUGIN_LDR), strlen(PLUGIN_LDR), loader);
		args.push_back(Argument);
	}
	return args;
}

string Plugin::GenerateCoverLink(dir_discHdr gameHeader, const string& constURL, Config &Checksums)
{
	string url(constURL);
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
	const string& cachedCRC = Checksums.getString("CHECKSUMS", gamePath);
	char crc_string[9];
	crc_string[8] = '\0';
	if(cachedCRC.size() == 8)
	{
		gprintf("CRC32 of %s is cached\n", gamePath);
		strncpy(crc_string, cachedCRC.c_str(), 8);
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
			strncpy(crc_string, fmt("%08x", (u32)__builtin_bswap32(buffer)), 8);
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
			strncpy(crc_string, fmt("%08x", (u32)__builtin_bswap32(buffer)), 8);
		}
		else
			strncpy(crc_string, fmt("%08x", crc32file(gameHeader.path)), 8);
		Checksums.setString("CHECKSUMS", gamePath, crc_string);
		Checksums.save();
	}
	url.replace(url.find(TAG_GAME_ID), strlen(TAG_GAME_ID), upperCase(crc_string).c_str());
	gprintf("URL: %s\n", url.c_str());
	return url;
}
