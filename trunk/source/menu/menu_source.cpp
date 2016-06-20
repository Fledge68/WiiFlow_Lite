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
s16 m_sourceBtnClear;
s16 m_sourceLblTitle;
s16 m_sourceBtnSource[12];
s16 m_sourceLblUser[4];

TexData m_sourceBg;

string source, themeName;
bool exitSource = false;
u8 sourceBtn;
u8 selectedBtns;
static u8 i, j, k;
int curPage;
int numPages;
vector<string> magicNums;
char btn_selected[16];
char current_btn[16];

void CMenu::_sourceFlow()
{
	const dir_discHdr *hdr = CoverFlow.getHdr();
	if(m_cfg.getBool("SOURCEFLOW", "remember_last_item", true))
		m_cfg.setString("SOURCEFLOW", "current_item", strrchr(hdr->path, '/') + 1);
	else
		m_cfg.remove("SOURCEFLOW", "current_item");

	memset(btn_selected, 0, 16);
	strncpy(btn_selected, fmt("BUTTON_%i", hdr->settings[0]), 15);
	source = m_source.getString(btn_selected, "source", "");
	cf_domain = "_COVERFLOW";
	_clearSources();// may have to move this
	/*if(source == "wii")
	{
		m_cfg.setBool(WII_DOMAIN, "source", true);
		if(sf_mode == 0)
		{
			m_current_view = COVERFLOW_WII;
			m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
		}
	}*/
	if(source == "wii")
	{
		m_current_view = COVERFLOW_WII;
		m_cfg.setBool(WII_DOMAIN, "source", true);
	}
	else if(source == "dml")
	{
		m_current_view = COVERFLOW_GAMECUBE;
		m_cfg.setBool(GC_DOMAIN, "source", true);
	}
	else if(source == "emunand")
	{
		m_current_view = COVERFLOW_CHANNEL;
		m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
		m_cfg.setBool(CHANNEL_DOMAIN, "emu_nand", true);
		m_cfg.setBool(CHANNEL_DOMAIN, "real_nand", false);
	}
	else if(source == "realnand")
	{
		m_current_view = COVERFLOW_CHANNEL;
		m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
		m_cfg.setBool(CHANNEL_DOMAIN, "real_nand", true);
		m_cfg.setBool(CHANNEL_DOMAIN, "emu_nand", false);
	}
	else if(source == "homebrew")
	{
		m_current_view = COVERFLOW_HOMEBREW;
		m_cfg.setBool(HOMEBREW_DOMAIN, "source", true);
		if(m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", true))
			cf_domain = "_SMALLFLOW";
	}
	else if(source == "allplugins")
	{
		m_current_view = COVERFLOW_PLUGIN;
		m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
		for(k = 0; k < m_numPlugins; ++k)
			m_plugin.SetEnablePlugin(m_cfg, k, 2); /* force enable */
	}
	else if(source == "plugin")
	{
		magicNums.clear();
		magicNums = m_source.getStrings(btn_selected, "magic", ',');
		if(magicNums.size() > 0 )
		{
			m_current_view = COVERFLOW_PLUGIN;
			m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
			for(k = 0; k < m_numPlugins; ++k)
				m_plugin.SetEnablePlugin(m_cfg, k, 1); /* force disable */
			for(vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
			{
				s8 exist = m_plugin.GetPluginPosition(strtoul(itr->c_str(), NULL, 16));
				if(exist >= 0)
					m_plugin.SetEnablePlugin(m_cfg, exist, 2);
			}
		}
	}
	m_sourceflow = false;// do something with this when in muilti
	//no autoboot if multi mode. may have to make sure autoboot plugins are hidden from flow when multi is on.
	_setSrcOptions();
}

void CMenu::_hideSource(bool instant)
{
	m_btnMgr.hide(m_sourceLblTitle, instant);
	m_btnMgr.hide(m_sourceLblNotice, instant);
	m_btnMgr.hide(m_sourceLblPage, instant);
	m_btnMgr.hide(m_sourceBtnPageM, instant);
	m_btnMgr.hide(m_sourceBtnPageP, instant);
	m_btnMgr.hide(m_sourceBtnBack, instant);
	m_btnMgr.hide(m_sourceBtnClear, instant);	

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
	
	for(i = 0; i < ARRAY_SIZE(m_sourceLblUser); ++i)
	{
		if(m_sourceLblUser[i] != -1)
			m_btnMgr.show(m_sourceLblUser[i]);
	}

	m_btnMgr.show(m_sourceLblTitle);
	m_btnMgr.show(m_sourceBtnBack);
	if(m_multisource)
		m_btnMgr.show(m_sourceBtnClear);
}

void CMenu::_updateSourceBtns(void)
{
	if(numPages > 1)
	{
		m_btnMgr.setText(m_sourceLblPage, wfmt(L"%i / %i", curPage, numPages));
		m_btnMgr.show(m_sourceLblPage);
		m_btnMgr.show(m_sourceBtnPageM);
		m_btnMgr.show(m_sourceBtnPageP);
	}

	j = (curPage - 1) * 12;
	sourceBtn = 0;
	selectedBtns = 0;
	for(i = 0; i < ((numPages - 1) * 12 + 12); ++i)
	{
		//if(i < 12)
		//	m_btnMgr.hide(m_sourceBtnSource[i], true);
		memset(current_btn, 0, 16);
		strncpy(current_btn, fmt("BUTTON_%i", i), 15);
		string btnSource = m_source.getString(current_btn, "source", "");
		const char *btn_image = m_source.getString(current_btn,"image", "").c_str();
		const char *btn_imageSel = m_source.getString(current_btn,"image_s", "").c_str();
		if(btnSource == "")
			continue;
		if(m_multisource)
		{
			if(btnSource == "allplugins")
			{
				const vector<bool> &EnabledPlugins = m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
				if(EnabledPlugins.size() == 0)//all plugns enabled
				{
					sourceBtn = i;
					selectedBtns++;
					btn_image = btn_imageSel;
				}
			}
			else if(btnSource == "plugin")
			{
				magicNums.clear();
				magicNums = m_source.getStrings(current_btn, "magic", ',');
				u32 magic = strtoul(magicNums.at(0).c_str(), NULL, 16);
				if(m_plugin.GetEnableStatus(m_cfg, magic))
				{
					m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
					sourceBtn = i;
					selectedBtns++;
					btn_image = btn_imageSel;
				}
			}
			else if((btnSource == "realnand" && m_cfg.getBool(CHANNEL_DOMAIN, "real_nand", false)) || 
					(btnSource == "emunand" && m_cfg.getBool(CHANNEL_DOMAIN, "emu_nand", false)))
			{
				m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
				sourceBtn = i;
				selectedBtns++;
				btn_image = btn_imageSel;
			}
			else if(btnSource == "dml" || btnSource == "homebrew" || btnSource == "wii")
			{
				string domain = (btnSource == "dml" ? GC_DOMAIN : (btnSource == "homebrew" ? HOMEBREW_DOMAIN : WII_DOMAIN));
				if(m_cfg.getBool(domain, "source", false))
				{
					sourceBtn = i;
					selectedBtns++;
					btn_image = btn_imageSel;
				}
			}
		}
		if(i >= j && i < (j + 12))
		{
			TexData texConsoleImg;
			TexData texConsoleImgs;
			if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), btn_image)) != TE_OK)
			{
				if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_sourceDir.c_str(), btn_image)) != TE_OK)
					TexHandle.fromImageFile(texConsoleImg, fmt("%s/favoriteson.png", m_imgsDir.c_str()));
			}
			if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), btn_image)) != TE_OK)
			{
				if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_sourceDir.c_str(), btn_image)) != TE_OK)
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

bool CMenu::_Source()
{
	bool newSource = false;
	bool updateSource = false;
	exitSource = false;
	m_showtimer = 0;
	curPage = 1;
	numPages = 1;
	
	SetupInput();
	_showSource();

	//set number of pages based on highest source btn number used
	for(i = m_cfg.getInt("GENERAL", "max_source_buttons", 71); i > 11; --i)
	{
		string source = m_source.getString(fmt("BUTTON_%i", i), "source", "");
		if(!source.empty())
		{
			numPages = (i / 12) + 1;
			break;
		}
	}
	_updateSourceBtns();

	while(!m_exit)
	{
		updateSource = false;
		_mainLoopCommon();
		if(BTN_HOME_PRESSED)
		{
			_hideSource();
			_CfgSrc();
			if(m_cfg.getBool("SOURCEFLOW", "enabled"))
				return true;
			if(m_multisource)
				newSource = true;
			_showSource();
			_updateSourceBtns();
		}
		if((BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnBack)) || BTN_B_PRESSED)
		{
			if(!m_multisource) break;
			cf_domain = "_COVERFLOW";
			m_combined_view = false;
			if(selectedBtns == 0)
			{
				m_cfg.setBool(WII_DOMAIN, "source", true);
				m_current_view = COVERFLOW_WII;
				break;
			}
			if(selectedBtns == 1)
			{
				memset(btn_selected, 0, 16);
				strncpy(btn_selected, fmt("BUTTON_%i", sourceBtn), 15);
				_setSrcOptions();
				//break;
			}
			u8 sourceCount = 0;
			if(m_cfg.getBool(WII_DOMAIN, "source", false))
			{
				sourceCount++;
				m_current_view = COVERFLOW_WII;
			}
			if(m_cfg.getBool(GC_DOMAIN, "source", false))
			{
				sourceCount++;
				m_current_view = COVERFLOW_GAMECUBE;
			}
			if(m_cfg.getBool(CHANNEL_DOMAIN, "source", false))
			{
				sourceCount++;
				m_current_view = COVERFLOW_CHANNEL;
			}
			if(m_cfg.getBool(HOMEBREW_DOMAIN, "source", false))
			{
				sourceCount++;
				m_current_view = COVERFLOW_HOMEBREW;
			}
			if(m_cfg.getBool(PLUGIN_DOMAIN, "source", false))
			{
				sourceCount++;
				m_current_view = COVERFLOW_PLUGIN;
			}
			if(sourceCount > 1)
			{
				m_combined_view = true;
				m_current_view = COVERFLOW_MAX;
			}
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(((BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) && numPages > 1)
				|| (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageM)))
		{
			curPage--;
			if(curPage < 1)
				curPage = numPages;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_sourceBtnPageM);
			_updateSourceBtns();
		}
		else if(((BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) && numPages > 1)
				|| (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageP)))
		{
			curPage++;
			if(curPage > numPages)
				curPage = 1;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_sourceBtnPageP);
			_updateSourceBtns();
		}
		else if(BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnClear))
		{
			_clearSources();
			for(u8 j = 0; m_plugin.PluginExist(j); j++)
				m_plugin.SetEnablePlugin(m_cfg, j, 1);
			m_cfg.setBool(CHANNEL_DOMAIN, "emu_nand", false);
			m_cfg.setBool(CHANNEL_DOMAIN, "real_nand", false);
			updateSource = true;
		}
		else if(BTN_A_PRESSED)
		{
			j = (curPage - 1) * 12;
			for(i = 0; i < 12; ++i)
			{
				if(m_btnMgr.selected(m_sourceBtnSource[i]))
				{
					memset(btn_selected, 0, 16);
					strncpy(btn_selected, fmt("BUTTON_%i", i + j), 15);
					source = m_source.getString(btn_selected, "source", "");
					break;
				}
			}
			if(!m_multisource && i <12)
			{
				_clearSources();
				exitSource = true;
				m_catStartPage = 1;
				if(source == "dml")
				{
					if(!show_gamecube)
						_showSourceNotice();
					else
					{
						m_cfg.setBool(GC_DOMAIN, "source", true);
						m_current_view = COVERFLOW_GAMECUBE;
						_setSrcOptions();
					}
				}
				else if(source == "emunand" || source == "realnand")
				{
					if(!show_channel)
						_showSourceNotice();
					else
					{
						if(source == "emunand")
						{
							m_current_view = COVERFLOW_CHANNEL;
							m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
							m_cfg.setBool(CHANNEL_DOMAIN, "emu_nand", true);
							m_cfg.setBool(CHANNEL_DOMAIN, "real_nand", false);
						}
						else
						{
							m_current_view = COVERFLOW_CHANNEL;
							m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
							m_cfg.setBool(CHANNEL_DOMAIN, "emu_nand", false);
							m_cfg.setBool(CHANNEL_DOMAIN, "real_nand", true);
						}
						m_current_view = COVERFLOW_CHANNEL;
						_setSrcOptions();
					}
				}
				else if(source == "homebrew")
				{
					if(!show_homebrew)
						_showSourceNotice();
					else
					{
						m_cfg.setBool(HOMEBREW_DOMAIN, "source", true);
						m_current_view = COVERFLOW_HOMEBREW;
						_setSrcOptions();
					}
				}
				else if(source == "allplugins")
				{
					if(!show_plugin)
						_showSourceNotice();
					else
					{
						m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
						m_current_view = COVERFLOW_PLUGIN;
						for(k = 0; k < m_numPlugins; ++k)
							m_plugin.SetEnablePlugin(m_cfg, k, 2); /* force enable */
						_setSrcOptions();
					}
				}
				else if(source == "plugin")
				{
					if(!show_plugin)
						_showSourceNotice();
					else
					{
						m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
						m_current_view = COVERFLOW_PLUGIN;
						_setSrcOptions();
						for(k = 0; k < m_numPlugins; ++k)
							m_plugin.SetEnablePlugin(m_cfg, k, 1); /* force disable */
						magicNums.clear();
						magicNums = m_source.getStrings(btn_selected, "magic", ',');
						if(magicNums.size() > 0)
						{
							for(vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
							{
								s8 exist = m_plugin.GetPluginPosition(strtoul(itr->c_str(), NULL, 16));// make sure magic# is valid
								if(exist >= 0)
									m_plugin.SetEnablePlugin(m_cfg, exist, 2);
							}
						}
						m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
						if(enabledPluginsCount == 0) // zero magic #'s or invalid ones so default to first plugin in list
							m_plugin.SetEnablePlugin(m_cfg, 0, 2);
					}
				}
				else //if(source == "wii") or source is invalid or empty default to wii
				{
					m_cfg.setBool(WII_DOMAIN, "source", true);
					m_current_view = COVERFLOW_WII;
					_setSrcOptions();
				}
			}
			if(m_multisource && i < 12) /* m_multisource */
			{
				updateSource = true;
				if(source == "wii")
				{
					m_cfg.setBool(WII_DOMAIN, "source", !m_cfg.getBool(WII_DOMAIN, "source"));
				}
				else if(source == "dml")
				{
					if(show_gamecube)
						m_cfg.setBool(GC_DOMAIN, "source", !m_cfg.getBool(GC_DOMAIN, "source"));
				}
				else if(source == "emunand" || source == "realnand")
				{
					if(show_channel)
					{
						if(source == "realnand")
							m_cfg.setBool(CHANNEL_DOMAIN, "real_nand", !m_cfg.getBool(CHANNEL_DOMAIN, "real_nand"));
						else
							m_cfg.setBool(CHANNEL_DOMAIN, "emu_nand", !m_cfg.getBool(CHANNEL_DOMAIN, "emu_nand"));
						if(m_cfg.getBool(CHANNEL_DOMAIN, "emu_nand") || m_cfg.getBool(CHANNEL_DOMAIN, "real_nand"))
							m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
						else
							m_cfg.setBool(CHANNEL_DOMAIN, "source", false);
					}
				}
				else if(source == "homebrew")
				{
					if(show_homebrew)
						m_cfg.setBool(HOMEBREW_DOMAIN, "source", !m_cfg.getBool(HOMEBREW_DOMAIN, "source"));
				}
				else if(source == "allplugins")
				{
					if(show_plugin)
					{
						m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
						for(j = 0; m_plugin.PluginExist(j); ++j)		/* opposite */
							m_plugin.SetEnablePlugin(m_cfg, j, (enabledPluginsCount == 0) ? 2 : 1);
						m_cfg.setBool(PLUGIN_DOMAIN, "source", (enabledPluginsCount == 0) ? true : false);
					}
				}
				else if(source == "plugin")
				{
					if(show_plugin)
					{
						magicNums.clear();
						magicNums = m_source.getStrings(btn_selected, "magic", ',');
						if(!magicNums.empty())
						{
							for(vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
							{
								s8 exist = m_plugin.GetPluginPosition(strtoul(itr->c_str(), NULL, 16));
								if(exist >= 0)
								{
									bool enabled = m_plugin.GetEnableStatus(m_cfg, strtoul(itr->c_str(), NULL, 16));
									m_plugin.SetEnablePlugin(m_cfg, exist, enabled ? 1 : 2);
									//break;
								}
							}
						}
						m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
						m_cfg.setBool(PLUGIN_DOMAIN, "source", enabledPluginsCount > 0 ? true : false);
					}
				}
			}
		}
		if(exitSource)
		{
			m_combined_view = false;
			newSource = true;
			break;
		}
		if(updateSource)
		{
			newSource = true;
			_updateSourceBtns();
		}
		if(m_showtimer > 0)
		{
			if(--m_showtimer == 0)
				m_btnMgr.hide(m_sourceLblNotice);
		}
	}
	_hideSource(true);
	return newSource;
}

//letters in lower case only
static const char sideCovers[4][9] = {
"534e5854", //Snes9X-Next
"534e4553", //SNES9xGX
"4e4f3634", //Not64
"57493634"  //Wii64
};

static const char shortCovers[12][9] = {
"474d4254", //Gambatte
"474d4264", //Gambatte GB
"474d4274", //Gambatte GBC
"56425854", //VBA-Next
"56424158", //VbaGX
"56424168", //VbaGX GB
"56424178", //VbaGX GBC
"56424188", //VbaGX GBA
"4d45445e", //WiiMednafen GB
"4d45446e", //WiiMednafen GBC
"4d45447e", //WiiMednafen GBA
"57495358"	//WiiSX - playstation
};

bool CMenu::_sideCover(const char *magic)
{
	if(magic == NULL)
		return false;
	for(i = 0; i < 4; i++)
	{
		if(memcmp(magic, sideCovers[i], 9) == 0)
			return true;
	}
	return false;
}

bool CMenu::_shortCover(const char *magic)
{
	if(magic == NULL)
		return false;
	for(i = 0; i < 12; i++)
	{
		if(memcmp(magic, shortCovers[i], 9) == 0)
			return true;
	}
	return false;
}

void CMenu::_setSrcOptions(void)
{
	m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
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
	if(m_multisource) return;
	/* autoboot */
	const char *autoboot = m_source.getString(btn_selected, "autoboot", "").c_str();
	if(autoboot != NULL && autoboot[0] != '\0')
	{
		m_source_autoboot = true;
		memset(&m_autoboot_hdr, 0, sizeof(dir_discHdr));
		if(source == "emunand" || source == "realnand")
		{
			if(source == "realnand")
				m_autoboot_hdr.type = TYPE_CHANNEL;
			else
				m_autoboot_hdr.type = TYPE_EMUCHANNEL;
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
	m_use_source = false;
	themeName = m_cfg.getString("GENERAL", "theme", "default");
	if(!m_source.load(fmt("%s/%s/%s", m_sourceDir.c_str(), themeName.c_str(), SOURCE_FILENAME)))
	{
		if(!m_source.load(fmt("%s/%s", m_sourceDir.c_str(), SOURCE_FILENAME)))
			return;
	}
	else
		m_sourceDir = fmt("%s/%s", m_sourceDir.c_str(), themeName.c_str());
	
	m_use_source = true;
	
	_addUserLabels(m_sourceLblUser, ARRAY_SIZE(m_sourceLblUser), "SOURCE");
	m_sourceBg = _texture("SOURCE/BG", "texture", theme.bg, false);
	m_sourceLblTitle = _addTitle("SOURCE/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_sourceLblNotice = _addLabel("SOURCE/NOTICE", theme.btnFont, L"", 20, 400, 600, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_sourceLblPage = _addLabel("SOURCE/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_sourceBtnPageM = _addPicButton("SOURCE/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_sourceBtnPageP = _addPicButton("SOURCE/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_sourceBtnBack = _addButton("SOURCE/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_sourceBtnClear = _addButton("SOURCE/ALL_BTN", theme.btnFont, L"", 270, 400, 100, 48, theme.btnFontColor);

	int row;
	int col;
	string ImgName;
	
	for(i = 0; i < 12; ++i)
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
	_setHideAnim(m_sourceLblTitle, "SOURCE/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_sourceLblNotice, "SOURCE/NOTICE", 0, 0, 1.f, 0.f);
	_setHideAnim(m_sourceLblPage, "SOURCE/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_sourceBtnPageM, "SOURCE/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_sourceBtnPageP, "SOURCE/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_sourceBtnBack, "SOURCE/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_sourceBtnClear, "SOURCE/ALL_BTN", 0, 0, 1.f, -1.f);

	_textSource();
	_hideSource(true);
}

void CMenu::_textSource(void)
{
	m_btnMgr.setText(m_sourceLblTitle, _t("stup1", L"Select Source"));
	m_btnMgr.setText(m_sourceLblNotice, _t("stup2", L"** DISABLED **"));
	m_btnMgr.setText(m_sourceBtnBack, _t("cfg10", L"Back"));
	m_btnMgr.setText(m_sourceBtnClear, _t("cat2", L"Clear"));
}
