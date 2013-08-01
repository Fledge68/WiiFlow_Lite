/****************************************************************************
 * Copyright (C) 2013 FIX94
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
#include <algorithm>
#include "menu.hpp"
#include "channel/nand.hpp"
#include "defines.h"

extern const u8 blank_png[];
extern const u8 btnnext_png[];
extern const u8 btnnexts_png[];
extern const u8 btnprev_png[];
extern const u8 btnprevs_png[];

TexData m_explorerBg;
s16 entries[7];
s16 entries_sel[7];
s16 explorerLblSelFolder;
s16 explorerBtnSave;
s16 explorerBtnCancel;
s16 explorerBtnPrev;
s16 explorerBtnNext;
s16 explorerLblUser[4];

char file[MAX_FAT_PATH];
char dir[MAX_FAT_PATH];
char entries_char[6][NAME_MAX+1];
u8 explorer_partition = 0;
bool folderExplorer = false;
string folderPath = "";
string path = "";
vector<string> folderList;
vector<string> fileList;

void CMenu::_hideExplorer(bool instant)
{
	for(u8 i = 0; i < 7; ++i)
	{
		m_btnMgr.hide(entries[i], instant);
		m_btnMgr.hide(entries_sel[i], instant);
	}
	
	m_btnMgr.hide(explorerBtnNext, instant);
	m_btnMgr.hide(explorerBtnPrev, instant);
	m_btnMgr.hide(explorerLblSelFolder, instant);
	if(folderExplorer)
	{
		m_btnMgr.hide(explorerBtnSave, instant);
		m_btnMgr.hide(explorerBtnCancel, instant);
	}
	for(u8 i = 0; i < ARRAY_SIZE(explorerLblUser); ++i)
	{
		if(explorerLblUser[i] != -1)
			m_btnMgr.hide(explorerLblUser[i], instant);
	}
}

void CMenu::_showExplorer(void)
{
	_setBg(m_explorerBg, m_explorerBg);

	m_btnMgr.show(explorerBtnNext);
	m_btnMgr.show(explorerBtnPrev);
	m_btnMgr.show(explorerLblSelFolder);
	if(folderExplorer)
	{
		m_btnMgr.show(explorerBtnSave);
		m_btnMgr.show(explorerBtnCancel);
	}
	for(u8 i = 0; i < ARRAY_SIZE(explorerLblUser); ++i)
	{
		if(explorerLblUser[i] != -1)
			m_btnMgr.show(explorerLblUser[i]);
	}
	_refreshExplorer();
}

void CMenu::_Explorer(void)
{
	_showExplorer();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_PLUS_PRESSED)
		{
			_refreshExplorer(1);
		}
		else if(BTN_MINUS_PRESSED)
		{
			_refreshExplorer(-1);
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(explorerBtnNext))
			{
				_refreshExplorer(1);
			}
			else if(m_btnMgr.selected(explorerBtnPrev))
			{
				_refreshExplorer(-1);
			}
			else if(m_btnMgr.selected(explorerBtnSave))
			{
				//only when save is clicked do we set path to dir
				if(dir[0] != '\0')
					path = dir;
				break;
			}
			else if(m_btnMgr.selected(explorerBtnCancel))
				break;
			//if "..." is selected and path is not empty then go up(back) one folder
			else if(m_btnMgr.selected(entries_sel[0]) && dir[0] != '\0')
			{
				//remove last folder or device+partition
				if(strchr(dir, '/') != NULL)
				{
					*strrchr(dir, '/') = '\0';
					if(strchr(dir, '/') != NULL)
						*(strrchr(dir, '/')+1) = '\0';
				}
				folderPath = dir;
				//if dir is just device and : then folderpath empty
				if(folderPath.find_last_of("/") == string::npos)
					folderPath = "";
				else
				{
					if(folderPath.find_first_of("/") != folderPath.find_last_of("/"))
					{
						folderPath = folderPath.erase(folderPath.find_last_of("/"));
						while(folderPath.length() > 32)
						{
							if(folderPath.find_first_of("/") == string::npos)
								break;
							folderPath = folderPath.erase(0, folderPath.find_first_of("/")+1);
						}
						if(folderPath.find_first_of(":") == string::npos)
							folderPath = "/"+folderPath;
						folderPath = folderPath+"/";
					}
				}
				//if we removed device then clear path completely
				if(strchr(dir, '/') == NULL)
				{
					memset(dir, 0, MAX_FAT_PATH);
					for(u8 i = 1; i < 7; ++i)
						memset(entries_char, 0, NAME_MAX+1);
				}
				_refreshExplorer();
			}
			for(u8 i = 1; i < 7; ++i)
			{
				if(m_btnMgr.selected(entries_sel[i]))
				{
					//if path is empty add device+partition#:/ to start path
					if(dir[0] == '\0')
					{
						explorer_partition = i-1;
						strcpy(dir, fmt("%s:/", DeviceName[i-1]));
						folderPath = dir;
						_refreshExplorer();
					}
					//if it's a folder add folder+/ to path
					else if(!fsop_FileExist(fmt("%s%s", dir, entries_char[i-1])))
					{
						strcat(dir, entries_char[i-1]);
						folderPath = dir;
						while(folderPath.length() > 32)
						{
							//this if won't happen the first time
							if(folderPath.find_first_of("/") == string::npos)
								break;
							folderPath = folderPath.erase(0, folderPath.find_first_of("/")+1);
						}
						if(folderPath.find_first_of(":") == string::npos)
							folderPath = "/"+folderPath;
						folderPath = folderPath+"/";
						/* otherwise it fails */
						strcat(dir, "/");
						_refreshExplorer();
					}
					else
					{
						memset(file, 0, MAX_FAT_PATH);
						strncpy(file, fmt("%s%s", dir, entries_char[i-1]), MAX_FAT_PATH);
						if(strcasestr(file, ".mp3") != NULL || strcasestr(file, ".ogg") != NULL)
							MusicPlayer.LoadFile(file, false);
						else if(strcasestr(file, ".iso") != NULL || strcasestr(file, ".wbfs") != NULL)
						{
							_hideExplorer();
							/* create header for id and path */
							dir_discHdr tmpHdr;
							memset(&tmpHdr, 0, sizeof(dir_discHdr));
							memcpy(tmpHdr.path, file, 255);
							/* check wii or gc */
							FILE *fp = fopen(file, "rb");
							fseek(fp, strcasestr(file, ".wbfs") != NULL ? 512 : 0, SEEK_SET);
							fread((void*)&wii_hdr, 1, sizeof(discHdr), fp);
							fclose(fp);
							memcpy(tmpHdr.id, wii_hdr.id, 6);
							if(wii_hdr.magic == WII_MAGIC)
							{
								currentPartition = explorer_partition;
								_launchGame(&tmpHdr, false);
							}
							else if(wii_hdr.gc_magic == GC_MAGIC)
							{
								currentPartition = explorer_partition;
								_launchGC(&tmpHdr, false);
							}
							_showExplorer();
						}
						else if(strcasestr(file, ".wad") != NULL)
						{
							_hideExplorer();
							_Wad(file);
							_showExplorer();
						}
					}
				}
			}
		}
	}
	_hideExplorer();
}

void CMenu::_initExplorer()
{
	memset(dir, 0, MAX_FAT_PATH);
	TexData blank_btn;
	TexData texPrev;
	TexData texPrevS;
	TexData texNext;
	TexData texNextS;
	TexHandle.fromPNG(blank_btn, blank_png);
	TexHandle.fromPNG(texPrev, btnprev_png);
	TexHandle.fromPNG(texPrevS, btnprevs_png);
	TexHandle.fromPNG(texNext, btnnext_png);
	TexHandle.fromPNG(texNextS, btnnexts_png);
	
	m_explorerBg = _texture("EXPLORER/BG", "texture", theme.bg, false);
	_addUserLabels(explorerLblUser, ARRAY_SIZE(explorerLblUser), "EXPLORER");
	explorerLblSelFolder = _addLabel("EXPLORER/SELECTED_FOLDER", theme.lblFont, L"", 30, 50, 560, 40, theme.lblFontColor, FTGX_JUSTIFY_LEFT);
	explorerBtnSave = _addButton("EXPLORER/SAVE_BTN", theme.btnFont, L"", 520, 344, 100, 40, theme.btnFontColor);
	explorerBtnCancel = _addButton("EXPLORER/CANCEL_BTN", theme.btnFont, L"", 520, 400, 100, 40, theme.btnFontColor);
	explorerBtnNext = _addPicButton("EXPLORER/NEXT_BTN", texNext, texNextS, 540, 146, 60, 60);
	explorerBtnPrev = _addPicButton("EXPLORER/PREV_BTN", texPrev, texPrevS, 20, 146, 60, 60);
	
	_setHideAnim(explorerLblSelFolder, "EXPLORER/SELECTED_FOLDER", 0, 0, -2.f, 0.f);
	_setHideAnim(explorerBtnSave, "EXPLORER/SAVE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(explorerBtnCancel, "EXPLORER/CANCEL_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(explorerBtnNext, "EXPLORER/NEXT_BTN", 0, 0, 0.f, 0.f);
	_setHideAnim(explorerBtnPrev, "EXPLORER/PREV_BTN", 0, 0, 0.f, 0.f);
	
	for(u8 i = 0; i < 7; ++i)
	{
		entries_sel[i] = _addPicButton(fmt("EXPLORER/ENTRY_%i_BTN", i), blank_btn, blank_btn, 100, 100+(i*50), 380, 45);
		entries[i] = _addLabel(fmt("EXPLORER/ENTRY_%i", i), theme.lblFont, L"", 100, 100+(i*50), 480, 40, theme.lblFontColor, FTGX_JUSTIFY_LEFT);
		_setHideAnim(entries[i], fmt("EXPLORER/ENTRY_%i", i), 0, 0, 1.f, 0.f);
		_setHideAnim(entries_sel[i], fmt("EXPLORER/ENTRY_%i_BTN", i), 0, 0, 1.f, 0.f);
	}
	_hideExplorer(true);
	_textExplorer();
}

void CMenu::_textExplorer(void)
{
	m_btnMgr.setText(explorerBtnSave, _t("cfgne34", L"Save"));
	m_btnMgr.setText(explorerBtnCancel, _t("cfgne35", L"Cancel"));
}

u32 cur_pos = 0;
u32 last_pos = 0;
void CMenu::_refreshExplorer(s8 direction)
{
	for(u8 i = 0; i < 7; ++i)
	{
		m_btnMgr.hide(entries[i], true);
		m_btnMgr.hide(entries_sel[i], true);
		m_btnMgr.setText(entries[i], L" ");
	}
	m_btnMgr.setText(entries[0], L". . .");
	m_btnMgr.setText(explorerLblSelFolder, wfmt(_fmt("cfgne36",L"Path = %.32s"), folderPath.c_str()), true);
	
	if(direction == 0)
	{
		cur_pos = 0;
		last_pos = 0;
	}

	//if path is empty show device+partitions only
	if(dir[0] == '\0')
	{
		for(u8 i = 1; i < 7; ++i)
		{
			if(DeviceHandle.IsInserted(i-1))
			{
				m_btnMgr.setText(entries[i], wfmt(L"%s:/", DeviceName[i-1]));
				m_btnMgr.show(entries[i]);
				m_btnMgr.show(entries_sel[i]);
			}
		}
	}
	//else show folders and files
	else
	{
		m_btnMgr.show(entries[0]);
		m_btnMgr.show(entries_sel[0]);
		
		if(direction == -1)
			cur_pos = last_pos > 5 ? (last_pos - 6) : 0;
		
		folderList.clear();
		fileList.clear();
			
		dirent *pent = NULL;
		DIR *pdir = NULL;
		pdir = opendir(dir);
		while((pent = readdir(pdir)) != NULL)
		{
			if(pent->d_name[0] == '.')
				continue;
			if(pent->d_type == DT_DIR)
				folderList.push_back(string(pent->d_name));
			else
				fileList.push_back(string(pent->d_name));
		}
		closedir(pdir);
		sort(folderList.begin(), folderList.end(), _sortEntries);
		sort(fileList.begin(), fileList.end(), _sortEntries);
		u32 folderListSize = folderList.size();
		if(!folderExplorer)
		{
			for(u32 i = 0; i < fileList.size(); ++i)
				folderList.push_back(fileList[i]);
		}
		u8 ent_pos = 0;
		if(cur_pos >= folderList.size())
			cur_pos = last_pos;
		last_pos = cur_pos;
		for (u32 i = cur_pos; i < folderList.size(); ++i)
		{
			strcpy(entries_char[ent_pos], folderList[i].c_str());
			if(cur_pos < folderListSize)
				m_btnMgr.setText(entries[ent_pos+1], wfmt(L"/%.32s", folderList[i].c_str()));
			else
				m_btnMgr.setText(entries[ent_pos+1], wfmt(L"%.32s", folderList[i].c_str()));
			m_btnMgr.show(entries[ent_pos+1]);
			m_btnMgr.show(entries_sel[ent_pos+1]);
			cur_pos++;
			ent_pos++;
			if(ent_pos == 6)
				break;
		}
	}
}

bool CMenu::_sortEntries (string first, string second)
{
  u32 i=0;
  while ((i < first.length()) && (i < second.length()))
  {
    if (tolower (first[i]) < tolower (second[i])) return true;
    else if (tolower (first[i]) > tolower (second[i])) return false;
    i++;
  }

  if (first.length() < second.length()) return true;
  else return false;
}

string CMenu::_FolderExplorer(void)
{
	folderExplorer = true;
	path = "";
	_Explorer();
	folderExplorer = false;
	return path;
}
