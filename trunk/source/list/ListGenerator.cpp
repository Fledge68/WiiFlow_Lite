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
#include <dirent.h>
#include <unistd.h>
#include "ListGenerator.hpp"
#include "cache.hpp"
#include "channel/channels.h"
#include "devicemounter/DeviceHandler.hpp"
#include "fileOps/fileOps.h"
#include "gui/text.hpp"

ListGenerator m_gameList;

void ListGenerator::Init(const char *settingsDir, const char *Language)
{
	gameTDB_Path = fmt("%s/wiitdb.xml", settingsDir);
	CustomTitlesPath = fmt("%s/" CTITLES_FILENAME, settingsDir);
	gameTDB_Language = Language;
}

void ListGenerator::Cleanup()
{
	this->clear(); //clear gamelist
}

void ListGenerator::OpenConfigs()
{
	gameTDB.OpenFile(gameTDB_Path.c_str());
	if(gameTDB.IsLoaded())
		gameTDB.SetLanguageCode(gameTDB_Language.c_str());
	CustomTitles.load(CustomTitlesPath.c_str());
}

void ListGenerator::CloseConfigs()
{
	if(gameTDB.IsLoaded())
		gameTDB.CloseFile();
	if(CustomTitles.loaded())
		CustomTitles.unload();
}

void ListGenerator::CreateList(u32 Flow, u32 Device, const string& Path,
const vector<string>& FileTypes, const string& DBName, bool UpdateCache, u32 Color, u32 Magic)
{
	Cleanup();
	if(UpdateCache)
	{
		gprintf("Force Update Cache\n");
		fsop_deleteFile(DBName.c_str());
	}
	else
	{
		CCache(*this, DBName, LOAD);
		if(!this->empty())
			return;
		fsop_deleteFile(DBName.c_str());
	}
	OpenConfigs();
	if(Flow == COVERFLOW_USB)
	{
		if(DeviceHandle.GetFSType(Device) == PART_FS_WBFS)
			Create_Wii_WBFS_List(DeviceHandle.GetWbfsHandle(Device));
		else
			Create_Wii_EXT_List(Path, FileTypes);
	}
	else if(Flow == COVERFLOW_CHANNEL)
		CreateChannelList();
	else if(DeviceHandle.GetFSType(Device) != PART_FS_WBFS)
	{
		if(Flow == COVERFLOW_DML)
			Create_GC_List(Path, FileTypes);
		else if(Flow == COVERFLOW_PLUGIN)
			Create_Plugin_List(Path, FileTypes, Color, Magic);
		else if(Flow == COVERFLOW_HOMEBREW)
			Create_Homebrew_List(Path, FileTypes);
	}
	CloseConfigs();
	if(!this->empty()) /* Write a new Cache */
		CCache(*this, DBName, SAVE);
}

void ListGenerator::Create_Wii_WBFS_List(wbfs_t *handle)
{
	for(u32 i = 0; i < wbfs_count_discs(handle); i++)
	{
		memset((void*)&WiiGameHeader, 0, sizeof(discHdr));
		s32 ret = wbfs_get_disc_info(handle, i, (u8*)&WiiGameHeader, sizeof(discHdr), NULL);
		if(ret == 0 && WiiGameHeader.magic == WII_MAGIC)
			AddISO((const char*)WiiGameHeader.id, (const char*)WiiGameHeader.title, 
					NULL, 1, TYPE_WII_GAME);
	}
}

void ListGenerator::Create_Wii_EXT_List(const string& Path, const vector<string>& FileTypes)
{
	GetFiles(Path.c_str(), FileTypes, InternalList, false);
	for(vector<string>::iterator Name = InternalList.begin(); Name != InternalList.end(); Name++)
	{
		memset((void*)&WiiGameHeader, 0, sizeof(discHdr));
		FILE *fp = fopen(Name->c_str(), "rb");
		if(fp)
		{
			fseek(fp, strcasestr(Name->c_str(), ".wbfs") != NULL ? 512 : 0, SEEK_SET);
			fread((void*)&WiiGameHeader, 1, sizeof(discHdr), fp);
			if(WiiGameHeader.magic == WII_MAGIC)
				AddISO((const char*)WiiGameHeader.id, (const char*)WiiGameHeader.title, 
						Name->c_str(), 1, TYPE_WII_GAME);
			fclose(fp);
		}
	}
	InternalList.clear();
}

void ListGenerator::Create_GC_List(const string& Path, const vector<string>& FileTypes)
{
	GetFiles(Path.c_str(), FileTypes, InternalList, true);
	for(vector<string>::iterator Name = InternalList.begin(); Name != InternalList.end(); Name++)
	{
		memset((void*)&GCGameHeader, 0, sizeof(gc_discHdr));
		FILE *fp = fopen(Name->c_str(), "rb");
		if(!fp && Name->find("root") != string::npos) //fst folder
		{
			Name->erase(Name->begin() + Name->find("root"), Name->end());
			Name->append("sys/boot.bin");
			gprintf("FST Name: %s\n", Name->c_str());
			fp = fopen(Name->c_str(), "rb");
		}
		if(fp)
		{
			fread((void*)&GCGameHeader, 1, sizeof(gc_discHdr), fp);
			if(GCGameHeader.magic == GC_MAGIC)
				AddISO((const char*)GCGameHeader.id, (const char*)GCGameHeader.title,
						Name->c_str(), 0, TYPE_GC_GAME);
			fclose(fp);
		}
	}
	InternalList.clear();
}

void ListGenerator::Create_Plugin_List(const string& Path, const vector<string>& FileTypes, 
									u32 Color, u32 Magic)
{
	dir_discHdr ListElement;
	GetFiles(Path.c_str(), FileTypes, InternalList, false, 30);
	for(vector<string>::const_iterator Name = InternalList.begin(); Name != InternalList.end(); ++Name)
	{
		memset((void*)&ListElement, 0, sizeof(dir_discHdr));
		ListElement.index = this->size();
		strncpy(ListElement.path, Name->c_str(), sizeof(ListElement.path));
		strncpy(ListElement.id, "PLUGIN", 6);

		string Title(Name->begin() + Name->find_last_of('/') + 1, Name->begin() + Name->find_last_of('.'));
		mbstowcs(ListElement.title, Title.c_str(), 63);
		Asciify(ListElement.title);

		ListElement.settings[0] = Magic; //Plugin magic
		ListElement.casecolor = Color;
		ListElement.type = TYPE_PLUGIN;
		this->push_back(ListElement);
	}
	InternalList.clear();
}

void ListGenerator::Create_Homebrew_List(const string& Path, const vector<string>& FileTypes)
{
	dir_discHdr ListElement;
	GetFiles(Path.c_str(), FileTypes, InternalList, false, 4);
	for(vector<string>::iterator Name = InternalList.begin(); Name != InternalList.end(); Name++)
	{
		if(Name->find("boot.") == string::npos)
			continue;
		memset((void*)&ListElement, 0, sizeof(dir_discHdr));
		ListElement.index = this->size();
		Name->erase(Name->begin() + Name->find_last_of('/'), Name->end());
		strncpy(ListElement.path, Name->c_str(), sizeof(ListElement.path));
		strncpy(ListElement.id, "HB_APP", 6);

		string FolderTitle(Name->begin() + Name->find_last_of('/') + 1, Name->end());
		ListElement.casecolor = CustomTitles.getColor("COVERS", FolderTitle.c_str(), 1).intVal();
		string CustomTitle = CustomTitles.getString("TITLES", FolderTitle.c_str());		
		if(CustomTitle.size() > 0)
			mbstowcs(ListElement.title, CustomTitle.c_str(), 63);
		else
			mbstowcs(ListElement.title, FolderTitle.c_str(), 63);
		Asciify(ListElement.title);

		ListElement.type = TYPE_HOMEBREW;
		this->push_back(ListElement);
		continue;
	}
}

void ListGenerator::CreateChannelList()
{
	u32 GameColor = 1;
	Channels ChannelHandle;
	dir_discHdr ListElement;
	ChannelHandle.Init(0, gameTDB_Language, true);
	for(int i = 0; i < ChannelHandle.Count(); i++)
	{
		Channel *chan = ChannelHandle.GetChannel(i);
		if(chan->id == NULL) 
			continue; // Skip invalid channels
		memset((void*)&ListElement, 0, sizeof(dir_discHdr));
		ListElement.index = this->size();
		ListElement.settings[0] = TITLE_UPPER(chan->title);
		ListElement.settings[1] = TITLE_LOWER(chan->title);
		strncpy(ListElement.id, chan->id, 4);
		ListElement.casecolor = CustomTitles.getColor("COVERS", ListElement.id, GameColor).intVal();
		string CustomTitle = CustomTitles.getString("TITLES", ListElement.id);
		if(gameTDB.IsLoaded())
		{
			if(ListElement.casecolor == GameColor)
				ListElement.casecolor = gameTDB.GetCaseColor(ListElement.id);
			if(CustomTitle.size() == 0)
				gameTDB.GetTitle(ListElement.id, CustomTitle);
			ListElement.wifi = gameTDB.GetWifiPlayers(ListElement.id);
			ListElement.players = gameTDB.GetPlayers(ListElement.id);
		}
		if(CustomTitle.size() > 0)
			mbstowcs(ListElement.title, CustomTitle.c_str(), 63);
		else
			wcsncpy(ListElement.title, chan->name, 64);
		ListElement.type = TYPE_CHANNEL;
		this->push_back(ListElement);
	}
}

void ListGenerator::AddISO(const char *GameID, const char *GameTitle, const char *GamePath, u32 GameColor, u8 Type)
{
	dir_discHdr ListElement;
	memset((void*)&ListElement, 0, sizeof(dir_discHdr));
	ListElement.index = this->size();
	strncpy(ListElement.id, GameID, 6);
	strncpy(ListElement.path, GamePath, sizeof(ListElement.path));
	ListElement.casecolor = CustomTitles.getColor("COVERS", ListElement.id, GameColor).intVal();
	string CustomTitle = CustomTitles.getString("TITLES", ListElement.id);
	if(gameTDB.IsLoaded())
	{
		if(ListElement.casecolor == GameColor)
			ListElement.casecolor = gameTDB.GetCaseColor(ListElement.id);
		if(CustomTitle.size() == 0)
			gameTDB.GetTitle(ListElement.id, CustomTitle);
		ListElement.wifi = gameTDB.GetWifiPlayers(ListElement.id);
		ListElement.players = gameTDB.GetPlayers(ListElement.id);
	}
	if(CustomTitle.size() > 0)
		mbstowcs(ListElement.title, CustomTitle.c_str(), 63);
	else
		mbstowcs(ListElement.title, GameTitle, 63);
	Asciify(ListElement.title);

	ListElement.type = Type;
	this->push_back(ListElement); //I am a vector! :P
}

bool ListGenerator::IsFileSupported(const char *File, const vector<string>& FileTypes)
{
	for(vector<string>::const_iterator cmp = FileTypes.begin(); cmp != FileTypes.end(); ++cmp)
	{
		if(strcasecmp(File, cmp->c_str()) == 0)
			return true;
	}
	return false;
}

void ListGenerator::GetFiles(const char *Path, const vector<string>& FileTypes, 
				vector<string>& FileList, bool CompareFolders, u32 max_depth, u32 depth)
{
	struct dirent *pent = NULL;
	vector<string> SubPaths;
	DIR *pdir = opendir(Path);
	if(pdir == NULL)
		return;
	while((pent = readdir(pdir)) != NULL)
	{
		if(strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;
		string CurrentItem = sfmt("%s/%s", Path, pent->d_name);
		if(pent->d_type == DT_DIR)
		{
			if(CompareFolders && IsFileSupported(pent->d_name, FileTypes))
			{
				FileList.push_back(CurrentItem);
				continue;
			}
			else if(depth < max_depth)
				SubPaths.push_back(CurrentItem); //thanks to libntfs for a complicated way
		}
		else if(pent->d_type == DT_REG)
		{
			const char *FileName = strstr(pent->d_name, ".");
			if(FileName == NULL) FileName = pent->d_name;
			if(IsFileSupported(FileName, FileTypes))
			{
				FileList.push_back(CurrentItem);
				continue;
			}
		}
	}
	closedir(pdir);
	for(vector<string>::iterator SubPath = SubPaths.begin(); SubPath != SubPaths.end(); SubPath++)
		GetFiles(SubPath->c_str(), FileTypes, FileList, CompareFolders, max_depth, depth + 1);
	SubPaths.clear();
}
