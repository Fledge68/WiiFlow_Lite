
#include "menu.hpp"

// Source menu
s16 m_sourceLblPage;
s16 m_sourceBtnPageM;
s16 m_sourceBtnPageP;
s16 m_sourceBtnBack;
s16 m_sourceBtnClear;
s16 m_sourceLblTitle;
s16 m_sourceBtnSource[12];
s16 m_sourceLblUser[4];

TexData m_sourceBg;

string source;
bool exitSource = false;
u8 sourceBtn;
u8 selectedBtns;
static u8 i, j, k;
int curPage;
int numPages;
vector<string> magicNums;
vector<string> tiers;
char btn_selected[16];
char current_btn[16];
int curflow = 1;
bool sm_tier = false;
int channels_type;

void CMenu::_sourceFlow()
{
	const dir_discHdr *hdr = CoverFlow.getHdr();
	if(m_cfg.getBool(SOURCEFLOW_DOMAIN, "remember_last_item", true))
		m_cfg.setString(SOURCEFLOW_DOMAIN, "current_item", strrchr(hdr->path, '/') + 1);
	else
		m_cfg.remove(SOURCEFLOW_DOMAIN, "current_item");

	memset(single_sourcebtn, 0, 16);
	memset(btn_selected, 0, 16);
	strncpy(btn_selected, fmt("BUTTON_%i", hdr->settings[0]), 15);
	source = m_source.getString(btn_selected, "source", "");
	
	if(source == "dml")
		m_current_view = COVERFLOW_GAMECUBE;
	else if(source == "emunand")
	{
		m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_EMU);
		m_current_view = COVERFLOW_CHANNEL;
	}
	else if(source == "realnand")
	{
		m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL);
		m_current_view = COVERFLOW_CHANNEL;
	}
	else if(source == "bothnand")
	{
		m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_BOTH);
		m_current_view = COVERFLOW_CHANNEL;
	}
	else if(source == "homebrew")
		m_current_view = COVERFLOW_HOMEBREW;
	else if(source == "allplugins")
	{
		strncpy(single_sourcebtn, btn_selected, 16);
		m_current_view = COVERFLOW_PLUGIN;
		for(k = 0; k < m_numPlugins; ++k)
			m_plugin.SetEnablePlugin(m_cfg, k, 2); /* force enable */
	}
	else if(source == "plugin")
	{
		magicNums.clear();
		magicNums = m_source.getStrings(btn_selected, "magic", ',');
		if(magicNums.size() > 0)
		{
			if(magicNums.size() > 1)
				strncpy(single_sourcebtn, btn_selected, 16);
			m_current_view = COVERFLOW_PLUGIN;
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
	else if(source =="new_source")
	{
		string fn = m_source.getString(btn_selected, "magic", "");
		if(fsop_FileExist(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str())))
		{
			if(fn == SOURCE_FILENAME)
			{
				sm_tier = false;
				tiers.erase(tiers.begin() + 1, tiers.end());
			}
			else
			{
				sm_tier = true;
				tiers.push_back(fn);
			}
			m_source.unload();
			m_source.load(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str()));
			SF_cacheCovers = true;
			fn.replace(fn.find("."), 4, "_flow");
			curflow = m_cfg.getInt(SOURCEFLOW_DOMAIN, fn, m_cfg.getInt(SOURCEFLOW_DOMAIN, "last_cf_mode", 1));
			/* get max source button # */
			m_max_source_btn = 0;
			const char *srcDomain = m_source.firstDomain().c_str();
			while(1)
			{
				if(strlen(srcDomain) < 2)
					break;
				if(strrchr(srcDomain, '_') != NULL)
				{
					int srcBtnNumber = atoi(strrchr(srcDomain, '_') + 1);
					if(srcBtnNumber > m_max_source_btn)
						m_max_source_btn = srcBtnNumber;
				}
				srcDomain = m_source.nextDomain().c_str();
			}
			return;
		}
	}
	else //(source == "wii")
		m_current_view = COVERFLOW_WII;
	m_sourceflow = false;
	m_cfg.setUInt("GENERAL", "sources", m_current_view);
	m_source_cnt = 1;
	_setSrcOptions();
}

int CMenu::_getSrcFlow(void)
{
	return curflow;
}

void CMenu::_setSrcFlow(int version)
{
	curflow = version;
	string fn = tiers[tiers.size() - 1];
	fn.replace(fn.find("."), 4, "_flow");
	m_cfg.setInt(SOURCEFLOW_DOMAIN, fn, curflow);
	if(!sm_tier)
		m_cfg.setInt(SOURCEFLOW_DOMAIN, "last_cf_mode", curflow);
}

bool CMenu::_srcTierBack(bool home)
{
	if(!sm_tier)
		return false;
	string fn;
	if(home)
	{
		fn = tiers[0];
		tiers.erase(tiers.begin() + 1, tiers.end());
	}
	else
	{
		fn = tiers[tiers.size() - 2];
		tiers.pop_back();
	}
	
	if(fn == SOURCE_FILENAME)
		sm_tier = false;
	else
		sm_tier = true;
	m_source.unload();
	m_source.load(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str()));
	fn.replace(fn.find("."), 4, "_flow");
	curflow = m_cfg.getInt(SOURCEFLOW_DOMAIN, fn, m_cfg.getInt(SOURCEFLOW_DOMAIN, "last_cf_mode", 1));
	/* get max source button # */
	m_max_source_btn = 0;
	const char *srcDomain = m_source.firstDomain().c_str();
	while(1)
	{
		if(strlen(srcDomain) < 2)
			break;
		if(strrchr(srcDomain, '_') != NULL)
		{
			int srcBtnNumber = atoi(strrchr(srcDomain, '_') + 1);
			if(srcBtnNumber > m_max_source_btn)
				m_max_source_btn = srcBtnNumber;
		}
		srcDomain = m_source.nextDomain().c_str();
	}
	_setSrcFlowBg();
	return true;
}

void CMenu::_setSrcFlowBg(void)
{
	TexHandle.Cleanup(sfbgimg);
	string fn = m_source.getString("general", "background", "");
	if(fn.length() > 0)
	{
		if(TexHandle.fromImageFile(sfbgimg, fmt("%s/backgrounds/%s/%s", m_sourceDir.c_str(), m_themeName.c_str(), fn.c_str())) != TE_OK)
		{
			if(TexHandle.fromImageFile(sfbgimg, fmt("%s/backgrounds/%s", m_sourceDir.c_str(), fn.c_str())) != TE_OK)
			{
				_setBg(m_mainBg, m_mainBgLQ);
				return;
			}
		}
		_setBg(sfbgimg, sfbgimg, true);
	}
	else
		_setBg(m_mainBg, m_mainBgLQ);
}

void CMenu::_hideSource(bool instant)
{
	m_btnMgr.hide(m_sourceLblTitle, instant);
	m_btnMgr.hide(m_sourceLblPage, instant);
	m_btnMgr.hide(m_sourceBtnPageM, instant);
	m_btnMgr.hide(m_sourceBtnPageP, instant);
	m_btnMgr.hide(m_sourceBtnBack, instant);
	m_btnMgr.hide(m_sourceBtnClear, instant);	

	for(i = 0; i < ARRAY_SIZE(m_sourceLblUser); ++i)
		if(m_sourceLblUser[i] != -1)
			m_btnMgr.hide(m_sourceLblUser[i], instant);

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
		if(m_sourceLblUser[i] != -1)
			m_btnMgr.show(m_sourceLblUser[i]);

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
	else
	{
		m_btnMgr.hide(m_sourceLblPage);
		m_btnMgr.hide(m_sourceBtnPageM);
		m_btnMgr.hide(m_sourceBtnPageP);
	}

	sourceBtn = 0;
	selectedBtns = 0;
	j = (curPage - 1) * 12;
	for(i = j; i < (j + 12); ++i)
	{
		if(i > m_max_source_btn)
			m_btnMgr.hide(m_sourceBtnSource[i -j]);
		else
		{
			memset(current_btn, 0, 16);
			strncpy(current_btn, fmt("BUTTON_%i", i), 15);
			string btnSource = m_source.getString(current_btn, "source", "");
			bool src_selected = false;
			if(btnSource == "")
				continue;
			if(m_multisource)
			{
				if(btnSource == "allplugins")
				{
					const vector<bool> &EnabledPlugins = m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
					if(EnabledPlugins.size() == 0)//all plugins enabled
					{
						if(m_current_view & COVERFLOW_PLUGIN)
						{
							sourceBtn = i;
							selectedBtns++;
							src_selected = true;
						}
					}
				}
				else if(btnSource == "plugin")
				{
					magicNums.clear();
					magicNums = m_source.getStrings(current_btn, "magic", ',');
					u32 magic = strtoul(magicNums.at(0).c_str(), NULL, 16);
					if(m_plugin.GetEnableStatus(m_cfg, magic))
					{
						if(m_current_view & COVERFLOW_PLUGIN)
						{
							sourceBtn = i;
							selectedBtns++;
							src_selected = true;
						}
					}
				}
				else if(btnSource == "realnand" && (m_current_view & COVERFLOW_CHANNEL) && (channels_type & CHANNELS_REAL))
				{
					sourceBtn = i;
					selectedBtns++;
					src_selected = true;
				}
				else if(btnSource == "emunand" && (m_current_view & COVERFLOW_CHANNEL) && (channels_type & CHANNELS_EMU))
				{
					sourceBtn = i;
					selectedBtns++;
					src_selected = true;
				}
				else if(btnSource == "bothnand" && (m_current_view & COVERFLOW_CHANNEL) && (channels_type & CHANNELS_BOTH))
				{
					sourceBtn = i;
					selectedBtns++;
					src_selected = true;
				}
				else if(btnSource == "dml" || btnSource == "homebrew" || btnSource == "wii")
				{
					u8 flow = (btnSource == "dml" ? COVERFLOW_GAMECUBE : (btnSource == "homebrew" ? COVERFLOW_HOMEBREW : COVERFLOW_WII));
					if(m_current_view & flow)
					{
						sourceBtn = i;
						selectedBtns++;
						src_selected = true;
					}
				}
			}
			char btn_image[255];
			if(src_selected)
				snprintf(btn_image, sizeof(btn_image), "%s", m_source.getString(current_btn,"image_s", "").c_str());
			else
				snprintf(btn_image, sizeof(btn_image), "%s", m_source.getString(current_btn,"image", "").c_str());
			
			//char btn_image_s[255];
			//snprintf(btn_image_s, sizeof(btn_image_s), "%s", m_source.getString(current_btn,"image_s", "").c_str());
			
			TexData texConsoleImg;
			TexData texConsoleImgs;
			if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s/%s", m_sourceDir.c_str(), m_themeName.c_str(), btn_image)) != TE_OK)
			{
				if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_sourceDir.c_str(), btn_image)) != TE_OK)
					TexHandle.fromImageFile(texConsoleImg, fmt("%s/favoriteson.png", m_imgsDir.c_str()));
			}
			if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s/%s", m_sourceDir.c_str(), m_themeName.c_str(), btn_image)) != TE_OK)
			{
				if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_sourceDir.c_str(), btn_image)) != TE_OK)
					TexHandle.fromImageFile(texConsoleImgs, fmt("%s/favoritesons.png", m_imgsDir.c_str()));
			}
			m_btnMgr.setBtnTexture(m_sourceBtnSource[i - j], texConsoleImg, texConsoleImgs);
			m_btnMgr.show(m_sourceBtnSource[i - j]);
		}
	}
}

bool CMenu::_Source()
{
	bool newSource = false;
	bool updateSource = false;
	exitSource = false;
	curPage = 1;
	numPages = (m_max_source_btn / 12) + 1;
	channels_type = m_cfg.getInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL);
	
	SetupInput();
	_showSource();
	_updateSourceBtns();

	while(!m_exit)
	{
		updateSource = false;
		_mainLoopCommon();
		if(BTN_HOME_PRESSED)
		{
			_hideSource();
			_CfgSrc();
			if(m_cfg.getBool(SOURCEFLOW_DOMAIN, "enabled"))
				return true;
			if(m_multisource)
				newSource = true;
			_showSource();
			_updateSourceBtns();
		}
		if((BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnBack)) || BTN_B_PRESSED)
		{
			if(!m_multisource) break;
			if(selectedBtns == 0)
			{
				m_current_view = COVERFLOW_WII;
				m_source_cnt = 1;
				m_cfg.setUInt("GENERAL", "sources", m_current_view);
				break;
			}
			if(selectedBtns == 1)
			{
				memset(single_sourcebtn, 0, 16);
				memset(btn_selected, 0, 16);
				strncpy(btn_selected, fmt("BUTTON_%i", sourceBtn), 15);

				source = m_source.getString(btn_selected, "source", "");
				if(source == "allplugins")
					strncpy(single_sourcebtn, btn_selected, 16);
				else if(source == "plugin")
				{
					magicNums.clear();
					magicNums = m_source.getStrings(btn_selected, "magic", ',');
					if(magicNums.size() > 1)
						strncpy(single_sourcebtn, btn_selected, 16);
				}
				_setSrcOptions();
			}
			
			m_cfg.setUInt("GENERAL", "sources", m_current_view);
			m_source_cnt = 0;
			for(i = 1; i < 32; i <<= 1)
				if(m_current_view & i)
					m_source_cnt++;
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if((BTN_LEFT_PRESSED && numPages > 1) || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageM)))
		{
			curPage--;
			if(curPage < 1)
				curPage = numPages;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_sourceBtnPageM);
			_updateSourceBtns();
		}
		else if((BTN_RIGHT_PRESSED && numPages > 1) || (BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnPageP)))
		{
			curPage++;
			if(curPage > numPages)
				curPage = 1;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_sourceBtnPageP);
			_updateSourceBtns();
		}
		else if((BTN_MINUS_PRESSED || BTN_PLUS_PRESSED) && sm_tier)
		{
			_srcTierBack(BTN_PLUS_PRESSED);
			updateSource = true;
			curPage = 1;
			numPages = (m_max_source_btn / 12) + 1;
		}
			
		else if(BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnClear))
		{
			m_current_view = COVERFLOW_NONE;
			for(j = 0; m_plugin.PluginExist(j); j++)
				m_plugin.SetEnablePlugin(m_cfg, j, 1);
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
				memset(single_sourcebtn, 0, 16);
				exitSource = true;
				m_catStartPage = 1;
				if(source == "dml")
				{
					m_current_view = COVERFLOW_GAMECUBE;
					_setSrcOptions();
				}
				else if(source == "emunand" || source == "realnand" || source == "bothnand")
				{
					if(source == "emunand")
						channels_type = CHANNELS_EMU;
					else if(source == "realnand")
						channels_type = CHANNELS_REAL;
					else if(source == "bothnand")
						channels_type = CHANNELS_BOTH;
					m_current_view = COVERFLOW_CHANNEL;
					_setSrcOptions();
				}
				else if(source == "homebrew")
				{
					m_current_view = COVERFLOW_HOMEBREW;
					_setSrcOptions();
				}
				else if(source == "allplugins")
				{
					strncpy(single_sourcebtn, btn_selected, 16);
					m_current_view = COVERFLOW_PLUGIN;
					for(k = 0; k < m_numPlugins; ++k)
						m_plugin.SetEnablePlugin(m_cfg, k, 2); /* force enable */
					_setSrcOptions();
				}
				else if(source == "plugin")
				{
					m_current_view = COVERFLOW_PLUGIN;
					_setSrcOptions();
					for(k = 0; k < m_numPlugins; ++k)
						m_plugin.SetEnablePlugin(m_cfg, k, 1); /* force disable */
					magicNums.clear();
					magicNums = m_source.getStrings(btn_selected, "magic", ',');
					if(magicNums.size() > 0)
					{
						if(magicNums.size() > 1)
							strncpy(single_sourcebtn, btn_selected, 16);
						for(vector<string>::iterator itr = magicNums.begin(); itr != magicNums.end(); itr++)
						{
							s8 exist = m_plugin.GetPluginPosition(strtoul(itr->c_str(), NULL, 16));// make sure magic# is valid
							if(exist >= 0)
								m_plugin.SetEnablePlugin(m_cfg, exist, 2);
						}
					}
					m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
					if(enabledPluginsCount == 0) // no magic #'s or invalid ones so default to first plugin in list
						m_plugin.SetEnablePlugin(m_cfg, 0, 2);
				}
				else if(source =="new_source")
				{
					string fn = m_source.getString(btn_selected, "magic", "");
					if(fsop_FileExist(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str())))
					{
						if(fn == SOURCE_FILENAME)
							sm_tier = false;
						else
							sm_tier = true;
						tiers.push_back(fn);
						m_source.unload();
						m_source.load(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str()));
						fn.replace(fn.find("."), 4, "_flow");
						curflow = m_cfg.getInt(SOURCEFLOW_DOMAIN, fn, m_cfg.getInt(SOURCEFLOW_DOMAIN, "last_cf_mode", 1));
						exitSource = false;
						updateSource = true;
						curPage = 1;
						/* get max source button # */
						m_max_source_btn = 0;
						const char *srcDomain = m_source.firstDomain().c_str();
						while(1)
						{
							if(strlen(srcDomain) < 2)
								break;
							if(strrchr(srcDomain, '_') != NULL)
							{
								int srcBtnNumber = atoi(strrchr(srcDomain, '_') + 1);
								if(srcBtnNumber > m_max_source_btn)
									m_max_source_btn = srcBtnNumber;
							}
							srcDomain = m_source.nextDomain().c_str();
						}
						numPages = (m_max_source_btn / 12) + 1;
					}
				}
				else //if(source == "wii") or source is invalid or empty default to wii
				{
					m_current_view = COVERFLOW_WII;
					_setSrcOptions();
				}
			}
			if(m_multisource && i < 12) /* m_multisource */
			{
				updateSource = true;
				if(source == "wii")
					m_current_view ^= COVERFLOW_WII;// toggle on/off
				else if(source == "dml")
					m_current_view ^= COVERFLOW_GAMECUBE;
				else if(source == "emunand")
				{
					if(m_current_view & COVERFLOW_CHANNEL)// if cf channel on then swith only emu type
						channels_type ^= CHANNELS_EMU;
					else
						channels_type = CHANNELS_EMU;// if cf channel off then set to only emu type
					if(channels_type == 0)// if cf channel on and type is set to nothing
					{
						channels_type = CHANNELS_REAL;// make sure channels type is set to default REAL
						m_current_view &= ~COVERFLOW_CHANNEL;// turn off coverflow channels
					}
					else
						m_current_view |= COVERFLOW_CHANNEL;// turn on coverflow channels
				}
				else if(source == "realnand")
				{
					if(m_current_view & COVERFLOW_CHANNEL)
						channels_type ^= CHANNELS_REAL;
					else
						channels_type = CHANNELS_REAL;
					if(channels_type == 0)
					{
						channels_type = CHANNELS_REAL;
						m_current_view &= ~COVERFLOW_CHANNEL;
					}
					else
						m_current_view |= COVERFLOW_CHANNEL;
				}
				else if(source == "bothnand")
				{
					if(m_current_view & COVERFLOW_CHANNEL)
						channels_type ^= CHANNELS_BOTH;
					else
						channels_type = CHANNELS_BOTH;
					if(channels_type == 0)
					{
						channels_type = CHANNELS_REAL;
						m_current_view &= ~COVERFLOW_CHANNEL;
					}
					else
						m_current_view |= COVERFLOW_CHANNEL;
				}
				else if(source == "homebrew")
					m_current_view ^= COVERFLOW_HOMEBREW;
				else if(source == "allplugins")
				{
					m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
					for(j = 0; m_plugin.PluginExist(j); ++j)		/* opposite */
						m_plugin.SetEnablePlugin(m_cfg, j, (enabledPluginsCount == 0) ? 2 : 1);
					m_current_view = enabledPluginsCount == 0 ? (m_current_view | COVERFLOW_PLUGIN) : (m_current_view & ~COVERFLOW_PLUGIN);
				}
				else if(source == "plugin")
				{
					if(!(m_current_view & COVERFLOW_PLUGIN))
					{
						for(j = 0; m_plugin.PluginExist(j); ++j)		/* clear all */
							m_plugin.SetEnablePlugin(m_cfg, j, 1);
					}
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
							}
						}
					}
					m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
					m_current_view = enabledPluginsCount > 0 ? (m_current_view | COVERFLOW_PLUGIN) : (m_current_view & ~COVERFLOW_PLUGIN);
				}
				else if(source =="new_source")
				{
					string fn = m_source.getString(btn_selected, "magic", "");
					if(fsop_FileExist(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str())))
					{
						if(fn == SOURCE_FILENAME)
							sm_tier = false;
						else
							sm_tier = true;
						tiers.push_back(fn);
						m_source.unload();
						m_source.load(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str()));
						fn.replace(fn.find("."), 4, "_flow");
						curflow = m_cfg.getInt(SOURCEFLOW_DOMAIN, fn, m_cfg.getInt(SOURCEFLOW_DOMAIN, "last_cf_mode", 1));
						exitSource = false;
						updateSource = true;
						curPage = 1;
						/* get max source button # */
						m_max_source_btn = 0;
						const char *srcDomain = m_source.firstDomain().c_str();
						while(1)
						{
							if(strlen(srcDomain) < 2)
								break;
							if(strrchr(srcDomain, '_') != NULL)
							{
								int srcBtnNumber = atoi(strrchr(srcDomain, '_') + 1);
								if(srcBtnNumber > m_max_source_btn)
									m_max_source_btn = srcBtnNumber;
							}
							srcDomain = m_source.nextDomain().c_str();
						}
						numPages = (m_max_source_btn / 12) + 1;
					}
				}
			}
		}
		if(exitSource)
		{
			m_cfg.setUInt("GENERAL", "sources", m_current_view);
			m_source_cnt = 1;
			newSource = true;
			break;
		}
		if(updateSource)
		{
			newSource = true;
			_updateSourceBtns();
		}
	}
	m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", channels_type);
	_hideSource(true);
	return newSource;
}

void CMenu::_setSrcOptions(void)
{
	m_catStartPage = m_source.getInt(btn_selected, "cat_page", 1);
	u8 category = m_source.getInt(btn_selected, "category", 0);
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
	char autoboot[64];
	autoboot[63] = '\0';
	strncpy(autoboot, m_source.getString(btn_selected, "autoboot", "").c_str(), sizeof(autoboot) - 1);
	if(autoboot[0] != '\0')
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

void CMenu::_initSourceMenu()
{
	memset(single_sourcebtn, 0, 16);
	m_use_source = false;
	
	if(!m_source.load(fmt("%s/%s/%s", m_sourceDir.c_str(), m_themeName.c_str(), SOURCE_FILENAME)))// check for source_menu/theme/source_menu.ini
	{
		if(!m_source.load(fmt("%s/%s", m_sourceDir.c_str(), SOURCE_FILENAME)))// check for source_menu/source_menu.ini
			return;// no source_menu.ini so we dont init nor use source menu, just return.
	}
	else // if source_menu/theme/source_menu.ini found then change m_sourceDir to source_menu/theme/
		m_sourceDir = fmt("%s/%s", m_sourceDir.c_str(), m_themeName.c_str());
	
	/* let wiiflow know source_menu.ini found and we will be using it */
	m_use_source = true;
	
	/* get max source button # */
	m_max_source_btn = 0;
	const char *srcDomain = m_source.firstDomain().c_str();
	while(1)
	{
		if(strlen(srcDomain) < 2)
			break;
		if(strrchr(srcDomain, '_') != NULL)
		{
			int srcBtnNumber = atoi(strrchr(srcDomain, '_') + 1);
			if(srcBtnNumber > m_max_source_btn)
				m_max_source_btn = srcBtnNumber;
		}
		srcDomain = m_source.nextDomain().c_str();
	}

	tiers.clear();
	tiers.push_back(SOURCE_FILENAME);
	sm_tier = false;
	string fn = tiers[0];
	fn.replace(fn.find("."), 4, "_flow");
	curflow = m_cfg.getInt(SOURCEFLOW_DOMAIN, fn, m_cfg.getInt(SOURCEFLOW_DOMAIN, "last_cf_mode", 1));
	
	_addUserLabels(m_sourceLblUser, ARRAY_SIZE(m_sourceLblUser), "SOURCE");
	m_sourceBg = _texture("SOURCE/BG", "texture", theme.bg, false);
	m_sourceLblTitle = _addLabel("SOURCE/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_sourceLblPage = _addLabel("SOURCE/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_sourceBtnPageM = _addPicButton("SOURCE/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_sourceBtnPageP = _addPicButton("SOURCE/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_sourceBtnBack = _addButton("SOURCE/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_sourceBtnClear = _addButton("SOURCE/ALL_BTN", theme.btnFont, L"", 270, 400, 100, 48, theme.btnFontColor);

	int row;
	int col;
	for(i = 0; i < 12; ++i)
	{
		TexData texConsoleImg;
		TexData texConsoleImgs;
		// use favoriteson.png just to initialize the buttons
		TexHandle.fromImageFile(texConsoleImg, fmt("%s/favoriteson.png", m_imgsDir.c_str()));
		TexHandle.fromImageFile(texConsoleImgs, fmt("%s/favoritesons.png", m_imgsDir.c_str()));
	
		row = i / 4;
		col = i - (row * 4);
		m_sourceBtnSource[i] = _addPicButton(fmt("SOURCE/SOURCE_BTN_%i", i), texConsoleImg, texConsoleImgs, (100 + 120 * col), (90 + 100 * row), 100, 80);
		_setHideAnim(m_sourceBtnSource[i], fmt("SOURCE/SOURCE_BTN_%i", i), 0, 0, -2.f, 0.f);
	}
	_setHideAnim(m_sourceLblTitle, "SOURCE/TITLE", 0, 0, -2.f, 0.f);
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
	m_btnMgr.setText(m_sourceBtnBack, _t("cfg10", L"Back"));
	m_btnMgr.setText(m_sourceBtnClear, _t("cat2", L"Clear"));
}
