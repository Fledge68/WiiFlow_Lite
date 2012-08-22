
#include "menu.hpp"
#include "svnrev.h"
#include "loader/cios.h"
#include "loader/nk.h"

u32 m_homeLblTitle;
u32 m_exittoLblTitle;

u32 m_homeBtnSettings;
u32 m_homeBtnReloadCache;
u32 m_homeBtnUpdate;
u32 m_homeBtnHelp;
u32 m_homeBtnAbout;
u32 m_homeBtnExitTo;

u32 m_homeBtnExitToHBC;
u32 m_homeBtnExitToMenu;
u32 m_homeBtnExitToPriiloader;
u32 m_homeBtnExitToBootmii;
u32 m_homeBtnExitToNeek;

STexture m_homeBg;

bool CMenu::_Home(void)
{
	SetupInput();
	_showHome();

	string prevTheme = m_cfg.getString("GENERAL", "theme", "default");
	while(1)
	{
		_mainLoopCommon();
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
				m_gameList.SetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
				UpdateCache(m_current_view);
				LoadView();
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnUpdate) && !m_locked)
			{
				m_cf.stopCoverLoader(true);
				_hideHome();
				_system();
				remove(m_ver.c_str());
				if(m_exit)
				{
					_launchHomebrew(m_dol.c_str(), m_homebrewArgs);
					break;
				}
				_showHome();
				m_cf.startCoverLoader();
			}
			else if(m_btnMgr.selected(m_homeBtnHelp))
			{
				_hideHome();
				_about(true);
				if(m_exit)
					break;
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnAbout))
			{
				_hideHome();
				_about();
				if(m_exit)
					break;
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnExitTo))
			{
				_hideHome();
				if(m_locked)
					exitHandler(0);
				else 
					_ExitTo();
				if(m_exit)
					break;
				_showHome();
			}
		}
		else if(BTN_HOME_PRESSED)
		{
			exitHandler(0);			
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

	while(1)
	{
		_mainLoopCommon();
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_homeBtnExitToHBC))
			{
				exitHandler(1);
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnExitToMenu))
			{
				exitHandler(2);
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnExitToPriiloader))
			{
				exitHandler(3);
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnExitToBootmii))
			{
				exitHandler(4);
				break;
			}
			else if(m_btnMgr.selected(m_homeBtnExitToNeek))
			{
				if(!Load_Neek2o_Kernel())
				{
					error(sfmt("errneek1", L"Cannot launch neek2o. Verify your neek2o setup"));
					exitHandler(2);
				}
				else
					exitHandler(5);
				break;
			}
		}
		else if(BTN_HOME_PRESSED)
		{
			exitHandler(0);
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
	m_btnMgr.show(m_homeBtnHelp);
	m_btnMgr.show(m_homeBtnAbout);	
	m_btnMgr.show(m_homeBtnExitTo);
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
}

void CMenu::_hideHome(bool instant)
{
	m_btnMgr.hide(m_homeLblTitle, instant);

	m_btnMgr.hide(m_homeBtnSettings, instant);
	m_btnMgr.hide(m_homeBtnReloadCache, instant);
	m_btnMgr.hide(m_homeBtnUpdate, instant);
	m_btnMgr.hide(m_homeBtnHelp, instant);
	m_btnMgr.hide(m_homeBtnAbout, instant);	
	m_btnMgr.hide(m_homeBtnExitTo, instant);
}

void CMenu::_hideExitTo(bool instant)
{
	m_btnMgr.hide(m_exittoLblTitle, instant);

	m_btnMgr.hide(m_homeBtnExitToHBC, instant);
	m_btnMgr.hide(m_homeBtnExitToMenu, instant);
	m_btnMgr.hide(m_homeBtnExitToPriiloader, instant);
	m_btnMgr.hide(m_homeBtnExitToBootmii, instant);
	m_btnMgr.hide(m_homeBtnExitToNeek, instant);
}

void CMenu::_initHomeAndExitToMenu(CMenu::SThemeData &theme)
{
    //Home Menu
	STexture emptyTex;
	m_homeBg = _texture(theme.texSet, "HOME/BG", "texture", theme.bg);

	m_homeLblTitle = _addTitle(theme, "HOME/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	_setHideAnim(m_homeLblTitle, "HOME/TITLE", 0, 0, -2.f, 0.f);

	m_homeBtnSettings = _addButton(theme, "HOME/SETTINGS", theme.btnFont, L"", 60, 120, 250, 56, theme.btnFontColor);
	m_homeBtnReloadCache = _addButton(theme, "HOME/RELOAD_CACHE", theme.btnFont, L"", 60, 230, 250, 56, theme.btnFontColor);
	m_homeBtnUpdate = _addButton(theme, "HOME/UPDATE", theme.btnFont, L"", 60, 340, 250, 56, theme.btnFontColor);
	m_homeBtnHelp = _addButton(theme, "HOME/HELP", theme.btnFont, L"", 330, 120, 250, 56, theme.btnFontColor);
	m_homeBtnAbout = _addButton(theme, "HOME/ABOUT", theme.btnFont, L"", 330, 230, 250, 56, theme.btnFontColor);
	m_homeBtnExitTo = _addButton(theme, "HOME/EXIT_TO", theme.btnFont, L"", 330, 340, 250, 56, theme.btnFontColor);

	_setHideAnim(m_homeBtnSettings, "HOME/SETTINGS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnReloadCache, "HOME/RELOAD_CACHE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnUpdate, "HOME/UPDATE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnHelp, "HOME/HELP", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnAbout, "HOME/ABOUT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnExitTo, "HOME/EXIT_TO", 0, 0, -2.f, 0.f);

	_textHome();
	_hideHome(true);
	
	//ExitTo Menu	
	m_exittoLblTitle = _addTitle(theme, "EXIT_TO/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	_setHideAnim(m_exittoLblTitle, "EXIT_TO/TITLE", 0, 0, -2.f, 0.f);

	m_homeBtnExitToHBC = _addButton(theme, "EXIT_TO/HBC", theme.btnFont, L"", 185, 120, 270, 56, theme.btnFontColor);
	m_homeBtnExitToMenu = _addButton(theme, "EXIT_TO/MENU", theme.btnFont, L"", 185, 180, 270, 56, theme.btnFontColor);
	m_homeBtnExitToPriiloader = _addButton(theme, "EXIT_TO/PRIILOADER", theme.btnFont, L"", 185, 240, 270, 56, theme.btnFontColor);
	m_homeBtnExitToBootmii = _addButton(theme, "EXIT_TO/BOOTMII", theme.btnFont, L"", 185, 300, 270, 56, theme.btnFontColor);
	m_homeBtnExitToNeek = _addButton(theme, "EXIT_TO/NEEK", theme.btnFont, L"", 185, 360, 270, 56, theme.btnFontColor);
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
	m_btnMgr.setText(m_homeLblTitle, wfmt( L"%s (%s-r%s)", APP_NAME, APP_VERSION, SVN_REV), false);
	
	m_btnMgr.setText(m_homeBtnSettings, _t("home1", L"Settings"));
	m_btnMgr.setText(m_homeBtnReloadCache, _t("home2", L"Reload Cache"));
	m_btnMgr.setText(m_homeBtnUpdate, _t("home3", L"Update"));
	m_btnMgr.setText(m_homeBtnHelp, _t("home6", L"Help"));
	m_btnMgr.setText(m_homeBtnAbout, _t("home4", L"About"));
	m_btnMgr.setText(m_homeBtnExitTo, _t("home5", L"Exit To"));
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