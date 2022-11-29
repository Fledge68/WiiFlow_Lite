
#include "menu.hpp"

s16 m_cfghbLblTitle;
s16 m_cfghbBtnBack;
s16 m_cfghbLblUser[4];

TexData m_cfghbBg;

void CMenu::_hideConfigHB(bool instant)
{
	m_btnMgr.hide(m_cfghbLblTitle, instant);
	m_btnMgr.hide(m_cfghbBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfghbLblUser); ++i)
		if(m_cfghbLblUser[i] != -1)
			m_btnMgr.hide(m_cfghbLblUser[i], instant);

	_hideConfigButtons(instant);
}

void CMenu::_showConfigHB(void)
{
	_setBg(m_cfghbBg, m_cfghbBg);
	m_btnMgr.show(m_cfghbLblTitle);
	m_btnMgr.show(m_cfghbBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfghbLblUser); ++i)
		if(m_cfghbLblUser[i] != -1)
			m_btnMgr.show(m_cfghbLblUser[i]);

	m_btnMgr.setText(m_configLbl1, _t("cfghb5", L"Hide homebrew button"));
	m_btnMgr.setText(m_configLbl2, _t("cfghb2", L"Coverflow Smallbox"));
	m_btnMgr.setText(m_configLbl3, _t("cfghb4", L"Box Mode"));
	m_btnMgr.setText(m_configLbl4, _t("cfghb3", L"Homebrew Partition"));
	
	m_btnMgr.setText(m_configBtn1, m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false) ? _t("yes", L"Yes") : _t("no", L"No"));
	m_btnMgr.setText(m_configBtn2, m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.setText(m_configBtn3, m_cfg.getBool(HOMEBREW_DOMAIN, "box_mode") ? _t("on", L"On") : _t("off", L"Off"));
	
	currentPartition = m_cfg.getInt(HOMEBREW_DOMAIN, "partition", 0);
	const char *partitionname = DeviceName[currentPartition];
	m_btnMgr.setText(m_configLbl4Val, upperCase(partitionname));

	m_btnMgr.show(m_configLbl1);
	m_btnMgr.show(m_configBtn1);
	
	m_btnMgr.show(m_configLbl2);
	m_btnMgr.show(m_configBtn2);
	
	m_btnMgr.show(m_configLbl3);
	m_btnMgr.show(m_configBtn3);

	m_btnMgr.show(m_configLbl4);
	m_btnMgr.show(m_configLbl4Val);
	m_btnMgr.show(m_configBtn4P);
	m_btnMgr.show(m_configBtn4M);
}

void CMenu::_ConfigHB(void)
{
	SetupInput();
	_showConfigHB();

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
			else if (m_btnMgr.selected(m_configBtn1))
			{
				m_cfg.setBool(HOMEBREW_DOMAIN, "disable", !m_cfg.getBool(HOMEBREW_DOMAIN, "disable"));
				m_btnMgr.setText(m_configBtn1, m_cfg.getBool(HOMEBREW_DOMAIN, "disable") ? _t("yes", L"Yes") : _t("no", L"No"));
			}
			else if (m_btnMgr.selected(m_configBtn2))
			{
				m_refreshGameList = true;
				m_cfg.setBool(HOMEBREW_DOMAIN, "update_cache", true);
				m_cfg.setBool(HOMEBREW_DOMAIN, "smallbox", !m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", false));
				m_btnMgr.setText(m_configBtn2, m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_configBtn3))
			{
				m_refreshGameList = true;
				m_cfg.setBool(HOMEBREW_DOMAIN, "box_mode", !m_cfg.getBool(HOMEBREW_DOMAIN, "box_mode", false));
				m_btnMgr.setText(m_configBtn3, m_cfg.getBool(HOMEBREW_DOMAIN, "box_mode") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
			{
				s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
				_setPartition(direction, m_cfg.getInt(HOMEBREW_DOMAIN, "partition"), COVERFLOW_HOMEBREW);
				const char *partitionname = DeviceName[currentPartition];
				m_btnMgr.setText(m_configLbl4Val, upperCase(partitionname));
				if(m_current_view & COVERFLOW_HOMEBREW || 
					(m_current_view & COVERFLOW_PLUGIN && m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(strtoul("48425257", NULL, 16)))))
				{
					m_refreshGameList = true;
					//m_cfg.setBool(HOMEBREW_DOMAIN, "update_cache", true);
				}
			}
		}
	}
	_hideConfigHB();
}

void CMenu::_initConfigHB(void)
{
	m_cfghbBg = _texture("CFG_HB/BG", "texture", theme.bg, false);
	
	_addUserLabels(m_cfghbLblUser, ARRAY_SIZE(m_cfghbLblUser), "CFG_HB");
	m_cfghbLblTitle = _addLabel("CFG_HB/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_cfghbBtnBack = _addButton("CFG_HB/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	
	_setHideAnim(m_cfghbLblTitle, "CFG_HB/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cfghbBtnBack, "CFG_HB/BACK_BTN", 0, 0, 1.f, -1.f);
	
	_hideConfigHB(true);
	_textConfigHB();
}

void CMenu::_textConfigHB(void)
{
	m_btnMgr.setText(m_cfghbLblTitle, _t("cfghb1", L"Homebrew Settings"));
	m_btnMgr.setText(m_cfghbBtnBack, _t("cfg10", L"Back"));
}
