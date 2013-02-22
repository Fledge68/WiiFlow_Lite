#include <dirent.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include "menu.hpp"
#include "loader/wbfs.h"

using namespace std;

static const int g_curPage = 2;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfigAdv(bool instant)
{
	_hideConfigCommon(instant);

	m_btnMgr.hide(m_configAdvLblBootChange, instant);
	m_btnMgr.hide(m_configAdvBtnBootChange, instant);
	m_btnMgr.hide(m_configAdvLblTheme, instant);
	m_btnMgr.hide(m_configAdvLblCurTheme, instant);
	m_btnMgr.hide(m_configAdvBtnCurThemeM, instant);
	m_btnMgr.hide(m_configAdvBtnCurThemeP, instant);
	m_btnMgr.hide(m_configAdvLblLanguage, instant);
	m_btnMgr.hide(m_configAdvLblCurLanguage, instant);
	m_btnMgr.hide(m_configAdvBtnCurLanguageM, instant);
	m_btnMgr.hide(m_configAdvBtnCurLanguageP, instant);
	m_btnMgr.hide(m_configAdvLblCFTheme, instant);
	m_btnMgr.hide(m_configAdvBtnCFTheme, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_configAdvLblUser); ++i)
		if(m_configAdvLblUser[i] != -1)
			m_btnMgr.hide(m_configAdvLblUser[i], instant);
}

void CMenu::_showConfigAdv(void)
{
	_showConfigCommon(m_configAdvBg, g_curPage);

	m_btnMgr.show(m_configAdvLblCurTheme);
	m_btnMgr.show(m_configAdvBtnCurThemeM);
	m_btnMgr.show(m_configAdvBtnCurThemeP);
	m_btnMgr.show(m_configAdvLblTheme);
	if(!m_locked)
	{
		m_btnMgr.show(m_configAdvLblBootChange);
		m_btnMgr.show(m_configAdvBtnBootChange);
		m_btnMgr.show(m_configAdvLblLanguage);
		m_btnMgr.show(m_configAdvLblCurLanguage);
		m_btnMgr.show(m_configAdvBtnCurLanguageM);
		m_btnMgr.show(m_configAdvBtnCurLanguageP);
		m_btnMgr.show(m_configAdvLblCFTheme);
		m_btnMgr.show(m_configAdvBtnCFTheme);
	}
	for(u32 i = 0; i < ARRAY_SIZE(m_configAdvLblUser); ++i)
		if(m_configAdvLblUser[i] != -1)
			m_btnMgr.show(m_configAdvLblUser[i]);

	m_btnMgr.setText(m_configAdvLblCurLanguage, m_curLanguage);
	m_btnMgr.setText(m_configAdvLblCurTheme, m_cfg.getString("GENERAL", "theme"));
}

static void listThemes(const char * path, vector<string> &themes)
{
	DIR *d;
	struct dirent *dir;
	bool def = false;

	themes.clear();
	d = opendir(path);
	if (d != 0)
	{
		dir = readdir(d);
		while (dir != 0)
		{
			string fileName = dir->d_name;
			def = def || (upperCase(fileName) == "DEFAULT.INI");
			if (fileName.size() > 4 && fileName.substr(fileName.size() - 4, 4) == ".ini")
				themes.push_back(fileName.substr(0, fileName.size() - 4));
			dir = readdir(d);
		}
		closedir(d);
	}
	if (!def)
		themes.push_back("DEFAULT");
	sort(themes.begin(), themes.end());
}

int CMenu::_configAdv(void)
{
	int change = CONFIG_PAGE_NO_CHANGE;
	vector<string> themes;
	string prevTheme = m_cfg.getString("GENERAL", "theme");

	bool lang_changed = false;

	listThemes(m_themeDir.c_str(), themes);
	int curTheme = 0;
	for (u32 i = 0; i < themes.size(); ++i)
		if (themes[i] == prevTheme)
		{
			curTheme = i;
			break;
		}
	_showConfigAdv();
	while(!m_exit)
	{
		change = _configCommon();
		if (change != CONFIG_PAGE_NO_CHANGE)
			break;
		if (BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_configAdvBtnBootChange))
			{
				_hideConfigAdv();
				if(_Boot())
					break; /* Settings changed */
				_showConfigAdv();
			}
			else if (m_btnMgr.selected(m_configAdvBtnCurThemeP) || m_btnMgr.selected(m_configAdvBtnCurThemeM))
			{
				_cfNeedsUpdate();
				s8 direction = m_btnMgr.selected(m_configAdvBtnCurThemeP) ? 1 : -1;
				curTheme = loopNum(curTheme + direction, (int)themes.size());
				m_cfg.setString("GENERAL", "theme", themes[curTheme]);
				m_cfg.setInt(_domainFromView(), "last_cf_mode", 1);
				_showConfigAdv();
			}
			else if (m_btnMgr.selected(m_configAdvBtnCurLanguageP) || m_btnMgr.selected(m_configAdvBtnCurLanguageM))
			{
				_cfNeedsUpdate();
				s8 direction = m_btnMgr.selected(m_configAdvBtnCurLanguageP) ? 1 : -1;
				int lang = (int)loopNum((u32)m_cfg.getInt("GENERAL", "language", 0) + direction, ARRAY_SIZE(CMenu::_translations));
				m_curLanguage = CMenu::_translations[lang];
				if (m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), lowerCase(m_curLanguage).c_str())))
				{
					m_cfg.setInt("GENERAL", "language", lang);
					lang_changed = true;
				}
				else
				{
					while (lang !=0)
					{
						lang = (int)loopNum((u32)lang + direction, ARRAY_SIZE(CMenu::_translations));
						m_curLanguage = CMenu::_translations[lang];
						struct stat langs;
						if (stat(fmt("%s/%s.ini", m_languagesDir.c_str(), lowerCase(m_curLanguage).c_str()), &langs) == 0)
							break;
					}
					m_cfg.setInt("GENERAL", "language", lang);
					lang_changed = true;
					m_curLanguage = CMenu::_translations[lang];
					m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), lowerCase(m_curLanguage).c_str()));
				}
				_updateText();
				_showConfigAdv();
			}
			else if (m_btnMgr.selected(m_configAdvBtnCFTheme))
			{
				_cfNeedsUpdate();
				_hideConfigAdv();
				_cfTheme();
				_showConfigAdv();
			}
		}
	}
	_hideConfigAdv();
	if (m_gameList.empty() || lang_changed)
	{
		//if(lang_changed)
			//m_gameList.SetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
		
		_loadList();
	}
	lang_changed = false;

	return change;
}

void CMenu::_initConfigAdvMenu()
{
	_addUserLabels(m_configAdvLblUser, ARRAY_SIZE(m_configAdvLblUser), "CONFIG_ADV");
	m_configAdvBg = _texture("CONFIG_ADV/BG", "texture", theme.bg, false);
	m_configAdvLblTheme = _addLabel("CONFIG_ADV/THEME", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvLblCurTheme = _addLabel("CONFIG_ADV/THEME_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configAdvBtnCurThemeM = _addPicButton("CONFIG_ADV/THEME_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_configAdvBtnCurThemeP = _addPicButton("CONFIG_ADV/THEME_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_configAdvLblLanguage = _addLabel("CONFIG_ADV/LANGUAGE", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvLblCurLanguage = _addLabel("CONFIG_ADV/LANGUAGE_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configAdvBtnCurLanguageM = _addPicButton("CONFIG_ADV/LANGUAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_configAdvBtnCurLanguageP = _addPicButton("CONFIG_ADV/LANGUAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);
	m_configAdvLblCFTheme = _addLabel("CONFIG_ADV/CUSTOMIZE_CF", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvBtnCFTheme = _addButton("CONFIG_ADV/CUSTOMIZE_CF_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	m_configAdvLblBootChange = _addLabel("CONFIG_ADV/BOOT_CHANGE", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvBtnBootChange = _addButton("CONFIG_ADV/BOOT_CHANGE_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);

	_setHideAnim(m_configAdvLblTheme, "CONFIG_ADV/THEME", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvLblCurTheme, "CONFIG_ADV/THEME_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvBtnCurThemeM, "CONFIG_ADV/THEME_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvBtnCurThemeP, "CONFIG_ADV/THEME_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvLblLanguage, "CONFIG_ADV/LANGUAGE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvLblCurLanguage, "CONFIG_ADV/LANGUAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvBtnCurLanguageM, "CONFIG_ADV/LANGUAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvBtnCurLanguageP, "CONFIG_ADV/LANGUAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvLblCFTheme, "CONFIG_ADV/CUSTOMIZE_CF", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvBtnCFTheme, "CONFIG_ADV/CUSTOMIZE_CF_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configAdvLblBootChange, "CONFIG_ADV/BOOT_CHANGE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvBtnBootChange, "CONFIG_ADV/BOOT_CHANGE_BTN", 0, 0, 1.f, -1.f);
	_hideConfigAdv(true);
	_textConfigAdv();
}

void CMenu::_textConfigAdv(void)
{
	m_btnMgr.setText(m_configAdvLblTheme, _t("cfga7", L"Theme"));
	m_btnMgr.setText(m_configAdvLblLanguage, _t("cfga6", L"Language"));
	m_btnMgr.setText(m_configAdvLblCFTheme, _t("cfgc4", L"Adjust Coverflow"));
	m_btnMgr.setText(m_configAdvBtnCFTheme, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_configAdvLblBootChange, _t("cfgc8", L"Startup Settings"));
	m_btnMgr.setText(m_configAdvBtnBootChange, _t("cfgc5", L"Go"));
}
