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
#include <algorithm>

TexData m_LangSettingsBg;
s16 m_LangSettingsLblUser[4];

s16 m_LangSettingsLblTitle;

s16 m_LangSettingsLblLanguage;
s16 m_LangSettingsLblCurLanguage;
s16 m_LangSettingsBtnCurLanguageM;
s16 m_LangSettingsBtnCurLanguageP;

s16 m_LangSettingsLblGetLanguages;
s16 m_LangSettingsBtnGetLanguages;

s16 m_LangSettingsLblDlLang;
s16 m_LangSettingsLblCurDLLang;
s16 m_LangSettingsBtnCurDlLangM;
s16 m_LangSettingsBtnCurDlLangP;

s16 m_LangSettingsLblDownload;
s16 m_LangSettingsBtnDownload;

s16 m_LangSettingsBtnBack;

typedef struct {
	char lang[32];
} language_list;

language_list *lang_list_mem = NULL;
u32 mem_pos = 0;
u32 language_cnt = 0;
wstringEx dl_lang_ex;

#define SVN_URL			"http://open-wiiflow-mod.googlecode.com/svn/trunk/wii/wiiflow"
template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

u32 available_pos = 0;
vector<string> languages_available;
void AddLanguage(char *Path)
{
	char lng[32];
	memset(lng, 0, 32);
	char *lang_chr = strrchr(Path, '/')+1;
	memcpy(lng, lang_chr, std::min(31u, (u32)(strrchr(lang_chr, '.')-lang_chr)));
	languages_available.push_back(lng);
}

void CMenu::_hideLangSettings(bool instant)
{
	m_btnMgr.hide(m_LangSettingsLblTitle, instant);

	m_btnMgr.hide(m_LangSettingsLblLanguage, instant);
	m_btnMgr.hide(m_LangSettingsLblCurLanguage, instant);
	m_btnMgr.hide(m_LangSettingsBtnCurLanguageM, instant);
	m_btnMgr.hide(m_LangSettingsBtnCurLanguageP, instant);

	m_btnMgr.hide(m_LangSettingsLblGetLanguages, instant);
	m_btnMgr.hide(m_LangSettingsBtnGetLanguages, instant);

	m_btnMgr.hide(m_LangSettingsLblDlLang, instant);
	m_btnMgr.hide(m_LangSettingsLblCurDLLang, instant);
	m_btnMgr.hide(m_LangSettingsBtnCurDlLangM, instant);
	m_btnMgr.hide(m_LangSettingsBtnCurDlLangP, instant);

	m_btnMgr.hide(m_LangSettingsLblDownload, instant);
	m_btnMgr.hide(m_LangSettingsBtnDownload, instant);

	m_btnMgr.hide(m_LangSettingsBtnBack, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_LangSettingsLblUser); ++i)
		if(m_LangSettingsLblUser[i] != -1)
			m_btnMgr.hide(m_LangSettingsLblUser[i], instant);
}

void CMenu::_showLangSettings(void)
{
	m_btnMgr.show(m_LangSettingsLblTitle);

	m_btnMgr.show(m_LangSettingsLblLanguage);
	m_btnMgr.show(m_LangSettingsLblCurLanguage);
	m_btnMgr.show(m_LangSettingsBtnCurLanguageM);
	m_btnMgr.show(m_LangSettingsBtnCurLanguageP);

	if(lang_list_mem == NULL)
	{
		m_btnMgr.show(m_LangSettingsLblGetLanguages);
		m_btnMgr.show(m_LangSettingsBtnGetLanguages);
	}
	else /* with the list we dont need the get languages button */
	{
		m_btnMgr.hide(m_LangSettingsLblGetLanguages);
		m_btnMgr.hide(m_LangSettingsBtnGetLanguages);

		m_btnMgr.show(m_LangSettingsLblDlLang);
		m_btnMgr.show(m_LangSettingsLblCurDLLang);
		m_btnMgr.show(m_LangSettingsBtnCurDlLangM);
		m_btnMgr.show(m_LangSettingsBtnCurDlLangP);

		m_btnMgr.show(m_LangSettingsLblDownload);
		m_btnMgr.show(m_LangSettingsBtnDownload);

		//gprintf("Currently %s selected (pos %u)\n", lang_list_mem[mem_pos].lang, mem_pos);
		dl_lang_ex.fromUTF8(lang_list_mem[mem_pos].lang);
		m_btnMgr.setText(m_LangSettingsLblCurDLLang, dl_lang_ex);
	}

	m_btnMgr.show(m_LangSettingsBtnBack);

	for(u32 i = 0; i < ARRAY_SIZE(m_LangSettingsLblUser); ++i)
		if(m_LangSettingsLblUser[i] != -1)
			m_btnMgr.show(m_LangSettingsLblUser[i]);

	m_btnMgr.setText(m_LangSettingsLblCurLanguage, m_curLanguage);
}

void CMenu::_refreshLangSettings(void)
{
	languages_available.clear();
	/* Get right Positions first */
	languages_available.push_back("Default");
	GetFiles(m_languagesDir.c_str(), stringToVector(".ini", '|'), AddLanguage, false, 0);
	std::sort(languages_available.begin(), languages_available.end());

	for(u32 i = 0; i < languages_available.size(); ++i)
	{
		if(m_curLanguage == languages_available[i])
		{
			available_pos = i;
			break;
		}
	}
	_showLangSettings();
}

bool CMenu::_LangSettings(void)
{
	_refreshLangSettings();
	bool lang_changed = false;

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_LangSettingsBtnBack))
				break;
			else if(m_btnMgr.selected(m_LangSettingsBtnCurLanguageP) || m_btnMgr.selected(m_LangSettingsBtnCurLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_LangSettingsBtnCurLanguageP) ? 1 : -1;
				available_pos = loopNum(available_pos + direction, languages_available.size());
				m_curLanguage = languages_available[available_pos];
				if(!m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str())))
				{
					m_curLanguage = "Default";
					m_cfg.setString("GENERAL", "language", m_curLanguage.c_str());
					m_loc.unload();
				}
				else
					m_cfg.setString("GENERAL", "language", m_curLanguage.c_str());
				lang_changed = true;
				_updateText();
				_showLangSettings();
			}
			else if(m_btnMgr.selected(m_LangSettingsBtnGetLanguages))
			{
				/* reset our variables doh */
				_hideLangSettings();
				language_cnt = 0;
				mem_pos = 0;
				u8 *file = NULL;
				u32 filesize = 0;
				_downloadUrl(fmt("%s/Languages/", SVN_URL), &file, &filesize);
				if(m_buffer != NULL)
				{
					const char *search_char = "<li><a";
					/* getting count */
					char *start = (strstr((char*)file, search_char)); /* skipping the .. */
					start = strstr(start, "\n") + 1; /* skipping the line */
					char *tmp = start;

					while((tmp = strstr(tmp, search_char)) != NULL)
					{
						language_cnt++;
						tmp = strstr(tmp, "\n") + 1; /* next line */
					}
					gprintf("Found %u languages\n", language_cnt);
					/* creating list */
					tmp = start;
					lang_list_mem = (language_list*)MEM2_alloc(language_cnt*sizeof(language_list));
					memset(lang_list_mem, 0, language_cnt*sizeof(language_list));
					for(u32 i = 0; i < language_cnt; ++i)
					{
						tmp = strstr(tmp, search_char);
						char *lang_chr = strchr(tmp, 0x22) + 1; /* the " is the beginning for the name */
						memcpy(lang_list_mem[i].lang, lang_chr, std::min(31u, (u32)(strchr(lang_chr, '.') - lang_chr)));
						//gprintf("%s added\n", lang_list_mem[i].lang);
						tmp = strstr(tmp, "\n") + 1; /* next line */
					}
					//gprintf("Finished creating the list\n");
					free(m_buffer);
					m_buffer = NULL;
				}
				_showLangSettings();
			}
			else if(m_btnMgr.selected(m_LangSettingsBtnCurDlLangP) || m_btnMgr.selected(m_LangSettingsBtnCurDlLangM))
			{
				s8 direction = m_btnMgr.selected(m_LangSettingsBtnCurDlLangP) ? 1 : -1;
				mem_pos = loopNum(mem_pos + direction, language_cnt);
				_showLangSettings();
			}
			else if(m_btnMgr.selected(m_LangSettingsBtnDownload))
			{
				_hideLangSettings();
				m_loc.unload();
				/* Get main ini */
				u8 *file = NULL;
				u32 filesize = 0;
				_downloadUrl(fmt("%s/Languages/%s.ini", SVN_URL, lang_list_mem[mem_pos].lang), &file, &filesize);
				if(m_buffer != NULL)
				{
					if(filesize > 0)
					{
						const char *language_ini = fmt("%s/%s.ini", m_languagesDir.c_str(), lang_list_mem[mem_pos].lang);
						fsop_deleteFile(language_ini);
						fsop_WriteFile(language_ini, file, filesize);
						gprintf("Wrote %s with the size %u\n", language_ini, filesize);
					}
					free(m_buffer);
					m_buffer = NULL;
				}
				/* Get help file */
				file = NULL;
				filesize = 0;
				_downloadUrl(fmt("%s/help/%s.txt", SVN_URL, lang_list_mem[mem_pos].lang), &file, &filesize);
				if(m_buffer != NULL)
				{
					if(filesize > 0)
					{
						const char *language_help = fmt("%s/%s.txt", m_helpDir.c_str(), lang_list_mem[mem_pos].lang);
						fsop_deleteFile(language_help);
						fsop_WriteFile(language_help, file, filesize);
						gprintf("Wrote %s with the size %u\n", language_help, filesize);
					}
					free(m_buffer);
					m_buffer = NULL;
				}
				/* reload */
				m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str()));
				_updateText();
				_refreshLangSettings();
			}
		}
	}
	_hideLangSettings();
	if(lang_changed)
		_cfNeedsUpdate();

	if(lang_list_mem != NULL)
		free(lang_list_mem);
	lang_list_mem = NULL;
	return lang_changed;
}

void CMenu::_initLangSettingsMenu()
{
	_addUserLabels(m_LangSettingsLblUser, ARRAY_SIZE(m_LangSettingsLblUser), "LANGUAGE");

	m_LangSettingsLblTitle = _addTitle("LANGUAGE/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	m_LangSettingsBg = _texture("LANGUAGE/BG", "texture", theme.bg, false);
	m_LangSettingsLblLanguage = _addLabel("LANGUAGE/LANGUAGE", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_LangSettingsLblCurLanguage = _addLabel("LANGUAGE/LANGUAGE_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_LangSettingsBtnCurLanguageM = _addPicButton("LANGUAGE/LANGUAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_LangSettingsBtnCurLanguageP = _addPicButton("LANGUAGE/LANGUAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);

	m_LangSettingsLblGetLanguages = _addLabel("LANGUAGE/GET_LANG", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_LangSettingsBtnGetLanguages = _addButton("LANGUAGE/GET_LANG_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_LangSettingsLblDlLang = _addLabel("LANGUAGE/DL_LANG", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_LangSettingsLblCurDLLang = _addLabel("LANGUAGE/DL_LANG_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_LangSettingsBtnCurDlLangM = _addPicButton("LANGUAGE/DL_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_LangSettingsBtnCurDlLangP = _addPicButton("LANGUAGE/DL_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);

	m_LangSettingsLblDownload = _addLabel("LANGUAGE/DOWNLOAD", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_LangSettingsBtnDownload = _addButton("LANGUAGE/DOWNLOAD_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

	m_LangSettingsBtnBack = _addButton("LANGUAGE/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);


	_setHideAnim(m_LangSettingsLblTitle, "LANGUAGE/TITLE", 0, 0, -2.f, 0.f);

	_setHideAnim(m_LangSettingsLblLanguage, "LANGUAGE/LANGUAGE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_LangSettingsLblCurLanguage, "LANGUAGE/LANGUAGE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_LangSettingsBtnCurLanguageM, "LANGUAGE/LANGUAGE_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_LangSettingsBtnCurLanguageP, "LANGUAGE/LANGUAGE_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_LangSettingsLblGetLanguages, "LANGUAGE/GET_LANG", 50, 0, -2.f, 0.f);
	_setHideAnim(m_LangSettingsBtnGetLanguages, "LANGUAGE/GET_LANG_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_LangSettingsLblDlLang, "LANGUAGE/DL_LANG", 50, 0, -2.f, 0.f);
	_setHideAnim(m_LangSettingsLblCurDLLang, "LANGUAGE/DL_LANG_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_LangSettingsBtnCurDlLangM, "LANGUAGE/DL_LANG_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_LangSettingsBtnCurDlLangP, "LANGUAGE/DL_LANG_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_LangSettingsLblDownload, "LANGUAGE/DOWNLOAD", 50, 0, -2.f, 0.f);
	_setHideAnim(m_LangSettingsBtnDownload, "LANGUAGE/DOWNLOAD_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_LangSettingsBtnBack, "LANGUAGE/BACK_BTN", 0, 0, 1.f, -1.f);

	_hideLangSettings(true);
	_textLangSettings();
}

void CMenu::_textLangSettings(void)
{
	m_btnMgr.setText(m_LangSettingsLblLanguage, _t("cfga6", L"Language"));

	m_btnMgr.setText(m_LangSettingsLblTitle, _t("cfglng1", L"Manage Languages"));

	m_btnMgr.setText(m_LangSettingsLblGetLanguages, _t("cfglng2", L"Get Languages"));
	m_btnMgr.setText(m_LangSettingsBtnGetLanguages, _t("cfgc5", L"Go"));

	m_btnMgr.setText(m_LangSettingsLblDlLang, _t("cfglng3", L"Select File"));

	m_btnMgr.setText(m_LangSettingsLblDownload, _t("cfglng4", L"Download selected File"));
	m_btnMgr.setText(m_LangSettingsBtnDownload, _t("cfg4", L"Download"));

	m_btnMgr.setText(m_LangSettingsBtnBack, _t("cfg10", L"Back"));
}
