#include "menu.hpp"

#include <string.h>
#include <gccore.h>
#include <cmath>

u8 m_max_plugins = 0;
u8 Plugin_curPage = 1;
u8 Plugin_Pages = 1;

// Plugin menu
s16 m_pluginLblPage;
s16 m_pluginBtnPageM;
s16 m_pluginBtnPageP;
s16 m_pluginBtnBack;
s16 m_pluginLblTitle;
s16 m_pluginLblCat[11];
s16 m_pluginBtn[11];
s16 m_pluginBtnCat[11];
s16 m_pluginBtnCats[11];
s16 m_pluginLblUser[4];
TexData m_pluginBg;

void CMenu::_hidePluginSettings(bool instant)
{
	m_btnMgr.hide(m_pluginLblTitle, instant);
	m_btnMgr.hide(m_pluginBtnBack, instant);
	m_btnMgr.hide(m_pluginLblPage, instant);
	m_btnMgr.hide(m_pluginBtnPageM, instant);
	m_btnMgr.hide(m_pluginBtnPageP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_pluginLblUser); ++i)
	{
		if(m_pluginLblUser[i] != -1)
			m_btnMgr.hide(m_pluginLblUser[i], instant);
	}
	for(u8 i = 0; i < 11; ++i)
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
		if(m_pluginLblUser[i] != -1)
			m_btnMgr.show(m_pluginLblUser[i]);
	}
	m_btnMgr.show(m_pluginLblTitle);
	m_btnMgr.show(m_pluginBtnBack);
	_updatePluginCheckboxes();
}

void CMenu::_updatePluginText(void)
{
	u32 IteratorHelp = (Plugin_curPage - 1) * 10;
	for(u8 i = 1; i < min(IteratorHelp+10, (u32)m_max_plugins)-IteratorHelp+1; i++)
		m_btnMgr.setText(m_pluginLblCat[i], m_plugin.GetPluginName(i+IteratorHelp-1));
}

void CMenu::_updatePluginCheckboxes(void)
{
	if(m_max_plugins > 10)
	{
		m_btnMgr.setText(m_pluginLblPage, wfmt(L"%i / %i", Plugin_curPage, Plugin_Pages));
		m_btnMgr.show(m_pluginLblPage);
		m_btnMgr.show(m_pluginBtnPageM);
		m_btnMgr.show(m_pluginBtnPageP);
	}
	for(int i = 0; i < 11; ++i)
	{
		m_btnMgr.hide(m_pluginBtn[i]);
		m_btnMgr.hide(m_pluginLblCat[i]);
	}
	const vector<bool> &EnabledPlugins = m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
	/* ALL Button */
	if(EnabledPlugins.size() == 0)
		m_pluginBtn[0] = m_pluginBtnCats[0];
	else
		m_pluginBtn[0] = m_pluginBtnCat[0];
	m_btnMgr.show(m_pluginBtn[0]);
	m_btnMgr.show(m_pluginLblCat[0]);
	/* Single Plugins */
	u32 IteratorHelp = (Plugin_curPage - 1) * 10;
	for(u8 i = 1; i < min(IteratorHelp+10, (u32)m_max_plugins)-IteratorHelp+1; ++i)
	{
		if(EnabledPlugins.size() == 0 || EnabledPlugins.at(i+IteratorHelp-1) == true)
			m_pluginBtn[i] = m_pluginBtnCats[i];
		else
			m_pluginBtn[i] = m_pluginBtnCat[i];
		m_btnMgr.show(m_pluginBtn[i]);
		m_btnMgr.show(m_pluginLblCat[i]);
	}
}

void CMenu::_PluginSettings()
{
	u8 i = 0;
	while(m_plugin.PluginExist(i)) i++;
	Plugin_Pages = static_cast<int>(ceil(static_cast<float>(i)/static_cast<float>(10)));
	m_max_plugins = i;
	//gprintf("Plugins found: %i, Pages: %i\n", m_max_plugins, Plugin_Pages);
	if(Plugin_Pages == 0)
		return;
	/* Only use Plugin Settings if Plugins are found */
	SetupInput();
	Plugin_curPage = 1;
	_showPluginSettings();
	_updatePluginText();
	while(!m_exit)
	{
		_mainLoopCommon();
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
			Plugin_curPage--;
			if(Plugin_curPage == 0) Plugin_curPage = Plugin_Pages;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_pluginBtnPageM);
			_updatePluginCheckboxes();
			_updatePluginText();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED)) || (BTN_A_PRESSED && m_btnMgr.selected(m_pluginBtnPageP)))
		{
			Plugin_curPage++;
			if(Plugin_curPage > Plugin_Pages) Plugin_curPage = 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_pluginBtnPageP);
			_updatePluginCheckboxes();
			_updatePluginText();
		}
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_pluginBtnBack))
			{
				m_cfg.save();
				break;
			}
			u32 IteratorHelp = (Plugin_curPage - 1) * 10;
			for(u8 i = 0; i < min(IteratorHelp+10, (u32)m_max_plugins)-IteratorHelp+1; ++i)
			{
				if(m_btnMgr.selected(m_pluginBtn[i]))
				{
					if(i == 0)
					{
						m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
						for(u8 j = 0; m_plugin.PluginExist(j); j++)
							m_plugin.SetEnablePlugin(m_cfg, j, (enabledPluginsCount == 0) ? 2 : 1);
					}
					else
						m_plugin.SetEnablePlugin(m_cfg, i+IteratorHelp-1);
					_updatePluginCheckboxes();
					break;
				}
			}
		}
	}
	_hidePluginSettings();
	_loadList();
}

void CMenu::_initPluginSettingsMenu()
{
	_addUserLabels(m_pluginLblUser, ARRAY_SIZE(m_pluginLblUser), "PLUGIN");
	m_pluginBg = _texture("PLUGIN/BG", "texture", theme.bg, false);
	m_pluginLblTitle = _addTitle("PLUGIN/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_pluginBtnBack = _addButton("PLUGIN/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_pluginLblPage = _addLabel("PLUGIN/PAGE_BTN", theme.btnFont, L"", 256, 400, 100, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_pluginBtnPageM = _addPicButton("PLUGIN/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 200, 400, 56, 56);
	m_pluginBtnPageP = _addPicButton("PLUGIN/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 356, 400, 56, 56);
	m_pluginBtnCat[0] = _addPicButton("PLUGIN/PLUGIN_0_BTN", theme.checkboxoff, theme.checkboxoffs, 30, 390, 44, 48);
	m_pluginBtnCats[0] = _addPicButton("PLUGIN/PLUGIN_0_BTNS", theme.checkboxon, theme.checkboxons, 30, 390, 44, 48);
	m_pluginLblCat[0] = _addLabel("PLUGIN/PLUGIN_0", theme.lblFont, L"", 85, 390, 100, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	for(int i = 1; i < 6; ++i)
	{ 	// Page 1
		m_pluginBtnCat[i] = _addPicButton(fmt("PLUGIN/PLUGIN_%i_BTN", i), theme.checkboxoff, theme.checkboxoffs, 30, (42+i*58), 44, 48);
		m_pluginBtnCats[i] = _addPicButton(fmt("PLUGIN/PLUGIN_%i_BTNS", i), theme.checkboxon, theme.checkboxons, 30, (42+i*58), 44, 48);
		m_pluginLblCat[i] = _addLabel(fmt("PLUGIN/PLUGIN_%i", i), theme.lblFont, L"", 85, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// right half
		m_pluginBtnCat[i+5] = _addPicButton(fmt("PLUGIN/PLUGIN_%i_BTN", i+5), theme.checkboxoff, theme.checkboxoffs, 325, (42+i*58), 44, 48);
		m_pluginBtnCats[i+5] = _addPicButton(fmt("PLUGIN/PLUGIN_%i_BTNS", i+5), theme.checkboxon, theme.checkboxons, 325, (42+i*58), 44, 48);
		m_pluginLblCat[i+5] = _addLabel(fmt("PLUGIN/PLUGIN_%i", i+5), theme.txtFont, L"", 380, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	}
	_setHideAnim(m_pluginLblTitle, "PLUGIN/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_pluginLblPage, "PLUGIN/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnPageM, "PLUGIN/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnPageP, "PLUGIN/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnBack, "PLUGIN/BACK_BTN", 0, 0, 1.f, -1.f);
	for(u8 i = 0; i < 11; ++i)
	{
		_setHideAnim(m_pluginBtnCat[i], fmt("PLUGIN/PLUGIN_%i_BTN", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_pluginBtnCats[i], fmt("PLUGIN/PLUGIN_%i_BTNS", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_pluginLblCat[i], fmt("PLUGIN/PLUGIN_%i", i), 0, 0, 1.f, 0.f);
		m_pluginBtn[i] = m_pluginBtnCat[i];
	}
	_hidePluginSettings(true);
	_textPluginSettings();
}

void CMenu::_textPluginSettings(void)
{
	m_btnMgr.setText(m_pluginLblTitle, _t("cfgpl1", L"Select Plugins"));
	m_btnMgr.setText(m_pluginBtnBack, _t("cd1", L"Back"));
	m_btnMgr.setText(m_pluginLblCat[0], _t("dl25", L"All"));
}
