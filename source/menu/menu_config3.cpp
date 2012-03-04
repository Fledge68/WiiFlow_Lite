
#include "menu.hpp"


#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

using namespace std;

static const int g_curPage = 3;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_hideConfig3(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);

	if(m_current_view != COVERFLOW_DML)
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
	m_btnMgr.hide(m_config3LblAsyncNet, instant);
	m_btnMgr.hide(m_config3BtnAsyncNet, instant);
	m_btnMgr.hide(m_config3LblOcarina, instant);
	m_btnMgr.hide(m_config3BtnOcarina, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_config3LblUser); ++i)
		if (m_config3LblUser[i] != -1u)
			m_btnMgr.hide(m_config3LblUser[i], instant);
}

void CMenu::_showConfig3(void)
{
	_setBg(m_config3Bg, m_config3Bg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);

	if(m_current_view != COVERFLOW_DML)
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
	m_btnMgr.show(m_config3LblAsyncNet);
	m_btnMgr.show(m_config3BtnAsyncNet);
	m_btnMgr.show(m_config3LblOcarina);
	m_btnMgr.show(m_config3BtnOcarina);

	for (u32 i = 0; i < ARRAY_SIZE(m_config3LblUser); ++i)
		if (m_config3LblUser[i] != -1u)
			m_btnMgr.show(m_config3LblUser[i]);

	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));
	int i;

	if(m_current_view != COVERFLOW_DML)
	{
		i = min(max(0, m_cfg.getInt("GENERAL", "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_videoModes) - 1);
		m_btnMgr.setText(m_config3LblVideo, _t(CMenu::_videoModes[i].id, CMenu::_videoModes[i].text));
		
		i = min(max(0, m_cfg.getInt("GENERAL", "game_language", 0)), (int)ARRAY_SIZE(CMenu::_languages) - 1);
		m_btnMgr.setText(m_config3LblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
	}
	else
	{
		i = min(max(0, m_cfg.getInt("DML", "video_mode", 0)), (int)ARRAY_SIZE(CMenu::_GlobalDMLvideoModes) - 1);
		m_btnMgr.setText(m_config3LblDMLVideo, _t(CMenu::_GlobalDMLvideoModes[i].id, CMenu::_GlobalDMLvideoModes[i].text));
		
		i = min(max(0, m_cfg.getInt("DML", "game_language", 0)), (int)ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1);
		m_btnMgr.setText(m_config3LblDMLLanguage, _t(CMenu::_GlobalGClanguages[i].id, CMenu::_GlobalGClanguages[i].text));
	}

	m_btnMgr.setText(m_config3BtnAsyncNet, m_cfg.getBool("GENERAL", "async_network", false) ? _t("on", L"On") : _t("off", L"Off"));

	m_btnMgr.setText(m_config3BtnOcarina, m_cfg.getBool(_domainFromView(), "cheat", false) ? _t("on", L"On") : _t("off", L"Off"));
}

int CMenu::_config3(void)
{
	int nextPage = 0;

	_showConfig3();
	while (true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if (BTN_LEFT_PRESSED || BTN_MINUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageM)))
		{
			nextPage = max(1, m_locked ? 1 : g_curPage - 1);
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_configBtnPageM);
			break;
		}
		if (!m_locked && (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageP))))
		{
			nextPage = min(g_curPage + 1, CMenu::_nbCfgPages);
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_configBtnPageP);
			break;
		}
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_configBtnBack))
				break;
			else if (m_btnMgr.selected(m_config3BtnLanguageP) || m_btnMgr.selected(m_config3BtnLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnLanguageP) ? 1 : -1;
				m_cfg.setInt("GENERAL", "game_language", (int)loopNum((u32)m_cfg.getInt("GENERAL", "game_language", 0) + direction, ARRAY_SIZE(CMenu::_languages)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnVideoP) || m_btnMgr.selected(m_config3BtnVideoM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnVideoP) ? 1 : -1;
				m_cfg.setInt("GENERAL", "video_mode", (int)loopNum((u32)m_cfg.getInt("GENERAL", "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_videoModes)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnDMLLanguageP) || m_btnMgr.selected(m_config3BtnDMLLanguageM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnDMLLanguageP) ? 1 : -1;
				m_cfg.setInt("DML", "game_language", (int)loopNum((u32)m_cfg.getInt("DML", "game_language", 0) + direction, ARRAY_SIZE(CMenu::_GlobalGClanguages)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnDMLVideoP) || m_btnMgr.selected(m_config3BtnDMLVideoM))
			{
				s8 direction = m_btnMgr.selected(m_config3BtnDMLVideoP) ? 1 : -1;
				m_cfg.setInt("DML", "video_mode", (int)loopNum((u32)m_cfg.getInt("DML", "video_mode", 0) + direction, ARRAY_SIZE(CMenu::_GlobalDMLvideoModes)));
				_showConfig3();
			}
			else if (m_btnMgr.selected(m_config3BtnAsyncNet))
			{
				m_cfg.setBool("GENERAL", "async_network", !m_cfg.getBool("GENERAL", "async_network", false));
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
	return nextPage;
}

void CMenu::_initConfig3Menu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_config3LblUser, ARRAY_SIZE(m_config3LblUser), "CONFIG3");
	m_config3Bg = _texture(theme.texSet, "CONFIG3/BG", "texture", theme.bg);
	m_config3LblGameVideo = _addLabel(theme, "CONFIG3/VIDEO", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblVideo = _addLabel(theme, "CONFIG3/VIDEO_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnVideoM = _addPicButton(theme, "CONFIG3/VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_config3BtnVideoP = _addPicButton(theme, "CONFIG3/VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_config3LblGameLanguage = _addLabel(theme, "CONFIG3/GAME_LANG", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblLanguage = _addLabel(theme, "CONFIG3/GAME_LANG_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnLanguageM = _addPicButton(theme, "CONFIG3/GAME_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_config3BtnLanguageP = _addPicButton(theme, "CONFIG3/GAME_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);

	m_config3LblDMLGameVideo = _addLabel(theme, "CONFIG3/DML_VIDEO", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblDMLVideo = _addLabel(theme, "CONFIG3/DML_VIDEO_BTN", theme.btnFont, L"", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnDMLVideoM = _addPicButton(theme, "CONFIG3/DML_VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_config3BtnDMLVideoP = _addPicButton(theme, "CONFIG3/DML_VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_config3LblDMLGameLanguage = _addLabel(theme, "CONFIG3/DML_LANG", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3LblDMLLanguage = _addLabel(theme, "CONFIG3/DML_LANG_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config3BtnDMLLanguageM = _addPicButton(theme, "CONFIG3/DML_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_config3BtnDMLLanguageP = _addPicButton(theme, "CONFIG3/DML_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);

	m_config3LblAsyncNet = _addLabel(theme, "CONFIG3/ASYNCNET", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3BtnAsyncNet = _addButton(theme, "CONFIG3/ASYNCNET_BTN", theme.btnFont, L"", 330, 250, 270, 56, theme.btnFontColor);
	m_config3LblOcarina = _addLabel(theme, "CONFIG3/OCARINA", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config3BtnOcarina = _addButton(theme, "CONFIG3/OCARINA_BTN", theme.btnFont, L"", 330, 310, 270, 56, theme.btnFontColor);

	_setHideAnim(m_config3LblGameVideo, "CONFIG3/VIDEO", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblVideo, "CONFIG3/VIDEO_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnVideoM, "CONFIG3/VIDEO_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnVideoP, "CONFIG3/VIDEO_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3LblGameLanguage, "CONFIG3/GAME_LANG", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblLanguage, "CONFIG3/GAME_LANG_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnLanguageM, "CONFIG3/GAME_LANG_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnLanguageP, "CONFIG3/GAME_LANG_PLUS", 0, 0, 1.f, -1.f);

	_setHideAnim(m_config3LblDMLGameVideo, "CONFIG3/DML_VIDEO", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblDMLVideo, "CONFIG3/DML_VIDEO_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnDMLVideoM, "CONFIG3/DML_VIDEO_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnDMLVideoP, "CONFIG3/DML_VIDEO_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3LblDMLGameLanguage, "CONFIG3/DML_LANG", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3LblDMLLanguage, "CONFIG3/DML_LANG_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnDMLLanguageM, "CONFIG3/DML_LANG_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3BtnDMLLanguageP, "CONFIG3/DML_LANG_PLUS", 0, 0, 1.f, -1.f);

	_setHideAnim(m_config3LblAsyncNet, "CONFIG3/ASYNCNET", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3BtnAsyncNet, "CONFIG3/ASYNCNET_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config3LblOcarina, "CONFIG3/OCARINA", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config3BtnOcarina, "CONFIG3/OCARINA_BTN", 0, 0, 1.f, -1.f);
	_hideConfig3(true);
	_textConfig3();
}

void CMenu::_textConfig3(void)
{
	m_btnMgr.setText(m_config3LblGameVideo, _t("cfgb3", L"Default video mode"));
	m_btnMgr.setText(m_config3LblGameLanguage, _t("cfgb4", L"Default game language"));
	m_btnMgr.setText(m_config3LblDMLGameVideo, _t("cfgb5", L"Default DML video mode"));
	m_btnMgr.setText(m_config3LblDMLGameLanguage, _t("cfgb6", L"Default DML game language"));

	m_btnMgr.setText(m_config3LblAsyncNet, _t("cfgp3", L"Init network on boot"));
	m_btnMgr.setText(m_config3LblOcarina, _t("cfgb1", L"Ocarina"));
}

