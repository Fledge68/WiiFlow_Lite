
#include "menu.hpp"
#include "defines.h"
#include "channel/channels.h"
#include "channel/nand.hpp"
#include "loader/cios.h"
#include "loader/nk.h"

static const int g_curPage = 4;

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

int currentChannelIndex = -1;
int amountOfChannels = -1;

const CMenu::SOption CMenu::_exitTo[5] = {
	{ "def", L"Default" },
	{ "menu", L"System Menu" },
	{ "hbc", L"HBC" },
	{ "prii", L"Priiloader" },
	{ "bootmii", L"BootMii" }
};

void CMenu::_hideConfig4(bool instant)
{
	_hideConfigCommon(instant);

	m_btnMgr.hide(m_config4LblPathManager, instant);
	m_btnMgr.hide(m_config4BtnPathManager, instant);
	m_btnMgr.hide(m_config4LblSaveFavMode, instant);
	m_btnMgr.hide(m_config4BtnSaveFavMode, instant);
	m_btnMgr.hide(m_config4LblHome, instant);
	m_btnMgr.hide(m_config4BtnHome, instant);
	m_btnMgr.hide(m_config4LblReturnTo, instant);
	m_btnMgr.hide(m_config4LblReturnToVal, instant);
	m_btnMgr.hide(m_config4BtnReturnToM, instant);
	m_btnMgr.hide(m_config4BtnReturnToP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_config4LblUser); ++i)
		if(m_config4LblUser[i] != -1)
			m_btnMgr.hide(m_config4LblUser[i], instant);
}

void CMenu::_showConfig4(void)
{
	_showConfigCommon(m_config4Bg, g_curPage);

	m_btnMgr.show(m_config4LblPathManager);
	m_btnMgr.show(m_config4BtnPathManager);
	m_btnMgr.show(m_config4LblSaveFavMode);
	m_btnMgr.show(m_config4BtnSaveFavMode);
	m_btnMgr.show(m_config4LblHome);
	m_btnMgr.show(m_config4BtnHome);
	m_btnMgr.show(m_config4LblReturnTo);
	m_btnMgr.show(m_config4LblReturnToVal);
	m_btnMgr.show(m_config4BtnReturnToM);
	m_btnMgr.show(m_config4BtnReturnToP);

	for(u32 i = 0; i < ARRAY_SIZE(m_config4LblUser); ++i)
		if(m_config4LblUser[i] != -1)
			m_btnMgr.show(m_config4LblUser[i]);

	int i;
	i = min(max(0, m_cfg.getInt("GENERAL", "exit_to", 0)), (int)ARRAY_SIZE(CMenu::_exitTo) - 1);
	m_btnMgr.setText(m_config4BtnHome, _t(CMenu::_exitTo[i].id, CMenu::_exitTo[i].text));
	m_btnMgr.setText(m_config4BtnSaveFavMode, m_cfg.getBool("GENERAL", "save_favorites_mode") ? _t("on", L"On") : _t("off", L"Off"));

	Config titles, custom_titles;
	titles.load(fmt("%s/" TITLES_FILENAME, m_settingsDir.c_str()));
	custom_titles.load(fmt("%s/" CTITLES_FILENAME, m_settingsDir.c_str()));

	wstringEx channelName = m_loc.getWString(m_curLanguage, "disabled", L"Disabled");

	ChannelHandle.Init(m_loc.getString(m_curLanguage, "gametdb_code", "EN"));
	amountOfChannels = ChannelHandle.Count();

	const string &currentChanId = m_cfg.getString("GENERAL", "returnto");
	if(!currentChanId.empty())
	{
		for(int i = 0; i < amountOfChannels; i++)
		{
			if(strncmp(currentChanId.c_str(), ChannelHandle.GetId(i), 4) == 0)
			{
				channelName = custom_titles.getWString("TITLES", currentChanId, titles.getWString("TITLES", currentChanId, ChannelHandle.GetName(i)));
				break;
			}
		}
	}
	m_btnMgr.setText(m_config4LblReturnToVal, channelName);
}

int CMenu::_config4(void)
{
	int change = CONFIG_PAGE_NO_CHANGE;

	_showConfig4();
	while(!m_exit)
	{
		change = _configCommon();
		if (change != CONFIG_PAGE_NO_CHANGE)
			break;
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_config4BtnHome))
			{
				int exit_to = (int)loopNum((u32)m_cfg.getInt("GENERAL", "exit_to", 0) + 1, ARRAY_SIZE(CMenu::_exitTo));
				m_cfg.setInt("GENERAL", "exit_to", exit_to);
				Sys_ExitTo(exit_to);
				_showConfig4();
			}
			else if (m_btnMgr.selected(m_config4BtnSaveFavMode))
			{
				m_cfg.setBool("GENERAL", "save_favorites_mode", !m_cfg.getBool("GENERAL", "save_favorites_mode"));
				_showConfig4();
			}
			else if (m_btnMgr.selected(m_config4BtnPathManager))
			{
				_hideConfig4();
				_Paths();
				_showConfig4();
				break;
			}
			else if (m_btnMgr.selected(m_config4BtnReturnToP))
			{
				currentChannelIndex = (currentChannelIndex >= amountOfChannels - 1) ? -1 : currentChannelIndex + 1;
				if (currentChannelIndex == -1)
					m_cfg.remove("GENERAL", "returnto");
				else
					m_cfg.setString("GENERAL", "returnto", ChannelHandle.GetId(currentChannelIndex));
				_showConfig4();
			}
			else if (m_btnMgr.selected(m_config4BtnReturnToM))
			{
				if (currentChannelIndex == -1) currentChannelIndex = amountOfChannels;
				currentChannelIndex--;
				if (currentChannelIndex == -1)
					m_cfg.remove("GENERAL", "returnto");
				else
					m_cfg.setString("GENERAL", "returnto", ChannelHandle.GetId(currentChannelIndex));
				_showConfig4();
			}
		}
	}
	_hideConfig4();
	return change;
}

void CMenu::_initConfig4Menu()
{
	_addUserLabels(m_config4LblUser, ARRAY_SIZE(m_config4LblUser), "CONFIG4");
	m_config4Bg = _texture("CONFIG4/BG", "texture", theme.bg, false);
	
	m_config4LblHome = _addLabel("CONFIG4/WIIMENU", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnHome = _addButton("CONFIG4/WIIMENU_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);
	m_config4LblSaveFavMode = _addLabel("CONFIG4/SAVE_FAVMODE", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnSaveFavMode = _addButton("CONFIG4/SAVE_FAVMODE_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	m_config4LblPathManager = _addLabel("CONFIG4/PATH_MANAGER", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnPathManager = _addButton("CONFIG4/PATH_MANAGER_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	m_config4LblReturnTo = _addLabel("CONFIG4/RETURN_TO", theme.lblFont, L"", 20, 305, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4LblReturnToVal = _addLabel("CONFIG4/RETURN_TO_BTN", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config4BtnReturnToM = _addPicButton("CONFIG4/RETURN_TO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_config4BtnReturnToP = _addPicButton("CONFIG4/RETURN_TO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);
//
	_setHideAnim(m_config4LblPathManager, "CONFIG4/PATH_MANAGER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnPathManager, "CONFIG4/PATH_MANAGER_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config4LblSaveFavMode, "CONFIG4/SAVE_FAVMODE", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnSaveFavMode, "CONFIG4/SAVE_FAVMODE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config4LblHome, "CONFIG4/WIIMENU", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnHome, "CONFIG4/WIIMENU_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config4LblReturnTo, "CONFIG4/RETURN_TO", 50, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblReturnToVal, "CONFIG4/RETURN_TO_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config4BtnReturnToM, "CONFIG4/RETURN_TO_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_config4BtnReturnToP, "CONFIG4/RETURN_TO_PLUS", -50, 0, 1.f, 0.f);
	_hideConfig4(true);
	_textConfig4();
}

void CMenu::_textConfig4(void)
{
	m_btnMgr.setText(m_config4LblHome, _t("cfgc1", L"Exit To"));
	m_btnMgr.setText(m_config4LblSaveFavMode, _t("cfgd5", L"Save favorite mode state"));
	m_btnMgr.setText(m_config4LblPathManager, _t("cfgd4", L"Path Manager"));
	m_btnMgr.setText(m_config4BtnPathManager, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_config4LblReturnTo, _t("cfgg21", L"Return To Channel"));
}
