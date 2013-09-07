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
#include "loader/cios.h"
#include "loader/nk.h"
#include "const_str.hpp"

s16 m_homeLblTitle;
s16 m_exittoLblTitle;
s16 m_exittoLblUser[4];

s16 m_homeBtnSettings;
s16 m_homeBtnReloadCache;
s16 m_homeBtnUpdate;
s16 m_homeBtnExplorer;

s16 m_homeBtnInstall;
s16 m_homeBtnAbout;
s16 m_homeBtnExitTo;
s16 m_homeBtnFTP;

s16 m_homeBtnExitToHBC;
s16 m_homeBtnExitToMenu;
s16 m_homeBtnExitToPriiloader;
s16 m_homeBtnExitToBootmii;
s16 m_homeBtnExitToNeek;

s16 m_homeLblBattery;
s16 m_homeLblUser[4];

TexData m_homeBg;

bool CMenu::_Home(void)
{
	SetupInput();
	_showHome();

	string prevTheme = m_cfg.getString("GENERAL", "theme", "default");
	while(!m_exit)
	{
		/* battery gets refreshed in here... */
		_mainLoopCommon();
		/* and it always changes so... */
		m_btnMgr.setText(m_homeLblBattery, wfmt(PLAYER_BATTERY_LABEL, min((float)wd[0]->battery_level, 100.f), 
			min((float)wd[1]->battery_level, 100.f), min((float)wd[2]->battery_level, 100.f), min((float)wd[3]->battery_level, 100.f)));
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_homeBtnSettings))
			{
				_hideHome();
				_config(1);
				if(prevTheme != m_cfg.getString("GENERAL", "theme") || m_reload == true)
				{
					m_exit = true;
					m_reload = true;
					break;
				}
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnReloadCache))
			{
				//m_gameList.SetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
				UpdateCache(m_current_view);
				LoadView();
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnUpdate) && !m_locked)
			{
				CoverFlow.stopCoverLoader(true);
				_hideHome();
				_system();
				remove(m_ver.c_str());
				if(m_exit)
					_launchHomebrew(m_dol.c_str(), m_homebrewArgs);
				else
				{
					_showHome();
					CoverFlow.startCoverLoader();
				}
			}
			else if(m_btnMgr.selected(m_homeBtnInstall))
			{
				_hideHome();
				_wbfsOp(WO_ADD_GAME);
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnAbout))
			{
				_hideHome();
				_about();
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnExitTo))
			{
				_hideHome();
				if(m_locked)
					exitHandler(WIIFLOW_DEF);
				else 
					_ExitTo();
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnExplorer))
			{
				_hideHome();
				_Explorer();
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnFTP))
			{
				_hideHome();
				_FTP();
				_showHome();
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
		if(BTN_A_PRESSED)
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
				if(!Load_Neek2o_Kernel())
				{
					error(sfmt("errneek1", L"Cannot launch neek2o. Verify your neek2o setup"));
					exitHandler(PRIILOADER_DEF);
				}
				else
				{
					bool nkWiiflow = m_cfg.getBool("NEEK2O", "launchwiiflow", true);
					if(nkWiiflow)
						exitHandler(EXIT_TO_WFNK2O);
					else
						exitHandler(EXIT_TO_SMNK2O);
				}
				break;
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

void CMenu::_showHome(void)
{
	_setBg(m_homeBg, m_homeBg);
	m_btnMgr.show(m_homeLblTitle);

	m_btnMgr.show(m_homeBtnSettings);
	m_btnMgr.show(m_homeBtnReloadCache);
	m_btnMgr.show(m_homeBtnUpdate);
	m_btnMgr.show(m_homeBtnExplorer);

	m_btnMgr.show(m_homeBtnInstall);
	m_btnMgr.show(m_homeBtnAbout);
	m_btnMgr.show(m_homeBtnExitTo);
	m_btnMgr.show(m_homeBtnFTP);

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
	m_btnMgr.show(m_homeBtnExitToPriiloader);
	m_btnMgr.show(m_homeBtnExitToBootmii);
	m_btnMgr.show(m_homeBtnExitToNeek);

	for(u8 i = 0; i < ARRAY_SIZE(m_exittoLblUser); ++i)
		if(m_exittoLblUser[i] != -1)
			m_btnMgr.show(m_exittoLblUser[i]);
}

void CMenu::_hideHome(bool instant)
{
	m_btnMgr.hide(m_homeLblTitle, instant);

	m_btnMgr.hide(m_homeBtnSettings, instant);
	m_btnMgr.hide(m_homeBtnReloadCache, instant);
	m_btnMgr.hide(m_homeBtnUpdate, instant);
	m_btnMgr.hide(m_homeBtnExplorer, instant);

	m_btnMgr.hide(m_homeBtnInstall, instant);
	m_btnMgr.hide(m_homeBtnAbout, instant);
	m_btnMgr.hide(m_homeBtnExitTo, instant);
	m_btnMgr.hide(m_homeBtnFTP, instant);

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
	m_btnMgr.hide(m_homeBtnExitToPriiloader, instant);
	m_btnMgr.hide(m_homeBtnExitToBootmii, instant);
	m_btnMgr.hide(m_homeBtnExitToNeek, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_exittoLblUser); ++i)
		if(m_exittoLblUser[i] != -1)
			m_btnMgr.hide(m_exittoLblUser[i], instant);
}

void CMenu::_initHomeAndExitToMenu()
{
	_addUserLabels(m_homeLblUser, ARRAY_SIZE(m_homeLblUser), "HOME");

	//Home Menu
	m_homeBg = _texture("HOME/BG", "texture", theme.bg, false);

	m_homeLblTitle = _addTitle("HOME/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	_setHideAnim(m_homeLblTitle, "HOME/TITLE", 0, 0, -2.f, 0.f);

	m_homeBtnSettings = _addButton("HOME/SETTINGS", theme.btnFont, L"", 60, 100, 250, 56, theme.btnFontColor);
	m_homeBtnReloadCache = _addButton("HOME/RELOAD_CACHE", theme.btnFont, L"", 60, 180, 250, 56, theme.btnFontColor);
	m_homeBtnUpdate = _addButton("HOME/UPDATE", theme.btnFont, L"", 60, 260, 250, 56, theme.btnFontColor);
	m_homeBtnExplorer = _addButton("HOME/EXPLORER", theme.btnFont, L"", 60, 340, 250, 56, theme.btnFontColor);

	m_homeBtnInstall = _addButton("HOME/INSTALL", theme.btnFont, L"", 330, 100, 250, 56, theme.btnFontColor);
	m_homeBtnAbout = _addButton("HOME/ABOUT", theme.btnFont, L"", 330, 180, 250, 56, theme.btnFontColor);
	m_homeBtnExitTo = _addButton("HOME/EXIT_TO", theme.btnFont, L"", 330, 260, 250, 56, theme.btnFontColor);
	m_homeBtnFTP = _addButton("HOME/FTP", theme.btnFont, L"", 330, 340, 250, 56, theme.btnFontColor);

	m_homeLblBattery = _addLabel("HOME/BATTERY", theme.btnFont, L"", 0, 420, 640, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);

	_setHideAnim(m_homeBtnSettings, "HOME/SETTINGS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnReloadCache, "HOME/RELOAD_CACHE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnUpdate, "HOME/UPDATE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnExplorer, "HOME/EXPLORER", 0, 0, -2.f, 0.f);

	_setHideAnim(m_homeBtnInstall, "HOME/INSTALL", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnAbout, "HOME/ABOUT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnExitTo, "HOME/EXIT_TO", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnFTP, "HOME/FTP", 0, 0, -2.f, 0.f);

	_setHideAnim(m_homeLblBattery, "HOME/BATTERY", 0, 0, -2.f, 0.f);

	_textHome();
	_hideHome(true);
	
	//ExitTo Menu
	_addUserLabels(m_exittoLblUser, ARRAY_SIZE(m_exittoLblUser), "EXIT_TO");
	m_exittoLblTitle = _addTitle("EXIT_TO/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	_setHideAnim(m_exittoLblTitle, "EXIT_TO/TITLE", 0, 0, -2.f, 0.f);

	m_homeBtnExitToHBC = _addButton("EXIT_TO/HBC", theme.btnFont, L"", 185, 120, 270, 56, theme.btnFontColor);
	m_homeBtnExitToMenu = _addButton("EXIT_TO/MENU", theme.btnFont, L"", 185, 180, 270, 56, theme.btnFontColor);
	m_homeBtnExitToPriiloader = _addButton("EXIT_TO/PRIILOADER", theme.btnFont, L"", 185, 240, 270, 56, theme.btnFontColor);
	m_homeBtnExitToBootmii = _addButton("EXIT_TO/BOOTMII", theme.btnFont, L"", 185, 300, 270, 56, theme.btnFontColor);
	m_homeBtnExitToNeek = _addButton("EXIT_TO/NEEK", theme.btnFont, L"", 185, 360, 270, 56, theme.btnFontColor);
	_setHideAnim(m_homeBtnExitToHBC, "EXIT_TO/HBC", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnExitToMenu, "EXIT_TO/MENU", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnExitToPriiloader, "EXIT_TO/PRIILOADER", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnExitToBootmii, "EXIT_TO/BOOTMII", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnExitToNeek, "EXIT_TO/NEEK", 0, 0, -2.f, 0.f);

	_textExitTo();
	_hideExitTo(true);
}

void CMenu::_textHome(void)
{
	m_btnMgr.setText(m_homeLblTitle, VERSION_STRING);
	m_btnMgr.setText(m_homeBtnSettings, _t("home1", L"Settings"));
	m_btnMgr.setText(m_homeBtnReloadCache, _t("home2", L"Reload Cache"));
	m_btnMgr.setText(m_homeBtnUpdate, _t("home3", L"Update"));
	m_btnMgr.setText(m_homeBtnExplorer, _t("home8", L"File Explorer"));

	m_btnMgr.setText(m_homeBtnInstall, _t("home7", L"Install Game"));
	m_btnMgr.setText(m_homeBtnAbout, _t("home4", L"Credits"));
	m_btnMgr.setText(m_homeBtnExitTo, _t("home5", L"Exit To"));
	m_btnMgr.setText(m_homeBtnFTP, _t("home10", L"FTP Server"));
}

void CMenu::_textExitTo(void)
{
	m_btnMgr.setText(m_exittoLblTitle, _t("exit_to", L"Exit To"));
	m_btnMgr.setText(m_homeBtnExitToHBC, _t("hbc", L"Homebrew Channel"));
	m_btnMgr.setText(m_homeBtnExitToMenu, _t("menu", L"System Menu"));
	m_btnMgr.setText(m_homeBtnExitToPriiloader, _t("prii", L"Priiloader"));
	m_btnMgr.setText(m_homeBtnExitToBootmii, _t("bootmii", L"Bootmii"));
	if(!neek2o())
		m_btnMgr.setText(m_homeBtnExitToNeek, _t("neek2o", L"neek2o"));
	else
		m_btnMgr.setText(m_homeBtnExitToNeek, _t("real", L"Real Nand"));
}
