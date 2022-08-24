
#include <algorithm>
#include "menu.hpp"

u8 mainCfg_Pages = 14;
u8 mainCfg_curPage = 1;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

/* page 2 stuff */
vector<string> languages_available;
void AddLanguage(char *Path)
{
	char lng[32];
	memset(lng, 0, 32);
	char *lang_chr = strrchr(Path, '/')+1;
	memcpy(lng, lang_chr, min(31u, (u32)(strrchr(lang_chr, '.')-lang_chr)));
	languages_available.push_back(lng);
}

void listThemes(const char * path, vector<string> &themes)
{
	DIR *d;
	struct dirent *dir;
	bool def = false;

	themes.clear();
	d = opendir(path);
	if(d != 0)
	{
		dir = readdir(d);
		while(dir != 0)
		{
			string fileName = dir->d_name;
			def = def || (upperCase(fileName) == "DEFAULT.INI");
			if(fileName.size() > 4 && fileName.substr(fileName.size() - 4, 4) == ".ini")
				themes.push_back(fileName.substr(0, fileName.size() - 4));
			dir = readdir(d);
		}
		closedir(d);
	}
	if(!def)
		themes.push_back("Default");
	sort(themes.begin(), themes.end());
}

/* page 4 stuff */
Config custom_titles;
int currentChannelIndex = -1;
int amountOfChannels = 0;
wstringEx channelName;

const CMenu::SOption CMenu::_exitTo[3] = {
	{ "menu", L"System Menu" },
	{ "hbc", L"HBC" },
	{ "wiiu", L"Wii U Menu" },
};

void CMenu::_hideConfigButtons(bool instant)
{
	m_btnMgr.hide(m_configLbl1, instant);
	m_btnMgr.hide(m_configBtn1, instant);
	m_btnMgr.hide(m_configLbl2, instant);
	m_btnMgr.hide(m_configBtn2, instant);
	m_btnMgr.hide(m_configLbl3, instant);
	m_btnMgr.hide(m_configBtn3, instant);
	m_btnMgr.hide(m_configLbl4, instant);
	m_btnMgr.hide(m_configBtn4, instant);
	
	m_btnMgr.hide(m_configLbl1Val, instant);
	m_btnMgr.hide(m_configBtn1M, instant);
	m_btnMgr.hide(m_configBtn1P, instant);

	m_btnMgr.hide(m_configLbl2Val, instant);
	m_btnMgr.hide(m_configBtn2M, instant);
	m_btnMgr.hide(m_configBtn2P, instant);

	m_btnMgr.hide(m_configLbl3Val, instant);
	m_btnMgr.hide(m_configBtn3M, instant);
	m_btnMgr.hide(m_configBtn3P, instant);

	m_btnMgr.hide(m_configLbl4Val, instant);
	m_btnMgr.hide(m_configBtn4M, instant);
	m_btnMgr.hide(m_configBtn4P, instant);
}

void CMenu::_hideConfigMain(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if(m_configLblUser[i] != -1)
			m_btnMgr.hide(m_configLblUser[i], instant);
	
	_hideConfigButtons(instant);
}

void CMenu::_showConfigMain()
{
	_setBg(m_configBg, m_configBg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if(m_configLblUser[i] != -1)
			m_btnMgr.show(m_configLblUser[i]);

	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", mainCfg_curPage, m_locked ? mainCfg_curPage : mainCfg_Pages));
	
	_hideConfigButtons(true);
	
	m_btnMgr.show(m_configLbl2);
	if(!m_locked)
	{
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configLbl3);
		if(mainCfg_curPage != 14)
			m_btnMgr.show(m_configLbl4);
	}

	if(mainCfg_curPage == 1)
	{
		m_btnMgr.show(m_configBtn2);
		if(!m_locked)
		{
			m_btnMgr.show(m_configBtn1);
			m_btnMgr.show(m_configBtn3);
			m_btnMgr.show(m_configBtn4);
		}
		m_btnMgr.setText(m_configLbl1, _t("cfg3", L"Download covers & banners"));
		m_btnMgr.setText(m_configLbl2, _t("cfg5", L"Parental control"));
		m_btnMgr.setText(m_configLbl3, _t("cfg17", L"Game Partitions"));
		m_btnMgr.setText(m_configLbl4, _t("cfg13", L"NAND Emulation Settings"));
		
		m_btnMgr.setText(m_configBtn1, _t("cfgc5", L"Go"));
		if(m_locked)
			m_btnMgr.setText(m_configBtn2, _t("cfg6", L"Unlock"));
		else
			m_btnMgr.setText(m_configBtn2, _t("cfg7", L"Set code"));
		m_btnMgr.setText(m_configBtn3, _t("cfg14", L"Set"));
		m_btnMgr.setText(m_configBtn4, _t("cfg14", L"Set"));
	}
	else if(mainCfg_curPage == 2)
	{
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1M);
		m_btnMgr.show(m_configBtn1P);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2M);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configBtn4);
		
		m_btnMgr.setText(m_configLbl1, _t("cfga7", L"Theme"));
		m_btnMgr.setText(m_configLbl2, _t("cfgc9", L"WiiFlow Language"));
		m_btnMgr.setText(m_configLbl3, _t("cfgc4", L"Adjust Coverflow"));
		m_btnMgr.setText(m_configLbl4, _t("cfgc8", L"Startup Settings"));
		
		m_btnMgr.setText(m_configLbl1Val, m_cfg.getString("GENERAL", "theme"));
		m_btnMgr.setText(m_configLbl2Val, m_curLanguage);
		m_btnMgr.setText(m_configBtn3, _t("cfgc5", L"Go"));
		m_btnMgr.setText(m_configBtn4, _t("cfgc5", L"Go"));
	}	
	else if(mainCfg_curPage == 3)
	{
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1M);
		m_btnMgr.show(m_configBtn1P);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2M);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4M);
		m_btnMgr.show(m_configBtn4P);

		m_btnMgr.setText(m_configLbl1, _t("cfgb3", L"Default video mode"));
		m_btnMgr.setText(m_configLbl2, _t("cfgb4", L"Default game language"));
		m_btnMgr.setText(m_configLbl3, _t("cfgb9", L"GameCube default settings"));
		m_btnMgr.setText(m_configLbl4, _t("cfgb7", L"Channels Type"));
		
		m_btnMgr.setText(m_configBtn3, _t("dl16", L"Set"));
		
		int i = min(max(0, m_cfg.getInt("GENERAL", "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1);
		m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_GlobalVideoModes[i].id, CMenu::_GlobalVideoModes[i].text));

		i = min(max(0, m_cfg.getInt("GENERAL", "game_language", 0)), (int)ARRAY_SIZE(CMenu::_languages) - 1);
		m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
		
		i = min(max(1, m_cfg.getInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL)), (int)ARRAY_SIZE(CMenu::_ChannelsType)) - 1;
		m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_ChannelsType[i].id, CMenu::_ChannelsType[i].text));
	}
	else if(mainCfg_curPage == 4)
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4M);
		m_btnMgr.show(m_configBtn4P);
		
		m_btnMgr.setText(m_configLbl1, _t("cfgc1", L"Exit To"));
		m_btnMgr.setText(m_configLbl2, _t("cfgd5", L"Save favorite mode state"));
		m_btnMgr.setText(m_configLbl3, _t("cfgd4", L"Path Manager"));
		m_btnMgr.setText(m_configLbl4, _t("cfgg21", L"Return To Channel"));

		int i = min(max(0, m_cfg.getInt("GENERAL", "exit_to", 0)), (int)ARRAY_SIZE(CMenu::_exitTo) - 1);
		m_btnMgr.setText(m_configBtn1, _t(CMenu::_exitTo[i].id, CMenu::_exitTo[i].text));
		
		m_btnMgr.setText(m_configBtn2, m_cfg.getBool("GENERAL", "save_favorites_mode") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configBtn3, _t("cfgc5", L"Go"));
		
		if(!custom_titles.loaded())
			custom_titles.load(fmt("%s/" CTITLES_FILENAME, m_settingsDir.c_str()));
		bool prevNANDemuView = NANDemuView;
		NANDemuView = false;
		ChannelHandle.Init(m_loc.getString(m_curLanguage, "gametdb_code", "EN"));
		NANDemuView = prevNANDemuView;
		amountOfChannels = ChannelHandle.Count();
		channelName = m_loc.getWString(m_curLanguage, "disabled", L"Disabled");
		currentChannelIndex = -1;
		const string &currentChanId = m_cfg.getString("GENERAL", "returnto");
		if(!currentChanId.empty())
		{
			for(int i = 0; i < amountOfChannels; i++)
			{
				if(strncmp(currentChanId.c_str(), ChannelHandle.GetId(i), 4) == 0)
				{
					channelName = custom_titles.getWString("TITLES", currentChanId, ChannelHandle.GetName(i));
					currentChannelIndex = i;
					break;
				}
			}
		}
		m_btnMgr.setText(m_configLbl4Val, channelName);
	}
	else if(mainCfg_curPage == 5)
	{
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1M);
		m_btnMgr.show(m_configBtn1P);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2M);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configLbl3Val);
		m_btnMgr.show(m_configBtn3M);
		m_btnMgr.show(m_configBtn3P);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4M);
		m_btnMgr.show(m_configBtn4P);
		m_btnMgr.setText(m_configLbl1, _t("cfgs1", L"Music volume"));
		m_btnMgr.setText(m_configLbl1Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_music", 255)));
		m_btnMgr.setText(m_configLbl2, _t("cfgs2", L"GUI sound volume"));
		m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_gui", 255)));
		m_btnMgr.setText(m_configLbl3, _t("cfgs3", L"Coverflow sound volume"));
		m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255)));
		m_btnMgr.setText(m_configLbl4, _t("cfgs4", L"Game sound volume"));
		m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_bnr", 255)));
	}
	else if(mainCfg_curPage == 6)
	{
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1M);
		m_btnMgr.show(m_configBtn1P);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2M);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configLbl3Val);
		m_btnMgr.show(m_configBtn3M);
		m_btnMgr.show(m_configBtn3P);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4M);
		m_btnMgr.show(m_configBtn4P);
		m_btnMgr.setText(m_configLbl1, _t("cfgc2", L"Adjust TV width"));
		m_btnMgr.setText(m_configLbl1Val, wfmt(L"%i", 640 * 640 / max(1, m_cfg.getInt("GENERAL", "tv_width", 640))));
		m_btnMgr.setText(m_configLbl2, _t("cfgc3", L"Adjust TV height"));
		m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", 480 * 480 / max(1, m_cfg.getInt("GENERAL", "tv_height", 480))));
		m_btnMgr.setText(m_configLbl3, _t("cfgc6", L"Horizontal offset"));
		m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", -m_cfg.getInt("GENERAL", "tv_x", 0)));
		m_btnMgr.setText(m_configLbl4, _t("cfgc7", L"Vertical offset"));
		m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "tv_y", 0)));
	}	
	else if(mainCfg_curPage == 7)
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configBtn4);
		m_btnMgr.setText(m_configLbl1, _t("cfg701", L"Hide all source buttons"));
		m_btnMgr.setText(m_configBtn1, m_cfg.getBool("GENERAL", "hideviews") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl2, _t("cfg702", L"Hide GameCube button"));
		m_btnMgr.setText(m_configBtn2, m_cfg.getBool(GC_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl3, _t("cfg703", L"Hide channels button"));
		m_btnMgr.setText(m_configBtn3, m_cfg.getBool(CHANNEL_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl4, _t("cfg704", L"Hide plugins button"));
		m_btnMgr.setText(m_configBtn4, m_cfg.getBool(PLUGIN_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
	}
	else if(mainCfg_curPage == 8)
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4M);
		m_btnMgr.show(m_configBtn4P);
		m_btnMgr.setText(m_configLbl1, _t("cfg705", L"Show banner in game settings"));
		m_btnMgr.setText(m_configBtn1, m_cfg.getBool("GENERAL", "banner_in_settings") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl2, _t("cfg706", L"Enable fanart"));
		m_btnMgr.setText(m_configBtn2, m_cfg.getBool("FANART", "enable_fanart") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl3, _t("cfg707", L"Fanart default loop"));
		m_btnMgr.setText(m_configBtn3, !m_cfg.getBool("FANART", "show_cover_after_animation") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl4, _t("cfg708", L"Fanart default ending delay"));
		m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("FANART", "delay_after_animation", 200)));
	}
	else if(mainCfg_curPage == 9)
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4M);
		m_btnMgr.show(m_configBtn4P);
		m_btnMgr.setText(m_configLbl1, _t("cfg709", L"Rumble"));
		m_btnMgr.setText(m_configBtn1, m_cfg.getBool("GENERAL", "rumble") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configLbl2, _t("cfg710", L"Wiimote gestures"));
		m_btnMgr.setText(m_configBtn2, m_cfg.getBool("GENERAL", "wiimote_gestures") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configLbl3, _t("cfg711", L"Screensaver"));
		m_btnMgr.setText(m_configBtn3, !m_cfg.getBool("GENERAL", "screensaver_disabled") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configLbl4, _t("cfg712", L"Screensaver idle seconds"));
		m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "screensaver_idle_seconds", 60)));// inc by 30
	}
	else if(mainCfg_curPage == 10)
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4M);
		m_btnMgr.show(m_configBtn4P);
		m_btnMgr.setText(m_configLbl1, _t("cfg728", L"Upsample music to 48khz"));
		m_btnMgr.setText(m_configBtn1, m_cfg.getBool("general", "resample_to_48khz", true) ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl2, _t("cfg714", L"Display music title"));
		m_btnMgr.setText(m_configBtn2, m_cfg.getBool("GENERAL", "display_music_info") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl3, _t("cfg715", L"Randomize music"));
		m_btnMgr.setText(m_configBtn3, m_cfg.getBool("GENERAL", "randomize_music") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl4, _t("cfg716", L"Music fade rate"));
		m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "music_fade_rate", 8)));
	}
	else if(mainCfg_curPage == 11)
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configBtn4);
		m_btnMgr.setText(m_configLbl1, _t("cfg717", L"Random game boot or select"));
		m_btnMgr.setText(m_configBtn1, m_cfg.getBool("GENERAL", "random_select") ? _t("select", L"Select") : _t("boot", L"Boot"));
		m_btnMgr.setText(m_configLbl2, _t("cfg725", L"Shutdown to idle standby"));
		m_btnMgr.setText(m_configBtn2, m_cfg.getBool("general", "idle_standby", false) ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl3, _t("cfg720", L"Play GC banner sound"));
		m_btnMgr.setText(m_configBtn3, m_cfg.getBool(GC_DOMAIN, "play_banner_sound") ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl4, _t("cfg721", L"Play GC default sound"));
		m_btnMgr.setText(m_configBtn4, m_cfg.getBool(GC_DOMAIN, "play_default_sound") ?  _t("yes", L"Yes") : _t("no", L"No"));
	}
	else if(mainCfg_curPage == 12)
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configBtn4);
		m_btnMgr.setText(m_configLbl1, _t("cfg722", L"Homebrew settings"));
		m_btnMgr.setText(m_configBtn1, _t("cfg14", L"Set"));
		m_btnMgr.setText(m_configLbl2, _t("cfg723", L"Sourceflow settings"));
		m_btnMgr.setText(m_configBtn2, _t("cfg14", L"Set"));
		m_btnMgr.setText(m_configLbl3, _t("cfg718", L"Source Menu on start"));
		m_btnMgr.setText(m_configBtn3, m_cfg.getBool("GENERAL", "source_on_start") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl4, _t("cfg727", L"Use Plugin Database Titles"));
		m_btnMgr.setText(m_configBtn4, m_cfg.getBool(PLUGIN_DOMAIN, "database_titles", true) ?  _t("yes", L"Yes") : _t("no", L"No"));
	}
	else if(mainCfg_curPage == 13)
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configBtn3);
		m_btnMgr.show(m_configBtn4);
		m_btnMgr.setText(m_configLbl1, _t("cfgg49", L"480p Pixel Patch"));
		m_btnMgr.setText(m_configBtn1, m_cfg.getBool(WII_DOMAIN, "fix480p", false) ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configLbl2, _t("cfg726", L"Covers Box Mode"));
		m_btnMgr.setText(m_configBtn2, m_cfg.getBool("general", "box_mode", false) ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configLbl3, _t("cfg713", L"Use HQ covers"));
		m_btnMgr.setText(m_configBtn3, m_cfg.getBool("GENERAL", "cover_use_hq") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl4, _t("cfg724", L"Lock coverflow layouts"));
		m_btnMgr.setText(m_configBtn4, m_cfg.getBool("general", "cf_locked") ? _t("yes", L"Yes") : _t("no", L"No"));
	}
	else // page 14
	{
		m_btnMgr.show(m_configBtn1);
		m_btnMgr.show(m_configBtn2);
		m_btnMgr.show(m_configLbl3Val);
		m_btnMgr.show(m_configBtn3M);
		m_btnMgr.show(m_configBtn3P);
		m_btnMgr.setText(m_configLbl1, _t("cfg729", L"Use system proxy settings"));
		m_btnMgr.setText(m_configBtn1, m_cfg.getBool("PROXY", "proxy_use_system") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configLbl2, _t("cfg730", L"Always show main icons"));
		m_btnMgr.setText(m_configBtn2, !m_cfg.getBool("GENERAL", "auto_hide_icons", true) ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_configLbl3, _t("cfgg61", L"Deflicker Filter"));
		int i = min(max(0, m_cfg.getInt("GENERAL", "deflicker_wii", 0)), (int)ARRAY_SIZE(CMenu::_GlobalDeflickerOptions) - 1);
		m_btnMgr.setText(m_configLbl3Val, _t(CMenu::_GlobalDeflickerOptions[i].id, CMenu::_GlobalDeflickerOptions[i].text));
	}
}

void CMenu::_configMain(void)
{
	mainCfg_curPage = 1;
	int val;
	bool rand_music = m_cfg.getBool("GENERAL", "randomize_music");
	bool hq_covers = m_cfg.getBool("GENERAL", "cover_use_hq");
	bool box_mode = m_cfg.getBool("GENERAL", "box_mode", true);
	bool db_titles = m_cfg.getBool(PLUGIN_DOMAIN, "database_titles", true);
	u32 curLanguage = 0;
	int curTheme = 0;
	vector<string> themes;
	string prevLanguage;
	
	string prevTheme = m_cfg.getString("GENERAL", "theme");
	listThemes(m_themeDir.c_str(), themes);
	for(u32 i = 0; i < themes.size(); ++i)
	{
		if(themes[i] == prevTheme)
		{
			curTheme = i;
			break;
		}
	}
	
	languages_available.clear();
	languages_available.push_back("Default");
	GetFiles(m_languagesDir.c_str(), stringToVector(".ini", '|'), AddLanguage, false, 0);
	sort(languages_available.begin(), languages_available.end());

	for(u32 i = 0; i < languages_available.size(); ++i)
	{
		if(m_curLanguage == languages_available[i])
		{
			curLanguage = i;
			break;
		}
	}
	prevLanguage = languages_available[curLanguage];
	_showConfigMain();
	
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnBack)))
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(!m_locked && (BTN_LEFT_PRESSED || BTN_MINUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageM))))
		{
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_configBtnPageM);
			mainCfg_curPage -= 1;
			if(mainCfg_curPage < 1)
				mainCfg_curPage = mainCfg_Pages;
			_showConfigMain();
		}
		else if(!m_locked && (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageP))))
		{
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_configBtnPageP);
			mainCfg_curPage += 1;
			if(mainCfg_curPage > mainCfg_Pages)
				mainCfg_curPage = 1;
			_showConfigMain();
		}
		else if(BTN_A_PRESSED)
		{
			if(mainCfg_curPage == 1)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					_hideConfigMain();
					_download();
					_showConfigMain();
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					if(m_locked)// unlock
					{
						char code[4];
						_hideConfigMain();
						if(_code(code) && memcmp(code, m_cfg.getString("GENERAL", "parent_code", "").c_str(), 4) == 0)
						{
							m_refreshGameList = true;
							m_locked = false;
						}
						else
							_error(_t("cfgg25",L"Password incorrect."));
						_showConfigMain();
					}
					else //set code
					{
						char code[4];
						_hideConfigMain();
						if(_code(code, true))
						{
							m_refreshGameList = true;
							m_cfg.setString("GENERAL", "parent_code", string(code, 4).c_str());
							m_locked = true;
						}
						_showConfigMain();
					}
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					_hideConfigMain();
					_partitionsCfg();
					_showConfigMain();
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					_hideConfigMain();
					_NandEmuCfg();
					_showConfigMain();
				}
			}
			if(mainCfg_curPage == 2)
			{
				if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
					curTheme = loopNum(curTheme + direction, (int)themes.size());
					m_themeName = themes[curTheme];
					m_cfg.setString("GENERAL", "theme", m_themeName);
					m_btnMgr.setText(m_configLbl1Val, m_cfg.getString("GENERAL", "theme"));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					curLanguage = loopNum(curLanguage + direction, (u32)languages_available.size());
					m_curLanguage = languages_available[curLanguage];
					if(!m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str())))
					{
						m_curLanguage = "Default";
						m_cfg.setString("GENERAL", "language", m_curLanguage.c_str());
						m_loc.unload();
					}
					else
						m_cfg.setString("GENERAL", "language", m_curLanguage.c_str());
					_updateText();
					_showConfigMain();
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_refreshGameList = true;
					_hideConfigMain();
					_cfTheme();
					_showConfigMain();
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					_hideConfigMain();
					_Boot();
					_showConfigMain();
				}
			}
			if(mainCfg_curPage == 3)
			{
				if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
					val = (int)loopNum(m_cfg.getUInt("GENERAL", "game_language", 0) + direction, ARRAY_SIZE(CMenu::_languages));
					m_cfg.setInt("GENERAL", "game_language", val);
					m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_languages[val].id, CMenu::_languages[val].text));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					val = (int)loopNum(m_cfg.getUInt("GENERAL", "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_GlobalVideoModes));
					m_cfg.setInt("GENERAL", "video_mode", val);
					m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_GlobalVideoModes[val].id, CMenu::_GlobalVideoModes[val].text));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					_hideConfigMain();
					_configGC();
					_showConfigMain();
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					val = (int)loopNum((m_cfg.getUInt(CHANNEL_DOMAIN, "channels_type", 1) - 1) + direction, ARRAY_SIZE(CMenu::_ChannelsType));
					m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", val + 1);
					m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_ChannelsType[val].id, CMenu::_ChannelsType[val].text));
					if(m_current_view & COVERFLOW_CHANNEL || m_current_view & COVERFLOW_PLUGIN)
						m_refreshGameList = true;
				}
			}
			if(mainCfg_curPage == 4)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					val = (int)loopNum(m_cfg.getUInt("GENERAL", "exit_to", 0) + 1, ARRAY_SIZE(CMenu::_exitTo));
					m_cfg.setInt("GENERAL", "exit_to", val);
					Sys_ExitTo(val);
					m_btnMgr.setText(m_configBtn1, _t(CMenu::_exitTo[val].id, CMenu::_exitTo[val].text));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					m_cfg.setBool("GENERAL", "save_favorites_mode", !m_cfg.getBool("GENERAL", "save_favorites_mode"));
					m_btnMgr.setText(m_configBtn2, m_cfg.getBool("GENERAL", "save_favorites_mode") ? _t("on", L"On") : _t("off", L"Off"));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					_hideConfigMain();
					_Paths();
					_showConfigMain();
				}
				else if(m_btnMgr.selected(m_configBtn4P))
				{
					if(currentChannelIndex == (amountOfChannels - 1))
					{
						currentChannelIndex = -1;
						m_cfg.remove("GENERAL", "returnto");
						channelName = m_loc.getWString(m_curLanguage, "disabled", L"Disabled");
					}
					else
					{
						currentChannelIndex++;
						m_cfg.setString("GENERAL", "returnto", ChannelHandle.GetId(currentChannelIndex));
						channelName = custom_titles.getWString("TITLES", m_cfg.getString("GENERAL", "returnto"), ChannelHandle.GetName(currentChannelIndex));
					}
					m_btnMgr.setText(m_configLbl4Val, channelName);
				}
				else if(m_btnMgr.selected(m_configBtn4M))
				{
					currentChannelIndex--;
					if(currentChannelIndex == -2)
						currentChannelIndex = amountOfChannels - 1;
					if(currentChannelIndex >= 0)
					{
						m_cfg.setString("GENERAL", "returnto", ChannelHandle.GetId(currentChannelIndex));
						channelName = custom_titles.getWString("TITLES", m_cfg.getString("GENERAL", "returnto"), ChannelHandle.GetName(currentChannelIndex));
					}
					else
					{
						m_cfg.remove("GENERAL", "returnto");
						channelName = m_loc.getWString(m_curLanguage, "disabled", L"Disabled");
					}
					m_btnMgr.setText(m_configLbl4Val, channelName);
				}
			}
			if(mainCfg_curPage == 5)
			{
				if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
					val = m_cfg.getInt("GENERAL", "sound_volume_music") + direction;
					if(val >= 0 && val < 256)
						m_cfg.setInt("GENERAL", "sound_volume_music", val);
					m_btnMgr.setText(m_configLbl1Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_music")));
					MusicPlayer.SetMaxVolume(m_cfg.getInt("GENERAL", "sound_volume_music"));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					val = m_cfg.getInt("GENERAL", "sound_volume_gui") + direction;
					if(val >= 0 && val < 256)
						m_cfg.setInt("GENERAL", "sound_volume_gui", val);
					m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_gui")));
				}
				else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn3P) ? 1 : -1;
					val = m_cfg.getInt("GENERAL", "sound_volume_coverflow") + direction;
					if(val >= 0 && val < 256)
						m_cfg.setInt("GENERAL", "sound_volume_coverflow", val);
					m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_coverflow")));
					CoverFlow.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_coverflow"));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					val = m_cfg.getInt("GENERAL", "sound_volume_bnr") + direction;
					if(val >= 0 && val < 256)
						m_cfg.setInt("GENERAL", "sound_volume_bnr", val);
					m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_bnr")));
					m_bnrSndVol = m_cfg.getInt("GENERAL", "sound_volume_bnr");
				}
			}
			if(mainCfg_curPage == 6)
			{
				if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? -2 : 2;
					val = m_cfg.getInt("GENERAL", "tv_width") + direction;
					if(val >= 512 && val < 801)
						m_cfg.setInt("GENERAL", "tv_width", val);
					m_btnMgr.setText(m_configLbl1Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "tv_width")));
					m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? -2 : 2;
					val = m_cfg.getInt("GENERAL", "tv_height") + direction;
					if(val >= 384 && val < 601)
						m_cfg.setInt("GENERAL", "tv_height", val);
					m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "tv_height")));
					m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
				}
				else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn3P) ? -1 : 1;
					val = m_cfg.getInt("GENERAL", "tv_x") + direction;
					if(val >= -50 && val < 51)
						m_cfg.setInt("GENERAL", "tv_x", val);
					m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "tv_x")));
					m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					val = m_cfg.getInt("GENERAL", "tv_y") + direction;
					if(val >= -50 && val < 51)
						m_cfg.setInt("GENERAL", "tv_y", val);
					m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "tv_y")));
					m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
				}
			}
			if(mainCfg_curPage == 7)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_cfg.setBool("GENERAL", "hideviews", !m_cfg.getBool("GENERAL", "hideviews"));
					m_btnMgr.setText(m_configBtn1, m_cfg.getBool("GENERAL", "hideviews") ? _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					m_cfg.setBool(GC_DOMAIN, "disable", !m_cfg.getBool(GC_DOMAIN, "disable"));
					m_btnMgr.setText(m_configBtn2, m_cfg.getBool(GC_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_cfg.setBool(CHANNEL_DOMAIN, "disable", !m_cfg.getBool(CHANNEL_DOMAIN, "disable"));
					m_btnMgr.setText(m_configBtn3, m_cfg.getBool(CHANNEL_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					m_cfg.setBool(PLUGIN_DOMAIN, "disable", !m_cfg.getBool(PLUGIN_DOMAIN, "disable"));
					m_btnMgr.setText(m_configBtn4, m_cfg.getBool(PLUGIN_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
			}
			if(mainCfg_curPage == 8)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_cfg.setBool("GENERAL", "banner_in_settings", !m_cfg.getBool("GENERAL", "banner_in_settings"));
					m_btnMgr.setText(m_configBtn1, m_cfg.getBool("GENERAL", "banner_in_settings") ? _t("yes", L"Yes") : _t("no", L"No"));
					m_bnr_settings = m_cfg.getBool("GENERAL", "banner_in_settings");
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					m_cfg.setBool("FANART", "enable_fanart", !m_cfg.getBool("FANART", "enable_fanart"));
					m_btnMgr.setText(m_configBtn2, m_cfg.getBool("FANART", "enable_fanart") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_cfg.setBool("FANART", "show_cover_after_animation", !m_cfg.getBool("FANART", "show_cover_after_animation"));
					m_btnMgr.setText(m_configBtn3, m_cfg.getBool("FANART", "show_cover_after_animation") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					val = m_cfg.getInt("FANART", "delay_after_animation") + direction;
					if(val >= 0 && val < 360)
						m_cfg.setInt("FANART", "delay_after_animation", val);
					m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("FANART", "delay_after_animation")));
				}
			}
			if(mainCfg_curPage == 9)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_cfg.setBool("GENERAL", "rumble", !m_cfg.getBool("GENERAL", "rumble"));
					m_btnMgr.setText(m_configBtn1, m_cfg.getBool("GENERAL", "rumble") ? _t("on", L"On") : _t("off", L"Off"));
					m_btnMgr.setRumble(m_cfg.getBool("GENERAL", "rumble"));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					m_cfg.setBool("GENERAL", "wiimote_gestures", !m_cfg.getBool("GENERAL", "wiimote_gestures"));
					m_btnMgr.setText(m_configBtn2, m_cfg.getBool("GENERAL", "wiimote_gestures") ?  _t("on", L"On") : _t("off", L"Off"));
					enable_wmote_roll = m_cfg.getBool("GENERAL", "wiimote_gestures");
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_cfg.setBool("GENERAL", "screensaver_disabled", !m_cfg.getBool("GENERAL", "screensaver_disabled"));
					m_btnMgr.setText(m_configBtn3, !m_cfg.getBool("GENERAL", "screensaver_disabled") ?  _t("on", L"On") : _t("off", L"Off"));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 30 : -30;
					val = m_cfg.getInt("GENERAL", "screensaver_idle_seconds") + direction;
					if(val >= 0 && val < 361)
						m_cfg.setInt("GENERAL", "screensaver_idle_seconds", val);
					m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "screensaver_idle_seconds")));
				}
			}
			if(mainCfg_curPage == 10)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					bool val = !m_cfg.getBool("general", "resample_to_48khz");
					m_cfg.setBool("general", "resample_to_48khz", val);
					m_btnMgr.setText(m_configBtn1, val ? _t("yes", L"Yes") : _t("no", L"No"));
					MusicPlayer.SetResampleSetting(val);
					MusicPlayer.ReLoadCurrentFile();
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					m_cfg.setBool("GENERAL", "display_music_info", !m_cfg.getBool("GENERAL", "display_music_info"));
					m_btnMgr.setText(m_configBtn2, m_cfg.getBool("GENERAL", "display_music_info") ?  _t("yes", L"Yes") : _t("no", L"No"));
					m_music_info = m_cfg.getBool("GENERAL", "display_music_info");
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_cfg.setBool("GENERAL", "randomize_music", !m_cfg.getBool("GENERAL", "randomize_music"));
					m_btnMgr.setText(m_configBtn3, m_cfg.getBool("GENERAL", "randomize_music") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					val = m_cfg.getInt("GENERAL", "music_fade_rate") + direction;
					if(val >= 0 && val < 30)
						m_cfg.setInt("GENERAL", "music_fade_rate", val);
					m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "music_fade_rate")));
					MusicPlayer.SetFadeRate(m_cfg.getInt("GENERAL", "music_fade_rate"));
				}
			}
			if(mainCfg_curPage == 11)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_cfg.setBool("GENERAL", "random_select", !m_cfg.getBool("GENERAL", "random_select"));
					m_btnMgr.setText(m_configBtn1, m_cfg.getBool("GENERAL", "random_select") ? _t("select", L"Select") : _t("boot", L"Boot"));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					bool val = !m_cfg.getBool("general", "idle_standby");
					m_cfg.setBool("general", "idle_standby", val);
					m_btnMgr.setText(m_configBtn2, val ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_cfg.setBool(GC_DOMAIN, "play_banner_sound", !m_cfg.getBool(GC_DOMAIN, "play_banner_sound"));
					m_btnMgr.setText(m_configBtn3, m_cfg.getBool(GC_DOMAIN, "play_banner_sound") ?  _t("yes", L"Yes") : _t("no", L"No"));
					m_gc_play_banner_sound = m_cfg.getBool(GC_DOMAIN, "play_banner_sound", true);
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					m_cfg.setBool(GC_DOMAIN, "play_default_sound", !m_cfg.getBool(GC_DOMAIN, "play_default_sound"));
					m_btnMgr.setText(m_configBtn4, m_cfg.getBool(GC_DOMAIN, "play_default_sound") ?  _t("yes", L"Yes") : _t("no", L"No"));
					m_gc_play_default_sound = m_cfg.getBool(GC_DOMAIN, "play_default_sound", true);
				}
			}
			if(mainCfg_curPage == 12)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					_hideConfigMain();
					_ConfigHB();
					_showConfigMain();
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					_hideConfigMain();
					_ConfigSrc();
					_showConfigMain();
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_cfg.setBool("GENERAL", "source_on_start", !m_cfg.getBool("GENERAL", "source_on_start"));
					m_btnMgr.setText(m_configBtn3, m_cfg.getBool("GENERAL", "source_on_start") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					bool val = !m_cfg.getBool(PLUGIN_DOMAIN, "database_titles");
					m_cfg.setBool(PLUGIN_DOMAIN, "database_titles", val);
					m_btnMgr.setText(m_configBtn4, val ? _t("yes", L"Yes") : _t("no", L"No"));
				}
			}
			if(mainCfg_curPage == 13)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					bool val = !m_cfg.getBool(WII_DOMAIN, "fix480p");
					m_cfg.setBool(WII_DOMAIN, "fix480p", val);
					m_btnMgr.setText(m_configBtn1, val ? _t("on", L"On") : _t("off", L"Off"));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					bool val = !m_cfg.getBool("general", "box_mode");
					m_cfg.setBool("general", "box_mode", val);
					m_btnMgr.setText(m_configBtn2, val ? _t("on", L"On") : _t("off", L"Off"));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_cfg.setBool("GENERAL", "cover_use_hq", !m_cfg.getBool("GENERAL", "cover_use_hq"));
					m_btnMgr.setText(m_configBtn3, m_cfg.getBool("GENERAL", "cover_use_hq") ? _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					bool val = !m_cfg.getBool("general", "cf_locked");
					m_cfg.setBool("general", "cf_locked", val);
					m_btnMgr.setText(m_configBtn4, val ?  _t("yes", L"Yes") : _t("no", L"No"));
					CFLocked = val;
				}
			}
			if(mainCfg_curPage == 14)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					bool val = !m_cfg.getBool("PROXY", "proxy_use_system");
					m_cfg.setBool("PROXY", "proxy_use_system", val);
					mainMenu.proxyUseSystem = val;
					m_btnMgr.setText(m_configBtn1, val ? _t("on", L"On") : _t("off", L"Off"));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					bool val = !m_cfg.getBool("GENERAL", "auto_hide_icons");
					m_cfg.setBool("GENERAL", "auto_hide_icons", val);
					m_btnMgr.setText(m_configBtn2, !val ?  _t("yes", L"Yes") : _t("no", L"No"));
					Auto_hide_icons = val;
				}
				else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn3P) ? 1 : -1;
					m_cfg.setInt("GENERAL", "deflicker_wii", loopNum(m_cfg.getUInt("GENERAL", "deflicker_wii") + direction, ARRAY_SIZE(CMenu::_GlobalDeflickerOptions)));
					int val = m_cfg.getInt("GENERAL", "deflicker_wii");
					m_btnMgr.setText(m_configLbl3Val, _t(CMenu::_GlobalDeflickerOptions[val].id, CMenu::_GlobalDeflickerOptions[val].text));
				}
			}
		}
	}
	if(mainCfg_curPage == 2 && m_curLanguage != prevLanguage)
	{
		m_cacheList.Init(m_settingsDir.c_str(), m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str(), m_pluginDataDir.c_str(),
				 m_cfg.getString(CONFIG_FILENAME_SKIP_DOMAIN,CONFIG_FILENAME_SKIP_KEY,CONFIG_FILENAME_SKIP_DEFAULT));
		fsop_deleteFolder(m_listCacheDir.c_str());// delete cache lists folder and remake it so all lists update.
		fsop_MakeFolder(m_listCacheDir.c_str());
		m_refreshGameList = true;
	}
	if(custom_titles.loaded())
		custom_titles.unload();
	if(rand_music != m_cfg.getBool("GENERAL", "randomize_music"))
		MusicPlayer.Init(m_cfg, m_musicDir, fmt("%s/music", m_themeDataDir.c_str()));
	if(db_titles != m_cfg.getBool(PLUGIN_DOMAIN, "database_titles"))
	{
		fsop_deleteFolder(m_listCacheDir.c_str());
		fsop_MakeFolder(m_listCacheDir.c_str());
		m_refreshGameList = true;
	}
	if(!m_refreshGameList && (hq_covers != m_cfg.getBool("GENERAL", "cover_use_hq") || box_mode != m_cfg.getBool("general", "box_mode")))
		_initCF();
	_hideConfigMain();
}

void CMenu::_initConfigMenu()
{
	m_configBg = _texture("CONFIG/BG", "texture", theme.bg, false);

	_addUserLabels(m_configLblUser, ARRAY_SIZE(m_configLblUser), "CONFIG");
	m_configLblTitle = _addLabel("CONFIG/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_configBtnBack = _addButton("CONFIG/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_configLblPage = _addLabel("CONFIG/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPageM = _addPicButton("CONFIG/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_configBtnPageP = _addPicButton("CONFIG/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	
	m_configLbl1 = _addLabel("CONFIG/LINE1", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtn1 = _addButton("CONFIG/LINE1_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);
	m_configLbl2 = _addLabel("CONFIG/LINE2", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtn2 = _addButton("CONFIG/LINE2_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	m_configLbl3 = _addLabel("CONFIG/LINE3", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtn3 = _addButton("CONFIG/LINE3_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	m_configLbl4 = _addLabel("CONFIG/LINE4", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtn4 = _addButton("CONFIG/LINE4_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);
	
	m_configLbl1Val = _addLabel("CONFIG/LINE1_VAL", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtn1M = _addPicButton("CONFIG/LINE1_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_configBtn1P = _addPicButton("CONFIG/LINE1_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	m_configLbl2Val = _addLabel("CONFIG/LINE2_VAL", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtn2M = _addPicButton("CONFIG/LINE2_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_configBtn2P = _addPicButton("CONFIG/LINE2_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	m_configLbl3Val = _addLabel("CONFIG/LINE3_VAL", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtn3M = _addPicButton("CONFIG/LINE3_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_configBtn3P = _addPicButton("CONFIG/LINE3_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);
	m_configLbl4Val = _addLabel("CONFIG/LINE4_VAL", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtn4M = _addPicButton("CONFIG/LINE4_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_configBtn4P = _addPicButton("CONFIG/LINE4_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);

	_setHideAnim(m_configLbl1, "CONFIG/LINE1", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configBtn1, "CONFIG/LINE1_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLbl2, "CONFIG/LINE2", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configBtn2, "CONFIG/LINE2_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLbl3, "CONFIG/LINE3", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configBtn3, "CONFIG/LINE3_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLbl4, "CONFIG/LINE4", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configBtn4, "CONFIG/LINE4_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_configLbl1Val, "CONFIG/LINE1_VAL", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtn1M, "CONFIG/LINE1_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtn1P, "CONFIG/LINE1_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLbl2Val, "CONFIG/LINE2_VAL", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtn2M, "CONFIG/LINE2_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtn2P, "CONFIG/LINE2_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLbl3Val, "CONFIG/LINE3_VAL", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtn3M, "CONFIG/LINE3_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtn3P, "CONFIG/LINE3_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLbl4Val, "CONFIG/LINE4_VAL", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtn4M, "CONFIG/LINE4_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtn4P, "CONFIG/LINE4_PLUS", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_configLblTitle, "CONFIG/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnBack, "CONFIG/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configLblPage, "CONFIG/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageM, "CONFIG/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageP, "CONFIG/PAGE_PLUS", 0, 0, 1.f, -1.f);

	_hideConfigMain(true);
	_textConfig();
}

void CMenu::_textConfig(void)
{
	m_btnMgr.setText(m_configLblTitle, _t("cfg1", L"Settings"));
	m_btnMgr.setText(m_configBtnBack, _t("cfg10", L"Back"));
}
