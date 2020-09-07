
#include "menu.hpp"
#include "channel/nand.hpp"
#include "loader/nk.h"

const int CMenu::_nbCfgPages = 14;

void CMenu::_hideConfigCommon(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
}

void CMenu::_hideConfig(bool instant)
{
	_hideConfigCommon(instant);

	m_btnMgr.hide(m_configLblPartition, instant);
	m_btnMgr.hide(m_configBtnPartition, instant);
	m_btnMgr.hide(m_configLblDownload, instant);
	m_btnMgr.hide(m_configBtnDownload, instant);
	m_btnMgr.hide(m_configLblParental, instant);
	m_btnMgr.hide(m_configBtnUnlock, instant);
	m_btnMgr.hide(m_configBtnSetCode, instant);
	m_btnMgr.hide(m_configLblCfg4, instant);
	m_btnMgr.hide(m_configBtnCfg4, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if(m_configLblUser[i] != -1)
			m_btnMgr.hide(m_configLblUser[i], instant);
}

void CMenu::_showConfigCommon(const TexData &bg, int page)
{
	_setBg(bg, bg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", page, m_locked ? page : _nbCfgPages));
}

void CMenu::_showConfig(void)
{
	_showConfigCommon(m_configBg, 1);

	if(!m_locked)
	{
		m_btnMgr.show(m_configLblDownload);
		m_btnMgr.show(m_configBtnDownload);
		
		m_btnMgr.show(m_configLblPartition);
		m_btnMgr.show(m_configBtnPartition);
	
		m_btnMgr.show(m_configLblCfg4);
		m_btnMgr.show(m_configBtnCfg4);

		for(u8 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
			if(m_configLblUser[i] != -1)
				m_btnMgr.show(m_configLblUser[i]);
	}
	m_btnMgr.show(m_configLblParental);
	m_btnMgr.show(m_locked ? m_configBtnUnlock : m_configBtnSetCode);
}

void CMenu::_config(int page)
{
	int change = CONFIG_PAGE_NO_CHANGE;
	while(!m_exit)
	{
		switch(page)
		{
			case 1:
				change = _config1();
				break;
			case 2:
				change = _configAdv();
				break;
			case 3:
				change = _config3();
				break;
			case 4:
				change = _config4();
				break;
			case 5:
				change = _configSnd();
				break;
			case 6:
				change = _configScreen();
				break;
			default:
				change = _config7(page);
				break;
		}
		if(change == CONFIG_PAGE_BACK)
			break;
		if(!m_locked)
		{
			// assumes change is in the range of CONFIG_PAGE_DEC to CONFIG_PAGE_INC
			page += change;
			if (page > _nbCfgPages)
				page = 1;
			else if (page <= 0)
				page = _nbCfgPages;
		}
	}
}

int CMenu::_configCommon(void)
{
	_mainLoopCommon();
	if (BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnBack)))
		return CONFIG_PAGE_BACK;
	else if (BTN_UP_PRESSED)
		m_btnMgr.up();
	else if (BTN_DOWN_PRESSED)
		m_btnMgr.down();
	else if (BTN_LEFT_PRESSED || BTN_MINUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageM)))
	{
		if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_configBtnPageM);
		return CONFIG_PAGE_DEC;
	}
	else if (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageP)))
	{
		if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_configBtnPageP);
		return CONFIG_PAGE_INC;
	}
	return CONFIG_PAGE_NO_CHANGE;
}

int CMenu::_config1(void)
{
	int change = CONFIG_PAGE_NO_CHANGE;

	SetupInput();
	_showConfig();

	while(!m_exit)
	{
		change = _configCommon();
		if (change != CONFIG_PAGE_NO_CHANGE)
			break;

		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_configBtnDownload))
			{
				_hideConfig();
				_download();
				_showConfig();
			}
			else if (m_btnMgr.selected(m_configBtnUnlock))
			{
				char code[4];
				_hideConfig();
				if (_code(code) && memcmp(code, m_cfg.getString("GENERAL", "parent_code", "").c_str(), 4) == 0)
				{
					m_refreshGameList = true;
					m_locked = false;
				}
				else
					error(_t("cfgg25",L"Password incorrect."));
				_showConfig();
			}
			else if (m_btnMgr.selected(m_configBtnSetCode))
			{
				char code[4];
				_hideConfig();
				if (_code(code, true))
				{
					m_refreshGameList = true;
					m_cfg.setString("GENERAL", "parent_code", string(code, 4).c_str());
					m_locked = true;
				}
				_showConfig();
			}
			else if (m_btnMgr.selected(m_configBtnPartition))
			{
				_hideConfig();
				_partitionsCfg();
				_showConfig();
			}
			else if (m_btnMgr.selected(m_configBtnCfg4))
			{
				_hideConfig();
				_NandEmuCfg();
				_showConfig();
			}
		}
	}
	_hideConfig();	
	return change;
}

void CMenu::_initConfigMenu()
{
	_addUserLabels(m_configLblUser, ARRAY_SIZE(m_configLblUser), "CONFIG");
	m_configBg = _texture("CONFIG/BG", "texture", theme.bg, false);
	m_configLblTitle = _addLabel("CONFIG/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_configLblDownload = _addLabel("CONFIG/DOWNLOAD", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnDownload = _addButton("CONFIG/DOWNLOAD_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);
	m_configLblParental = _addLabel("CONFIG/PARENTAL", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnUnlock = _addButton("CONFIG/UNLOCK_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	m_configBtnSetCode = _addButton("CONFIG/SETCODE_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	m_configLblPartition = _addLabel("CONFIG/PARTITION", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnPartition = _addButton("CONFIG/PARTITION_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	m_configLblCfg4 = _addLabel("CONFIG/CFG4", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnCfg4 = _addButton("CONFIG/CFG4_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);
	m_configLblPage = _addLabel("CONFIG/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPageM = _addPicButton("CONFIG/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_configBtnPageP = _addPicButton("CONFIG/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_configBtnBack = _addButton("CONFIG/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	_setHideAnim(m_configLblTitle, "CONFIG/TITLE", 0, 0, -2.f, 0.f);

	_setHideAnim(m_configLblDownload, "CONFIG/DOWNLOAD", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnDownload, "CONFIG/DOWNLOAD_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLblParental, "CONFIG/PARENTAL", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnUnlock, "CONFIG/UNLOCK_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtnSetCode, "CONFIG/SETCODE_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLblPartition, "CONFIG/PARTITION", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnPartition, "CONFIG/PARTITION_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configLblCfg4, "CONFIG/CFG4", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnCfg4, "CONFIG/CFG4_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configBtnBack, "CONFIG/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configLblPage, "CONFIG/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageM, "CONFIG/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageP, "CONFIG/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_hideConfig(true);
	_textConfig();
}

void CMenu::_textConfig(void)
{
	m_btnMgr.setText(m_configLblTitle, _t("cfg1", L"Settings"));
	m_btnMgr.setText(m_configLblDownload, _t("cfg3", L"Download covers & banners"));
	m_btnMgr.setText(m_configBtnDownload, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_configLblParental, _t("cfg5", L"Parental control"));
	m_btnMgr.setText(m_configBtnUnlock, _t("cfg6", L"Unlock"));
	m_btnMgr.setText(m_configBtnSetCode, _t("cfg7", L"Set code"));
	m_btnMgr.setText(m_configLblPartition, _t("cfg17", L"Game Partitions"));
	m_btnMgr.setText(m_configBtnPartition, _t("cfg14", L"Set"));
	m_btnMgr.setText(m_configLblCfg4, _t("cfg13", L"NAND Emulation Settings"));
	m_btnMgr.setText(m_configBtnCfg4, _t("cfg14", L"Set"));
	m_btnMgr.setText(m_configBtnBack, _t("cfg10", L"Back"));
}
