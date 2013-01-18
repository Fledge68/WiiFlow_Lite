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

int Source_curPage;
int pages;
u8 numPlugins;
u8 maxBtns = 71;
string m_sourceDir;
Config m_source;
vector<string> magicNums;

// Source menu
s16 m_sourceLblNotice;
s16 m_sourceLblPage;
s16 m_sourceBtnPageM;
s16 m_sourceBtnPageP;
s16 m_sourceBtnBack;
s16 m_sourceLblTitle;
s16 m_sourceBtnSource[12];
s16 m_sourceLblUser[4];
TexData m_sourceBg;
s16 m_sourceBtnDML;
s16 m_sourceBtnEmu;
s16 m_sourceBtnUsb;
s16 m_sourceBtnChannel;
s16 m_sourceBtnHomebrew;

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

	u8 i = 0;
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

	u8 i = 0;
	for(i = 0; i < ARRAY_SIZE(m_sourceLblUser); ++i)
	{
		if(m_sourceLblUser[i] != -1)
			m_btnMgr.show(m_sourceLblUser[i]);
	}

	m_btnMgr.show(m_sourceLblTitle);
	m_btnMgr.show(m_sourceBtnBack);
	
	for(i = maxBtns; i > 11; --i)
	{
		string source = m_source.getString(fmt("BUTTON_%i", i), "source", "");
		if (!source.empty())
		{
			pages = (i / 12) + 1;
			break;
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
	for (u8 i = 0; i < 12; ++i)
		m_btnMgr.hide(m_sourceBtnSource[i], true);

	const char *ImgName = NULL;
	u8 j = (Source_curPage - 1) * 12;
	
	for(u8 i = 0; i < 12; ++i)
	{
		string domain;
		string btnSource = m_source.getString(fmt("BUTTON_%i", i + j), "source", "").c_str();
		if(btnSource == "wii")
			domain = WII_DOMAIN;
		else if(btnSource == "dml")
			domain = GC_DOMAIN;
		else if(btnSource == "homebrew")
			domain = HOMEBREW_DOMAIN;
		else if(btnSource == "emunand")
			domain = CHANNEL_DOMAIN;
		else if(btnSource == "realnand")
			domain = CHANNEL_DOMAIN;
		else if(btnSource == "")
			continue;
		else if(btnSource == "allplugins")
		{
			domain = PLUGIN_DOMAIN;
				bool EnableAll = m_plugin.GetEnabledPlugins(m_cfg).size();
				if(EnableAll)
					ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();
				else
					ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image", "").c_str();
		}
		else if(btnSource == "plugin")
		{
			domain = PLUGIN_DOMAIN;
			if(m_cfg.getBool(domain, "source", false))
			{
				magicNums.clear();
				magicNums = m_source.getStrings(fmt("BUTTON_%i", i + j), "magic", ',');
				if(m_cfg.getBool("PLUGIN", magicNums.at(0), false))
					ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();
				else
					ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image", "").c_str();
			}
			else
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image", "").c_str();
		}
		else
			domain = WII_DOMAIN;
		if(domain != PLUGIN_DOMAIN)
		{
			if(m_cfg.getBool(domain, "source", false))
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image_s", "").c_str();
			else
				ImgName = m_source.getString(fmt("BUTTON_%i", i + j),"image", "").c_str();
		}
		
		TexData texConsoleImg;
		TexData texConsoleImgs;
		
		if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_themeDataDir.c_str(), ImgName)) != TE_OK)
		{
			if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_sourceDir.c_str(), ImgName)) != TE_OK)
				TexHandle.fromPNG(texConsoleImg, favoriteson_png);
		}
		if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_themeDataDir.c_str(), ImgName)) != TE_OK)
		{
			if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_sourceDir.c_str(), ImgName)) != TE_OK)
				TexHandle.fromPNG(texConsoleImgs, favoritesons_png);
		}
		m_btnMgr.setBtnTexture(m_sourceBtnSource[i], texConsoleImg, texConsoleImgs);
		
		const char *source = m_source.getString(fmt("BUTTON_%i", i + j), "source", "").c_str();
		if(source != NULL && source[0] != '\0')
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
	u8 numPlugins = 0;
	bool back = true;

	while((pent = readdir(pdir)) != NULL)
	{
		if(pent->d_name[0] == '.'|| strcasecmp(pent->d_name, "plugins.ini") == 0 || 
			strcasecmp(pent->d_name, "scummvm.ini") == 0)
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

	SetupInput();
	bool show_homebrew = !m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false);
	bool show_channel = !m_cfg.getBool("GENERAL", "hidechannel", false);
	bool show_emu = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
	bool parental_homebrew = m_cfg.getBool(HOMEBREW_DOMAIN, "parental", false);	

	m_showtimer = 0;
	Source_curPage = 1;
	pages = 1;

	_showSource();
	if(!m_source.loaded())
	{
		m_btnMgr.show(m_sourceBtnHomebrew);
		m_btnMgr.show(m_sourceBtnChannel);
		m_btnMgr.show(m_sourceBtnUsb);
		m_btnMgr.show(m_sourceBtnDML);
		m_btnMgr.show(m_sourceBtnEmu);
	}
	else
		_updateSourceBtns();

	while(!m_exit)
	{
		_mainLoopCommon();
		bool imgSelected = false;
		if(BTN_HOME_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnBack)))
		{
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
			if(sourceCount == 0)
				m_cfg.setBool(WII_DOMAIN, "source", true);
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) && pages > 1) || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageM)))
		{
			Source_curPage--;
			if(Source_curPage < 1)
				Source_curPage = pages;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_sourceBtnPageM);
			_updateSourceBtns();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED) && pages > 1) || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageP)))
		{
			Source_curPage++;
			if(Source_curPage > pages)
				Source_curPage = 1;
			if (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_sourceBtnPageP);
			_updateSourceBtns();
		}
		else if(BTN_A_PRESSED)
		{
			// check default source buttons when no source_menu.ini
			if(m_btnMgr.selected(m_sourceBtnUsb))
			{
				m_current_view = COVERFLOW_USB;
				imgSelected = true;
			}
			if(m_btnMgr.selected(m_sourceBtnDML))
			{
				if (!m_show_dml && !m_devo_installed) _showSourceNotice();
				else
				{
					m_current_view = COVERFLOW_DML;
					imgSelected = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnChannel))
			{
				if (!show_channel) _showSourceNotice();
				else
				{
					m_current_view = COVERFLOW_CHANNEL;
					imgSelected = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnHomebrew))
			{
				if (!show_homebrew || (!parental_homebrew && m_locked)) _showSourceNotice(); 
				else
				{
					m_current_view = COVERFLOW_HOMEBREW;
					imgSelected = true;
				}
			}
			if(m_btnMgr.selected(m_sourceBtnEmu))
			{
				if (!show_emu) _showSourceNotice();
				else
				{
					m_current_view = COVERFLOW_PLUGIN;
					imgSelected = true;
				}
			}
			// check actual source menu buttons
			u8 j = (Source_curPage - 1) * 12;
			for(int i = 0; i < 12; ++i)
			{
				if(m_btnMgr.selected(m_sourceBtnSource[i]))
				{
					string source = m_source.getString(fmt("BUTTON_%i", i + j), "source", "");
					if (source == "wii")
					{
						m_current_view = COVERFLOW_USB;
						imgSelected = true;
						break;
					}
					if (source == "dml")
					{
						if (!m_show_dml && !m_devo_installed) _showSourceNotice();
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
							m_cfg.setBool(CHANNEL_DOMAIN, "disable", false);
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
							m_cfg.setBool(CHANNEL_DOMAIN, "disable", true);
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
							m_current_view = COVERFLOW_PLUGIN;
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
						if(!show_emu)
							_showSourceNotice();
						else
						{
							m_current_view = COVERFLOW_PLUGIN;
							imgSelected = true;
							magicNums.clear();
							magicNums = m_source.getStrings(fmt("BUTTON_%i", i + j), "magic", ',');
							if (magicNums.size() > 0)
							{
								for (u8 k = 0; k < numPlugins; ++k)
									m_plugin.SetEnablePlugin(m_cfg, k, 1);
								for (vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
								{
									string magic = *itr;
									u32 sourceMagic = strtoul(magic.c_str(), NULL, 16);
									for (u8 k = 0; k < numPlugins; ++k)
									{
										if (sourceMagic == m_plugin.getPluginMagic(k))
											m_plugin.SetEnablePlugin(m_cfg, k, 2);
									}
								}
							}
							_checkForSinglePlugin();
							if(enabledPluginsCount == 1)
							{
								currentPartition = m_cfg.getInt("PLUGINS/PARTITION", PluginMagicWord, 1);
								m_cfg.setInt(PLUGIN_DOMAIN, "partition", currentPartition);
							}
							int layout = m_source.getInt(fmt("BUTTON_%i", i + j), "emuflow", 0);
							if(layout != 0)
								m_cfg.setInt(PLUGIN_DOMAIN, "last_cf_mode", layout);
							break;
						}
					}
				}
			}
			if(imgSelected)
			{
				back = false;
				m_cfg.setBool(WII_DOMAIN, "source", false);
				m_cfg.setBool(GC_DOMAIN, "source", false);
				m_cfg.setBool(CHANNEL_DOMAIN, "source", false);
				m_cfg.setBool(HOMEBREW_DOMAIN, "source", false);
				m_cfg.setBool(PLUGIN_DOMAIN, "source", false);
				switch(m_current_view)
				{
					case COVERFLOW_USB:
						m_cfg.setBool(WII_DOMAIN, "source", true);
						break;
					case COVERFLOW_DML:
						m_cfg.setBool(GC_DOMAIN, "source", true);
						break;
					case COVERFLOW_CHANNEL:
						m_cfg.setBool(CHANNEL_DOMAIN, "source", true);
						break;
					case COVERFLOW_HOMEBREW:
						m_cfg.setBool(HOMEBREW_DOMAIN, "source", true);
						break;
					default:
						m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
				}
				break;
			}
		}
		else if(BTN_B_PRESSED)
		{
			u8 j = (Source_curPage - 1) * 12;
			for(int i = 0; i < 12; ++i)
			{
				if(m_btnMgr.selected(m_sourceBtnSource[i]))
				{
					string source = m_source.getString(fmt("BUTTON_%i", i + j), "source", "");
					if (source == "wii")
					{
						m_cfg.setBool(WII_DOMAIN, "source", !m_cfg.getBool(WII_DOMAIN, "source", false));
						imgSelected = true;
						break;
					}
					if (source == "dml")
					{
						if (!m_show_dml && !m_devo_installed) _showSourceNotice();
						else
						{
							m_cfg.setBool(GC_DOMAIN, "source", !m_cfg.getBool(GC_DOMAIN, "source", false));
							imgSelected = true;
							break;
						}
					}
					if (source == "emunand")
					{
						if (!show_channel) _showSourceNotice();
						else
						{
							m_cfg.setBool(CHANNEL_DOMAIN, "source", !m_cfg.getBool(CHANNEL_DOMAIN, "source", false));
							//m_cfg.setBool(CHANNEL_DOMAIN, "disable", false);
							imgSelected = true;
							break;
						}
					}
					if (source == "realnand")
					{
						if (!show_channel) _showSourceNotice();
						else
						{
							m_cfg.setBool(CHANNEL_DOMAIN, "source", !m_cfg.getBool(CHANNEL_DOMAIN, "source", false));
							//m_cfg.setBool(CHANNEL_DOMAIN, "disable", true);
							imgSelected = true;
							break;
						}
					}
					if (source == "homebrew")
					{
						if (!show_homebrew || (!parental_homebrew && m_locked)) _showSourceNotice(); 
						else
						{
							m_cfg.setBool(HOMEBREW_DOMAIN, "source", !m_cfg.getBool(HOMEBREW_DOMAIN, "source", false));
							imgSelected = true;
							break;
						}
					}
					if (source == "allplugins")
					{
						if (!show_emu) _showSourceNotice();
						else
						{
							imgSelected = true;
							bool EnableAll = m_plugin.GetEnabledPlugins(m_cfg).size();
							for(u8 j = 0; m_plugin.PluginExist(j); j++)
								m_plugin.SetEnablePlugin(m_cfg, j, EnableAll ? 2 : 1);
							if(EnableAll)
								m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
							else
								m_cfg.setBool(PLUGIN_DOMAIN, "source", false);
							break;
						}
					}
					if (source == "plugin")
					{
						if(!show_emu)
							_showSourceNotice();
						else
						{
							imgSelected = true;
							// if all plugin btns off clear plugins before turning one on
							if(!m_cfg.getBool(PLUGIN_DOMAIN, "source", false))
							{
								for (u8 k = 0; k < numPlugins; ++k)
									m_plugin.SetEnablePlugin(m_cfg, k, 1);
							}
							magicNums.clear();
							magicNums = m_source.getStrings(fmt("BUTTON_%i", i + j), "magic", ',');
							if (magicNums.size() > 0)
							{
								for (vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
								{
									string magic = *itr;
									if(m_cfg.getBool("PLUGIN", magic, false))// if plugin btn on turn it off
									{
										_checkForSinglePlugin();
										if(enabledPluginsCount == 1) // if last plugin leave it set but turn domain off
											m_cfg.setBool(PLUGIN_DOMAIN, "source", false);
										else
											m_cfg.setBool("PLUGIN", magic, false);
									}
									else // turn on plugin btn
									{
										m_cfg.setBool(PLUGIN_DOMAIN, "source", true);
										m_cfg.setBool("PLUGIN", magic, true);
									}
								}
							}
							break;
						}
					}
				}
			}
		}
		if(imgSelected)
		{
			back = false;
			_updateSourceBtns();
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

	if(!m_source.loaded())
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

	for(int i = 0; i < 12; ++i)
	{
		_setHideAnim(m_sourceBtnSource[i], fmt("SOURCE/SOURCE_BTN_%i", i), 0, 0, 1.f, 1.f);
	}
	_textSource();
	_hideSource(true);
}

void CMenu::_textSource(void)
{
	m_btnMgr.setText(m_sourceLblTitle, _t("stup1", L"Select Source"));
	m_btnMgr.setText(m_sourceLblNotice, _t("NMMOff", L"** DISABLED **"));
	m_btnMgr.setText(m_sourceBtnBack, _t("cfg10", L"Back"));
}
