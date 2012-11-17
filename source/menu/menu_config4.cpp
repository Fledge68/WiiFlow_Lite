
#include "menu.hpp"
#include "defines.h"
#include "channel/channels.h"
#include "channel/nand.hpp"
#include "gecko/gecko.h"
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
	for(u8 i = 0; i < ARRAY_SIZE(m_config4LblUser); ++i)
		if(m_config4LblUser[i] != -1)
			m_btnMgr.hide(m_config4LblUser[i], instant);
}

void CMenu::_showConfig4(void)
{
	_showConfigCommon(m_config4Bg, g_curPage);

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

	for(u32 i = 0; i < ARRAY_SIZE(m_config4LblUser); ++i)
		if(m_config4LblUser[i] != -1)
			m_btnMgr.show(m_config4LblUser[i]);
 
	int i;
	i = min(max(0, m_cfg.getInt("GENERAL", "exit_to", 0)), (int)ARRAY_SIZE(CMenu::_exitTo) - 1);
	m_btnMgr.setText(m_config4BtnHome, _t(CMenu::_exitTo[i].id, CMenu::_exitTo[i].text));
	m_btnMgr.setText(m_config4BtnSaveFavMode, m_cfg.getBool("GENERAL", "save_favorites_mode") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_config4BtnCategoryOnBoot, m_cat.getBool("GENERAL", "category_on_start") ? _t("on", L"On") : _t("off", L"Off"));

	Config titles, custom_titles;
	titles.load(fmt("%s/" TITLES_FILENAME, m_settingsDir.c_str()));
	custom_titles.load(fmt("%s/" CTITLES_FILENAME, m_settingsDir.c_str()));

	wstringEx channelName = m_loc.getWString(m_curLanguage, "disabled", L"Disabled");

	NandHandle.Disable_Emu();
	ChannelHandle.Init(m_loc.getString(m_curLanguage, "gametdb_code", "EN"));
	amountOfChannels = ChannelHandle.Count();

	string currentChanId = m_cfg.getString("GENERAL", "returnto" );
	if (currentChanId.size() > 0)
	{
		for (int i = 0; i < amountOfChannels; i++)
		{
			if (currentChanId == ChannelHandle.GetId(i))
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
	if(!neek2o() && m_current_view == COVERFLOW_CHANNEL && m_cfg.getBool(CHANNEL_DOMAIN, "disable", true)  == false)
		NandHandle.Enable_Emu();
	_hideConfig4();
	return change;
}

void CMenu::_initConfig4Menu()
{
	_addUserLabels(m_config4LblUser, ARRAY_SIZE(m_config4LblUser), "CONFIG4");
	m_config4Bg = _texture("CONFIG4/BG", "texture", theme.bg, false);
	m_config4LblHome = _addLabel("CONFIG4/WIIMENU", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnHome = _addButton("CONFIG4/WIIMENU_BTN", theme.btnFont, L"", 370, 130, 230, 56, theme.btnFontColor);
	m_config4LblSaveFavMode = _addLabel("CONFIG4/SAVE_FAVMODE", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnSaveFavMode = _addButton("CONFIG4/SAVE_FAVMODE_BTN", theme.btnFont, L"", 370, 190, 230, 56, theme.btnFontColor);
	m_config4LblCategoryOnBoot = _addLabel("CONFIG4/CAT_ON_START", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4BtnCategoryOnBoot = _addButton("CONFIG4/CAT_ON_START_BTN", theme.btnFont, L"", 370, 250, 230, 56, theme.btnFontColor);
	m_config4LblReturnTo = _addLabel("CONFIG4/RETURN_TO", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_config4LblReturnToVal = _addLabel("CONFIG4/RETURN_TO_BTN", theme.btnFont, L"", 426, 310, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_config4BtnReturnToM = _addPicButton("CONFIG4/RETURN_TO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 310, 56, 56);
	m_config4BtnReturnToP = _addPicButton("CONFIG4/RETURN_TO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
//
	_setHideAnim(m_config4LblHome, "CONFIG4/WIIMENU", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnHome, "CONFIG4/WIIMENU_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config4LblSaveFavMode, "CONFIG4/SAVE_FAVMODE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnSaveFavMode, "CONFIG4/SAVE_FAVMODE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_config4LblCategoryOnBoot, "CONFIG4/CAT_ON_START", 100, 0, -2.f, 0.f);
	_setHideAnim(m_config4BtnCategoryOnBoot, "CONFIG4/CAT_ON_START_BTN", 0, 0, 1.f, -1.f);
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
