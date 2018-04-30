
#include "menu.hpp"

static const int g_curPage = 3;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfig3(bool instant)
{
	_hideConfigCommon(instant);

	m_btnMgr.hide(m_config3LblGameLanguage, instant);
	m_btnMgr.hide(m_config3LblLanguage, instant);
	m_btnMgr.hide(m_config3BtnLanguageP, instant);
	m_btnMgr.hide(m_config3BtnLanguageM, instant);
	
	m_btnMgr.hide(m_config3LblGameVideo, instant);
	m_btnMgr.hide(m_config3LblVideo, instant);
	m_btnMgr.hide(m_config3BtnVideoP, instant);
	m_btnMgr.hide(m_config3BtnVideoM, instant);
	
	m_btnMgr.hide(m_config3LblGCDefaults, instant);
	m_btnMgr.hide(m_config3BtnGCDefaults, instant);
	
	m_btnMgr.hide(m_config3LblChannelsType, instant);
	m_btnMgr.hide(m_config3LblChannelsTypeVal, instant);
	m_btnMgr.hide(m_config3BtnChannelsTypeP, instant);
	m_btnMgr.hide(m_config3BtnChannelsTypeM, instant);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_config3LblUser); ++i)
		if(m_config3LblUser[i] != -1)
			m_btnMgr.hide(m_config3LblUser[i], instant);
}

void CMenu::_showConfig3(void)
{
	_showConfigCommon(m_config3Bg, g_curPage);

	m_btnMgr.show(m_config3LblGameLanguage);
	m_btnMgr.show(m_config3LblLanguage);
	m_btnMgr.show(m_config3BtnLanguageP);
	m_btnMgr.show(m_config3BtnLanguageM);
	
	m_btnMgr.show(m_config3LblGameVideo);
	m_btnMgr.show(m_config3LblVideo);
	m_btnMgr.show(m_config3BtnVideoP);
	m_btnMgr.show(m_config3BtnVideoM);
	
	m_btnMgr.show(m_config3LblGCDefaults);
	m_btnMgr.show(m_config3BtnGCDefaults);

	m_btnMgr.show(m_config3LblChannelsType);
	m_btnMgr.show(m_config3LblChannelsTypeVal);
	m_btnMgr.show(m_config3BtnChannelsTypeP);
	m_btnMgr.show(m_config3BtnChannelsTypeM);

	for(u8 i = 0; i < ARRAY_SIZE(m_config3LblUser); ++i)
		if(m_config3LblUser[i] != -1)
			m_btnMgr.show(m_config3LblUser[i]);

	int i = min(max(0, m_cfg.getInt("GENERAL", "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1);
	m_btnMgr.setText(m_config3LblVideo, _t(CMenu::_GlobalVideoModes[i].id, CMenu::_GlobalVideoModes[i].text));

	i = min(max(0, m_cfg.getInt("GENERAL", "game_language", 0)), (int)ARRAY_SIZE(CMenu::_languages) - 1);
	m_btnMgr.setText(m_config3LblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
	
	i = m_cfg.getInt(CHANNEL_DOMAIN, "channels_type") - 1;
	m_btnMgr.setText(m_config3LblChannelsTypeVal, _t(CMenu::_ChannelsType[i].id, CMenu::_ChannelsType[i].text));
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
			else if (m_btnMgr.selected(m_config3BtnGCDefaults))
			{
				_hideConfig3();
				_configGC();
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnChannelsTypeP) || m_btnMgr.selected(m_config3BtnChannelsTypeM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnChannelsTypeP) ? 1 : -1;
				m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", 1 + (int)loopNum((u32)m_cfg.getInt(CHANNEL_DOMAIN, "channels_type", 1) - 1 + direction, ARRAY_SIZE(CMenu::_ChannelsType)));
				_showConfig3();
				if(m_current_view & COVERFLOW_CHANNEL || m_current_view & COVERFLOW_PLUGIN)
					m_refreshGameList = true;
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

	m_config3LblGCDefaults = _addLabel("CONFIG3/GC_DEFAULTS", theme.lblFont, L"", 20, 245, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3BtnGCDefaults = _addButton("CONFIG3/GC_DEFAULTS_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	
	m_config3LblChannelsType = _addLabel("CONFIG3/CHANNELS_TYPE", theme.lblFont, L"", 20, 305, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblChannelsTypeVal = _addLabel("CONFIG3/CHANNELS_TYPE_BTN", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnChannelsTypeM = _addPicButton("CONFIG3/CHANNELS_TYPE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_config3BtnChannelsTypeP = _addPicButton("CONFIG3/CHANNELS_TYPE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);

	_setHideAnim(m_config3LblGameVideo, "CONFIG3/VIDEO", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblVideo, "CONFIG3/VIDEO_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnVideoM, "CONFIG3/VIDEO_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnVideoP, "CONFIG3/VIDEO_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3LblGameLanguage, "CONFIG3/GAME_LANG", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblLanguage, "CONFIG3/GAME_LANG_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnLanguageM, "CONFIG3/GAME_LANG_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnLanguageP, "CONFIG3/GAME_LANG_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_config3LblGCDefaults, "CONFIG3/GC_DEFAULTS", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3BtnGCDefaults, "CONFIG3/GC_DEFAULTS_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_config3LblChannelsType, "CONFIG3/CHANNELS_TYPE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblChannelsTypeVal, "CONFIG3/CHANNELS_TYPE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnChannelsTypeM, "CONFIG3/CHANNELS_TYPE_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config3BtnChannelsTypeP, "CONFIG3/CHANNELS_TYPE_PLUS", -50, 0, 1.f, 0.f);
	_hideConfig3(true);
	_textConfig3();
}

void CMenu::_textConfig3(void)
{
	m_btnMgr.setText(m_config3LblGameVideo, _t("cfgb3", L"Default video mode"));
	m_btnMgr.setText(m_config3LblGameLanguage, _t("cfgb4", L"Default game language"));
	m_btnMgr.setText(m_config3LblGCDefaults, _t("cfgb9", L"GameCube default settings"));
	m_btnMgr.setText(m_config3BtnGCDefaults, _t("dl16", L"Set"));
	m_btnMgr.setText(m_config3LblChannelsType, _t("cfgb7", L"Channels Type"));
}

