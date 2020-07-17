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
#include "menu.hpp"
#include "channel/nand.hpp"
#include "loader/cios.h"
#include "loader/nk.h"
#include "wstringEx/wstringEx.hpp"

/* home menu */
s16 m_homeLblTitle;
s16 m_homeLblUser[4];

s16 m_homeBtnHelp;
s16 m_homeBtnReloadCache;
s16 m_homeBtnExplorer;
s16 m_homeBtnSelPlugin;

s16 m_homeBtnCredits;
s16 m_homeBtnShutdown;
s16 m_homeBtnExitTo;
s16 m_homeBtnSettings;

s16 m_homeLblBattery;

/* exit to menu */
s16 m_exittoLblTitle;
s16 m_exittoLblUser[4];
s16 m_homeBtnExitToHBC;
s16 m_homeBtnExitToMenu;
s16 m_homeBtnExitToNeek;
s16 m_homeBtnExitToPriiloader;
s16 m_homeBtnExitToBootmii;

/* shutdown menu */
s16 m_shutdownLblQuestion;
s16 m_shutdownBtnFull;
s16 m_shutdownBtnStandby;
s16 m_shutdownBtnCancel;

TexData m_homeBg;
static const wstringEx PLAYER_BATTERY_LABEL("P1 %003.f%% | P2 %003.f%% | P3 %003.f%% | P4 %003.f%%");

bool CMenu::_Home(void)
{
	SetupInput();
	if(isWiiVC || m_locked)
		m_btnMgr.setText(m_homeBtnExitTo, _t("home12", L"Exit"));
	else
		m_btnMgr.setText(m_homeBtnExitTo, _t("home5", L"Exit To"));
	_showHome();

	while(!m_exit)
	{
		/* battery gets refreshed in here... */
		_mainLoopCommon();
		/* and it always changes so... */
		m_btnMgr.setText(m_homeLblBattery, wfmt(PLAYER_BATTERY_LABEL, min((float)wd[0]->battery_level, 100.f), 
			min((float)wd[1]->battery_level, 100.f), min((float)wd[2]->battery_level, 100.f), min((float)wd[3]->battery_level, 100.f)));
		if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_1_PRESSED)
		{
			//m_theme.load(fmt("%s.ini", m_themeDataDir.c_str()));
			m_theme.save();
			_hideHome();
			error(_t("savedtheme", L"Theme config saved!"));
			_showHome();
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_homeBtnHelp))
			{
				_hideHome();
				_about(true);
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnReloadCache))
			{
				if(m_current_view & COVERFLOW_WII)
					m_cfg.setBool(WII_DOMAIN, "update_cache", true);
				if(m_current_view & COVERFLOW_GAMECUBE)
					m_cfg.setBool(GC_DOMAIN, "update_cache", true);
				if(m_current_view & COVERFLOW_CHANNEL)
					m_cfg.setBool(CHANNEL_DOMAIN, "update_cache", true);
				if(m_current_view & COVERFLOW_PLUGIN)
					m_cfg.setBool(PLUGIN_DOMAIN, "update_cache", true);
				if(m_current_view & COVERFLOW_HOMEBREW)
					m_cfg.setBool(HOMEBREW_DOMAIN, "update_cache", true);
				m_refreshGameList = true;
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnSettings))
			{
				_hideHome();
				_config(1);
				//if(m_refreshGameList)
					break;
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnShutdown))
			{
				_hideHome();
				_Shutdown();
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnCredits))
			{
				_hideHome();
				_about();
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnExitTo))
			{
				_hideHome();
				if(isWiiVC)
				{
					exitHandler(EXIT_TO_MENU);
					break;
				}
				_ExitTo();
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnExplorer))
			{
				_hideHome();
				_Explorer();
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnSelPlugin))
			{
				_hideHome();
				_PluginSettings();
				if(m_refreshGameList)
					break;
				_showHome();
			}
		}
		else if(BTN_HOME_PRESSED)
		{
			if(isWiiVC)
			{
				exitHandler(EXIT_TO_MENU);
				break;
			}
			exitHandler(WIIFLOW_DEF);// WIIFLOW_DEF is what is set in main config as the exit to option
			break;
		}
		else if(BTN_B_PRESSED)
			break;
	}

	_hideHome();
	return m_exit;
}

bool CMenu::_ExitTo(void)
{
	SetupInput();
	_showExitTo();

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_A_PRESSED)// note exitHandler sets m_exit = true
		{
			if(m_btnMgr.selected(m_homeBtnExitToHBC))
			{
				exitHandler(EXIT_TO_HBC);
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnExitToMenu))
			{
				exitHandler(EXIT_TO_MENU);
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnExitToPriiloader))
			{
				if(IsOnWiiU())
					exitHandler(EXIT_TO_WIIU);
				else
					exitHandler(EXIT_TO_PRIILOADER);
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnExitToBootmii))
			{
				exitHandler(EXIT_TO_BOOTMII);
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnExitToNeek))
			{
				if(Load_Neek2o_Kernel() && _FindEmuPart(EMU_NAND, false) >= 0)// make sure neek2o and emunand exists
				{
					Sys_SetNeekPath(NandHandle.Get_NandPath());
					exitHandler(EXIT_TO_SMNK2O);
					break;
				}
				else
				{
					error(_fmt("errneek1", L"Cannot launch neek2o. Verify your neek2o setup"));
					_showExitTo();
				}
			}
		}
		else if(BTN_HOME_PRESSED)
		{
			exitHandler(WIIFLOW_DEF);
			break;
		}
		else if(BTN_B_PRESSED)
			break;
	}
	_hideExitTo();
	return m_exit;
}

void CMenu::_Shutdown(void)
{
	SetupInput();
	_showShutdown();

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_B_PRESSED)
			break;
		else if(BTN_A_PRESSED)// note exitHandler sets m_exit = true
		{
			if(m_btnMgr.selected(m_shutdownBtnFull))
				exitHandler(SHUTDOWN_STANDBY);
			else if(m_btnMgr.selected(m_shutdownBtnStandby))
				exitHandler(SHUTDOWN_IDLE);
			else if(m_btnMgr.selected(m_shutdownBtnCancel))
				break;
		}
	}
	_hideShutdown();
}

void CMenu::_showHome(void)
{
	_setBg(m_homeBg, m_homeBg);
	m_btnMgr.show(m_homeLblTitle);

	if(!m_locked)
	{	
		m_btnMgr.show(m_homeBtnHelp);
		m_btnMgr.show(m_homeBtnReloadCache);
		m_btnMgr.show(m_homeBtnExplorer);

		m_btnMgr.show(m_homeBtnCredits);
		m_btnMgr.show(m_homeBtnShutdown);
		m_btnMgr.show(m_homeBtnExitTo);
	}
	m_btnMgr.show(m_homeBtnSettings);
	m_btnMgr.show(m_homeBtnSelPlugin);

	m_btnMgr.show(m_homeLblBattery);

	for(u8 i = 0; i < ARRAY_SIZE(m_homeLblUser); ++i)
		if(m_homeLblUser[i] != -1)
			m_btnMgr.show(m_homeLblUser[i]);
}

void CMenu::_showExitTo(void)
{
	_setBg(m_homeBg, m_homeBg);
	m_btnMgr.show(m_exittoLblTitle);

	m_btnMgr.show(m_homeBtnExitToHBC);
	m_btnMgr.show(m_homeBtnExitToMenu);
	m_btnMgr.show(m_homeBtnExitToNeek);
	m_btnMgr.show(m_homeBtnExitToPriiloader);// exit to wii u on wii u
	if(IsOnWiiU() == false)
		m_btnMgr.show(m_homeBtnExitToBootmii);

	for(u8 i = 0; i < ARRAY_SIZE(m_exittoLblUser); ++i)
		if(m_exittoLblUser[i] != -1)
			m_btnMgr.show(m_exittoLblUser[i]);
}

void CMenu::_showShutdown(void)
{
	_setBg(m_homeBg, m_homeBg);

	m_btnMgr.show(m_shutdownLblQuestion);
	m_btnMgr.show(m_shutdownBtnFull);
	if(IsOnWiiU() == false)
		m_btnMgr.show(m_shutdownBtnStandby);
	m_btnMgr.show(m_shutdownBtnCancel);
}

void CMenu::_hideHome(bool instant)
{
	m_btnMgr.hide(m_homeLblTitle, instant);

	m_btnMgr.hide(m_homeBtnHelp, instant);
	m_btnMgr.hide(m_homeBtnReloadCache, instant);
	m_btnMgr.hide(m_homeBtnExplorer, instant);
	m_btnMgr.hide(m_homeBtnSelPlugin, instant);

	m_btnMgr.hide(m_homeBtnCredits, instant);
	m_btnMgr.hide(m_homeBtnShutdown, instant);
	m_btnMgr.hide(m_homeBtnExitTo, instant);
	m_btnMgr.hide(m_homeBtnSettings, instant);

	m_btnMgr.hide(m_homeLblBattery, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_homeLblUser); ++i)
		if(m_homeLblUser[i] != -1)
			m_btnMgr.hide(m_homeLblUser[i], instant);
}

void CMenu::_hideExitTo(bool instant)
{
	m_btnMgr.hide(m_exittoLblTitle, instant);

	m_btnMgr.hide(m_homeBtnExitToHBC, instant);
	m_btnMgr.hide(m_homeBtnExitToMenu, instant);
	m_btnMgr.hide(m_homeBtnExitToNeek, instant);
	m_btnMgr.hide(m_homeBtnExitToPriiloader, instant);
	m_btnMgr.hide(m_homeBtnExitToBootmii, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_exittoLblUser); ++i)
		if(m_exittoLblUser[i] != -1)
			m_btnMgr.hide(m_exittoLblUser[i], instant);
}

void CMenu::_hideShutdown(bool instant)
{
	m_btnMgr.hide(m_shutdownLblQuestion, instant);
	m_btnMgr.hide(m_shutdownBtnFull, instant);
	m_btnMgr.hide(m_shutdownBtnStandby, instant);
	m_btnMgr.hide(m_shutdownBtnCancel, instant);
}

void CMenu::_initHomeAndExitToMenu()
{
	m_homeBg = _texture("HOME/BG", "texture", theme.bg, false);
	
	//Home Menu
	_addUserLabels(m_homeLblUser, ARRAY_SIZE(m_homeLblUser), "HOME");
	m_homeLblTitle = _addLabel("HOME/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	m_homeBtnHelp = _addButton("HOME/HELP", theme.btnFont, L"", 60, 100, 250, 48, theme.btnFontColor);
	m_homeBtnReloadCache = _addButton("HOME/RELOAD_CACHE", theme.btnFont, L"", 60, 180, 250, 48, theme.btnFontColor);
	m_homeBtnExplorer = _addButton("HOME/EXPLORER", theme.btnFont, L"", 60, 260, 250, 48, theme.btnFontColor);
	m_homeBtnSelPlugin = _addButton("HOME/SELECT_PLUGIN", theme.btnFont, L"", 60, 340, 250, 48, theme.btnFontColor);

	m_homeBtnCredits = _addButton("HOME/CREDITS", theme.btnFont, L"", 330, 100, 250, 48, theme.btnFontColor);
	m_homeBtnShutdown = _addButton("HOME/SHUTDOWN", theme.btnFont, L"", 330, 180, 250, 48, theme.btnFontColor);
	m_homeBtnExitTo = _addButton("HOME/EXIT_TO", theme.btnFont, L"", 330, 260, 250, 48, theme.btnFontColor);
	m_homeBtnSettings = _addButton("HOME/SETTINGS", theme.btnFont, L"", 330, 340, 250, 48, theme.btnFontColor);

	m_homeLblBattery = _addLabel("HOME/BATTERY", theme.btnFont, L"", 0, 420, 640, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);

	_setHideAnim(m_homeLblTitle, "HOME/TITLE", 0, 0, -2.f, 0.f);

	_setHideAnim(m_homeBtnHelp, "HOME/HELP", 50, 0, 1.f, 0.f);
	_setHideAnim(m_homeBtnReloadCache, "HOME/RELOAD_CACHE", 50, 0, 1.f, 0.f);
	_setHideAnim(m_homeBtnExplorer, "HOME/EXPLORER", 50, 0, 1.f, 0.f);
	_setHideAnim(m_homeBtnSelPlugin, "HOME/SELECT_PLUGIN", 50, 0, 1.f, 0.f);
	
	_setHideAnim(m_homeBtnCredits, "HOME/CREDITS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_homeBtnShutdown, "HOME/SHUTDOWN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_homeBtnExitTo, "HOME/EXIT_TO", -50, 0, 1.f, 0.f);
	_setHideAnim(m_homeBtnSettings, "HOME/SETTINGS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_homeLblBattery, "HOME/BATTERY", 0, 0, -2.f, 0.f);

	_textHome();
	_hideHome(true);
	
	//ExitTo Menu
	_addUserLabels(m_exittoLblUser, ARRAY_SIZE(m_exittoLblUser), "EXIT_TO");
	m_exittoLblTitle = _addLabel("EXIT_TO/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	m_homeBtnExitToHBC = _addButton("EXIT_TO/HBC", theme.btnFont, L"", 185, 120, 270, 48, theme.btnFontColor);
	m_homeBtnExitToMenu = _addButton("EXIT_TO/MENU", theme.btnFont, L"", 185, 180, 270, 48, theme.btnFontColor);
	m_homeBtnExitToNeek = _addButton("EXIT_TO/NEEK", theme.btnFont, L"", 185, 240, 270, 48, theme.btnFontColor);
	m_homeBtnExitToPriiloader = _addButton("EXIT_TO/PRIILOADER", theme.btnFont, L"", 185, 300, 270, 48, theme.btnFontColor);
	m_homeBtnExitToBootmii = _addButton("EXIT_TO/BOOTMII", theme.btnFont, L"", 185, 360, 270, 48, theme.btnFontColor);

	_setHideAnim(m_exittoLblTitle, "EXIT_TO/TITLE", 0, 0, -2.f, 0.f);

	_setHideAnim(m_homeBtnExitToHBC, "EXIT_TO/HBC", 0, 0, -4.f, 0.f);
	_setHideAnim(m_homeBtnExitToMenu, "EXIT_TO/MENU", 0, 0, -4.f, 0.f);
	_setHideAnim(m_homeBtnExitToPriiloader, "EXIT_TO/PRIILOADER", 0, 0, -4.f, 0.f);
	_setHideAnim(m_homeBtnExitToBootmii, "EXIT_TO/BOOTMII", 0, 0, -4.f, 0.f);
	_setHideAnim(m_homeBtnExitToNeek, "EXIT_TO/NEEK", 0, 0, -4.f, 0.f);

	_textExitTo();
	_hideExitTo(true);
	
	//Shutdown Menu
	m_shutdownLblQuestion = _addLabel("SHUTDOWN/QUESTION", theme.lblFont, L"", 185, 120, 270, 48, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_shutdownBtnFull = _addButton("SHUTDOWN/FULL", theme.btnFont, L"", 185, 180, 270, 48, theme.btnFontColor);
	m_shutdownBtnStandby = _addButton("SHUTDOWN/STANDBY", theme.btnFont, L"", 185, 240, 270, 48, theme.btnFontColor);
	m_shutdownBtnCancel = _addButton("SHUTDOWN/CANCEL", theme.btnFont, L"", 185, 300, 270, 48, theme.btnFontColor);

	_setHideAnim(m_shutdownLblQuestion, "SHUTDOWN/QUESTION", 0, 0, -4.f, 0.f);
	_setHideAnim(m_shutdownBtnFull, "SHUTDOWN/FULL", 0, 0, -4.f, 0.f);
	_setHideAnim(m_shutdownBtnStandby, "SHUTDOWN/STANDBY", 0, 0, -4.f, 0.f);
	_setHideAnim(m_shutdownBtnCancel, "SHUTDOWN/CANCEL", 0, 0, -4.f, 0.f);

	_textShutdown();
	_hideShutdown(true);
}

void CMenu::_textHome(void)
{
	m_btnMgr.setText(m_homeLblTitle, wfmt(L"%s %s", APP_NAME, APP_VERSION));
	m_btnMgr.setText(m_homeBtnHelp, _t("about10", L"Help Guide"));
	m_btnMgr.setText(m_homeBtnReloadCache, _t("home2", L"Reload Cache"));
	m_btnMgr.setText(m_homeBtnExplorer, _t("home8", L"File Explorer"));
	m_btnMgr.setText(m_homeBtnSelPlugin, _t("cfgpl1", L"Select Plugins"));

	m_btnMgr.setText(m_homeBtnCredits, _t("home4", L"Credits"));
	m_btnMgr.setText(m_homeBtnShutdown, _t("home13", L"Shutdown"));
	m_btnMgr.setText(m_homeBtnExitTo, _t("home5", L"Exit To"));
	m_btnMgr.setText(m_homeBtnSettings, _t("cfg1", L"Settings"));
}

void CMenu::_textExitTo(void)
{
	m_btnMgr.setText(m_exittoLblTitle, _t("exit_to", L"Exit To"));
	m_btnMgr.setText(m_homeBtnExitToHBC, _t("hbc", L"Homebrew Channel"));
	m_btnMgr.setText(m_homeBtnExitToMenu, _t("menu", L"System Menu"));
	m_btnMgr.setText(m_homeBtnExitToNeek, _t("neek2o", L"neek2o"));
	if(IsOnWiiU())
		m_btnMgr.setText(m_homeBtnExitToPriiloader, _t("wiiu", L"Wii U Menu"));
	else
		m_btnMgr.setText(m_homeBtnExitToPriiloader, _t("prii", L"Priiloader"));
	m_btnMgr.setText(m_homeBtnExitToBootmii, _t("bootmii", L"Bootmii"));
}

void CMenu::_textShutdown(void)
{
	m_btnMgr.setText(m_shutdownLblQuestion, _t("shutdown1", L"Shutdown how?"));
	m_btnMgr.setText(m_shutdownBtnFull, _t("shutdown2", L"Full Shutdown"));
	m_btnMgr.setText(m_shutdownBtnStandby, _t("shutdown3", L"Standby"));
	m_btnMgr.setText(m_shutdownBtnCancel, _t("shutdown4", L"Cancel"));
}
/*******************************************************************************/

int CMenu::_sfCacheCoversNeeded()// for sourceflow
{
	CoverFlow.stopCoverLoader(true);
	
	string coverPath;
	string wfcPath;
	string cachePath = m_cacheDir + "/sourceflow/";
	string gameNameOrID;

	bool smallBox = m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox", false);
	int missing = 0;
	
	for(vector<dir_discHdr>::iterator hdr = m_gameList.begin(); hdr != m_gameList.end(); ++hdr)
	{
		/* get cover png path */
		bool blankCover = false;
		bool fullCover = true;
		coverPath.assign(getBoxPath(&(*hdr)));
		if(!fsop_FileExist(coverPath.c_str()) || smallBox)
		{
			fullCover = false;
			coverPath.assign(getFrontPath(&(*hdr)));
			if(!fsop_FileExist(coverPath.c_str()) && !smallBox)
			{
				fullCover = true;
				coverPath.assign(getBlankCoverPath(&(*hdr)));
				blankCover = true;
				if(!fsop_FileExist(coverPath.c_str()))
					continue;
			}
		}

		/* get game name or ID */
		if(!blankCover)
			gameNameOrID.assign(CoverFlow.getFilenameId(&(*hdr)));
		else
			gameNameOrID.assign(coverPath.substr(coverPath.find_last_of("/") + 1));
		
		/* get cover wfc path */
		if(smallBox)
			wfcPath.assign(cachePath + gameNameOrID + "_small.wfc");
		else
			wfcPath.assign(cachePath + gameNameOrID);
		
		/* if wfc doesn't exist or is flat and have full cover */
		if(!fsop_FileExist(wfcPath.c_str()) || (!CoverFlow.fullCoverCached(wfcPath.c_str()) && fullCover))
			missing++;
	}
	return missing;
}

int CMenu::_cacheCovers()
{
	CoverFlow.stopCoverLoader(true);
	bool m_pluginCacheFolders = m_cfg.getBool(PLUGIN_DOMAIN, "subfolder_cache", true);
	
	char coverPath[MAX_FAT_PATH];//1024
	char wfcPath[MAX_FAT_PATH+20];
	char cachePath[MAX_FAT_PATH];
	
	u32 total = m_gameList.size();
	m_thrdTotal = total;
	u32 index = 0;
	
	bool smallBox = false;
	if(m_current_view == COVERFLOW_HOMEBREW && !m_sourceflow)
		smallBox = m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", false);
	else if(m_sourceflow)
		smallBox = m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox", false);
	else if(m_current_view == COVERFLOW_PLUGIN && !m_sourceflow)
	{
		if(enabledPluginsCount == 1 && m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(strtoul("48425257", NULL, 16))))
			smallBox = m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", false);
	}

	for(vector<dir_discHdr>::iterator hdr = m_gameList.begin(); hdr != m_gameList.end(); ++hdr)
	{
		index++;
		update_pThread(index, false);
		m_thrdMessage = wfmt(_fmt("dlmsg31", L"Converting cover %i of %i"), index, total);
		m_thrdMessageAdded = true;
		
		/* get cover png path */
		bool blankCover = false;
		bool fullCover = true;
		strlcpy(coverPath, getBoxPath(&(*hdr)), sizeof(coverPath));
		//gprintf("boxpath=%s\n", coverPath);
		if(!fsop_FileExist(coverPath) || smallBox)
		{
			fullCover = false;
			strlcpy(coverPath, getFrontPath(&(*hdr)), sizeof(coverPath));
			//gprintf("frontpath=%s\n", coverPath);
			if(!fsop_FileExist(coverPath) && !smallBox)
			{
				fullCover = true;
				strlcpy(coverPath, getBlankCoverPath(&(*hdr)), sizeof(coverPath));
				//gprintf("blankpath=%s\n", coverPath);
				blankCover = true;
				if(!fsop_FileExist(coverPath))
					continue;
			}
		}
				
		/* get cache folder path */
		if(hdr->type == TYPE_PLUGIN && m_pluginCacheFolders)
			snprintf(cachePath, sizeof(cachePath), "%s/%s", m_cacheDir.c_str(), m_plugin.GetCoverFolderName(hdr->settings[0]));
		else if(m_sourceflow)
			snprintf(cachePath, sizeof(cachePath), "%s/sourceflow", m_cacheDir.c_str());
		else if(hdr->type == TYPE_HOMEBREW)
			snprintf(cachePath, sizeof(cachePath), "%s/homebrew", m_cacheDir.c_str());
		else
			snprintf(cachePath, sizeof(cachePath), "%s", m_cacheDir.c_str());
		//gprintf("cachepath=%s\n", cachePath);
			
		/* get game name or ID */
		const char *gameNameOrID = NULL;
		if(!blankCover)
			gameNameOrID = CoverFlow.getFilenameId(&(*hdr));// &(*hdr) converts iterator to pointer to mem address
		else
			gameNameOrID = strrchr(coverPath, '/') + 1;
		//gprintf("nameorid=%s\n", gameNameOrID);
		
		/* get cover wfc path */
		if(smallBox)
			snprintf(wfcPath, sizeof(wfcPath), "%s/%s_small.wfc", cachePath, gameNameOrID);
		else
			snprintf(wfcPath, sizeof(wfcPath), "%s/%s.wfc", cachePath, gameNameOrID);
		//gprintf("wfcpath=%s\n", wfcPath);
		
		/* if wfc doesn't exist or is flat and have full cover */
		if(!fsop_FileExist(wfcPath) || (!CoverFlow.fullCoverCached(wfcPath) && fullCover))
		{
			/* create cache subfolders if needed */
			if(!fsop_FolderExist(cachePath))
				fsop_MakeFolder(cachePath);
		
			/* create cover texture */
			CoverFlow.cacheCoverFile(wfcPath, coverPath, fullCover);
		}
		
		// cache wii and channel banners
		if(hdr->type == TYPE_WII_GAME || hdr->type == TYPE_CHANNEL || hdr->type == TYPE_EMUCHANNEL)
		{
			CurrentBanner.ClearBanner();
			char cached_banner[256];
			strlcpy(cached_banner, fmt("%s/%s.bnr", m_bnrCacheDir.c_str(), hdr->id), sizeof(cached_banner));
			if(fsop_FileExist(cached_banner))
				continue;
			if(hdr->type == TYPE_WII_GAME)
			{
				_extractBnr(&(*hdr));
			}
			else if(hdr->type == TYPE_CHANNEL || hdr->type == TYPE_EMUCHANNEL)
			{
				ChannelHandle.GetBanner(TITLE_ID(hdr->settings[0], hdr->settings[1]));
			}
			
			if(CurrentBanner.IsValid())
				fsop_WriteFile(cached_banner, CurrentBanner.GetBannerFile(), CurrentBanner.GetBannerFileSize());
		}
	}
	CurrentBanner.ClearBanner();
	CoverFlow.startCoverLoader();
	return 0;
}