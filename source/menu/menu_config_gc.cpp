
#include "menu.hpp"

s16 m_configGCLblTitle;
s16 m_configGCBtnBack;
s16 m_configGCLblUser[4];

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

TexData m_configGCBg;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfigGC(bool instant)
{
	m_btnMgr.hide(m_configGCLblTitle, instant);
	m_btnMgr.hide(m_configGCBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_configGCLblUser); ++i)
		if(m_configGCLblUser[i] != -1)
			m_btnMgr.hide(m_configGCLblUser[i], instant);

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
}

void CMenu::_showConfigGC(void)
{
	m_btnMgr.show(m_configGCLblTitle);
	m_btnMgr.show(m_configGCBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_configGCLblUser); ++i)
		if(m_configGCLblUser[i] != -1)
			m_btnMgr.show(m_configGCLblUser[i]);

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

	int i = min(max(0, m_cfg.getInt(GC_DOMAIN, "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_GlobalGCvideoModes) - 1);
	m_btnMgr.setText(m_configGCLblGCVideo, _t(CMenu::_GlobalGCvideoModes[i].id, CMenu::_GlobalGCvideoModes[i].text));
	
	i = min(max(0, m_cfg.getInt(GC_DOMAIN, "game_language", 0)), (int)ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1);
	m_btnMgr.setText(m_configGCLblGCLanguage, _t(CMenu::_GlobalGClanguages[i].id, CMenu::_GlobalGClanguages[i].text));

	i = min(max(0, m_cfg.getInt(GC_DOMAIN, "default_loader", 2)), (int)ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1);
	m_btnMgr.setText(m_configGCLblGCLoader, _t(CMenu::_GlobalGCLoaders[i].id, CMenu::_GlobalGCLoaders[i].text));
}

void CMenu::_configGC(void)
{
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
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_configGCBtnBack))
				break;
			else if(m_btnMgr.selected(m_configGCBtnGCLanguageP) || m_btnMgr.selected(m_configGCBtnGCLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_configGCBtnGCLanguageP) ? 1 : -1;
				m_cfg.setInt(GC_DOMAIN, "game_language", (int)loopNum(m_cfg.getUInt(GC_DOMAIN, "game_language", 0) + direction, ARRAY_SIZE(CMenu::_GlobalGClanguages)));
				_showConfigGC();
			}
			else if(m_btnMgr.selected(m_configGCBtnGCVideoP) || m_btnMgr.selected(m_configGCBtnGCVideoM))
			{
				s8 direction = m_btnMgr.selected(m_configGCBtnGCVideoP) ? 1 : -1;
				m_cfg.setInt(GC_DOMAIN, "video_mode", (int)loopNum(m_cfg.getUInt(GC_DOMAIN, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_GlobalGCvideoModes)));
				_showConfigGC();
			}
			else if(m_btnMgr.selected(m_configGCBtnGCLoaderP) || m_btnMgr.selected(m_configGCBtnGCLoaderM))
			{
				s8 direction = m_btnMgr.selected(m_configGCBtnGCLoaderP) ? 1 : -1;
				m_cfg.setInt(GC_DOMAIN, "default_loader", (int)loopNum(m_cfg.getUInt(GC_DOMAIN, "default_loader", 1) + direction, ARRAY_SIZE(CMenu::_GlobalGCLoaders)));
				_showConfigGC();
			}
		}
	}
	_hideConfigGC();
}

void CMenu::_initConfigGCMenu(void)
{
	m_configGCBg = _texture("CONFIGGC/BG", "texture", theme.bg, false);

	_addUserLabels(m_configGCLblUser, ARRAY_SIZE(m_configGCLblUser), "CONFIGGC");
	m_configGCLblTitle = _addTitle("CONFIGGC/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_configGCBtnBack = _addButton("CONFIGGC/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

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
	
	_setHideAnim(m_configGCLblTitle, "CONFIGGC/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configGCBtnBack, "CONFIGGC/BACK_BTN", 0, 0, 1.f, -1.f);

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
	
	_hideConfigGC(true);
	_textConfigGC();
}

void CMenu::_textConfigGC(void)
{
	m_btnMgr.setText(m_configGCLblTitle, _t("cfgb8", L"GameCube Settings"));
	m_btnMgr.setText(m_configGCLblGCGameVideo, _t("cfgb5", L"Default GC video mode"));
	m_btnMgr.setText(m_configGCLblGCGameLanguage, _t("cfgb6", L"Default GC game language"));
	m_btnMgr.setText(m_configGCLblGCGameLoader, _t("cfgb2", L"Default GC game loader"));
	m_btnMgr.setText(m_configGCBtnBack, _t("cfg10", L"Back"));
}

