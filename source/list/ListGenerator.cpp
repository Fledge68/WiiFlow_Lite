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
#include "gui/coverflow.hpp"
#include "gui/text.hpp"

ListGenerator m_gameList;
Config CustomTitles;
GameTDB gameTDB;

dir_discHdr ListElement;
void ListGenerator::Init(const char *settingsDir, const char *Language)
{
	if(settingsDir != NULL)
	{
		gameTDB_Path = fmt("%s/wiitdb.xml", settingsDir);
		CustomTitlesPath = fmt("%s/" CTITLES_FILENAME, settingsDir);
	}
	if(Language != NULL) gameTDB_Language = Language;
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

/*
static const u32 LIST_TMP_SIZE = 5000;
dir_discHdr *tmpList = NULL;
u32 tmpListPos = 0;
static void AddToList(const dir_discHdr *element)
{
	
}
*/
static void AddISO(const char *GameID, const char *GameTitle, const char *GamePath, 
							u32 GameColor, u8 Type)
{
	memset((void*)&ListElement, 0, sizeof(dir_discHdr));
	ListElement.index = m_gameList.size();
	if(GameID != NULL) strncpy(ListElement.id, GameID, 6);
	if(GamePath != NULL) strncpy(ListElement.path, GamePath, sizeof(ListElement.path) - 1);
	ListElement.casecolor = CustomTitles.getColor("COVERS", ListElement.id, GameColor).intVal();
	const char *CustomTitle = CustomTitles.getString("TITLES", ListElement.id).c_str();
	if(gameTDB.IsLoaded())
	{
		if(ListElement.casecolor == GameColor)
			ListElement.casecolor = gameTDB.GetCaseColor(ListElement.id);
		ListElement.wifi = gameTDB.GetWifiPlayers(ListElement.id);
		ListElement.players = gameTDB.GetPlayers(ListElement.id);
		if(CustomTitle == NULL || CustomTitle[0] == '\0')
			gameTDB.GetTitle(ListElement.id, CustomTitle);
	}
	if(!ValidColor(ListElement.casecolor))
		ListElement.casecolor = CoverFlow.InternalCoverColor(ListElement.id, GameColor);

	if(CustomTitle != NULL && CustomTitle[0] != '\0')
		mbstowcs(ListElement.title, CustomTitle, 63);
	else if(GameTitle != NULL)
		mbstowcs(ListElement.title, GameTitle, 63);
	Asciify(ListElement.title);

	ListElement.type = Type;
	m_gameList.push_back(ListElement);
}

static void Create_Wii_WBFS_List(wbfs_t *handle)
{
	for(u32 i = 0; i < wbfs_count_discs(handle); i++)
	{
		memset((void*)&wii_hdr, 0, sizeof(discHdr));
		s32 ret = wbfs_get_disc_info(handle, i, (u8*)&wii_hdr, sizeof(discHdr), NULL);
		if(ret == 0 && wii_hdr.magic == WII_MAGIC)
			AddISO((const char*)wii_hdr.id, (const char*)wii_hdr.title, 
					NULL, 0xFFFFFF, TYPE_WII_GAME);
	}
}

static void Create_Wii_EXT_List(char *FullPath)
{
	FILE *fp = fopen(FullPath, "rb");
	if(fp)
	{
		fseek(fp, strcasestr(FullPath, ".wbfs") != NULL ? 512 : 0, SEEK_SET);
		fread((void*)&wii_hdr, 1, sizeof(discHdr), fp);
		if(wii_hdr.magic == WII_MAGIC)
			AddISO((const char*)wii_hdr.id, (const char*)wii_hdr.title, 
					FullPath, 0xFFFFFF, TYPE_WII_GAME);
		fclose(fp);
	}
}

u8 gc_disc[1];
const char *FST_APPEND = "sys/boot.bin";
const u8 FST_APPEND_SIZE = strlen(FST_APPEND);
static void Create_GC_List(char *FullPath)
{
	FILE *fp = fopen(FullPath, "rb");
	if(!fp && strstr(FullPath, "/root") != NULL) //fst folder
	{
		*(strstr(FullPath, "/root") + 1) = '\0';
		if(strlen(FullPath) + FST_APPEND_SIZE < MAX_MSG_SIZE) strcat(FullPath, FST_APPEND);
		fp = fopen(FullPath, "rb");
	}
	if(fp)
	{
		fread((void*)&gc_hdr, 1, sizeof(gc_discHdr), fp);
		if(gc_hdr.magic == GC_MAGIC)
		{
			AddISO((const char*)gc_hdr.id, (const char*)gc_hdr.title,
					FullPath, 0x000000, TYPE_GC_GAME);
			/* Check for disc 2 */
			fseek(fp, 6, SEEK_SET);
			fread(gc_disc, 1, 1, fp);
			if(gc_disc[0])
			{
				wcslcat(m_gameList.back().title, L" disc 2", 63);
				m_gameList.back().settings[0] = 1;
			}
		}
		fclose(fp);
	}
}

const char *FolderTitle = NULL;
static void Create_Plugin_List(char *FullPath)
{
	memset((void*)&ListElement, 0, sizeof(dir_discHdr));

	strncpy(ListElement.path, FullPath, sizeof(ListElement.path) - 1);
	strncpy(ListElement.id, "PLUGIN", 6);

	FolderTitle = strrchr(FullPath, '/') + 1;
	*strrchr(FolderTitle, '.') = '\0';
	
	char PluginMagicWord[9];
	memset(PluginMagicWord, 0, sizeof(PluginMagicWord));
	strncpy(PluginMagicWord, fmt("%08x", m_gameList.Magic), 8);
	const char *CustomTitle = CustomTitles.getString(PluginMagicWord, FolderTitle).c_str();
	if(CustomTitle != NULL && CustomTitle[0] != '\0')
		mbstowcs(ListElement.title, CustomTitle, 63);
	else	
		mbstowcs(ListElement.title, FolderTitle, 63);
	Asciify(ListElement.title);

	ListElement.settings[0] = m_gameList.Magic; //Plugin magic
	ListElement.casecolor = m_gameList.Color;
	ListElement.type = TYPE_PLUGIN;
	m_gameList.push_back(ListElement);
}

static void Create_Homebrew_List(char *FullPath)
{
	if(strcasestr(FullPath, "boot.") == NULL)
		return;
	memset((void*)&ListElement, 0, sizeof(dir_discHdr));
	ListElement.index = m_gameList.size();
	*strrchr(FullPath, '/') = '\0';
	strncpy(ListElement.path, FullPath, sizeof(ListElement.path) - 1);
	strncpy(ListElement.id, "HB_APP", 6);

	FolderTitle = strrchr(FullPath, '/') + 1;
	ListElement.casecolor = CustomTitles.getColor("COVERS", FolderTitle, 0xFFFFFF).intVal();
	const string &CustomTitle = CustomTitles.getString("TITLES", FolderTitle);
	if(CustomTitle.size() > 0)
		mbstowcs(ListElement.title, CustomTitle.c_str(), 63);
	else
		mbstowcs(ListElement.title, FolderTitle, 63);
	Asciify(ListElement.title);

	ListElement.type = TYPE_HOMEBREW;
	m_gameList.push_back(ListElement);
}

Channel *chan = NULL;
static void Create_Channel_List()
{
	for(u32 i = 0; i < ChannelHandle.Count(); i++)
	{
		chan = ChannelHandle.GetChannel(i);
		if(chan->id == NULL) 
			continue; // Skip invalid channels
		memset((void*)&ListElement, 0, sizeof(dir_discHdr));
		ListElement.index = m_gameList.size();
		ListElement.settings[0] = TITLE_UPPER(chan->title);
		ListElement.settings[1] = TITLE_LOWER(chan->title);
		strncpy(ListElement.id, chan->id, 4);
		ListElement.casecolor = CustomTitles.getColor("COVERS", ListElement.id, 0xFFFFFF).intVal();
		const char *CustomTitle = CustomTitles.getString("TITLES", ListElement.id).c_str();
		if(gameTDB.IsLoaded())
		{
			if(ListElement.casecolor == 0xFFFFFF)
				ListElement.casecolor = gameTDB.GetCaseColor(ListElement.id);
			ListElement.wifi = gameTDB.GetWifiPlayers(ListElement.id);
			ListElement.players = gameTDB.GetPlayers(ListElement.id);
			if(CustomTitle == NULL || CustomTitle[0] == '\0')
				gameTDB.GetTitle(ListElement.id, CustomTitle);
		}
		if(CustomTitle != NULL && CustomTitle[0] != '\0')
			mbstowcs(ListElement.title, CustomTitle, 63);
		else
			wcsncpy(ListElement.title, chan->name, 64);
		ListElement.type = TYPE_CHANNEL;
		m_gameList.push_back(ListElement);
	}
}

void ListGenerator::CreateList(u32 Flow, u32 Device, const string& Path, const vector<string>& FileTypes, 
								const string& DBName, bool UpdateCache)
{
	if(!DBName.empty())
	{
		if(UpdateCache)
			fsop_deleteFile(DBName.c_str());
		else
		{
			CCache(*this, DBName, LOAD);
			if(!this->empty())
				return;
			fsop_deleteFile(DBName.c_str());
		}
	}
	//if(Flow != COVERFLOW_PLUGIN)
		OpenConfigs();
	if(Flow == COVERFLOW_USB)
	{
		if(DeviceHandle.GetFSType(Device) == PART_FS_WBFS)
			Create_Wii_WBFS_List(DeviceHandle.GetWbfsHandle(Device));
		else
			GetFiles(Path.c_str(), FileTypes, Create_Wii_EXT_List, false);
	}
	else if(Flow == COVERFLOW_CHANNEL)
	{
		ChannelHandle.Init(gameTDB_Language);
		Create_Channel_List();
	}
	else if(DeviceHandle.GetFSType(Device) != PART_FS_WBFS)
	{
		if(Flow == COVERFLOW_DML)
			GetFiles(Path.c_str(), FileTypes, Create_GC_List, true);
		else if(Flow == COVERFLOW_PLUGIN)
			GetFiles(Path.c_str(), FileTypes, Create_Plugin_List, false, 30);
		else if(Flow == COVERFLOW_HOMEBREW)
			GetFiles(Path.c_str(), FileTypes, Create_Homebrew_List, false);
	}
	CloseConfigs();
	if(!this->empty() && !DBName.empty()) /* Write a new Cache */
		CCache(*this, DBName, SAVE);
}

static inline bool IsFileSupported(const char *File, const vector<string>& FileTypes)
{
	for(vector<string>::const_iterator cmp = FileTypes.begin(); cmp != FileTypes.end(); ++cmp)
	{
		if(strcasecmp(File, cmp->c_str()) == 0)
			return true;
	}
	return false;
}

const char *NewFileName = NULL;
char *FullPathChar = NULL;
dirent *pent = NULL;
DIR *pdir = NULL;
void GetFiles(const char *Path, const vector<string>& FileTypes, 
				FileAdder AddFile, bool CompareFolders, u32 max_depth, u32 depth)
{
	vector<string> SubPaths;

	pdir = opendir(Path);
	if(pdir == NULL)
		return;
	while((pent = readdir(pdir)) != NULL)
	{
		if(pent->d_name[0] == '.')
			continue;
		FullPathChar = fmt("%s/%s", Path, pent->d_name);
		if(pent->d_type == DT_DIR)
		{
			if(CompareFolders && IsFileSupported(pent->d_name, FileTypes))
			{
				AddFile(FullPathChar);
				continue;
			}
			else if(depth < max_depth) //thanks libntfs (fail opendir) and thanks seekdir (slowass speed)
				SubPaths.push_back(FullPathChar);
		}
		else if(pent->d_type == DT_REG)
		{
			NewFileName = strrchr(pent->d_name, '.');
			if(NewFileName == NULL) NewFileName = pent->d_name;
			if(IsFileSupported(NewFileName, FileTypes))
			{
				AddFile(FullPathChar);
				continue;
			}
		}
	}
	closedir(pdir);
	for(vector<string>::const_iterator p = SubPaths.begin(); p != SubPaths.end(); ++p)
		GetFiles(p->c_str(), FileTypes, AddFile, CompareFolders, max_depth, depth + 1);
	SubPaths.clear();
}
