
#include "menu.hpp"
#include "sys.h"


using namespace std;

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

const int CMenu::_nbCfgPages = 6;
static const int g_curPage = 1;

void CMenu::_hideConfig(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPage, instant);
	m_btnMgr.hide(m_configBtnPageM, instant);
	m_btnMgr.hide(m_configBtnPageP, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_configLblPartitionName, instant);
	m_btnMgr.hide(m_configLblPartition, instant);
	m_btnMgr.hide(m_configBtnPartitionP, instant);
	m_btnMgr.hide(m_configBtnPartitionM, instant);
	m_btnMgr.hide(m_configLblDownload, instant);
	m_btnMgr.hide(m_configBtnDownload, instant);
	m_btnMgr.hide(m_configLblParental, instant);
	m_btnMgr.hide(m_configBtnUnlock, instant);
	m_btnMgr.hide(m_configBtnSetCode, instant);
	m_btnMgr.hide(m_configBtnEmulation, instant);
	m_btnMgr.hide(m_configLblEmulation, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if (m_configLblUser[i] != -1u)
			m_btnMgr.hide(m_configLblUser[i], instant);
}

void CMenu::_showConfig(void)
{
	_setBg(m_configBg, m_configBg);
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.show(m_configLblPartitionName);
	m_btnMgr.show(m_configLblPartition);
	m_btnMgr.show(m_configBtnPartitionP);
	m_btnMgr.show(m_configBtnPartitionM);
	m_btnMgr.show(m_configLblDownload);
	m_btnMgr.show(m_configBtnDownload);
	m_btnMgr.show(m_configLblParental);
	m_btnMgr.show(m_configLblPage);
	m_btnMgr.show(m_configBtnPageM);
	m_btnMgr.show(m_configBtnPageP);

	if(m_current_view != COVERFLOW_HOMEBREW)
	{
		m_btnMgr.show(m_configBtnEmulation);
		m_btnMgr.show(m_configLblEmulation);
	}

	m_btnMgr.show(m_locked ? m_configBtnUnlock : m_configBtnSetCode);

	bool disable = m_current_view == COVERFLOW_CHANNEL && m_cfg.getBool("NAND", "disable", true);
	char *partitionname = disable ? (char *)"NAND" : (char *)DeviceName[m_cfg.getInt(_domainFromView(), "partition", -1)];

	for(u8 i = 0; strncmp((const char *)&partitionname[i], "\0", 1) != 0; i++)
		partitionname[i] = toupper(partitionname[i]);


	for (u32 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if (m_configLblUser[i] != -1u)
			m_btnMgr.show(m_configLblUser[i]);

	m_btnMgr.setText(m_configLblPartition, (string)partitionname);
	m_btnMgr.setText(m_configLblPage, wfmt(L"%i / %i", g_curPage, m_locked ? g_curPage + 1 : CMenu::_nbCfgPages));

	bool isON = false;
	switch(m_current_view)
	{
		case COVERFLOW_CHANNEL:
			m_btnMgr.setText(m_configLblEmulation, _t("cfg12", L"NAND Emulation"));
			isON = m_cfg.getBool("NAND", "disable", true);
			break;
		case COVERFLOW_USB:
			m_btnMgr.setText(m_configLblEmulation, _t("cfg11", L"USB Saves Emulation"));
			isON = !m_cfg.getBool("GAMES", "save_emulation", false);
			break;
	}
	m_btnMgr.setText(m_configBtnEmulation, isON ?  _t("off", L"Off") : _t("on", L"On"));
}

void CMenu::_config(int page)
{
	m_curGameId = m_cf.getId();
	m_cf.clear();
	while (page > 0 && page <= CMenu::_nbCfgPages)
		switch (page)
		{
			case 1:
				page = _config1();
				break;
			case 2:
				page = _configAdv();
				break;
			case 3:
				page = _config3();
				break;
			case 4:
				page = _config4();
				break;
			case 5:
				page = _configSnd();
				break;
			case 6:
				page = _configScreen();
				break;
		}
	m_cfg.save();
	m_cf.setBoxMode(m_cfg.getBool("GENERAL", "box_mode"));
	_initCF();
}

int CMenu::_config1(void)
{
	int nextPage = 0;
	SetupInput();

	s32 bCurrentPartition = currentPartition;

	gprintf("Current Partition: %d\n", currentPartition);
	
	_showConfig();
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
			nextPage = g_curPage == 1 && !m_locked ? CMenu::_nbCfgPages : max(1, m_locked ? 1 : g_curPage - 1);
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_configBtnPageM);
			break;
		}
		if (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnPageP)))
		{
			nextPage = (g_curPage == CMenu::_nbCfgPages) ? 1 : min(g_curPage + 1, CMenu::_nbCfgPages);
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_configBtnPageP);
			break;
		}
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_configBtnBack))
				break;
			else if (m_btnMgr.selected(m_configBtnDownload))
			{
				m_cf.stopCoverLoader(true);
				_hideConfig();
				_download();
				_showConfig();
				m_cf.startCoverLoader();
			}
			else if (m_btnMgr.selected(m_configBtnUnlock))
			{
				char code[4];
				_hideConfig();
				if (_code(code) && memcmp(code, m_cfg.getString("GENERAL", "parent_code", "").c_str(), 4) == 0)
					m_locked = false;
				_showConfig();
			}
			else if (m_btnMgr.selected(m_configBtnSetCode))
			{
				char code[4];
				_hideConfig();
				if (_code(code, true))
				{
					m_cfg.setString("GENERAL", "parent_code", string(code, 4).c_str());
					m_locked = true;
				}
				_showConfig();
			}
			else if (!m_locked && (m_btnMgr.selected(m_configBtnPartitionP) || m_btnMgr.selected(m_configBtnPartitionM)))
			{
				bool disable = m_current_view == COVERFLOW_CHANNEL && m_cfg.getBool("NAND", "disable", true);
				if(!disable)
				{
					bool isD2Xv7 = IOS_GetRevision() % 100 == 7;
					u8 limiter = 0;
					s8 direction = m_btnMgr.selected(m_configBtnPartitionP) ? 1 : -1;
					currentPartition = loopNum(currentPartition + direction, (int)USB8);
					while(!DeviceHandler::Instance()->IsInserted(currentPartition) ||
						(m_current_view == COVERFLOW_CHANNEL && (DeviceHandler::Instance()->GetFSType(currentPartition) != PART_FS_FAT ||
							(!isD2Xv7 && DeviceHandler::Instance()->PathToDriveType(m_appDir.c_str()) == currentPartition) ||
							(!isD2Xv7 && DeviceHandler::Instance()->PathToDriveType(m_dataDir.c_str()) == currentPartition))) ||
						(m_current_view == COVERFLOW_HOMEBREW && DeviceHandler::Instance()->GetFSType(currentPartition) == PART_FS_WBFS))
					{
						currentPartition = loopNum(currentPartition + direction, (int)USB8);
						if (limiter > 10) break;
						limiter++;
					}

					gprintf("Next item: %s\n", DeviceName[currentPartition]);
					m_cfg.setInt(_domainFromView(), "partition", currentPartition);
				}
					_showConfig();
			}
			else if (!m_locked && m_btnMgr.selected(m_configBtnEmulation))
			{
				m_cfg.setBool("GAMES", "save_emulation", !m_cfg.getBool("GAMES", "save_emulation", false));
				_showConfig();
			}
		}
	}
	if (currentPartition != bCurrentPartition)
	{
		bool disable = m_current_view == COVERFLOW_CHANNEL && m_cfg.getBool("NAND", "disable", true);
		if(!disable)
		{
			char *newpartition = disable ? (char *)"NAND" : (char *)DeviceName[m_cfg.getInt(_domainFromView(), "partition", currentPartition)];
				
			for(u8 i = 0; strncmp((const char *)&newpartition[i], "\0", 1) != 0; i++)
				newpartition[i] = toupper(newpartition[i]);

			gprintf("Switching partition to %s\n", newpartition);
			_showWaitMessage();
			_loadList();
			_hideWaitMessage();
		}
	}

	_hideConfig();
	
	return nextPage;
}

void CMenu::_initConfigMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_configLblUser, ARRAY_SIZE(m_configLblUser), "CONFIG");
	m_configBg = _texture(theme.texSet, "CONFIG/BG", "texture", theme.bg);
	m_configLblTitle = _addLabel(theme, "CONFIG/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_configLblDownload = _addLabel(theme, "CONFIG/DOWNLOAD", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnDownload = _addButton(theme, "CONFIG/DOWNLOAD_BTN", theme.btnFont, L"", 400, 130, 200, 56, theme.btnFontColor);
	m_configLblParental = _addLabel(theme, "CONFIG/PARENTAL", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnUnlock = _addButton(theme, "CONFIG/UNLOCK_BTN", theme.btnFont, L"", 400, 190, 200, 56, theme.btnFontColor);
	m_configBtnSetCode = _addButton(theme, "CONFIG/SETCODE_BTN", theme.btnFont, L"", 400, 190, 200, 56, theme.btnFontColor);
	m_configLblPartitionName = _addLabel(theme, "CONFIG/PARTITION", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configLblPartition = _addLabel(theme, "CONFIG/PARTITION_BTN", theme.btnFont, L"", 456, 250, 88, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPartitionM = _addPicButton(theme, "CONFIG/PARTITION_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 400, 250, 56, 56);
	m_configBtnPartitionP = _addPicButton(theme, "CONFIG/PARTITION_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);
	m_configLblEmulation = _addLabel(theme, "CONFIG/EMU_SAVE", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configBtnEmulation = _addButton(theme, "CONFIG/EMU_SAVE_BTN", theme.btnFont, L"", 400, 310, 200, 56, theme.btnFontColor);
	m_configLblPage = _addLabel(theme, "CONFIG/PAGE_BTN", theme.btnFont, L"", 76, 410, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configBtnPageM = _addPicButton(theme, "CONFIG/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 410, 56, 56);
	m_configBtnPageP = _addPicButton(theme, "CONFIG/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 410, 56, 56);
	m_configBtnBack = _addButton(theme, "CONFIG/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);
	// 
	_setHideAnim(m_configLblTitle, "CONFIG/TITLE", 0, 0, -2.f, 0.f);

	_setHideAnim(m_configLblDownload, "CONFIG/DOWNLOAD", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnDownload, "CONFIG/DOWNLOAD_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configLblParental, "CONFIG/PARENTAL", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnUnlock, "CONFIG/UNLOCK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnSetCode, "CONFIG/SETCODE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_configLblPartitionName, "CONFIG/PARTITION", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configLblPartition, "CONFIG/PARTITION_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPartitionM, "CONFIG/PARTITION_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configBtnPartitionP, "CONFIG/PARTITION_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configLblEmulation, "CONFIG/EMU_SAVE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configBtnEmulation, "CONFIG/EMU_SAVE_BTN", 0, 0, -2.f, 0.f);
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
}
