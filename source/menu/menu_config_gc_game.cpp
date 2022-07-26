
#include "menu.hpp"
#include "types.h"

u8 m_configGCGameMaxPgs = 5;
u8 m_configGCGamePage = 0;
u8 GCLoader = 0;
int videoScale, videoOffset;
static const dir_discHdr *GameHdr;

const CMenu::SOption CMenu::_GlobalGCvideoModes[6] = {
	{ "vidgame", L"Game" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "DMLmpal", L"MPAL" },
	{ "vidprog", L"Progressive" },
};

const CMenu::SOption CMenu::_GCvideoModes[7] = {
	{ "viddef", L"Default" },
	{ "vidgame", L"Game" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "DMLmpal", L"MPAL" },
	{ "vidprog", L"Progressive" },
};

const CMenu::SOption CMenu::_GlobalGClanguages[7] = {
	{ "lngsys", L"System" },
	{ "lngeng", L"English" },
	{ "lngger", L"German" },
	{ "lngfre", L"French" },
	{ "lngspa", L"Spanish" },
	{ "lngita", L"Italian" },
	{ "lngdut", L"Dutch" }
};

const CMenu::SOption CMenu::_GClanguages[8] = {
	{ "lngdef", L"Default" },
	{ "lngsys", L"System" },
	{ "lngeng", L"English" },
	{ "lngger", L"German" },
	{ "lngfre", L"French" },
	{ "lngspa", L"Spanish" },
	{ "lngita", L"Italian" },
	{ "lngdut", L"Dutch" }
};

const CMenu::SOption CMenu::_NinEmuCard[5] = {
	{ "NinMCDef", L"Default" },
	{ "NinMCOff", L"Disabled" },
	{ "NinMCon", L"Enabled" },
	{ "NinMCMulti", L"Multi Saves" },
	{ "NinMCdebug", L"Debug" },
};

const CMenu::SOption CMenu::_GlobalGCLoaders[2] = {
	{ "GC_Devo", L"Devolution" },
	{ "GC_Nindnt", L"Nintendont" },
};

const CMenu::SOption CMenu::_GCLoader[3] = {
	{ "GC_Def", L"Default" },
	{ "GC_Devo", L"Devolution" },
	{ "GC_Nindnt", L"Nintendont" },
};

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfigGCGame(bool instant)
{
	m_btnMgr.hide(m_gameSettingsLblTitle, instant);
	m_btnMgr.hide(m_gameSettingsBtnBack, instant);
	m_btnMgr.hide(m_gameSettingsLblPage, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if(m_gameSettingsLblUser[i] != -1)
			m_btnMgr.hide(m_gameSettingsLblUser[i], instant);
	_hideConfigButtons(instant);
}

void CMenu::_showConfigGCGame()
{
	u8 i;
	_setBg(m_gameSettingsBg, m_gameSettingsBg);
	
	string id(GameHdr->id);
	wstringEx title(_t("cfgg1", L"Settings"));
	title.append(wfmt(L" [%s]", id));
	m_btnMgr.setText(m_gameSettingsLblTitle, title);
	m_btnMgr.show(m_gameSettingsLblTitle);
	
	m_btnMgr.show(m_gameSettingsBtnBack);
	for(i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if(m_gameSettingsLblUser[i] != -1)
			m_btnMgr.show(m_gameSettingsLblUser[i]);
	
	_hideConfigButtons();
	
	GCLoader = min(m_gcfg2.getUInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
	GCLoader = (GCLoader == 0) ? min(m_cfg.getUInt(GC_DOMAIN, "default_loader", 1), ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1u) : GCLoader-1;
	if(GCLoader == DEVOLUTION)
		m_configGCGameMaxPgs = 2;
	else
		m_configGCGameMaxPgs = 6;
	
	m_btnMgr.setText(m_gameSettingsLblPage, wfmt(L"%i / %i", m_configGCGamePage, m_configGCGameMaxPgs));
	m_btnMgr.show(m_gameSettingsLblPage);
	m_btnMgr.show(m_gameSettingsBtnPageM);
	m_btnMgr.show(m_gameSettingsBtnPageP);
	
	if(m_configGCGamePage == 1)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg58", L"Adult only"));
		m_btnMgr.setText(m_configLbl2, _t("cfgg35", L"GameCube Loader"));
		m_btnMgr.setText(m_configLbl3, _t("cfgg2", L"Video mode"));
		m_btnMgr.setText(m_configLbl4, _t("cfgg3", L"Language"));
		
		m_btnMgr.setText(m_configBtn1, m_gcfg1.getBool("ADULTONLY", id, false) ? _t("yes", L"Yes") : _t("no", L"No"));
		
		i = min(m_gcfg2.getUInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
		m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_GCLoader[i].id, CMenu::_GCLoader[i].text));
		
		i = min(m_gcfg2.getUInt(id, "language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
		m_btnMgr.setText(m_configLbl3Val, _t(CMenu::_GClanguages[i].id, CMenu::_GClanguages[i].text));
		
		i = min(m_gcfg2.getUInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_GCvideoModes) - 1u);
		m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_GCvideoModes[i].id, CMenu::_GCvideoModes[i].text));
		
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configBtn1);
		
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configBtn2M);

		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configLbl3Val);
		m_btnMgr.show(m_configBtn3P);
		m_btnMgr.show(m_configBtn3M);

		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4P);
		m_btnMgr.show(m_configBtn4M);
	}
	else if(m_configGCGamePage == 2)
	{
		if(GCLoader == DEVOLUTION)
		{
			m_btnMgr.setText(m_configLbl1, _t("cfgg38", L"Activity LED"));
			m_btnMgr.setText(m_configLbl2, _t("cfgg47", L"Emulated MemCard"));
			m_btnMgr.setText(m_configLbl3, _t("cfgg36", L"Widescreen Patch"));
			m_btnMgr.setText(m_configLbl4, _t("cfgg40", L"Manage Cover and Banner"));
			
			m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "led", 0)));
			
			m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "devo_memcard_emu", 2)));
			
			m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "widescreen", 0)));
			
			m_btnMgr.setText(m_configBtn4, _t("cfgg41", L"Manage"));
			
			m_btnMgr.show(m_configLbl1);
			m_btnMgr.show(m_configBtn1);

			m_btnMgr.show(m_configLbl2);
			m_btnMgr.show(m_configBtn2);

			m_btnMgr.show(m_configLbl3);
			m_btnMgr.show(m_configBtn3);

			m_btnMgr.show(m_configLbl4);
			m_btnMgr.show(m_configBtn4);
		}
		else  //nintendont
		{
			m_btnMgr.setText(m_configLbl1, _t("cfgg22", L"Debugger"));
			m_btnMgr.setText(m_configLbl2, _t("cfgg44", L"Video Deflicker"));
			m_btnMgr.setText(m_configLbl3, _t("cfgg5", L"Ocarina"));
			m_btnMgr.setText(m_configLbl4, _t("cfgg15", L"Cheat Codes"));
			
			i = min(m_gcfg2.getUInt(id, "debugger", 0), ARRAY_SIZE(CMenu::_debugger) - 1u);
			m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_debugger[i].id, CMenu::_debugger[i].text));
			
			m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "deflicker", 0)));
			
			m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "cheat", 0)));
			
			m_btnMgr.setText(m_configBtn4, _t("cfgg16", L"Select"));

			m_btnMgr.show(m_configLbl1);
			m_btnMgr.show(m_configLbl1Val);
			m_btnMgr.show(m_configBtn1P);
			m_btnMgr.show(m_configBtn1M);
			
			m_btnMgr.show(m_configLbl2);
			m_btnMgr.show(m_configBtn2);
			
			m_btnMgr.show(m_configLbl3);
			m_btnMgr.show(m_configBtn3);
		
			m_btnMgr.show(m_configLbl4);
			m_btnMgr.show(m_configBtn4);
		}
	}
	else if(m_configGCGamePage == 3)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg46", L"WiiU Widescreen"));
		m_btnMgr.setText(m_configLbl2, _t("cfgg47", L"Emulated MemCard"));
		m_btnMgr.setText(m_configLbl3, _t("cfgg36", L"Widescreen Patch"));
		m_btnMgr.setText(m_configLbl4, _t("cfgg40", L"Manage Cover and Banner"));
		
		m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "wiiu_widescreen", 0)));
		
		i = min(m_gcfg2.getUInt(id, "emu_memcard", 0), ARRAY_SIZE(CMenu::_NinEmuCard) - 1u);
		m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_NinEmuCard[i].id, CMenu::_NinEmuCard[i].text));
		
		m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "widescreen", 0)));
		
		m_btnMgr.setText(m_configBtn4, _t("cfgg41", L"Manage"));
		
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configBtn1);

		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configBtn2M);

		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configBtn3);

		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configBtn4);
	}
	else if(m_configGCGamePage == 4)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg52", L"Wiimote CC Rumble"));
		m_btnMgr.setText(m_configLbl2, _t("cfgg43", L"Native Control"));
		m_btnMgr.setText(m_configLbl3, _t("cfgg48", L"Triforce Arcade Mode"));
		m_btnMgr.setText(m_configLbl4, _t("cfgg53", L"Skip IPL BIOS"));
	
		m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "cc_rumble", 2)));
		
		m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "native_ctl", 2)));
		
		m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "triforce_arcade", 0)));
		
		m_btnMgr.setText(m_configBtn4, _optBoolToString(m_gcfg2.getOptBool(id, "skip_ipl", 0)));
		
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configBtn1);
		
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configBtn2);

		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configBtn3);

		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configBtn4);
	}
	else if(m_configGCGamePage == 5)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg38", L"Activity LED"));
		m_btnMgr.setText(m_configLbl2, _t("cfgg54", L"Video Width"));
		m_btnMgr.setText(m_configLbl3, _t("cfgg55", L"Video Position"));
		m_btnMgr.setText(m_configLbl4, _t("cfgg56", L"Patch PAL50"));
		
		m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "led", 0)));
		
		if(videoScale == 0)
			m_btnMgr.setText(m_configLbl2Val, _t("GC_Auto", L"Auto"));
		else if(videoScale == 127)
			m_btnMgr.setText(m_configLbl2Val, _t("def", L"Default"));
		else
			m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", max(40, min(120, videoScale)) + 600));
			
		if(videoOffset == 127)
			m_btnMgr.setText(m_configLbl3Val, _t("def", L"Default"));
		else
			m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", max(-20, min(20, videoOffset))));
		
		m_btnMgr.setText(m_configBtn4, _optBoolToString(m_gcfg2.getOptBool(id, "patch_pal50", 0)));
		
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configBtn1);
		
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configBtn2M);
		
		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configLbl3Val);
		m_btnMgr.show(m_configBtn3P);
		m_btnMgr.show(m_configBtn3M);
		
		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configBtn4);
	}
	else if(m_configGCGamePage == 6)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg59", L"BBA Emulation"));
		m_btnMgr.setText(m_configLbl2, _t("cfgg60", L"BBA Net Profile"));
	
		m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "bba_emu", 0)));
		
		u8 netprofile = m_gcfg2.getUInt(id, "net_profile", 0);
		if(netprofile == 0)
			m_btnMgr.setText(m_configLbl2Val, _t("GC_Auto", L"Auto"));
		else
			m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", netprofile));
		
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configBtn1);
	
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configBtn2M);
	}
}

void CMenu::_configGCGame(const dir_discHdr *hdr, bool disc)
{
	u8 i;
	m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
	GameHdr = hdr;// set for global use in other fuctions
	string id(GameHdr->id);
	
	videoScale = m_gcfg2.getInt(id, "nin_width", 127);
	videoOffset = m_gcfg2.getInt(id, "nin_pos", 127);
	
	m_configGCGamePage = 1;
	_showConfigGCGame();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(BTN_MINUS_PRESSED || BTN_LEFT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageM)))
		{
			if(m_configGCGamePage == 1)
				m_configGCGamePage = m_configGCGameMaxPgs;
			else
				--m_configGCGamePage;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_gameSettingsBtnPageM);
			_showConfigGCGame();
		}
		else if(BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageP)))
		{
			if(m_configGCGamePage == m_configGCGameMaxPgs)
				m_configGCGamePage = 1;
			else
				++m_configGCGamePage;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_gameSettingsBtnPageP);
			_showConfigGCGame();
		}
		else if(BTN_A_PRESSED)
		{
			if(m_configGCGamePage == 1)
			{
				//m_btnMgr.setText(m_configLbl1, _t("cfgg58", L"Adult only"));
				//m_btnMgr.setText(m_configLbl2, _t("cfgg35", L"GameCube Loader"));
				//m_btnMgr.setText(m_configLbl3, _t("cfgg2", L"Video mode"));
				//m_btnMgr.setText(m_configLbl4, _t("cfgg3", L"Language"));
				if(m_btnMgr.selected(m_configBtn1))
				{
					if(disc)
					{
						_error(_t("cfgg57", L"Not allowed for disc!"));
						_showConfigGCGame();
					}
					else
					{
						m_gcfg1.setBool("ADULTONLY", id, !m_gcfg1.getBool("ADULTONLY", id, false));
						m_btnMgr.setText(m_configBtn1, m_gcfg1.getBool("ADULTONLY", id, false) ? _t("yes", L"Yes") : _t("no", L"No"));
					}
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					m_gcfg2.setInt(id, "gc_loader", (int)loopNum(m_gcfg2.getUInt(id, "gc_loader", 0) + direction, ARRAY_SIZE(CMenu::_GCLoader)));
					i = min(m_gcfg2.getUInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
					m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_GCLoader[i].id, CMenu::_GCLoader[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn3P) ? 1 : -1;
					m_gcfg2.setInt(id, "language", (int)loopNum(m_gcfg2.getUInt(id, "language", 0) + direction, ARRAY_SIZE(CMenu::_GClanguages)));
					i = min(m_gcfg2.getUInt(id, "language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
					m_btnMgr.setText(m_configLbl3Val, _t(CMenu::_GClanguages[i].id, CMenu::_GClanguages[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					m_gcfg2.setInt(id, "video_mode", (int)loopNum(m_gcfg2.getUInt(id, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_GCvideoModes)));
					i = min(m_gcfg2.getUInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_GCvideoModes) - 1u);
					m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_GCvideoModes[i].id, CMenu::_GCvideoModes[i].text));
				}
			}
			else if(m_configGCGamePage == 2)
			{
				if(GCLoader == DEVOLUTION)
				{
					//m_btnMgr.setText(m_configLbl1, _t("cfgg38", L"Activity LED"));
					//m_btnMgr.setText(m_configLbl2, _t("cfgg47", L"Emulated MemCard"));
					//m_btnMgr.setText(m_configLbl3, _t("cfgg36", L"Widescreen Patch"));
					//m_btnMgr.setText(m_configLbl4, _t("cfgg40", L"Manage Cover and Banner"))
					if(m_btnMgr.selected(m_configBtn1))
					{
						m_gcfg2.setBool(id, "led", !m_gcfg2.getBool(id, "led", 0));
						m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "led", 0)));
					}
					else if(m_btnMgr.selected(m_configBtn2))
					{
						m_gcfg2.setBool(id, "devo_memcard_emu", !m_gcfg2.getBool(id, "devo_memcard_emu", 0));
						m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "devo_memcard_emu", 0)));
					}
					else if(m_btnMgr.selected(m_configBtn3))
					{
						m_gcfg2.setBool(id, "widescreen", !m_gcfg2.getBool(id, "widescreen", 0));
						m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "widescreen", 0)));
					}
					else if(m_btnMgr.selected(m_configBtn4))
					{
						if(disc)
							_error(_t("cfgg57", L"Not allowed for disc!"));
						else
						{
							CoverFlow.stopCoverLoader(true);
							_hideConfigGCGame();
							_CoverBanner();
							CoverFlow.startCoverLoader();
						}
						_showConfigGCGame();
					}
				}
				else
				{
					//m_btnMgr.setText(m_configLbl1, _t("cfgg22", L"Debugger"));
					//m_btnMgr.setText(m_configLbl2, _t("cfgg44", L"Video Deflicker"));
					//m_btnMgr.setText(m_configLbl3, _t("cfgg5", L"Ocarina"));
					//m_btnMgr.setText(m_configLbl4, _t("cfgg15", L"Cheat Codes"));
					if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
					{
						s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
						m_gcfg2.setInt(id, "debugger", (int)loopNum(m_gcfg2.getUInt(id, "debugger", 0) + direction, ARRAY_SIZE(CMenu::_debugger)));
						i = min(m_gcfg2.getUInt(id, "debugger", 0), ARRAY_SIZE(CMenu::_debugger) - 1u);
						m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_debugger[i].id, CMenu::_debugger[i].text));
					}
					else if(m_btnMgr.selected(m_configBtn2))
					{
						m_gcfg2.setBool(id, "deflicker", !m_gcfg2.getBool(id, "deflicker", 0));
						m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "deflicker", 0)));
					}	
					else if(m_btnMgr.selected(m_configBtn3))
					{
						m_gcfg2.setBool(id, "cheat", !m_gcfg2.getBool(id, "cheat", 0));
						m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "cheat", 0)));
					}
					else if(m_btnMgr.selected(m_configBtn4))
					{
						_hideConfigGCGame();
						_CheatSettings();
						_showConfigGCGame();
					}
				}
			}
			else if(m_configGCGamePage == 3)
			{
				//m_btnMgr.setText(m_configLbl1, _t("cfgg46", L"WiiU Widescreen"));
				//m_btnMgr.setText(m_configLbl2, _t("cfgg47", L"Emulated MemCard"));
				//m_btnMgr.setText(m_configLbl3, _t("cfgg36", L"Widescreen Patch"));
				//m_btnMgr.setText(m_configLbl4, _t("cfgg40", L"Manage Cover and Banner"))
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_gcfg2.setBool(id, "wiiu_widescreen", !m_gcfg2.getBool(id, "wiiu_widescreen", 0));
					m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "wiiu_widescreen", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					m_gcfg2.setInt(id, "emu_memcard", (int)loopNum(m_gcfg2.getUInt(id, "emu_memcard", 2) + direction, ARRAY_SIZE(CMenu::_NinEmuCard)));
					i = min(m_gcfg2.getUInt(id, "emu_memcard", 0), ARRAY_SIZE(CMenu::_NinEmuCard) - 1u);
					m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_NinEmuCard[i].id, CMenu::_NinEmuCard[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_gcfg2.setBool(id, "widescreen", !m_gcfg2.getBool(id, "widescreen", 0));
					m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "widescreen", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					if(disc)
						_error(_t("cfgg57", L"Not allowed for disc!"));
					else
					{
						CoverFlow.stopCoverLoader(true);
						_hideConfigGCGame();
						_CoverBanner();
						CoverFlow.startCoverLoader();
					}
					_showConfigGCGame();
				}
			}
			else if(m_configGCGamePage == 4)
			{
				//m_btnMgr.setText(m_configLbl1, _t("cfgg52", L"Wiimote CC Rumble"));
				//m_btnMgr.setText(m_configLbl2, _t("cfgg43", L"Native Control"));
				//m_btnMgr.setText(m_configLbl3, _t("cfgg48", L"Triforce Arcade Mode"));
				//m_btnMgr.setText(m_configLbl4, _t("cfgg53", L"Skip IPL BIOS"));
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_gcfg2.setOptBool(id, "cc_rumble", loopNum(m_gcfg2.getOptBool(id, "cc_rumble") + 1, 3));
					m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "cc_rumble", 2)));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					m_gcfg2.setOptBool(id, "native_ctl", loopNum(m_gcfg2.getOptBool(id, "native_ctl") + 1, 3));
					m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "native_ctl", 2)));
				}	
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_gcfg2.setBool(id, "triforce_arcade", !m_gcfg2.getBool(id, "triforce_arcade", 0));
					m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "triforce_arcade", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					m_gcfg2.setBool(id, "skip_ipl", !m_gcfg2.getBool(id, "skip_ipl", 0));
					m_btnMgr.setText(m_configBtn4, _optBoolToString(m_gcfg2.getOptBool(id, "skip_ipl", 0)));
				}
			}
			else if(m_configGCGamePage == 5)
			{
				//m_btnMgr.setText(m_configLbl1, _t("cfgg38", L"Activity LED"));
				//m_btnMgr.setText(m_configLbl2, _t("cfgg54", L"Video Width"));
				//m_btnMgr.setText(m_configLbl3, _t("cfgg55", L"Video Position"));
				//m_btnMgr.setText(m_configLbl4, _t("cfgg56", L"Patch PAL50"));
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_gcfg2.setBool(id, "led", !m_gcfg2.getBool(id, "led", 0));
					m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "led", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					if(m_btnMgr.selected(m_configBtn2P))
					{
						if(videoScale == 0)
							videoScale = 40;
						else if(videoScale == 127)
							videoScale = 0;
						else if(videoScale < 120)
							videoScale += 2;
					}
					else
					{
						if(videoScale == 40)
							videoScale = 0;
						else if(videoScale == 0)
							videoScale = 127;
						else if(videoScale > 40 && videoScale != 127)
							videoScale -= 2;
					}
					m_gcfg2.setInt(id, "nin_width", videoScale);
					if(videoScale == 0)
						m_btnMgr.setText(m_configLbl2Val, _t("GC_Auto", L"Auto"));
					else if(videoScale == 127)
						m_btnMgr.setText(m_configLbl2Val, _t("def", L"Default"));
					else
						m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", max(40, min(120, videoScale)) + 600));
				}
				else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
				{
					if(m_btnMgr.selected(m_configBtn3P))
					{
						if(videoOffset == 127)
							videoOffset = -20;
						else if(videoOffset < 20)
							videoOffset++;
					}
					else
					{
						if(videoOffset == -20)
							videoOffset = 127;
						else if(videoOffset > -20 && videoOffset != 127)
							videoOffset--;
					}
					m_gcfg2.setInt(id, "nin_pos", videoOffset);
					if(videoOffset == 127)
						m_btnMgr.setText(m_configLbl3Val, _t("def", L"Default"));
					else
						m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", max(-20, min(20, videoOffset))));
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					m_gcfg2.setBool(id, "patch_pal50", !m_gcfg2.getBool(id, "patch_pal50", 0));
					m_btnMgr.setText(m_configBtn4, _optBoolToString(m_gcfg2.getOptBool(id, "patch_pal50", 0)));
				}
			}
			else
			{
				//m_btnMgr.setText(m_configLbl1, _t("cfgg59", L"BBA Emulation"));
				//m_btnMgr.setText(m_configLbl2, _t("cfgg60", L"BBA Net Profile"));
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_gcfg2.setBool(id, "bba_emu", !m_gcfg2.getBool(id, "bba_emu", 0));
					m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "bba_emu", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					m_gcfg2.setInt(id, "net_profile", loopNum(m_gcfg2.getInt(id, "net_profile") + direction, 4));
					u8 netprofile = m_gcfg2.getUInt(id, "net_profile", 0);
					if(netprofile == 0)
						m_btnMgr.setText(m_configLbl2Val, _t("GC_Auto", L"Auto"));
					else
						m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", netprofile));
				}
			}
		}
	}
	if(!disc)
		m_gcfg2.save(true);// do not save changes for disc games
	_hideConfigGCGame();
}
