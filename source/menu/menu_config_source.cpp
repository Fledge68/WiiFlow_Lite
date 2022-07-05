
#include "menu.hpp"

s16 m_cfgsrcLblTitle;
s16 m_cfgsrcBtnBack;
s16 m_cfgsrcLblUser[4];

TexData m_cfgsrcBg;

void CMenu::_hideConfigSrc(bool instant)
{
	m_btnMgr.hide(m_cfgsrcLblTitle, instant);
	m_btnMgr.hide(m_cfgsrcBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfgsrcLblUser); ++i)
		if(m_cfgsrcLblUser[i] != -1)
			m_btnMgr.hide(m_cfgsrcLblUser[i], instant);

	_hideConfigButtons(instant);
}

void CMenu::_showConfigSrc(bool m_sourceflow)
{
	_setBg(m_cfgsrcBg, m_cfgsrcBg);
	m_btnMgr.show(m_cfgsrcLblTitle);
	m_btnMgr.show(m_cfgsrcBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfgsrcLblUser); ++i)
		if(m_cfgsrcLblUser[i] != -1)
			m_btnMgr.show(m_cfgsrcLblUser[i]);

	m_btnMgr.setText(m_configLbl1, _t("cfgsm3", L"Enable Sourceflow"));
	m_btnMgr.setText(m_configBtn1, m_cfg.getBool(SOURCEFLOW_DOMAIN, "enabled") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.show(m_configLbl1);
	m_btnMgr.show(m_configBtn1);
	
	if(m_sourceflow)
	{
		m_btnMgr.setText(m_configLbl2, _t("cfgsm4", L"Sourceflow Smallbox"));
		m_btnMgr.setText(m_configLbl3, _t("cfghb4", L"Box Mode"));
		m_btnMgr.setText(m_configLbl4, _t("cfgc4", L"Adjust Coverflow"));

		m_btnMgr.setText(m_configBtn2, m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configBtn3, m_cfg.getBool(SOURCEFLOW_DOMAIN, "box_mode") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_configBtn4, _t("cfgc5", L"Go"));
	
		m_btnMgr.show(m_configLbl2);
		m_btnMgr.show(m_configBtn2);

		m_btnMgr.show(m_configLbl3);
		m_btnMgr.show(m_configBtn3);
		
		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configBtn4);
	}
	else
	{
		m_btnMgr.hide(m_configLbl2, true);
		m_btnMgr.hide(m_configBtn2, true);

		m_btnMgr.hide(m_configLbl3, true);
		m_btnMgr.hide(m_configBtn3, true);

		m_btnMgr.hide(m_configLbl4, true);
		m_btnMgr.hide(m_configBtn4, true);
	}
}

void CMenu::_ConfigSrc(void)
{
	if(!m_use_source)
	{
		error(_t("cfgsmerr", L"No source menu found!"));
		return;
	}
	SetupInput();
	_showConfigSrc(m_cfg.getBool(SOURCEFLOW_DOMAIN, "enabled", false));
	bool temp;
	
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
			else if(m_btnMgr.selected(m_configBtn1))
			{
				temp = !m_cfg.getBool(SOURCEFLOW_DOMAIN, "enabled", false);
				m_cfg.setBool(SOURCEFLOW_DOMAIN, "enabled", temp);
				_showConfigSrc(temp);
			}
			else if(m_btnMgr.selected(m_configBtn2))
			{
				SF_cacheCovers = true;
				temp = !m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox", false);
				m_cfg.setBool(SOURCEFLOW_DOMAIN, "smallbox", temp);
				m_btnMgr.setText(m_configBtn2, temp ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if(m_btnMgr.selected(m_configBtn3))
			{
				SF_cacheCovers = true;
				temp = !m_cfg.getBool(SOURCEFLOW_DOMAIN, "box_mode", false);
				m_cfg.setBool(SOURCEFLOW_DOMAIN, "box_mode", temp);
				m_btnMgr.setText(m_configBtn3, temp ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if(m_btnMgr.selected(m_configBtn4))
			{
				_hideConfigSrc(true);
				m_sourceflow = true;
				_showCF(true);
				_cfTheme();
				m_sourceflow = false;
				_showCF(true);
				_showConfigSrc(true);
			}
		}
	}
	_hideConfigSrc(true);
}

void CMenu::_initConfigSrc(void)
{
	m_cfgsrcBg = _texture("CFG_SRC/BG", "texture", theme.bg, false);
	
	_addUserLabels(m_cfgsrcLblUser, ARRAY_SIZE(m_cfgsrcLblUser), "CFG_SRC");
	m_cfgsrcLblTitle = _addLabel("CFG_SRC/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnBack = _addButton("CFG_SRC/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	
	_setHideAnim(m_cfgsrcLblTitle, "CFG_SRC/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnBack, "CFG_SRC/BACK_BTN", 0, 0, 1.f, -1.f);
	
	_hideConfigSrc(true);
	_textConfigSrc();
}

void CMenu::_textConfigSrc(void)
{
	m_btnMgr.setText(m_cfgsrcLblTitle, _t("cfgsm1", L"Sourceflow Settings"));
	m_btnMgr.setText(m_cfgsrcBtnBack, _t("cfg10", L"Back"));
}
