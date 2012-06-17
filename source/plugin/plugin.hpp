
//============================================================================
// Name        : plugin.hpp
// Copyright   : 2012 FIX94
//============================================================================

#ifndef _PLUGIN_HPP_
#define _PLUGIN_HPP_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "config/config.hpp"
#include "loader/disc.h"

#define SWAP32(x)		((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))

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
