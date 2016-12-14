
#include "menu.hpp"

s16 m_cfgsrcLblTitle;
s16 m_cfgsrcBtnBack;
s16 m_cfgsrcLblUser[4];

s16 m_cfgsrcLblEnableSF;
s16 m_cfgsrcBtnEnableSF;

s16 m_cfgsrcLblSmallbox;
s16 m_cfgsrcBtnSmallbox;

s16 m_cfgsrcLblBoxMode;
s16 m_cfgsrcBtnBoxMode;

s16 m_cfgsrcLblAdjustCF;
s16 m_cfgsrcBtnAdjustCF;

s16 m_cfgsrcLblMultisource;
s16 m_cfgsrcBtnMultisource;

TexData m_cfgsrcBg;

static void _hideCfgSrc(bool instant)
{
	m_btnMgr.hide(m_cfgsrcLblTitle, instant);
	m_btnMgr.hide(m_cfgsrcBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfgsrcLblUser); ++i)
		if(m_cfgsrcLblUser[i] != -1)
			m_btnMgr.hide(m_cfgsrcLblUser[i], instant);

	m_btnMgr.hide(m_cfgsrcLblEnableSF, instant);
	m_btnMgr.hide(m_cfgsrcBtnEnableSF, instant);
	
	m_btnMgr.hide(m_cfgsrcLblSmallbox, instant);
	m_btnMgr.hide(m_cfgsrcBtnSmallbox, instant);

	m_btnMgr.hide(m_cfgsrcLblBoxMode, instant);
	m_btnMgr.hide(m_cfgsrcBtnBoxMode, instant);

	m_btnMgr.hide(m_cfgsrcLblAdjustCF, instant);
	m_btnMgr.hide(m_cfgsrcBtnAdjustCF, instant);

	m_btnMgr.hide(m_cfgsrcLblMultisource, instant);
	m_btnMgr.hide(m_cfgsrcBtnMultisource, instant);
}

static void _showCfgSrc(bool m_sourceflow)
{
	m_btnMgr.show(m_cfgsrcLblTitle);
	m_btnMgr.show(m_cfgsrcBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfgsrcLblUser); ++i)
		if(m_cfgsrcLblUser[i] != -1)
			m_btnMgr.show(m_cfgsrcLblUser[i]);

	m_btnMgr.show(m_cfgsrcLblEnableSF);
	m_btnMgr.show(m_cfgsrcBtnEnableSF);
	
	if(m_sourceflow)
	{
		m_btnMgr.show(m_cfgsrcLblSmallbox);
		m_btnMgr.show(m_cfgsrcBtnSmallbox);

		m_btnMgr.show(m_cfgsrcLblBoxMode);
		m_btnMgr.show(m_cfgsrcBtnBoxMode);
		
		m_btnMgr.show(m_cfgsrcLblAdjustCF);
		m_btnMgr.show(m_cfgsrcBtnAdjustCF);
		
		m_btnMgr.hide(m_cfgsrcLblMultisource, true);
		m_btnMgr.hide(m_cfgsrcBtnMultisource, true);
	}
	else
	{
		m_btnMgr.show(m_cfgsrcLblMultisource);
		m_btnMgr.show(m_cfgsrcBtnMultisource);
		
		m_btnMgr.hide(m_cfgsrcLblSmallbox, true);
		m_btnMgr.hide(m_cfgsrcBtnSmallbox, true);

		m_btnMgr.hide(m_cfgsrcLblBoxMode, true);
		m_btnMgr.hide(m_cfgsrcBtnBoxMode, true);

		m_btnMgr.hide(m_cfgsrcLblAdjustCF, true);
		m_btnMgr.hide(m_cfgsrcBtnAdjustCF, true);
	}
}

void CMenu::_CfgSrc(void)
{
	SetupInput();
	_setBg(m_cfgsrcBg, m_cfgsrcBg);
	m_btnMgr.setText(m_cfgsrcBtnEnableSF, m_cfg.getBool(SOURCEFLOW_DOMAIN, "enabled") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_cfgsrcBtnSmallbox, m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_cfgsrcBtnBoxMode, m_cfg.getBool(SOURCEFLOW_DOMAIN, "box_mode") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_cfgsrcBtnMultisource, m_cfg.getBool("GENERAL", "multisource") ? _t("on", L"On") : _t("off", L"Off"));
	_showCfgSrc(m_sourceflow);

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
			if(m_btnMgr.selected(m_cfgsrcBtnBack))
				break;
			else if(m_btnMgr.selected(m_cfgsrcBtnEnableSF))
			{
				m_sourceflow = !m_sourceflow;
				m_cfg.setBool(SOURCEFLOW_DOMAIN, "enabled", m_sourceflow);
				m_btnMgr.setText(m_cfgsrcBtnEnableSF, m_sourceflow ? _t("on", L"On") : _t("off", L"Off"));
				_showCfgSrc(m_sourceflow);
			}
			else if(m_btnMgr.selected(m_cfgsrcBtnSmallbox))
			{
				m_refreshGameList = true;
				fsop_deleteFolder(fmt("%s/sourceflow", m_cacheDir.c_str()));
				m_cfg.setBool(SOURCEFLOW_DOMAIN, "smallbox", !m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox", false));
				m_btnMgr.setText(m_cfgsrcBtnSmallbox, m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if(m_btnMgr.selected(m_cfgsrcBtnBoxMode))
			{
				m_refreshGameList = true;
				fsop_deleteFolder(fmt("%s/sourceflow", m_cacheDir.c_str()));
				m_cfg.setBool(SOURCEFLOW_DOMAIN, "box_mode", !m_cfg.getBool(SOURCEFLOW_DOMAIN, "box_mode", false));
				m_btnMgr.setText(m_cfgsrcBtnBoxMode, m_cfg.getBool(SOURCEFLOW_DOMAIN, "box_mode") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if(m_btnMgr.selected(m_cfgsrcBtnAdjustCF))
			{
				m_refreshGameList = true;
				_hideCfgSrc(true);
				_cfTheme();
				_showCfgSrc(m_sourceflow);
			}
			else if(m_btnMgr.selected(m_cfgsrcBtnMultisource))
			{
				m_multisource = !m_multisource;
				m_cfg.setBool("GENERAL", "multisource", m_multisource);
				m_btnMgr.setText(m_cfgsrcBtnMultisource, m_multisource ? _t("on", L"On") : _t("off", L"Off"));
			}
		}
	}
	_hideCfgSrc(false);
}

void CMenu::_initCfgSrc(void)
{
	m_cfgsrcBg = _texture("CFG_SRC/BG", "texture", theme.bg, false);
	
	_addUserLabels(m_cfgsrcLblUser, ARRAY_SIZE(m_cfgsrcLblUser), "CFG_SRC");
	m_cfgsrcLblTitle = _addTitle("CFG_SRC/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnBack = _addButton("CFG_SRC/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	
	m_cfgsrcLblEnableSF = _addLabel("CFG_SRC/ENABLE_SF", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnEnableSF = _addButton("CFG_SRC/ENABLE_SF_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_cfgsrcLblSmallbox = _addLabel("CFG_SRC/SF_SMALLBOX", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnSmallbox = _addButton("CFG_SRC/SF_SMALLBOX_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_cfgsrcLblBoxMode = _addLabel("CFG_SRC/SF_BOXMODE", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnBoxMode = _addButton("CFG_SRC/SF_BOXMODE_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);

	m_cfgsrcLblAdjustCF = _addLabel("CFG_SRC/SF_ADJUSTCF", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnAdjustCF = _addButton("CFG_SRC/SF_ADJUSTCF_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);
	
	m_cfgsrcLblMultisource = _addLabel("CFG_SRC/MULTISOURCE", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnMultisource = _addButton("CFG_SRC/MULTISOURCE_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	
	_setHideAnim(m_cfgsrcLblTitle, "CFG_SRC/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnBack, "CFG_SRC/BACK_BTN", 0, 0, 1.f, -1.f);
	
	_setHideAnim(m_cfgsrcLblEnableSF, "CFG_SRC/ENABLE_SF", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnEnableSF, "CFG_SRC/ENABLE_SF_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_cfgsrcLblSmallbox, "CFG_SRC/SF_SMALLBOX", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnSmallbox, "CFG_SRC/SF_SMALLBOX_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_cfgsrcLblBoxMode, "CFG_SRC/SF_BOX_MODE", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnBoxMode, "CFG_SRC/SF_BOX_MODE_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_cfgsrcLblAdjustCF, "CFG_SRC/SF_ADJUSTCF", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnAdjustCF, "CFG_SRC/SF_ADJUSTCF_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_cfgsrcLblMultisource, "CFG_SRC/MULTISOURCE", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnMultisource, "CFG_SRC/MULTISOURCE_BTN", -50, 0, 1.f, 0.f);

	_hideCfgSrc(true);
	_textCfgSrc();
}

void CMenu::_textCfgSrc(void)
{
	m_btnMgr.setText(m_cfgsrcLblTitle, _t("cfgsm1", L"Source Menu Settings"));
	m_btnMgr.setText(m_cfgsrcLblEnableSF, _t("cfgsm3", L"Enable Sourceflow"));
	m_btnMgr.setText(m_cfgsrcLblSmallbox, _t("cfgsm4", L"Sourceflow Smallbox"));
	m_btnMgr.setText(m_cfgsrcLblBoxMode, _t("cfghb4", L"Box Mode"));
	m_btnMgr.setText(m_cfgsrcLblAdjustCF, _t("cfgc4", L"Adjust Coverflow"));
	m_btnMgr.setText(m_cfgsrcBtnAdjustCF, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_cfgsrcLblMultisource, _t("cfgbt6", L"Enable Multisource Features"));
	m_btnMgr.setText(m_cfgsrcBtnBack, _t("cfg10", L"Back"));
}
