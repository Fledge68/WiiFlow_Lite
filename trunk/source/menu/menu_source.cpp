#include <dirent.h>
#include <unistd.h>

#include "menu.hpp"
#include "defines.h"

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

bool exitSource = false;
u8 sourceBtn;
u8 selectedBtns;
int source_curPage;
int source_Pages;
string themeName;
vector<string> magicNums;
char btn_selected[256];

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
	for(u8 i = 0; i < ((source_Pages - 1) * 12 + 12); ++i)
	{
		if(i < 12)
			m_btnMgr.hide(m_sourceBtnSource[i], true);
		string btnSource = m_source.getString(fmt("BUTTON_%i", i), "source", "");
		
		if(btnSource == "")
			continue;
		else if(m_multisource == false)
			ImgName = m_source.getString(fmt("BUTTON_%i", i),"image", "").c_str();
		else if(btnSource == "allplugins")
		{
			const vector<bool> &EnabledPlugins = m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
			if(EnabledPlugins.size() == 0)
			{
				sourceBtn = i;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "").c_str();
			}
			else
				ImgName = m_source.getString(fmt("BUTTON_%i", i),"image", "").c_str();
		}
		else if(btnSource == "plugin")
		{
			magicNums.clear();
			magicNums = m_source.getStrings(fmt("BUTTON_%i", i), "magic", ',');
			if(m_cfg.getBool(PLUGIN_DOMAIN, "source", false) && m_cfg.getBool("PLUGIN", magicNums.at(0), false))
			{
				sourceBtn = i;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "").c_str();
			}
			else
				ImgName = m_source.getString(fmt("BUTTON_%i", i),"image", "").c_str();
		}
		else if(btnSource == "realnand" || btnSource == "emunand")
		{
			ImgName = m_source.getString(fmt("BUTTON_%i", i),"image", "").c_str();
			if(m_cfg.getBool(CHANNEL_DOMAIN, "source", false) && m_cfg.getBool(CHANNEL_DOMAIN, "disable") && btnSource == "realnand")
			{
				sourceBtn = i;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "").c_str();
			}
			else if(m_cfg.getBool(CHANNEL_DOMAIN, "source", false) && !m_cfg.getBool(CHANNEL_DOMAIN, "disable") && btnSource == "emunand")
			{
				sourceBtn = i;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "").c_str();
			}
		}
		else if(btnSource != "realnand" && btnSource != "emunand" && btnSource != "plugin" && btnSource != "allplugins")
		{
			string domain = (btnSource == "dml" ? GC_DOMAIN : (btnSource == "homebrew" ? HOMEBREW_DOMAIN : WII_DOMAIN));
			if(m_cfg.getBool(domain, "source", false))
			{
				sourceBtn = i;
				selectedBtns++;
				ImgName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "").c_str();
			}
			else
				ImgName = m_source.getString(fmt("BUTTON_%i", i),"image", "").c_str();
		}
		
		ImgSelName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "").c_str();
		if(i >= j && i < (j + 12))
		{
			TexData texConsoleImg;
			TexData texConsoleImgs;
			if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), ImgName)) != TE_OK)
			{
				if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_sourceDir.c_str(), ImgName)) != TE_OK)
					TexHandle.fromImageFile(texConsoleImg, fmt("%s/favoriteson.png", m_imgsDir.c_str()));
			}
			if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), ImgSelName)) != TE_OK)
			{
				if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_sourceDir.c_str(), ImgSelName)) != TE_OK)
					TexHandle.fromImageFile(texConsoleImgs, fmt("%s/favoritesons.png", m_imgsDir.c_str()));
			}
			m_btnMgr.setBtnTexture(m_sourceBtnSource[i - j], texConsoleImg, texConsoleImgs);
			m_btnMgr.show(m_sourceBtnSource[i - j]);
		}
	}
}

void CMenu::_showSourceNotice(void)
{
	m_showtimer = 90;
	m_btnMgr.show(m_sourceLblNotice);
	exitSource = false;
}

dir_discHdr sourceList;
void CMenu::_createSFList()
{
	bool show_homebrew = !m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false);
	bool show_channel = !m_cfg.getBool("GENERAL", "hidechannel", false);
	bool show_emu = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
	bool parental_homebrew = m_cfg.getBool(HOMEBREW_DOMAIN, "parental", false);
	m_gameList.clear();
	for(u8 i = 0; i < m_cfg.getInt("GENERAL", "max_source_buttons", 71); i++)
	{
		memset(btn_selected, 0, 256);
		strncpy(btn_selected, fmt("BUTTON_%i", i), 255);
		string source = m_source.getString(btn_selected, "source","");
		if(source == "")
			continue;
		if(source == "dml" && !m_show_dml && !m_devo_installed)
			continue;
		else if(source == "emunand" && !show_channel)
			continue;
		else if(source == "homebrew" && (!show_homebrew || (!parental_homebrew && m_locked)))
			continue;
		else if((source == "plugin" || source == "allplugins") && !show_emu)
			continue;
		const char *path = fmt("%s/%s", m_sourceDir.c_str(), m_source.getString(btn_selected, "image", "").c_str());
		memset((void*)&sourceList, 0, sizeof(dir_discHdr));
		sourceList.index = m_gameList.size();
		strncpy(sourceList.id, "SOURCE", 6);
		strncpy(sourceList.path, path, sizeof(sourceList.path) - 1);
		sourceList.casecolor = 0xFFFFFF;
		sourceList.type = TYPE_SOURCE;		
		sourceList.settings[0] = i;
		const char *title = m_source.getString(btn_selected, "title", fmt("title_%i", i)).c_str();
		mbstowcs(sourceList.title, title, 63);
		Asciify(sourceList.title);
		m_gameList.push_back(sourceList);
	}
}

void CMenu::_sourceFlow()
{
	u8 numPlugins = 0, k;
	if(!m_cfg.getBool(PLUGIN_DOMAIN, "disable", false))
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
	const dir_discHdr *hdr = CoverFlow.getHdr();
	if(m_cfg.getBool("SOURCEFLOW", "remember_last_item", true))
		m_cfg.setString("SOURCEFLOW", "current_item", strrchr(hdr->path, '/') + 1);
	else
		m_cfg.remove("SOURCEFLOW", "current_item");

	memset(btn_selected, 0, 256);
	strncpy(btn_selected, fmt("BUTTON_%i", hdr->settings[0]), 255);
	string source = m_source.getString(btn_selected, "source", "");
	_clearSources();
	if(source == "wii")
	{
		m_current_view = COVERFLOW_USB;
		m_cfg.setBool(WII_DOMAIN, "source", true);
		m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
	}
	else if(source == "dml")
	{
		m_current_view = COVERFLOW_DML;
		m_cfg.setBool(GC_DOMAIN, "source", true);
		m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
	}
	else if(source == "emunand")
	{
		m_current_view = COVERFLOW_CHANNEL;
		m_cfg.setBool(CHANNEL_DOMAIN, "disable", false);
		m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
		m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
	}
	else if(source == "realnand")
	{
		m_current_view = COVERFLOW_CHANNEL;
		m_cfg.setBool(CHANNEL_DOMAIN, "disable", true);
		m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
		m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
	}
	else if(source == "homebrew")
	{
		m_current_view = COVERFLOW_HOMEBREW;
		m_cfg.setBool(HOMEBREW_DOMAIN, "source", true);
		m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
	}
	else if(source == "allplugins")
	{
		m_current_view = COVERFLOW_PLUGIN;
		m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
		m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
		for(k = 0; k < numPlugins; ++k)
			m_plugin.SetEnablePlugin(m_cfg, k, 2); /* force enable */
	}
	else if(source == "plugin")
	{
		magicNums.clear();
		magicNums = m_source.getStrings(btn_selected, "magic", ',');
		u32 plugin_magic_nums = magicNums.size();
		if(plugin_magic_nums != 0)
		{
			m_current_view = COVERFLOW_PLUGIN;
			m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
			for(k = 0; k < numPlugins; ++k)
				m_plugin.SetEnablePlugin(m_cfg, k, 1); /* force disable */
			for(vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
			{
				s8 exist = m_plugin.GetPluginPosition(strtoul(itr->c_str(), NULL, 16));
				if(exist >= 0)
					m_plugin.SetEnablePlugin(m_cfg, exist, 2);
					if(plugin_magic_nums == 1)
					{
						currentPartition = m_cfg.getInt("PLUGINS/PARTITION", itr->c_str(), 1);
						m_cfg.setInt(PLUGIN_DOMAIN, "partition", currentPartition);
					}
			}
		}
		m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
		int layout = m_source.getInt(btn_selected, "emuflow", 0);
		if(layout > 0)
			m_cfg.setInt(PLUGIN_DOMAIN, "last_cf_mode", layout);
		int category = m_source.getInt(btn_selected, "category", 0);
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
	m_sourceflow = false;
	/* autoboot */
	const char *autoboot = m_source.getString(btn_selected, "autoboot", "").c_str();
	if(autoboot != NULL && autoboot[0] != '\0')
	{
		m_source_autoboot = true;
		memset(&m_autoboot_hdr, 0, sizeof(dir_discHdr));
		if(source == "emunand" || source == "realnand")
		{
			m_autoboot_hdr.type = TYPE_CHANNEL;
			memcpy(m_autoboot_hdr.id, autoboot, 4);
		}
		else if(source == "wii")
		{
			m_autoboot_hdr.type = TYPE_WII_GAME;
			memcpy(m_autoboot_hdr.id, autoboot, 6);
		}
		else if(source == "dml")
		{
			m_autoboot_hdr.type = TYPE_GC_GAME;
			memcpy(m_autoboot_hdr.id, autoboot, 6);
		}
		else if(source == "homebrew")
		{
			m_autoboot_hdr.type = TYPE_HOMEBREW;
			mbstowcs(m_autoboot_hdr.title, autoboot, 63);
		}
		else if(source == "plugin")
		{
			m_autoboot_hdr.type = TYPE_PLUGIN;
			mbstowcs(m_autoboot_hdr.title, autoboot, 63);
		}
		else
			m_source_autoboot = false;
	}
}
	
bool CMenu::_Source()
{
	CoverFlow.clear();
	u8 i, j, k;
	bool show_homebrew = !m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false);
	bool show_channel = !m_cfg.getBool("GENERAL", "hidechannel", false);
	bool show_emu = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
	bool parental_homebrew = m_cfg.getBool(HOMEBREW_DOMAIN, "parental", false);	
	bool noChanges = true;
	bool updateSource = false;
	exitSource = false;
	u8 numPlugins = 0;
	m_showtimer = 0;
	source_curPage = 1;
	source_Pages = 1;
	
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

			vector<bool> plugin_list = m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
			if(enabledPluginsCount == 1)
			{ 		
				u8 i = 0;
				for(i = 0; i < plugin_list.size(); ++i)
				{
					if(plugin_list[i] == true)
					break;
				}
				char PluginMagicWord[9];
				memset(PluginMagicWord, 0, sizeof(PluginMagicWord));
				strncpy(PluginMagicWord, fmt("%08x", m_plugin.getPluginMagic(i)), 8);
				currentPartition = m_cfg.getInt("PLUGINS/PARTITION", PluginMagicWord, 1); 		
				m_cfg.setInt(PLUGIN_DOMAIN, "partition", currentPartition);
			}
			u8 sourceCount = 0;
			if(m_cfg.getBool(WII_DOMAIN, "source", false))
				sourceCount++;
			if(m_cfg.getBool(GC_DOMAIN, "source", false))
				sourceCount++;
			if(m_cfg.getBool(CHANNEL_DOMAIN, "source", false))
				sourceCount++;
			if(m_cfg.getBool(HOMEBREW_DOMAIN, "source", false))
				sourceCount++;
			if(m_cfg.getBool(PLUGIN_DOMAIN, "source", false))
				sourceCount++;
			if(sourceCount > 1)
				m_combined_view = true;
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(((BTN_LEFT_PRESSED || (!m_multisource && BTN_MINUS_PRESSED)) && source_Pages > 1)
				|| (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageM)))
		{
			source_curPage--;
			if(source_curPage < 1)
				source_curPage = source_Pages;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_sourceBtnPageM);
			_updateSourceBtns();
		}
		else if(((BTN_RIGHT_PRESSED || (!m_multisource && BTN_PLUS_PRESSED)) && source_Pages > 1)
				|| (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageP)))
		{
			source_curPage++;
			if(source_curPage > source_Pages)
				source_curPage = 1;
			if (!BTN_A_PRESSED)
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
				if(!m_show_dml && !m_devo_installed)
					_showSourceNotice();
				else
				{
					_clearSources();
					m_cfg.setBool(GC_DOMAIN, "source", true);
					exitSource = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnChannel))
			{
				if(!show_channel)
					_showSourceNotice();
				else
				{
					_clearSources();
					m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
					exitSource = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnHomebrew))
			{
				if(!show_homebrew || (!parental_homebrew && m_locked))
					_showSourceNotice();
				else
				{
					_clearSources();
					m_cfg.setBool(HOMEBREW_DOMAIN, "source", true);
					exitSource = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnEmu))
			{
				if(!show_emu)
					_showSourceNotice();
				else
				{
					_clearSources();
					m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
					exitSource = true;
				}
			}
		}
		else if((BTN_A_PRESSED || (BTN_PLUS_PRESSED && m_multisource)) && sourceIniLoaded)
		{
			// check actual source menu buttons
			j = (source_curPage - 1) * 12;
			for(i = 0; i < 12; ++i)
			{
				if(m_btnMgr.selected(m_sourceBtnSource[i]))
				{
					memset(btn_selected, 0, 256);
					strncpy(btn_selected, fmt("BUTTON_%i", i + j), 255);
					string source = m_source.getString(btn_selected, "source", "");
					if(BTN_A_PRESSED)
					{
						_clearSources();
						exitSource = true;
						m_catStartPage = 1;
						if(source == "wii")
						{
							m_cfg.setBool(WII_DOMAIN, "source", true);
							m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
						}
						else if(source == "dml")
						{
							if(!m_show_dml && !m_devo_installed)
								_showSourceNotice();
							else
							{
								m_cfg.setBool(GC_DOMAIN, "source", true);
								m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
							}
						}
						else if(source == "emunand")
						{
							if(!show_channel)
								_showSourceNotice();
							else
							{
								m_cfg.setBool(CHANNEL_DOMAIN, "disable", false);
								m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
								m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
							}
						}
						else if(source == "realnand")
						{
							if(!show_channel)
								_showSourceNotice();
							else
							{
								m_cfg.setBool(CHANNEL_DOMAIN, "disable", true);
								m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
								m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
							}
						}
						else if(source == "homebrew")
						{
							if(!show_homebrew || (!parental_homebrew && m_locked))
								_showSourceNotice();
							else
							{
								m_cfg.setBool(HOMEBREW_DOMAIN, "source", true);
								m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
							}
						}
						else if(source == "allplugins")
						{
							if(!show_emu)
								_showSourceNotice();
							else
							{
								m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
								m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
								for(k = 0; k < numPlugins; ++k)
									m_plugin.SetEnablePlugin(m_cfg, k, 2); /* force enable */
							}
						}
						else if(source == "plugin")
						{
							if(!show_emu)
								_showSourceNotice();
							else
							{
								magicNums.clear();
								magicNums = m_source.getStrings(btn_selected, "magic", ',');
								u32 plugin_magic_nums = magicNums.size();
								if(plugin_magic_nums != 0)
								{
									m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
									for(k = 0; k < numPlugins; ++k)
										m_plugin.SetEnablePlugin(m_cfg, k, 1); /* force disable */
									for(vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
									{
										s8 exist = m_plugin.GetPluginPosition(strtoul(itr->c_str(), NULL, 16));
										if(exist >= 0)
										{
											m_plugin.SetEnablePlugin(m_cfg, exist, 2);
											if(plugin_magic_nums == 1)
											{
												currentPartition = m_cfg.getInt("PLUGINS/PARTITION", itr->c_str(), 1);
												m_cfg.setInt(PLUGIN_DOMAIN, "partition", currentPartition);
											}
										}
									}
								}
								m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
								int layout = m_source.getInt(btn_selected, "emuflow", 0);
								if(layout > 0)
									m_cfg.setInt(PLUGIN_DOMAIN, "last_cf_mode", layout);
								int category = m_source.getInt(btn_selected, "category", 0);
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
						/* autoboot */
						const char *autoboot = m_source.getString(btn_selected, "autoboot", "").c_str();
						if(autoboot != NULL && autoboot[0] != '\0')
						{
							m_source_autoboot = true;
							memset(&m_autoboot_hdr, 0, sizeof(dir_discHdr));
							if(source == "emunand" || source == "realnand")
							{
								m_autoboot_hdr.type = TYPE_CHANNEL;
								memcpy(m_autoboot_hdr.id, autoboot, 4);
							}
							else if(source == "wii")
							{
								m_autoboot_hdr.type = TYPE_WII_GAME;
								memcpy(m_autoboot_hdr.id, autoboot, 6);
							}
							else if(source == "dml")
							{
								m_autoboot_hdr.type = TYPE_GC_GAME;
								memcpy(m_autoboot_hdr.id, autoboot, 6);
							}
							else if(source == "homebrew")
							{
								m_autoboot_hdr.type = TYPE_HOMEBREW;
								mbstowcs(m_autoboot_hdr.title, autoboot, 63);
							}
							else if(source == "plugin")
							{
								m_autoboot_hdr.type = TYPE_PLUGIN;
								mbstowcs(m_autoboot_hdr.title, autoboot, 63);
							}
							else
								m_source_autoboot = false;
						}
						break;
					}
					else
					{
						updateSource = true;
						if(source == "wii")
						{
							m_cfg.setBool(WII_DOMAIN, "source", !m_cfg.getBool(WII_DOMAIN, "source"));
						}
						else if(source == "dml")
						{
							if(m_show_dml || m_devo_installed)
								m_cfg.setBool(GC_DOMAIN, "source", !m_cfg.getBool(GC_DOMAIN, "source"));
						}
						else if(source == "emunand")
						{
							if(show_channel)
							{
								m_cfg.setBool(CHANNEL_DOMAIN, "disable", false);
								m_cfg.setBool(CHANNEL_DOMAIN, "source", !m_cfg.getBool(CHANNEL_DOMAIN, "source"));
							}
						}
						else if(source == "realnand")
						{
							if(show_channel)
							{
								m_cfg.setBool(CHANNEL_DOMAIN, "disable", true);
								m_cfg.setBool(CHANNEL_DOMAIN, "source", !m_cfg.getBool(CHANNEL_DOMAIN, "source"));
							}
						}
						else if(source == "homebrew")
						{
							if(show_homebrew && (parental_homebrew || !m_locked))
								m_cfg.setBool(HOMEBREW_DOMAIN, "source", !m_cfg.getBool(HOMEBREW_DOMAIN, "source"));
						}
						else if(source == "allplugins")
						{
							if(show_emu)
							{
								m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
								for(j = 0; m_plugin.PluginExist(j); ++j)		/* opposite */
									m_plugin.SetEnablePlugin(m_cfg, j, (enabledPluginsCount == 0) ? 2 : 1);
								m_cfg.setBool(PLUGIN_DOMAIN, "source", (enabledPluginsCount == 0) ? true : false);
							}
						}
						else if(source == "plugin")
						{
							if(show_emu)
							{
								magicNums.clear();
								magicNums = m_source.getStrings(btn_selected, "magic", ',');
								if(!magicNums.empty())
								{
									for(vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
									{
										u32 cur_magic = strtoul(itr->c_str(), NULL, 16);
										s8 exist = m_plugin.GetPluginPosition(cur_magic);
										if(exist >= 0)
										{
											bool enabled = m_plugin.GetEnableStatus(m_cfg, cur_magic);
											m_plugin.SetEnablePlugin(m_cfg, exist, enabled ? 1 : 2);
											break;
										}
									}
								}
								m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
								m_cfg.setBool(PLUGIN_DOMAIN, "source", enabledPluginsCount > 0 ? true : false);
							}
						}
						break;
					}
				}
			}
		}
		if(exitSource)
		{
			m_combined_view = false;
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

	TexHandle.fromImageFile(texUsb, fmt("%s/btnusb.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texUsbs, fmt("%s/btnusbs.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texDML, fmt("%s/btndml.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texDMLs, fmt("%s/btndmls.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texEmu, fmt("%s/btnemu.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texEmus, fmt("%s/btnemus.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texChannel, fmt("%s/btnchannel.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texChannels, fmt("%s/btnchannels.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texHomebrew, fmt("%s/btnhomebrew.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texHomebrews, fmt("%s/btnhomebrews.png", m_imgsDir.c_str()));

	_addUserLabels(m_sourceLblUser, ARRAY_SIZE(m_sourceLblUser), "SOURCE");
	m_sourceBg = _texture("SOURCE/BG", "texture", theme.bg, false);
	m_sourceLblTitle = _addTitle("SOURCE/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_sourceLblNotice = _addLabel("SOURCE/NOTICE", theme.btnFont, L"", 20, 400, 600, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_sourceLblPage = _addLabel("SOURCE/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_sourceBtnPageM = _addPicButton("SOURCE/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_sourceBtnPageP = _addPicButton("SOURCE/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_sourceBtnBack = _addButton("SOURCE/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	m_sourceBtnChannel = _addPicButton("SOURCE/CHANNEL_BTN", texChannel, texChannels, 265, 260, 48, 48);
	m_sourceBtnHomebrew = _addPicButton("SOURCE/HOMEBREW_BTN", texHomebrew, texHomebrews, 325, 260, 48, 48);
	m_sourceBtnUsb = _addPicButton("SOURCE/USB_BTN", texUsb, texUsbs, 235, 200, 48, 48);
	m_sourceBtnDML = _addPicButton("SOURCE/DML_BTN", texDML, texDMLs, 295, 200, 48, 48);
	m_sourceBtnEmu = _addPicButton("SOURCE/EMU_BTN", texEmu, texEmus, 355, 200, 48, 48);
	
	themeName = m_cfg.getString("GENERAL", "theme", "default");
	if(!m_source.load(fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), SOURCE_FILENAME)))
		m_source.load(fmt("%s/%s", m_sourceDir.c_str(), SOURCE_FILENAME));
	else
		m_sourceDir = fmt("%s/%s", m_sourceDir.c_str(), themeName.c_str());

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
				TexHandle.fromImageFile(texConsoleImg, fmt("%s/favoriteson.png", m_imgsDir.c_str()));
		}
		ImgName = m_source.getString(fmt("BUTTON_%i", i),"image_s", "");
		if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_themeDataDir.c_str(), ImgName.c_str())) != TE_OK)
		{
			if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_sourceDir.c_str(), ImgName.c_str())) != TE_OK)
				TexHandle.fromImageFile(texConsoleImgs, fmt("%s/favoritesons.png", m_imgsDir.c_str()));
		}
	
		row = i / 4;
		col = i - (row * 4);
		m_sourceBtnSource[i] = _addPicButton(fmt("SOURCE/SOURCE_BTN_%i", i), texConsoleImg, texConsoleImgs, (100 + 120 * col), (90 + 100 * row), 80, 80);
		_setHideAnim(m_sourceBtnSource[i], fmt("SOURCE/SOURCE_BTN_%i", i), 0, 0, -2.f, 0.f);
	}
	_setHideAnim(m_sourceBtnChannel, "SOURCE/CHANNEL_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceBtnHomebrew, "SOURCE/HOMEBREW_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceBtnUsb, "SOURCE/USB_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceBtnDML, "SOURCE/DML_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceBtnEmu, "SOURCE/EMU_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_sourceLblTitle, "SOURCE/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_sourceLblNotice, "SOURCE/NOTICE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_sourceLblPage, "SOURCE/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_sourceBtnPageM, "SOURCE/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_sourceBtnPageP, "SOURCE/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_sourceBtnBack, "SOURCE/BACK_BTN", 0, 0, 1.f, -1.f);

	_textSource();
	_hideSource(true);
}

void CMenu::_textSource(void)
{
	m_btnMgr.setText(m_sourceLblTitle, _t("stup1", L"Select Source"));
	m_btnMgr.setText(m_sourceLblNotice, _t("stup2", L"** DISABLED **"));
	m_btnMgr.setText(m_sourceBtnBack, _t("cfg10", L"Back"));
}
