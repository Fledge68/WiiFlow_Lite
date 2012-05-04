
//============================================================================
// Name        : plugin.hpp
// Copyright   : 2012 FIX94
//============================================================================

#ifndef _PLUGIN_HPP_
#define _PLUGIN_HPP_

#include <fstream>
#include <iostream>
#include <string>

#include "config/config.hpp"
#include "safe_vector.hpp"
#include "loader/disc.h"

using namespace std;

struct PluginOptions
{
	u8 *BannerSound;
	u32 BannerSoundSize;
	u32 magicWord;
	string DolName;
	string coverFolder;
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
	bool UseReturnLoader(u32 magic);
	void init(string);
	void Cleanup();
	void EndAdd();
	safe_vector<dir_discHdr> ParseScummvmINI(Config &ini, string Device);
private:
	s8 GetPluginPosition(u32 magic);

	safe_vector<PluginOptions> Plugins;
	s8 Plugin_Pos;
	string pluginsDir;
	bool adding;
};
#endif
