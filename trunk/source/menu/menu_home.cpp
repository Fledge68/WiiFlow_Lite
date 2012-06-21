
#include "menu.hpp"
#include "svnrev.h"

u32 m_homeLblTitle;

u32 m_homeBtnSettings;
u32 m_homeBtnReloadCache;
u32 m_homeBtnUpdate;
u32 m_homeBtnAbout;
u32 m_homeBtnExit;

STexture m_homeBg;

bool CMenu::_Home(void)
{
	bool exit = false;

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
					exit = true;
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
			else if(m_btnMgr.selected(m_homeBtnAbout))
			{
				_hideHome();
				_about();
				if(m_exit)
					break;
				_showHome();
			}
			else if(m_btnMgr.selected(m_homeBtnExit))
			{
				exitHandler();
				exit = true;
				break;
			}
		}
		else if(BTN_B_PRESSED)
		{
			break;
		}
		else if(BTN_HOME_PRESSED)
		{
			exitHandler();
			exit = true;
			break;
		}
	}
	_hideHome();
	return exit;
}

void CMenu::_showHome(void)
{
	_setBg(m_homeBg, m_homeBg);
	m_btnMgr.show(m_homeLblTitle);

	m_btnMgr.show(m_homeBtnSettings);
	m_btnMgr.show(m_homeBtnReloadCache);
	m_btnMgr.show(m_homeBtnUpdate);
	m_btnMgr.show(m_homeBtnAbout);
	m_btnMgr.show(m_homeBtnExit);
}

void CMenu::_hideHome(bool instant)
{
	m_btnMgr.hide(m_homeLblTitle, instant);

	m_btnMgr.hide(m_homeBtnSettings, instant);
	m_btnMgr.hide(m_homeBtnReloadCache, instant);
	m_btnMgr.hide(m_homeBtnUpdate, instant);
	m_btnMgr.hide(m_homeBtnAbout, instant);
	m_btnMgr.hide(m_homeBtnExit, instant);
}

void CMenu::_initHomeMenu(CMenu::SThemeData &theme)
{
	STexture emptyTex;
	m_homeBg = _texture(theme.texSet, "HOME/BG", "texture", theme.bg);

	m_homeLblTitle = _addTitle(theme, "HOME/TITLE", theme.titleFont, L"", 20, 30, 600, 75, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	_setHideAnim(m_homeLblTitle, "HOME/TITLE", 0, 100, 0.f, 0.f);

	m_homeBtnSettings = _addButton(theme, "HOME/SETTINGS", theme.btnFont, L"", 220, 120, 200, 56, theme.btnFontColor);
	m_homeBtnReloadCache = _addButton(theme, "HOME/RELOAD_CACHE", theme.btnFont, L"", 220, 180, 200, 56, theme.btnFontColor);
	m_homeBtnUpdate = _addButton(theme, "HOME/UPDATE", theme.btnFont, L"", 220, 240, 200, 56, theme.btnFontColor);
	m_homeBtnAbout = _addButton(theme, "HOME/ABOUT", theme.btnFont, L"", 220, 300, 200, 56, theme.btnFontColor);
	m_homeBtnExit = _addButton(theme, "HOME/EXIT", theme.btnFont, L"", 220, 360, 200, 56, theme.btnFontColor);

	_setHideAnim(m_homeBtnSettings, "HOME/SETTINGS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnReloadCache, "HOME/RELOAD_CACHE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnUpdate, "HOME/UPDATE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnAbout, "HOME/ABOUT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_homeBtnExit, "HOME/EXIT", 0, 0, -2.f, 0.f);

	_textHome();
	_hideHome(true);
}

void CMenu::_textHome(void)
{
	m_btnMgr.setText(m_homeLblTitle, wfmt(_fmt("appname", L"%s (%s-r%s)"), APP_NAME, APP_VERSION, SVN_REV), false);
	
	m_btnMgr.setText(m_homeBtnSettings, _t("home1", L"Settings"));
	m_btnMgr.setText(m_homeBtnReloadCache, _t("home2", L"Reload Cache"));
	m_btnMgr.setText(m_homeBtnUpdate, _t("home3", L"Update"));
	m_btnMgr.setText(m_homeBtnAbout, _t("home4", L"About"));
	m_btnMgr.setText(m_homeBtnExit, _t("home5", L"Exit"));
}
