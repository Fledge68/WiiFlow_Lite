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
	void Init(const char *settingsDir, const char *Language);
	void CreateList(u32 Flow, u32 Device, const string& Path, const vector<string>& FileTypes, 
				const string& DBName, bool UpdateCache);
	u32 Color;
	u32 Magic;
private:
	void OpenConfigs();
	void CloseConfigs();
	string gameTDB_Path;
	string CustomTitlesPath;
	string gameTDB_Language;
};

typedef void (*FileAdder)(char *Path);
void GetFiles(const char *Path, const vector<string>& FileTypes, 
			FileAdder AddFile, bool CompareFolders, u32 max_depth = 2, u32 depth = 1);
extern ListGenerator m_gameList;

#endif /*_LISTGENERATOR_HPP_*/
