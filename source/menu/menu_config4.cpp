
#include "menu.hpp"
#include "loader/sys.h"
#include "channels.h"
#include "gecko.h"
#include "defines.h"
#include "nand.hpp"

using namespace std;

static const int g_curPage = 4;

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

int currentChannelIndex = -1;
int amountOfChannels = -1;

const CMenu::SOption CMenu::_exitTo[5] = {
	{ "menu", L"Menu" },
	{ "hbc", L"HBC" },
	{ "prii", L"Prii" },
	{ "disabled", L"Disabled" },
	{ "bootmii", L"BootMii" }
};

void CMenu::_hideConfig4(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	// 
	m_btnMgr.hide(m_config4LblHome, instant);
	m_btnMgr.hide(m_config4BtnHome, instant);
	m_btnMgr.hide(m_config4LblSaveFavMode, instant);
	m_btnMgr.hide(m_config4BtnSaveFavMode, instant);
	m_btnMgr.hide(m_config4LblCategoryOnBoot, instant);
	m_btnMgr.hide(m_config4BtnCategoryOnBoot, instant);
	m_btnMgr.hide(m_config4LblReturnTo, instant);
	m_btnMgr.hide(m_config4LblReturnToVal, instant);
	m_btnMgr.hide(m_config4BtnReturnToM, instant);
	m_btnMgr.hide(m_config4BtnReturnToP, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_config4LblUser); ++i)
		if (m_config4LblUser[i] != -1u)
			m_btnMgr.hide(m_config4LblUser[i], instant);
}

void CMenu::_showConfig4(void)
{
	_setBg(m_config4Bg, m_config4Bg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	// 
	m_btnMgr.show(m_config4LblHome);
	m_btnMgr.show(m_config4BtnHome);
	m_btnMgr.show(m_config4LblSaveFavMode);
	m_btnMgr.show(m_config4BtnSaveFavMode);
	m_btnMgr.show(m_config4LblCategoryOnBoot);
	m_btnMgr.show(m_config4BtnCategoryOnBoot);
	m_btnMgr.show(m_config4LblReturnTo);
	m_btnMgr.show(m_config4LblReturnToVal);
	m_btnMgr.show(m_config4BtnReturnToM);
	m_btnMgr.show(m_config4BtnReturnToP);

	for (u32 i = 0; i < ARRAY_SIZE(m_config4LblUser); ++i)
		if (m_config4LblUser[i] != -1u)
			m_btnMgr.show(m_config4LblUser[i]);
 
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage : CMenu::_nbCfgPages));
	int i;
	i = min(max(0, m_cfg.getInt("GENERAL", "exit_to", 0)), (int)ARRAY_SIZE(CMenu::_exitTo) - 1);
	m_btnMgr.setText(m_config4BtnHome, _t(CMenu::_exitTo[i].id, CMenu::_exitTo[i].text));
	m_btnMgr.setText(m_config4BtnSaveFavMode, m_cfg.getBool("GENERAL", "favorites_on_startup") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_config4BtnCategoryOnBoot, m_cat.getBool("GENERAL", "category_on_start") ? _t("on", L"On") : _t("off", L"Off"));

	Config titles, custom_titles;
	titles.load(sfmt("%s/" TITLES_FILENAME, m_settingsDir.c_str()).c_str());
	custom_titles.load(sfmt("%s/" CTITLES_FILENAME, m_settingsDir.c_str()).c_str());

	wstringEx channelName = m_loc.getWString(m_curLanguage, "disabled", L"Disabled");

	string langCode = m_loc.getString(m_curLanguage, "gametdb_code", "EN");

	Nand::Instance()->Disable_Emu();

	m_channels.Init(0x00010001, langCode, true);
	amountOfChannels = m_channels.Count();

	string currentChanId = m_cfg.getString("GENERAL", "returnto" );
	if (currentChanId.size() > 0)
	{
		for (int i = 0; i < amountOfChannels; i++)
		{
			if (currentChanId == m_channels.GetId(i))
			{
				channelName = custom_titles.getWString("TITLES", currentChanId, titles.getWString("TITLES", currentChanId, m_channels.GetName(i)));
				break;
			}
		}
	}

	Nand::Instance()->Enable_Emu();

	m_btnMgr.setText(m_config4LblReturnToVal, channelName);
}

int CMenu::_config4(void)
{
	int nextPage = 0;

	_showConfig4();
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
			else if (m_btnMgr.selected(m_config4BtnHome))
			{
				int exit_to = (int)loopNum((u32)m_cfg.getInt("GENERAL", "exit_to", 0) + 1, ARRAY_SIZE(CMenu::_exitTo));
				m_cfg.setInt("GENERAL", "exit_to", exit_to);
				Sys_ExitTo(exit_to);
				m_disable_exit = exit_to == 3;
				_showConfig4();
			}
			else if (m_btnMgr.selected(m_config4BtnSaveFavMode))
			{
				m_cfg.setBool("GENERAL", "favorites_on_startup", !m_cfg.getBool("GENERAL", "favorites_on_startup"));
				_showConfig4();
			}
			else if (m_btnMgr.selected(m_config4BtnCategoryOnBoot))
			{
				m_cat.setBool("GENERAL", "category_on_start", !m_cat.getBool("GENERAL", "category_on_start", false));
				_showConfig4();
			}
			else if (m_btnMgr.selected(m_config4BtnReturnToP))
			{
				currentChannelIndex = (currentChannelIndex >= amountOfChannels - 1) ? -1 : currentChannelIndex + 1;
				if (currentChannelIndex == -1)
					m_cfg.remove("GENERAL", "returnto");
				else
					m_cfg.setString("GENERAL", "returnto", m_channels.GetId(currentChannelIndex));
				_showConfig4();
			}
			else if (m_btnMgr.selected(m_config4BtnReturnToM))
			{
				if (currentChannelIndex == -1) currentChannelIndex = amountOfChannels;
				currentChannelIndex--;
				if (currentChannelIndex == -1)
					m_cfg.remove("GENERAL", "returnto");
				else
					m_cfg.setString("GENERAL", "returnto", m_channels.GetId(currentChannelIndex));
				_showConfig4();
			}
		}
	}
	_hideConfig4();
	return nextPage;
}

void CMenu::_initConfig4Menu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_config4LblUser, ARRAY_SIZE(m_config4LblUser), "CONFIG4");
	m_config4Bg = _texture(theme.texSet, "CONFIG4/BG", "texture", theme.bg);
	m_config4LblHome = _addLabel(theme, "CONFIG4/WIIMENU", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnHome = _addButton(theme, "CONFIG4/WIIMENU_BTN", theme.btnFont, L"", 400, 130, 200, 56, theme.btnFontColor);
	m_config4LblSaveFavMode = _addLabel(theme, "CONFIG4/SAVE_FAVMODE", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnSaveFavMode = _addButton(theme, "CONFIG4/SAVE_FAVMODE_BTN", theme.btnFont, L"", 400, 190, 200, 56, theme.btnFontColor);
	m_config4LblCategoryOnBoot = _addLabel(theme, "CONFIG4/CAT_ON_START", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnCategoryOnBoot = _addButton(theme, "CONFIG4/CAT_ON_START_BTN", theme.btnFont, L"", 400, 250, 200, 56, theme.btnFontColor);
	m_config4LblReturnTo = _addLabel(theme, "CONFIG4/RETURN_TO", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4LblReturnToVal = _addLabel(theme, "CONFIG4/RETURN_TO_BTN", theme.btnFont, L"", 426, 310, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config4BtnReturnToM = _addPicButton(theme, "CONFIG4/RETURN_TO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 310, 56, 56);
	m_config4BtnReturnToP = _addPicButton(theme, "CONFIG4/RETURN_TO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
//
	_setHideAnim(m_config4LblHome, "CONFIG4/WIIMENU", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnHome, "CONFIG4/WIIMENU_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblSaveFavMode, "CONFIG4/SAVE_FAVMODE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnSaveFavMode, "CONFIG4/SAVE_FAVMODE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblCategoryOnBoot, "CONFIG4/CAT_ON_START", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnCategoryOnBoot, "CONFIG4/CAT_ON_START_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblReturnTo, "CONFIG4/RETURN_TO", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4LblReturnToVal, "CONFIG4/RETURN_TO_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config4BtnReturnToM, "CONFIG4/RETURN_TO_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config4BtnReturnToP, "CONFIG4/RETURN_TO_PLUS", 0, 0, 1.f, -1.f);
	_hideConfig4(true);
	_textConfig4();
}

void CMenu::_textConfig4(void)
{
	m_btnMgr.setText(m_config4LblHome, _t("cfgc1", L"Exit To"));
	m_btnMgr.setText(m_config4LblSaveFavMode, _t("cfgd5", L"Save favorite mode state"));
	m_btnMgr.setText(m_config4LblCategoryOnBoot, _t("cfgd7", L"Show categories on boot"));
	m_btnMgr.setText(m_config4LblReturnTo, _t("cfgg21", L"Return To Channel"));
}
