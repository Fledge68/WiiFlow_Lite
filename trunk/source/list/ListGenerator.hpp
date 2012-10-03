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
#ifndef _LISTGENERATOR_HPP_
#define _LISTGENERATOR_HPP_

#include <string>
#include <vector>
#include <stdio.h>

#include "types.h"
#include "config/config.hpp"
#include "loader/wbfs.h"
#include "loader/disc.h"
#include "gui/GameTDB.hpp"

using namespace std;

class ListGenerator : public vector<dir_discHdr>
{
public:
	void Init(string settingsDir, string Language);
	void Cleanup();
	void CreateList(u32 Flow, u32 Device, string Path, vector<string> FileTypes, 
				string DBName, bool UpdateCache, u32 Color = 0, u32 Magic = 0);
private:
	void Create_Wii_WBFS_List(wbfs_t *handle);
	void Create_Wii_EXT_List(string Path, vector<string> FileTypes);
	void Create_GC_List(string Path, vector<string> FileTypes);
	void Create_Plugin_List(string Path, vector<string> FileTypes, u32 Color, u32 Magic);
	void Create_Homebrew_List(string Path, vector<string> FileTypes);
	void CreateChannelList();
	void GetFiles(const char *Path, vector<string> FileTypes, vector<string> *FileList, 
				bool gc = false, u32 max_depth = 2, u32 depth = 1);
	void AddISO(const char *GameID, const char *GameTitle, const char *GamePath, u32 GameColor, u8 Type);
	void OpenConfigs();
	void CloseConfigs();
	string gameTDB_Path;
	string CustomTitlesPath;
	string gameTDB_Language;
	GameTDB gameTDB;
	Config CustomTitles;
};

#endif /*_LISTGENERATOR_HPP_*/
