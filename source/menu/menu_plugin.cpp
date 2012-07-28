#include "menu.hpp"

#include <string.h>
#include <gccore.h>

u32 Plugin_curPage;
u8 Plugin_lastBtn;

// Plugin menu
u16 m_pluginLblPage;
u16 m_pluginBtnPageM;
u16 m_pluginBtnPageP;
u16 m_pluginBtnBack;
u16 m_pluginLblTitle;
u16 m_pluginLblCat[21];
u16 m_pluginBtn[21];
u16 m_pluginBtnCat[21];
u16 m_pluginBtnCats[21];
u16 m_pluginLblUser[4];
u8 m_max_plugins;
STexture m_pluginBg;

void CMenu::_hidePluginSettings(bool instant)
{
	m_btnMgr.hide(m_pluginLblTitle, instant);
	m_btnMgr.hide(m_pluginBtnBack, instant);
	m_btnMgr.hide(m_pluginLblPage, instant);
	m_btnMgr.hide(m_pluginBtnPageM, instant);
	m_btnMgr.hide(m_pluginBtnPageP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_pluginLblUser); ++i)
	{
		if(m_pluginLblUser[i] != (u16)-1)
			m_btnMgr.hide(m_pluginLblUser[i], instant);
	}

	for(int i = 0; i < 21; ++i)
	{
		m_btnMgr.hide(m_pluginLblCat[i]);
		m_btnMgr.hide(m_pluginBtn[i]);
	}
}

void CMenu::_showPluginSettings(void)
{
	_setBg(m_pluginBg, m_pluginBg);
	for(u8 i = 0; i < ARRAY_SIZE(m_pluginLblUser); ++i)
	{
		if(m_pluginLblUser[i] != (u16)-1)
			m_btnMgr.show(m_pluginLblUser[i]);
	}
	m_btnMgr.show(m_pluginLblTitle);
	m_btnMgr.show(m_pluginBtnBack);
	_updatePluginCheckboxes();
}

void CMenu::_updatePluginCheckboxes(void)
{
	if(m_max_plugins > 10)
	{
		m_btnMgr.setText(m_pluginLblPage, wfmt(L"%i / 2", Plugin_curPage));
		m_btnMgr.show(m_pluginLblPage);
		m_btnMgr.show(m_pluginBtnPageM);
		m_btnMgr.show(m_pluginBtnPageP);
	}
	for(int i = 0; i < 21; ++i)
	{
		m_btnMgr.hide(m_pluginBtn[i]);
		m_btnMgr.hide(m_pluginLblCat[i]);
	}

	vector<bool> EnabledPlugins = m_plugin.GetEnabledPlugins(m_cfg);
	if(Plugin_curPage == 1)
	{
		int j = 11;
		if(m_max_plugins < 11)
			j = m_max_plugins;
		for(u8 i = 0; i < j; ++i)
		{
			if((EnabledPlugins.size() == 0) || (i != 0 && EnabledPlugins.size() >= i && EnabledPlugins[i - 1] == true))
				m_pluginBtn[i] = m_pluginBtnCats[i];
			else
				m_pluginBtn[i] = m_pluginBtnCat[i];
			m_btnMgr.show(m_pluginBtn[i]);
			m_btnMgr.show(m_pluginLblCat[i]);
		}
	}
	else
	{
		for(int i = 11; i < m_max_plugins; ++i)
		{
			m_pluginBtn[i] = m_pluginBtnCat[i];
			m_btnMgr.show(m_pluginBtn[i]);
			m_btnMgr.show(m_pluginLblCat[i]);
		}
	}
}

void CMenu::_PluginSettings()
{
	SetupInput();
	Plugin_curPage = 1;
	_textPluginSettings();
	_showPluginSettings();
	while(true)
	{
		_mainLoopCommon();
		if(!m_btnMgr.selected(Plugin_lastBtn))
			m_btnMgr.noHover(false);
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			m_cfg.save();
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) || (BTN_A_PRESSED && m_btnMgr.selected(m_pluginBtnPageM)))
		{
			Plugin_lastBtn = m_pluginBtnPageM;
			m_btnMgr.noHover(true);
			Plugin_curPage = Plugin_curPage == 1 ? 2 : 1;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_pluginBtnPageM);
			_updatePluginCheckboxes();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED)) || (BTN_A_PRESSED && m_btnMgr.selected(m_pluginBtnPageP)))
		{
			Plugin_lastBtn = m_pluginBtnPageP;
			m_btnMgr.noHover(true);
			Plugin_curPage = Plugin_curPage == 1 ? 2 : 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_pluginBtnPageP);
			_updatePluginCheckboxes();
		}
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_pluginBtnBack))
			{
				m_cfg.save();
				break;
			}
			for(int i = 0; i < 21; ++i)
			{
				if(m_btnMgr.selected(m_pluginBtn[i]))
				{
					Plugin_lastBtn = m_pluginBtn[i];
					m_btnMgr.noHover(true);
					if(i == 0)
					{
						int j = 0;
						bool EnableAll = (m_plugin.GetEnabledPlugins(m_cfg).size());
						while(true)
						{
								if(m_plugin.PluginExist(j))
									m_plugin.SetEnablePlugin(m_cfg, j, EnableAll ? 2 : 1);
								else
									break;
							j++;
						}
					}
					else
						m_plugin.SetEnablePlugin(m_cfg, i - 1);
					_updatePluginCheckboxes();
					break;
				}
			}
		}
	}
	_hidePluginSettings();
}

void CMenu::_initPluginSettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_pluginLblUser, ARRAY_SIZE(m_pluginLblUser), "PLUGIN");
	m_pluginBg = _texture(theme.texSet, "PLUGIN/BG", "texture", theme.bg);
	m_pluginLblTitle = _addTitle(theme, "PLUGIN/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_pluginBtnBack = _addButton(theme, "PLUGIN/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_pluginLblPage = _addLabel(theme, "PLUGIN/PAGE_BTN", theme.btnFont, L"", 256, 400, 100, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_pluginBtnPageM = _addPicButton(theme, "PLUGIN/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 200, 400, 56, 56);
	m_pluginBtnPageP = _addPicButton(theme, "PLUGIN/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 356, 400, 56, 56);
	m_pluginBtnCat[0] = _addPicButton(theme, "PLUGIN/PLUGIN_0_BTN", theme.checkboxoff, theme.checkboxoffs, 30, 390, 44, 48);
	m_pluginBtnCats[0] = _addPicButton(theme, "PLUGIN/PLUGIN_0_BTNS", theme.checkboxon, theme.checkboxons, 30, 390, 44, 48);
	m_pluginLblCat[0] = _addLabel(theme, "PLUGIN/PLUGIN_0", theme.lblFont, L"", 85, 390, 100, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	for(int i = 1; i < 6; ++i)
	{ 	// Page 1
		m_pluginBtnCat[i] = _addPicButton(theme, fmt("PLUGIN/PLUGIN_%i_BTN", i), theme.checkboxoff, theme.checkboxoffs, 30, (42+i*58), 44, 48);
		m_pluginBtnCats[i] = _addPicButton(theme, fmt("PLUGIN/PLUGIN_%i_BTNS", i), theme.checkboxon, theme.checkboxons, 30, (42+i*58), 44, 48);
		m_pluginLblCat[i] = _addLabel(theme, fmt("PLUGIN/PLUGIN_%i", i), theme.lblFont, L"", 85, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// right half
		m_pluginBtnCat[i+5] = _addPicButton(theme, fmt("PLUGIN/PLUGIN_%i_BTN", i+5), theme.checkboxoff, theme.checkboxoffs, 325, (42+i*58), 44, 48);
		m_pluginBtnCats[i+5] = _addPicButton(theme, fmt("PLUGIN/PLUGIN_%i_BTNS", i+5), theme.checkboxon, theme.checkboxons, 325, (42+i*58), 44, 48);
		m_pluginLblCat[i+5] = _addLabel(theme, fmt("PLUGIN/PLUGIN_%i", i+5), theme.txtFont, L"", 380, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// Page 2
		m_pluginBtnCat[i+10] = _addPicButton(theme, fmt("PLUGIN/PLUGIN_%i_BTN", i+10), theme.checkboxoff, theme.checkboxoffs, 30, (42+i*58), 44, 48);
		m_pluginBtnCats[i+10] = _addPicButton(theme, fmt("PLUGIN/PLUGIN_%i_BTNS", i+10), theme.checkboxon, theme.checkboxons, 30, (42+i*58), 44, 48);
		m_pluginLblCat[i+10] = _addLabel(theme, fmt("PLUGIN/PLUGIN_%i", i+10), theme.lblFont, L"", 85, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// right half
		m_pluginBtnCat[i+15] = _addPicButton(theme, fmt("PLUGIN/PLUGIN_%i_BTN", i+15), theme.checkboxoff, theme.checkboxoffs, 325, (42+i*58), 44, 48);
		m_pluginBtnCats[i+15] = _addPicButton(theme, fmt("PLUGIN/PLUGIN_%i_BTNS", i+15), theme.checkboxon, theme.checkboxons, 325, (42+i*58), 44, 48);
		m_pluginLblCat[i+15] = _addLabel(theme, fmt("PLUGIN/PLUGIN_%i", i+15), theme.txtFont, L"", 380, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	}
	_setHideAnim(m_pluginLblTitle, "PLUGIN/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_pluginLblPage, "PLUGIN/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnPageM, "PLUGIN/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnPageP, "PLUGIN/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnBack, "PLUGIN/BACK_BTN", 0, 0, 1.f, -1.f);
	for(u8 i = 0; i < 21; ++i)
	{
		_setHideAnim(m_pluginBtnCat[i], fmt("PLUGIN/PLUGIN_%i_BTN", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_pluginBtnCats[i], fmt("PLUGIN/PLUGIN_%i_BTNS", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_pluginLblCat[i], fmt("PLUGIN/PLUGIN_%i", i), 0, 0, 1.f, 0.f);
		m_pluginBtn[i] = m_pluginBtnCat[i];
	}
	m_max_plugins = 0;
	_hidePluginSettings(true);
	_textPluginSettings();
}

void CMenu::_textPluginSettings(void)
{
	m_btnMgr.setText(m_pluginLblTitle, _t("cfgpl1", L"Select Plugins"));
	m_btnMgr.setText(m_pluginBtnBack, _t("cd1", L"Back"));
	u8 i = 0;
	while(true)
	{
		if(i == 0)
			m_btnMgr.setText(m_pluginLblCat[i], _t("dl25", L"All"));
		else
		{
			if(m_plugin.PluginExist(i - 1))
				m_btnMgr.setText(m_pluginLblCat[i], m_plugin.GetPluginName(i - 1));
			else
			{
				m_max_plugins = i;
				break;
			}
		}
		i++;
	}
}
