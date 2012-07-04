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
#ifndef _PLUGIN_HPP_
#define _PLUGIN_HPP_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "config/config.hpp"
#include "loader/disc.h"

using namespace std;

#define TAG_GAME_ID		"{gameid}"
#define TAG_LOC			"{loc}"
#define TAG_CONSOLE		"{console}"

struct PluginOptions
{
	u8 *BannerSound;
	u32 BannerSoundSize;
	u32 magicWord;
	string DolName;
	string coverFolder;
	string consoleCoverID;
	wstringEx DisplayName;
	u32 caseColor;
	bool ReturnLoader;
};

class Plugin
{
public:
	bool AddPlugin(Config &plugin);
	u8* GetBannerSound(u32 magic);
	u32 GetBannerSoundSize();
	char* GetDolName(u32 magic);
	char* GetCoverFolderName(u32 magic);
	string GenerateCoverLink(dir_discHdr gameHeader, string url, Config &Checksums);
	wstringEx GetPluginName(u8 pos);
	u32 getPluginMagic(u8 pos);
	bool PluginExist(u8 pos);
	void SetEnablePlugin(Config &cfg, u8 pos, u8 ForceMode = 0);
	vector<bool> GetEnabledPlugins(Config &cfg);
	bool UseReturnLoader(u32 magic);
	bool isScummVM(u32 magic);
	bool isMplayerCE(u32 magic);
	vector<string> CreateMplayerCEArguments(const char *src);
	void init(string);
	void Cleanup();
	void EndAdd();
	vector<dir_discHdr> ParseScummvmINI(Config &ini, string Device);

private:
	s8 GetPluginPosition(u32 magic);
	vector<PluginOptions> Plugins;
	s8 Plugin_Pos;
	string pluginsDir;
	bool adding;
};
#endif
