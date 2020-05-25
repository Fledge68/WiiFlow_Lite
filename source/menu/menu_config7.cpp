
#include "menu.hpp"

using namespace std;

s16 m_config7Lbl1;
s16 m_config7Lbl2;
s16 m_config7Lbl3;
s16 m_config7Lbl4;

s16 m_config7Btn1;
s16 m_config7Btn2;
s16 m_config7Btn3;
s16 m_config7Btn4;

s16 m_config7Lbl4Val;
s16 m_config7Btn4M;
s16 m_config7Btn4P;

s16 m_config7LblUser[4];

TexData m_config7Bg;

void CMenu::_hideConfig7(bool instant)
{
	_hideConfigCommon(instant);
	
	m_btnMgr.hide(m_config7Lbl1, instant);
	m_btnMgr.hide(m_config7Btn1, instant);
	m_btnMgr.hide(m_config7Lbl2, instant);
	m_btnMgr.hide(m_config7Btn2, instant);
	m_btnMgr.hide(m_config7Lbl3, instant);
	m_btnMgr.hide(m_config7Btn3, instant);
	m_btnMgr.hide(m_config7Lbl4, instant);
	m_btnMgr.hide(m_config7Btn4, instant);
	
	m_btnMgr.hide(m_config7Lbl4Val, instant);
	m_btnMgr.hide(m_config7Btn4M, instant);
	m_btnMgr.hide(m_config7Btn4P, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_config7LblUser); ++i)
		if(m_config7LblUser[i] != -1)
			m_btnMgr.hide(m_config7LblUser[i], instant);
}

void CMenu::_showConfig7(int curPage)
{
	_showConfigCommon(m_config7Bg,curPage);
	
	m_btnMgr.hide(m_config7Lbl1, true);
	m_btnMgr.hide(m_config7Btn1, true);
	m_btnMgr.hide(m_config7Lbl2, true);
	m_btnMgr.hide(m_config7Btn2, true);
	m_btnMgr.hide(m_config7Lbl3, true);
	m_btnMgr.hide(m_config7Btn3, true);
	m_btnMgr.hide(m_config7Lbl4, true);
	m_btnMgr.hide(m_config7Btn4, true);

	m_btnMgr.hide(m_config7Lbl4Val, true);
	m_btnMgr.hide(m_config7Btn4M, true);
	m_btnMgr.hide(m_config7Btn4P, true);
	
	_setBg(m_config7Bg, m_config7Bg);

	for(u32 i = 0; i < ARRAY_SIZE(m_config7LblUser); ++i)
		if(m_config7LblUser[i] != -1)
			m_btnMgr.show(m_config7LblUser[i]);

	m_btnMgr.show(m_config7Lbl1);
	m_btnMgr.show(m_config7Btn1);
	m_btnMgr.show(m_config7Lbl2);
	m_btnMgr.show(m_config7Btn2);
	m_btnMgr.show(m_config7Lbl3);
	m_btnMgr.show(m_config7Btn3);
	m_btnMgr.show(m_config7Lbl4);

	if(curPage == 7 || curPage == 11 || curPage == 12 || curPage == 13)
		m_btnMgr.show(m_config7Btn4);
	else
	{
		m_btnMgr.show(m_config7Lbl4Val);
		m_btnMgr.show(m_config7Btn4M);
		m_btnMgr.show(m_config7Btn4P);
	}
	
	if(curPage == 7)
	{
		m_btnMgr.setText(m_config7Lbl1, _t("cfg701", L"Hide all source buttons"));
		m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "hideviews") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl2, _t("cfg702", L"Hide GameCube button"));
		m_btnMgr.setText(m_config7Btn2, m_cfg.getBool(GC_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl3, _t("cfg703", L"Hide channels button"));
		m_btnMgr.setText(m_config7Btn3, m_cfg.getBool(CHANNEL_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl4, _t("cfg704", L"Hide plugins button"));
		m_btnMgr.setText(m_config7Btn4, m_cfg.getBool(PLUGIN_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
	}
	else if(curPage == 8)
	{
		m_btnMgr.setText(m_config7Lbl1, _t("cfg705", L"Show banner in game settings"));
		m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "banner_in_settings") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl2, _t("cfg706", L"Enable fanart"));
		m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("FANART", "enable_fanart") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl3, _t("cfg707", L"Fanart default loop"));
		m_btnMgr.setText(m_config7Btn3, !m_cfg.getBool("FANART", "show_cover_after_animation") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl4, _t("cfg708", L"Fanart default ending delay"));
		m_btnMgr.setText(m_config7Lbl4Val, wfmt(L"%i", m_cfg.getInt("FANART", "delay_after_animation", 200)));
	}
	else if(curPage == 9)
	{
		m_btnMgr.setText(m_config7Lbl1, _t("cfg709", L"Rumble"));
		m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "rumble") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_config7Lbl2, _t("cfg710", L"Wiimote gestures"));
		m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("GENERAL", "wiimote_gestures") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_config7Lbl3, _t("cfg711", L"Screensaver"));
		m_btnMgr.setText(m_config7Btn3, !m_cfg.getBool("GENERAL", "screensaver_disabled") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_config7Lbl4, _t("cfg712", L"Screensaver idle seconds"));
		m_btnMgr.setText(m_config7Lbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "screensaver_idle_seconds", 60)));// inc by 30
	}
	else if(curPage == 10)
	{
		m_btnMgr.setText(m_config7Lbl1, _t("cfg713", L"Use HQ covers"));
		m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "cover_use_hq") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl2, _t("cfg714", L"Display music title"));
		m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("GENERAL", "display_music_info") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl3, _t("cfg715", L"Randomize music"));
		m_btnMgr.setText(m_config7Btn3, m_cfg.getBool("GENERAL", "randomize_music") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl4, _t("cfg716", L"Music fade rate"));
		m_btnMgr.setText(m_config7Lbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "music_fade_rate", 8)));
	}
	else if(curPage == 11)
	{
		m_btnMgr.setText(m_config7Lbl1, _t("cfg717", L"Random game boot or select"));
		m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "random_select") ? _t("select", L"Select") : _t("boot", L"Boot"));
		m_btnMgr.setText(m_config7Lbl2, _t("cfg718", L"Source Menu on start"));
		m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("GENERAL", "source_on_start") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl3, _t("cfg720", L"Play GC banner sound"));
		m_btnMgr.setText(m_config7Btn3, m_cfg.getBool(GC_DOMAIN, "play_banner_sound") ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl4, _t("cfg721", L"Play GC default sound"));
		m_btnMgr.setText(m_config7Btn4, m_cfg.getBool(GC_DOMAIN, "play_default_sound") ?  _t("yes", L"Yes") : _t("no", L"No"));
	}
	else if(curPage == 12)
	{
		m_btnMgr.setText(m_config7Lbl1, _t("cfg722", L"Homebrew settings"));
		m_btnMgr.setText(m_config7Btn1, _t("cfg14", L"Set"));
		m_btnMgr.setText(m_config7Lbl2, _t("cfg723", L"Source menu settings"));
		m_btnMgr.setText(m_config7Btn2, _t("cfg14", L"Set"));
		m_btnMgr.setText(m_config7Lbl3, _t("cfg724", L"Lock coverflow layouts"));
		m_btnMgr.setText(m_config7Btn3, m_cfg.getBool("general", "cf_locked") ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl4, _t("cfg725", L"Shutdown to idle standby"));
		m_btnMgr.setText(m_config7Btn4, m_cfg.getBool("general", "idle_standby", false) ? _t("yes", L"Yes") : _t("no", L"No"));
	}
	else // page 13
	{
		m_btnMgr.setText(m_config7Lbl1, _t("cfgg49", L"480p Pixel Patch"));
		m_btnMgr.setText(m_config7Btn1, m_cfg.getBool(WII_DOMAIN, "fix480p", false) ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_config7Lbl2, _t("cfg726", L"Covers Box Mode"));
		m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("general", "box_mode", false) ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_config7Lbl3, _t("cfg727", L"Use Plugin Database Titles"));
		m_btnMgr.setText(m_config7Btn3, m_cfg.getBool(PLUGIN_DOMAIN, "database_titles", true) ?  _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.setText(m_config7Lbl4, _t("cfg728", L"Upsample music to 48khz"));
		m_btnMgr.setText(m_config7Btn4, m_cfg.getBool("general", "resample_to_48khz", true) ?  _t("yes", L"Yes") : _t("no", L"No"));
	}
}

int CMenu::_config7(int curPage)
{
	bool rand_music = m_cfg.getBool("GENERAL", "randomize_music");
	bool hq_covers = m_cfg.getBool("GENERAL", "cover_use_hq");
	bool box_mode = m_cfg.getBool("GENERAL", "box_mode", true);
	bool db_titles = m_cfg.getBool(PLUGIN_DOMAIN, "database_titles", true);
	int change = CONFIG_PAGE_NO_CHANGE;
	_showConfig7(curPage);
	
	while(!m_exit)
	{
		change = _configCommon();
		if(change != CONFIG_PAGE_NO_CHANGE)
			break;
		if(BTN_A_PRESSED)
		{
			if(curPage == 7)
			{
				if(m_btnMgr.selected(m_config7Btn1))
				{
					m_cfg.setBool("GENERAL", "hideviews", !m_cfg.getBool("GENERAL", "hideviews"));
					m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "hideviews") ? _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_config7Btn2))
				{
					m_cfg.setBool(GC_DOMAIN, "disable", !m_cfg.getBool(GC_DOMAIN, "disable"));
					m_btnMgr.setText(m_config7Btn2, m_cfg.getBool(GC_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_config7Btn3))
				{
					m_cfg.setBool(CHANNEL_DOMAIN, "disable", !m_cfg.getBool(CHANNEL_DOMAIN, "disable"));
					m_btnMgr.setText(m_config7Btn3, m_cfg.getBool(CHANNEL_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_config7Btn4))
				{
					m_cfg.setBool(PLUGIN_DOMAIN, "disable", !m_cfg.getBool(PLUGIN_DOMAIN, "disable"));
					m_btnMgr.setText(m_config7Btn4, m_cfg.getBool(PLUGIN_DOMAIN, "disable") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
			}
			if(curPage == 8)
			{
				if(m_btnMgr.selected(m_config7Btn1))
				{
					m_cfg.setBool("GENERAL", "banner_in_settings", !m_cfg.getBool("GENERAL", "banner_in_settings"));
					m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "banner_in_settings") ? _t("yes", L"Yes") : _t("no", L"No"));
					m_bnr_settings = m_cfg.getBool("GENERAL", "banner_in_settings");
				}
				else if(m_btnMgr.selected(m_config7Btn2))
				{
					m_cfg.setBool("FANART", "enable_fanart", !m_cfg.getBool("FANART", "enable_fanart"));
					m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("FANART", "enable_fanart") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_config7Btn3))
				{
					m_cfg.setBool("FANART", "show_cover_after_animation", !m_cfg.getBool("FANART", "show_cover_after_animation"));
					m_btnMgr.setText(m_config7Btn3, m_cfg.getBool("FANART", "show_cover_after_animation") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_config7Btn4P) || m_btnMgr.selected(m_config7Btn4M))
				{
					s8 direction = m_btnMgr.selected(m_config7Btn4P) ? 1 : -1;
					int val = m_cfg.getInt("FANART", "delay_after_animation") + direction;
					if(val >= 0 && val < 360)
						m_cfg.setInt("FANART", "delay_after_animation", val);
					m_btnMgr.setText(m_config7Lbl4Val, wfmt(L"%i", m_cfg.getInt("FANART", "delay_after_animation")));
				}
			}
			if(curPage == 9)
			{
				if(m_btnMgr.selected(m_config7Btn1))
				{
					m_cfg.setBool("GENERAL", "rumble", !m_cfg.getBool("GENERAL", "rumble"));
					m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "rumble") ? _t("on", L"On") : _t("off", L"Off"));
					m_btnMgr.setRumble(m_cfg.getBool("GENERAL", "rumble"));
				}
				else if(m_btnMgr.selected(m_config7Btn2))
				{
					m_cfg.setBool("GENERAL", "wiimote_gestures", !m_cfg.getBool("GENERAL", "wiimote_gestures"));
					m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("GENERAL", "wiimote_gestures") ?  _t("on", L"On") : _t("off", L"Off"));
					enable_wmote_roll = m_cfg.getBool("GENERAL", "wiimote_gestures");
				}
				else if(m_btnMgr.selected(m_config7Btn3))
				{
					m_cfg.setBool("GENERAL", "screensaver_disabled", !m_cfg.getBool("GENERAL", "screensaver_disabled"));
					m_btnMgr.setText(m_config7Btn3, !m_cfg.getBool("GENERAL", "screensaver_disabled") ?  _t("on", L"On") : _t("off", L"Off"));
				}
				else if(m_btnMgr.selected(m_config7Btn4P) || m_btnMgr.selected(m_config7Btn4M))
				{
					s8 direction = m_btnMgr.selected(m_config7Btn4P) ? 30 : -30;
					int val = m_cfg.getInt("GENERAL", "screensaver_idle_seconds") + direction;
					if(val >= 0 && val < 361)
						m_cfg.setInt("GENERAL", "screensaver_idle_seconds", val);
					m_btnMgr.setText(m_config7Lbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "screensaver_idle_seconds")));
				}
			}
			if(curPage == 10)
			{
				if(m_btnMgr.selected(m_config7Btn1))
				{
					m_cfg.setBool("GENERAL", "cover_use_hq", !m_cfg.getBool("GENERAL", "cover_use_hq"));
					m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "cover_use_hq") ? _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_config7Btn2))
				{
					m_cfg.setBool("GENERAL", "display_music_info", !m_cfg.getBool("GENERAL", "display_music_info"));
					m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("GENERAL", "display_music_info") ?  _t("yes", L"Yes") : _t("no", L"No"));
					m_music_info = m_cfg.getBool("GENERAL", "display_music_info");
				}
				else if(m_btnMgr.selected(m_config7Btn3))
				{
					m_cfg.setBool("GENERAL", "randomize_music", !m_cfg.getBool("GENERAL", "randomize_music"));
					m_btnMgr.setText(m_config7Btn3, m_cfg.getBool("GENERAL", "randomize_music") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_config7Btn4P) || m_btnMgr.selected(m_config7Btn4M))
				{
					s8 direction = m_btnMgr.selected(m_config7Btn4P) ? 1 : -1;
					int val = m_cfg.getInt("GENERAL", "music_fade_rate") + direction;
					if(val >= 0 && val < 30)
						m_cfg.setInt("GENERAL", "music_fade_rate", val);
					m_btnMgr.setText(m_config7Lbl4Val, wfmt(L"%i", m_cfg.getInt("GENERAL", "music_fade_rate")));
					MusicPlayer.SetFadeRate(m_cfg.getInt("GENERAL", "music_fade_rate"));
				}
			}
			if(curPage == 11)
			{
				if(m_btnMgr.selected(m_config7Btn1))
				{
					m_cfg.setBool("GENERAL", "random_select", !m_cfg.getBool("GENERAL", "random_select"));
					m_btnMgr.setText(m_config7Btn1, m_cfg.getBool("GENERAL", "random_select") ? _t("select", L"Select") : _t("boot", L"Boot"));
				}
				else if(m_btnMgr.selected(m_config7Btn2))
				{
					m_cfg.setBool("GENERAL", "source_on_start", !m_cfg.getBool("GENERAL", "source_on_start"));
					m_btnMgr.setText(m_config7Btn2, m_cfg.getBool("GENERAL", "source_on_start") ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
				else if(m_btnMgr.selected(m_config7Btn3))
				{
					m_cfg.setBool(GC_DOMAIN, "play_banner_sound", !m_cfg.getBool(GC_DOMAIN, "play_banner_sound"));
					m_btnMgr.setText(m_config7Btn3, m_cfg.getBool(GC_DOMAIN, "play_banner_sound") ?  _t("yes", L"Yes") : _t("no", L"No"));
					m_gc_play_banner_sound = m_cfg.getBool(GC_DOMAIN, "play_banner_sound", true);
				}
				else if(m_btnMgr.selected(m_config7Btn4))
				{
					m_cfg.setBool(GC_DOMAIN, "play_default_sound", !m_cfg.getBool(GC_DOMAIN, "play_default_sound"));
					m_btnMgr.setText(m_config7Btn4, m_cfg.getBool(GC_DOMAIN, "play_default_sound") ?  _t("yes", L"Yes") : _t("no", L"No"));
					m_gc_play_default_sound = m_cfg.getBool(GC_DOMAIN, "play_default_sound", true);
				}
			}
			if(curPage == 12)
			{
				if(m_btnMgr.selected(m_config7Btn1))
				{
					_hideConfig7();
					_CfgHB();
					_showConfig7(12);
				}
				else if(m_btnMgr.selected(m_config7Btn2))
				{
					_hideConfig7();
					_CfgSrc();
					_showConfig7(12);
				}
				else if(m_btnMgr.selected(m_config7Btn3))
				{
					bool val = !m_cfg.getBool("general", "cf_locked");
					m_cfg.setBool("general", "cf_locked", val);
					m_btnMgr.setText(m_config7Btn3, val ?  _t("yes", L"Yes") : _t("no", L"No"));
					CFLocked = val;
				}
				else if(m_btnMgr.selected(m_config7Btn4))
				{
					bool val = !m_cfg.getBool("general", "idle_standby");
					m_cfg.setBool("general", "idle_standby", val);
					m_btnMgr.setText(m_config7Btn4, val ?  _t("yes", L"Yes") : _t("no", L"No"));
				}
			}
			if(curPage == 13)
			{
				if(m_btnMgr.selected(m_config7Btn1))
				{
					bool val = !m_cfg.getBool(WII_DOMAIN, "fix480p");
					m_cfg.setBool(WII_DOMAIN, "fix480p", val);
					m_btnMgr.setText(m_config7Btn1, val ? _t("on", L"On") : _t("off", L"Off"));
				}
				if(m_btnMgr.selected(m_config7Btn2))
				{
					bool val = !m_cfg.getBool("general", "box_mode");
					m_cfg.setBool("general", "box_mode", val);
					m_btnMgr.setText(m_config7Btn2, val ? _t("on", L"On") : _t("off", L"Off"));
				}
				if(m_btnMgr.selected(m_config7Btn3))
				{
					bool val = !m_cfg.getBool(PLUGIN_DOMAIN, "database_titles");
					m_cfg.setBool(PLUGIN_DOMAIN, "database_titles", val);
					m_btnMgr.setText(m_config7Btn3, val ? _t("yes", L"Yes") : _t("no", L"No"));
				}
				if(m_btnMgr.selected(m_config7Btn4))
				{
					bool val = !m_cfg.getBool("general", "resample_to_48khz");
					m_cfg.setBool("general", "resample_to_48khz", val);
					m_btnMgr.setText(m_config7Btn4, val ? _t("yes", L"Yes") : _t("no", L"No"));
					MusicPlayer.SetResampleSetting(val);
					MusicPlayer.Stop();
					MusicPlayer.LoadCurrentFile();
				}
			}
		}
	}
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
	_hideConfig7();
	return change;
}

void CMenu::_initConfig7Menu()
{
	_addUserLabels(m_config7LblUser, ARRAY_SIZE(m_config7LblUser), "CONFIG7");
	m_config7Bg = _texture("CONFIG7/BG", "texture", theme.bg, false);
	
	m_config7Lbl1 = _addLabel("CONFIG7/LINE1", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config7Btn1 = _addButton("CONFIG7/LINE1_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);
	m_config7Lbl2 = _addLabel("CONFIG7/LINE2", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config7Btn2 = _addButton("CONFIG7/LINE2_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	m_config7Lbl3 = _addLabel("CONFIG7/LINE3", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config7Btn3 = _addButton("CONFIG7/LINE3_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	m_config7Lbl4 = _addLabel("CONFIG7/LINE4", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config7Btn4 = _addButton("CONFIG7/LINE4_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);
	
	m_config7Lbl4Val = _addLabel("CONFIG7/LINE4_VAL", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config7Btn4M = _addPicButton("CONFIG7/LINE4_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_config7Btn4P = _addPicButton("CONFIG7/LINE4_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);

	_setHideAnim(m_config7Lbl1, "CONFIG7/LINE1", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config7Btn1, "CONFIG7/LINE1_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config7Lbl2, "CONFIG7/LINE2", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config7Btn2, "CONFIG7/LINE2_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config7Lbl3, "CONFIG7/LINE3", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config7Btn3, "CONFIG7/LINE3_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config7Lbl4, "CONFIG7/LINE4", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config7Btn4, "CONFIG7/LINE4_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_config7Lbl4Val, "CONFIG7/LINE4_VAL", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config7Btn4M, "CONFIG7/LINE4_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config7Btn4P, "CONFIG7/LINE4_PLUS", -50, 0, 1.f, 0.f);

	_hideConfig7(true);
}

/*
void CMenu::_textConfig7(void)
{
}
*/
