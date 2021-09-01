
#include <dirent.h>
#include <algorithm>
#include "menu.hpp"

using namespace std;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

vector<string> languages_available;
void AddLanguage(char *Path)
{
	char lng[32];
	memset(lng, 0, 32);
	char *lang_chr = strrchr(Path, '/')+1;
	memcpy(lng, lang_chr, min(31u, (u32)(strrchr(lang_chr, '.')-lang_chr)));
	languages_available.push_back(lng);
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
	_showConfigCommon(m_configAdvBg, 2);

	m_btnMgr.show(m_configAdvLblCurTheme);
	m_btnMgr.show(m_configAdvBtnCurThemeM);
	m_btnMgr.show(m_configAdvBtnCurThemeP);
	m_btnMgr.show(m_configAdvLblTheme);
	
	m_btnMgr.show(m_configAdvLblBootChange);
	m_btnMgr.show(m_configAdvBtnBootChange);

	m_btnMgr.show(m_configAdvLblLanguage);
	m_btnMgr.show(m_configAdvLblCurLanguage);
	m_btnMgr.show(m_configAdvBtnCurLanguageM);
	m_btnMgr.show(m_configAdvBtnCurLanguageP);

	m_btnMgr.show(m_configAdvLblCFTheme);
	m_btnMgr.show(m_configAdvBtnCFTheme);

	for(u32 i = 0; i < ARRAY_SIZE(m_configAdvLblUser); ++i)
		if(m_configAdvLblUser[i] != -1)
			m_btnMgr.show(m_configAdvLblUser[i]);

	m_btnMgr.setText(m_configAdvLblCurTheme, m_themeName);
	m_btnMgr.setText(m_configAdvLblCurLanguage, m_curLanguage);
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
		themes.push_back("Default");
	sort(themes.begin(), themes.end());
}

int CMenu::_configAdv(void)
{
	int change = CONFIG_PAGE_NO_CHANGE;
	
	vector<string> themes;
	listThemes(m_themeDir.c_str(), themes);
	u32 curTheme = 0;
	for (u32 i = 0; i < themes.size(); ++i)
		if (themes[i] == m_themeName)
		{
			curTheme = i;
			break;
		}
	
	languages_available.clear();
	languages_available.push_back("Default");
	GetFiles(m_languagesDir.c_str(), stringToVector(".ini", '|'), AddLanguage, false, 0);
	sort(languages_available.begin(), languages_available.end());

	u32 curLang = 0;
	for(u32 i = 0; i < languages_available.size(); ++i)
	{
		if(m_curLanguage == languages_available[i])
		{
			curLang = i;
			break;
		}
	}
	string prevLanguage = m_curLanguage;

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
			else if(m_btnMgr.selected(m_configAdvBtnCurThemeP) || m_btnMgr.selected(m_configAdvBtnCurThemeM))
			{
				s8 direction = m_btnMgr.selected(m_configAdvBtnCurThemeP) ? 1 : -1;
				curTheme = loopNum(curTheme + direction, (u32)themes.size());
				m_themeName = themes[curTheme];
				m_cfg.setString("GENERAL", "theme", m_themeName);
				_showConfigAdv();
			}
			else if(m_btnMgr.selected(m_configAdvBtnCurLanguageP) || m_btnMgr.selected(m_configAdvBtnCurLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_configAdvBtnCurLanguageP) ? 1 : -1;
				curLang = loopNum(curLang + direction, (u32)languages_available.size());
				m_curLanguage = languages_available[curLang];
				if(!m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str())))
				{
					m_curLanguage = "Default";
					m_cfg.setString("GENERAL", "language", m_curLanguage);
					m_loc.unload();
				}
				else
					m_cfg.setString("GENERAL", "language", m_curLanguage);
				_updateText();
				_showConfigAdv();
			}
			else if(m_btnMgr.selected(m_configAdvBtnCFTheme))
			{
				m_refreshGameList = true;
				_hideConfigAdv();
				_cfTheme();
				_showConfigAdv();
			}
		}
	}
	_hideConfigAdv();
	if(m_curLanguage != prevLanguage)
	{
		m_cacheList.Init(m_settingsDir.c_str(), m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str(), m_pluginDataDir.c_str());
		fsop_deleteFolder(m_listCacheDir.c_str());// delete cache lists folder and remake it so all lists update.
		fsop_MakeFolder(m_listCacheDir.c_str());
		m_refreshGameList = true;
	}

	return change;
}

void CMenu::_initConfigAdvMenu()
{
	_addUserLabels(m_configAdvLblUser, ARRAY_SIZE(m_configAdvLblUser), "CONFIG_ADV");
	m_configAdvBg = _texture("CONFIG_ADV/BG", "texture", theme.bg, false);
	
	m_configAdvLblTheme = _addLabel("CONFIG_ADV/THEME", theme.lblFont, L"", 20, 125, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvLblCurTheme = _addLabel("CONFIG_ADV/THEME_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configAdvBtnCurThemeM = _addPicButton("CONFIG_ADV/THEME_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_configAdvBtnCurThemeP = _addPicButton("CONFIG_ADV/THEME_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	m_configAdvLblLanguage = _addLabel("CONFIG_ADV/LANGUAGE", theme.lblFont, L"", 20, 185, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvLblCurLanguage = _addLabel("CONFIG_ADV/LANGUAGE_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configAdvBtnCurLanguageM = _addPicButton("CONFIG_ADV/LANGUAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_configAdvBtnCurLanguageP = _addPicButton("CONFIG_ADV/LANGUAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	m_configAdvLblCFTheme = _addLabel("CONFIG_ADV/CUSTOMIZE_CF", theme.lblFont, L"", 20, 245, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvBtnCFTheme = _addButton("CONFIG_ADV/CUSTOMIZE_CF_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	m_configAdvLblBootChange = _addLabel("CONFIG_ADV/BOOT_CHANGE", theme.lblFont, L"", 20, 305, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configAdvBtnBootChange = _addButton("CONFIG_ADV/BOOT_CHANGE_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

	_setHideAnim(m_configAdvLblTheme, "CONFIG_ADV/THEME", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvLblCurTheme, "CONFIG_ADV/THEME_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configAdvBtnCurThemeM, "CONFIG_ADV/THEME_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configAdvBtnCurThemeP, "CONFIG_ADV/THEME_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configAdvLblLanguage, "CONFIG_ADV/LANGUAGE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvLblCurLanguage, "CONFIG_ADV/LANGUAGE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configAdvBtnCurLanguageM, "CONFIG_ADV/LANGUAGE_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configAdvBtnCurLanguageP, "CONFIG_ADV/LANGUAGE_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configAdvLblCFTheme, "CONFIG_ADV/CUSTOMIZE_CF", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvBtnCFTheme, "CONFIG_ADV/CUSTOMIZE_CF_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configAdvLblBootChange, "CONFIG_ADV/BOOT_CHANGE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configAdvBtnBootChange, "CONFIG_ADV/BOOT_CHANGE_BTN", -50, 0, 1.f, 0.f);
	_hideConfigAdv(true);
	_textConfigAdv();
}

void CMenu::_textConfigAdv(void)
{
	m_btnMgr.setText(m_configAdvLblTheme, _t("cfga7", L"Theme"));
	m_btnMgr.setText(m_configAdvLblLanguage, _t("cfgc9", L"WiiFlow Language"));// manage wiiflow languages
	//m_btnMgr.setText(m_configAdvBtnLanguage, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_configAdvLblCFTheme, _t("cfgc4", L"Adjust Coverflow"));
	m_btnMgr.setText(m_configAdvBtnCFTheme, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_configAdvLblBootChange, _t("cfgc8", L"Startup Settings"));
	m_btnMgr.setText(m_configAdvBtnBootChange, _t("cfgc5", L"Go"));
}
