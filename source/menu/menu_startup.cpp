#include "menu.hpp"

#include <string.h>
#include <gccore.h>

#include "defines.h"

int Source_curPage;
int pages;
u8 numPlugins;
string m_sourceDir;
Config m_source;

// Source menu
u32 m_sourceLblNotice;
u32 m_sourceLblPage;
u32 m_sourceBtnPageM;
u32 m_sourceBtnPageP;
u32 m_sourceBtnBack;
u32 m_sourceLblTitle;
u32 m_sourceBtnSource[36];
u32 m_sourceLblUser[4];
STexture m_sourceBg;

void CMenu::_hideSource(bool instant)
{
	m_btnMgr.hide(m_sourceLblTitle, instant);
	m_btnMgr.hide(m_sourceBtnBack, instant);
	m_btnMgr.hide(m_sourceLblNotice, instant);
	m_btnMgr.hide(m_sourceLblPage, instant);
	m_btnMgr.hide(m_sourceBtnPageM, instant);
	m_btnMgr.hide(m_sourceBtnPageP, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_sourceLblUser); ++i)
	{
		if (m_sourceLblUser[i] != -1u)
			m_btnMgr.hide(m_sourceLblUser[i], instant);
	}

	for (int i = 0; i < 36; ++i)
	{
		m_btnMgr.hide(m_sourceBtnSource[i]);
	}
}

void CMenu::_showSource(void)
{
	_setBg(m_sourceBg, m_sourceBg);
	
	for (u32 i = 0; i < ARRAY_SIZE(m_sourceLblUser); ++i)
	{
		if (m_sourceLblUser[i] != -1u)
			m_btnMgr.show(m_sourceLblUser[i]);
	}

	m_btnMgr.show(m_sourceLblTitle);
	m_btnMgr.show(m_sourceBtnBack);
	
	for (int i = 12; i < 36; ++i)
	{
		string source = m_source.getString(fmt("BUTTON_%i", i), "source", "");
		if (!source.empty())
		{
			pages = (i / 12) + 1;
		}
	}
	
}

void CMenu::_updateSourceBtns(void)
{
	if (pages > 1)
	{
		m_btnMgr.setText(m_sourceLblPage, wfmt(L"%i / %i", Source_curPage, pages));
		m_btnMgr.show(m_sourceLblPage);
		m_btnMgr.show(m_sourceBtnPageM);
		m_btnMgr.show(m_sourceBtnPageP);
	}
	for (int i = 0; i < 36; ++i)
		m_btnMgr.hide(m_sourceBtnSource[i]);		

	int j = (Source_curPage - 1) * 12;
	
	for (int i = j; i < (j + 12); ++i)
	{	
		string source = m_source.getString(fmt("BUTTON_%i", i), "source", "");
		if (!source.empty())
			m_btnMgr.show(m_sourceBtnSource[i]);
	}	
}

void CMenu::_showSourceNotice(void)
{
	m_showtimer = 90;
	m_btnMgr.show(m_sourceLblNotice);
}

bool CMenu::_Source()
{
	DIR *pdir;
	struct dirent *pent;
	if(!m_source.loaded())
		m_source.load(fmt("%s/%s", m_sourceDir.c_str(), SOURCE_FILENAME));

	pdir = opendir(m_pluginsDir.c_str());
	Config m_plugin_cfg;
	u8 i = 0;
	bool back = false;

	while((pent = readdir(pdir)) != NULL)
	{
		if(strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0 
		|| strcasecmp(pent->d_name, "plugins.ini") == 0 || strcasecmp(pent->d_name, "scummvm.ini") == 0)
			continue;
		if(strcasestr(pent->d_name, ".ini") != NULL)
		{
			m_plugin_cfg.load(fmt("%s/%s", m_pluginsDir.c_str(), pent->d_name));
			if (m_plugin_cfg.loaded())
			{
				i++;
				m_plugin.AddPlugin(m_plugin_cfg);
			}
			m_plugin_cfg.unload();
		}
	}
	closedir(pdir);
	m_plugin.EndAdd();
	numPlugins = i;

	SetupInput();
	bool show_homebrew = !m_cfg.getBool("HOMEBREW", "disable", false);
	bool show_channel = !m_cfg.getBool("GENERAL", "hidechannel", false);
	bool show_emu = !m_cfg.getBool("EMULATOR", "disable", false);
	bool parental_homebrew = m_cfg.getBool("HOMEBREW", "parental", false);	
	bool imgSelected = false;
	m_showtimer = 0;
	Source_curPage = 1;
	pages = 1;
	_showSource();
	_updateSourceBtns();
	
	while(true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			back = true;
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) && pages > 1) || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageM)))
		{
			Source_curPage = Source_curPage == 1 ? pages : (Source_curPage == 2 ? 1 : 2);
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_sourceBtnPageM);
			_updateSourceBtns();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED) && pages > 1) || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageP)))
		{
			Source_curPage = Source_curPage == 3 ? 1 : (Source_curPage == 1 ? 2 : (pages == 2 ? 1 : 3));
			if (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_sourceBtnPageP);
			_updateSourceBtns();
		}
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_sourceBtnBack))
			{
				back = true;
				break;
			}
			for(int i = 0; i < 36; ++i)
			{
				if(m_btnMgr.selected(m_sourceBtnSource[i]))
				{
					string source = m_source.getString(fmt("BUTTON_%i", i), "source", "");
					if (source == "wii")
					{
						m_current_view = COVERFLOW_USB;
						imgSelected = true;
						break;
					}
					if (source == "dml")
					{
						if (!m_show_dml) _showSourceNotice();
						else
						{
							m_current_view = COVERFLOW_DML;
							imgSelected = true;
							break;
						}
					}
					if (source == "emunand")
					{
						if (!show_channel) _showSourceNotice();
						else
						{
							m_current_view = COVERFLOW_CHANNEL;
							m_cfg.setBool("NAND", "disable", false);
							imgSelected = true;
							break;
						}
					}
					if (source == "realnand")
					{
						if (!show_channel) _showSourceNotice();
						else
						{
							m_current_view = COVERFLOW_CHANNEL;
							m_cfg.setBool("NAND", "disable", true);
							imgSelected = true;
							break;
						}
					}
					if (source == "homebrew")
					{
						if (!show_homebrew || (!parental_homebrew && m_locked)) _showSourceNotice(); 
						else
						{
							m_current_view = COVERFLOW_HOMEBREW;
							imgSelected = true;
							break;
						}
					}
					if (source == "allplugins")
					{
						if (!show_emu) _showSourceNotice();
						else
						{
							m_current_view = COVERFLOW_EMU;
							imgSelected = true;
							
							for (u8 j = 0; j < numPlugins; ++j)
							{
								m_plugin.SetEnablePlugin(m_cfg, j, 2);
							}
							break;
						}
					}
					if (source == "plugin")
					{
						if (!show_emu) _showSourceNotice();
						else
						{
							m_current_view = COVERFLOW_EMU;
							imgSelected = true;
							
							u32 sourceMagic;
							sscanf(m_source.getString(fmt("BUTTON_%i", i), "magic","").c_str(), "%08x", &sourceMagic);
							
							for (u8 j = 0; j < numPlugins; ++j)
							{
								if (sourceMagic == m_plugin.getPluginMagic(j))
								{
									m_plugin.SetEnablePlugin(m_cfg, j, 2);
								}
								else
								{
									m_plugin.SetEnablePlugin(m_cfg, j, 1);
								}
							}
							
							int layout = m_source.getInt(fmt("BUTTON_%i", i), "emuflow", 0);
							if(layout != 0)
								m_cfg.setInt("EMULATOR", "last_cf_mode", layout);
							break;
						}
					}
				}
			}
			if(imgSelected)
				break;
		}
		if(m_showtimer > 0)
		{
			if(--m_showtimer == 0)
				m_btnMgr.hide(m_sourceLblNotice);
		}
	}
	_hideSource(true);
	return back;
}

void CMenu::_initSourceMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_sourceLblUser, ARRAY_SIZE(m_sourceLblUser), "SOURCE");
	m_sourceBg = _texture(theme.texSet, "SOURCE/BG", "texture", theme.bg);
	m_sourceLblTitle = _addTitle(theme, "SOURCE/TITLE", theme.titleFont, L"", 20, 20, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_sourceBtnBack = _addButton(theme, "SOURCE/BACK_BTN", theme.btnFont, L"", 424, 400, 210, 56, theme.btnFontColor);
	m_sourceLblNotice = _addLabel(theme, "SOURCE/NOTICE", theme.btnFont, L"", 20, 400, 600, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_sourceLblPage = _addLabel(theme, "SOURCE/PAGE_BTN", theme.btnFont, L"", 62, 400, 98, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_sourceBtnPageM = _addPicButton(theme, "SOURCE/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 10, 400, 52, 56);
	m_sourceBtnPageP = _addPicButton(theme, "SOURCE/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 160, 400, 52, 56);

	m_sourceDir = m_cfg.getString("GENERAL", "dir_Source", sfmt("%s/source_menu", m_dataDir.c_str()));

	if(!m_source.loaded())
		m_source.load(fmt("%s/%s", m_sourceDir.c_str(), SOURCE_FILENAME));
	int page;
	int row;
	int col;
	string ImgName;
	
	for ( int i = 0; i < 36; ++i)
	{
		STexture texConsoleImg;
		STexture texConsoleImgs;
	
		ImgName = m_source.getString(fmt("BUTTON_%i", i),"image", "default.png");
		texConsoleImg.fromPNGFile(fmt("%s/%s", m_sourceDir.c_str(), ImgName.c_str()), GX_TF_RGBA8, ALLOC_MEM2);
		ImgName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "default.png");
		texConsoleImgs.fromPNGFile(fmt("%s/%s", m_sourceDir.c_str(), ImgName.c_str()), GX_TF_RGBA8, ALLOC_MEM2);
	
		page = i / 12;
		row = (i / 4 ) - (page * 3);
		col = (i - (page * 12)) - (row * 4);
		m_sourceBtnSource[i] = _addPicButton(theme, fmt("SOURCE/SOURCE_BTN_%i", i), texConsoleImg, texConsoleImgs, (30 + 150 * col), (90 + 100 * row), 120, 90);
	}
	
	_setHideAnim(m_sourceLblTitle, "SOURCE/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_sourceLblNotice, "SOURCE/NOTICE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_sourceLblPage, "SOURCE/PAGE_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_sourceBtnPageM, "SOURCE/PAGE_MINUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_sourceBtnPageP, "SOURCE/PAGE_PLUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_sourceBtnBack, "SOURCE/BACK_BTN", 0, 200, 1.f, 0.f);

	for(int i = 0; i < 36; ++i)
	{
		_setHideAnim(m_sourceBtnSource[i], fmt("SOURCE/SOURCE_BTN_%i", i), 0, 0, 1.f, 0.f);
	}
	_textSource();
	_hideSource(true);
}

void CMenu::_textSource(void)
{
	m_btnMgr.setText(m_sourceLblTitle, _t("stup1", L"Select Source"));
	m_btnMgr.setText(m_sourceBtnBack, _t("stup2", L"Exit"));
	m_btnMgr.setText(m_sourceLblNotice, _t("NMMOff", L"** DISABLED **"));
}
