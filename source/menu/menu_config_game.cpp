
#include "menu.hpp"
#include "types.h"
#include "gecko/gecko.h"
#include "loader/wbfs.h"
#include "loader/sys.h"
#include "libwbfs/wiidisc.h"

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

u8 g_numGCfPages = 5;
u8 m_gameSettingsPage = 0;

void CMenu::_hideGameSettings(bool instant)
{
	m_btnMgr.hide(m_gameSettingsLblPage, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnBack, instant);
	m_btnMgr.hide(m_gameSettingsLblTitle, instant);
	m_btnMgr.hide(m_gameSettingsLblGameLanguage, instant);
	m_btnMgr.hide(m_gameSettingsLblLanguage, instant);
	m_btnMgr.hide(m_gameSettingsBtnLanguageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnLanguageM, instant);
	m_btnMgr.hide(m_gameSettingsLblGameVideo, instant);
	m_btnMgr.hide(m_gameSettingsLblVideo, instant);
	m_btnMgr.hide(m_gameSettingsBtnVideoP, instant);
	m_btnMgr.hide(m_gameSettingsBtnVideoM, instant);
	m_btnMgr.hide(m_gameSettingsLblDMLGameVideo, instant);
	m_btnMgr.hide(m_gameSettingsLblDMLVideo, instant);
	m_btnMgr.hide(m_gameSettingsBtnDMLVideoP, instant);
	m_btnMgr.hide(m_gameSettingsBtnDMLVideoM, instant);
	m_btnMgr.hide(m_gameSettingsLblGClanguage, instant);
	m_btnMgr.hide(m_gameSettingsLblGClanguageVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnGClanguageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnGClanguageM, instant);
	m_btnMgr.hide(m_gameSettingsLblAspectRatio, instant);
	m_btnMgr.hide(m_gameSettingsLblAspectRatioVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnAspectRatioP, instant);
	m_btnMgr.hide(m_gameSettingsBtnAspectRatioM, instant);
	m_btnMgr.hide(m_gameSettingsLblNMM, instant);
	m_btnMgr.hide(m_gameSettingsLblNMM_Val, instant);
	m_btnMgr.hide(m_gameSettingsBtnNMM_P, instant);
	m_btnMgr.hide(m_gameSettingsBtnNMM_M, instant);
	m_btnMgr.hide(m_gameSettingsLblNoDVD, instant);
	m_btnMgr.hide(m_gameSettingsLblNoDVD_Val, instant);
	m_btnMgr.hide(m_gameSettingsBtnNoDVD_P, instant);
	m_btnMgr.hide(m_gameSettingsBtnNoDVD_M, instant);
	m_btnMgr.hide(m_gameSettingsLblDevoMemcardEmu, instant);
	m_btnMgr.hide(m_gameSettingsBtnDevoMemcardEmu, instant);
	m_btnMgr.hide(m_gameSettingsLblDM_Widescreen, instant);
	m_btnMgr.hide(m_gameSettingsBtnDM_Widescreen, instant);
	m_btnMgr.hide(m_gameSettingsLblGCLoader, instant);
	m_btnMgr.hide(m_gameSettingsLblGCLoader_Val, instant);
	m_btnMgr.hide(m_gameSettingsBtnGCLoader_P, instant);
	m_btnMgr.hide(m_gameSettingsBtnGCLoader_M, instant);
	m_btnMgr.hide(m_gameSettingsLblCustom, instant);
	m_btnMgr.hide(m_gameSettingsBtnCustom, instant);
	m_btnMgr.hide(m_gameSettingsLblLaunchNK, instant);
	m_btnMgr.hide(m_gameSettingsBtnLaunchNK, instant);
	m_btnMgr.hide(m_gameSettingsLblOcarina, instant);
	m_btnMgr.hide(m_gameSettingsBtnOcarina, instant);
	m_btnMgr.hide(m_gameSettingsLblCheat, instant);
	m_btnMgr.hide(m_gameSettingsBtnCheat, instant);
	m_btnMgr.hide(m_gameSettingsLblVipatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnVipatch, instant);
	m_btnMgr.hide(m_gameSettingsLblCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsLblCover, instant);
	m_btnMgr.hide(m_gameSettingsBtnCover, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModes, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP, instant);
	m_btnMgr.hide(m_gameSettingsLblHooktype, instant);
	m_btnMgr.hide(m_gameSettingsLblHooktypeVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnHooktypeM, instant);
	m_btnMgr.hide(m_gameSettingsBtnHooktypeP, instant);
	m_btnMgr.hide(m_gameSettingsBtnCategoryMain, instant);
	m_btnMgr.hide(m_gameSettingsLblCategoryMain, instant);
	m_btnMgr.hide(m_gameSettingsLblEmulationVal, instant);
	m_btnMgr.hide(m_gameSettingsLblEmulation, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmulationP, instant);
	m_btnMgr.hide(m_gameSettingsBtnEmulationM, instant);
	m_btnMgr.hide(m_gameSettingsLblDebugger, instant);
	m_btnMgr.hide(m_gameSettingsLblDebuggerV, instant);
	m_btnMgr.hide(m_gameSettingsBtnDebuggerP, instant);
	m_btnMgr.hide(m_gameSettingsBtnDebuggerM, instant);
	m_btnMgr.hide(m_gameSettingsLblGameIOS, instant);
	m_btnMgr.hide(m_gameSettingsLblIOS, instant);
	m_btnMgr.hide(m_gameSettingsBtnIOSP, instant);
	m_btnMgr.hide(m_gameSettingsBtnIOSM, instant);
	m_btnMgr.hide(m_gameSettingsLblExtractSave, instant);
	m_btnMgr.hide(m_gameSettingsBtnExtractSave, instant);
	m_btnMgr.hide(m_gameSettingsLblFlashSave, instant);
	m_btnMgr.hide(m_gameSettingsBtnFlashSave, instant);

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

void CMenu::_showGameSettings(void)
{
	wstringEx title(_t("cfgg1", L"Settings"));
	title += L" [";
	title += wstringEx(m_cf.getId());
	title += L"]";
	m_btnMgr.setText(m_gameSettingsLblTitle, title);
	_setBg(m_gameSettingsBg, m_gameSettingsBg);
	m_btnMgr.show(m_gameSettingsLblPage);
	m_btnMgr.show(m_gameSettingsBtnPageM);
	m_btnMgr.show(m_gameSettingsBtnPageP);
	m_btnMgr.show(m_gameSettingsBtnBack);
	m_btnMgr.show(m_gameSettingsLblTitle);
	if(m_cf.getHdr()->type == TYPE_GC_GAME)
	{
		if(m_new_dml)
			g_numGCfPages = 3;
		else
			g_numGCfPages = 2;
	}
	else
	{
		if(m_current_view == COVERFLOW_USB && _checkSave(string((const char *)m_cf.getHdr()->id), false))
			g_numGCfPages = 5;
		else 
			g_numGCfPages = 4;
	}

	if (m_gameSettingsPage == 1)
	{
		m_btnMgr.show(m_gameSettingsLblCover);
		m_btnMgr.show(m_gameSettingsBtnCover);
		if(m_cf.getHdr()->type == TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblDMLGameVideo);
			m_btnMgr.show(m_gameSettingsLblDMLVideo);
			m_btnMgr.show(m_gameSettingsBtnDMLVideoP);
			m_btnMgr.show(m_gameSettingsBtnDMLVideoM);

			m_btnMgr.show(m_gameSettingsLblGClanguage);
			m_btnMgr.show(m_gameSettingsLblGClanguageVal);
			m_btnMgr.show(m_gameSettingsBtnGClanguageP);
			m_btnMgr.show(m_gameSettingsBtnGClanguageM);

			m_btnMgr.show(m_gameSettingsLblGCLoader);
			m_btnMgr.show(m_gameSettingsLblGCLoader_Val);
			m_btnMgr.show(m_gameSettingsBtnGCLoader_P);
			m_btnMgr.show(m_gameSettingsBtnGCLoader_M);
		}
		else
		{
			m_btnMgr.show(m_gameSettingsBtnCategoryMain);
			m_btnMgr.show(m_gameSettingsLblCategoryMain);

			m_btnMgr.show(m_gameSettingsLblGameLanguage);
			m_btnMgr.show(m_gameSettingsLblLanguage);
			m_btnMgr.show(m_gameSettingsBtnLanguageP);
			m_btnMgr.show(m_gameSettingsBtnLanguageM);

			m_btnMgr.show(m_gameSettingsLblGameVideo);
			m_btnMgr.show(m_gameSettingsLblVideo);
			m_btnMgr.show(m_gameSettingsBtnVideoP);
			m_btnMgr.show(m_gameSettingsBtnVideoM);
		}
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblCover);
		m_btnMgr.hide(m_gameSettingsBtnCover);
		if(m_cf.getHdr()->type == TYPE_GC_GAME)
		{
			m_btnMgr.hide(m_gameSettingsLblGClanguage);
			m_btnMgr.hide(m_gameSettingsLblGClanguageVal);
			m_btnMgr.hide(m_gameSettingsBtnGClanguageP);
			m_btnMgr.hide(m_gameSettingsBtnGClanguageM);

			m_btnMgr.hide(m_gameSettingsLblDMLGameVideo);
			m_btnMgr.hide(m_gameSettingsLblDMLVideo);
			m_btnMgr.hide(m_gameSettingsBtnDMLVideoP);
			m_btnMgr.hide(m_gameSettingsBtnDMLVideoM);

			m_btnMgr.hide(m_gameSettingsLblGCLoader);
			m_btnMgr.hide(m_gameSettingsLblGCLoader_Val);
			m_btnMgr.hide(m_gameSettingsBtnGCLoader_P);
			m_btnMgr.hide(m_gameSettingsBtnGCLoader_M);
		}
		else
		{
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
	}
	if (m_gameSettingsPage == 2)
	{
		m_btnMgr.show(m_gameSettingsLblDebugger);
		m_btnMgr.show(m_gameSettingsLblDebuggerV);
		m_btnMgr.show(m_gameSettingsBtnDebuggerP);
		m_btnMgr.show(m_gameSettingsBtnDebuggerM);

		if(m_cf.getHdr()->type != TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblHooktype);
			m_btnMgr.show(m_gameSettingsLblHooktypeVal);
			m_btnMgr.show(m_gameSettingsBtnHooktypeM);
			m_btnMgr.show(m_gameSettingsBtnHooktypeP);
		}

		m_btnMgr.show(m_gameSettingsLblOcarina);
		m_btnMgr.show(m_gameSettingsBtnOcarina);

		m_btnMgr.show(m_gameSettingsLblCheat);
		m_btnMgr.show(m_gameSettingsBtnCheat);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblDebugger);
		m_btnMgr.hide(m_gameSettingsLblDebuggerV);
		m_btnMgr.hide(m_gameSettingsBtnDebuggerP);
		m_btnMgr.hide(m_gameSettingsBtnDebuggerM);

		if(m_cf.getHdr()->type != TYPE_GC_GAME)
		{
			m_btnMgr.hide(m_gameSettingsLblHooktype);
			m_btnMgr.hide(m_gameSettingsLblHooktypeVal);
			m_btnMgr.hide(m_gameSettingsBtnHooktypeM);
			m_btnMgr.hide(m_gameSettingsBtnHooktypeP);
		}

		m_btnMgr.hide(m_gameSettingsLblOcarina);
		m_btnMgr.hide(m_gameSettingsBtnOcarina);

		m_btnMgr.hide(m_gameSettingsLblCheat);
		m_btnMgr.hide(m_gameSettingsBtnCheat);
	}
	if (m_gameSettingsPage == 3)
	{
		if(m_cf.getHdr()->type != TYPE_GC_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblPatchVidModes);
			m_btnMgr.show(m_gameSettingsLblPatchVidModesVal);
			m_btnMgr.show(m_gameSettingsBtnPatchVidModesM);
			m_btnMgr.show(m_gameSettingsBtnPatchVidModesP);
			m_btnMgr.show(m_gameSettingsLblVipatch);
			m_btnMgr.show(m_gameSettingsBtnVipatch);

			m_btnMgr.show(m_gameSettingsLblCountryPatch);
			m_btnMgr.show(m_gameSettingsBtnCountryPatch);

			if(m_cf.getHdr()->type == TYPE_WII_GAME)
			{
				m_btnMgr.show(m_gameSettingsLblAspectRatio);
				m_btnMgr.show(m_gameSettingsLblAspectRatioVal);
				m_btnMgr.show(m_gameSettingsBtnAspectRatioP);
				m_btnMgr.show(m_gameSettingsBtnAspectRatioM);
			}
		}
		else
		{
			m_btnMgr.show(m_gameSettingsLblNMM);
			m_btnMgr.show(m_gameSettingsLblNMM_Val);
			m_btnMgr.show(m_gameSettingsBtnNMM_P);
			m_btnMgr.show(m_gameSettingsBtnNMM_M);

			m_btnMgr.show(m_gameSettingsLblNoDVD);
			m_btnMgr.show(m_gameSettingsLblNoDVD_Val);
			m_btnMgr.show(m_gameSettingsBtnNoDVD_P);
			m_btnMgr.show(m_gameSettingsBtnNoDVD_M);

			m_btnMgr.show(m_gameSettingsLblDevoMemcardEmu);
			m_btnMgr.show(m_gameSettingsBtnDevoMemcardEmu);

			m_btnMgr.show(m_gameSettingsLblDM_Widescreen);
			m_btnMgr.show(m_gameSettingsBtnDM_Widescreen);
		}
	}
	else
	{
		if(m_cf.getHdr()->type != TYPE_GC_GAME)
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
		}
		else
		{
			m_btnMgr.hide(m_gameSettingsLblNMM);
			m_btnMgr.hide(m_gameSettingsLblNMM_Val);
			m_btnMgr.hide(m_gameSettingsBtnNMM_P);
			m_btnMgr.hide(m_gameSettingsBtnNMM_M);

			m_btnMgr.hide(m_gameSettingsLblNoDVD);
			m_btnMgr.hide(m_gameSettingsLblNoDVD_Val);
			m_btnMgr.hide(m_gameSettingsBtnNoDVD_P);
			m_btnMgr.hide(m_gameSettingsBtnNoDVD_M);

			m_btnMgr.hide(m_gameSettingsLblDevoMemcardEmu);
			m_btnMgr.hide(m_gameSettingsBtnDevoMemcardEmu);

			m_btnMgr.hide(m_gameSettingsLblDM_Widescreen);
			m_btnMgr.hide(m_gameSettingsBtnDM_Widescreen);
		}
	}
	if (m_gameSettingsPage == 4)
	{
		if(m_cf.getHdr()->type == TYPE_CHANNEL)
		{
			m_btnMgr.show(m_gameSettingsLblCustom);
			m_btnMgr.show(m_gameSettingsBtnCustom);
			if(m_cfg.getInt("NAND", "partition", 0) == 1)
			{
				m_btnMgr.show(m_gameSettingsLblLaunchNK);
				m_btnMgr.show(m_gameSettingsBtnLaunchNK);
			}
		}
		else if(m_cf.getHdr()->type == TYPE_WII_GAME)
		{
			m_btnMgr.show(m_gameSettingsLblEmulationVal);
			m_btnMgr.show(m_gameSettingsLblEmulation);
			m_btnMgr.show(m_gameSettingsBtnEmulationP);
			m_btnMgr.show(m_gameSettingsBtnEmulationM);
			if(_checkSave(string((const char *)m_cf.getHdr()->id), true))
			{
				m_btnMgr.show(m_gameSettingsLblExtractSave);
				m_btnMgr.show(m_gameSettingsBtnExtractSave);
			}
		}
		m_btnMgr.show(m_gameSettingsLblGameIOS);
		m_btnMgr.show(m_gameSettingsLblIOS);
		m_btnMgr.show(m_gameSettingsBtnIOSP);
		m_btnMgr.show(m_gameSettingsBtnIOSM);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblCustom);
		m_btnMgr.hide(m_gameSettingsBtnCustom);
		m_btnMgr.hide(m_gameSettingsLblLaunchNK);
		m_btnMgr.hide(m_gameSettingsBtnLaunchNK);
	
		m_btnMgr.hide(m_gameSettingsLblEmulationVal);
		m_btnMgr.hide(m_gameSettingsLblEmulation);
		m_btnMgr.hide(m_gameSettingsBtnEmulationP);
		m_btnMgr.hide(m_gameSettingsBtnEmulationM);
		
		m_btnMgr.hide(m_gameSettingsLblGameIOS);
		m_btnMgr.hide(m_gameSettingsLblIOS);
		m_btnMgr.hide(m_gameSettingsBtnIOSP);
		m_btnMgr.hide(m_gameSettingsBtnIOSM);

		m_btnMgr.hide(m_gameSettingsLblExtractSave);
		m_btnMgr.hide(m_gameSettingsBtnExtractSave);
	}
	if (m_gameSettingsPage == 5)
	{
		m_btnMgr.show(m_gameSettingsLblFlashSave);
		m_btnMgr.show(m_gameSettingsBtnFlashSave);		
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblFlashSave);
		m_btnMgr.hide(m_gameSettingsBtnFlashSave);		
	}

	u32 i = 0;
	for(i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if(m_gameSettingsLblUser[i] != -1)
			m_btnMgr.show(m_gameSettingsLblUser[i]);

	string id(m_cf.getId());
	int page = m_gameSettingsPage;
	u32 maxpage = g_numGCfPages;

	m_btnMgr.setText(m_gameSettingsLblPage, wfmt(L"%i / %i", page, maxpage));
	m_btnMgr.setText(m_gameSettingsBtnOcarina, _optBoolToString(m_gcfg2.getOptBool(id, "cheat")));
	if(m_cf.getHdr()->type == TYPE_GC_GAME)
	{
		m_btnMgr.setText(m_gameSettingsBtnDM_Widescreen, _optBoolToString(m_gcfg2.getOptBool(id, "dm_widescreen", 0)));
		m_btnMgr.setText(m_gameSettingsBtnDevoMemcardEmu, _optBoolToString(m_gcfg2.getOptBool(id, "devo_memcard_emu", 0)));
		i = min((u32)m_gcfg2.getInt(id, "dml_video_mode", 0), ARRAY_SIZE(CMenu::_DMLvideoModes) - 1u);
		m_btnMgr.setText(m_gameSettingsLblDMLVideo, _t(CMenu::_DMLvideoModes[i].id, CMenu::_DMLvideoModes[i].text));
		i = min((u32)m_gcfg2.getInt(id, "gc_language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
		m_btnMgr.setText(m_gameSettingsLblGClanguageVal, _t(CMenu::_GClanguages[i].id, CMenu::_GClanguages[i].text));
		i = min((u32)m_gcfg2.getInt(id, "dml_nmm", 0), ARRAY_SIZE(CMenu::_NMM) - 1u);
		m_btnMgr.setText(m_gameSettingsLblNMM_Val, _t(CMenu::_NMM[i].id, CMenu::_NMM[i].text));
		i = min((u32)m_gcfg2.getInt(id, "no_disc_patch", 0), ARRAY_SIZE(CMenu::_NoDVD) - 1u);
		m_btnMgr.setText(m_gameSettingsLblNoDVD_Val, _t(CMenu::_NoDVD[i].id, CMenu::_NoDVD[i].text));
		i = min((u32)m_gcfg2.getInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
		m_btnMgr.setText(m_gameSettingsLblGCLoader_Val, _t(CMenu::_GCLoader[i].id, CMenu::_GCLoader[i].text));
	}
	else
	{
		m_btnMgr.setText(m_gameSettingsBtnVipatch, _optBoolToString(m_gcfg2.getOptBool(id, "vipatch", 0)));
		m_btnMgr.setText(m_gameSettingsBtnCountryPatch, _optBoolToString(m_gcfg2.getOptBool(id, "country_patch", 0)));
		m_btnMgr.setText(m_gameSettingsBtnIOSreloadBlock, _optBoolToString(m_gcfg2.getOptBool(id, "reload_block", 0)));
		i = min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
		m_btnMgr.setText(m_gameSettingsLblVideo, _t(CMenu::_VideoModes[i].id, CMenu::_VideoModes[i].text));
		i = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
		m_btnMgr.setText(m_gameSettingsLblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
		i = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u);
		m_btnMgr.setText(m_gameSettingsLblAspectRatioVal, _t(CMenu::_AspectRatio[i].id, CMenu::_AspectRatio[i].text));
		m_btnMgr.setText(m_gameSettingsBtnCustom, _optBoolToString(m_gcfg2.getOptBool(id, "custom", 0)));
		m_btnMgr.setText(m_gameSettingsBtnLaunchNK, _optBoolToString(m_gcfg2.getOptBool(id, "useneek", 0)));
	}

	int j = 0;
	if (m_gcfg2.getInt(id, "ios", &j) && _installed_cios.size() > 0)
	{
		CIOSItr itr = _installed_cios.find(j);
		j = (itr == _installed_cios.end()) ? 0 : itr->first;
	}
	else j = 0;

	if (j != 0)
	{
		m_btnMgr.setText(m_gameSettingsLblIOS, wstringEx(sfmt("%i", j)));
	}
	else
		m_btnMgr.setText(m_gameSettingsLblIOS, L"AUTO");

	i = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	m_btnMgr.setText(m_gameSettingsLblPatchVidModesVal, _t(CMenu::_vidModePatch[i].id, CMenu::_vidModePatch[i].text));

	i = min((u32)m_gcfg2.getInt(id, "hooktype", 1), ARRAY_SIZE(CMenu::_hooktype) - 1u);
	m_btnMgr.setText(m_gameSettingsLblHooktypeVal, _t(CMenu::_hooktype[i].id, CMenu::_hooktype[i].text));

	m_btnMgr.setText(m_gameSettingsBtnCategoryMain, _fmt("cfgg16",  wfmt(L"Select",i).c_str() )); 
	
	i = min((u32)m_gcfg2.getInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);
	m_btnMgr.setText(m_gameSettingsLblEmulationVal, _t(CMenu::_SaveEmu[i].id, CMenu::_SaveEmu[i].text));	
	
	m_btnMgr.setText(m_gameSettingsLblDebuggerV, m_gcfg2.getBool(id, "debugger", false) ? _t("gecko", L"Gecko") : _t("def", L"Default"));		
}

void CMenu::_gameSettings(void)
{
	m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
	string id(m_cf.getId());

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
			if (m_gameSettingsPage == 1)
				m_gameSettingsPage = g_numGCfPages;
			else --m_gameSettingsPage;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_gameSettingsBtnPageM);
			_showGameSettings();
		}
		else if((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_gameSettingsBtnPageP))) && !m_locked)
		{
			if(m_gameSettingsPage == g_numGCfPages)
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
				bool booloption = m_gcfg2.getBool(id, "vipatch");
				if(booloption != false)
					m_gcfg2.remove(id, "vipatch");
				else
					m_gcfg2.setBool(id, "vipatch", true);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnCountryPatch))
			{
				bool booloption = m_gcfg2.getBool(id, "country_patch");
				if(booloption != false)
					m_gcfg2.remove(id, "country_patch");
				else
					m_gcfg2.setBool(id, "country_patch", true);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnIOSreloadBlock))
			{
				bool booloption = m_gcfg2.getBool(id, "reload_block");
				if(booloption != false)
					m_gcfg2.remove(id, "reload_block");
				else
					m_gcfg2.setBool(id, "reload_block", true);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnCustom))
			{
				bool booloption = m_gcfg2.getBool(id, "custom");
				if(booloption != false)
					m_gcfg2.remove(id, "custom");
				else
					m_gcfg2.setBool(id, "custom", true);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnLaunchNK))
			{
				bool booloption = m_gcfg2.getBool(id, "useneek");
				if(booloption != false)
					m_gcfg2.remove(id, "useneek");
				else
					m_gcfg2.setBool(id, "useneek", true);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnDevoMemcardEmu))
			{
				bool booloption = m_gcfg2.getBool(id, "devo_memcard_emu");
				if(booloption != false)
					m_gcfg2.remove(id, "devo_memcard_emu");
				else
					m_gcfg2.setBool(id, "devo_memcard_emu", true);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnDM_Widescreen))
			{
				bool booloption = m_gcfg2.getBool(id, "dm_widescreen");
				if(booloption != false)
					m_gcfg2.remove(id, "dm_widescreen");
				else
					m_gcfg2.setBool(id, "dm_widescreen", true);
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnLanguageP) || m_btnMgr.selected(m_gameSettingsBtnLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnLanguageP) ? 1 : -1;
				m_gcfg2.setInt(id, "language", (int)loopNum((u32)m_gcfg2.getInt(id, "language", 0) + direction, ARRAY_SIZE(CMenu::_languages)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnVideoP) || m_btnMgr.selected(m_gameSettingsBtnVideoM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnVideoP) ? 1 : -1;
				m_gcfg2.setInt(id, "video_mode", (int)loopNum((u32)m_gcfg2.getInt(id, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_VideoModes)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnDMLVideoP) || m_btnMgr.selected(m_gameSettingsBtnDMLVideoM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnDMLVideoP) ? 1 : -1;
				m_gcfg2.setInt(id, "dml_video_mode", (int)loopNum((u32)m_gcfg2.getInt(id, "dml_video_mode", 0) + direction, ARRAY_SIZE(CMenu::_DMLvideoModes)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnGClanguageP) || m_btnMgr.selected(m_gameSettingsBtnGClanguageM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnGClanguageP) ? 1 : -1;
				m_gcfg2.setInt(id, "gc_language", (int)loopNum((u32)m_gcfg2.getInt(id, "gc_language", 0) + direction, ARRAY_SIZE(CMenu::_GClanguages)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnAspectRatioP) || m_btnMgr.selected(m_gameSettingsBtnAspectRatioM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnAspectRatioP) ? 1 : -1;
				m_gcfg2.setInt(id, "aspect_ratio", (int)loopNum((u32)m_gcfg2.getInt(id, "aspect_ratio", 0) + direction, ARRAY_SIZE(CMenu::_AspectRatio)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnNMM_P) || m_btnMgr.selected(m_gameSettingsBtnNMM_M))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnNMM_P) ? 1 : -1;
				m_gcfg2.setInt(id, "dml_nmm", (int)loopNum((u32)m_gcfg2.getInt(id, "dml_nmm", 0) + direction, ARRAY_SIZE(CMenu::_NMM)));
				_showGameSettings();
			}
			else if(m_btnMgr.selected(m_gameSettingsBtnNoDVD_P) || m_btnMgr.selected(m_gameSettingsBtnNoDVD_M))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnNoDVD_P) ? 1 : -1;
				m_gcfg2.setInt(id, "no_disc_patch", (int)loopNum((u32)m_gcfg2.getInt(id, "no_disc_patch", 0) + direction, ARRAY_SIZE(CMenu::_NoDVD)));
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
				if( _installed_cios.size() > 0)
				{
					bool direction = m_btnMgr.selected(m_gameSettingsBtnIOSP);

					CIOSItr itr = _installed_cios.find((u32)m_gcfg2.getInt(id, "ios", 0));
					
					if (direction && itr == _installed_cios.end())
						itr = _installed_cios.begin();
					else if(!direction && itr == _installed_cios.begin())
						itr = _installed_cios.end();
					else if (direction)
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
			else if (m_btnMgr.selected(m_gameSettingsBtnPatchVidModesP) || m_btnMgr.selected(m_gameSettingsBtnPatchVidModesM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnPatchVidModesP) ? 1 : -1;
				m_gcfg2.setInt(id, "patch_video_modes", (int)loopNum((u32)m_gcfg2.getInt(id, "patch_video_modes", 0) + direction, ARRAY_SIZE(CMenu::_vidModePatch)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnCover))
			{
				m_cf.stopCoverLoader(true);
				_hideGameSettings();
				_download(id);
				_showGameSettings();
				m_cf.startCoverLoader();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnCheat))
			{
				_hideGameSettings();
				_CheatSettings();
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnHooktypeP) || m_btnMgr.selected(m_gameSettingsBtnHooktypeM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnHooktypeP) ? 1 : -1;
				m_gcfg2.setInt(id, "hooktype", (int)loopNum((u32)m_gcfg2.getInt(id, "hooktype", 1) + direction, ARRAY_SIZE(CMenu::_hooktype)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnEmulationP) || m_btnMgr.selected(m_gameSettingsBtnEmulationM))
			{
				s8 direction = m_btnMgr.selected(m_gameSettingsBtnEmulationP) ? 1 : -1;
				m_gcfg2.setInt(id, "emulate_save", (int)loopNum((u32)m_gcfg2.getInt(id, "emulate_save", 0) + direction, ARRAY_SIZE(CMenu::_SaveEmu)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnDebuggerP) || m_btnMgr.selected(m_gameSettingsBtnDebuggerM))
			{
				bool booloption = m_gcfg2.getBool(id, "debugger");
				if (booloption != false)
					m_gcfg2.remove(id, "debugger");
				else
				m_gcfg2.setBool(id, "debugger", true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnCategoryMain))
			{
				_hideGameSettings();
				_CategorySettings(true);
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnExtractSave))
			{
				_hideGameSettings();
				m_forceext = true;
				_AutoExtractSave(id);
				_showGameSettings();
			}
			else if (m_btnMgr.selected(m_gameSettingsBtnFlashSave))
			{
				_hideGameSettings();
				m_forceext = true;
				_FlashSave(id);
				_showGameSettings();
			}
		}
		else if ((WBTN_2_HELD && WBTN_1_PRESSED) || (WBTN_1_HELD && WBTN_2_PRESSED))
		{	
			if (m_btnMgr.selected(m_gameSettingsBtnCover))
			{
					m_cf.stopCoverLoader(true);	// Empty cover cache
					remove(fmt("%s/%s.png", m_picDir.c_str(), m_cf.getId().c_str()));
					remove(fmt("%s/%s.png", m_boxPicDir.c_str(), m_cf.getId().c_str()));
					remove(fmt("%s/%s.wfc", m_cacheDir.c_str(), m_cf.getId().c_str()));
					m_cf.startCoverLoader();
			}
		}

	}
	m_gcfg2.save(true);
	_hideGameSettings();
}

void CMenu::_initGameSettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_gameSettingsLblUser, ARRAY_SIZE(m_gameSettingsLblUser), "GAME_SETTINGS");
	m_gameSettingsBg = _texture(theme.texSet, "GAME_SETTINGS/BG", "texture", theme.bg);
	m_gameSettingsLblTitle = _addTitle(theme, "GAME_SETTINGS/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	
	// Page 1
	m_gameSettingsLblCover = _addLabel(theme, "GAME_SETTINGS/COVER", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCover = _addButton(theme, "GAME_SETTINGS/COVER_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);
	
	m_gameSettingsLblCategoryMain = _addLabel(theme, "GAME_SETTINGS/CAT_MAIN", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCategoryMain = _addButton(theme, "GAME_SETTINGS/CAT_MAIN_BTN", theme.btnFont, L"", 330, 190, 270, 56, theme.btnFontColor);
	
	m_gameSettingsLblGameLanguage = _addLabel(theme, "GAME_SETTINGS/GAME_LANG", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblLanguage = _addLabel(theme, "GAME_SETTINGS/GAME_LANG_BTN", theme.btnFont, L"", 386, 250, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnLanguageM = _addPicButton(theme, "GAME_SETTINGS/GAME_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 250, 56, 56);
	m_gameSettingsBtnLanguageP = _addPicButton(theme, "GAME_SETTINGS/GAME_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);

	m_gameSettingsLblGameVideo = _addLabel(theme, "GAME_SETTINGS/VIDEO", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblVideo = _addLabel(theme, "GAME_SETTINGS/VIDEO_BTN", theme.btnFont, L"", 386, 310, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnVideoM = _addPicButton(theme, "GAME_SETTINGS/VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 310, 56, 56);
	m_gameSettingsBtnVideoP = _addPicButton(theme, "GAME_SETTINGS/VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
	
	//DML Page 1
	m_gameSettingsLblDMLGameVideo = _addLabel(theme, "GAME_SETTINGS/DML_VIDEO", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblDMLVideo = _addLabel(theme, "GAME_SETTINGS/DML_VIDEO_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnDMLVideoM = _addPicButton(theme, "GAME_SETTINGS/DML_VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_gameSettingsBtnDMLVideoP = _addPicButton(theme, "GAME_SETTINGS/DML_VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);

	m_gameSettingsLblGClanguage = _addLabel(theme, "GAME_SETTINGS/GC_LANG", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblGClanguageVal = _addLabel(theme, "GAME_SETTINGS/GC_LANG_BTN", theme.btnFont, L"", 386, 250, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnGClanguageM = _addPicButton(theme, "GAME_SETTINGS/GC_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 250, 56, 56);
	m_gameSettingsBtnGClanguageP = _addPicButton(theme, "GAME_SETTINGS/GC_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);

	// Page 2
	m_gameSettingsLblDebugger = _addLabel(theme, "GAME_SETTINGS/GAME_DEBUGGER", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblDebuggerV = _addLabel(theme, "GAME_SETTINGS/GAME_DEBUGGER_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnDebuggerM = _addPicButton(theme, "GAME_SETTINGS/GAME_DEBUGGER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_gameSettingsBtnDebuggerP = _addPicButton(theme, "GAME_SETTINGS/GAME_DEBUGGER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);

	m_gameSettingsLblHooktype = _addLabel(theme, "GAME_SETTINGS/HOOKTYPE", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblHooktypeVal = _addLabel(theme, "GAME_SETTINGS/HOOKTYPE_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnHooktypeM = _addPicButton(theme, "GAME_SETTINGS/HOOKTYPE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_gameSettingsBtnHooktypeP = _addPicButton(theme, "GAME_SETTINGS/HOOKTYPE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);

	m_gameSettingsLblOcarina = _addLabel(theme, "GAME_SETTINGS/OCARINA", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnOcarina = _addButton(theme, "GAME_SETTINGS/OCARINA_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);

	m_gameSettingsLblCheat = _addLabel(theme, "GAME_SETTINGS/CHEAT", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCheat = _addButton(theme, "GAME_SETTINGS/CHEAT_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);

	// Page 3
	m_gameSettingsLblCountryPatch = _addLabel(theme, "GAME_SETTINGS/COUNTRY_PATCH", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCountryPatch = _addButton(theme, "GAME_SETTINGS/COUNTRY_PATCH_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);

	m_gameSettingsLblVipatch = _addLabel(theme, "GAME_SETTINGS/VIPATCH", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnVipatch = _addButton(theme, "GAME_SETTINGS/VIPATCH_BTN", theme.btnFont, L"", 330, 190, 270, 56, theme.btnFontColor);

	m_gameSettingsLblPatchVidModes = _addLabel(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblPatchVidModesVal = _addLabel(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", theme.btnFont, L"", 386, 250, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPatchVidModesM = _addPicButton(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 250, 56, 56);
	m_gameSettingsBtnPatchVidModesP = _addPicButton(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);

	m_gameSettingsLblAspectRatio = _addLabel(theme, "GAME_SETTINGS/ASPECT_RATIO", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblAspectRatioVal = _addLabel(theme, "GAME_SETTINGS/ASPECT_RATIO_BTN", theme.btnFont, L"", 386, 310, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnAspectRatioM = _addPicButton(theme, "GAME_SETTINGS/ASPECT_RATIO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 310, 56, 56);
	m_gameSettingsBtnAspectRatioP = _addPicButton(theme, "GAME_SETTINGS/ASPECT_RATIO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);

	//DML Page 3
	m_gameSettingsLblNMM = _addLabel(theme, "GAME_SETTINGS/DML_NMM", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblNMM_Val = _addLabel(theme, "GAME_SETTINGS/DML_NMM_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnNMM_M = _addPicButton(theme, "GAME_SETTINGS/DML_NMM_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_gameSettingsBtnNMM_P = _addPicButton(theme, "GAME_SETTINGS/DML_NMM_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);

	m_gameSettingsLblNoDVD = _addLabel(theme, "GAME_SETTINGS/NO_DVD_PATCH", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblNoDVD_Val = _addLabel(theme, "GAME_SETTINGS/NO_DVD_PATCH_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnNoDVD_M = _addPicButton(theme, "GAME_SETTINGS/NO_DVD_PATCH_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_gameSettingsBtnNoDVD_P = _addPicButton(theme, "GAME_SETTINGS/NO_DVD_PATCH_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);

	m_gameSettingsLblDevoMemcardEmu = _addLabel(theme, "GAME_SETTINGS/DEVO_MEMCARD_EMU", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnDevoMemcardEmu = _addButton(theme, "GAME_SETTINGS/DEVO_MEMCARD_EMU_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);

	m_gameSettingsLblGCLoader = _addLabel(theme, "GAME_SETTINGS/GC_LOADER", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblGCLoader_Val = _addLabel(theme, "GAME_SETTINGS/GC_LOADER_BTN", theme.btnFont, L"", 386, 310, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnGCLoader_M = _addPicButton(theme, "GAME_SETTINGS/GC_LOADER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 310, 56, 56);
	m_gameSettingsBtnGCLoader_P = _addPicButton(theme, "GAME_SETTINGS/GC_LOADER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);

	m_gameSettingsLblDM_Widescreen = _addLabel(theme, "GAME_SETTINGS/DM_WIDESCREEN", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnDM_Widescreen = _addButton(theme, "GAME_SETTINGS/DM_WIDESCREEN_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);

	//Page 4
 	m_gameSettingsLblCustom = _addLabel(theme, "GAME_SETTINGS/CUSTOM", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCustom =  _addButton(theme, "GAME_SETTINGS/CUSTOM_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);
	
	m_gameSettingsLblEmulation = _addLabel(theme, "GAME_SETTINGS/EMU_SAVE", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblEmulationVal = _addLabel(theme, "GAME_SETTINGS/EMU_SAVE_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnEmulationM = _addPicButton(theme, "GAME_SETTINGS/EMU_SAVE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_gameSettingsBtnEmulationP = _addPicButton(theme, "GAME_SETTINGS/EMU_SAVE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	
	m_gameSettingsLblGameIOS = _addLabel(theme, "GAME_SETTINGS/IOS", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
 	m_gameSettingsLblIOS = _addLabel(theme, "GAME_SETTINGS/IOS_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
 	m_gameSettingsBtnIOSM = _addPicButton(theme, "GAME_SETTINGS/IOS_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
 	m_gameSettingsBtnIOSP = _addPicButton(theme, "GAME_SETTINGS/IOS_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);

	m_gameSettingsLblLaunchNK = _addLabel(theme, "GAME_SETTINGS/LAUNCHNEEK", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnLaunchNK =  _addButton(theme, "GAME_SETTINGS/LAUNCHNEEK_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	
	m_gameSettingsLblExtractSave = _addLabel(theme, "GAME_SETTINGS/EXTRACT_SAVE", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnExtractSave = _addButton(theme, "GAME_SETTINGS/EXTRACT_SAVE_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);

	//Page 5
	m_gameSettingsLblFlashSave = _addLabel(theme, "GAME_SETTINGS/FLASH_SAVE", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnFlashSave = _addButton(theme, "GAME_SETTINGS/FLASH_SAVE_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);
		
	//Footer
	m_gameSettingsLblPage = _addLabel(theme, "GAME_SETTINGS/PAGE_BTN", theme.btnFont, L"", 76, 400, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPageM = _addPicButton(theme, "GAME_SETTINGS/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_gameSettingsBtnPageP = _addPicButton(theme, "GAME_SETTINGS/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 400, 56, 56);
	m_gameSettingsBtnBack = _addButton(theme, "GAME_SETTINGS/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);

	_setHideAnim(m_gameSettingsLblTitle, "GAME_SETTINGS/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_gameSettingsLblGameVideo, "GAME_SETTINGS/VIDEO", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblVideo, "GAME_SETTINGS/VIDEO_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoM, "GAME_SETTINGS/VIDEO_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoP, "GAME_SETTINGS/VIDEO_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCustom, "GAME_SETTINGS/CUSTOM", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCustom, "GAME_SETTINGS/CUSTOM_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblLaunchNK, "GAME_SETTINGS/LAUNCHNEEK", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLaunchNK, "GAME_SETTINGS/LAUNCHNEEK_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDMLGameVideo, "GAME_SETTINGS/DML_VIDEO", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDMLVideo, "GAME_SETTINGS/DML_VIDEO_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDMLVideoM, "GAME_SETTINGS/DML_VIDEO_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDMLVideoP, "GAME_SETTINGS/DML_VIDEO_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGClanguage, "GAME_SETTINGS/GC_LANG", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGClanguageVal, "GAME_SETTINGS/GC_LANG_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnGClanguageM, "GAME_SETTINGS/GC_LANG_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnGClanguageP, "GAME_SETTINGS/GC_LANG_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGameLanguage, "GAME_SETTINGS/GAME_LANG", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblLanguage, "GAME_SETTINGS/GAME_LANG_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageM, "GAME_SETTINGS/GAME_LANG_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageP, "GAME_SETTINGS/GAME_LANG_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblOcarina, "GAME_SETTINGS/OCARINA", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnOcarina, "GAME_SETTINGS/OCARINA_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCheat, "GAME_SETTINGS/CHEAT", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCheat, "GAME_SETTINGS/CHEAT_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblVipatch, "GAME_SETTINGS/VIPATCH", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVipatch, "GAME_SETTINGS/VIPATCH_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGameIOS, "GAME_SETTINGS/IOS", -200, 0, 1.f, 0.f);
 	_setHideAnim(m_gameSettingsLblIOS, "GAME_SETTINGS/IOS_BTN", 200, 0, 1.f, 0.f);
 	_setHideAnim(m_gameSettingsBtnIOSM, "GAME_SETTINGS/IOS_MINUS", 200, 0, 1.f, 0.f);
 	_setHideAnim(m_gameSettingsBtnIOSP, "GAME_SETTINGS/IOS_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCover, "GAME_SETTINGS/COVER", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCover, "GAME_SETTINGS/COVER_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPage, "GAME_SETTINGS/PAGE_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPageM, "GAME_SETTINGS/PAGE_MINUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPageP, "GAME_SETTINGS/PAGE_PLUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnBack, "GAME_SETTINGS/BACK_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModes, "GAME_SETTINGS/PATCH_VIDEO_MODE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModesVal, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesM, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesP, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblAspectRatio, "GAME_SETTINGS/ASPECT_RATIO", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblAspectRatioVal, "GAME_SETTINGS/ASPECT_RATIO_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnAspectRatioP, "GAME_SETTINGS/ASPECT_RATIO_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnAspectRatioM, "GAME_SETTINGS/ASPECT_RATIO_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblNMM, "GAME_SETTINGS/DML_NMM", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblNMM_Val, "GAME_SETTINGS/DML_NMM_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnNMM_P, "GAME_SETTINGS/DML_NMM_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnNMM_M, "GAME_SETTINGS/DML_NMM_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblNoDVD, "GAME_SETTINGS/NO_DVD_PATCH", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblNoDVD_Val, "GAME_SETTINGS/NO_DVD_PATCH_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnNoDVD_P, "GAME_SETTINGS/NO_DVD_PATCH_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnNoDVD_M, "GAME_SETTINGS/NO_DVD_PATCH_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDevoMemcardEmu, "GAME_SETTINGS/DEVO_MEMCARD_EMU", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDevoMemcardEmu, "GAME_SETTINGS/DEVO_MEMCARD_EMU_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDM_Widescreen, "GAME_SETTINGS/DM_WIDESCREEN", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDM_Widescreen, "GAME_SETTINGS/DM_WIDESCREEN_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGCLoader, "GAME_SETTINGS/GC_LOADER", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGCLoader_Val, "GAME_SETTINGS/GC_LOADER_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnGCLoader_P, "GAME_SETTINGS/GC_LOADER_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnGCLoader_M, "GAME_SETTINGS/GC_LOADER_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblHooktype, "GAME_SETTINGS/HOOKTYPE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblHooktypeVal, "GAME_SETTINGS/HOOKTYPE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnHooktypeM, "GAME_SETTINGS/HOOKTYPE_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnHooktypeP, "GAME_SETTINGS/HOOKTYPE_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblEmulation, "GAME_SETTINGS/EMU_SAVE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblEmulationVal, "GAME_SETTINGS/EMU_SAVE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnEmulationP, "GAME_SETTINGS/EMU_SAVE_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnEmulationM, "GAME_SETTINGS/EMU_SAVE_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDebugger, "GAME_SETTINGS/GAME_DEBUGGER", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblDebuggerV, "GAME_SETTINGS/GAME_DEBUGGER_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDebuggerM, "GAME_SETTINGS/GAME_DEBUGGER_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnDebuggerP, "GAME_SETTINGS/GAME_DEBUGGER_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblExtractSave, "GAME_SETTINGS/EXTRACT_SAVE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnExtractSave, "GAME_SETTINGS/EXTRACT_SAVE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblFlashSave, "GAME_SETTINGS/FLASH_SAVE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnFlashSave, "GAME_SETTINGS/FLASH_SAVE_BTN", 200, 0, 1.f, 0.f);
	//Categories
	_setHideAnim(m_gameSettingsBtnCategoryMain, "GAME_SETTINGS/CAT_MAIN_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCategoryMain, "GAME_SETTINGS/CAT_MAIN", -200, 0, 1.f, 0.f);

	_hideGameSettings(true);
	_textGameSettings();
}

void CMenu::_textGameSettings(void)
{
	m_btnMgr.setText(m_gameSettingsLblTitle, _t("cfgg1", L"Settings"));
	m_btnMgr.setText(m_gameSettingsLblGameVideo, _t("cfgg2", L"Video mode"));
	m_btnMgr.setText(m_gameSettingsLblGameLanguage, _t("cfgg3", L"Language"));
	m_btnMgr.setText(m_gameSettingsLblCountryPatch, _t("cfgg4", L"Patch country strings"));
	m_btnMgr.setText(m_gameSettingsLblOcarina, _t("cfgg5", L"Ocarina"));
	m_btnMgr.setText(m_gameSettingsLblDMLGameVideo, _t("cfgg2", L"Video mode"));
	m_btnMgr.setText(m_gameSettingsLblGClanguage, _t("cfgg3", L"Language"));
	m_btnMgr.setText(m_gameSettingsLblVipatch, _t("cfgg7", L"Vipatch"));
	m_btnMgr.setText(m_gameSettingsBtnBack, _t("cfgg8", L"Back"));
	m_btnMgr.setText(m_gameSettingsLblGameIOS, _t("cfgg10", L"IOS"));
	m_btnMgr.setText(m_gameSettingsLblCover, _t("cfgg12", L"Download cover"));
	m_btnMgr.setText(m_gameSettingsBtnCover, _t("cfgg13", L"Download"));
	m_btnMgr.setText(m_gameSettingsLblPatchVidModes, _t("cfgg14", L"Patch video modes"));
	m_btnMgr.setText(m_gameSettingsLblCheat, _t("cfgg15", L"Cheat Codes"));
	m_btnMgr.setText(m_gameSettingsBtnCheat, _t("cfgg16", L"Select"));
	m_btnMgr.setText(m_gameSettingsLblCategoryMain, _t("cfgg17", L"Categories"));
	m_btnMgr.setText(m_gameSettingsBtnCategoryMain, _t("cfgg16", L"Select"));
	m_btnMgr.setText(m_gameSettingsLblHooktype, _t("cfgg18", L"Hook Type"));
	m_btnMgr.setText(m_gameSettingsLblDebugger, _t("cfgg22", L"Debugger"));
	m_btnMgr.setText(m_gameSettingsLblEmulation, _t("cfgg24", L"NAND Emulation"));
	m_btnMgr.setText(m_gameSettingsLblAspectRatio, _t("cfgg27", L"Aspect Ratio"));
	m_btnMgr.setText(m_gameSettingsLblNMM, _t("cfgg28", L"NMM"));
	m_btnMgr.setText(m_gameSettingsLblNoDVD, _t("cfgg29", L"No DVD Patch"));
	m_btnMgr.setText(m_gameSettingsLblCustom, _t("custom", L"Custom"));
	m_btnMgr.setText(m_gameSettingsLblLaunchNK, _t("neek1", L"Launch Title with neek2o"));
	m_btnMgr.setText(m_gameSettingsLblExtractSave, _t("cfgg30", L"Extract Save from NAND"));
	m_btnMgr.setText(m_gameSettingsBtnExtractSave, _t("cfgg31", L"Extract"));
	m_btnMgr.setText(m_gameSettingsLblFlashSave, _t("cfgg32", L"Flash Save to NAND"));
	m_btnMgr.setText(m_gameSettingsBtnFlashSave, _t("cfgg33", L"Flash"));
	m_btnMgr.setText(m_gameSettingsLblDevoMemcardEmu, _t("cfgg34", L"Devolution Memcard Emulator"));
	m_btnMgr.setText(m_gameSettingsLblGCLoader, _t("cfgg35", L"GameCube Loader"));
	m_btnMgr.setText(m_gameSettingsLblDM_Widescreen, _t("cfgg36", L"DM Widescreen Patch"));
}
