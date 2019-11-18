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
#include "loader/sys.h"

ListGenerator m_cacheList;
Config CustomTitles;
GameTDB gameTDB;
Config romNamesDB;
string platformName;
string pluginsDataDir;

dir_discHdr ListElement;
void ListGenerator::Init(const char *settingsDir, const char *Language, const char *plgnsDataDir)
{
	if(settingsDir != NULL)
	{
		gameTDB_Path = fmt("%s/wiitdb.xml", settingsDir);
		CustomTitlesPath = fmt("%s/" CTITLES_FILENAME, settingsDir);
	}
	if(Language != NULL) gameTDB_Language = Language;
	if(plgnsDataDir != NULL) pluginsDataDir = fmt("%s", plgnsDataDir);
}

void ListGenerator::Clear(void)
{
	m_cacheList.clear();
	vector<dir_discHdr>().swap(m_cacheList);
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

/* used for adding wii games to the list */
static void AddISO(const char *GameID, const char *GameTitle, const char *GamePath, 
							u32 GameColor, u8 Type)
{
	memset((void*)&ListElement, 0, sizeof(dir_discHdr));
	ListElement.index = m_cacheList.size();
	if(GameID != NULL) strncpy(ListElement.id, GameID, 6);
	if(GamePath != NULL) strncpy(ListElement.path, GamePath, sizeof(ListElement.path) - 1);
	ListElement.casecolor = CustomTitles.getColor("COVERS", ListElement.id, GameColor).intVal();
	char CustomTitle[64];
	memset(CustomTitle, 0, sizeof(CustomTitle));
	strncpy(CustomTitle, CustomTitles.getString("TITLES", ListElement.id).c_str(), 63);
	const char *gameTDB_Title = NULL;
	if(gameTDB.IsLoaded())
	{
		if(ListElement.casecolor == GameColor)
			ListElement.casecolor = gameTDB.GetCaseColor(ListElement.id);
		ListElement.wifi = gameTDB.GetWifiPlayers(ListElement.id);
		ListElement.players = gameTDB.GetPlayers(ListElement.id);
		if(strlen(CustomTitle) == 0)
			gameTDB.GetTitle(ListElement.id, gameTDB_Title);
	}
	if(!ValidColor(ListElement.casecolor))
		ListElement.casecolor = CoverFlow.InternalCoverColor(ListElement.id, GameColor);

	if(strlen(CustomTitle) > 0)
		mbstowcs(ListElement.title, CustomTitle, 63);
	else if(gameTDB_Title != NULL && gameTDB_Title[0] != '\0')
		mbstowcs(ListElement.title, gameTDB_Title, 63);
	else if(GameTitle != NULL)
		mbstowcs(ListElement.title, GameTitle, 63);
	Asciify(ListElement.title);

	ListElement.type = Type;
	m_cacheList.push_back(ListElement);
}

/* read wbfs partition to add wii games to the list */
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

/* add wii game iso(ntfs) or wbfs(fat) to the list. wbf1 and wbf2 are skipped and not added. */
static void Add_Wii_Game(char *FullPath)
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

/* add gamecube game to the list */
u8 gc_disc[1];
const char *FST_APPEND = "sys/boot.bin";
const u8 FST_APPEND_SIZE = strlen(FST_APPEND);
static const u8 CISO_MAGIC[8] = {'C','I','S','O',0x00,0x00,0x20,0x00};
static void Add_GameCube_Game(char *FullPath)
{
	u32 hdr_offset = 0x00;
	FILE *fp = fopen(FullPath, "rb");
	if(!fp && strstr(FullPath, "/root") != NULL) //fst folder (extracted game)
	{
		*(strstr(FullPath, "/root") + 1) = '\0';
		if(strlen(FullPath) + FST_APPEND_SIZE < MAX_MSG_SIZE) strcat(FullPath, FST_APPEND);
		fp = fopen(FullPath, "rb");
	}
	if(fp)
	{
		fread((void*)&gc_hdr, 1, sizeof(gc_discHdr), fp);
		//check for CISO disc image and change offset to read the true header
		if(!memcmp((void*)&gc_hdr, CISO_MAGIC, sizeof(CISO_MAGIC)))
		{
			hdr_offset = 0x8000;
			fseek(fp, hdr_offset, SEEK_SET);
			fread((void*)&gc_hdr, 1, sizeof(gc_discHdr), fp);
		}
		if(gc_hdr.magic == GC_MAGIC)
		{
			AddISO((const char*)gc_hdr.id, (const char*)gc_hdr.title,
					FullPath, 0x000000, TYPE_GC_GAME);
			/* Check for disc 2 */
			fseek(fp, hdr_offset + 0x06, SEEK_SET);
			fread(gc_disc, 1, 1, fp);
			if(gc_disc[0])
			{
				wcslcat(m_cacheList.back().title, L" disc 2", 63);
				m_cacheList.back().settings[0] = 1;
			}
		}
		fclose(fp);
	}
}

/* add homebrew boot.dol to the list */
static void Add_Homebrew_Dol(char *FullPath)
{
	if(strcasestr(FullPath, "boot.") == NULL)
		return;
	memset((void*)&ListElement, 0, sizeof(dir_discHdr));
	ListElement.index = m_cacheList.size();
	*strrchr(FullPath, '/') = '\0';
	strncpy(ListElement.path, FullPath, sizeof(ListElement.path) - 1);
	memcpy(ListElement.id, "HB_APP", 6);

	const char *FolderTitle = strrchr(FullPath, '/') + 1;
	ListElement.casecolor = CustomTitles.getColor("COVERS", FolderTitle, 0xFFFFFF).intVal();
	const string &CustomTitle = CustomTitles.getString("TITLES", FolderTitle);
	if(CustomTitle.size() > 0)
		mbstowcs(ListElement.title, CustomTitle.c_str(), 63);
	else
		mbstowcs(ListElement.title, FolderTitle, 63);
	Asciify(ListElement.title);

	ListElement.type = TYPE_HOMEBREW;
	m_cacheList.push_back(ListElement);
}

/* create channel list from nand or emu nand */
Channel *chan = NULL;
static void Create_Channel_List(bool realNAND)
{
	for(u32 i = 0; i < ChannelHandle.Count(); i++)
	{
		chan = ChannelHandle.GetChannel(i);
		if(chan->id == NULL) 
			continue; // Skip invalid channels
		memset((void*)&ListElement, 0, sizeof(dir_discHdr));
		ListElement.index = m_cacheList.size();
		ListElement.settings[0] = TITLE_UPPER(chan->title);
		ListElement.settings[1] = TITLE_LOWER(chan->title);
		if(chan->title == HBC_108)
			memcpy(ListElement.id, "JODI", 4);
		else
			strncpy(ListElement.id, chan->id, 4);
		ListElement.casecolor = CustomTitles.getColor("COVERS", ListElement.id, 0xFFFFFF).intVal();
		char CustomTitle[64];
		memset(CustomTitle, 0, sizeof(CustomTitle));
		strncpy(CustomTitle, CustomTitles.getString("TITLES", ListElement.id).c_str(), 63);
		const char *gameTDB_Title = NULL;
		if(gameTDB.IsLoaded())
		{
			if(ListElement.casecolor == 0xFFFFFF)
				ListElement.casecolor = gameTDB.GetCaseColor(ListElement.id);
			ListElement.wifi = gameTDB.GetWifiPlayers(ListElement.id);
			ListElement.players = gameTDB.GetPlayers(ListElement.id);
			if(strlen(CustomTitle) == 0)
				gameTDB.GetTitle(ListElement.id, gameTDB_Title);
		}
		if(strlen(CustomTitle) > 0)
			mbstowcs(ListElement.title, CustomTitle, 63);
		else if(gameTDB_Title != NULL && gameTDB_Title[0] != '\0')
			mbstowcs(ListElement.title, gameTDB_Title, 63);
		else
			wcsncpy(ListElement.title, chan->name, 64);
		if(realNAND)
			ListElement.type = TYPE_CHANNEL;
		else
			ListElement.type = TYPE_EMUCHANNEL;
		m_cacheList.push_back(ListElement);
	}
}

/* add plugin rom, song, or video to the list. */
static void Add_Plugin_Game(char *FullPath)
{
	/* Get roms's title without the extra ()'s or []'s */
	string ShortName = m_plugin.GetRomName(FullPath);
	//gprintf("shortName=%s\n", ShortName.c_str());
	
	/* get rom's ID */
	string romID = "";
	if(gameTDB.IsLoaded())
	{
		/* Get 6 character unique romID (from Screenscraper.fr) using shortName. if fails then use CRC or CD serial to get romID */
		romID = m_plugin.GetRomId(FullPath, m_cacheList.Magic, romNamesDB, pluginsDataDir.c_str(), platformName.c_str(), ShortName.c_str());
	}
	if(romID.empty())
		romID = "PLUGIN";
	//gprintf("romID=%s\n", romID.c_str());

	/* add rom to list */
	memset((void*)&ListElement, 0, sizeof(dir_discHdr));

	strncpy(ListElement.path, FullPath, sizeof(ListElement.path) - 1);
	strncpy(ListElement.id, romID.c_str(), 6);

	/* Get titles - Rom filename, custom title, and database xml title */
	const char *RomFilename = strrchr(FullPath, '/') + 1;
	*strrchr(RomFilename, '.') = '\0';
	
	string customTitle = CustomTitles.getString(m_plugin.PluginMagicWord, RomFilename, "");
	
	const char *gameTDB_Title = NULL;
	if(gameTDB.IsLoaded() && customTitle.empty())
		gameTDB.GetTitle(ListElement.id, gameTDB_Title, true);
	
	/* set the roms title */
	if(!customTitle.empty())
		mbstowcs(ListElement.title, customTitle.c_str(), 63);
	else if(gameTDB_Title != NULL && gameTDB_Title[0] != '\0')
		mbstowcs(ListElement.title, gameTDB_Title, 63);
	else
		mbstowcs(ListElement.title, RomFilename, 63);
	Asciify(ListElement.title);
	
	ListElement.settings[0] = m_cacheList.Magic; //Plugin magic
	ListElement.casecolor = m_cacheList.Color;
	ListElement.type = TYPE_PLUGIN;
	m_cacheList.push_back(ListElement);
}

/* note: scummvm games have list generator in plugin.cpp */
void ListGenerator::CreateRomList(Config &platform_cfg, const string& romsDir, const vector<string>& FileTypes, const string& DBName, bool UpdateCache)
{
	Clear();
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
	
	if(platform_cfg.loaded())
	{
		/* Search platform.ini to find plugin magic to get platformName */
		platformName = platform_cfg.getString("PLUGINS", m_plugin.PluginMagicWord);
		if(!platformName.empty())
		{
			/* check COMBINED for platform names that mean the same system just different region */
			/* some platforms have different names per country (ex. Genesis/Megadrive) */
			/* but we use only one platform name for both */
			string newName = platform_cfg.getString("COMBINED", platformName);
			if(newName.empty())
				platform_cfg.remove("COMBINED", platformName);
			else
				platformName = newName;
			
			/* Load rom names and crc database */
			romNamesDB.load(fmt("%s/%s/%s.ini", pluginsDataDir.c_str(), platformName.c_str(), platformName.c_str()));
			/* Load platform name.xml database to get game's info using the gameID */
			gameTDB.OpenFile(fmt("%s/%s/%s.xml", pluginsDataDir.c_str(), platformName.c_str(), platformName.c_str()));
			if(gameTDB.IsLoaded())
				gameTDB.SetLanguageCode(gameTDB_Language.c_str());
		}
	}
	CustomTitles.load(CustomTitlesPath.c_str());
	GetFiles(romsDir.c_str(), FileTypes, Add_Plugin_Game, false, 30);//wow 30 subfolders! really?
	CloseConfigs();
	romNamesDB.unload();
	if(!this->empty() && !DBName.empty()) /* Write a new Cache */
		CCache(*this, DBName, SAVE);
}
	
void ListGenerator::CreateList(u32 Flow, u32 Device, const string& Path, const vector<string>& FileTypes, 
								const string& DBName, bool UpdateCache)
{
	Clear();
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
	OpenConfigs();
	if(Flow == COVERFLOW_WII)
	{
		if(DeviceHandle.GetFSType(Device) == PART_FS_WBFS)
			Create_Wii_WBFS_List(DeviceHandle.GetWbfsHandle(Device));
		else
			GetFiles(Path.c_str(), FileTypes, Add_Wii_Game, false);
	}
	else if(Flow == COVERFLOW_CHANNEL)
	{
		ChannelHandle.Init(gameTDB_Language);
		if(Device == 9)
			Create_Channel_List(true);
		else
			Create_Channel_List(false);
	}
	else if(DeviceHandle.GetFSType(Device) != PART_FS_WBFS)
	{
		if(Flow == COVERFLOW_GAMECUBE)
			GetFiles(Path.c_str(), FileTypes, Add_GameCube_Game, true);// true means to look for a folder (/root)
		else if(Flow == COVERFLOW_HOMEBREW)
			GetFiles(Path.c_str(), FileTypes, Add_Homebrew_Dol, false);
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
			if(CompareFolders && IsFileSupported(pent->d_name, FileTypes))//if root folder for extracted gc games
			{
				AddFile(FullPathChar);
				continue;
			}
			else if(depth < max_depth && strcmp(pent->d_name, "samples") != 0) //skip samples folder in mame roms folder
				SubPaths.push_back(FullPathChar);
		}
		else if(pent->d_type == DT_REG)
		{
			NewFileName = strrchr(pent->d_name, '.');//the extension
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

/* create sourceflow list from current source_menu.ini */
void ListGenerator::createSFList(u8 maxBtns, Config &m_sourceMenuCfg, const string& sourceDir)
{
	Clear();
	char btn_selected[256];	
	for(u8 i = 0; i <= maxBtns; i++)
	{
		memset(btn_selected, 0, 256);
		strncpy(btn_selected, fmt("BUTTON_%i", i), 255);
		const char *source = m_sourceMenuCfg.getString(btn_selected, "source","").c_str();
		if(source == NULL)
			continue;
		const char *path = fmt("%s/%s", sourceDir.c_str(), m_sourceMenuCfg.getString(btn_selected, "image", "").c_str());
		
		memset((void*)&ListElement, 0, sizeof(dir_discHdr));
		ListElement.index = m_cacheList.size();
		memcpy(ListElement.id, "SOURCE", 6);
		strncpy(ListElement.path, path, sizeof(ListElement.path) - 1);
		ListElement.casecolor = 0xFFFFFF;
		ListElement.type = TYPE_SOURCE;		
		ListElement.settings[0] = i;
		//const char *title = m_sourceMenuCfg.getString(btn_selected, "title", fmt("title_%i", i)).c_str();
		char SourceTitle[64];
		memset(SourceTitle, 0, sizeof(SourceTitle));
		strncpy(SourceTitle, m_sourceMenuCfg.getString(btn_selected, "title", fmt("title_%i", i)).c_str(), 63);
		mbstowcs(ListElement.title, SourceTitle, 63);
		Asciify(ListElement.title);
		m_cacheList.push_back(ListElement);
	}
}