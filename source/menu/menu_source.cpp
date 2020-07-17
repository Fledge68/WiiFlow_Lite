
#include "menu.hpp"

// Source menu
s16 m_sourceLblPage;
s16 m_sourceBtnPageM;
s16 m_sourceBtnPageP;
s16 m_sourceBtnBack;
s16 m_sourceLblTitle;
s16 m_sourceBtnSource[12];
s16 m_sourceLblUser[4];

TexData m_sourceBg;

string source;
bool exitSource = false;
static u8 i, j;
int curPage;
int numPages;
vector<string> magicNums;

char btn_selected[16];
char current_btn[16];
int curflow = 1;
bool sm_tier = false;
int channels_type;

static inline int loopNum(int i, int s)
{
	return (i + s) % s;
}

/* this is what happens when a sourceflow cover is clicked on */
void CMenu::_sourceFlow()
{
	string numbers;
	string trs;
	const dir_discHdr *hdr = CoverFlow.getHdr();
	
	// save source number for return
	sm_numbers[sm_numbers.size() - 1] = to_string(hdr->settings[0]);
	numbers = sm_numbers[0];
	for(u8 i = 1; i < sm_numbers.size(); i++)
		numbers.append(',' + sm_numbers[i]);
	m_cfg.setString(SOURCEFLOW_DOMAIN, "numbers", numbers);

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
	else if(source == "plugin")
	{
		magicNums.clear();
		magicNums = m_source.getStrings(btn_selected, "magic", ',');
		if(magicNums.size() > 0)
		{
			for(u8 pos = 0; m_plugin.PluginExist(pos); pos++)
				m_plugin.SetEnablePlugin(pos, 1); // force disable all
			enabledPluginsCount = 0;
			string enabledMagics;
			for(i = 0; i < magicNums.size(); i++)
			{
				u8 pos = m_plugin.GetPluginPosition(strtoul(magicNums[i].c_str(), NULL, 16));
				if(pos < 255)
				{
					enabledPluginsCount++;
					m_plugin.SetEnablePlugin(pos, 2);
					if(i == 0)
						enabledMagics = magicNums[0];
					else
						enabledMagics.append(',' + magicNums[i]);
				}
			}
			m_cfg.setString(PLUGIN_DOMAIN, "enabled_plugins", enabledMagics);
			m_current_view = COVERFLOW_PLUGIN;
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
				sm_numbers.erase(sm_numbers.begin() + 1, sm_numbers.end());
			}
			else
			{
				sm_tier = true;
				tiers.push_back(fn);
				sm_numbers.push_back("0");
			}
			trs = tiers[0];
			numbers = sm_numbers[0];
			for(u8 i = 1; i < tiers.size(); i++)
			{
				trs.append(',' + tiers[i]);
				numbers.append(',' + sm_numbers[i]);
			}
			m_cfg.setString(SOURCEFLOW_DOMAIN, "tiers", trs);
			m_cfg.setString(SOURCEFLOW_DOMAIN, "numbers", numbers);
			
			_srcTierLoad(fn);
			return;
		}
	}
	else if(source == "back_tier")
	{
		_srcTierBack(false);
		return;
	}
	else //(source == "wii")
		m_current_view = COVERFLOW_WII;
	m_sourceflow = false;
	m_cfg.setUInt("GENERAL", "sources", m_current_view);
	m_source_cnt = 1;
	_setSrcOptions();
}

/* get sourceflow cover layout number */
int CMenu::_getSrcFlow(void)
{
	return curflow;
}

/* set sourceflow cover layout to version number and set it in wiiflow_lite.ini */
void CMenu::_setSrcFlow(int version)
{
	curflow = version;
	string fn = tiers[tiers.size() - 1];
	fn.replace(fn.find("."), 4, "_flow");
	m_cfg.setInt(SOURCEFLOW_DOMAIN, fn, curflow);
	if(!sm_tier)
		m_cfg.setInt(SOURCEFLOW_DOMAIN, "last_cf_mode", curflow);
}

/* return back to previous tier or home base tier */
bool CMenu::_srcTierBack(bool home)
{
	if(!sm_tier)
		return false;
	string fn;
	if(home)
	{
		fn = tiers[0];
		tiers.erase(tiers.begin() + 1, tiers.end());
		sm_numbers.erase(sm_numbers.begin() + 1, sm_numbers.end());
	}
	else
	{
		fn = tiers[tiers.size() - 2];
		tiers.pop_back();
		sm_numbers.pop_back();
	}
	
	if(fn == SOURCE_FILENAME)
		sm_tier = false;
	else
		sm_tier = true;
	_srcTierLoad(fn);
	return true;
}

void CMenu::_srcTierLoad(string fn)
{
	m_source.unload();
	m_source.load(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str()));
	fn.replace(fn.find("."), 4, "_flow");
	if(m_source.has("general", "flow"))
		curflow = m_source.getInt("general", "flow", 1);
	else
		curflow = m_cfg.getInt(SOURCEFLOW_DOMAIN, fn, m_cfg.getInt(SOURCEFLOW_DOMAIN, "last_cf_mode", 1));
	if(m_source.has("general", "box_mode"))
		m_cfg.setBool(SOURCEFLOW_DOMAIN, "box_mode", m_source.getBool("general", "box_mode", true));
	if(m_source.has("general", "smallbox"))
		m_cfg.setBool(SOURCEFLOW_DOMAIN, "smallbox", m_source.getBool("general", "smallbox", false));
	SF_cacheCovers = true;
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
	if(!m_sourceflow)
	{
		curPage = stoi(sm_numbers[sm_numbers.size() - 1]) / 12 + 1;
		numPages = (m_max_source_btn / 12) + 1;
	}
}

void CMenu::_restoreSrcTiers()
{
	m_cfg.setString(SOURCEFLOW_DOMAIN, "numbers", sm_numbers_backup);// restore if no source chosen
	m_cfg.setString(SOURCEFLOW_DOMAIN, "tiers", sm_tiers_backup);
	sm_numbers.clear();
	tiers.clear();
	sm_numbers = m_cfg.getStrings(SOURCEFLOW_DOMAIN, "numbers");
	tiers = m_cfg.getStrings(SOURCEFLOW_DOMAIN, "tiers");
	sm_tier = false;
	if(tiers.size() > 1)
		sm_tier = true;
	_srcTierLoad(tiers[tiers.size() - 1]);
}

/* get custom sourceflow background image if available */
void CMenu::_getSFlowBgTex(void)
{
	curCustBg = loopNum(curCustBg + 1, 2);
	string fn = m_source.getString("general", "background", "");
	if(fn.length() > 0)
	{
		if(TexHandle.fromImageFile(m_mainCustomBg[curCustBg], fmt("%s/backgrounds/%s/%s", m_sourceDir.c_str(), m_themeName.c_str(), fn.c_str())) != TE_OK)
		{
			if(TexHandle.fromImageFile(m_mainCustomBg[curCustBg], fmt("%s/backgrounds/%s", m_sourceDir.c_str(), fn.c_str())) != TE_OK)
			{
				curCustBg = loopNum(curCustBg + 1, 2);//reset it
				customBg = false;
				return;
			}
		}
		customBg = true;
	}
	else
	{
		curCustBg = loopNum(curCustBg + 1, 2);//reset it
		customBg = false;
	}
}

/* end of sourceflow stuff - start of source menu stuff */
void CMenu::_hideSource(bool instant)
{
	m_btnMgr.hide(m_sourceLblTitle, instant);
	m_btnMgr.hide(m_sourceLblPage, instant);
	m_btnMgr.hide(m_sourceBtnPageM, instant);
	m_btnMgr.hide(m_sourceBtnPageP, instant);
	m_btnMgr.hide(m_sourceBtnBack, instant);

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
			if(btnSource == "")
				continue;
			
			char btn_image[255];
			snprintf(btn_image, sizeof(btn_image), "%s", m_source.getString(current_btn,"image", "").c_str());
			
			char btn_image_s[255];
			snprintf(btn_image_s, sizeof(btn_image_s), "%s", m_source.getString(current_btn,"image_s", "").c_str());
			
			TexData texConsoleImg;
			TexData texConsoleImgs;
			if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s/%s", m_sourceDir.c_str(), m_themeName.c_str(), btn_image)) != TE_OK)
			{
				if(TexHandle.fromImageFile(texConsoleImg, fmt("%s/%s", m_sourceDir.c_str(), btn_image)) != TE_OK)
					TexHandle.fromImageFile(texConsoleImg, fmt("%s/favoriteson.png", m_imgsDir.c_str()));
			}
			if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s/%s", m_sourceDir.c_str(), m_themeName.c_str(), btn_image_s)) != TE_OK)
			{
				if(TexHandle.fromImageFile(texConsoleImgs, fmt("%s/%s", m_sourceDir.c_str(), btn_image_s)) != TE_OK)
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
	exitSource = false;
	channels_type = m_cfg.getInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL);
	sm_numbers_backup = m_cfg.getString(SOURCEFLOW_DOMAIN, "numbers");//backup for possible restore later
	sm_tiers_backup = m_cfg.getString(SOURCEFLOW_DOMAIN, "tiers");
	
	SetupInput();
	_showSource();
	_updateSourceBtns();
	_hideWaitMessage();// needed for source menu on start

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			if(!_srcTierBack(BTN_HOME_PRESSED))
			{
				_restoreSrcTiers();
				break;
			}
			else
				_updateSourceBtns();
		}
		if(BTN_A_PRESSED && m_btnMgr.selected(m_sourceBtnBack))
		{
			_restoreSrcTiers();
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
			if(i < 12)
			{
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
				else if(source == "plugin")
				{
					magicNums.clear();
					magicNums = m_source.getStrings(btn_selected, "magic", ',');
					if(magicNums.size() > 0)
					{
						for(u8 pos = 0; m_plugin.PluginExist(pos); pos++)
							m_plugin.SetEnablePlugin(pos, 1); // force disable all
						enabledPluginsCount = 0;
						string enabledMagics;
						for(i = 0; i < magicNums.size(); i++)
						{
							u8 pos = m_plugin.GetPluginPosition(strtoul(magicNums[i].c_str(), NULL, 16));
							if(pos < 255)
							{
								enabledPluginsCount++;
								m_plugin.SetEnablePlugin(pos, 2);
								if(i == 0)
									enabledMagics = magicNums[0];
								else
									enabledMagics.append(',' + magicNums[i]);
							}
						}
						m_cfg.setString(PLUGIN_DOMAIN, "enabled_plugins", enabledMagics);
						m_current_view = COVERFLOW_PLUGIN;
						_setSrcOptions();
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
							sm_numbers.erase(sm_numbers.begin() + 1, sm_numbers.end());
						}
						else
						{
							sm_tier = true;
							tiers.push_back(fn);
							sm_numbers.push_back("0");
						}
						string trs = tiers[0];
						string numbers = sm_numbers[0];
						for(u8 i = 1; i < tiers.size(); i++)
						{
							trs.append(',' + tiers[i]);
							numbers.append(',' + sm_numbers[i]);
						}
						m_cfg.setString(SOURCEFLOW_DOMAIN, "tiers", trs);
						m_cfg.setString(SOURCEFLOW_DOMAIN, "numbers", numbers);
			
						m_source.unload();
						m_source.load(fmt("%s/%s", m_sourceDir.c_str(), fn.c_str()));
						fn.replace(fn.find("."), 4, "_flow");
						curflow = m_cfg.getInt(SOURCEFLOW_DOMAIN, fn, m_cfg.getInt(SOURCEFLOW_DOMAIN, "last_cf_mode", 1));
						exitSource = false;
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
						curPage = stoi(sm_numbers[sm_numbers.size() - 1]) / 12 + 1;
						numPages = (m_max_source_btn / 12) + 1;
						_updateSourceBtns();
					}
				}
				else if(source == "back_tier")
				{
					exitSource = false;
					_srcTierBack(false);
					_updateSourceBtns();
				}
				else //if(source == "wii") or source is invalid or empty default to wii
				{
					m_current_view = COVERFLOW_WII;
					_setSrcOptions();
				}
			}
		}
		if(exitSource)// a new source has been chosen
		{
			// save source number for return
			sm_numbers.pop_back();
			sm_numbers.push_back(to_string(i + j));
			string numbers = sm_numbers[0];
			for(u8 i = 1; i < sm_numbers.size(); i++)
				numbers.append(',' + sm_numbers[i]);
			m_cfg.setString(SOURCEFLOW_DOMAIN, "numbers", numbers);
			m_cfg.setUInt("GENERAL", "sources", m_current_view);
			m_source_cnt = 1;
			newSource = true;
			break;
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
	m_use_source = false;
	
	if(!fsop_FileExist(fmt("%s/%s/%s", m_sourceDir.c_str(), m_themeName.c_str(), SOURCE_FILENAME)))// check for source_menu/theme/source_menu.ini
	{
		if(!fsop_FileExist(fmt("%s/%s", m_sourceDir.c_str(), SOURCE_FILENAME)))// check for source_menu/source_menu.ini
			return;// no source_menu.ini so we dont init nor use source menu, just return.
	}
	else // if source_menu/theme/source_menu.ini found then change m_sourceDir to source_menu/theme/
		m_sourceDir = fmt("%s/%s", m_sourceDir.c_str(), m_themeName.c_str());
	
	/* let wiiflow know source_menu.ini found and we will be using it */
	m_use_source = true;
	
	/* Source Menu on start reset tiers before buid menus */
	if(m_cfg.getBool("GENERAL", "source_on_start", false))
	{
		m_cfg.remove(SOURCEFLOW_DOMAIN, "tiers");
		m_cfg.remove(SOURCEFLOW_DOMAIN, "numbers");
	}
	
	sm_numbers.clear();
	tiers.clear();
	sm_numbers = m_cfg.getStrings(SOURCEFLOW_DOMAIN, "numbers");
	tiers = m_cfg.getStrings(SOURCEFLOW_DOMAIN, "tiers");
	if(tiers.size() == 0)
	{
		tiers.push_back(SOURCE_FILENAME);
		sm_numbers.push_back("0");
	}
	sm_tier = false;
	if(tiers.size() > 1)
		sm_tier = true;
		
	string trs = tiers[0];
	string numbers = sm_numbers[0];
	for(u8 i = 1; i < tiers.size(); i++)
	{
		trs.append(',' + tiers[i]);
		numbers.append(',' + sm_numbers[i]);
	}
	m_cfg.setString(SOURCEFLOW_DOMAIN, "tiers", trs);
	m_cfg.setString(SOURCEFLOW_DOMAIN, "numbers", numbers);
	
	_srcTierLoad(tiers[tiers.size() - 1]);

	_addUserLabels(m_sourceLblUser, ARRAY_SIZE(m_sourceLblUser), "SOURCE");
	m_sourceBg = _texture("SOURCE/BG", "texture", theme.bg, false);
	m_sourceLblTitle = _addLabel("SOURCE/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_sourceLblPage = _addLabel("SOURCE/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_sourceBtnPageM = _addPicButton("SOURCE/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_sourceBtnPageP = _addPicButton("SOURCE/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_sourceBtnBack = _addButton("SOURCE/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

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

	_textSource();
	_hideSource(true);
}

void CMenu::_textSource(void)
{
	m_btnMgr.setText(m_sourceLblTitle, _t("stup1", L"Select Source"));
	m_btnMgr.setText(m_sourceBtnBack, _t("cfg10", L"Back"));
}
