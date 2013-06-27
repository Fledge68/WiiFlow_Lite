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
#include "defines.h"
#include "fileOps/fileOps.h"

extern const u8 blank_png[];

TexData m_explorerBg;
s16 entries[8];
s16 entries_sel[8];
char dir[MAX_FAT_PATH];
char entries_char[7][NAME_MAX+1];
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
						const char *file = fmt("%s%s", dir, entries_char[i-1]);
						if(strstr(file, ".mp3") != NULL || strstr(file, ".ogg") != NULL)
							MusicPlayer.LoadFile(file, false);
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
