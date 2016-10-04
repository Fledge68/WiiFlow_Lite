
#include "menu.hpp"
#include "types.h"
#include "loader/wbfs.h"
#include "libwbfs/wiidisc.h"

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

u8 m_gameSettingsMaxPgs = 5;
u8 m_gameSettingsPage = 0;
u8 GCLoader = 0;

void CMenu::_hideGameSettings(bool instant)
{
	m_btnMgr.hide(m_gameSettingsLblPage, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnBack, instant);
	m_btnMgr.hide(m_gameSettingsLblTitle, instant);
	m_btnMgr.hide(m_gameSettingsLblManage, instant);
	m_btnMgr.hide(m_gameSettingsBtnManage, instant);
	m_btnMgr.hide(m_gameSettingsBtnCategoryMain, instant);
	m_btnMgr.hide(m_gameSettingsLblCategoryMain, instant);
	m_btnMgr.hide(m_gameSettingsLblHooktype, instant);
	m_btnMgr.hide(m_gameSettingsLblHooktypeVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnHooktypeM, instant);
	m_btnMgr.hide(m_gameSettingsBtnHooktypeP, instant);
	m_btnMgr.hide(m_gameSettingsLblVipatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnVipatch, instant);
	m_btnMgr.hide(m_gameSettingsLblCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsLblPrivateServer, instant);
	m_btnMgr.hide(m_gameSettingsBtnPrivateServer, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModes, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP, instant);
	m_btnMgr.hide(m_gameSettingsLblAspectRatio, instant);
	m_btnMgr.hide(m_gameSettingsLblAspectRatioVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnAspectRatioP, instant);
	m_btnMgr.hide(m_gameSettingsBtnAspectRatioM, instant);
	m_btnMgr.hide(m_gameSettingsLblEmulationVal, instant);
	m_btnMgr.hide(m_gameSettingsLblEmulation, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmulationP, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmulationM, instant);
	m_btnMgr.hide(m_gameSettingsLblGameIOS, instant);
	m_btnMgr.hide(m_gameSettingsLblIOS, instant);
	m_btnMgr.hide(m_gameSettingsBtnIOSP, instant);
	m_btnMgr.hide(m_gameSettingsBtnIOSM, instant);
	m_btnMgr.hide(m_gameSettingsLblExtractSave, instant);
	m_btnMgr.hide(m_gameSettingsBtnExtractSave, instant);
	m_btnMgr.hide(m_gameSettingsLblFlashSave, instant);
	m_btnMgr.hide(m_gameSettingsBtnFlashSave, instant);
	m_btnMgr.hide(m_gameSettingsLblApploader, instant);
	m_btnMgr.hide(m_gameSettingsBtnApploader, instant);
	//shared
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
	m_btnMgr.hide(m_gameSettingsLblGCLoader, instant);
	m_btnMgr.hide(m_gameSettingsLblGCLoader_Val, instant);
	m_btnMgr.hide(m_gameSettingsBtnGCLoader_P, instant);
	m_btnMgr.hide(m_gameSettingsBtnGCLoader_M, instant);
	m_btnMgr.hide(m_gameSettingsLblUSB_HID, instant);
	m_btnMgr.hide(m_gameSettingsBtnUSB_HID, instant);
	m_btnMgr.hide(m_gameSettingsLblNATIVE_CTL, instant);
	m_btnMgr.hide(m_gameSettingsBtnNATIVE_CTL, instant);
	m_btnMgr.hide(m_gameSettingsLblDeflicker, instant);
	m_btnMgr.hide(m_gameSettingsBtnDeflicker, instant);
	m_btnMgr.hide(m_gameSettingsLblCustom, instant);
	m_btnMgr.hide(m_gameSettingsBtnCustom, instant);
	m_btnMgr.hide(m_gameSettingsLblLaunchNK, instant);
	m_btnMgr.hide(m_gameSettingsBtnLaunchNK, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if(m_gameSettingsLblUser[i] != -1)
			m_btnMgr.hide(m_gameSettingsLblUser[i], instant);
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

void CMenu::_hideGameSettingsPg(void)
{
	//Does not hide title, page, back, or user labels
	if(m_gameSettingsPage == 1)
	{
		m_btnMgr.hide(m_gameSettingsLblManage);
		m_btnMgr.hide(m_gameSettingsBtnManage);

		m_btnMgr.hide(m_gameSettingsLblGCLoader);
		m_btnMgr.hide(m_gameSettingsLblGCLoader_Val);
		m_btnMgr.hide(m_gameSettingsBtnGCLoader_P);
		m_btnMgr.hide(m_gameSettingsBtnGCLoader_M);
		
		m_btnMgr.hide(m_gameSettingsBtnCategoryMain);
		m_btnMgr.hide(m_gameSettingsLblCategoryMain);
		
		m_btnMgr.hide(m_gameSettingsLblGameLanguage);
		m_btnMgr.hide(m_gameSettingsLblLanguage);
		m_btnMgr.hide(m_gameSettingsBtnLanguageP);
		m_btnMgr.hide(m_gameSettingsBtnLanguageM);

		m_btnMgr.hide(m_gameSettingsLblGameVideo);
		m_btnMgr.hide(m_gameSettingsLblVideo);
		m_btnMgr.hide(m_gameSettingsBtnVideoP);
		m_btnMgr.hide(m_gameSettingsBtnVideoM);
	}
	if(m_gameSettingsPage == 2)
	{
		m_btnMgr.hide(m_gameSettingsLblDebugger);
		m_btnMgr.hide(m_gameSettingsLblDebuggerV);
		m_btnMgr.hide(m_gameSettingsBtnDebuggerP);
		m_btnMgr.hide(m_gameSettingsBtnDebuggerM);
		
		m_btnMgr.hide(m_gameSettingsLblHooktype);
		m_btnMgr.hide(m_gameSettingsLblHooktypeVal);
		m_btnMgr.hide(m_gameSettingsBtnHooktypeM);
		m_btnMgr.hide(m_gameSettingsBtnHooktypeP);
		
		m_btnMgr.hide(m_gameSettingsLblDeflicker);
		m_btnMgr.hide(m_gameSettingsBtnDeflicker);
		
		m_btnMgr.hide(m_gameSettingsLblOcarina);
		m_btnMgr.hide(m_gameSettingsBtnOcarina);
		
		m_btnMgr.hide(m_gameSettingsLblCheat);
		m_btnMgr.hide(m_gameSettingsBtnCheat);
		
		m_btnMgr.hide(m_gameSettingsLblDevoMemcardEmu);
		m_btnMgr.hide(m_gameSettingsBtnDevoMemcardEmu);

		m_btnMgr.hide(m_gameSettingsLblWidescreen);
		m_btnMgr.hide(m_gameSettingsBtnWidescreen);

		m_btnMgr.hide(m_gameSettingsLblLED);
		m_btnMgr.hide(m_gameSettingsBtnLED);
	}
	if(m_gameSettingsPage == 3)
	{
		m_btnMgr.hide(m_gameSettingsLblPatchVidModes);
		m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal);
		m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM);
		m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP);

		m_btnMgr.hide(m_gameSettingsLblVipatch);
		m_btnMgr.hide(m_gameSettingsBtnVipatch);

		m_btnMgr.hide(m_gameSettingsLblCountryPatch);
		m_btnMgr.hide(m_gameSettingsBtnCountryPatch);

		m_btnMgr.hide(m_gameSettingsLblAspectRatio);
		m_btnMgr.hide(m_gameSettingsLblAspectRatioVal);
		m_btnMgr.hide(m_gameSettingsBtnAspectRatioP);
		m_btnMgr.hide(m_gameSettingsBtnAspectRatioM);

		m_btnMgr.hide(m_gameSettingsLblApploader);
		m_btnMgr.hide(m_gameSettingsBtnApploader);

		m_btnMgr.hide(m_gameSettingsLblEmuMemCard);
		m_btnMgr.hide(m_gameSettingsLblEmuMemCard_Val);
		m_btnMgr.hide(m_gameSettingsBtnEmuMemCard_P);
		m_btnMgr.hide(m_gameSettingsBtnEmuMemCard_M);

		m_btnMgr.hide(m_gameSettingsLblWidescreen);
		m_btnMgr.hide(m_gameSettingsBtnWidescreen);

		m_btnMgr.hide(m_gameSettingsLblUSB_HID);
		m_btnMgr.hide(m_gameSettingsBtnUSB_HID);
		
		m_btnMgr.hide(m_gameSettingsLblNATIVE_CTL);
		m_btnMgr.hide(m_gameSettingsBtnNATIVE_CTL);
	}
	if(m_gameSettingsPage == 4)
	{
		m_btnMgr.hide(m_gameSettingsLblCustom);
		m_btnMgr.hide(m_gameSettingsBtnCustom);

		m_btnMgr.hide(m_gameSettingsLblLaunchNK);
		m_btnMgr.hide(m_gameSettingsBtnLaunchNK);
		
		m_btnMgr.hide(m_gameSettingsLblEmulationVal);
		m_btnMgr.hide(m_gameSettingsLblEmulation);
		m_btnMgr.hide(m_gameSettingsBtnEmulationP);
		m_btnMgr.hide(m_gameSettingsBtnEmulationM);

		m_btnMgr.hide(m_gameSettingsLblExtractSave);
		m_btnMgr.hide(m_gameSettingsBtnExtractSave);

		m_btnMgr.hide(m_gameSettingsLblGameIOS);
		m_btnMgr.hide(m_gameSettingsLblIOS);
		m_btnMgr.hide(m_gameSettingsBtnIOSP);
		m_btnMgr.hide(m_gameSettingsBtnIOSM);
		
		m_btnMgr.hide(m_gameSettingsLblLED);
		m_btnMgr.hide(m_gameSettingsBtnLED);
	}
	if(m_gameSettingsPage == 5)
	{
		m_btnMgr.hide(m_gameSettingsLblFlashSave);
		m_btnMgr.hide(m_gameSettingsBtnFlashSave);
		
		m_btnMgr.hide(m_gameSettingsLblPrivateServer);
		m_btnMgr.hide(m_gameSettingsBtnPrivateServer);		
	}
}
	
void CMenu::_showGameSettings(void)
{
	const char *id = CoverFlow.getId();
	const dir_discHdr *GameHdr = CoverFlow.getHdr();
	
	_setBg(m_gameSettingsBg, m_gameSettingsBg);
	m_btnMgr.show(m_gameSettingsLblPage);
	m_btnMgr.show(m_gameSettingsBtnPageM);
	m_btnMgr.show(m_gameSettingsBtnPageP);
	m_btnMgr.show(m_gameSettingsBtnBack);
	
	wstringEx title(_t("cfgg1", L"Settings"));
	title.append(wfmt(L" [%.6s]", id));
	m_btnMgr.setText(m_gameSettingsLblTitle, title);
	m_btnMgr.show(m_gameSettingsLblTitle);

	if(GameHdr->type == TYPE_GC_GAME)
	{
		GCLoader = min((u32)m_gcfg2.getInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
		GCLoader = (GCLoader == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "default_loader", 1), ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1u) : GCLoader-1;
	}
	
	m_gameSettingsMaxPgs = 4;
	if(GameHdr->type == TYPE_WII_GAME)
		m_gameSettingsMaxPgs = 5;
	else if(GameHdr->type == TYPE_GC_GAME && GCLoader == DEVOLUTION)
		m_gameSettingsMaxPgs = 2;
	else if(GameHdr->type == TYPE_GC_GAME && GCLoader != DEVOLUTION)
		m_gameSettingsMaxPgs = 3;

	if(m_gameSettingsPage == 1)
	{
		m_btnMgr.show(m_gameSettingsLblManage);
		m_btnMgr.show(m_gameSettingsBtnManage);
		
		if(GameHdr->type == TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblGCLoader);
			m_btnMgr.show(m_gameSettingsLblGCLoader_Val);
			m_btnMgr.show(m_gameSettingsBtnGCLoader_P);
			m_btnMgr.show(m_gameSettingsBtnGCLoader_M);
		}
		else
		{
			m_btnMgr.show(m_gameSettingsBtnCategoryMain);
			m_btnMgr.show(m_gameSettingsLblCategoryMain);
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
		//if it's not GC or it is but loader is not devolution
		if(GameHdr->type != TYPE_GC_GAME || (GameHdr->type == TYPE_GC_GAME && GCLoader != DEVOLUTION))
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
		else //devolution
		{
			m_btnMgr.show(m_gameSettingsLblDevoMemcardEmu);
			m_btnMgr.show(m_gameSettingsBtnDevoMemcardEmu);

			m_btnMgr.show(m_gameSettingsLblWidescreen);
			m_btnMgr.show(m_gameSettingsBtnWidescreen);
			
			m_btnMgr.show(m_gameSettingsLblLED);
			m_btnMgr.show(m_gameSettingsBtnLED);
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

			if(GameHdr->type == TYPE_WII_GAME)
			{
				m_btnMgr.show(m_gameSettingsLblAspectRatio);
				m_btnMgr.show(m_gameSettingsLblAspectRatioVal);
				m_btnMgr.show(m_gameSettingsBtnAspectRatioP);
				m_btnMgr.show(m_gameSettingsBtnAspectRatioM);
			}
			else if(GameHdr->type == TYPE_CHANNEL || GameHdr->type == TYPE_EMUCHANNEL)
			{
				m_btnMgr.show(m_gameSettingsLblApploader);
				m_btnMgr.show(m_gameSettingsBtnApploader);
			}
		}
		else if(GCLoader == NINTENDONT)
		{
			m_btnMgr.show(m_gameSettingsLblEmuMemCard);
			m_btnMgr.show(m_gameSettingsLblEmuMemCard_Val);
			m_btnMgr.show(m_gameSettingsBtnEmuMemCard_P);
			m_btnMgr.show(m_gameSettingsBtnEmuMemCard_M);

			m_btnMgr.show(m_gameSettingsLblWidescreen);
			m_btnMgr.show(m_gameSettingsBtnWidescreen);
			
			m_btnMgr.show(m_gameSettingsLblUSB_HID);
			m_btnMgr.show(m_gameSettingsBtnUSB_HID);
			
		 	m_btnMgr.show(m_gameSettingsLblNATIVE_CTL);
			m_btnMgr.show(m_gameSettingsBtnNATIVE_CTL);
		}
	}
	if(m_gameSettingsPage == 4)
	{
		if(GameHdr->type == TYPE_CHANNEL || GameHdr->type == TYPE_EMUCHANNEL)
		{
			m_btnMgr.show(m_gameSettingsLblCustom);
			m_btnMgr.show(m_gameSettingsBtnCustom);
			
			if(m_cfg.getInt(CHANNEL_DOMAIN, "partition", 0) == 1)
			{
				m_btnMgr.show(m_gameSettingsLblLaunchNK);
				m_btnMgr.show(m_gameSettingsBtnLaunchNK);
			}
		}
		else if(GameHdr->type == TYPE_WII_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblEmulationVal);
			m_btnMgr.show(m_gameSettingsLblEmulation);
			m_btnMgr.show(m_gameSettingsBtnEmulationP);
			m_btnMgr.show(m_gameSettingsBtnEmulationM);
			
			if(_checkSave(string((const char *)GameHdr->id), true))
			{
				m_btnMgr.show(m_gameSettingsLblExtractSave);
				m_btnMgr.show(m_gameSettingsBtnExtractSave);
			}
		}
		
		m_btnMgr.show(m_gameSettingsLblGameIOS);
		m_btnMgr.show(m_gameSettingsLblIOS);
		m_btnMgr.show(m_gameSettingsBtnIOSP);
		m_btnMgr.show(m_gameSettingsBtnIOSM);
		
		m_btnMgr.show(m_gameSettingsLblLED);
		m_btnMgr.show(m_gameSettingsBtnLED);
	}
	if(m_gameSettingsPage == 5)
	{
		m_btnMgr.show(m_gameSettingsLblFlashSave);
		m_btnMgr.show(m_gameSettingsBtnFlashSave);
		
		m_btnMgr.show(m_gameSettingsLblPrivateServer);
		m_btnMgr.show(m_gameSettingsBtnPrivateServer);
	}

	u32 i = 0;
	for(i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if(m_gameSettingsLblUser[i] != -1)
			m_btnMgr.show(m_gameSettingsLblUser[i]);

	int page = m_gameSettingsPage;
	u32 maxpage = m_gameSettingsMaxPgs;

	m_btnMgr.setText(m_gameSettingsLblPage, wfmt(L"%i / %i", page, maxpage));
	m_btnMgr.setText(m_gameSettingsBtnOcarina, _optBoolToString(m_gcfg2.getOptBool(id, "cheat", 0)));
	m_btnMgr.setText(m_gameSettingsBtnLED, _optBoolToString(m_gcfg2.getOptBool(id, "led", 0)));
	
	i = min((u32)m_gcfg2.getInt(id, "debugger", 0), ARRAY_SIZE(CMenu::_debugger) - 1u);
	m_btnMgr.setText(m_gameSettingsLblDebuggerV, _t(CMenu::_debugger[i].id, CMenu::_debugger[i].text));
	
	if(GameHdr->type == TYPE_GC_GAME)
	{
		//widescreen is shared by all, in the future might seperate them
		m_btnMgr.setText(m_gameSettingsBtnWidescreen, _optBoolToString(m_gcfg2.getOptBool(id, "widescreen", m_cfg.getBool(GC_DOMAIN, "widescreen", 0))));
		m_btnMgr.setText(m_gameSettingsBtnDevoMemcardEmu, _optBoolToString(m_gcfg2.getOptBool(id, "devo_memcard_emu", 0)));
		m_btnMgr.setText(m_gameSettingsBtnUSB_HID, _optBoolToString(m_gcfg2.getOptBool(id, "USB_HID", m_cfg.getBool(GC_DOMAIN, "USB_HID", 0))));
		m_btnMgr.setText(m_gameSettingsBtnNATIVE_CTL, _optBoolToString(m_gcfg2.getOptBool(id, "NATIVE_CTL", m_cfg.getBool(GC_DOMAIN, "NATIVE_CTL", 0))));
		m_btnMgr.setText(m_gameSettingsBtnDeflicker, _optBoolToString(m_gcfg2.getOptBool(id, "Deflicker", m_cfg.getBool(GC_DOMAIN, "Deflicker", 0))));

		i = min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_GCvideoModes) - 1u);
		m_btnMgr.setText(m_gameSettingsLblVideo, _t(CMenu::_GCvideoModes[i].id, CMenu::_GCvideoModes[i].text));
		
		i = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
		m_btnMgr.setText(m_gameSettingsLblLanguage, _t(CMenu::_GClanguages[i].id, CMenu::_GClanguages[i].text));
		
		if(GCLoader == NINTENDONT && IsOnWiiU())
			m_btnMgr.setText(m_gameSettingsLblWidescreen, _t("cfgg46", L"WiiU Widescreen"));
		else
			m_btnMgr.setText(m_gameSettingsLblWidescreen, _t("cfgg36", L"Widescreen Patch"));
		
		if(GCLoader == NINTENDONT)
		{
			m_btnMgr.setText(m_gameSettingsLblEmuMemCard, _t("cfgg47", L"Emulated MemCard"));
			i = min((u32)m_gcfg2.getInt(id, "nin_memcard", 0), ARRAY_SIZE(CMenu::_NinEmuCard) - 1u);
			m_btnMgr.setText(m_gameSettingsLblEmuMemCard_Val, _t(CMenu::_NinEmuCard[i].id, CMenu::_NinEmuCard[i].text));
		}
		
		i = min((u32)m_gcfg2.getInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
		m_btnMgr.setText(m_gameSettingsLblGCLoader_Val, _t(CMenu::_GCLoader[i].id, CMenu::_GCLoader[i].text));
	}
	else
	{
		m_btnMgr.setText(m_gameSettingsBtnVipatch, _optBoolToString(m_gcfg2.getOptBool(id, "vipatch", 0)));
		m_btnMgr.setText(m_gameSettingsBtnCountryPatch, _optBoolToString(m_gcfg2.getOptBool(id, "country_patch", 0)));
		m_btnMgr.setText(m_gameSettingsBtnPrivateServer, _optBoolToString(m_gcfg2.getOptBool(id, "private_server", m_cfg.getBool(WII_DOMAIN, "private_server", 0))));
		m_btnMgr.setText(m_gameSettingsBtnCustom, _optBoolToString(m_gcfg2.getOptBool(id, "custom", 0)));
		m_btnMgr.setText(m_gameSettingsBtnLaunchNK, _optBoolToString(m_gcfg2.getOptBool(id, "useneek", m_cfg.getBool(CHANNEL_DOMAIN, "useneek", 0))));
		m_btnMgr.setText(m_gameSettingsBtnApploader, _optBoolToString(m_gcfg2.getOptBool(id, "apploader", 0)));
		
		i = min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
		m_btnMgr.setText(m_gameSettingsLblVideo, _t(CMenu::_VideoModes[i].id, CMenu::_VideoModes[i].text));
		
		i = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
		m_btnMgr.setText(m_gameSettingsLblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
		
		i = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u);
		m_btnMgr.setText(m_gameSettingsLblAspectRatioVal, _t(CMenu::_AspectRatio[i].id, CMenu::_AspectRatio[i].text));

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

		i = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
		m_btnMgr.setText(m_gameSettingsLblPatchVidModesVal, _t(CMenu::_vidModePatch[i].id, CMenu::_vidModePatch[i].text));

		i = min((u32)m_gcfg2.getInt(id, "hooktype", 0), ARRAY_SIZE(CMenu::_hooktype) - 1u);
		m_btnMgr.setText(m_gameSettingsLblHooktypeVal, _t(CMenu::_hooktype[i].id, CMenu::_hooktype[i].text));

		i = min((u32)m_gcfg2.getInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);
		m_btnMgr.setText(m_gameSettingsLblEmulationVal, _t(CMenu::_SaveEmu[i].id, CMenu::_SaveEmu[i].text));
	}
}

void CMenu::_gameSettings(void)
{
	m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
	const char *id = CoverFlow.getId();
	const dir_discHdr *GameHdr = CoverFlow.getHdr();
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
		if((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageM))) && !m_locked)
		{
			_hideGameSettingsPg();
			if(m_gameSettingsPage == 1)
				m_gameSettingsPage = m_gameSettingsMaxPgs;
			else --m_gameSettingsPage;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_gameSettingsBtnPageM);
			_showGameSettings();
		}
		else if((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageP))) && !m_locked)
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
				int intoption = loopNum(m_gcfg2.getBool(id, "cheat") + 1, 3);
				if(intoption > 1)
					m_gcfg2.remove(id, "cheat");
				else
					m_gcfg2.setOptBool(id, "cheat", intoption);
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
				m_gcfg2.setBool(id, "devo_memcard_emu", !m_gcfg2.getBool(id, "devo_memcard_emu", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnWidescreen))
			{
				if(IsOnWiiU() and GCLoader == NINTENDONT)
					m_gcfg2.setBool(id, "wiiu_widescreen", !m_gcfg2.getBool(id, "wiiu_widescreen", 0));
				else
					m_gcfg2.setBool(id, "widescreen", !m_gcfg2.getBool(id, "widescreen", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnLanguageP) || m_btnMgr.selected(m_gameSettingsBtnLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnLanguageP) ? 1 : -1;
				if(GameHdr->type == TYPE_GC_GAME)
					m_gcfg2.setInt(id, "language", (int)loopNum((u32)m_gcfg2.getInt(id, "language", 0) + direction, ARRAY_SIZE(CMenu::_GClanguages)));
				else
					m_gcfg2.setInt(id, "language", (int)loopNum((u32)m_gcfg2.getInt(id, "language", 0) + direction, ARRAY_SIZE(CMenu::_languages)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnVideoP) || m_btnMgr.selected(m_gameSettingsBtnVideoM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnVideoP) ? 1 : -1;
				if(GameHdr->type == TYPE_GC_GAME)
					m_gcfg2.setInt(id, "video_mode", (int)loopNum((u32)m_gcfg2.getInt(id, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_GCvideoModes)));
				else
					m_gcfg2.setInt(id, "video_mode", (int)loopNum((u32)m_gcfg2.getInt(id, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_VideoModes)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnAspectRatioP) || m_btnMgr.selected(m_gameSettingsBtnAspectRatioM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnAspectRatioP) ? 1 : -1;
				m_gcfg2.setInt(id, "aspect_ratio", (int)loopNum((u32)m_gcfg2.getInt(id, "aspect_ratio", 0) + direction, ARRAY_SIZE(CMenu::_AspectRatio)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnEmuMemCard_P) || m_btnMgr.selected(m_gameSettingsBtnEmuMemCard_M))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnEmuMemCard_P) ? 1 : -1;
				m_gcfg2.setInt(id, "nin_memcard", (int)loopNum((u32)m_gcfg2.getInt(id, "nin_memcard", 2) + direction, ARRAY_SIZE(CMenu::_NinEmuCard)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnGCLoader_P) || m_btnMgr.selected(m_gameSettingsBtnGCLoader_M))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnGCLoader_P) ? 1 : -1;
				m_gcfg2.setInt(id, "gc_loader", (int)loopNum((u32)m_gcfg2.getInt(id, "gc_loader", 0) + direction, ARRAY_SIZE(CMenu::_GCLoader)));
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
				m_gcfg2.setInt(id, "patch_video_modes", (int)loopNum((u32)m_gcfg2.getInt(id, "patch_video_modes", 0) + direction, ARRAY_SIZE(CMenu::_vidModePatch)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnManage))
			{
				CoverFlow.stopCoverLoader(true);
				_hideGameSettings();
				_CoverBanner();
				_showGameSettings();
				CoverFlow.startCoverLoader();
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
				m_gcfg2.setInt(id, "hooktype", (int)loopNum((u32)m_gcfg2.getInt(id, "hooktype", 1) + direction, ARRAY_SIZE(CMenu::_hooktype)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnEmulationP) || m_btnMgr.selected(m_gameSettingsBtnEmulationM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnEmulationP) ? 1 : -1;
				m_gcfg2.setInt(id, "emulate_save", (int)loopNum((u32)m_gcfg2.getInt(id, "emulate_save", 0) + direction, ARRAY_SIZE(CMenu::_SaveEmu)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnDebuggerP) || m_btnMgr.selected(m_gameSettingsBtnDebuggerM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnDebuggerP) ? 1 : -1;
				m_gcfg2.setInt(id, "debugger", (int)loopNum((u32)m_gcfg2.getInt(id, "debugger", 0) + direction, ARRAY_SIZE(CMenu::_debugger)));
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
				m_gcfg2.setBool(id, "NATIVE_CTL", !m_gcfg2.getBool(id, "NATIVE_CTL", 0));
				_showGameSettings();
			}	
			else if(m_btnMgr.selected(m_gameSettingsBtnDeflicker))
			{
				m_gcfg2.setBool(id, "Deflicker", !m_gcfg2.getBool(id, "Deflicker", 0));
				_showGameSettings();
			}	
			else if(m_btnMgr.selected(m_gameSettingsBtnUSB_HID))
			{
				m_gcfg2.setBool(id, "USB_HID", !m_gcfg2.getBool(id, "USB_HID", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnPrivateServer))
			{
				m_gcfg2.setBool(id, "private_server", !m_gcfg2.getBool(id, "private_server", 0));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnCategoryMain) && !m_locked)
			{
				_hideGameSettings();
				_CategorySettings(true);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnExtractSave))
			{
				_hideGameSettings();
				m_forceext = true;
				_AutoExtractSave(id);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnFlashSave))
			{
				_hideGameSettings();
				m_forceext = true;
				_FlashSave(id);
				_showGameSettings();
			}
		}
	}
	m_gcfg2.save(true);
	_hideGameSettings();
}

void CMenu::_initGameSettingsMenu()
{
	_addUserLabels(m_gameSettingsLblUser, ARRAY_SIZE(m_gameSettingsLblUser), "GAME_SETTINGS");
	m_gameSettingsBg = _texture("GAME_SETTINGS/BG", "texture", theme.bg, false);
	m_gameSettingsLblTitle = _addTitle("GAME_SETTINGS/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

// Page 1
	m_gameSettingsLblManage = _addLabel("GAME_SETTINGS/MANAGE", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnManage = _addButton("GAME_SETTINGS/MANAGE_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblCategoryMain = _addLabel("GAME_SETTINGS/CAT_MAIN", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategoryMain = _addButton("GAME_SETTINGS/CAT_MAIN_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

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
	//devolution = emucard, widescreen, led_activity(pg4), (blank)
	
	m_gameSettingsLblDevoMemcardEmu = _addLabel("GAME_SETTINGS/DEVO_MEMCARD_EMU", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnDevoMemcardEmu = _addButton("GAME_SETTINGS/DEVO_MEMCARD_EMU_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblDeflicker = _addLabel("GAME_SETTINGS/DEFLICKER", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnDeflicker = _addButton("GAME_SETTINGS/DEFLICKER_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_gameSettingsLblWidescreen = _addLabel("GAME_SETTINGS/WIDESCREEN", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnWidescreen = _addButton("GAME_SETTINGS/WIDESCREEN_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

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

//Channels Page 3 line 4
	m_gameSettingsLblApploader = _addLabel("GAME_SETTINGS/APPLDR", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnApploader = _addButton("GAME_SETTINGS/APPLDR_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

//GC Page 3
	//nintendont=emucard, widescreen(pg2)/wiiu widescreen, usb_hid, native_ctl
	//devo=nothing
	
	m_gameSettingsLblEmuMemCard = _addLabel("GAME_SETTINGS/DML_NMM", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblEmuMemCard_Val = _addLabel("GAME_SETTINGS/DML_NMM_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnEmuMemCard_M = _addPicButton("GAME_SETTINGS/DML_NMM_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_gameSettingsBtnEmuMemCard_P = _addPicButton("GAME_SETTINGS/DML_NMM_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);

	m_gameSettingsLblUSB_HID = _addLabel("GAME_SETTINGS/USB_HID", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnUSB_HID = _addButton("GAME_SETTINGS/USB_HID_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

	m_gameSettingsLblNATIVE_CTL = _addLabel("GAME_SETTINGS/NATIVE_CTL", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnNATIVE_CTL = _addButton("GAME_SETTINGS/NATIVE_CTL_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

//Page 4
	m_gameSettingsLblEmulation = _addLabel("GAME_SETTINGS/EMU_SAVE", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblEmulationVal = _addLabel("GAME_SETTINGS/EMU_SAVE_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnEmulationM = _addPicButton("GAME_SETTINGS/EMU_SAVE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_gameSettingsBtnEmulationP = _addPicButton("GAME_SETTINGS/EMU_SAVE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);

	m_gameSettingsLblGameIOS = _addLabel("GAME_SETTINGS/IOS", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblIOS = _addLabel("GAME_SETTINGS/IOS_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnIOSM = _addPicButton("GAME_SETTINGS/IOS_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_gameSettingsBtnIOSP = _addPicButton("GAME_SETTINGS/IOS_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);

	m_gameSettingsLblLED = _addLabel("GAME_SETTINGS/LED", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnLED = _addButton("GAME_SETTINGS/LED_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

	m_gameSettingsLblExtractSave = _addLabel("GAME_SETTINGS/EXTRACT_SAVE", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnExtractSave = _addButton("GAME_SETTINGS/EXTRACT_SAVE_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

//Channels Page 4 line 1 and line 4 but only if emu NAND on USB1
	m_gameSettingsLblCustom = _addLabel("GAME_SETTINGS/CUSTOM", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCustom =  _addButton("GAME_SETTINGS/CUSTOM_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblLaunchNK = _addLabel("GAME_SETTINGS/LAUNCHNEEK", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnLaunchNK =  _addButton("GAME_SETTINGS/LAUNCHNEEK_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

//Page 5
	m_gameSettingsLblFlashSave = _addLabel("GAME_SETTINGS/FLASH_SAVE", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnFlashSave = _addButton("GAME_SETTINGS/FLASH_SAVE_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_gameSettingsLblPrivateServer = _addLabel("GAME_SETTINGS/PRIVATE_SERVER", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnPrivateServer = _addButton("GAME_SETTINGS/PRIVATE_SERVER_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);


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
	_setHideAnim(m_gameSettingsBtnPrivateServer, "GAME_SETTINGS/PRIVATE_SERVER_BTN", -50, 0, 1.f, 0.f);

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
	
	_setHideAnim(m_gameSettingsLblUSB_HID, "GAME_SETTINGS/USB_HID", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnUSB_HID, "GAME_SETTINGS/USB_HID_BTN", -50, 0, 1.f, 0.f);
	
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

	_setHideAnim(m_gameSettingsLblWidescreen, "GAME_SETTINGS/DM_WIDESCREEN", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnWidescreen, "GAME_SETTINGS/DM_WIDESCREEN_BTN", -50, 0, 1.f, 0.f);

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

	_setHideAnim(m_gameSettingsLblCategoryMain, "GAME_SETTINGS/CAT_MAIN", 50, 0, -2.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCategoryMain, "GAME_SETTINGS/CAT_MAIN_BTN", -50, 0, 1.f, 0.f);

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
	m_btnMgr.setText(m_gameSettingsLblCategoryMain, _t("cfgg17", L"Categories"));
	m_btnMgr.setText(m_gameSettingsBtnCategoryMain, _t("cfgg16", L"Select"));
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
	
	m_btnMgr.setText(m_gameSettingsLblCountryPatch, _t("cfgg4", L"Patch country strings"));
	m_btnMgr.setText(m_gameSettingsLblVipatch, _t("cfgg7", L"Vipatch"));
	m_btnMgr.setText(m_gameSettingsLblPatchVidModes, _t("cfgg14", L"Patch video modes"));
	m_btnMgr.setText(m_gameSettingsLblAspectRatio, _t("cfgg27", L"Aspect Ratio"));
	m_btnMgr.setText(m_gameSettingsLblApploader, _t("cfgg37", L"Boot Apploader"));	
	m_btnMgr.setText(m_gameSettingsLblEmuMemCard, _t("cfgg47", L"Emulated MemCard"));
	m_btnMgr.setText(m_gameSettingsLblUSB_HID, _t("cfgg42", L"USB-HID Controller"));
	m_btnMgr.setText(m_gameSettingsLblNATIVE_CTL, _t("cfgg43", L"Native Control"));
	
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
}
