
#include "menu.hpp"

s16 m_cfghbLblTitle;
s16 m_cfghbBtnBack;
s16 m_cfghbLblUser[4];

s16 m_cfghbLblAdjustCF;
s16 m_cfghbBtnAdjustCF;

s16 m_cfghbLblSmallbox;
s16 m_cfghbBtnSmallbox;

s16 m_cfghbLblBoxMode;
s16 m_cfghbBtnBoxMode;

s16 m_cfghbLblPartition;
s16 m_cfghbLblPartitionVal;
s16 m_cfghbBtnPartitionP;
s16 m_cfghbBtnPartitionM;

TexData m_cfghbBg;

static void _showCfgHB(void)
{
	m_btnMgr.show(m_cfghbLblTitle);
	m_btnMgr.show(m_cfghbBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfghbLblUser); ++i)
		if(m_cfghbLblUser[i] != -1)
			m_btnMgr.show(m_cfghbLblUser[i]);

	m_btnMgr.show(m_cfghbLblAdjustCF);
	m_btnMgr.show(m_cfghbBtnAdjustCF);
	
	m_btnMgr.show(m_cfghbLblSmallbox);
	m_btnMgr.show(m_cfghbBtnSmallbox);
	
	m_btnMgr.show(m_cfghbLblBoxMode);
	m_btnMgr.show(m_cfghbBtnBoxMode);

	m_btnMgr.show(m_cfghbLblPartition);
	m_btnMgr.show(m_cfghbLblPartitionVal);
	m_btnMgr.show(m_cfghbBtnPartitionP);
	m_btnMgr.show(m_cfghbBtnPartitionM);

}

static void _hideCfgHB(bool instant)
{
	m_btnMgr.hide(m_cfghbLblTitle, instant);
	m_btnMgr.hide(m_cfghbBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfghbLblUser); ++i)
		if(m_cfghbLblUser[i] != -1)
			m_btnMgr.hide(m_cfghbLblUser[i], instant);

	m_btnMgr.hide(m_cfghbLblAdjustCF, instant);
	m_btnMgr.hide(m_cfghbBtnAdjustCF, instant);
	
	m_btnMgr.hide(m_cfghbLblSmallbox, instant);
	m_btnMgr.hide(m_cfghbBtnSmallbox, instant);
	
	m_btnMgr.hide(m_cfghbLblBoxMode, instant);
	m_btnMgr.hide(m_cfghbBtnBoxMode, instant);

	m_btnMgr.hide(m_cfghbLblPartition, instant);
	m_btnMgr.hide(m_cfghbLblPartitionVal, instant);
	m_btnMgr.hide(m_cfghbBtnPartitionP, instant);
	m_btnMgr.hide(m_cfghbBtnPartitionM, instant);
}

void CMenu::_CfgHB(void)
{
	m_btnMgr.setText(m_cfghbBtnSmallbox, m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_cfghbBtnBoxMode, m_cfg.getBool(HOMEBREW_DOMAIN, "box_mode") ? _t("on", L"On") : _t("off", L"Off"));
	
	currentPartition = m_cfg.getInt(HOMEBREW_DOMAIN, "partition", 0);
	const char *partitionname = DeviceName[currentPartition];
	m_btnMgr.setText(m_cfghbLblPartitionVal, upperCase(partitionname));
	
	SetupInput();
	_setBg(m_cfghbBg, m_cfghbBg);
	_showCfgHB();

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
			if(m_btnMgr.selected(m_cfghbBtnBack))
				break;
			else if (m_btnMgr.selected(m_cfghbBtnAdjustCF))
			{
				m_refreshGameList = true;
				_hideCfgHB(true);
				_cfTheme();
				_showCfgHB();
			}
			else if (m_btnMgr.selected(m_cfghbBtnSmallbox))
			{
				m_refreshGameList = true;
				m_cfg.setBool(HOMEBREW_DOMAIN, "smallbox", !m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", false));
				m_btnMgr.setText(m_cfghbBtnSmallbox, m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_cfghbBtnBoxMode))
			{
				m_refreshGameList = true;
				m_cfg.setBool(HOMEBREW_DOMAIN, "box_mode", !m_cfg.getBool(HOMEBREW_DOMAIN, "box_mode", false));
				m_btnMgr.setText(m_cfghbBtnBoxMode, m_cfg.getBool(HOMEBREW_DOMAIN, "box_mode") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_cfghbBtnPartitionP) || m_btnMgr.selected(m_cfghbBtnPartitionM))
			{
				m_refreshGameList = true;
				s8 direction = m_btnMgr.selected(m_cfghbBtnPartitionP) ? 1 : -1;
				_setPartition(direction);
				const char *partitionname = DeviceName[currentPartition];
				m_btnMgr.setText(m_cfghbLblPartitionVal, upperCase(partitionname));
			}
		}
	}
	_hideCfgHB(true);
}

void CMenu::_initCfgHB(void)
{
	m_cfghbBg = _texture("CFG_HB/BG", "texture", theme.bg, false);
	
	_addUserLabels(m_cfghbLblUser, ARRAY_SIZE(m_cfghbLblUser), "CFG_HB");
	m_cfghbLblTitle = _addTitle("CFG_HB/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_cfghbBtnBack = _addButton("CFG_HB/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	
	m_cfghbLblAdjustCF = _addLabel("CFG_HB/ADJUST_CF", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfghbBtnAdjustCF = _addButton("CFG_HB/ADJUST_CF_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_cfghbLblSmallbox = _addLabel("CFG_HB/HB_SMALLBOX", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfghbBtnSmallbox = _addButton("CFG_HB/HB_SMALLBOX_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_cfghbLblBoxMode = _addLabel("CFG_HB/HB_BOXMODE", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfghbBtnBoxMode = _addButton("CFG_HB/HB_BOXMODE_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

	m_cfghbLblPartition = _addLabel("CFG_HB/HB_PARTITION", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfghbLblPartitionVal = _addLabel("CFG_HB/HB_PARTITION_VAL", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_cfghbBtnPartitionM = _addPicButton("CFG_HB/HB_PARTITION_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_cfghbBtnPartitionP = _addPicButton("CFG_HB/HB_PARTITION_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);
	
	_setHideAnim(m_cfghbLblTitle, "CFG_HB/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cfghbBtnBack, "CFG_HB/BACK_BTN", 0, 0, 1.f, -1.f);
	
	_setHideAnim(m_cfghbLblAdjustCF, "CFG_HB/ADJUST_CF", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfghbBtnAdjustCF, "CFG_HB/ADJUST_CF_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_cfghbLblSmallbox, "CFG_HB/HB_SMALLBOX", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfghbBtnSmallbox, "CFG_HB/HB_SMALLBOX_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_cfghbLblBoxMode, "CFG_HB/HB_BOXMODE", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfghbBtnBoxMode, "CFG_HB/HB_BOXMODE_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_cfghbLblPartition, "CFG_HB/HB_PARTITION", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfghbLblPartitionVal, "CFG_HB/HB_PARTITION_VAL", -50, 0, 1.f, 0.f);
	_setHideAnim(m_cfghbBtnPartitionM, "CFG_HB/HB_PARTITION_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_cfghbBtnPartitionP, "CFG_HB/HB_PARTITION_PLUS", -50, 0, 1.f, 0.f);

	_hideCfgHB(true);
	_textCfgHB();
}

void CMenu::_textCfgHB(void)
{
	m_btnMgr.setText(m_cfghbLblTitle, _t("cfghb1", L"Homebrew Settings"));
	m_btnMgr.setText(m_cfghbLblAdjustCF, _t("cfgc4", L"Adjust Coverflow"));
	m_btnMgr.setText(m_cfghbBtnAdjustCF, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_cfghbLblSmallbox, _t("cfghb2", L"Coverflow Smallbox"));
	m_btnMgr.setText(m_cfghbLblBoxMode, _t("cfghb4", L"Box Mode"));
	m_btnMgr.setText(m_cfghbLblPartition, _t("cfghb3", L"Homebrew Partition"));
	m_btnMgr.setText(m_cfghbBtnBack, _t("cfg10", L"Back"));
}
