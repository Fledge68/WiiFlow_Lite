
#include "menu.hpp"
#include "channel/nand.hpp"
#include "loader/nk.h"
#include "loader/sys.h"

const int CMenu::_nbCfgPages = 6;
static const int g_curPage = 1;

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

	m_btnMgr.hide(m_configLblPartitionName, instant);
	m_btnMgr.hide(m_configLblPartition, instant);
	m_btnMgr.hide(m_configBtnPartitionP, instant);
	m_btnMgr.hide(m_configBtnPartitionM, instant);
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

void CMenu::_showConfigCommon(const STexture & bg, int page)
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
	_showConfigCommon(m_configBg, g_curPage);

	if (!m_locked)
	{
		m_btnMgr.show(m_configLblPartitionName);
		m_btnMgr.show(m_configLblPartition);
		m_btnMgr.show(m_configBtnPartitionP);
		m_btnMgr.show(m_configBtnPartitionM);
		m_btnMgr.show(m_configLblDownload);
		m_btnMgr.show(m_configBtnDownload);
	
		bool disable = (m_cfg.getBool("NAND", "disable", true) || neek2o()) && m_current_view == COVERFLOW_CHANNEL && !m_tempView;
		char *partitionname = disable ? (char *)"NAND" : (char *)DeviceName[m_tempView ? m_cfg.getInt("GAMES", "savepartition", 0) : m_cfg.getInt(_domainFromView(), "partition", 0)];

		for(u8 i = 0; strncmp((const char *)&partitionname[i], "\0", 1) != 0; i++)
			partitionname[i] = toupper(partitionname[i]);

		for(u8 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
			if(m_configLblUser[i] != -1)
				m_btnMgr.show(m_configLblUser[i]);
		
		m_btnMgr.setText(m_configLblPartition, (string)partitionname);

		m_btnMgr.show(m_configLblCfg4);
		m_btnMgr.show(m_configBtnCfg4);
	}
	m_btnMgr.show(m_configLblParental);
	m_btnMgr.show(m_locked ? m_configBtnUnlock : m_configBtnSetCode);
}

void CMenu::_cfNeedsUpdate(void)
{
	if (!m_cfNeedsUpdate)
		m_cf.clear();
	m_cfNeedsUpdate = true;
}

void CMenu::_config(int page)
{
	m_curGameId = m_cf.getId();
	m_cfNeedsUpdate = false;
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
		}
		if(change == CONFIG_PAGE_BACK)
			break;
		if(!m_locked)
		{
			// assumes change is in the range of CONFIG_PAGE_DEC to CONFIG_PAGE_INC
			page += change;
			if (page > _nbCfgPages)
				page = 1;
			else if (page < 0)
				page = _nbCfgPages;
		}
	}
	if(m_cfNeedsUpdate)
	{
		m_cfg.save();
		_initCF();
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

	s32 bCurrentPartition = currentPartition;

	gprintf("Current Partition: %d\n", currentPartition);
	
	_showConfig();
	_textConfig();

	while(!m_exit)
	{
		change = _configCommon();
		if (change != CONFIG_PAGE_NO_CHANGE)
			break;

		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			_enableNandEmu(false);
			break;
		}
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_configBtnDownload))
			{
				_cfNeedsUpdate();
				m_cf.stopCoverLoader(true);
				_hideConfig();
				_download();
				_showConfig();
				m_cf.startCoverLoader();
			}
			else if ((m_btnMgr.selected(m_configBtnUnlock)) || (m_btnMgr.selected(m_configBtnSetCode)))
				_code();
			else if ((m_btnMgr.selected(m_configBtnPartitionP) || m_btnMgr.selected(m_configBtnPartitionM)))
			{
				_enableNandEmu(true);
				_showConfig();
			}
			else if (m_btnMgr.selected(m_configBtnCfg4))
			{
				_cfNeedsUpdate();
				m_cf.stopCoverLoader(true);
				_hideConfig();
				if(m_current_view != COVERFLOW_EMU)
					_NandEmuCfg();
				else
					_PluginSettings();
				_showConfig();
				m_cf.startCoverLoader();
			}
		}
	}
	
	if (currentPartition != bCurrentPartition)
	{	
		bool disable = (m_cfg.getBool("NAND", "disable", true) || neek2o()) && m_current_view == COVERFLOW_CHANNEL && !m_tempView;

		if(!disable)
		{
			char *newpartition = (char *)DeviceName[m_cfg.getInt(m_tempView ? "GAMES" : _domainFromView(), m_tempView ? "savepartition" : "partition", currentPartition)];

			for(u8 i = 0; strncmp((const char *)&newpartition[i], "\0", 1) != 0; i++)
				newpartition[i] = toupper(newpartition[i]);

			gprintf("Switching partition to %s\n", newpartition);
			_showWaitMessage();
			_loadList();
			_hideWaitMessage();
		}
	}

	_hideConfig();
	
	return change;
}

void CMenu::_initConfigMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_configLblUser, ARRAY_SIZE(m_configLblUser), "CONFIG");
	m_configBg = _texture(theme.texSet, "CONFIG/BG", "texture", theme.bg);
	m_configLblTitle = _addTitle(theme, "CONFIG/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_configLblDownload = _addLabel(theme, "CONFIG/DOWNLOAD", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnDownload = _addButton(theme, "CONFIG/DOWNLOAD_BTN", theme.btnFont, L"", 400, 130, 200, 56, theme.btnFontColor);
	m_configLblParental = _addLabel(theme, "CONFIG/PARENTAL", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnUnlock = _addButton(theme, "CONFIG/UNLOCK_BTN", theme.btnFont, L"", 400, 190, 200, 56, theme.btnFontColor);
	m_configBtnSetCode = _addButton(theme, "CONFIG/SETCODE_BTN", theme.btnFont, L"", 400, 190, 200, 56, theme.btnFontColor);
	m_configLblPartitionName = _addLabel(theme, "CONFIG/PARTITION", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configLblPartition = _addLabel(theme, "CONFIG/PARTITION_BTN", theme.btnFont, L"", 456, 250, 88, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPartitionM = _addPicButton(theme, "CONFIG/PARTITION_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 400, 250, 56, 56);
	m_configBtnPartitionP = _addPicButton(theme, "CONFIG/PARTITION_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);
	m_configLblCfg4 = _addLabel(theme, "CONFIG/CFG4", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnCfg4 = _addButton(theme, "CONFIG/CFG4_BTN", theme.btnFont, L"", 400, 310, 200, 56, theme.btnFontColor);
	m_configLblPage = _addLabel(theme, "CONFIG/PAGE_BTN", theme.btnFont, L"", 76, 400, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPageM = _addPicButton(theme, "CONFIG/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_configBtnPageP = _addPicButton(theme, "CONFIG/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 400, 56, 56);
	m_configBtnBack = _addButton(theme, "CONFIG/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);

	_setHideAnim(m_configLblTitle, "CONFIG/TITLE", 0, 0, -2.f, 0.f);

	_setHideAnim(m_configLblDownload, "CONFIG/DOWNLOAD", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnDownload, "CONFIG/DOWNLOAD_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configLblParental, "CONFIG/PARENTAL", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnUnlock, "CONFIG/UNLOCK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnSetCode, "CONFIG/SETCODE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configLblPartitionName, "CONFIG/PARTITION", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configLblPartition, "CONFIG/PARTITION_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPartitionM, "CONFIG/PARTITION_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPartitionP, "CONFIG/PARTITION_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configLblCfg4, "CONFIG/CFG4", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnCfg4, "CONFIG/CFG4_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnBack, "CONFIG/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configLblPage, "CONFIG/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageM, "CONFIG/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPageP, "CONFIG/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_hideConfig(true);
	_textConfig();
}

void CMenu::_textConfig(void)
{
	m_btnMgr.setText(m_configLblTitle, _t("cfg1", L"Settings"));
	m_btnMgr.setText(m_configLblDownload, _t("cfg3", L"Download covers & titles"));
	m_btnMgr.setText(m_configBtnDownload, _t("cfg4", L"Download"));
	m_btnMgr.setText(m_configLblParental, _t("cfg5", L"Parental control"));
	m_btnMgr.setText(m_configBtnUnlock, _t("cfg6", L"Unlock"));
	m_btnMgr.setText(m_configBtnSetCode, _t("cfg7", L"Set code"));
	m_btnMgr.setText(m_configLblPartitionName, _t("cfgp1", L"Game Partition"));
	m_btnMgr.setText(m_configBtnBack, _t("cfg10", L"Back"));
	if(m_current_view != COVERFLOW_EMU)
	{
		m_btnMgr.setText(m_configLblCfg4, _t("cfg13", L"NAND Emulation Settings"));
		m_btnMgr.setText(m_configBtnCfg4, _t("cfg14", L"Set"));
	}
	else
	{
		m_btnMgr.setText(m_configLblCfg4, _t("cfg15", L"Plugins"));
		m_btnMgr.setText(m_configBtnCfg4, _t("cfg16", L"Select"));
	}
}
