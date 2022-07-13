
#include "menu.hpp"

TexData m_configGCBg;

s16 m_configGCLblTitle;
s16 m_configGCBtnBack;
s16 m_configGCLblUser[4];

s16 m_configGCLblPage;
s16 m_configGCBtnPageM;
s16 m_configGCBtnPageP;

u8 configGC_curPage = 1;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfigGC(bool instant)
{
	m_btnMgr.hide(m_configGCLblTitle, instant);
	m_btnMgr.hide(m_configGCBtnBack, instant);
	m_btnMgr.hide(m_configGCLblPage, instant);
	m_btnMgr.hide(m_configGCBtnPageM, instant);
	m_btnMgr.hide(m_configGCBtnPageP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_configGCLblUser); ++i)
		if(m_configGCLblUser[i] != -1)
			m_btnMgr.hide(m_configGCLblUser[i], instant);
	_hideConfigButtons(instant);
}

void CMenu::_showConfigGC(void)
{
	_setBg(m_configGCBg, m_configGCBg);
	m_btnMgr.show(m_configGCLblTitle);
	m_btnMgr.show(m_configGCBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_configGCLblUser); ++i)
		if(m_configGCLblUser[i] != -1)
			m_btnMgr.show(m_configGCLblUser[i]);

	m_btnMgr.show(m_configGCLblPage);
	m_btnMgr.show(m_configGCBtnPageM);
	m_btnMgr.show(m_configGCBtnPageP);
	m_btnMgr.setText(m_configGCLblPage, wfmt(L"%i / %i", configGC_curPage, 2));

	_hideConfigButtons(true);

	int i;
	if(configGC_curPage == 1)
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgb5", L"Video mode"));
		m_btnMgr.setText(m_configLbl2, _t("cfgb6", L"Game language"));
		m_btnMgr.setText(m_configLbl3, _t("cfgb2", L"Game loader"));
		m_btnMgr.setText(m_configLbl4, _t("cfgb10", L"Devo Emu Memcard"));
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configLbl4);
		
		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_GlobalGCvideoModes) - 1);
		m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_GlobalGCvideoModes[i].id, CMenu::_GlobalGCvideoModes[i].text));
		
		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "game_language", 0)), (int)ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1);
		m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_GlobalGClanguages[i].id, CMenu::_GlobalGClanguages[i].text));

		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "default_loader", 2)), (int)ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1);
		m_btnMgr.setText(m_configLbl3Val, _t(CMenu::_GlobalGCLoaders[i].id, CMenu::_GlobalGCLoaders[i].text));

		m_btnMgr.setText(m_configBtn4, m_cfg.getBool(GC_DOMAIN, "devo_memcard_emu", false) ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1M);
		m_btnMgr.show(m_configBtn1P);
		m_btnMgr.show(m_configLbl2Val);
		m_btnMgr.show(m_configBtn2M);
		m_btnMgr.show(m_configBtn2P);
		m_btnMgr.show(m_configLbl3Val);
		m_btnMgr.show(m_configBtn3M);
		m_btnMgr.show(m_configBtn3P);
		m_btnMgr.show(m_configBtn4);
	}
	else
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgb11", L"Nintendont Emu Memcard"));
		m_btnMgr.setText(m_configLbl2, _t("cfgb12", L"Nintendont Wii U Widescreen"));
		m_btnMgr.show(m_configLbl1);
		m_btnMgr.show(m_configLbl2);
		
		// minus 2 and [i + 1] because there is no global array that does not include 'default'
		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "emu_memcard", 1)), (int)ARRAY_SIZE(CMenu::_NinEmuCard) - 2);
		m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_NinEmuCard[i + 1].id, CMenu::_NinEmuCard[i + 1].text));
		
		m_btnMgr.setText(m_configBtn2, m_cfg.getBool(GC_DOMAIN, "wiiu_widescreen", false) ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_configLbl1Val);
		m_btnMgr.show(m_configBtn1M);
		m_btnMgr.show(m_configBtn1P);
		m_btnMgr.show(m_configBtn2);
	}
}

void CMenu::_configGC(void)
{
	int i;
	bool j;
	configGC_curPage = 1;
	SetupInput();
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
			configGC_curPage--;
			if(configGC_curPage < 1)
				configGC_curPage = 2;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_configGCBtnPageM);
			_showConfigGC();
		}
		else if(BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configGCBtnPageP)))
		{
			configGC_curPage++;
			if(configGC_curPage > 2)
				configGC_curPage = 1;
			if(!BTN_A_PRESSED)
				m_btnMgr.click(m_configGCBtnPageP);
			_showConfigGC();
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_configGCBtnBack))
				break;
			if(configGC_curPage == 1)
			{
				if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
					i = loopNum(m_cfg.getInt(GC_DOMAIN, "game_language", 0) + direction, (int)ARRAY_SIZE(CMenu::_GlobalGClanguages));
					m_cfg.setInt(GC_DOMAIN, "game_language", i);
					m_btnMgr.setText(m_configLbl2Val, _t(CMenu::_GlobalGClanguages[i].id, CMenu::_GlobalGClanguages[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
					i = loopNum(m_cfg.getInt(GC_DOMAIN, "video_mode", 0) + direction, (int)ARRAY_SIZE(CMenu::_GlobalGCvideoModes));
					m_cfg.setInt(GC_DOMAIN, "video_mode", i);
					m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_GlobalGCvideoModes[i].id, CMenu::_GlobalGCvideoModes[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn3P) ? 1 : -1;
					i = loopNum(m_cfg.getInt(GC_DOMAIN, "default_loader", 1) + direction, (int)ARRAY_SIZE(CMenu::_GlobalGCLoaders));
					m_cfg.setInt(GC_DOMAIN, "default_loader", i);
					m_btnMgr.setText(m_configLbl3Val, _t(CMenu::_GlobalGCLoaders[i].id, CMenu::_GlobalGCLoaders[i].text));
				}
				else if(m_btnMgr.selected(m_configBtn4))
				{
					j = !m_cfg.getBool(GC_DOMAIN, "devo_memcard_emu", false);
					m_cfg.setBool(GC_DOMAIN, "devo_memcard_emu", j);
					m_btnMgr.setText(m_configBtn4, j ? _t("on", L"On") : _t("off", L"Off"));
				}
			}
			else
			{
				if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
				{
					s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
					i = loopNum(m_cfg.getInt(GC_DOMAIN, "emu_memcard", 1) + direction, (int)ARRAY_SIZE(CMenu::_NinEmuCard) - 1);
					m_cfg.setInt(GC_DOMAIN, "emu_memcard", i);
					m_btnMgr.setText(m_configLbl1Val, _t(CMenu::_NinEmuCard[i + 1].id, CMenu::_NinEmuCard[i + 1].text));
				}
				else if(m_btnMgr.selected(m_configBtn2))
				{
					j = !m_cfg.getBool(GC_DOMAIN, "wiiu_widescreen", false);
					m_cfg.setBool(GC_DOMAIN, "wiiu_widescreen", j);
					m_btnMgr.setText(m_configBtn2, j ? _t("on", L"On") : _t("off", L"Off"));
				}
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

	_setHideAnim(m_configGCLblTitle, "CONFIGGC/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configGCBtnBack, "CONFIGGC/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configGCLblPage, "CONFIGGC/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configGCBtnPageM, "CONFIGGC/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configGCBtnPageP, "CONFIGGC/PAGE_PLUS", 0, 0, 1.f, -1.f);

	_hideConfigGC(true);
	_textConfigGC();
}

void CMenu::_textConfigGC(void)
{
	m_btnMgr.setText(m_configGCLblTitle, _t("cfgb8", L"GC Default Settings"));
	m_btnMgr.setText(m_configGCBtnBack, _t("cfg10", L"Back"));
}
