#include <dirent.h>
#include <unistd.h>

#include "menu.hpp"
#include "defines.h"

extern const u8 btnchannel_png[];
extern const u8 btnchannels_png[];
extern const u8 btnusb_png[];
extern const u8 btnusbs_png[];
extern const u8 btndml_png[];
extern const u8 btndmls_png[];
extern const u8 btnemu_png[];
extern const u8 btnemus_png[];
extern const u8 btnhomebrew_png[];
extern const u8 btnhomebrews_png[];
extern const u8 favoriteson_png[];
extern const u8 favoritesons_png[];

// Source menu
s16 m_sourceLblNotice;
s16 m_sourceLblPage;
s16 m_sourceBtnPageM;
s16 m_sourceBtnPageP;
s16 m_sourceBtnBack;
s16 m_sourceLblTitle;
s16 m_sourceBtnSource[12];
s16 m_sourceLblUser[4];
s16 m_sourceBtnDML;
s16 m_sourceBtnEmu;
s16 m_sourceBtnUsb;
s16 m_sourceBtnChannel;
s16 m_sourceBtnHomebrew;

TexData m_sourceBg;

u8 sourceBtn;
u8 selectedBtns;
int source_curPage;
int source_Pages;
string m_sourceDir;
string themeName;
vector<string> magicNums;

void CMenu::_hideSource(bool instant)
{
	m_btnMgr.hide(m_sourceLblTitle, instant);
	m_btnMgr.hide(m_sourceLblNotice, instant);
	m_btnMgr.hide(m_sourceLblPage, instant);
	m_btnMgr.hide(m_sourceBtnPageM, instant);
	m_btnMgr.hide(m_sourceBtnPageP, instant);
	m_btnMgr.hide(m_sourceBtnBack, instant);	
	m_btnMgr.hide(m_sourceBtnHomebrew, instant);
	m_btnMgr.hide(m_sourceBtnChannel, instant);
	m_btnMgr.hide(m_sourceBtnUsb, instant);
	m_btnMgr.hide(m_sourceBtnDML, instant);
	m_btnMgr.hide(m_sourceBtnEmu, instant);

	u8 i;
	for(i = 0; i < ARRAY_SIZE(m_sourceLblUser); ++i)
	{
		if(m_sourceLblUser[i] != -1)
			m_btnMgr.hide(m_sourceLblUser[i], instant);
	}

	for(i = 0; i < 12; ++i)
	{
		m_btnMgr.hide(m_sourceBtnSource[i], instant);
		m_btnMgr.freeBtnTexture(m_sourceBtnSource[i]);
	}
}

void CMenu::_showSource(void)
{
	_setBg(m_sourceBg, m_sourceBg);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_sourceLblUser); ++i)
	{
		if(m_sourceLblUser[i] != -1)
			m_btnMgr.show(m_sourceLblUser[i]);
	}

	m_btnMgr.show(m_sourceLblTitle);
	m_btnMgr.show(m_sourceBtnBack);
}

void CMenu::_updateSourceBtns(void)
{
	if (source_Pages > 1)
	{
		m_btnMgr.setText(m_sourceLblPage, wfmt(L"%i / %i", source_curPage, source_Pages));
		m_btnMgr.show(m_sourceLblPage);
		m_btnMgr.show(m_sourceBtnPageM);
		m_btnMgr.show(m_sourceBtnPageP);
	}

	const char *ImgName = NULL;
	const char *ImgSelName = NULL;
	u8 j = (source_curPage - 1) * 12;
	sourceBtn = 0;
	selectedBtns = 0;
	for(u8 i = 0; i < 12; ++i)
	{
		m_btnMgr.hide(m_sourceBtnSource[i], true);
		string btnSource = m_source.getString(fmt("BUTTON_%i", (i + j)), "source", "");
		
		if(btnSource == "")
			continue;
		else if(btnSource == "allplugins")
		{
			const vector<bool> &EnabledPlugins = m_plugin.GetEnabledPlugins(m_cfg);
			if(EnabledPlugins.size() == 0)
			{
				sourceBtn = i + j;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();
			}
			else
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image", "").c_str();
		}
		else if(btnSource == "plugin")
		{
			magicNums.clear();
			magicNums = m_source.getStrings(fmt("BUTTON_%i", i + j), "magic", ',');
			if(m_cfg.getBool(PLUGIN_DOMAIN, "source", false) && m_cfg.getBool("PLUGIN", magicNums.at(0), false))
			{
				sourceBtn = i + j;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();
			}
			else
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image", "").c_str();
		}
		else if(btnSource == "realnand" || btnSource == "emunand")
		{
			ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image", "").c_str();
			if(m_cfg.getBool(CHANNEL_DOMAIN, "source", false) && m_cfg.getBool(CHANNEL_DOMAIN, "disable") && btnSource == "realnand")
			{
				sourceBtn = i + j;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();
			}
			else if(m_cfg.getBool(CHANNEL_DOMAIN, "source", false) && !m_cfg.getBool(CHANNEL_DOMAIN, "disable") && btnSource == "emunand")
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();
		}
		else if(btnSource != "realnand" && btnSource != "emunand" && btnSource != "plugin" && btnSource != "allplugins")
		{
			string domain = (btnSource == "dml" ? GC_DOMAIN : (btnSource == "homebrew" ? HOMEBREW_DOMAIN : WII_DOMAIN));
			if(m_cfg.getBool(domain, "source", false))
			{
				sourceBtn = i + j;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();
			}
			else
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image", "").c_str();
		}
		
		ImgSelName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();

		TexData texConsoleImg;
		TexData texConsoleImgs;
		if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), ImgName)) != TE_OK)
		{
			if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_sourceDir.c_str(), ImgName)) != TE_OK)
				TexHandle.fromPNG(texConsoleImg, favoriteson_png);
		}
		if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), ImgSelName)) != TE_OK)
		{
			if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_sourceDir.c_str(), ImgSelName)) != TE_OK)
				TexHandle.fromPNG(texConsoleImgs, favoritesons_png);
		}
		m_btnMgr.setBtnTexture(m_sourceBtnSource[i], texConsoleImg, texConsoleImgs);
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
	u8 i, j, k;
	bool show_homebrew = !m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false);
	bool show_channel = !m_cfg.getBool("GENERAL", "hidechannel", false);
	bool show_emu = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
	bool parental_homebrew = m_cfg.getBool(HOMEBREW_DOMAIN, "parental", false);	
	bool noChanges = true;
	bool updateSource = false;
	bool exitSource = false;
	u8 numPlugins = 0;
	m_showtimer = 0;
	source_curPage = 1;
	
	SetupInput();
	_showSource();
	bool sourceIniLoaded = m_source.loaded();
	if(!sourceIniLoaded)
	{
		m_btnMgr.show(m_sourceBtnHomebrew);
		m_btnMgr.show(m_sourceBtnChannel);
		m_btnMgr.show(m_sourceBtnUsb);
		m_btnMgr.show(m_sourceBtnDML);
		m_btnMgr.show(m_sourceBtnEmu);
	}
	else
	{
		//set number of pages based on highest source btn number used
		for(i = m_cfg.getInt("GENERAL", "max_source_buttons", 71); i > 11; --i)
		{
			string source = m_source.getString(fmt("BUTTON_%i", i), "source", "");
			if (!source.empty())
			{
				source_Pages = (i / 12) + 1;
				break;
			}
		}
		if(show_emu)
		{
			Config m_plugin_cfg;
			DIR *pdir;
			struct dirent *pent;
			pdir = opendir(m_pluginsDir.c_str());
			while((pent = readdir(pdir)) != NULL)
			{
				if(pent->d_name[0] == '.'|| strcasecmp(pent->d_name, "scummvm.ini") == 0)
					continue;
				if(strcasestr(pent->d_name, ".ini") != NULL)
				{
					m_plugin_cfg.load(fmt("%s/%s", m_pluginsDir.c_str(), pent->d_name));
					if (m_plugin_cfg.loaded())
					{
						numPlugins++;
						m_plugin.AddPlugin(m_plugin_cfg);
					}
					m_plugin_cfg.unload();
				}
			}
			closedir(pdir);
			m_plugin.EndAdd();
		}
		_updateSourceBtns();
	}

	while(!m_exit)
	{
		updateSource = false;
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnBack)) || (BTN_B_PRESSED))
		{
			if(selectedBtns == 1)
			{
				m_catStartPage = m_source.getInt(fmt("BUTTON_%i", sourceBtn), "cat_page", 1);
				if(m_source.getString(fmt("BUTTON_%i", sourceBtn), "source") == "plugin")
				{
					int layout = m_source.getInt(fmt("BUTTON_%i", sourceBtn), "emuflow", 0);
					if(layout > 0)
						m_cfg.setInt(PLUGIN_DOMAIN, "last_cf_mode", layout);
					int category = m_source.getInt(fmt("BUTTON_%i", sourceBtn), "category", 0);
					if(category > 0)
					{
						m_cat.remove("GENERAL", "selected_categories");
						m_cat.remove("GENERAL", "required_categories");
						char cCh = static_cast<char>(category + 32);
						string newSelCats(1, cCh);
						m_cat.setString("GENERAL", "selected_categories", newSelCats);
						m_clearCats = false;
					}
				}
			}
			if(selectedBtns == 0)
				m_cfg.setBool(WII_DOMAIN, "source", true);
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if((BTN_LEFT_PRESSED && source_Pages > 1) || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageM)))
		{
			source_curPage--;
			if(source_curPage < 1)
				source_curPage = source_Pages;
			if(BTN_LEFT_PRESSED)
				m_btnMgr.click(m_sourceBtnPageM);
			_updateSourceBtns();
		}
		else if((BTN_RIGHT_PRESSED && source_Pages > 1) || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageP)))
		{
			source_curPage++;
			if(source_curPage > source_Pages)
				source_curPage = 1;
			if (BTN_RIGHT_PRESSED)
				m_btnMgr.click(m_sourceBtnPageP);
			_updateSourceBtns();
		}
		else if(BTN_A_PRESSED && !sourceIniLoaded)
		{
			// check default source buttons when no source_menu.ini
			if(m_btnMgr.selected(m_sourceBtnUsb))
			{
				_clearSources();
				m_cfg.setBool(WII_DOMAIN, "source", true);
				exitSource = true;
			}
			if(m_btnMgr.selected(m_sourceBtnDML))
			{
				if(!m_show_dml && !m_devo_installed) _showSourceNotice();
				else
				{
					_clearSources();
					m_cfg.setBool(GC_DOMAIN, "source", true);
					exitSource = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnChannel))
			{
				if(!show_channel) _showSourceNotice();
				else
				{
					_clearSources();
					m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
					exitSource = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnHomebrew))
			{
				if(!show_homebrew || (!parental_homebrew && m_locked)) _showSourceNotice(); 
				else
				{
					_clearSources();
					m_cfg.setBool(HOMEBREW_DOMAIN, "source", true);
					exitSource = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnEmu))
			{
				if(!show_emu) _showSourceNotice();
				else
				{
					_clearSources();
					m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
					exitSource = true;
				}
			}
		}
		else if((BTN_A_PRESSED || BTN_PLUS_PRESSED) && sourceIniLoaded)
		{
			// check actual source menu buttons
			j = (source_curPage - 1) * 12;
			for(i = 0; i < 12; ++i)
			{
				if(m_btnMgr.selected(m_sourceBtnSource[i]))
				{
					string source = m_source.getString(fmt("BUTTON_%i", i + j), "source", "");
					if(source == "wii" && BTN_A_PRESSED)
					{
						_clearSources();
						m_cfg.setBool(WII_DOMAIN, "source", true);
						m_catStartPage = m_source.getInt(fmt("BUTTON_%i", i + j), "cat_page", 1);
						exitSource = true;
						break;
					}
					else if(source == "wii" && BTN_PLUS_PRESSED)
					{
						updateSource = true;
						m_cfg.setBool(WII_DOMAIN, "source", !m_cfg.getBool(WII_DOMAIN, "source"));
						break;
					}
					if(source == "dml")
					{
						if(!m_show_dml && !m_devo_installed) _showSourceNotice();
						else if(BTN_A_PRESSED)
						{
							_clearSources();
							m_cfg.setBool(GC_DOMAIN, "source", true);
							m_catStartPage = m_source.getInt(fmt("BUTTON_%i", i + j), "cat_page", 1);
							exitSource = true;
						}
						else if(BTN_PLUS_PRESSED)
						{
							updateSource = true;
							m_cfg.setBool(GC_DOMAIN, "source", !m_cfg.getBool(GC_DOMAIN, "source"));
						}
						break;
					}
					if(source == "emunand")
					{
						if(!show_channel) _showSourceNotice();
						else
						{
							m_cfg.setBool(CHANNEL_DOMAIN, "disable", false);
							if(BTN_A_PRESSED)
							{
								_clearSources();
								m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
								m_catStartPage = m_source.getInt(fmt("BUTTON_%i", i + j), "cat_page", 1);
								exitSource = true;
							}
							else
							{
								updateSource = true;
								m_cfg.setBool(CHANNEL_DOMAIN, "source", !m_cfg.getBool(CHANNEL_DOMAIN, "source"));
							}
						}
						break;
					}
					if(source == "realnand")
					{
						if(!show_channel) _showSourceNotice();
						else
						{
							m_cfg.setBool(CHANNEL_DOMAIN, "disable", true);
							if(BTN_A_PRESSED)
							{
								_clearSources();
								m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
								m_catStartPage = m_source.getInt(fmt("BUTTON_%i", i + j), "cat_page", 1);
								exitSource = true;
							}
							else
							{
								updateSource = true;
								m_cfg.setBool(CHANNEL_DOMAIN, "source", !m_cfg.getBool(CHANNEL_DOMAIN, "source"));
							}
						}
						break;
					}
					if(source == "homebrew")
					{
						if(!show_homebrew || (!parental_homebrew && m_locked)) _showSourceNotice(); 
						else if(BTN_A_PRESSED)
						{
							_clearSources();
							m_cfg.setBool(HOMEBREW_DOMAIN, "source", true);
							m_catStartPage = m_source.getInt(fmt("BUTTON_%i", i + j), "cat_page", 1);
							exitSource = true;
						}
						else if(BTN_PLUS_PRESSED)
						{
							updateSource = true;
							m_cfg.setBool(HOMEBREW_DOMAIN, "source", !m_cfg.getBool(HOMEBREW_DOMAIN, "source"));
						}
						break;
					}
					if(source == "allplugins")
					{
						if(!show_emu) _showSourceNotice();
						else if(BTN_A_PRESSED)
						{
							_clearSources();
							m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
							m_catStartPage = m_source.getInt(fmt("BUTTON_%i", i + j), "cat_page", 1);
							exitSource = true;
							for (j = 0; j < numPlugins; ++j)
								m_plugin.SetEnablePlugin(m_cfg, j, 2);
						}
						else if(BTN_PLUS_PRESSED)
						{
							updateSource = true;
							bool EnableAll = m_plugin.GetEnabledPlugins(m_cfg).size();
							for(j = 0; m_plugin.PluginExist(j); j++)
								m_plugin.SetEnablePlugin(m_cfg, j, EnableAll ? 2 : 1);
							if(EnableAll)
								m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
							else
								m_cfg.setBool(PLUGIN_DOMAIN, "source", false);
						}
						break;
					}
					if (source == "plugin")
					{
						if(!show_emu) _showSourceNotice();
						else
						{
							magicNums.clear();
							magicNums = m_source.getStrings(fmt("BUTTON_%i", i + j), "magic", ',');
							if (magicNums.size() > 0)
							{							
								if(BTN_A_PRESSED)
								{
									_clearSources();
									m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
									exitSource = true;						
									for (k = 0; k < numPlugins; ++k)
										m_plugin.SetEnablePlugin(m_cfg, k, 1);
									for (vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
									{
										string magic = *itr;
										u32 sourceMagic = strtoul(magic.c_str(), NULL, 16);
										for (k = 0; k < numPlugins; ++k)
										{
											if (sourceMagic == m_plugin.getPluginMagic(k))
												m_plugin.SetEnablePlugin(m_cfg, k, 2);
										}
									}
									m_catStartPage = m_source.getInt(fmt("BUTTON_%i", i + j), "cat_page", 1);
									int layout = m_source.getInt(fmt("BUTTON_%i", i + j), "emuflow", 0);
									if(layout > 0)
										m_cfg.setInt(PLUGIN_DOMAIN, "last_cf_mode", layout);
									int category = m_source.getInt(fmt("BUTTON_%i", i + j), "category", 0);
									if(category > 0)
									{
										m_cat.remove("GENERAL", "selected_categories");
										m_cat.remove("GENERAL", "required_categories");
										char cCh = static_cast<char>(category + 32);
										string newSelCats(1, cCh);
										m_cat.setString("GENERAL", "selected_categories", newSelCats);
										m_clearCats = false;
									}
								}
								else
								{
									updateSource = true;
									// turn off all plugins unless plugin source is already on
									if(!m_cfg.getBool(PLUGIN_DOMAIN, "source", false))
									{
										for (k = 0; k < numPlugins; ++k)
											m_plugin.SetEnablePlugin(m_cfg, k, 1);
									}
									for (vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
									{
										string magic = *itr;
										m_cfg.setBool("PLUGIN", magic, !m_cfg.getBool("PLUGIN", magic, false));
										if(m_cfg.getBool("PLUGIN", magic))
											m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
									}
								}
								_checkForSinglePlugin();
								if(enabledPluginsCount == 0 && BTN_PLUS_PRESSED) // if last plugin turn domain off
									m_cfg.setBool(PLUGIN_DOMAIN, "source", false);
								if(enabledPluginsCount == 1)
								{
									currentPartition = m_cfg.getInt("PLUGINS/PARTITION", PluginMagicWord, 1);
									m_cfg.setInt(PLUGIN_DOMAIN, "partition", currentPartition);
								}
							}
						}
						break;
					}
				}
			}
		}
		if(exitSource)
		{
			noChanges = false;
			break;
		}
		if(updateSource)
		{
			noChanges = false;
			_updateSourceBtns();
		}
		if(m_showtimer > 0)
		{
			if(--m_showtimer == 0)
				m_btnMgr.hide(m_sourceLblNotice);
		}
	}
	_hideSource(true);
	return noChanges;
}

void CMenu::_clearSources(void)
{
	m_cfg.setBool(WII_DOMAIN, "source", false);
	m_cfg.setBool(GC_DOMAIN, "source", false);
	m_cfg.setBool(CHANNEL_DOMAIN, "source", false);
	m_cfg.setBool(HOMEBREW_DOMAIN, "source", false);
	m_cfg.setBool(PLUGIN_DOMAIN, "source", false);
}

void CMenu::_initSourceMenu()
{
	TexData texDML;
	TexData texDMLs;
	TexData texEmu;
	TexData texEmus;
	TexData texUsb;
	TexData texUsbs;
	TexData texChannel;
	TexData texChannels;
	TexData texHomebrew;
	TexData texHomebrews;

	TexHandle.fromPNG(texUsb, btnusb_png);
	TexHandle.fromPNG(texUsbs, btnusbs_png);
	TexHandle.fromPNG(texDML, btndml_png);
	TexHandle.fromPNG(texDMLs, btndmls_png);
	TexHandle.fromPNG(texEmu, btnemu_png);
	TexHandle.fromPNG(texEmus, btnemus_png);
	TexHandle.fromPNG(texChannel, btnchannel_png);
	TexHandle.fromPNG(texChannels, btnchannels_png);
	TexHandle.fromPNG(texHomebrew, btnhomebrew_png);
	TexHandle.fromPNG(texHomebrews, btnhomebrews_png);

	m_sourceBtnChannel = _addPicButton("SOURCE/CHANNEL_BTN", texChannel, texChannels, 265, 260, 48, 48);
	m_sourceBtnHomebrew = _addPicButton("SOURCE/HOMEBREW_BTN", texHomebrew, texHomebrews, 325, 260, 48, 48);
	m_sourceBtnUsb = _addPicButton("SOURCE/USB_BTN", texUsb, texUsbs, 235, 200, 48, 48);
	m_sourceBtnDML = _addPicButton("SOURCE/DML_BTN", texDML, texDMLs, 295, 200, 48, 48);
	m_sourceBtnEmu = _addPicButton("SOURCE/EMU_BTN", texEmu, texEmus, 355, 200, 48, 48);

	_addUserLabels(m_sourceLblUser, ARRAY_SIZE(m_sourceLblUser), "SOURCE");
	m_sourceBg = _texture("SOURCE/BG", "texture", theme.bg, false);
	m_sourceLblTitle = _addTitle("SOURCE/TITLE", theme.titleFont, L"", 20, 20, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_sourceLblNotice = _addLabel("SOURCE/NOTICE", theme.btnFont, L"", 20, 400, 600, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_sourceLblPage = _addLabel("SOURCE/PAGE_BTN", theme.btnFont, L"", 62, 400, 98, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_sourceBtnPageM = _addPicButton("SOURCE/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 10, 400, 52, 56);
	m_sourceBtnPageP = _addPicButton("SOURCE/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 160, 400, 52, 56);
	m_sourceBtnBack = _addButton("SOURCE/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	
	m_sourceDir = m_cfg.getString("GENERAL", "dir_Source", fmt("%s/source_menu", m_dataDir.c_str()));

	themeName = m_cfg.getString("GENERAL", "theme", "default");
	if(!m_source.load(fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), SOURCE_FILENAME)))
			m_source.load(fmt("%s/%s", m_sourceDir.c_str(), SOURCE_FILENAME));

	int row;
	int col;
	string ImgName;
	
	for(u8 i = 0; i < 12; ++i)
	{
		TexData texConsoleImg;
		TexData texConsoleImgs;
		ImgName = m_source.getString(fmt("BUTTON_%i", i),"image", "");
		if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_themeDataDir.c_str(), ImgName.c_str())) != TE_OK)
		{
			if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_sourceDir.c_str(), ImgName.c_str())) != TE_OK)
				TexHandle.fromPNG(texConsoleImg, favoriteson_png);
		}
		ImgName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "");
		if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_themeDataDir.c_str(), ImgName.c_str())) != TE_OK)
		{
			if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_sourceDir.c_str(), ImgName.c_str())) != TE_OK)
				TexHandle.fromPNG(texConsoleImgs, favoritesons_png);
		}
	
		row = i / 4;
		col = i - (row * 4);
		m_sourceBtnSource[i] = _addPicButton(fmt("SOURCE/SOURCE_BTN_%i", i), texConsoleImg, texConsoleImgs, (30 + 150 * col), (90 + 100 * row), 120, 90);
		_setHideAnim(m_sourceBtnSource[i], fmt("SOURCE/SOURCE_BTN_%i", i), 0, 0, 1.f, 1.f);
	}
	_setHideAnim(m_sourceBtnChannel, "SOURCE/CHANNEL_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceBtnHomebrew, "SOURCE/HOMEBREW_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceBtnUsb, "SOURCE/USB_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceBtnDML, "SOURCE/DML_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceBtnEmu, "SOURCE/EMU_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceLblTitle, "SOURCE/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_sourceLblNotice, "SOURCE/NOTICE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_sourceLblPage, "SOURCE/PAGE_BTN", 0, 0, -1.f, 1.f);
	_setHideAnim(m_sourceBtnPageM, "SOURCE/PAGE_MINUS", 0, 0, -1.f, 1.f);
	_setHideAnim(m_sourceBtnPageP, "SOURCE/PAGE_PLUS", 0, 0, -1.f, 1.f);
	_setHideAnim(m_sourceBtnBack, "SOURCE/BACK_BTN", 0, 0, -2.f, 0.f);	

	_textSource();
	_hideSource(true);
}

void CMenu::_textSource(void)
{
	m_btnMgr.setText(m_sourceLblTitle, _t("stup1", L"Select Source"));
	m_btnMgr.setText(m_sourceLblNotice, _t("stup2", L"** DISABLED **"));
	m_btnMgr.setText(m_sourceBtnBack, _t("cfg10", L"Back"));
}
