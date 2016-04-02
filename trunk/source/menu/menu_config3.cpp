
#include "menu.hpp"

static const int g_curPage = 3;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfig3(bool instant)
{
	_hideConfigCommon(instant);

	if(m_current_view != COVERFLOW_GAMECUBE)
	{
		m_btnMgr.hide(m_config3LblGameLanguage, instant);
		m_btnMgr.hide(m_config3LblLanguage, instant);
		m_btnMgr.hide(m_config3BtnLanguageP, instant);
		m_btnMgr.hide(m_config3BtnLanguageM, instant);
		m_btnMgr.hide(m_config3LblGameVideo, instant);
		m_btnMgr.hide(m_config3LblVideo, instant);
		m_btnMgr.hide(m_config3BtnVideoP, instant);
		m_btnMgr.hide(m_config3BtnVideoM, instant);
	}
	else
	{
		m_btnMgr.hide(m_config3LblDMLGameLanguage, instant);
		m_btnMgr.hide(m_config3LblDMLLanguage, instant);
		m_btnMgr.hide(m_config3BtnDMLLanguageP, instant);
		m_btnMgr.hide(m_config3BtnDMLLanguageM, instant);
		m_btnMgr.hide(m_config3LblDMLGameVideo, instant);
		m_btnMgr.hide(m_config3LblDMLVideo, instant);
		m_btnMgr.hide(m_config3BtnDMLVideoP, instant);
		m_btnMgr.hide(m_config3BtnDMLVideoM, instant);
	}
	m_btnMgr.hide(m_config3LblDMLGameLoader, instant);
	m_btnMgr.hide(m_config3LblDMLLoader, instant);
	m_btnMgr.hide(m_config3BtnDMLLoaderP, instant);
	m_btnMgr.hide(m_config3BtnDMLLoaderM, instant);
	m_btnMgr.hide(m_config3LblOcarina, instant);
	m_btnMgr.hide(m_config3BtnOcarina, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_config3LblUser); ++i)
		if(m_config3LblUser[i] != -1)
			m_btnMgr.hide(m_config3LblUser[i], instant);
}

void CMenu::_showConfig3(void)
{
	_showConfigCommon(m_config3Bg, g_curPage);

	if(m_current_view != COVERFLOW_GAMECUBE)
	{
		m_btnMgr.show(m_config3LblGameLanguage);
		m_btnMgr.show(m_config3LblLanguage);
		m_btnMgr.show(m_config3BtnLanguageP);
		m_btnMgr.show(m_config3BtnLanguageM);
		m_btnMgr.show(m_config3LblGameVideo);
		m_btnMgr.show(m_config3LblVideo);
		m_btnMgr.show(m_config3BtnVideoP);
		m_btnMgr.show(m_config3BtnVideoM);
	}
	else
	{
		m_btnMgr.show(m_config3LblDMLGameLanguage);
		m_btnMgr.show(m_config3LblDMLLanguage);
		m_btnMgr.show(m_config3BtnDMLLanguageP);
		m_btnMgr.show(m_config3BtnDMLLanguageM);
		m_btnMgr.show(m_config3LblDMLGameVideo);
		m_btnMgr.show(m_config3LblDMLVideo);
		m_btnMgr.show(m_config3BtnDMLVideoP);
		m_btnMgr.show(m_config3BtnDMLVideoM);
	}
	m_btnMgr.show(m_config3LblDMLGameLoader);
	m_btnMgr.show(m_config3LblDMLLoader);
	m_btnMgr.show(m_config3BtnDMLLoaderP);
	m_btnMgr.show(m_config3BtnDMLLoaderM);
	m_btnMgr.show(m_config3LblOcarina);
	m_btnMgr.show(m_config3BtnOcarina);

	for(u8 i = 0; i < ARRAY_SIZE(m_config3LblUser); ++i)
		if(m_config3LblUser[i] != -1)
			m_btnMgr.show(m_config3LblUser[i]);

	int i;

	if(m_current_view != COVERFLOW_GAMECUBE)
	{
		i = min(max(0, m_cfg.getInt("GENERAL", "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1);
		m_btnMgr.setText(m_config3LblVideo, _t(CMenu::_GlobalVideoModes[i].id, CMenu::_GlobalVideoModes[i].text));

		i = min(max(0, m_cfg.getInt("GENERAL", "game_language", 0)), (int)ARRAY_SIZE(CMenu::_languages) - 1);
		m_btnMgr.setText(m_config3LblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
	}
	else
	{
		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_GlobalDMLvideoModes) - 1);
		m_btnMgr.setText(m_config3LblDMLVideo, _t(CMenu::_GlobalDMLvideoModes[i].id, CMenu::_GlobalDMLvideoModes[i].text));
		
		i = min(max(0, m_cfg.getInt(GC_DOMAIN, "game_language", 0)), (int)ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1);
		m_btnMgr.setText(m_config3LblDMLLanguage, _t(CMenu::_GlobalGClanguages[i].id, CMenu::_GlobalGClanguages[i].text));
	}
	i = min(max(0, m_cfg.getInt(GC_DOMAIN, "default_loader", 2)), (int)ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1);
	m_btnMgr.setText(m_config3LblDMLLoader, _t(CMenu::_GlobalGCLoaders[i].id, CMenu::_GlobalGCLoaders[i].text));

	m_btnMgr.setText(m_config3BtnOcarina, m_cfg.getBool(_domainFromView(), "cheat", false) ? _t("on", L"On") : _t("off", L"Off"));
}

int CMenu::_config3(void)
{
	int change = CONFIG_PAGE_NO_CHANGE;

	_showConfig3();
	while(!m_exit)
	{
		change = _configCommon();
		if (change != CONFIG_PAGE_NO_CHANGE)
			break;
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_config3BtnLanguageP) || m_btnMgr.selected(m_config3BtnLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnLanguageP) ? 1 : -1;
				m_cfg.setInt("GENERAL", "game_language", (int)loopNum((u32)m_cfg.getInt("GENERAL", "game_language", 0) + direction, ARRAY_SIZE(CMenu::_languages)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnVideoP) || m_btnMgr.selected(m_config3BtnVideoM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnVideoP) ? 1 : -1;
				m_cfg.setInt("GENERAL", "video_mode", (int)loopNum((u32)m_cfg.getInt("GENERAL", "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_GlobalVideoModes)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnDMLLanguageP) || m_btnMgr.selected(m_config3BtnDMLLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnDMLLanguageP) ? 1 : -1;
				m_cfg.setInt(GC_DOMAIN, "game_language", (int)loopNum((u32)m_cfg.getInt(GC_DOMAIN, "game_language", 0) + direction, ARRAY_SIZE(CMenu::_GlobalGClanguages)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnDMLVideoP) || m_btnMgr.selected(m_config3BtnDMLVideoM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnDMLVideoP) ? 1 : -1;
				m_cfg.setInt(GC_DOMAIN, "video_mode", (int)loopNum((u32)m_cfg.getInt(GC_DOMAIN, "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_GlobalDMLvideoModes)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnDMLLoaderP) || m_btnMgr.selected(m_config3BtnDMLLoaderM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnDMLLoaderP) ? 1 : -1;
				m_cfg.setInt(GC_DOMAIN, "default_loader", (int)loopNum((u32)m_cfg.getInt(GC_DOMAIN, "default_loader", 2) + direction, ARRAY_SIZE(CMenu::_GlobalGCLoaders)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnOcarina))
			{
				m_cfg.setBool(_domainFromView(), "cheat", !m_cfg.getBool(_domainFromView(), "cheat", false));
				_showConfig3();
			}
		}
	}
	_hideConfig3();
	return change;
}

void CMenu::_initConfig3Menu()
{
	_addUserLabels(m_config3LblUser, ARRAY_SIZE(m_config3LblUser), "CONFIG3");
	m_config3Bg = _texture("CONFIG3/BG", "texture", theme.bg, false);
	m_config3LblGameVideo = _addLabel("CONFIG3/VIDEO", theme.lblFont, L"", 20, 125, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblVideo = _addLabel("CONFIG3/VIDEO_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnVideoM = _addPicButton("CONFIG3/VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_config3BtnVideoP = _addPicButton("CONFIG3/VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	m_config3LblGameLanguage = _addLabel("CONFIG3/GAME_LANG", theme.lblFont, L"", 20, 185, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblLanguage = _addLabel("CONFIG3/GAME_LANG_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnLanguageM = _addPicButton("CONFIG3/GAME_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_config3BtnLanguageP = _addPicButton("CONFIG3/GAME_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);

	m_config3LblDMLGameVideo = _addLabel("CONFIG3/DML_VIDEO", theme.lblFont, L"", 20, 125, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblDMLVideo = _addLabel("CONFIG3/DML_VIDEO_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnDMLVideoM = _addPicButton("CONFIG3/DML_VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_config3BtnDMLVideoP = _addPicButton("CONFIG3/DML_VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	m_config3LblDMLGameLanguage = _addLabel("CONFIG3/DML_LANG", theme.lblFont, L"", 20, 185, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblDMLLanguage = _addLabel("CONFIG3/DML_LANG_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnDMLLanguageM = _addPicButton("CONFIG3/DML_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_config3BtnDMLLanguageP = _addPicButton("CONFIG3/DML_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	m_config3LblDMLGameLoader = _addLabel("CONFIG3/GC_LOADER", theme.lblFont, L"", 20, 245, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblDMLLoader = _addLabel("CONFIG3/GC_LOADER_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnDMLLoaderM = _addPicButton("CONFIG3/GC_LOADER_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_config3BtnDMLLoaderP = _addPicButton("CONFIG3/GC_LOADER_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);
	
	m_config3LblOcarina = _addLabel("CONFIG3/OCARINA", theme.lblFont, L"", 20, 305, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3BtnOcarina = _addButton("CONFIG3/OCARINA_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

	_setHideAnim(m_config3LblGameVideo, "CONFIG3/VIDEO", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblVideo, "CONFIG3/VIDEO_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnVideoM, "CONFIG3/VIDEO_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnVideoP, "CONFIG3/VIDEO_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3LblGameLanguage, "CONFIG3/GAME_LANG", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblLanguage, "CONFIG3/GAME_LANG_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnLanguageM, "CONFIG3/GAME_LANG_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnLanguageP, "CONFIG3/GAME_LANG_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_config3LblDMLGameVideo, "CONFIG3/DML_VIDEO", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblDMLVideo, "CONFIG3/DML_VIDEO_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnDMLVideoM, "CONFIG3/DML_VIDEO_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnDMLVideoP, "CONFIG3/DML_VIDEO_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3LblDMLGameLanguage, "CONFIG3/DML_LANG", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblDMLLanguage, "CONFIG3/DML_LANG_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnDMLLanguageM, "CONFIG3/DML_LANG_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnDMLLanguageP, "CONFIG3/DML_LANG_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3LblDMLGameLoader, "CONFIG3/GC_LOADER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblDMLLoader, "CONFIG3/GC_LOADER_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnDMLLoaderM, "CONFIG3/GC_LOADER_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnDMLLoaderP, "CONFIG3/GC_LOADER_PLUS", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_config3LblOcarina, "CONFIG3/OCARINA", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3BtnOcarina, "CONFIG3/OCARINA_BTN", -50, 0, 1.f, 0.f);
	_hideConfig3(true);
	_textConfig3();
}

void CMenu::_textConfig3(void)
{
	m_btnMgr.setText(m_config3LblGameVideo, _t("cfgb3", L"Default video mode"));
	m_btnMgr.setText(m_config3LblGameLanguage, _t("cfgb4", L"Default game language"));
	m_btnMgr.setText(m_config3LblDMLGameVideo, _t("cfgb5", L"Default GC video mode"));
	m_btnMgr.setText(m_config3LblDMLGameLanguage, _t("cfgb6", L"Default GC game language"));
	m_btnMgr.setText(m_config3LblDMLGameLoader, _t("cfgb2", L"Default GC game loader"));
	m_btnMgr.setText(m_config3LblOcarina, _t("cfgb1", L"Ocarina"));
}

