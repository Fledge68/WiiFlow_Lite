
#include "menu.hpp"

TexData m_configGCBg;

s16 m_configGCLblTitle;
s16 m_configGCBtnBack;
s16 m_configGCLblUser[4];

s16 m_configGCLblPage;
s16 m_configGCBtnPageM;
s16 m_configGCBtnPageP;

//pg 1
s16 m_configGCLblGCGameLanguage;
s16 m_configGCLblGCLanguage;
s16 m_configGCBtnGCLanguageP;
s16 m_configGCBtnGCLanguageM;

s16 m_configGCLblGCGameVideo;
s16 m_configGCLblGCVideo;
s16 m_configGCBtnGCVideoP;
s16 m_configGCBtnGCVideoM;

s16 m_configGCLblGCGameLoader;
s16 m_configGCLblGCLoader;
s16 m_configGCBtnGCLoaderP;
s16 m_configGCBtnGCLoaderM;

s16 m_configGCLblDevMemCard;
s16 m_configGCBtnDevMemCard;

//pg 2
s16 m_configGCLblNinMemCard;
s16 m_configGCLblNinMemCardVal;
s16 m_configGCBtnNinMemCardP;
s16 m_configGCBtnNinMemCardM;

s16 m_configGCLblNinWiiUWide;
s16 m_configGCBtnNinWiiUWide;

static int curPage;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfigGC(bool instant)
{
	m_btnMgr.hide(m_configGCLblTitle, instant);
	m_btnMgr.hide(m_configGCLblPage, instant);
	m_btnMgr.hide(m_configGCBtnPageM, instant);
	m_btnMgr.hide(m_configGCBtnPageP, instant);
	m_btnMgr.hide(m_configGCBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_configGCLblUser); ++i)
		if(m_configGCLblUser[i] != -1)
			m_btnMgr.hide(m_configGCLblUser[i], instant);
	_hideConfigGCPage(instant);
}

void CMenu::_hideConfigGCPage(bool instant)
{
	//pg 1
	m_btnMgr.hide(m_configGCLblGCGameLanguage, instant);
	m_btnMgr.hide(m_configGCLblGCLanguage, instant);
	m_btnMgr.hide(m_configGCBtnGCLanguageP, instant);
	m_btnMgr.hide(m_configGCBtnGCLanguageM, instant);
	
	m_btnMgr.hide(m_configGCLblGCGameVideo, instant);
	m_btnMgr.hide(m_configGCLblGCVideo, instant);
	m_btnMgr.hide(m_configGCBtnGCVideoP, instant);
	m_btnMgr.hide(m_configGCBtnGCVideoM, instant);

	m_btnMgr.hide(m_configGCLblGCGameLoader, instant);
	m_btnMgr.hide(m_configGCLblGCLoader, instant);
	m_btnMgr.hide(m_configGCBtnGCLoaderP, instant);
	m_btnMgr.hide(m_configGCBtnGCLoaderM, instant);

	m_btnMgr.hide(m_configGCLblDevMemCard, instant);
	m_btnMgr.hide(m_configGCBtnDevMemCard, instant);
	
	//pg 2
	m_btnMgr.hide(m_configGCLblNinMemCard, instant);
	m_btnMgr.hide(m_configGCLblNinMemCardVal, instant);
	m_btnMgr.hide(m_configGCBtnNinMemCardP, instant);
	m_btnMgr.hide(m_configGCBtnNinMemCardM, instant);

	m_btnMgr.hide(m_configGCLblNinWiiUWide, instant);
	m_btnMgr.hide(m_configGCBtnNinWiiUWide, instant);
}

void CMenu::_showConfigGC(void)
{
	m_btnMgr.show(m_configGCLblTitle);
	m_btnMgr.show(m_configGCBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_configGCLblUser); ++i)
		if(m_configGCLblUser[i] != -1)
			m_btnMgr.show(m_configGCLblUser[i]);

	m_btnMgr.show(m_configGCLblPage);
	m_btnMgr.show(m_configGCBtnPageM);
	m_btnMgr.show(m_configGCBtnPageP);
	_showConfigGCPage();
}

void CMenu::_showConfigGCPage(void)
{
	_hideConfigGCPage();
	m_btnMgr.setText(m_configGCLblPage, wfmt(L"%i / %i", curPage, 2));

	int i;
	if(curPage == 1)
	{
		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_GlobalGCvideoModes) - 1);
		m_btnMgr.setText(m_configGCLblGCVideo, _t(CMenu::_GlobalGCvideoModes[i].id, CMenu::_GlobalGCvideoModes[i].text));
		
		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "game_language", 0)), (int)ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1);
		m_btnMgr.setText(m_configGCLblGCLanguage, _t(CMenu::_GlobalGClanguages[i].id, CMenu::_GlobalGClanguages[i].text));

		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "default_loader", 2)), (int)ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1);
		m_btnMgr.setText(m_configGCLblGCLoader, _t(CMenu::_GlobalGCLoaders[i].id, CMenu::_GlobalGCLoaders[i].text));

		m_btnMgr.setText(m_configGCBtnDevMemCard, m_cfg.getBool(GC_DOMAIN, "devo_memcard_emu", false) ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_configGCLblGCGameLanguage);
		m_btnMgr.show(m_configGCLblGCLanguage);
		m_btnMgr.show(m_configGCBtnGCLanguageP);
		m_btnMgr.show(m_configGCBtnGCLanguageM);
		
		m_btnMgr.show(m_configGCLblGCGameVideo);
		m_btnMgr.show(m_configGCLblGCVideo);
		m_btnMgr.show(m_configGCBtnGCVideoP);
		m_btnMgr.show(m_configGCBtnGCVideoM);

		m_btnMgr.show(m_configGCLblGCGameLoader);
		m_btnMgr.show(m_configGCLblGCLoader);
		m_btnMgr.show(m_configGCBtnGCLoaderP);
		m_btnMgr.show(m_configGCBtnGCLoaderM);

		m_btnMgr.show(m_configGCLblDevMemCard);
		m_btnMgr.show(m_configGCBtnDevMemCard);
	}
	else
	{
		// minus 2 and [i + 1] because there is no global array that does not include 'default'
		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "emu_memcard", 1)), (int)ARRAY_SIZE(CMenu::_NinEmuCard) - 2);
		m_btnMgr.setText(m_configGCLblNinMemCardVal, _t(CMenu::_NinEmuCard[i + 1].id, CMenu::_NinEmuCard[i + 1].text));
		
		m_btnMgr.setText(m_configGCBtnNinWiiUWide, m_cfg.getBool(GC_DOMAIN, "wiiu_widescreen", false) ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_configGCLblNinMemCard);
		m_btnMgr.show(m_configGCLblNinMemCardVal);
		m_btnMgr.show(m_configGCBtnNinMemCardP);
		m_btnMgr.show(m_configGCBtnNinMemCardM);

		m_btnMgr.show(m_configGCLblNinWiiUWide);
		m_btnMgr.show(m_configGCBtnNinWiiUWide);
	}
}

void CMenu::_configGC(void)
{
	int i;
	bool j;
	curPage = 1;
	SetupInput();
	_setBg(m_configGCBg, m_configGCBg);
	_showConfigGC();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_LEFT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configGCBtnPageM)))
		{
			curPage--;
			if(curPage < 1)
				curPage = 2;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_configGCBtnPageM);
			_showConfigGCPage();
		}
		else if(BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configGCBtnPageP)))
		{
			curPage++;
			if(curPage > 2)
				curPage = 1;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_configGCBtnPageP);
			_showConfigGCPage();
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_configGCBtnBack))
				break;
			else if(m_btnMgr.selected(m_configGCBtnGCLanguageP) || m_btnMgr.selected(m_configGCBtnGCLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_configGCBtnGCLanguageP) ? 1 : -1;
				i = loopNum(m_cfg.getInt(GC_DOMAIN, "game_language", 0) + direction, (int)ARRAY_SIZE(CMenu::_GlobalGClanguages));
				m_cfg.setInt(GC_DOMAIN, "game_language", i);
				m_btnMgr.setText(m_configGCLblGCLanguage, _t(CMenu::_GlobalGClanguages[i].id, CMenu::_GlobalGClanguages[i].text));
			}
			else if(m_btnMgr.selected(m_configGCBtnGCVideoP) || m_btnMgr.selected(m_configGCBtnGCVideoM))
			{
				s8 direction = m_btnMgr.selected(m_configGCBtnGCVideoP) ? 1 : -1;
				i = loopNum(m_cfg.getInt(GC_DOMAIN, "video_mode", 0) + direction, (int)ARRAY_SIZE(CMenu::_GlobalGCvideoModes));
				m_cfg.setInt(GC_DOMAIN, "video_mode", i);
				m_btnMgr.setText(m_configGCLblGCVideo, _t(CMenu::_GlobalGCvideoModes[i].id, CMenu::_GlobalGCvideoModes[i].text));
			}
			else if(m_btnMgr.selected(m_configGCBtnGCLoaderP) || m_btnMgr.selected(m_configGCBtnGCLoaderM))
			{
				s8 direction = m_btnMgr.selected(m_configGCBtnGCLoaderP) ? 1 : -1;
				i = loopNum(m_cfg.getInt(GC_DOMAIN, "default_loader", 1) + direction, (int)ARRAY_SIZE(CMenu::_GlobalGCLoaders));
				m_cfg.setInt(GC_DOMAIN, "default_loader", i);
				m_btnMgr.setText(m_configGCLblGCLoader, _t(CMenu::_GlobalGCLoaders[i].id, CMenu::_GlobalGCLoaders[i].text));
			}
			else if(m_btnMgr.selected(m_configGCBtnNinMemCardP) || m_btnMgr.selected(m_configGCBtnNinMemCardM))
			{
				s8 direction = m_btnMgr.selected(m_configGCBtnNinMemCardP) ? 1 : -1;
				i = loopNum(m_cfg.getInt(GC_DOMAIN, "emu_memcard", 1) + direction, (int)ARRAY_SIZE(CMenu::_NinEmuCard) - 1);
				m_cfg.setInt(GC_DOMAIN, "emu_memcard", i);
				m_btnMgr.setText(m_configGCLblNinMemCardVal, _t(CMenu::_NinEmuCard[i + 1].id, CMenu::_NinEmuCard[i + 1].text));
			}
			else if(m_btnMgr.selected(m_configGCBtnDevMemCard))
			{
				j = !m_cfg.getBool(GC_DOMAIN, "devo_memcard_emu", false);
				m_cfg.setBool(GC_DOMAIN, "devo_memcard_emu", j);
				m_btnMgr.setText(m_configGCBtnDevMemCard, j ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if(m_btnMgr.selected(m_configGCBtnNinWiiUWide))
			{
				j = !m_cfg.getBool(GC_DOMAIN, "wiiu_widescreen", false);
				m_cfg.setBool(GC_DOMAIN, "wiiu_widescreen", j);
				m_btnMgr.setText(m_configGCBtnNinWiiUWide, j ? _t("on", L"On") : _t("off", L"Off"));
			}
		}
	}
	_hideConfigGC();
}

void CMenu::_initConfigGCMenu(void)
{
	m_configGCBg = _texture("CONFIGGC/BG", "texture", theme.bg, false);

	_addUserLabels(m_configGCLblUser, ARRAY_SIZE(m_configGCLblUser), "CONFIGGC");
	m_configGCLblTitle = _addLabel("CONFIGGC/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_configGCBtnBack = _addButton("CONFIGGC/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_configGCLblPage = _addLabel("CONFIGGC/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configGCBtnPageM = _addPicButton("CONFIGGC/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_configGCBtnPageP = _addPicButton("CONFIGGC/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);

	//pg 1
	m_configGCLblGCGameVideo = _addLabel("CONFIGGC/GC_VIDEO", theme.lblFont, L"", 20, 125, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configGCLblGCVideo = _addLabel("CONFIGGC/GC_VIDEO_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configGCBtnGCVideoM = _addPicButton("CONFIGGC/GC_VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_configGCBtnGCVideoP = _addPicButton("CONFIGGC/GC_VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	
	m_configGCLblGCGameLanguage = _addLabel("CONFIGGC/GC_LANG", theme.lblFont, L"", 20, 185, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configGCLblGCLanguage = _addLabel("CONFIGGC/GC_LANG_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configGCBtnGCLanguageM = _addPicButton("CONFIGGC/GC_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_configGCBtnGCLanguageP = _addPicButton("CONFIGGC/GC_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	
	m_configGCLblGCGameLoader = _addLabel("CONFIGGC/GC_LOADER", theme.lblFont, L"", 20, 245, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configGCLblGCLoader = _addLabel("CONFIGGC/GC_LOADER_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configGCBtnGCLoaderM = _addPicButton("CONFIGGC/GC_LOADER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_configGCBtnGCLoaderP = _addPicButton("CONFIGGC/GC_LOADER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);
	
	m_configGCLblDevMemCard = _addLabel("CONFIGGC/DEV_MEMCARD", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configGCBtnDevMemCard = _addButton("CONFIGGC/DEV_MEMCARD_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);
	
	//pg 2
	m_configGCLblNinMemCard = _addLabel("CONFIGGC/NIN_MEMCARD", theme.lblFont, L"", 20, 125, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configGCLblNinMemCardVal = _addLabel("CONFIGGC/NIN_MEMCARD_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configGCBtnNinMemCardM = _addPicButton("CONFIGGC/NIN_MEMCARD_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_configGCBtnNinMemCardP = _addPicButton("CONFIGGC/NIN_MEMCARD_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);

	m_configGCLblNinWiiUWide = _addLabel("CONFIGGC/NIN_WIIU_WIDE", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configGCBtnNinWiiUWide = _addButton("CONFIGGC/NIN_WIIU_WIDE_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	
	_setHideAnim(m_configGCLblTitle, "CONFIGGC/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configGCBtnBack, "CONFIGGC/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configGCLblPage, "CONFIGGC/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configGCBtnPageM, "CONFIGGC/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configGCBtnPageP, "CONFIGGC/PAGE_PLUS", 0, 0, 1.f, -1.f);

	_setHideAnim(m_configGCLblGCGameVideo, "CONFIGGC/GC_VIDEO", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configGCLblGCVideo, "CONFIGGC/GC_VIDEO_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configGCBtnGCVideoM, "CONFIGGC/GC_VIDEO_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configGCBtnGCVideoP, "CONFIGGC/GC_VIDEO_PLUS", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_configGCLblGCGameLanguage, "CONFIGGC/GC_LANG", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configGCLblGCLanguage, "CONFIGGC/GC_LANG_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configGCBtnGCLanguageM, "CONFIGGC/GC_LANG_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configGCBtnGCLanguageP, "CONFIGGC/GC_LANG_PLUS", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_configGCLblGCGameLoader, "CONFIGGC/GC_LOADER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configGCLblGCLoader, "CONFIGGC/GC_LOADER_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configGCBtnGCLoaderM, "CONFIGGC/GC_LOADER_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configGCBtnGCLoaderP, "CONFIGGC/GC_LOADER_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_configGCLblDevMemCard, "CONFIGGC/DEV_MEMCARD", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configGCBtnDevMemCard, "CONFIGGC/DEV_MEMCARD_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_configGCLblNinMemCard, "CONFIGGC/NIN_MEMCARD", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configGCLblNinMemCardVal, "CONFIGGC/NIN_MEMCARD_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configGCBtnNinMemCardM, "CONFIGGC/NIN_MEMCARD_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configGCBtnNinMemCardP, "CONFIGGC/NIN_MEMCARD_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_configGCLblNinWiiUWide, "CONFIGGC/NIN_WIIU_WIDE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configGCBtnNinWiiUWide, "CONFIGGC/NIN_WIIU_WIDE_BTN", -50, 0, 1.f, 0.f);

	_hideConfigGC(true);
	_textConfigGC();
}

void CMenu::_textConfigGC(void)
{
	m_btnMgr.setText(m_configGCLblTitle, _t("cfgb8", L"GC Default Settings"));
	m_btnMgr.setText(m_configGCBtnBack, _t("cfg10", L"Back"));

	m_btnMgr.setText(m_configGCLblGCGameVideo, _t("cfgb5", L"Video mode"));
	m_btnMgr.setText(m_configGCLblGCGameLanguage, _t("cfgb6", L"Game language"));
	m_btnMgr.setText(m_configGCLblGCGameLoader, _t("cfgb2", L"Game loader"));
	m_btnMgr.setText(m_configGCLblDevMemCard, _t("cfgb10", L"Devo Emu Memcard"));
	m_btnMgr.setText(m_configGCLblNinMemCard, _t("cfgb11", L"Nintendont Emu Memcard"));
	m_btnMgr.setText(m_configGCLblNinWiiUWide, _t("cfgb12", L"Nintendont Wii U Widescreen"));
}
