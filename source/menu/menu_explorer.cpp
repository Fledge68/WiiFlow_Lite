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

TexData m_explorerBg;
s16 entries[7];
s16 entries_sel[7];
s16 m_explorerLblSelFolder;
s16 m_explorerBtnSet;
s16 m_explorerBtnBack;
s16 m_explorerLblPage;
s16 m_explorerBtnPageM;
s16 m_explorerBtnPageP;
s16 m_explorerLblUser[4];

u32 dirs = 0;
u32 files = 0;
u32 start_pos = 0;
typedef struct {
	char name[NAME_MAX];
} list_element;
list_element *elements = NULL;
u32 elements_num = 0;
char file[MAX_FAT_PATH];
char dir[MAX_FAT_PATH];
char folderPath[MAX_FAT_PATH];
char tmpPath[MAX_FAT_PATH];
u8 explorer_partition = 0;
bool folderExplorer = false;

void CMenu::_hideExplorer(bool instant)
{
	for(u8 i = 0; i < 7; ++i)
	{
		m_btnMgr.hide(entries[i], instant);
		m_btnMgr.hide(entries_sel[i], instant);
	}
	
	m_btnMgr.hide(m_explorerLblSelFolder, instant);
	m_btnMgr.hide(m_explorerLblPage, instant);
	m_btnMgr.hide(m_explorerBtnPageM, instant);
	m_btnMgr.hide(m_explorerBtnPageP, instant);
	m_btnMgr.hide(m_explorerBtnSet, instant);
	m_btnMgr.hide(m_explorerBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_explorerLblUser); ++i)
	{
		if(m_explorerLblUser[i] != -1)
			m_btnMgr.hide(m_explorerLblUser[i], instant);
	}
	if(elements != NULL)
		free(elements);
	elements = NULL;
	elements_num = 0;
}

void CMenu::_showExplorer(void)
{
	_setBg(m_explorerBg, m_explorerBg);

	m_btnMgr.show(m_explorerLblSelFolder);
	m_btnMgr.show(m_explorerBtnBack);
	if(folderExplorer)
		m_btnMgr.show(m_explorerBtnSet);
		
	for(u8 i = 0; i < ARRAY_SIZE(m_explorerLblUser); ++i)
	{
		if(m_explorerLblUser[i] != -1)
			m_btnMgr.show(m_explorerLblUser[i]);
	}
	_refreshExplorer();
}

void CMenu::_Explorer(void)
{
	CoverFlow.clear();
	strcpy(folderPath, dir);
	_showExplorer();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			memset(folderPath, 0, MAX_FAT_PATH);
			break;
		}
		else if(BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED)
		{
			_refreshExplorer(1);
		}
		else if(BTN_MINUS_PRESSED || BTN_LEFT_PRESSED)
		{
			_refreshExplorer(-1);
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_explorerBtnPageP))
			{
				_refreshExplorer(1);
			}
			else if(m_btnMgr.selected(m_explorerBtnPageM))
			{
				_refreshExplorer(-1);
			}
			else if(m_btnMgr.selected(m_explorerBtnSet))
			{
				strcpy(folderPath, dir);
				break;
			}
			else if(m_btnMgr.selected(m_explorerBtnBack))
			{
				memset(folderPath, 0, MAX_FAT_PATH);
				break;
			}
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
				strcpy(folderPath, dir);
				//if dir is just device and : then clear path completely
				if(strchr(dir, '/') == NULL)
				{
					memset(dir, 0, MAX_FAT_PATH);
					memset(folderPath, 0, MAX_FAT_PATH);
				}
				else if(strchr(folderPath, '/') != strrchr(folderPath, '/'))
				{
					*strrchr(folderPath, '/') = '\0';
					while(strlen(folderPath) > 48)
					{
						if(strchr(folderPath, '/') == strrchr(folderPath, '/'))
							break;
						memset(tmpPath, 0, MAX_FAT_PATH);
						strncpy(tmpPath, strchr(folderPath, '/') + 1, MAX_FAT_PATH - 1);
						strcpy(folderPath, tmpPath);
					}
					memset(tmpPath, 0, MAX_FAT_PATH);
					if(strchr(folderPath, ':') == NULL)
						strcpy(tmpPath, "/");
					strcat(tmpPath, folderPath);
					strcat(tmpPath, "/");
					strcpy(folderPath, tmpPath);
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
						strcpy(folderPath, dir);
						_refreshExplorer();
					}
					//if it's a folder add folder+/ to path
					else if(!fsop_FileExist(fmt("%s%s", dir, elements[start_pos+(i-1)].name)))
					{
						strcat(dir, elements[start_pos+(i-1)].name);
						strcpy(folderPath, dir);
						strcat(dir, "/");
						while(strlen(folderPath) > 48)
						{
							if(strchr(folderPath, '/') == strrchr(folderPath, '/'))
								break;
							memset(tmpPath, 0, MAX_FAT_PATH);
							strncpy(tmpPath, strchr(folderPath, '/') + 1, MAX_FAT_PATH - 1);
							strcpy(folderPath, tmpPath);
						}
						memset(tmpPath, 0, MAX_FAT_PATH);
						if(strchr(folderPath, ':') == NULL)
							strcpy(tmpPath, "/");
						strcat(tmpPath, folderPath);
						strcat(tmpPath, "/");
						strcpy(folderPath, tmpPath);
						_refreshExplorer();
					}
					else
					{
						memset(file, 0, MAX_FAT_PATH);
						strncpy(file, fmt("%s%s", dir, elements[start_pos+(i-1)].name), MAX_FAT_PATH);
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
						else if(strcasestr(file, ".dol") != NULL || strcasestr(file, ".elf") != NULL)
						{
							_hideExplorer();
							_launchHomebrew(file, m_homebrewArgs);
							_showExplorer();
						}
						else if(strcasestr(file, ".txt") != NULL || strcasestr(file, ".nfo") != NULL
							|| strcasestr(file, ".ini") != NULL || strcasestr(file, ".conf") != NULL
							|| strcasestr(file, ".cfg") != NULL || strcasestr(file, ".xml") != NULL
							|| strcasestr(file, ".log") != NULL)
						{
							_hideExplorer();
							m_txt_view = true;
							m_txt_path = file;
							_about(false);
							m_txt_view = false;
							m_txt_path = NULL;
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
	_initCF();
}

void CMenu::_initExplorer()
{
	memset(dir, 0, MAX_FAT_PATH);
	TexData blank_btn;
	TexHandle.fromPNG(blank_btn, blank_png);
	
	m_explorerBg = _texture("EXPLORER/BG", "texture", theme.bg, false);
	_addUserLabels(m_explorerLblUser, ARRAY_SIZE(m_explorerLblUser), "EXPLORER");
	m_explorerLblSelFolder = _addText("EXPLORER/SELECTED_FOLDER", theme.txtFont, L"", 30, 30, 560, 40, theme.txtFontColor, FTGX_JUSTIFY_LEFT);
	m_explorerBtnSet = _addButton("EXPLORER/SET_BTN", theme.btnFont, L"", 255, 400, 150, 56, theme.btnFontColor);
	m_explorerBtnBack = _addButton("EXPLORER/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_explorerBtnPageM = _addPicButton("EXPLORER/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_explorerLblPage = _addLabel("EXPLORER/PAGE_BTN", theme.btnFont, L"", 76, 400, 100, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_explorerBtnPageP = _addPicButton("EXPLORER/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 176, 400, 56, 56);
		
	_setHideAnim(m_explorerLblSelFolder, "EXPLORER/SELECTED_FOLDER", 0, 0, -2.f, 0.f);
	_setHideAnim(m_explorerBtnSet, "EXPLORER/SET_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_explorerBtnBack, "EXPLORER/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_explorerLblPage, "EXPLORER/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_explorerBtnPageM, "EXPLORER/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_explorerBtnPageP, "EXPLORER/PAGE_PLUS", 0, 0, 1.f, -1.f);
	
	for(u8 i = 0; i < 7; ++i)
	{
		entries_sel[i] = _addPicButton(fmt("EXPLORER/ENTRY_%i_BTN", i), blank_btn, blank_btn, 40, 75+(i*45), 380, 45);
		entries[i] = _addText(fmt("EXPLORER/ENTRY_%i", i), theme.txtFont, L"", 40, 75+(i*45), 480, 40, theme.txtFontColor, FTGX_JUSTIFY_LEFT);
		_setHideAnim(entries[i], fmt("EXPLORER/ENTRY_%i", i), 0, 0, 1.f, 0.f);
		_setHideAnim(entries_sel[i], fmt("EXPLORER/ENTRY_%i_BTN", i), 0, 0, 1.f, 0.f);
	}
	_hideExplorer(true);
	_textExplorer();
}

void CMenu::_textExplorer(void)
{
	m_btnMgr.setText(m_explorerBtnSet, _t("cfgne34", L"Set"));
	m_btnMgr.setText(m_explorerBtnBack, _t("cfgne35", L"Back"));
}

static bool list_element_cmp(list_element a, list_element b)
{
	const char *first = a.name;
	const char *second = b.name;
	return char_cmp(first, second, strlen(first), strlen(second));
}

void CMenu::_refreshExplorer(s8 direction)
{
	for(u8 i = 0; i < 7; ++i)
	{
		m_btnMgr.hide(entries[i], true);
		m_btnMgr.hide(entries_sel[i], true);
		m_btnMgr.setText(entries[i], L" ");
	}
	m_btnMgr.setText(entries[0], L". . .");
	wstringEx path(_t("cfgne36", L"Path ="));
	path.append(wfmt(L" %.48s", folderPath));
	m_btnMgr.setText(m_explorerLblSelFolder, path, true);

	if(direction == 0)
		start_pos = 0;

	//if path is empty show device+partitions only
	if(dir[0] == '\0')
	{
		elements_num = 0;
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
		if(direction == 0)
		{
			dirent *pent = NULL;
			DIR *pdir = NULL;
			pdir = opendir(dir);
			/* some sorting */
			dirs = 0;
			files = 0;
			while((pent = readdir(pdir)) != NULL)
			{
				if(pent->d_name[0] == '.')
					continue;
				if(pent->d_type == DT_DIR)
					dirs++;
				else if(pent->d_type == DT_REG)
					files++;
			}
			u32 pos = 0;
			if(elements != NULL)
				free(elements);
			elements_num = (folderExplorer ? dirs : dirs+files);
			elements = (list_element*)MEM2_alloc(elements_num*sizeof(list_element));
			if(dirs > 0)
			{
				rewinddir(pdir);
				while((pent = readdir(pdir)) != NULL)
				{
					if(pent->d_name[0] == '.')
						continue;
					if(pent->d_type == DT_DIR)
					{
						memcpy(elements[pos].name, pent->d_name, NAME_MAX);
						pos++;
					}
				}
				std::sort(elements, elements+pos, list_element_cmp);
			}
			if(folderExplorer == false && files > 0)
			{
				rewinddir(pdir);
				while((pent = readdir(pdir)) != NULL)
				{
					if(pent->d_name[0] == '.')
						continue;
					if(pent->d_type == DT_REG)
					{
						memcpy(elements[pos].name, pent->d_name, NAME_MAX);
						pos++;
					}
				}
				std::sort(elements+dirs, elements+pos, list_element_cmp);
			}
			closedir(pdir);
		}
		if(direction == -1)									/* dont question */
			start_pos = start_pos >= 6 ? start_pos - 6 : (elements_num % 6 ? (elements_num - elements_num % 6) : elements_num - 6);
		else if(direction == 1)
			start_pos = start_pos + 6 >= elements_num ? 0 : start_pos + 6;

		for(u8 i = 1; i < 7; i++)
		{
			if(start_pos+i > elements_num)
				break;
			if(start_pos+i <= dirs)
				m_btnMgr.setText(entries[i], wfmt(L"/%.48s", elements[start_pos+i-1].name));
			else
				m_btnMgr.setText(entries[i], wfmt(L"%.48s", elements[start_pos+i-1].name));
			m_btnMgr.show(entries[i]);
			m_btnMgr.show(entries_sel[i]);
		}
	}
	if(elements_num > 0)
		m_btnMgr.setText(m_explorerLblPage, wfmt(L"%i / %i", (start_pos/6 + 1), ((elements_num - 1)/6 +1)));
	else
		m_btnMgr.setText(m_explorerLblPage, L"1 / 1");
	m_btnMgr.show(m_explorerLblPage);
	m_btnMgr.show(m_explorerBtnPageM);	
	m_btnMgr.show(m_explorerBtnPageP);
}

const char *CMenu::_FolderExplorer(const char *startPath)
{
	folderExplorer = true;
	memset(dir, 0, MAX_FAT_PATH);
	strcpy(dir, startPath);
	if(dir[strlen(dir) - 1] != '/')
		strcat(dir, "/");
	_Explorer();
	folderExplorer = false;
	return folderPath;
}
