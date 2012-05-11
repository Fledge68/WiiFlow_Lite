#include "menu.hpp"

#include <string.h>
#include <gccore.h>

u32 Startup_curPage;
u8 numPlugins;

// Startup menu
u32 m_startupLblNotice;
u32 m_startupLblPage;
u32 m_startupBtnPageM;
u32 m_startupBtnPageP;
u32 m_startupBtnBack;
u32 m_startupLblTitle;
u32 m_startupBtnSource[20];
u32 m_startupLblUser[4];
STexture m_startupBg;

void CMenu::_hideStartup(bool instant)
{
	m_btnMgr.hide(m_startupLblTitle, instant);
	m_btnMgr.hide(m_startupBtnBack, instant);
	m_btnMgr.hide(m_startupLblNotice, instant);
	m_btnMgr.hide(m_startupLblPage, instant);
	m_btnMgr.hide(m_startupBtnPageM, instant);
	m_btnMgr.hide(m_startupBtnPageP, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_startupLblUser); ++i)
	{
		if (m_startupLblUser[i] != -1u)
			m_btnMgr.hide(m_startupLblUser[i], instant);
	}

	for (int i = 0; i < 20; ++i)
	{
		m_btnMgr.hide(m_startupBtnSource[i]);
	}
}

void CMenu::_showStartup(void)
{
	_setBg(m_startupBg, m_startupBg);
	
	for (u32 i = 0; i < ARRAY_SIZE(m_startupLblUser); ++i)
	{
		if (m_startupLblUser[i] != -1u)
			m_btnMgr.show(m_startupLblUser[i]);
	}

	m_btnMgr.show(m_startupLblTitle);
	m_btnMgr.show(m_startupBtnBack);
}

void CMenu::_updateStartupBtns(void)
{
	_textStartup();
	if (numPlugins > 4)
	{
		m_btnMgr.setText(m_startupLblPage, wfmt(L"%i / 2", Startup_curPage));
		m_btnMgr.show(m_startupLblPage);
		m_btnMgr.show(m_startupBtnPageM);
		m_btnMgr.show(m_startupBtnPageP);
	}
	
	for (u8 i = 0; i < 10; ++i)
		m_btnMgr.hide(m_startupBtnSource[i]);
		
	u8 j = 6 + min((int) numPlugins, 4);
	if (Startup_curPage != 1) j = numPlugins - 4;
	for (u8 i = 0; i < j; ++i)
		m_btnMgr.show(m_startupBtnSource[i]);
}

void CMenu::_showStartupNotice(void)
{
	m_showtimer = 90;
	m_btnMgr.show(m_startupLblNotice);
}

void CMenu::_Startup()
{
	//_loadEmuList();
	_showStartup();
	DIR *pdir;
	struct dirent *pent;

	pdir = opendir(m_pluginsDir.c_str());
	Config m_plugin_cfg;

	while ((pent = readdir(pdir)) != NULL)
	{
		// Skip it
		if (strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0 
		|| strcasecmp(pent->d_name, "plugins.ini") == 0 || strcasecmp(pent->d_name, "scummvm.ini") == 0)
			continue;
		if (strcasestr(pent->d_name, ".ini") != NULL)
		{
			m_plugin_cfg.load(fmt("%s/%s", m_pluginsDir.c_str(), pent->d_name));
			if (m_plugin_cfg.loaded())
			{
				m_plugin.AddPlugin(m_plugin_cfg);
			}
		m_plugin_cfg.unload();
		}
	}
	closedir(pdir);
	m_plugin.EndAdd();
	u8 i = 0;
	while(true)
	{
		if (m_plugin.PluginExist(i))
			i++;
		else 
		{
			numPlugins = i;
			break;
		}
	}
	SetupInput();
	bool show_homebrew = !m_cfg.getBool("HOMEBREW", "disable", false);
	bool show_channel = !m_cfg.getBool("GENERAL", "hidechannel", false);
	bool show_emu = !m_cfg.getBool("EMULATOR", "disable", false);
	bool parental_homebrew = m_cfg.getBool("HOMEBREW", "parental", false);	
	bool pluginSelected = false;
	m_showtimer = 0;
	Startup_curPage = 1;
	_updateStartupBtns();
	while(true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) && (numPlugins + 6) > 11) || (BTN_A_PRESSED && m_btnMgr.selected(m_startupBtnPageM)))
		{
			Startup_curPage = Startup_curPage == 1 ? 2 : 1;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_startupBtnPageM);
			_updateStartupBtns();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED) && (numPlugins + 6) > 11) || (BTN_A_PRESSED && m_btnMgr.selected(m_startupBtnPageP)))
		{
			Startup_curPage = Startup_curPage == 1 ? 2 : 1;
			if (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_startupBtnPageP);
			_updateStartupBtns();
		}
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_startupBtnBack))
				break;
			if (Startup_curPage == 1)
			{
				if (m_btnMgr.selected(m_startupBtnSource[0]))
				{
					m_current_view = COVERFLOW_USB;
					break;
				}
				if (m_btnMgr.selected(m_startupBtnSource[2]))
				{
					if (!m_show_dml) _showStartupNotice();
					else
					{
						m_current_view = COVERFLOW_DML;
						break;
					}
				}
				if (m_btnMgr.selected(m_startupBtnSource[5]))
				{
					if (!show_emu) _showStartupNotice();
					else 
					{
						m_current_view = COVERFLOW_EMU;
						for (u8 j = 0; j < numPlugins; ++j)
							m_plugin.SetEnablePlugin(m_cfg, j, 2);
						break;
					}
				}
				if (m_btnMgr.selected(m_startupBtnSource[1]))
				{
					if (!show_channel) _showStartupNotice();
					else
					{
						m_current_view = COVERFLOW_CHANNEL;
						m_cfg.setBool("NAND", "disable", true);
						break;
					}
				}
				if (m_btnMgr.selected(m_startupBtnSource[3]))
				{
					if (!show_channel) _showStartupNotice();
					else
					{
						m_current_view = COVERFLOW_CHANNEL;
						m_cfg.setBool("NAND", "disable", false);
						break;
					}
				}
				if (m_btnMgr.selected(m_startupBtnSource[4]))
				{
					if (!show_homebrew || (!parental_homebrew && m_locked)) _showStartupNotice(); 
					else
					{
						m_current_view = COVERFLOW_HOMEBREW;
						break;
					}
				}
				for (u8 i = 6; i < (6 + min((int) numPlugins, 4)); ++i)
				{
					if (m_btnMgr.selected(m_startupBtnSource[i]))
					{
						if (!show_emu) _showStartupNotice();
						else
						{
							m_current_view = COVERFLOW_EMU;
							pluginSelected = true;
							for (u8 j = 0; j < numPlugins; ++j)
								m_plugin.SetEnablePlugin(m_cfg, j, 1);
							m_plugin.SetEnablePlugin(m_cfg, i - 6, 2);
							break;
						}
					}
				}
			}
			else
			{
				for (u8 i = 0; i < (numPlugins - 4); ++i)
				{
					if (m_btnMgr.selected(m_startupBtnSource[i]))
					{
						if (!show_emu) _showStartupNotice();
						else
						{
							m_current_view = COVERFLOW_EMU;
							pluginSelected = true;
							for (u8 j = 0; j < numPlugins; ++j)
								m_plugin.SetEnablePlugin(m_cfg, j, 1);
							m_plugin.SetEnablePlugin(m_cfg, i + 4, 2);
							break;
						}
					}
				}
			}
			if (pluginSelected) 
			{
				break;
			}
		}
		if (m_showtimer > 0)
			if (--m_showtimer == 0)
					m_btnMgr.hide(m_startupLblNotice);
	}
	_hideStartup(true);
}

void CMenu::_initStartupMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_startupLblUser, ARRAY_SIZE(m_startupLblUser), "STARTUP");
	m_startupBg = _texture(theme.texSet, "STARTUP/BG", "texture", theme.bg);
	m_startupLblTitle = _addTitle(theme, "STARTUP/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_startupBtnBack = _addButton(theme, "STARTUP/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_startupLblNotice = _addLabel(theme, "STARTUP/NOTICE", theme.btnFont, L"", 20, 400, 600, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_startupLblPage = _addLabel(theme, "STARTUP/PAGE_BTN", theme.btnFont, L"", 76, 400, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_startupBtnPageM = _addPicButton(theme, "STARTUP/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_startupBtnPageP = _addPicButton(theme, "STARTUP/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 400, 56, 56);
	for(int i = 0; i < 10; i += 2)
	{
		m_startupBtnSource[i] = _addButton(theme, fmt("STARTUP/SOURCE_BTN_%i", i), theme.btnFont, L"", 110, (100 + 58 * (i / 2)), 200, 56, theme.btnFontColor);
		m_startupBtnSource[i +1] = _addButton(theme, fmt("STARTUP/SOURCE_BTN_%i", i + 1), theme.btnFont, L"", 330, (100 + 58 * (i / 2)), 200, 56, theme.btnFontColor);
	}
	_setHideAnim(m_startupLblTitle, "STARTUP/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_startupLblNotice, "STARTUP/NOTICE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_startupLblPage, "STARTUP/PAGE_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_startupBtnPageM, "STARTUP/PAGE_MINUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_startupBtnPageP, "STARTUP/PAGE_PLUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_startupBtnBack, "STARTUP/BACK_BTN", 0, 200, 1.f, 0.f);
	for(int i = 0; i < 10; ++i)
	{
		_setHideAnim(m_startupBtnSource[i], fmt("STARTUP/SOURCE_BTN_%i", i), 0, 0, 1.f, 0.f);
	}
	_hideStartup(true);
}

void CMenu::_textStartup(void)
{
	m_btnMgr.setText(m_startupLblTitle, _t("", L"Select Source"));
	m_btnMgr.setText(m_startupBtnBack, _t("", L"Exit"));
	m_btnMgr.setText(m_startupLblNotice, _t("", L"** DISABLED **"));
	if(Startup_curPage == 1)
	{
		m_btnMgr.setText(m_startupBtnSource[0], _t("", L"Wii Games"));
		m_btnMgr.setText(m_startupBtnSource[1], _t("", L"Real NAND"));
		m_btnMgr.setText(m_startupBtnSource[2], _t("", L"GC Games"));
		m_btnMgr.setText(m_startupBtnSource[3], _t("", L"Emu NAND"));
		m_btnMgr.setText(m_startupBtnSource[4], _t("", L"Homebrew"));
		m_btnMgr.setText(m_startupBtnSource[5], _t("", L"Emulators"));
		
		if (numPlugins != 0)
		{
			for(u8 i = 0; i < min((int) numPlugins, 4); ++i)
				m_btnMgr.setText(m_startupBtnSource[i + 6], m_plugin.GetPluginName(i));
		}
	}
	else
	{
		for( u8 i = 0; i < (numPlugins - 4); ++i)
			m_btnMgr.setText(m_startupBtnSource[i], m_plugin.GetPluginName(i + 4));
	}
}

