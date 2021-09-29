
#include "menu.hpp"
#include "types.h"
#include "loader/wbfs.h"
#include "libwbfs/wiidisc.h"

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

u8 m_gameSettingsMaxPgs = 5;
u8 m_gameSettingsPage = 0;
u8 GCLoader = 0;
int videoScale, videoOffset;
const dir_discHdr *GameHdr;

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

void CMenu::_hideGameSettings(bool instant)
{
	m_btnMgr.hide(m_gameSettingsLblPage, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnBack, instant);
	m_btnMgr.hide(m_gameSettingsLblTitle, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if(m_gameSettingsLblUser[i] != -1)
			m_btnMgr.hide(m_gameSettingsLblUser[i], instant);
	_hideGameSettingsPg(instant);
}

void CMenu::_hideGameSettingsPg(bool instant)
{
	//Does not hide title, page, back, or user labels	
	//Wii & Channels
	m_btnMgr.hide(m_gameSettingsLblHooktype, instant);
	m_btnMgr.hide(m_gameSettingsLblHooktypeVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnHooktypeM, instant);
	m_btnMgr.hide(m_gameSettingsBtnHooktypeP, instant);
	m_btnMgr.hide(m_gameSettingsLblVipatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnVipatch, instant);
	m_btnMgr.hide(m_gameSettingsLblCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModes, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP, instant);
	m_btnMgr.hide(m_gameSettingsLblAspectRatio, instant);
	m_btnMgr.hide(m_gameSettingsLblAspectRatioVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnAspectRatioP, instant);
	m_btnMgr.hide(m_gameSettingsBtnAspectRatioM, instant);
	m_btnMgr.hide(m_gameSettingsLblGameIOS, instant);
	m_btnMgr.hide(m_gameSettingsLblIOS, instant);
	m_btnMgr.hide(m_gameSettingsBtnIOSP, instant);
	m_btnMgr.hide(m_gameSettingsBtnIOSM, instant);
	m_btnMgr.hide(m_gameSettingsLblWidescreenWiiu, instant);
	m_btnMgr.hide(m_gameSettingsLblWidescreenWiiuVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnWidescreenWiiuP, instant);
	m_btnMgr.hide(m_gameSettingsBtnWidescreenWiiuM, instant);
	//wii only
	m_btnMgr.hide(m_gameSettingsLblEmulation, instant);
	m_btnMgr.hide(m_gameSettingsLblEmulationVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmulationP, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmulationM, instant);
	m_btnMgr.hide(m_gameSettingsLblExtractSave, instant);
	m_btnMgr.hide(m_gameSettingsBtnExtractSave, instant);
	m_btnMgr.hide(m_gameSettingsLblFlashSave, instant);
	m_btnMgr.hide(m_gameSettingsBtnFlashSave, instant);
	m_btnMgr.hide(m_gameSettingsLblPrivateServer, instant);
	m_btnMgr.hide(m_gameSettingsLblPrivateServerVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnPrivateServerP, instant);
	m_btnMgr.hide(m_gameSettingsBtnPrivateServerM, instant);
	m_btnMgr.hide(m_gameSettingsLblFix480p, instant);
	m_btnMgr.hide(m_gameSettingsBtnFix480p, instant);
	m_btnMgr.hide(m_gameSettingsLblDeflickerWii, instant);
	m_btnMgr.hide(m_gameSettingsLblDeflickerWiiVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnDeflickerWiiP, instant);
	m_btnMgr.hide(m_gameSettingsBtnDeflickerWiiM, instant);
	//All
	m_btnMgr.hide(m_gameSettingsLblManage, instant);
	m_btnMgr.hide(m_gameSettingsBtnManage, instant);
	m_btnMgr.hide(m_gameSettingsBtnAdultOnly, instant);
	m_btnMgr.hide(m_gameSettingsLblAdultOnly, instant);
	m_btnMgr.hide(m_gameSettingsLblGameLanguage, instant);
	m_btnMgr.hide(m_gameSettingsLblLanguage, instant);
	m_btnMgr.hide(m_gameSettingsBtnLanguageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnLanguageM, instant);
	m_btnMgr.hide(m_gameSettingsLblGameVideo, instant);
	m_btnMgr.hide(m_gameSettingsLblVideo, instant);
	m_btnMgr.hide(m_gameSettingsBtnVideoP, instant);
	m_btnMgr.hide(m_gameSettingsBtnVideoM, instant);
	m_btnMgr.hide(m_gameSettingsLblOcarina, instant);
	m_btnMgr.hide(m_gameSettingsBtnOcarina, instant);
	m_btnMgr.hide(m_gameSettingsLblCheat, instant);
	m_btnMgr.hide(m_gameSettingsBtnCheat, instant);
	m_btnMgr.hide(m_gameSettingsLblDebugger, instant);
	m_btnMgr.hide(m_gameSettingsLblDebuggerV, instant);
	m_btnMgr.hide(m_gameSettingsBtnDebuggerP, instant);
	m_btnMgr.hide(m_gameSettingsBtnDebuggerM, instant);
	m_btnMgr.hide(m_gameSettingsLblLED, instant);
	m_btnMgr.hide(m_gameSettingsBtnLED, instant);
	//GC only
	m_btnMgr.hide(m_gameSettingsLblEmuMemCard, instant);
	m_btnMgr.hide(m_gameSettingsLblEmuMemCard_Val, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmuMemCard_P, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmuMemCard_M, instant);
	m_btnMgr.hide(m_gameSettingsLblDevoMemcardEmu, instant);
	m_btnMgr.hide(m_gameSettingsBtnDevoMemcardEmu, instant);
	m_btnMgr.hide(m_gameSettingsLblWidescreen, instant);
	m_btnMgr.hide(m_gameSettingsBtnWidescreen, instant);
	m_btnMgr.hide(m_gameSettingsLblWiiuWidescreen, instant);
	m_btnMgr.hide(m_gameSettingsBtnWiiuWidescreen, instant);
	m_btnMgr.hide(m_gameSettingsLblGCLoader, instant);
	m_btnMgr.hide(m_gameSettingsLblGCLoader_Val, instant);
	m_btnMgr.hide(m_gameSettingsBtnGCLoader_P, instant);
	m_btnMgr.hide(m_gameSettingsBtnGCLoader_M, instant);
	m_btnMgr.hide(m_gameSettingsLblCC_Rumble, instant);
	m_btnMgr.hide(m_gameSettingsBtnCC_Rumble, instant);
	m_btnMgr.hide(m_gameSettingsLblNATIVE_CTL, instant);
	m_btnMgr.hide(m_gameSettingsBtnNATIVE_CTL, instant);
	m_btnMgr.hide(m_gameSettingsLblDeflicker, instant);
	m_btnMgr.hide(m_gameSettingsBtnDeflicker, instant);
	m_btnMgr.hide(m_gameSettingsLblArcade, instant);
	m_btnMgr.hide(m_gameSettingsBtnArcade, instant);
	m_btnMgr.hide(m_gameSettingsLblSkip_IPL, instant);
	m_btnMgr.hide(m_gameSettingsBtnSkip_IPL, instant);
	m_btnMgr.hide(m_gameSettingsLblPatch50, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatch50, instant);
	m_btnMgr.hide(m_gameSettingsLblWidthVal, instant);
	m_btnMgr.hide(m_gameSettingsLblWidth, instant);
	m_btnMgr.hide(m_gameSettingsBtnWidthP, instant);
	m_btnMgr.hide(m_gameSettingsBtnWidthM, instant);
	m_btnMgr.hide(m_gameSettingsLblPosVal, instant);
	m_btnMgr.hide(m_gameSettingsLblPos, instant);
	m_btnMgr.hide(m_gameSettingsBtnPosP, instant);
	m_btnMgr.hide(m_gameSettingsBtnPosM, instant);
	m_btnMgr.hide(m_gameSettingsLblBBA, instant);
	m_btnMgr.hide(m_gameSettingsBtnBBA, instant);
	m_btnMgr.hide(m_gameSettingsLblNetProfVal, instant);
	m_btnMgr.hide(m_gameSettingsLblNetProf, instant);
	m_btnMgr.hide(m_gameSettingsBtnNetProfP, instant);
	m_btnMgr.hide(m_gameSettingsBtnNetProfM, instant);
	// Channels only
	m_btnMgr.hide(m_gameSettingsLblApploader, instant);
	m_btnMgr.hide(m_gameSettingsBtnApploader, instant);
	m_btnMgr.hide(m_gameSettingsLblCustom, instant);
	m_btnMgr.hide(m_gameSettingsBtnCustom, instant);
	m_btnMgr.hide(m_gameSettingsLblLaunchNK, instant);
	m_btnMgr.hide(m_gameSettingsBtnLaunchNK, instant);
}

void CMenu::_showGameSettings()
{
	vector<string> custom_servers = stringToVector(m_cfg.getString("custom_servers", "servers"), '|');
	u32 i;
	char id[74];
	memset(id, 0, 74);
	
	if(GameHdr->type == TYPE_HOMEBREW)
		wcstombs(id, GameHdr->title, 63);
	else if(GameHdr->type == TYPE_PLUGIN)
	{
		strncpy(m_plugin.PluginMagicWord, fmt("%08x", GameHdr->settings[0]), 8);
		if(strrchr(GameHdr->path, '/') != NULL)
		{
			char gameTitle[64];
			gameTitle[63] = '\0';
			wcstombs(gameTitle, GameHdr->title, 63);
			strncpy(id, fmt("%s/%s", m_plugin.PluginMagicWord, gameTitle), sizeof(id) - 1);
		}
		else
			strncpy(id, fmt("%s/%s", m_plugin.PluginMagicWord, GameHdr->path), sizeof(id) - 1);
	}
	else
	{
		strcpy(id, GameHdr->id);
	}
	
	_setBg(m_gameSettingsBg, m_gameSettingsBg);
	for(i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if(m_gameSettingsLblUser[i] != -1)
			m_btnMgr.show(m_gameSettingsLblUser[i]);

	wstringEx title(_t("cfgg1", L"Settings"));
	if(!NoGameID(GameHdr->type))
		title.append(wfmt(L" [%.6s]", id));
	m_btnMgr.setText(m_gameSettingsLblTitle, title);
	m_btnMgr.show(m_gameSettingsLblTitle);
	
	m_btnMgr.show(m_gameSettingsBtnBack);
	
	if(GameHdr->type == TYPE_PLUGIN || GameHdr->type == TYPE_HOMEBREW)
	{
		m_btnMgr.show(m_gameSettingsBtnAdultOnly);
		m_btnMgr.show(m_gameSettingsLblAdultOnly);
		if(GameHdr->type == TYPE_PLUGIN)
			m_btnMgr.setText(m_gameSettingsBtnAdultOnly, m_gcfg1.getBool("ADULTONLY_PLUGINS", id, false) ? _t("yes", L"Yes") : _t("no", L"No"));
		else
			m_btnMgr.setText(m_gameSettingsBtnAdultOnly, m_gcfg1.getBool("ADULTONLY", id, false) ? _t("yes", L"Yes") : _t("no", L"No"));
		return;
	}

	if(GameHdr->type == TYPE_GC_GAME)
	{
		GCLoader = min(m_gcfg2.getUInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
		GCLoader = (GCLoader == 0) ? min(m_cfg.getUInt(GC_DOMAIN, "default_loader", 1), ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1u) : GCLoader-1;
	}
	
	m_gameSettingsMaxPgs = IsOnWiiU() ? 6 : 5;
	if(GameHdr->type == TYPE_GC_GAME)
	{
		if(GCLoader == DEVOLUTION)
			m_gameSettingsMaxPgs = 2;
		else
			m_gameSettingsMaxPgs = 6;
	}
	
	m_btnMgr.setText(m_gameSettingsLblPage, wfmt(L"%i / %i", m_gameSettingsPage, m_gameSettingsMaxPgs));
	m_btnMgr.show(m_gameSettingsLblPage);
	m_btnMgr.show(m_gameSettingsBtnPageM);
	m_btnMgr.show(m_gameSettingsBtnPageP);
	
	if(m_gameSettingsPage == 1)
	{
		m_btnMgr.show(m_gameSettingsBtnAdultOnly);
		m_btnMgr.show(m_gameSettingsLblAdultOnly);
		
		if(GameHdr->type == TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblGCLoader);
			m_btnMgr.show(m_gameSettingsLblGCLoader_Val);
			m_btnMgr.show(m_gameSettingsBtnGCLoader_P);
			m_btnMgr.show(m_gameSettingsBtnGCLoader_M);
		}
		else
		{
			m_btnMgr.show(m_gameSettingsLblGameIOS);
			m_btnMgr.show(m_gameSettingsLblIOS);
			m_btnMgr.show(m_gameSettingsBtnIOSP);
			m_btnMgr.show(m_gameSettingsBtnIOSM);
		}
		
		m_btnMgr.show(m_gameSettingsLblGameLanguage);
		m_btnMgr.show(m_gameSettingsLblLanguage);
		m_btnMgr.show(m_gameSettingsBtnLanguageP);
		m_btnMgr.show(m_gameSettingsBtnLanguageM);

		m_btnMgr.show(m_gameSettingsLblGameVideo);
		m_btnMgr.show(m_gameSettingsLblVideo);
		m_btnMgr.show(m_gameSettingsBtnVideoP);
		m_btnMgr.show(m_gameSettingsBtnVideoM);
	}
	if(m_gameSettingsPage == 2)
	{
		if(GameHdr->type == TYPE_GC_GAME && GCLoader == DEVOLUTION)
		{
			m_btnMgr.show(m_gameSettingsLblLED);
			m_btnMgr.show(m_gameSettingsBtnLED);

			m_btnMgr.show(m_gameSettingsLblDevoMemcardEmu);
			m_btnMgr.show(m_gameSettingsBtnDevoMemcardEmu);

			m_btnMgr.show(m_gameSettingsLblWidescreen);
			m_btnMgr.show(m_gameSettingsBtnWidescreen);

			m_btnMgr.show(m_gameSettingsLblManage);
			m_btnMgr.show(m_gameSettingsBtnManage);
		}
		else
		{
			m_btnMgr.show(m_gameSettingsLblDebugger);
			m_btnMgr.show(m_gameSettingsLblDebuggerV);
			m_btnMgr.show(m_gameSettingsBtnDebuggerP);
			m_btnMgr.show(m_gameSettingsBtnDebuggerM);
		
			if(GameHdr->type != TYPE_GC_GAME)
			{
				m_btnMgr.show(m_gameSettingsLblHooktype);
				m_btnMgr.show(m_gameSettingsLblHooktypeVal);
				m_btnMgr.show(m_gameSettingsBtnHooktypeM);
				m_btnMgr.show(m_gameSettingsBtnHooktypeP);
			}
			else
			{
				m_btnMgr.show(m_gameSettingsLblDeflicker);
				m_btnMgr.show(m_gameSettingsBtnDeflicker);
			}
			
			m_btnMgr.show(m_gameSettingsLblOcarina);
			m_btnMgr.show(m_gameSettingsBtnOcarina);
		
			m_btnMgr.show(m_gameSettingsLblCheat);
			m_btnMgr.show(m_gameSettingsBtnCheat);
		}
	}
	if(m_gameSettingsPage == 3)
	{
		if(GameHdr->type != TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblPatchVidModes);
			m_btnMgr.show(m_gameSettingsLblPatchVidModesVal);
			m_btnMgr.show(m_gameSettingsBtnPatchVidModesM);
			m_btnMgr.show(m_gameSettingsBtnPatchVidModesP);
			
			m_btnMgr.show(m_gameSettingsLblVipatch);
			m_btnMgr.show(m_gameSettingsBtnVipatch);

			m_btnMgr.show(m_gameSettingsLblCountryPatch);
			m_btnMgr.show(m_gameSettingsBtnCountryPatch);

			m_btnMgr.show(m_gameSettingsLblAspectRatio);
			m_btnMgr.show(m_gameSettingsLblAspectRatioVal);
			m_btnMgr.show(m_gameSettingsBtnAspectRatioP);
			m_btnMgr.show(m_gameSettingsBtnAspectRatioM);
		}
		else
		{
			m_btnMgr.show(m_gameSettingsLblWiiuWidescreen);
			m_btnMgr.show(m_gameSettingsBtnWiiuWidescreen);

			m_btnMgr.show(m_gameSettingsLblEmuMemCard);
			m_btnMgr.show(m_gameSettingsLblEmuMemCard_Val);
			m_btnMgr.show(m_gameSettingsBtnEmuMemCard_P);
			m_btnMgr.show(m_gameSettingsBtnEmuMemCard_M);

			m_btnMgr.show(m_gameSettingsLblWidescreen);
			m_btnMgr.show(m_gameSettingsBtnWidescreen);

			m_btnMgr.show(m_gameSettingsLblManage);
			m_btnMgr.show(m_gameSettingsBtnManage);
		}
	}
	if(m_gameSettingsPage == 4)
	{
		if(GameHdr->type == TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblCC_Rumble);
			m_btnMgr.show(m_gameSettingsBtnCC_Rumble);
			
		 	m_btnMgr.show(m_gameSettingsLblNATIVE_CTL);
			m_btnMgr.show(m_gameSettingsBtnNATIVE_CTL);

			m_btnMgr.show(m_gameSettingsBtnArcade);
			m_btnMgr.show(m_gameSettingsLblArcade);

			m_btnMgr.show(m_gameSettingsBtnSkip_IPL);
			m_btnMgr.show(m_gameSettingsLblSkip_IPL);

		}
		else
		{
			m_btnMgr.show(m_gameSettingsLblManage);
			m_btnMgr.show(m_gameSettingsBtnManage);

			if(GameHdr->type == TYPE_CHANNEL || GameHdr->type == TYPE_EMUCHANNEL)
			{
				m_btnMgr.show(m_gameSettingsLblCustom);
				m_btnMgr.show(m_gameSettingsBtnCustom);
				
				m_btnMgr.show(m_gameSettingsLblApploader);
				m_btnMgr.show(m_gameSettingsBtnApploader);

				m_btnMgr.show(m_gameSettingsLblLaunchNK);
				m_btnMgr.show(m_gameSettingsBtnLaunchNK);
			}
			else if(GameHdr->type == TYPE_WII_GAME)
			{
				m_btnMgr.show(m_gameSettingsLblEmulationVal);
				m_btnMgr.show(m_gameSettingsLblEmulation);
				m_btnMgr.show(m_gameSettingsBtnEmulationP);
				m_btnMgr.show(m_gameSettingsBtnEmulationM);
				
				m_btnMgr.show(m_gameSettingsLblExtractSave);
				m_btnMgr.show(m_gameSettingsBtnExtractSave);
				
				m_btnMgr.show(m_gameSettingsLblFlashSave);
				m_btnMgr.show(m_gameSettingsBtnFlashSave);
			}
		}
	}
	if(m_gameSettingsPage == 5)
	{
		if(GameHdr->type == TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblLED);
			m_btnMgr.show(m_gameSettingsBtnLED);
			
			m_btnMgr.show(m_gameSettingsLblWidthVal);
			m_btnMgr.show(m_gameSettingsLblWidth);
			m_btnMgr.show(m_gameSettingsBtnWidthP);
			m_btnMgr.show(m_gameSettingsBtnWidthM);
			
			m_btnMgr.show(m_gameSettingsLblPosVal);
			m_btnMgr.show(m_gameSettingsLblPos);
			m_btnMgr.show(m_gameSettingsBtnPosP);
			m_btnMgr.show(m_gameSettingsBtnPosM);
			
			m_btnMgr.show(m_gameSettingsBtnPatch50);
			m_btnMgr.show(m_gameSettingsLblPatch50);
		}
		else
		{
			m_btnMgr.show(m_gameSettingsLblLED);
			m_btnMgr.show(m_gameSettingsBtnLED);

			//if(GameHdr->type == TYPE_WII_GAME)
			//{
				m_btnMgr.show(m_gameSettingsLblPrivateServer);
				m_btnMgr.show(m_gameSettingsLblPrivateServerVal);
				m_btnMgr.show(m_gameSettingsBtnPrivateServerP);
				m_btnMgr.show(m_gameSettingsBtnPrivateServerM);
				
				m_btnMgr.show(m_gameSettingsLblFix480p);
				m_btnMgr.show(m_gameSettingsBtnFix480p);

				m_btnMgr.show(m_gameSettingsLblDeflickerWii);
				m_btnMgr.show(m_gameSettingsLblDeflickerWiiVal);
				m_btnMgr.show(m_gameSettingsBtnDeflickerWiiM);
				m_btnMgr.show(m_gameSettingsBtnDeflickerWiiP);
			//}
		}
	}
	if(m_gameSettingsPage == 6)
	{
		if(GameHdr->type == TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblBBA);
			m_btnMgr.show(m_gameSettingsBtnBBA);
		
			m_btnMgr.show(m_gameSettingsLblNetProfVal);
			m_btnMgr.show(m_gameSettingsLblNetProf);
			m_btnMgr.show(m_gameSettingsBtnNetProfP);
			m_btnMgr.show(m_gameSettingsBtnNetProfM);
		}
		else
		{
			m_btnMgr.show(m_gameSettingsLblWidescreenWiiu);
			m_btnMgr.show(m_gameSettingsLblWidescreenWiiuVal);
			m_btnMgr.show(m_gameSettingsBtnWidescreenWiiuP);
			m_btnMgr.show(m_gameSettingsBtnWidescreenWiiuM);
		}
	}

	m_btnMgr.setText(m_gameSettingsBtnAdultOnly, m_gcfg1.getBool("ADULTONLY", id, false) ? _t("yes", L"Yes") : _t("no", L"No"));
	m_btnMgr.setText(m_gameSettingsBtnOcarina, _optBoolToString(m_gcfg2.getOptBool(id, "cheat", 0)));
	m_btnMgr.setText(m_gameSettingsBtnLED, _optBoolToString(m_gcfg2.getOptBool(id, "led", 0)));
	
	i = min(m_gcfg2.getUInt(id, "debugger", 0), ARRAY_SIZE(CMenu::_debugger) - 1u);
	m_btnMgr.setText(m_gameSettingsLblDebuggerV, _t(CMenu::_debugger[i].id, CMenu::_debugger[i].text));
	
	if(GameHdr->type == TYPE_GC_GAME)
	{
		//widescreen is shared by devo and nintendont
		m_btnMgr.setText(m_gameSettingsBtnWidescreen, _optBoolToString(m_gcfg2.getOptBool(id, "widescreen", 0)));
		m_btnMgr.setText(m_gameSettingsBtnWiiuWidescreen, _optBoolToString(m_gcfg2.getOptBool(id, "wiiu_widescreen", 0)));
		m_btnMgr.setText(m_gameSettingsBtnDevoMemcardEmu, _optBoolToString(m_gcfg2.getOptBool(id, "devo_memcard_emu", 2)));
		m_btnMgr.setText(m_gameSettingsBtnCC_Rumble, _optBoolToString(m_gcfg2.getOptBool(id, "cc_rumble", 2)));
		m_btnMgr.setText(m_gameSettingsBtnNATIVE_CTL, _optBoolToString(m_gcfg2.getOptBool(id, "native_ctl", 2)));
		m_btnMgr.setText(m_gameSettingsBtnDeflicker, _optBoolToString(m_gcfg2.getOptBool(id, "deflicker", 0)));
		m_btnMgr.setText(m_gameSettingsBtnArcade, _optBoolToString(m_gcfg2.getOptBool(id, "triforce_arcade", 0)));
		m_btnMgr.setText(m_gameSettingsBtnSkip_IPL, _optBoolToString(m_gcfg2.getOptBool(id, "skip_ipl", 0)));
		m_btnMgr.setText(m_gameSettingsBtnPatch50, _optBoolToString(m_gcfg2.getOptBool(id, "patch_pal50", 0)));
		m_btnMgr.setText(m_gameSettingsBtnBBA, _optBoolToString(m_gcfg2.getOptBool(id, "bba_emu", 0)));
		
		u8 netprofile = m_gcfg2.getUInt(id, "net_profile", 0);
		if(netprofile == 0)
			m_btnMgr.setText(m_gameSettingsLblNetProfVal, _t("GC_Auto", L"Auto"));
		else
			m_btnMgr.setText(m_gameSettingsLblNetProfVal, wfmt(L"%i", netprofile));
		
		if(videoScale == 0)
			m_btnMgr.setText(m_gameSettingsLblWidthVal, _t("GC_Auto", L"Auto"));
		else if(videoScale == 127)
			m_btnMgr.setText(m_gameSettingsLblWidthVal, _t("def", L"Default"));
		else
			m_btnMgr.setText(m_gameSettingsLblWidthVal, wfmt(L"%i", max(40, min(120, videoScale)) + 600));
			
		if(videoOffset == 127)
			m_btnMgr.setText(m_gameSettingsLblPosVal, _t("def", L"Default"));
		else
			m_btnMgr.setText(m_gameSettingsLblPosVal, wfmt(L"%i", max(-20, min(20, videoOffset))));

		i = min(m_gcfg2.getUInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_GCvideoModes) - 1u);
		m_btnMgr.setText(m_gameSettingsLblVideo, _t(CMenu::_GCvideoModes[i].id, CMenu::_GCvideoModes[i].text));
		
		i = min(m_gcfg2.getUInt(id, "language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
		m_btnMgr.setText(m_gameSettingsLblLanguage, _t(CMenu::_GClanguages[i].id, CMenu::_GClanguages[i].text));
		
		i = min(m_gcfg2.getUInt(id, "emu_memcard", 0), ARRAY_SIZE(CMenu::_NinEmuCard) - 1u);
		m_btnMgr.setText(m_gameSettingsLblEmuMemCard_Val, _t(CMenu::_NinEmuCard[i].id, CMenu::_NinEmuCard[i].text));
			
		i = min(m_gcfg2.getUInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
		m_btnMgr.setText(m_gameSettingsLblGCLoader_Val, _t(CMenu::_GCLoader[i].id, CMenu::_GCLoader[i].text));
	}
	else
	{
		m_btnMgr.setText(m_gameSettingsBtnVipatch, _optBoolToString(m_gcfg2.getOptBool(id, "vipatch", 0)));
		m_btnMgr.setText(m_gameSettingsBtnCountryPatch, _optBoolToString(m_gcfg2.getOptBool(id, "country_patch", 0)));
		i = min(m_gcfg2.getUInt(id, "private_server", 0), ARRAY_SIZE(CMenu::_privateServer) - 1u);
		if(i < 3)
			m_btnMgr.setText(m_gameSettingsLblPrivateServerVal, _t(CMenu::_privateServer[i].id, CMenu::_privateServer[i].text));
		else
			m_btnMgr.setText(m_gameSettingsLblPrivateServerVal, custom_servers[i - 3]);//wstringEx()
		
		m_btnMgr.setText(m_gameSettingsBtnFix480p, _optBoolToString(m_gcfg2.getOptBool(id, "fix480p", 2)));

		i = min(m_gcfg2.getUInt(id, "deflicker_wii", 0), ARRAY_SIZE(CMenu::_DeflickerOptions) - 1u);
		m_btnMgr.setText(m_gameSettingsLblDeflickerWiiVal, _t(CMenu::_DeflickerOptions[i].id, CMenu::_DeflickerOptions[i].text));

		i = min(m_gcfg2.getUInt(id, "widescreen_wiiu", 0), ARRAY_SIZE(CMenu::_WidescreenWiiu) - 1u);
		m_btnMgr.setText(m_gameSettingsLblWidescreenWiiuVal, _t(CMenu::_WidescreenWiiu[i].id, CMenu::_WidescreenWiiu[i].text));
		
		m_btnMgr.setText(m_gameSettingsBtnCustom, _optBoolToString(m_gcfg2.getOptBool(id, "custom", 0)));
		m_btnMgr.setText(m_gameSettingsBtnLaunchNK, _optBoolToString(m_gcfg2.getOptBool(id, "useneek", 0)));
		m_btnMgr.setText(m_gameSettingsBtnApploader, _optBoolToString(m_gcfg2.getOptBool(id, "apploader", 0)));
		
		i = min(m_gcfg2.getUInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
		m_btnMgr.setText(m_gameSettingsLblVideo, _t(CMenu::_VideoModes[i].id, CMenu::_VideoModes[i].text));
		
		i = min(m_gcfg2.getUInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
		m_btnMgr.setText(m_gameSettingsLblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
		
		i = min(m_gcfg2.getUInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u);
		m_btnMgr.setText(m_gameSettingsLblAspectRatioVal, _t(CMenu::_AspectRatio[i].id, CMenu::_AspectRatio[i].text));

		// this getInt() is different than the normal getInt()
		// this getInt() will actually set the j variable to the value of the key. hence the & before j.
		// this appears to be the only time this getInt() is called of all wiiflow code.
		// why they didn't just set int j = m_gcfg2.getInt(id, "ios", 0); i do not know.
		int j = 0;
		if(m_gcfg2.getInt(id, "ios", &j) && _installed_cios.size() > 0)
		{
			CIOSItr itr = _installed_cios.find(j);
			j = (itr == _installed_cios.end()) ? 0 : itr->first;
		}
		else j = 0;

		if(j > 0)
			m_btnMgr.setText(m_gameSettingsLblIOS, wfmt(L"%i", j));
		else
			m_btnMgr.setText(m_gameSettingsLblIOS, L"AUTO");

		i = min(m_gcfg2.getUInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
		m_btnMgr.setText(m_gameSettingsLblPatchVidModesVal, _t(CMenu::_vidModePatch[i].id, CMenu::_vidModePatch[i].text));

		i = min(m_gcfg2.getUInt(id, "hooktype", 0), ARRAY_SIZE(CMenu::_hooktype) - 1u);
		m_btnMgr.setText(m_gameSettingsLblHooktypeVal, _t(CMenu::_hooktype[i].id, CMenu::_hooktype[i].text));

		i = min(m_gcfg2.getUInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);
		m_btnMgr.setText(m_gameSettingsLblEmulationVal, _t(CMenu::_SaveEmu[i].id, CMenu::_SaveEmu[i].text));
	}
}

void CMenu::_gameSettings(const dir_discHdr *hdr, bool disc)
{
	vector<string> custom_servers = stringToVector(m_cfg.getString("custom_servers", "servers"), '|');
	m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
	GameHdr = hdr;// set for global use in other fuctions
	char id[74];
	memset(id, 0, 74);
	
	if(GameHdr->type == TYPE_HOMEBREW)
		wcstombs(id, GameHdr->title, 63);
	else if(GameHdr->type == TYPE_PLUGIN)
	{
		strncpy(m_plugin.PluginMagicWord, fmt("%08x", GameHdr->settings[0]), 8);
		if(strrchr(GameHdr->path, '/') != NULL)
		{
			char gameTitle[64];
			gameTitle[63] = '\0';
			wcstombs(gameTitle, GameHdr->title, 63);
			strncpy(id, fmt("%s/%s", m_plugin.PluginMagicWord, gameTitle), sizeof(id) - 1);
		}
		else
			strncpy(id, fmt("%s/%s", m_plugin.PluginMagicWord, GameHdr->path), sizeof(id) - 1);
	}
	else
	{
		strcpy(id, GameHdr->id);
	}
	
	if(GameHdr->type == TYPE_GC_GAME)
	{
		videoScale = m_gcfg2.getInt(id, "nin_width", 127);
		videoOffset = m_gcfg2.getInt(id, "nin_pos", 127);
	}
	m_gameSettingsPage = 1;
	_showGameSettings();
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
			_hideGameSettingsPg();
			if(m_gameSettingsPage == 1)
				m_gameSettingsPage = m_gameSettingsMaxPgs;
			else --m_gameSettingsPage;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_gameSettingsBtnPageM);
			_showGameSettings();
		}
		else if(BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageP)))
		{
			_hideGameSettingsPg();
			if(m_gameSettingsPage == m_gameSettingsMaxPgs)
				m_gameSettingsPage = 1;
			else
				++m_gameSettingsPage;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_gameSettingsBtnPageP);
			_showGameSettings();
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_gameSettingsBtnBack))
				break;
			else if(m_btnMgr.selected(m_gameSettingsBtnOcarina))
			{
				m_gcfg2.setBool(id, "cheat", !m_gcfg2.getBool(id, "cheat", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnVipatch))
			{
				m_gcfg2.setBool(id, "vipatch", !m_gcfg2.getBool(id, "vipatch", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnCountryPatch))
			{
				m_gcfg2.setBool(id, "country_patch", !m_gcfg2.getBool(id, "country_patch", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnCustom))
			{
				m_gcfg2.setBool(id, "custom", !m_gcfg2.getBool(id, "custom", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnLaunchNK))
			{
				m_gcfg2.setBool(id, "useneek", !m_gcfg2.getBool(id, "useneek", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnDevoMemcardEmu))
			{
				m_gcfg2.setOptBool(id, "devo_memcard_emu", loopNum(m_gcfg2.getOptBool(id, "devo_memcard_emu") + 1, 3));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnWidescreen))
			{
				m_gcfg2.setBool(id, "widescreen", !m_gcfg2.getBool(id, "widescreen", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnWiiuWidescreen))
			{
				m_gcfg2.setBool(id, "wiiu_widescreen", !m_gcfg2.getBool(id, "wiiu_widescreen", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnWidescreenWiiuP) || m_btnMgr.selected(m_gameSettingsBtnWidescreenWiiuM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnWidescreenWiiuP) ? 1 : -1;
				m_gcfg2.setInt(id, "widescreen_wiiu", (int)loopNum(m_gcfg2.getUInt(id, "widescreen_wiiu", 0) + direction, ARRAY_SIZE(CMenu::_WidescreenWiiu)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnLanguageP) || m_btnMgr.selected(m_gameSettingsBtnLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnLanguageP) ? 1 : -1;
				if(GameHdr->type == TYPE_GC_GAME)
					m_gcfg2.setInt(id, "language", (int)loopNum(m_gcfg2.getUInt(id, "language", 0) + direction, ARRAY_SIZE(CMenu::_GClanguages)));
				else
					m_gcfg2.setInt(id, "language", (int)loopNum(m_gcfg2.getUInt(id, "language", 0) + direction, ARRAY_SIZE(CMenu::_languages)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnVideoP) || m_btnMgr.selected(m_gameSettingsBtnVideoM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnVideoP) ? 1 : -1;
				if(GameHdr->type == TYPE_GC_GAME)
					m_gcfg2.setInt(id, "video_mode", (int)loopNum(m_gcfg2.getUInt(id, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_GCvideoModes)));
				else
					m_gcfg2.setInt(id, "video_mode", (int)loopNum(m_gcfg2.getUInt(id, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_VideoModes)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnAspectRatioP) || m_btnMgr.selected(m_gameSettingsBtnAspectRatioM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnAspectRatioP) ? 1 : -1;
				m_gcfg2.setInt(id, "aspect_ratio", (int)loopNum(m_gcfg2.getUInt(id, "aspect_ratio", 0) + direction, ARRAY_SIZE(CMenu::_AspectRatio)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnWidthP) || m_btnMgr.selected(m_gameSettingsBtnWidthM))
			{
				if(m_btnMgr.selected(m_gameSettingsBtnWidthP))
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
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnPosP) || m_btnMgr.selected(m_gameSettingsBtnPosM))
			{
				if(m_btnMgr.selected(m_gameSettingsBtnPosP))
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
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnEmuMemCard_P) || m_btnMgr.selected(m_gameSettingsBtnEmuMemCard_M))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnEmuMemCard_P) ? 1 : -1;
				m_gcfg2.setInt(id, "emu_memcard", (int)loopNum(m_gcfg2.getUInt(id, "emu_memcard", 2) + direction, ARRAY_SIZE(CMenu::_NinEmuCard)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnGCLoader_P) || m_btnMgr.selected(m_gameSettingsBtnGCLoader_M))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnGCLoader_P) ? 1 : -1;
				m_gcfg2.setInt(id, "gc_loader", (int)loopNum(m_gcfg2.getUInt(id, "gc_loader", 0) + direction, ARRAY_SIZE(CMenu::_GCLoader)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnIOSM) || m_btnMgr.selected(m_gameSettingsBtnIOSP))
			{
				if(_installed_cios.size() > 0)
				{
					bool direction = m_btnMgr.selected(m_gameSettingsBtnIOSP);
					CIOSItr itr = _installed_cios.find((u32)m_gcfg2.getInt(id, "ios", 0));
					if(direction && itr == _installed_cios.end())
						itr = _installed_cios.begin();
					else if(!direction && itr == _installed_cios.begin())
						itr = _installed_cios.end();
					else if(direction)
						itr++;

					if(!direction)
						itr--;

					if(itr->first != 0)
						m_gcfg2.setInt(id, "ios", itr->first);
					else
						m_gcfg2.remove(id, "ios");
					_showGameSettings();
				}
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnPatchVidModesP) || m_btnMgr.selected(m_gameSettingsBtnPatchVidModesM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnPatchVidModesP) ? 1 : -1;
				m_gcfg2.setInt(id, "patch_video_modes", (int)loopNum(m_gcfg2.getUInt(id, "patch_video_modes", 0) + direction, ARRAY_SIZE(CMenu::_vidModePatch)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnManage))
			{
				if(disc)
					error(_t("cfgg57", L"Not allowed for disc!"));
				else
				{
					CoverFlow.stopCoverLoader(true);
					_hideGameSettings();
					_CoverBanner();
					CoverFlow.startCoverLoader();
				}
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnCheat))
			{
				_hideGameSettings();
				_CheatSettings();
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnHooktypeP) || m_btnMgr.selected(m_gameSettingsBtnHooktypeM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnHooktypeP) ? 1 : -1;
				m_gcfg2.setInt(id, "hooktype", (int)loopNum(m_gcfg2.getUInt(id, "hooktype", 1) + direction, ARRAY_SIZE(CMenu::_hooktype)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnEmulationP) || m_btnMgr.selected(m_gameSettingsBtnEmulationM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnEmulationP) ? 1 : -1;
				m_gcfg2.setInt(id, "emulate_save", (int)loopNum(m_gcfg2.getUInt(id, "emulate_save", 0) + direction, ARRAY_SIZE(CMenu::_SaveEmu)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnDebuggerP) || m_btnMgr.selected(m_gameSettingsBtnDebuggerM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnDebuggerP) ? 1 : -1;
				m_gcfg2.setInt(id, "debugger", (int)loopNum(m_gcfg2.getUInt(id, "debugger", 0) + direction, ARRAY_SIZE(CMenu::_debugger)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnApploader))
			{
				m_gcfg2.setBool(id, "apploader", !m_gcfg2.getBool(id, "apploader", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnLED))
			{
				m_gcfg2.setBool(id, "led", !m_gcfg2.getBool(id, "led", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnNATIVE_CTL))
			{
				m_gcfg2.setOptBool(id, "native_ctl", loopNum(m_gcfg2.getOptBool(id, "native_ctl") + 1, 3));
				_showGameSettings();
			}	
			else if(m_btnMgr.selected(m_gameSettingsBtnDeflicker))
			{
				m_gcfg2.setBool(id, "deflicker", !m_gcfg2.getBool(id, "deflicker", 0));
				_showGameSettings();
			}	
			else if(m_btnMgr.selected(m_gameSettingsBtnCC_Rumble))
			{
				m_gcfg2.setOptBool(id, "cc_rumble", loopNum(m_gcfg2.getOptBool(id, "cc_rumble") + 1, 3));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnArcade))
			{
				m_gcfg2.setBool(id, "triforce_arcade", !m_gcfg2.getBool(id, "triforce_arcade", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnSkip_IPL))
			{
				m_gcfg2.setBool(id, "skip_ipl", !m_gcfg2.getBool(id, "skip_ipl", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnPatch50))
			{
				m_gcfg2.setBool(id, "patch_pal50", !m_gcfg2.getBool(id, "patch_pal50", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnBBA))
			{
				m_gcfg2.setBool(id, "bba_emu", !m_gcfg2.getBool(id, "bba_emu", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnNetProfP) || m_btnMgr.selected(m_gameSettingsBtnNetProfM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnNetProfP) ? 1 : -1;
				m_gcfg2.setInt(id, "net_profile", loopNum(m_gcfg2.getInt(id, "net_profile") + direction, 4));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnPrivateServerP) || m_btnMgr.selected(m_gameSettingsBtnPrivateServerM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnPrivateServerP) ? 1 : -1;
				u8 val = loopNum(m_gcfg2.getUInt(id, "private_server") + direction, ARRAY_SIZE(CMenu::_privateServer) + custom_servers.size());
				m_gcfg2.setUInt(id, "private_server", val);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnFix480p))
			{
				m_gcfg2.setOptBool(id, "fix480p", loopNum(m_gcfg2.getOptBool(id, "fix480p") + 1, 3));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnDeflickerWiiP) || m_btnMgr.selected(m_gameSettingsBtnDeflickerWiiM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnDeflickerWiiP) ? 1 : -1;
				m_gcfg2.setInt(id, "deflicker_wii", loopNum(m_gcfg2.getUInt(id, "deflicker_wii") + direction, ARRAY_SIZE(CMenu::_DeflickerOptions)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnAdultOnly))
			{
				if(disc)
					error(_t("cfgg57", L"Not allowed for disc!"));
				else
				{
					if(GameHdr->type == TYPE_PLUGIN)
						m_gcfg1.setBool("ADULTONLY_PLUGINS", id, !m_gcfg1.getBool("ADULTONLY_PLUGINS", id, false));
					else
						m_gcfg1.setBool("ADULTONLY", id, !m_gcfg1.getBool("ADULTONLY", id, false));
				}
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnExtractSave))
			{
				_hideGameSettings();
				m_forceext = true;
				if(!_AutoExtractSave(id))
					error(_t("cfgg50", L"No save to extract!"));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnFlashSave))
			{
				_hideGameSettings();
				m_forceext = true;
				if(!_FlashSave(id))
					error(_t("cfgg51", L"No save to flash to real NAND!"));
				_showGameSettings();
			}
		}
	}
	if(!disc)
		m_gcfg2.save(true);// do not save changes for disc games
	_hideGameSettings();
}

void CMenu::_initGameSettingsMenu()
{
	_addUserLabels(m_gameSettingsLblUser, ARRAY_SIZE(m_gameSettingsLblUser), "GAME_SETTINGS");
	m_gameSettingsBg = _texture("GAME_SETTINGS/BG", "texture", theme.bg, false);
	m_gameSettingsLblTitle = _addLabel("GAME_SETTINGS/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

// Page 1
	m_gameSettingsLblAdultOnly = _addLabel("GAME_SETTINGS/ADULT_ONLY", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnAdultOnly = _addButton("GAME_SETTINGS/ADULT_ONLY_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblGameIOS = _addLabel("GAME_SETTINGS/IOS", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblIOS = _addLabel("GAME_SETTINGS/IOS_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnIOSM = _addPicButton("GAME_SETTINGS/IOS_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_gameSettingsBtnIOSP = _addPicButton("GAME_SETTINGS/IOS_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);

	m_gameSettingsLblGameLanguage = _addLabel("GAME_SETTINGS/GAME_LANG", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblLanguage = _addLabel("GAME_SETTINGS/GAME_LANG_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnLanguageM = _addPicButton("GAME_SETTINGS/GAME_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_gameSettingsBtnLanguageP = _addPicButton("GAME_SETTINGS/GAME_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);

	m_gameSettingsLblGameVideo = _addLabel("GAME_SETTINGS/VIDEO", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblVideo = _addLabel("GAME_SETTINGS/VIDEO_BTN", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnVideoM = _addPicButton("GAME_SETTINGS/VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_gameSettingsBtnVideoP = _addPicButton("GAME_SETTINGS/VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);

//GC Page 1 line 2
	m_gameSettingsLblGCLoader = _addLabel("GAME_SETTINGS/GC_LOADER", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblGCLoader_Val = _addLabel("GAME_SETTINGS/GC_LOADER_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnGCLoader_M = _addPicButton("GAME_SETTINGS/GC_LOADER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_gameSettingsBtnGCLoader_P = _addPicButton("GAME_SETTINGS/GC_LOADER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);

// Page 2
	m_gameSettingsLblDebugger = _addLabel("GAME_SETTINGS/GAME_DEBUGGER", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblDebuggerV = _addLabel("GAME_SETTINGS/GAME_DEBUGGER_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnDebuggerM = _addPicButton("GAME_SETTINGS/GAME_DEBUGGER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_gameSettingsBtnDebuggerP = _addPicButton("GAME_SETTINGS/GAME_DEBUGGER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);

	m_gameSettingsLblHooktype = _addLabel("GAME_SETTINGS/HOOKTYPE", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblHooktypeVal = _addLabel("GAME_SETTINGS/HOOKTYPE_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnHooktypeM = _addPicButton("GAME_SETTINGS/HOOKTYPE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_gameSettingsBtnHooktypeP = _addPicButton("GAME_SETTINGS/HOOKTYPE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);

	m_gameSettingsLblOcarina = _addLabel("GAME_SETTINGS/OCARINA", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnOcarina = _addButton("GAME_SETTINGS/OCARINA_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

	m_gameSettingsLblCheat = _addLabel("GAME_SETTINGS/CHEAT", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCheat = _addButton("GAME_SETTINGS/CHEAT_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

//GC Page 2
	//nintendont = debugger, deflicker, ocarina, cheats
	//devolution = led_activity(pg5), emu_memcard, widescreen(pg3), mng_cvr_bnr (pg4)
	
	m_gameSettingsLblDeflicker = _addLabel("GAME_SETTINGS/DEFLICKER", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnDeflicker = _addButton("GAME_SETTINGS/DEFLICKER_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_gameSettingsLblDevoMemcardEmu = _addLabel("GAME_SETTINGS/DEVO_MEMCARD_EMU", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnDevoMemcardEmu = _addButton("GAME_SETTINGS/DEVO_MEMCARD_EMU_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

//Page 3
	m_gameSettingsLblCountryPatch = _addLabel("GAME_SETTINGS/COUNTRY_PATCH", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCountryPatch = _addButton("GAME_SETTINGS/COUNTRY_PATCH_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblVipatch = _addLabel("GAME_SETTINGS/VIPATCH", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnVipatch = _addButton("GAME_SETTINGS/VIPATCH_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_gameSettingsLblPatchVidModes = _addLabel("GAME_SETTINGS/PATCH_VIDEO_MODE", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblPatchVidModesVal = _addLabel("GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPatchVidModesM = _addPicButton("GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_gameSettingsBtnPatchVidModesP = _addPicButton("GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);

	m_gameSettingsLblAspectRatio = _addLabel("GAME_SETTINGS/ASPECT_RATIO", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblAspectRatioVal = _addLabel("GAME_SETTINGS/ASPECT_RATIO_BTN", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnAspectRatioM = _addPicButton("GAME_SETTINGS/ASPECT_RATIO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_gameSettingsBtnAspectRatioP = _addPicButton("GAME_SETTINGS/ASPECT_RATIO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);

//GC Nintendont Page 3
	//nintendont = wiiu widescreen, emu_memcard, widescreen, mng_cvr_bnr (pg4)
	m_gameSettingsLblWiiuWidescreen = _addLabel("GAME_SETTINGS/WIIU_WIDESCREEN", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnWiiuWidescreen = _addButton("GAME_SETTINGS/WIIU_WIDESCREEN_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblEmuMemCard = _addLabel("GAME_SETTINGS/DML_NMM", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblEmuMemCard_Val = _addLabel("GAME_SETTINGS/DML_NMM_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnEmuMemCard_M = _addPicButton("GAME_SETTINGS/DML_NMM_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_gameSettingsBtnEmuMemCard_P = _addPicButton("GAME_SETTINGS/DML_NMM_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);

	m_gameSettingsLblWidescreen = _addLabel("GAME_SETTINGS/WIDESCREEN", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnWidescreen = _addButton("GAME_SETTINGS/WIDESCREEN_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

//Page 4
	m_gameSettingsLblEmulation = _addLabel("GAME_SETTINGS/EMU_SAVE", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblEmulationVal = _addLabel("GAME_SETTINGS/EMU_SAVE_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnEmulationM = _addPicButton("GAME_SETTINGS/EMU_SAVE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_gameSettingsBtnEmulationP = _addPicButton("GAME_SETTINGS/EMU_SAVE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);

	m_gameSettingsLblExtractSave = _addLabel("GAME_SETTINGS/EXTRACT_SAVE", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnExtractSave = _addButton("GAME_SETTINGS/EXTRACT_SAVE_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_gameSettingsLblFlashSave = _addLabel("GAME_SETTINGS/FLASH_SAVE", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnFlashSave = _addButton("GAME_SETTINGS/FLASH_SAVE_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

	m_gameSettingsLblManage = _addLabel("GAME_SETTINGS/MANAGE", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnManage = _addButton("GAME_SETTINGS/MANAGE_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

//Channels Page 4
	m_gameSettingsLblCustom = _addLabel("GAME_SETTINGS/CUSTOM", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCustom = _addButton("GAME_SETTINGS/CUSTOM_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblApploader = _addLabel("GAME_SETTINGS/APPLDR", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnApploader = _addButton("GAME_SETTINGS/APPLDR_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_gameSettingsLblLaunchNK = _addLabel("GAME_SETTINGS/LAUNCHNEEK", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnLaunchNK = _addButton("GAME_SETTINGS/LAUNCHNEEK_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

//GC Nintendont Page 4
	m_gameSettingsLblCC_Rumble = _addLabel("GAME_SETTINGS/CC_RUMBLE", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCC_Rumble = _addButton("GAME_SETTINGS/CC_RUMBLE_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblNATIVE_CTL = _addLabel("GAME_SETTINGS/NATIVE_CTL", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnNATIVE_CTL = _addButton("GAME_SETTINGS/NATIVE_CTL_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_gameSettingsLblArcade = _addLabel("GAME_SETTINGS/ARCADE", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnArcade = _addButton("GAME_SETTINGS/ARCADE_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

	m_gameSettingsLblSkip_IPL = _addLabel("GAME_SETTINGS/SKIP_IPL", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnSkip_IPL = _addButton("GAME_SETTINGS/SKIP_IPL_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

//Page 5
	m_gameSettingsLblLED = _addLabel("GAME_SETTINGS/LED", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnLED = _addButton("GAME_SETTINGS/LED_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblPrivateServer = _addLabel("GAME_SETTINGS/PRIVATE_SERVER", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblPrivateServerVal = _addLabel("GAME_SETTINGS/PRIVATE_SERVER_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPrivateServerM = _addPicButton("GAME_SETTINGS/PRIVATE_SERVER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_gameSettingsBtnPrivateServerP = _addPicButton("GAME_SETTINGS/PRIVATE_SERVER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	
	m_gameSettingsLblFix480p = _addLabel("GAME_SETTINGS/FIX480P", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnFix480p = _addButton("GAME_SETTINGS/FIX480P_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

	m_gameSettingsLblDeflickerWii = _addLabel("GAME_SETTINGS/DEFLICKER_WII", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblDeflickerWiiVal = _addLabel("GAME_SETTINGS/DEFLICKER_WII_BTN", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnDeflickerWiiM = _addPicButton("GAME_SETTINGS/DEFLICKER_WII_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_gameSettingsBtnDeflickerWiiP = _addPicButton("GAME_SETTINGS/DEFLICKER_WII_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);

	//GC
	m_gameSettingsLblWidth = _addLabel("GAME_SETTINGS/NIN_WIDTH", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblWidthVal = _addLabel("GAME_SETTINGS/NIN_WIDTH_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnWidthM = _addPicButton("GAME_SETTINGS/NIN_WIDTH_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_gameSettingsBtnWidthP = _addPicButton("GAME_SETTINGS/NIN_WIDTH_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);

	m_gameSettingsLblPos = _addLabel("GAME_SETTINGS/NIN_POS", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblPosVal = _addLabel("GAME_SETTINGS/NIN_POS_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPosM = _addPicButton("GAME_SETTINGS/NIN_POS_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_gameSettingsBtnPosP = _addPicButton("GAME_SETTINGS/NIN_POS_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);

	m_gameSettingsLblPatch50 = _addLabel("GAME_SETTINGS/PATCH_PAL50", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnPatch50 = _addButton("GAME_SETTINGS/PATCH_PAL50_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

//Page 6 GC
	m_gameSettingsLblBBA = _addLabel("GAME_SETTINGS/BBA", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnBBA = _addButton("GAME_SETTINGS/BBA_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);
	
	m_gameSettingsLblNetProf = _addLabel("GAME_SETTINGS/NET_PROFILE", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblNetProfVal = _addLabel("GAME_SETTINGS/NET_PROFILE_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnNetProfM = _addPicButton("GAME_SETTINGS/NET_PROFILE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_gameSettingsBtnNetProfP = _addPicButton("GAME_SETTINGS/NET_PROFILE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	
	//wii
	m_gameSettingsLblWidescreenWiiu = _addLabel("GAME_SETTINGS/WIDESCREEN_WIIU", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblWidescreenWiiuVal = _addLabel("GAME_SETTINGS/WIDESCREEN_WIIU_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnWidescreenWiiuM = _addPicButton("GAME_SETTINGS/WIDESCREEN_WIIU_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_gameSettingsBtnWidescreenWiiuP = _addPicButton("GAME_SETTINGS/WIDESCREEN_WIIU_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	
//Footer
	m_gameSettingsLblPage = _addLabel("GAME_SETTINGS/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPageM = _addPicButton("GAME_SETTINGS/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_gameSettingsBtnPageP = _addPicButton("GAME_SETTINGS/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_gameSettingsBtnBack = _addButton("GAME_SETTINGS/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

// Hide Animations
	_setHideAnim(m_gameSettingsLblTitle, "GAME_SETTINGS/TITLE", 0, 0, -2.f, 0.f);

	_setHideAnim(m_gameSettingsLblGameVideo, "GAME_SETTINGS/VIDEO", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblVideo, "GAME_SETTINGS/VIDEO_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoM, "GAME_SETTINGS/VIDEO_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoP, "GAME_SETTINGS/VIDEO_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblCustom, "GAME_SETTINGS/CUSTOM", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCustom, "GAME_SETTINGS/CUSTOM_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblLaunchNK, "GAME_SETTINGS/LAUNCHNEEK", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLaunchNK, "GAME_SETTINGS/LAUNCHNEEK_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblGameLanguage, "GAME_SETTINGS/GAME_LANG", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblLanguage, "GAME_SETTINGS/GAME_LANG_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageM, "GAME_SETTINGS/GAME_LANG_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageP, "GAME_SETTINGS/GAME_LANG_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblOcarina, "GAME_SETTINGS/OCARINA", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnOcarina, "GAME_SETTINGS/OCARINA_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblCheat, "GAME_SETTINGS/CHEAT", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCheat, "GAME_SETTINGS/CHEAT_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblPrivateServer, "GAME_SETTINGS/PRIVATE_SERVER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblPrivateServerVal, "GAME_SETTINGS/PRIVATE_SERVER_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPrivateServerM, "GAME_SETTINGS/PRIVATE_SERVER_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPrivateServerP, "GAME_SETTINGS/PRIVATE_SERVER_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblFix480p, "GAME_SETTINGS/FIX480P", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnFix480p, "GAME_SETTINGS/FIX480P_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblDeflickerWii, "GAME_SETTINGS/DEFLICKER_WII", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblDeflickerWiiVal, "GAME_SETTINGS/DEFLICKER_WII_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDeflickerWiiM, "GAME_SETTINGS/DEFLICKER_WII_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDeflickerWiiP, "GAME_SETTINGS/DEFLICKER_WII_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblVipatch, "GAME_SETTINGS/VIPATCH", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVipatch, "GAME_SETTINGS/VIPATCH_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblGameIOS, "GAME_SETTINGS/IOS", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblIOS, "GAME_SETTINGS/IOS_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnIOSM, "GAME_SETTINGS/IOS_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnIOSP, "GAME_SETTINGS/IOS_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblManage, "GAME_SETTINGS/MANAGE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnManage, "GAME_SETTINGS/MANAGE_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblPatchVidModes, "GAME_SETTINGS/PATCH_VIDEO_MODE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModesVal, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesM, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesP, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblAspectRatio, "GAME_SETTINGS/ASPECT_RATIO", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblAspectRatioVal, "GAME_SETTINGS/ASPECT_RATIO_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnAspectRatioP, "GAME_SETTINGS/ASPECT_RATIO_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnAspectRatioM, "GAME_SETTINGS/ASPECT_RATIO_MINUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblApploader, "GAME_SETTINGS/APPLDR", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnApploader, "GAME_SETTINGS/APPLDR_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblLED, "GAME_SETTINGS/LED", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLED, "GAME_SETTINGS/LED_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_gameSettingsLblCC_Rumble, "GAME_SETTINGS/CC_RUMBLE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCC_Rumble, "GAME_SETTINGS/CC_RUMBLE_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_gameSettingsLblNATIVE_CTL, "GAME_SETTINGS/NATIVE_CTL", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnNATIVE_CTL, "GAME_SETTINGS/NATIVE_CTL_BTN", -50, 0, 1.f, 0.f);	
	
	_setHideAnim(m_gameSettingsLblDeflicker, "GAME_SETTINGS/DEFLICKER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDeflicker, "GAME_SETTINGS/DEFLICKER_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblEmuMemCard, "GAME_SETTINGS/DML_NMM", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblEmuMemCard_Val, "GAME_SETTINGS/DML_NMM_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnEmuMemCard_P, "GAME_SETTINGS/DML_NMM_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnEmuMemCard_M, "GAME_SETTINGS/DML_NMM_MINUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblDevoMemcardEmu, "GAME_SETTINGS/DEVO_MEMCARD_EMU", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDevoMemcardEmu, "GAME_SETTINGS/DEVO_MEMCARD_EMU_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblWidescreen, "GAME_SETTINGS/WIDESCREEN", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnWidescreen, "GAME_SETTINGS/WIDESCREEN_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblWiiuWidescreen, "GAME_SETTINGS/WIIU_WIDESCREEN", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnWiiuWidescreen, "GAME_SETTINGS/WIIU_WIDESCREEN_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblWidescreenWiiu, "GAME_SETTINGS/WIDESCREEN_WIIU", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblWidescreenWiiuVal, "GAME_SETTINGS/WIDESCREEN_WIIU_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnWidescreenWiiuM, "GAME_SETTINGS/WIDESCREEN_WIIU_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnWidescreenWiiuP, "GAME_SETTINGS/WIDESCREEN_WIIU_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblArcade, "GAME_SETTINGS/ARCADE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnArcade, "GAME_SETTINGS/ARCADE_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblSkip_IPL, "GAME_SETTINGS/SKIP_IPL", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnSkip_IPL, "GAME_SETTINGS/SKIP_IPL_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblPatch50, "GAME_SETTINGS/PATCH_PAL50", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatch50, "GAME_SETTINGS/PATCH_PAL50_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblWidth, "GAME_SETTINGS/NIN_WIDTH", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblWidthVal, "GAME_SETTINGS/NIN_WIDTH_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnWidthM, "GAME_SETTINGS/NIN_WIDTH_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnWidthP, "GAME_SETTINGS/NIN_WIDTH_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblPos, "GAME_SETTINGS/NIN_POS", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblPosVal, "GAME_SETTINGS/NIN_POS_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPosM, "GAME_SETTINGS/NIN_POS_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPosP, "GAME_SETTINGS/NIN_POS_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblBBA, "GAME_SETTINGS/BBA", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnBBA, "GAME_SETTINGS/BBA_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblNetProf, "GAME_SETTINGS/NET_PROFILE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblNetProfVal, "GAME_SETTINGS/NET_PROFILE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnNetProfM, "GAME_SETTINGS/NET_PROFILE_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnNetProfP, "GAME_SETTINGS/NET_PROFILE_PLUS", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_gameSettingsLblGCLoader, "GAME_SETTINGS/GC_LOADER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblGCLoader_Val, "GAME_SETTINGS/GC_LOADER_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnGCLoader_P, "GAME_SETTINGS/GC_LOADER_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnGCLoader_M, "GAME_SETTINGS/GC_LOADER_MINUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblHooktype, "GAME_SETTINGS/HOOKTYPE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblHooktypeVal, "GAME_SETTINGS/HOOKTYPE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnHooktypeM, "GAME_SETTINGS/HOOKTYPE_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnHooktypeP, "GAME_SETTINGS/HOOKTYPE_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblEmulation, "GAME_SETTINGS/EMU_SAVE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblEmulationVal, "GAME_SETTINGS/EMU_SAVE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnEmulationP, "GAME_SETTINGS/EMU_SAVE_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnEmulationM, "GAME_SETTINGS/EMU_SAVE_MINUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblDebugger, "GAME_SETTINGS/GAME_DEBUGGER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsLblDebuggerV, "GAME_SETTINGS/GAME_DEBUGGER_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDebuggerM, "GAME_SETTINGS/GAME_DEBUGGER_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDebuggerP, "GAME_SETTINGS/GAME_DEBUGGER_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblExtractSave, "GAME_SETTINGS/EXTRACT_SAVE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnExtractSave, "GAME_SETTINGS/EXTRACT_SAVE_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblFlashSave, "GAME_SETTINGS/FLASH_SAVE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnFlashSave, "GAME_SETTINGS/FLASH_SAVE_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblAdultOnly, "GAME_SETTINGS/ADULT_ONLY", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnAdultOnly, "GAME_SETTINGS/ADULT_ONLY_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_gameSettingsLblPage, "GAME_SETTINGS/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameSettingsBtnPageM, "GAME_SETTINGS/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameSettingsBtnPageP, "GAME_SETTINGS/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameSettingsBtnBack, "GAME_SETTINGS/BACK_BTN", 0, 0, 1.f, -1.f);

	_hideGameSettings(true);
	_textGameSettings();
}

void CMenu::_textGameSettings(void)
{
	m_btnMgr.setText(m_gameSettingsLblTitle, _t("cfgg1", L"Settings"));
	m_btnMgr.setText(m_gameSettingsBtnBack, _t("cfgg8", L"Back"));
	
	m_btnMgr.setText(m_gameSettingsLblManage, _t("cfgg40", L"Manage Cover and Banner"));
	m_btnMgr.setText(m_gameSettingsBtnManage, _t("cfgg41", L"Manage"));
	m_btnMgr.setText(m_gameSettingsLblAdultOnly, _t("cfgg58", L"Adult only"));
	m_btnMgr.setText(m_gameSettingsLblGCLoader, _t("cfgg35", L"GameCube Loader"));
	m_btnMgr.setText(m_gameSettingsLblGameVideo, _t("cfgg2", L"Video mode"));
	m_btnMgr.setText(m_gameSettingsLblGameLanguage, _t("cfgg3", L"Language"));
	
	m_btnMgr.setText(m_gameSettingsLblHooktype, _t("cfgg18", L"Hook Type"));
	m_btnMgr.setText(m_gameSettingsLblDebugger, _t("cfgg22", L"Debugger"));
	m_btnMgr.setText(m_gameSettingsLblOcarina, _t("cfgg5", L"Ocarina"));
	m_btnMgr.setText(m_gameSettingsLblCheat, _t("cfgg15", L"Cheat Codes"));
	m_btnMgr.setText(m_gameSettingsBtnCheat, _t("cfgg16", L"Select"));
	m_btnMgr.setText(m_gameSettingsLblDeflicker, _t("cfgg44", L"Video Deflicker"));
	m_btnMgr.setText(m_gameSettingsLblDevoMemcardEmu, _t("cfgg47", L"Emulated MemCard"));
	m_btnMgr.setText(m_gameSettingsLblWidescreen, _t("cfgg36", L"Widescreen Patch"));
	m_btnMgr.setText(m_gameSettingsLblWiiuWidescreen, _t("cfgg46", L"WiiU Widescreen"));
	m_btnMgr.setText(m_gameSettingsLblWidescreenWiiu, _t("cfgg46", L"WiiU Widescreen"));
	
	m_btnMgr.setText(m_gameSettingsLblCountryPatch, _t("cfgg4", L"Patch country strings"));
	m_btnMgr.setText(m_gameSettingsLblVipatch, _t("cfgg7", L"Vipatch"));
	m_btnMgr.setText(m_gameSettingsLblPatchVidModes, _t("cfgg14", L"Patch video modes"));
	m_btnMgr.setText(m_gameSettingsLblAspectRatio, _t("cfgg27", L"Aspect Ratio"));
	m_btnMgr.setText(m_gameSettingsLblApploader, _t("cfgg37", L"Boot Apploader"));	
	m_btnMgr.setText(m_gameSettingsLblEmuMemCard, _t("cfgg47", L"Emulated MemCard"));
	m_btnMgr.setText(m_gameSettingsLblCC_Rumble, _t("cfgg52", L"Wiimote CC Rumble"));
	m_btnMgr.setText(m_gameSettingsLblNATIVE_CTL, _t("cfgg43", L"Native Control"));
	m_btnMgr.setText(m_gameSettingsLblSkip_IPL, _t("cfgg53", L"Skip IPL BIOS"));
	m_btnMgr.setText(m_gameSettingsLblPatch50, _t("cfgg56", L"Patch PAL50"));
	m_btnMgr.setText(m_gameSettingsLblBBA, _t("cfgg59", L"BBA Emulation"));
	m_btnMgr.setText(m_gameSettingsLblNetProf, _t("cfgg60", L"BBA Net Profile"));
	
	m_btnMgr.setText(m_gameSettingsLblArcade, _t("cfgg48", L"Triforce Arcade Mode"));
	m_btnMgr.setText(m_gameSettingsLblEmulation, _t("cfgg24", L"NAND Emulation"));
	m_btnMgr.setText(m_gameSettingsLblGameIOS, _t("cfgg10", L"IOS"));
	m_btnMgr.setText(m_gameSettingsLblLED, _t("cfgg38", L"Activity LED"));
	m_btnMgr.setText(m_gameSettingsLblExtractSave, _t("cfgg30", L"Extract Save from NAND"));
	m_btnMgr.setText(m_gameSettingsBtnExtractSave, _t("cfgg31", L"Extract"));
	m_btnMgr.setText(m_gameSettingsLblCustom, _t("custom", L"Custom"));
	m_btnMgr.setText(m_gameSettingsLblLaunchNK, _t("neek1", L"Launch Title with neek2o"));

	m_btnMgr.setText(m_gameSettingsLblFlashSave, _t("cfgg32", L"Flash Save to NAND"));
	m_btnMgr.setText(m_gameSettingsBtnFlashSave, _t("cfgg33", L"Flash"));
	m_btnMgr.setText(m_gameSettingsLblPrivateServer, _t("cfgg45", L"Private Server"));
	m_btnMgr.setText(m_gameSettingsLblFix480p, _t("cfgg49", L"480p Pixel Patch"));
	m_btnMgr.setText(m_gameSettingsLblWidth, _t("cfgg54", L"Video Width"));
	m_btnMgr.setText(m_gameSettingsLblPos, _t("cfgg55", L"Video Position"));
	m_btnMgr.setText(m_gameSettingsLblDeflickerWii, _t("cfgg61", L"Deflicker Filter"));
}
