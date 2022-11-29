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
s16 m_pluginBtnAll;
s16 m_pluginLblUser[4];
TexData m_pluginBg;

void CMenu::_hidePluginSettings(bool instant)
{
	m_btnMgr.hide(m_pluginLblTitle, instant);
	m_btnMgr.hide(m_pluginBtnBack, instant);
	m_btnMgr.hide(m_pluginLblPage, instant);
	m_btnMgr.hide(m_pluginBtnPageM, instant);
	m_btnMgr.hide(m_pluginBtnPageP, instant);
	m_btnMgr.hide(m_pluginBtnAll, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_pluginLblUser); ++i)
		if(m_pluginLblUser[i] != -1)
			m_btnMgr.hide(m_pluginLblUser[i], instant);
			
	for(u8 i = 1; i < 11; ++i)
	{
		m_btnMgr.hide(m_checkboxLblTxt[i]);
		m_btnMgr.hide(m_checkboxBtn[i]);
	}
}

void CMenu::_showPluginSettings(void)
{
	_setBg(m_pluginBg, m_pluginBg);
	for(u8 i = 0; i < ARRAY_SIZE(m_pluginLblUser); ++i)
		if(m_pluginLblUser[i] != -1)
			m_btnMgr.show(m_pluginLblUser[i]);
			
	m_btnMgr.show(m_pluginLblTitle);
	m_btnMgr.show(m_pluginBtnBack);
	m_btnMgr.show(m_pluginBtnAll);
	_updatePluginCheckboxes();
}

void CMenu::_updatePluginText(void)
{
	u32 IteratorHelp = (Plugin_curPage - 1) * 10;
	for(u8 i = 1; i < min(IteratorHelp+10, (u32)m_max_plugins)-IteratorHelp+1; i++)
		m_btnMgr.setText(m_checkboxLblTxt[i], m_plugin.GetPluginName(i+IteratorHelp-1));
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
	for(int i = 1; i < 11; ++i)
	{
		m_btnMgr.hide(m_checkboxBtn[i]);// instant?
		m_btnMgr.hide(m_checkboxLblTxt[i]);
	}
	const vector<bool> &EnabledPlugins = m_plugin.GetEnabledPlugins(&enabledPluginsCount);
	/* ALL Button */
	if(enabledPluginsCount == 0)
		m_btnMgr.setText(m_pluginBtnAll, _t("cfgpl2", L"Set All"));
	else
		m_btnMgr.setText(m_pluginBtnAll, _t("cfgpl3", L"Clear All"));
	/* Single Plugins */
	u32 IteratorHelp = (Plugin_curPage - 1) * 10;
	for(u8 i = 1; i < min(IteratorHelp+10, (u32)m_max_plugins)-IteratorHelp+1; ++i)
	{
		if(m_current_view == COVERFLOW_PLUGIN && (EnabledPlugins.size() == 0 || EnabledPlugins.at(i+IteratorHelp-1) == true))
			m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxon, theme.checkboxons, false);
		else
			m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxoff, theme.checkboxoffs, false);
		m_btnMgr.show(m_checkboxBtn[i]);
		m_btnMgr.show(m_checkboxLblTxt[i]);
	}
}

void CMenu::_PluginSettings()
{
	u8 i = 0;
	while(m_plugin.PluginExist(i)) i++;
	Plugin_Pages = static_cast<int>(ceil(static_cast<float>(i)/static_cast<float>(10)));
	m_max_plugins = i;
	if(Plugin_Pages == 0)// Only use Plugin Settings if Plugins are found
		return;
	SetupInput();
	Plugin_curPage = 1;
	_showPluginSettings();
	_updatePluginText();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_pluginBtnBack)))
		{
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
			if(m_btnMgr.selected(m_pluginBtnAll))
			{
				// if all clear then set(2) them else clear(1) them all
				for(u8 j = 0; m_plugin.PluginExist(j); j++)
					m_plugin.SetEnablePlugin(j, (enabledPluginsCount == 0) ? 2 : 1);
				_updatePluginCheckboxes();
			}
			else
			{
				u32 IteratorHelp = (Plugin_curPage - 1) * 10;
				for(u8 i = 1; i < min(IteratorHelp+10, (u32)m_max_plugins)-IteratorHelp+1; ++i)
				{
					if(m_btnMgr.selected(m_checkboxBtn[i]))
					{
						m_refreshGameList = true;
						m_plugin.SetEnablePlugin(i+IteratorHelp-1);// switch plugin from off to on or vice versa
						_updatePluginCheckboxes();
						m_btnMgr.setSelected(m_checkboxBtn[i]);
						break;
					}
				}
			}
		}
	}
	_hidePluginSettings();
	
	int channels_type = 0;
	string enabledMagics;
	enabledPluginsCount = 0;
	for(u8 i = 0; m_plugin.PluginExist(i); i++)
	{
		if(m_plugin.GetEnabledStatus(i))
		{
			enabledPluginsCount++;
			string magic = sfmt("%08x", m_plugin.GetPluginMagic(i));
			if(enabledPluginsCount == 1)
				enabledMagics = magic;
			else
				enabledMagics.append(',' + magic);
				
			if(magic == "454e414e")
				channels_type |= CHANNELS_EMU;
			else if(magic == "4e414e44")
				channels_type |= CHANNELS_REAL;
		}
	}
	m_cfg.setString(PLUGIN_DOMAIN, "enabled_plugins", enabledMagics);

	if(m_refreshGameList && enabledPluginsCount > 0)
	{
		m_cfg.setUInt("GENERAL", "sources", m_current_view);
		m_source_cnt = 1;
		m_catStartPage = 1;
		m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", channels_type);
	}
	else
		m_current_view = m_cfg.getUInt("GENERAL", "sources");
}

void CMenu::_initPluginSettingsMenu()
{
	_addUserLabels(m_pluginLblUser, ARRAY_SIZE(m_pluginLblUser), "PLUGIN");
	m_pluginBg = _texture("PLUGIN/BG", "texture", theme.bg, false);
	m_pluginLblTitle = _addLabel("PLUGIN/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_pluginBtnBack = _addButton("PLUGIN/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_pluginLblPage = _addLabel("PLUGIN/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_pluginBtnPageM = _addPicButton("PLUGIN/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_pluginBtnPageP = _addPicButton("PLUGIN/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_pluginBtnAll = _addButton("PLUGIN/ALL_BTN", theme.btnFont, L"", 230, 400, 180, 48, theme.btnFontColor);
	
	_setHideAnim(m_pluginLblTitle, "PLUGIN/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_pluginLblPage, "PLUGIN/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnPageM, "PLUGIN/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnPageP, "PLUGIN/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnBack, "PLUGIN/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pluginBtnAll, "PLUGIN/ALL_BTN", 0, 0, 1.f, -1.f);
	
	_hidePluginSettings(true);
	_textPluginSettings();
}

void CMenu::_textPluginSettings(void)
{
	m_btnMgr.setText(m_pluginLblTitle, _t("cfgpl1", L"Select Plugins"));
	m_btnMgr.setText(m_pluginBtnBack, _t("cd1", L"Back"));
}
