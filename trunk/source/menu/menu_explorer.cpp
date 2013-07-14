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
#include "menu.hpp"
#include "channel/nand.hpp"
#include "defines.h"

extern const u8 blank_png[];

TexData m_explorerBg;
s16 entries[8];
s16 entries_sel[8];
char file[MAX_FAT_PATH];
char dir[MAX_FAT_PATH];
char entries_char[7][NAME_MAX+1];
u8 explorer_partition = 0;

void CMenu::_hideExplorer(bool instant)
{
	for(u8 i = 0; i < 8; ++i)
	{
		m_btnMgr.hide(entries[i], instant);
		m_btnMgr.hide(entries_sel[i], instant);
	}
	/* general movement */
	m_btnMgr.hide(m_mainBtnNext, instant);
	m_btnMgr.hide(m_mainBtnPrev, instant);
}

void CMenu::_showExplorer(void)
{
	_setBg(m_explorerBg, m_explorerBg);
	for(u8 i = 0; i < 8; ++i)
	{
		m_btnMgr.show(entries[i]);
		m_btnMgr.show(entries_sel[i]);
	}
	/* general movement */
	m_btnMgr.show(m_mainBtnNext);
	m_btnMgr.show(m_mainBtnPrev);
}

void CMenu::_Explorer(void)
{
	_showExplorer();
	_refreshExplorer();
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
			if(m_btnMgr.selected(m_mainBtnNext))
			{
				_refreshExplorer(1);
			}
			else if(m_btnMgr.selected(m_mainBtnPrev))
			{
				_refreshExplorer(-1);
			}
			else if(m_btnMgr.selected(entries_sel[0]) && dir[0] != '\0')
			{
				if(strchr(dir, '/') != NULL)
				{
					*strrchr(dir, '/') = '\0';
					if(strchr(dir, '/') != NULL)
						*(strrchr(dir, '/')+1) = '\0';
				}
				if(strchr(dir, '/') == NULL)
				{
					memset(dir, 0, MAX_FAT_PATH);
					for(u8 i = 1; i < 7; ++i)
						memset(entries_char, 0, NAME_MAX+1);
				}
				_refreshExplorer();
			}
			for(u8 i = 1; i < 8; ++i)
			{
				if(m_btnMgr.selected(entries_sel[i]))
				{
					if(dir[0] == '\0')
					{
						explorer_partition = i-1;
						if(DeviceHandle.IsInserted(i-1))
							strcpy(dir, fmt("%s:/", DeviceName[i-1]));
						_refreshExplorer();
					}
					else if(!fsop_FileExist(fmt("%s%s", dir, entries_char[i-1])))
					{
						strcat(dir, entries_char[i-1]);
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
	TexHandle.fromPNG(blank_btn, blank_png);

	m_explorerBg = _texture("EXPLORER/BG", "texture", theme.bg, false);
	for(u8 i = 0; i < 8; ++i)
	{
		entries[i] = _addLabel(fmt("EXPLORER/ENTRY_%i", i), theme.lblFont, L"", 120, 50+(i*50), 480, 40, theme.lblFontColor, FTGX_JUSTIFY_LEFT);
		entries_sel[i] = _addPicButton(fmt("EXPLORER/ENTRY_%i_BTN", i), blank_btn, blank_btn, 120, 50+(i*50), 380, 45);
		_setHideAnim(entries[i], fmt("EXPLORER/ENTRY_%i", i), 0, 0, 1.f, 0.f);
		_setHideAnim(entries_sel[i], fmt("EXPLORER/ENTRY_%i_BTN", i), 0, 0, 1.f, 0.f);
	}
	_hideExplorer(true);
	_textExplorer();
}

void CMenu::_textExplorer(void)
{
	m_btnMgr.setText(entries[0], L". . .");
	for(u8 i = 1; i < 8; ++i)
		m_btnMgr.setText(entries[i], L" ");
}

u32 cur_pos = 0;
void CMenu::_refreshExplorer(s8 direction)
{
	_textExplorer();
	if(direction == 0)
		cur_pos = 0;

	if(dir[0] == '\0')
	{
		for(u8 i = 1; i < 8; ++i)
		{
			if(DeviceHandle.IsInserted(i-1))
				m_btnMgr.setText(entries[i], wfmt(L"%s:/", DeviceName[i-1]));
		}
	}
	else
	{
		dirent *pent = NULL;
		DIR *pdir = NULL;
		u8 limit = 1;
		u32 itr = 0;
		if(direction == -1)
			cur_pos = cur_pos > 14 ? cur_pos - 14 : 0;
		pdir = opendir(dir);
		while((pent = readdir(pdir)) != NULL && limit < 8)
		{
			if(pent->d_name[0] == '.')
				continue;
			if(pent->d_type == DT_DIR)
			{
				if(itr < cur_pos)
				{
					itr++;
					continue;
				}
				strcpy(entries_char[limit-1], pent->d_name);
				m_btnMgr.setText(entries[limit], wfmt(L"/%.32s", pent->d_name));
				cur_pos++;
				itr++;
				limit++;
			}
			else if(pent->d_type == DT_REG)
			{
				if(itr < cur_pos)
				{
					itr++;
					continue;
				}
				strcpy(entries_char[limit-1], pent->d_name);
				m_btnMgr.setText(entries[limit], wfmt(L"%.32s", pent->d_name));
				cur_pos++;
				itr++;
				limit++;
			}
		}
		closedir(pdir);
	}
}

s16 m_folderExplorerLblSelFolder;
s16 m_folderExplorerBtnSave;
s16 m_folderExplorerBtnCancel;

string folderName="";

void CMenu::_initFolderExplorer()
{
	m_folderExplorerLblSelFolder = _addLabel("EXPLORER/SELECTED_FOLDER", theme.lblFont, L"", 30, 50, 560, 40, theme.lblFontColor, FTGX_JUSTIFY_LEFT);
	m_folderExplorerBtnSave = _addButton("EXPLORER/SAVE_BTN", theme.btnFont, L"Save", 520, 40, 100, 40, theme.btnFontColor);
	m_folderExplorerBtnCancel = _addButton("EXPLORER/CANCEL_BTN", theme.btnFont, L"Cancel", 520, 90, 100, 40, theme.btnFontColor);
	
	_hideFolderExplorer(true);
	_textFolderExplorer();
}

void CMenu::_hideFolderExplorer(bool instant)
{
	for(u8 i = 0; i < 8; ++i)
	{
		m_btnMgr.hide(entries[i], instant);
		m_btnMgr.hide(entries_sel[i], instant);
	}

	m_btnMgr.hide(m_mainBtnNext, instant);
	m_btnMgr.hide(m_mainBtnPrev, instant);
	m_btnMgr.hide(m_folderExplorerLblSelFolder, instant);
	m_btnMgr.hide(m_folderExplorerBtnSave, instant);
	m_btnMgr.hide(m_folderExplorerBtnCancel, instant);
}

void CMenu::_showFolderExplorer(void)
{
	_setBg(m_explorerBg, m_explorerBg);
	for(u8 i = 1; i < 8; ++i)
	{
		m_btnMgr.show(entries[i]);
		m_btnMgr.show(entries_sel[i]);
	}

	m_btnMgr.show(m_mainBtnNext);
	m_btnMgr.show(m_mainBtnPrev);
	m_btnMgr.show(m_folderExplorerLblSelFolder);
	m_btnMgr.show(m_folderExplorerBtnSave);
	m_btnMgr.show(m_folderExplorerBtnCancel);
}

void CMenu::_textFolderExplorer(void)
{
	m_btnMgr.setText(m_folderExplorerBtnSave, _t("cfgne34", L"Save"));
	m_btnMgr.setText(m_folderExplorerBtnCancel, _t("cfgne35", L"Cancel"));
	for(u8 i = 0; i < 8; ++i)
		m_btnMgr.setText(entries[i], L" ");
	m_btnMgr.setText(entries[1], L". . .");
	m_btnMgr.setText(m_folderExplorerLblSelFolder, wfmt(_fmt("cfgne36",L"Dir = %.32s"), folderName.c_str()));
}

void CMenu::_refreshFolderExplorer(s8 direction)
{
	_textFolderExplorer();
	if(direction == 0)
		cur_pos = 0;

	//if path is empty show device+partitions and not folders
	if(dir[0] == '\0')
	{
		for(u8 i = 1; i < 8; ++i)
		{
			if(DeviceHandle.IsInserted(i-1))
				m_btnMgr.setText(entries[i+1], wfmt(L"%s:/", DeviceName[i-1]));
		}
	}
	//else show folders
	else
	{
		dirent *pent = NULL;
		DIR *pdir = NULL;
		u8 limit = 1;
		u32 itr = 0;
		if(direction == -1)
			cur_pos = cur_pos > 12 ? cur_pos - 12 : 0;
		pdir = opendir(dir);
		while((pent = readdir(pdir)) != NULL && limit < 7)
		{
			if(pent->d_name[0] == '.')
				continue;
			if(pent->d_type == DT_DIR)
			{
				if(itr < cur_pos)
				{
					itr++;
					continue;
				}
				strcpy(entries_char[limit-1], pent->d_name);
				m_btnMgr.setText(entries[limit+1], wfmt(L"/%.32s", pent->d_name));
				cur_pos++;
				itr++;
				limit++;
			}
		}
		closedir(pdir);
	}
}

string CMenu::_FolderExplorer(void)
{
	string path = "";
	_showFolderExplorer();
	_refreshFolderExplorer();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_PLUS_PRESSED)
		{
			_refreshFolderExplorer(1);
		}
		else if(BTN_MINUS_PRESSED)
		{
			_refreshFolderExplorer(-1);
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_mainBtnNext))
			{
				_refreshFolderExplorer(1);
			}
			else if(m_btnMgr.selected(m_mainBtnPrev))
			{
				_refreshFolderExplorer(-1);
			}
			else if(m_btnMgr.selected(m_folderExplorerBtnSave))
			{
				//only when save is clicked do we set path to dir
				if(dir[0] != '\0')
					path = dir;
				break;
			}
			else if(m_btnMgr.selected(m_folderExplorerBtnCancel))
			{
				break;
			}
			//if "..." is selected and path is not empty then go up(back) one folder
			else if(m_btnMgr.selected(entries_sel[1]) && dir[0] != '\0')
			{
				//remove last folder or device+partition
				if(strchr(dir, '/') != NULL)
				{
					*strrchr(dir, '/') = '\0';
					if(strchr(dir, '/') != NULL)
						*(strrchr(dir, '/')+1) = '\0';
				}
				//set folderName to display current selected folder
				folderName = dir;
				//if it's just device and : then foldername empty
				if(folderName.find_last_of("/") == string::npos)
					folderName = "";
				else
				{
					if(folderName.find_first_of("/") != folderName.find_last_of("/"))
					{
						folderName = folderName.erase(folderName.find_last_of("/"));
						while(folderName.length() > 32)
						{
							if(folderName.find_first_of("/") == string::npos)
								break;
							folderName = folderName.erase(0, folderName.find_first_of("/")+1);
						}
						if(folderName.find_first_of(":") == string::npos)
							folderName = "/"+folderName;
						folderName = folderName+"/";
					}
				}
				//if we removed device then clear path completely
				if(strchr(dir, '/') == NULL)
				{
					memset(dir, 0, MAX_FAT_PATH);
					for(u8 i = 1; i < 7; ++i)
						memset(entries_char, 0, NAME_MAX+1);
				}
				_refreshFolderExplorer();
			}
			for(u8 i = 2; i < 8; ++i)
			{
				if(m_btnMgr.selected(entries_sel[i]))
				{
					//if path is empty add device+partition#:/ to start path
					if(dir[0] == '\0')
					{
						explorer_partition = i-2;
						if(DeviceHandle.IsInserted(i-2))
						{
							strcpy(dir, fmt("%s:/", DeviceName[i-2]));
							folderName = dir;
						}
						_refreshFolderExplorer();
					}
					//if it's a folder add folder+/ to path
					else if(!fsop_FileExist(fmt("%s%s", dir, entries_char[i-2])))
					{
						strcat(dir, entries_char[i-2]);
						folderName = dir;
						while(folderName.length() > 32)
						{
							//this if won't happen the first time
							if(folderName.find_first_of("/") == string::npos)
								break;
							folderName = folderName.erase(0, folderName.find_first_of("/")+1);
						}
						if(folderName.find_first_of(":") == string::npos)
							folderName = "/"+folderName;
						folderName = folderName+"/";
						strcat(dir, "/");
						_refreshFolderExplorer();
					}
				}
			}
		}
	}
	_hideFolderExplorer();
	return path;
}
