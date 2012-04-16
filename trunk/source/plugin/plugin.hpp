
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

class Plugin
{
public:
	bool AddPlugin(Config &plugin);
	u8* GetBannerSound(u32 magic);
	u32 GetBannerSoundSize();
	char* GetDolName(u32 magic);
	bool UseReturnLoader(u32 magic);
	void init(string);
	void Cleanup();
	void EndAdd();
	safe_vector<dir_discHdr> ParseScummvmINI(Config &ini, string Device);
private:
	safe_vector<u8*> BannerSound;
	safe_vector<u32> BannerSoundSize;
	safe_vector<u32> magicWords;
	safe_vector<string> DolName;
	safe_vector<u32> caseColors;
	safe_vector<bool> ReturnLoader;
	
	u8 banner_pos;
	string pluginsDir;
	bool adding;
};
#endif
