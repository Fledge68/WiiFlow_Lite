
#include "menu.hpp"
#include "types.h"
#include "loader/wbfs.h"
#include "libwbfs/wiidisc.h"

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

u8 m_configGameMaxPgs = 5;
u8 m_configGamePage = 0;
static const dir_discHdr *GameHdr;

const CMenu::SOption CMenu::_GlobalVideoModes[6] = {
	{ "vidgame", L"Game" },
	{ "vidsys", L"System" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "vidprog", L"Progressive" },
};

const CMenu::SOption CMenu::_VideoModes[7] = {
	{ "viddef", L"Default" },
	{ "vidgame", L"Game" },
	{ "vidsys", L"System" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "vidprog", L"Progressive" },
};

const CMenu::SOption CMenu::_GlobalDeflickerOptions[6] = {
	{ "df_norm", L"Normal" },
	{ "df_off", L"Off (Safe)" },
	{ "df_ext", L"Off (Extended)" },
	{ "df_low", L"On (Low)" },
	{ "df_med", L"On (Medium)" },
	{ "df_high", L"On (High)" },
};

const CMenu::SOption CMenu::_DeflickerOptions[7] = {
	{ "df_def", L"Default" },
	{ "df_norm", L"Normal" },
	{ "df_off", L"Off (Safe)" },
	{ "df_ext", L"Off (Extended)" },
	{ "df_low", L"On (Low)" },
	{ "df_med", L"On (Medium)" },
	{ "df_high", L"On (High)" },
};

const CMenu::SOption CMenu::_languages[11] = {
	{ "lngdef", L"Default" },// next should be console
	{ "lngjap", L"Japanese" },
	{ "lngeng", L"English" },
	{ "lngger", L"German" },
	{ "lngfre", L"French" },
	{ "lngspa", L"Spanish" },
	{ "lngita", L"Italian" },
	{ "lngdut", L"Dutch" },
	{ "lngsch", L"S. Chinese" },
	{ "lngtch", L"T. Chinese" },
	{ "lngkor", L"Korean" }
};

const CMenu::SOption CMenu::_ChannelsType[3] = {
	{ "ChanReal", L"Real NAND" },
	{ "ChanEmu", L"Emu NAND" },
	{ "ChanBoth", L"Both" },
};

const CMenu::SOption CMenu::_NandEmu[2] = {
	{ "NANDpart", L"Partial" },
	{ "NANDfull", L"Full" },
};

const CMenu::SOption CMenu::_GlobalSaveEmu[3] = {
	{ "SaveOffG", L"Off" },
	{ "SavePartG", L"Game save" },
	{ "SaveFullG", L"Full" },
};

const CMenu::SOption CMenu::_SaveEmu[4] = {
	{ "SaveDef", L"Default" },
	{ "SaveOff", L"Off" },
	{ "SavePart", L"Game save" },
	{ "SaveFull", L"Full" },
};

const CMenu::SOption CMenu::_AspectRatio[3] = {
	{ "aspectDef", L"Default" },
	{ "aspect43", L"Force 4:3" },
	{ "aspect169", L"Force 16:9" },
};

const CMenu::SOption CMenu::_WidescreenWiiu[3] = {
	{ "lngsys", L"System" },
	{ "aspect43", L"Force 4:3" },
	{ "aspect169", L"Force 16:9" },
};

const CMenu::SOption CMenu::_vidModePatch[4] = {
	{ "vmpnone", L"None" },
	{ "vmpnormal", L"Normal" },
	{ "vmpmore", L"More" },
	{ "vmpall", L"All" }
};

const CMenu::SOption CMenu::_hooktype[8] = {
	{ "hook_auto", L"AUTO" },
	{ "hooktype1", L"VBI" },
	{ "hooktype2", L"KPAD read" },
	{ "hooktype3", L"Joypad" },
	{ "hooktype4", L"GXDraw" },
	{ "hooktype5", L"GXFlush" },
	{ "hooktype6", L"OSSleepThread" },
	{ "hooktype7", L"AXNextFrame" },
};

const CMenu::SOption CMenu::_debugger[3] = {
	{ "disabled", L"Disabled" },
	{ "dbg_gecko", L"Gecko" },
	{ "dbgfwrite", L"OSReport" },
};

const CMenu::SOption CMenu::_privateServer[3] = {
	{ "off", L"Off" },
	{ "ps_nossl", L"No SSL only" },
	{ "ps_wiimmfi", L"wiimmfi" },
};

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

wstringEx CMenu::_optBoolToString(int i)
{
	switch (i)
	{
		case 0:
			return _t("off", L"Off");
		case 1:
			return _t("on", L"On");
		default:
			return _t("def", L"Default");
	}
}

void CMenu::_hideConfigGame(bool instant)
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

void CMenu::_showConfigGame()
{
	vector<string> custom_servers = stringToVector(m_cfg.getString("custom_servers", "servers"), '|');
	
	bool b;
	string adultDomain;
	if(GameHdr->type == TYPE_PLUGIN)
		adultDomain = "ADULTONLY_PLUGINS";
	else
		adultDomain = "ADULTONLY";

	u8 i;
	string id;
	char gameTitle[64];
	memset(gameTitle, 0 , 64);
	
	if(GameHdr->type == TYPE_HOMEBREW)
	{
		wcstombs(gameTitle, GameHdr->title, 63);
		id = gameTitle;
	}
	else if(GameHdr->type == TYPE_PLUGIN)
	{
		strncpy(m_plugin.PluginMagicWord, fmt("%08x", GameHdr->settings[0]), 8);
		// if game has an id from the plugin database we use the new method which uses platform name/id
		if(strcmp(GameHdr->id, "PLUGIN") != 0 && !m_platform.getString("PLUGINS", m_plugin.PluginMagicWord, "").empty())
			id =sfmt("%s/%s", m_platform.getString("PLUGINS", m_plugin.PluginMagicWord).c_str(), GameHdr->id);
		else // old pre 5.4.4 method which uses plugin magic/title of game
		{
			if(strrchr(GameHdr->path, '/') != NULL)
				wcstombs(gameTitle, GameHdr->title, 63);
			else
				memcpy(gameTitle, GameHdr->path, 63);// scummvm
			id = sfmt("%s/%s", m_plugin.PluginMagicWord, gameTitle);
		}
	}
	else
		id = GameHdr->id;
	
	_setBg(m_gameSettingsBg, m_gameSettingsBg);
	for(i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if(m_gameSettingsLblUser[i] != -1)
			m_btnMgr.show(m_gameSettingsLblUser[i]);
	m_btnMgr.show(m_gameSettingsBtnBack);

	wstringEx title(_t("cfgg1", L"Settings"));
	if(!NoGameID(GameHdr->type))
		title.append(wfmt(L" [%.6s]", id.c_str()));
	m_btnMgr.setText(m_gameSettingsLblTitle, title);
	m_btnMgr.show(m_gameSettingsLblTitle);
	
	if(GameHdr->type == TYPE_PLUGIN || GameHdr->type == TYPE_HOMEBREW)
	{
		m_configGameMaxPgs = 1;
		m_btnMgr.setText(m_configLbl1, _t("cfgg58", L"Adult only"));
		b = m_gcfg1.getBool(adultDomain, id, false);
		if(!b)
			m_gcfg1.remove(adultDomain, id);
		m_btnMgr.setText(m_configBtn1, b ? _t("yes", L"Yes") : _t("no", L"No"));
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configBtn1);
		
		m_btnMgr.setText(m_configLbl2, _t("cfgg62", L"Reload cached cover"));
		m_btnMgr.setText(m_configBtn2, _t("Yes", L"Yes"));
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configBtn2);
		return;
	}
	
	_hideConfigButtons();
	m_configGameMaxPgs = IsOnWiiU() ? 6 : 5;
	
	m_btnMgr.setText(m_gameSettingsLblPage, wfmt(L"%i / %i", m_configGamePage, m_configGameMaxPgs));
	m_btnMgr.show(m_gameSettingsLblPage);
	m_btnMgr.show(m_gameSettingsBtnPageM);
	m_btnMgr.show(m_gameSettingsBtnPageP);
	
	if(m_configGamePage == 1)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg58", L"Adult only"));
		b = m_gcfg1.getBool(adultDomain, id, false);
		if(!b)
			m_gcfg1.remove(adultDomain, id);
		m_btnMgr.setText(m_configBtn1, b ? _t("yes", L"Yes") : _t("no", L"No"));
		
		m_btnMgr.setText(m_configLbl2, _t("cfgg10", L"IOS"));
		i = m_gcfg2.getUInt(id, "ios", 0);
		if(i && _installed_cios.size() > 0)
		{
			CIOSItr itr = _installed_cios.find(i);
			i = (itr == _installed_cios.end()) ? 0 : itr->first;
		}
		else
			i = 0;

		if(i > 0)
			m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", i));
		else
			m_btnMgr.setText(m_configLbl2Val, L"AUTO");
			
		i = min(m_gcfg2.getUInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
		m_btnMgr.setText(m_configLbl3Val, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
		m_btnMgr.setText(m_configLbl3, _t("cfgg3", L"Language"));
		
		i = min(m_gcfg2.getUInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
		m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_VideoModes[i].id, CMenu::_VideoModes[i].text));
		m_btnMgr.setText(m_configLbl4, _t("cfgg2", L"Video mode"));

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
	else if(m_configGamePage == 2)
	{
		i = min(m_gcfg2.getUInt(id, "debugger", 0), ARRAY_SIZE(CMenu::_debugger) - 1u);
		m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_debugger[i].id, CMenu::_debugger[i].text));
		m_btnMgr.setText(m_configLbl1, _t("cfgg22", L"Debugger"));
	
		i = min(m_gcfg2.getUInt(id, "hooktype", 0), ARRAY_SIZE(CMenu::_hooktype) - 1u);
		m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_hooktype[i].id, CMenu::_hooktype[i].text));
		m_btnMgr.setText(m_configLbl2, _t("cfgg18", L"Hook Type"));
		
		m_btnMgr.setText(m_configLbl3, _t("cfgg5", L"Ocarina"));
		m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "cheat", 0)));
		
		m_btnMgr.setText(m_configLbl4, _t("cfgg15", L"Cheat Codes"));
		m_btnMgr.setText(m_configBtn4, _t("cfgg16", L"Select"));
		
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1P);
		m_btnMgr.show(m_configBtn1M);
	
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2M);
		m_btnMgr.show(m_configBtn2P);
		
		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configBtn3);
	
		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configBtn4);
	}
	else if(m_configGamePage == 3)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg14", L"Patch video modes"));
		i = min(m_gcfg2.getUInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
		m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_vidModePatch[i].id, CMenu::_vidModePatch[i].text));

		m_btnMgr.setText(m_configLbl2, _t("cfgg7", L"Vipatch"));
		m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "vipatch", 0)));

		m_btnMgr.setText(m_configLbl3, _t("cfgg4", L"Patch country strings"));
		m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "country_patch", 0)));
		
		m_btnMgr.setText(m_configLbl4, _t("cfgg27", L"Aspect Ratio"));
		i = min(m_gcfg2.getUInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u);
		m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_AspectRatio[i].id, CMenu::_AspectRatio[i].text));
		
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1M);
		m_btnMgr.show(m_configBtn1P);
		
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configBtn2);

		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configBtn3);

		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4M);
		m_btnMgr.show(m_configBtn4P);
	}
	else if(m_configGamePage == 4)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg40", L"Manage Cover and Banner"));
		m_btnMgr.setText(m_configBtn1, _t("cfgg41", L"Manage"));
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configBtn1);

		if(GameHdr->type == TYPE_CHANNEL || GameHdr->type == TYPE_EMUCHANNEL)
		{
			m_btnMgr.setText(m_configLbl2, _t("custom", L"Custom"));
			m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "custom", 0)));
			m_btnMgr.show(m_configLbl2);
			m_btnMgr.show(m_configBtn2);
			
			m_btnMgr.setText(m_configLbl3, _t("cfgg37", L"Boot Apploader"));
			m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "apploader", 0)));
			m_btnMgr.show(m_configLbl3);
			m_btnMgr.show(m_configBtn3);

			m_btnMgr.setText(m_configLbl4, _t("neek1", L"Launch Title with neek2o"));
			m_btnMgr.setText(m_configBtn4, _optBoolToString(m_gcfg2.getOptBool(id, "useneek", 0)));
			m_btnMgr.show(m_configLbl4);
			m_btnMgr.show(m_configBtn4);
		}
		else if(GameHdr->type == TYPE_WII_GAME)
		{
			m_btnMgr.setText(m_configLbl2, _t("cfgg24", L"NAND Emulation"));
			i = min(m_gcfg2.getUInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);
			m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_SaveEmu[i].id, CMenu::_SaveEmu[i].text));
			m_btnMgr.show(m_configLbl2);
			m_btnMgr.show(m_configLbl2Val);
			m_btnMgr.show(m_configBtn2M);
			m_btnMgr.show(m_configBtn2P);
			
			m_btnMgr.setText(m_configLbl3, _t("cfgg30", L"Extract Save from NAND"));
			m_btnMgr.setText(m_configBtn3, _t("cfgg31", L"Extract"));
			m_btnMgr.show(m_configLbl3);
			m_btnMgr.show(m_configBtn3);
			
			m_btnMgr.setText(m_configLbl4, _t("cfgg32", L"Flash Save to NAND"));
			m_btnMgr.setText(m_configBtn4, _t("cfgg33", L"Flash"));
			m_btnMgr.show(m_configLbl4);
			m_btnMgr.show(m_configBtn4);
		}
	}
	else if(m_configGamePage == 5)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg38", L"Activity LED"));
		m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "led", 0)));
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configBtn1);

		m_btnMgr.setText(m_configLbl2, _t("cfgg45", L"Private Server"));
		i = min(m_gcfg2.getUInt(id, "private_server", 0), ARRAY_SIZE(CMenu::_privateServer) - 1u);
		if(i < 3)
			m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_privateServer[i].id, CMenu::_privateServer[i].text));
		else
			m_btnMgr.setText(m_configLbl2Val, custom_servers[i - 3]);//wstringEx()
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configBtn2M);
		
		m_btnMgr.setText(m_configLbl3, _t("cfgg49", L"480p Pixel Patch"));
		m_btnMgr.setText(m_configLbl3Val, _optBoolToString(m_gcfg2.getOptBool(id, "fix480p", 2)));
		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configLbl3Val);
		m_btnMgr.show(m_configBtn3P);
		m_btnMgr.show(m_configBtn3M);

		m_btnMgr.setText(m_configLbl4, _t("cfgg61", L"Deflicker Filter"));
		i = min(m_gcfg2.getUInt(id, "deflicker_wii", 0), ARRAY_SIZE(CMenu::_DeflickerOptions) - 1u);
		m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_DeflickerOptions[i].id, CMenu::_DeflickerOptions[i].text));
		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configLbl4Val);
		m_btnMgr.show(m_configBtn4P);
		m_btnMgr.show(m_configBtn4M);
	}
	else if(m_configGamePage == 6)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgg46", L"WiiU Widescreen"));
		i = min(m_gcfg2.getUInt(id, "widescreen_wiiu", 0), ARRAY_SIZE(CMenu::_WidescreenWiiu) - 1u);
		m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_WidescreenWiiu[i].id, CMenu::_WidescreenWiiu[i].text));
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1P);
		m_btnMgr.show(m_configBtn1M);
	}
}

void CMenu::_configGame(const dir_discHdr *hdr, bool disc)
{
	vector<string> custom_servers = stringToVector(m_cfg.getString("custom_servers", "servers"), '|');
	m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
	GameHdr = hdr;// set for global use in other fuctions
	string adultDomain;
	if(hdr->type == TYPE_PLUGIN)
		adultDomain = "ADULTONLY_PLUGINS";
	else
		adultDomain = "ADULTONLY";
	u8 i;
	string id;
	//s8 direction;
	char gameTitle[64];
	memset(gameTitle, 0 , 64);
	
	if(GameHdr->type == TYPE_HOMEBREW)
	{
		wcstombs(gameTitle, GameHdr->title, 63);
		id = gameTitle;
	}
	else if(GameHdr->type == TYPE_PLUGIN)
	{
		strncpy(m_plugin.PluginMagicWord, fmt("%08x", GameHdr->settings[0]), 8);
		// if game has an id from the plugin database we use the new method which uses platform name/id
		if(strcmp(GameHdr->id, "PLUGIN") != 0 && !m_platform.getString("PLUGINS", m_plugin.PluginMagicWord, "").empty())
			id = sfmt("%s/%s", m_platform.getString("PLUGINS", m_plugin.PluginMagicWord).c_str(), GameHdr->id);
		else // old pre 5.4.4 method which uses plugin magic/title of game
		{
			if(strrchr(GameHdr->path, '/') != NULL)
				wcstombs(gameTitle, GameHdr->title, 63);
			else
				memcpy(gameTitle, GameHdr->path, 63);// scummvm
			id = sfmt("%s/%s", m_plugin.PluginMagicWord, gameTitle);
		}
	}
	else
		id = GameHdr->id;
	
	m_configGamePage = 1;
	_showConfigGame();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(m_configGameMaxPgs > 1 && (BTN_MINUS_PRESSED || BTN_LEFT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageM))))
		{
			if(m_configGamePage == 1)
				m_configGamePage = m_configGameMaxPgs;
			else
				--m_configGamePage;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_gameSettingsBtnPageM);
			_showConfigGame();
		}
		else if(m_configGameMaxPgs > 1 && (BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageP))))
		{
			if(m_configGamePage == m_configGameMaxPgs)
				m_configGamePage = 1;
			else
				++m_configGamePage;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_gameSettingsBtnPageP);
			_showConfigGame();
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_gameSettingsBtnBack))
				break;
			if(m_configGamePage == 1)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					if(disc)
					{
						_error(_t("cfgg57", L"Not allowed for disc!"));
						_showConfigGame();
					}
					else
					{
						m_gcfg1.setBool(adultDomain, id, !m_gcfg1.getBool(adultDomain, id, false));
						bool b = m_gcfg1.getBool(adultDomain, id);
						if(!b)
							m_gcfg1.remove(adultDomain, id);
						m_btnMgr.setText(m_configBtn1, b ? _t("yes", L"Yes") : _t("no", L"No"));
					}
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					_hideConfigGame(true);
					m_btnMgr.setText(m_downloadLblDialog, L"");
					m_btnMgr.show(m_downloadLblDialog);
					_start_pThread();
					CoverFlow.stopCoverLoader(true);
					
					bool smallBox = false;
					if(m_current_view == COVERFLOW_HOMEBREW && !m_sourceflow)
						smallBox = m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", false);
					//else if(m_sourceflow)
					//	smallBox = m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox", false);
					else if(m_current_view == COVERFLOW_PLUGIN && !m_sourceflow)
					{
						if(enabledPluginsCount == 1 && m_plugin.GetEnabledStatus(HB_PMAGIC))
							smallBox = m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", false);
					}
					
					//delete cached wfc
					const char *gameNameOrID = CoverFlow.getFilenameId(GameHdr);
					if(GameHdr->type == TYPE_PLUGIN)
						fsop_deleteFile(fmt("%s/%s/%s.wfc", m_cacheDir.c_str(), m_plugin.GetCoverFolderName(GameHdr->settings[0]), gameNameOrID));
					else
						fsop_deleteFile(fmt("%s/homebrew/%s.wfc", m_cacheDir.c_str(), gameNameOrID));
					
					//cache new cover
					m_thrdMessage = wfmt(_t("cfgg63", L"Converting cover please wait..."));
					m_thrdMessageAdded = true;
					_cacheCover(GameHdr, smallBox);
					
					_stop_pThread();
					m_btnMgr.setText(m_downloadLblDialog, _t("dlmsg14", L"Done."));
					u8 pause = 150;
					do
					{
						_mainLoopCommon();
						pause--;
						if(pause == 0)
							m_btnMgr.hide(m_downloadLblDialog);
					}while(!m_exit && pause > 0);
					CoverFlow.startCoverLoader();
					_showConfigGame();
				}
				else if(m_btnMgr.selected(m_configBtn2M) || m_btnMgr.selected(m_configBtn2P))
				{
					if(_installed_cios.size() > 0)
					{
						bool direction = m_btnMgr.selected(m_configBtn2P);
						CIOSItr itr = _installed_cios.find((u32)m_gcfg2.getInt(id, "ios", 0));
						if(direction && itr == _installed_cios.end())
							itr = _installed_cios.begin();
						else if(!direction && itr == _installed_cios.begin())
							itr = _installed_cios.end();
						else if(direction)
							itr++;

						if(!direction)
							itr--;

						i = itr->first;
							m_gcfg2.setInt(id, "ios", i);
						if(i > 0)
							m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", i));
						else
							m_btnMgr.setText(m_configLbl2Val, L"AUTO");
					}
				}
				else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn3P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "language", 0) + direction, ARRAY_SIZE(CMenu::_languages));
					m_gcfg2.setInt(id, "language", i);
					m_btnMgr.setText(m_configLbl3Val, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_VideoModes));
					m_gcfg2.setInt(id, "video_mode", i);
					m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_VideoModes[i].id, CMenu::_VideoModes[i].text));
				}
			}
			else if(m_configGamePage == 2)
			{	
				if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "debugger", 0) + direction, ARRAY_SIZE(CMenu::_debugger));
					m_gcfg2.setInt(id, "debugger", i);
					m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_debugger[i].id, CMenu::_debugger[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "hooktype", 1) + direction, ARRAY_SIZE(CMenu::_hooktype));
					m_gcfg2.setInt(id, "hooktype", i);
					m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_hooktype[i].id, CMenu::_hooktype[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_gcfg2.setBool(id, "cheat", !m_gcfg2.getBool(id, "cheat", 0));
					m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "cheat", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					_hideConfigGame();
					_CheatSettings();
					_showConfigGame();
				}
			}
			else if(m_configGamePage == 3)
			{	
				if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "patch_video_modes", 0) + direction, ARRAY_SIZE(CMenu::_vidModePatch));
					m_gcfg2.setInt(id, "patch_video_modes", i);
					m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_vidModePatch[i].id, CMenu::_vidModePatch[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					m_gcfg2.setBool(id, "vipatch", !m_gcfg2.getBool(id, "vipatch", 0));
					m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "vipatch", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn3))
				{
					m_gcfg2.setBool(id, "country_patch", !m_gcfg2.getBool(id, "country_patch", 0));
					m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "country_patch", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "aspect_ratio", 0) + direction, ARRAY_SIZE(CMenu::_AspectRatio));
					m_gcfg2.setInt(id, "aspect_ratio", i);
					m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_AspectRatio[i].id, CMenu::_AspectRatio[i].text));
				}
			}
			else if(m_configGamePage == 4)
			{	
				if(m_btnMgr.selected(m_configBtn1))
				{
					if(disc)
						_error(_t("cfgg57", L"Not allowed for disc!"));
					else
					{
						CoverFlow.stopCoverLoader(true);
						_hideConfigGame();
						_CoverBanner();
						CoverFlow.startCoverLoader();
					}
					_showConfigGame();
				}
				if(GameHdr->type == TYPE_CHANNEL || GameHdr->type == TYPE_EMUCHANNEL)
				{
					if(m_btnMgr.selected(m_configBtn2))
					{
						m_gcfg2.setBool(id, "custom", !m_gcfg2.getBool(id, "custom", 0));
						m_btnMgr.setText(m_configBtn2, _optBoolToString(m_gcfg2.getOptBool(id, "custom", 0)));
					}
					else if(m_btnMgr.selected(m_configBtn3))
					{
						m_gcfg2.setBool(id, "apploader", !m_gcfg2.getBool(id, "apploader", 0));
						m_btnMgr.setText(m_configBtn3, _optBoolToString(m_gcfg2.getOptBool(id, "apploader", 0)));
					}
					else if(m_btnMgr.selected(m_configBtn4))
					{
						m_gcfg2.setBool(id, "useneek", !m_gcfg2.getBool(id, "useneek", 0));
						m_btnMgr.setText(m_configBtn4, _optBoolToString(m_gcfg2.getOptBool(id, "useneek", 0)));
					}
				}
				else if(GameHdr->type == TYPE_WII_GAME)
				{	
					if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
					{
						s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
						i = loopNum(m_gcfg2.getUInt(id, "emulate_save", 0) + direction, ARRAY_SIZE(CMenu::_SaveEmu));
						m_gcfg2.setInt(id, "emulate_save", i);
						m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_SaveEmu[i].id, CMenu::_SaveEmu[i].text));
					}
					else if(m_btnMgr.selected(m_configBtn3))
					{
						_hideConfigGame();
						m_forceext = true;
						if(!_ExtractGameSave(id))
							_error(_t("cfgg50", L"No save to extract!"));
						_showConfigGame();
					}
					else if(m_btnMgr.selected(m_configBtn4))
					{
						_hideConfigGame();
						m_forceext = true;
						if(!_FlashGameSave(id))
							_error(_t("cfgg51", L"No save to flash to real NAND!"));
						_showConfigGame();
					}
				}
			}
			else if(m_configGamePage == 5)
			{
				if(m_btnMgr.selected(m_configBtn1))
				{
					m_gcfg2.setBool(id, "led", !m_gcfg2.getBool(id, "led", 0));
					m_btnMgr.setText(m_configBtn1, _optBoolToString(m_gcfg2.getOptBool(id, "led", 0)));
				}
				else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "private_server") + direction, ARRAY_SIZE(CMenu::_privateServer) + custom_servers.size());
					m_gcfg2.setUInt(id, "private_server", i);
					if(i < 3)
						m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_privateServer[i].id, CMenu::_privateServer[i].text));
					else
						m_btnMgr.setText(m_configLbl2Val, custom_servers[i - 3]);//wstringEx()
				}
				else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn3P) ? 1 : -1;
					i = loopNum(m_gcfg2.getOptBool(id, "fix480p") + direction, 3);
					m_gcfg2.setOptBool(id, "fix480p", i);
					m_btnMgr.setText(m_configLbl3Val, _optBoolToString(m_gcfg2.getOptBool(id, "fix480p", 2)));
				}
				else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "deflicker_wii") + direction, ARRAY_SIZE(CMenu::_DeflickerOptions));
					m_gcfg2.setInt(id, "deflicker_wii", i);
					m_btnMgr.setText(m_configLbl4Val, _t(CMenu::_DeflickerOptions[i].id, CMenu::_DeflickerOptions[i].text));
				}
			}
			else if(m_configGamePage == 6)
			{
				if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
					i = loopNum(m_gcfg2.getUInt(id, "widescreen_wiiu", 0) + direction, ARRAY_SIZE(CMenu::_WidescreenWiiu));
					m_gcfg2.setInt(id, "widescreen_wiiu", i);
					m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_WidescreenWiiu[i].id, CMenu::_WidescreenWiiu[i].text));
				}
			}
		}
	}
	if(!disc)
		m_gcfg2.save(true);// do not save changes for disc games
	_hideConfigGame();
}

void CMenu::_initConfigGameMenu()
{
	_addUserLabels(m_gameSettingsLblUser, ARRAY_SIZE(m_gameSettingsLblUser), "GAME_SETTINGS");
	m_gameSettingsBg = _texture("GAME_SETTINGS/BG", "texture", theme.bg, false);
	m_gameSettingsLblTitle = _addLabel("GAME_SETTINGS/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblPage = _addLabel("GAME_SETTINGS/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPageM = _addPicButton("GAME_SETTINGS/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_gameSettingsBtnPageP = _addPicButton("GAME_SETTINGS/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_gameSettingsBtnBack = _addButton("GAME_SETTINGS/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	_setHideAnim(m_gameSettingsLblTitle, "GAME_SETTINGS/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblPage, "GAME_SETTINGS/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameSettingsBtnPageM, "GAME_SETTINGS/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameSettingsBtnPageP, "GAME_SETTINGS/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameSettingsBtnBack, "GAME_SETTINGS/BACK_BTN", 0, 0, 1.f, -1.f);

	_hideConfigGame(true);
	_textConfigGame();
}

void CMenu::_textConfigGame(void)
{
	m_btnMgr.setText(m_gameSettingsLblTitle, _t("cfgg1", L"Settings"));
	m_btnMgr.setText(m_gameSettingsBtnBack, _t("cfgg8", L"Back"));
}
