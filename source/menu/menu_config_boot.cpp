
#include "menu.hpp"
#include "channel/nand_save.hpp"

TexData m_bootBg;

s16 m_bootLblTitle;
s16 m_bootBtnBack;
s16 m_bootLblUser[4];

u8 set_port = 0;

void CMenu::_hideBoot(bool instant)
{
	m_btnMgr.hide(m_bootLblTitle, instant);
	m_btnMgr.hide(m_bootBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_bootLblUser); ++i)
		if(m_bootLblUser[i] != -1)
			m_btnMgr.hide(m_bootLblUser[i], instant);
				
	_hideConfigButtons(instant);
}

void CMenu::_showBoot()
{
	_setBg(m_bootBg, m_bootBg);
	m_btnMgr.show(m_bootLblTitle);
	m_btnMgr.show(m_bootBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_bootLblUser); ++i)
		if(m_bootLblUser[i] != -1)
			m_btnMgr.show(m_bootLblUser[i]);

	m_btnMgr.setText(m_configLbl1, _t("cfgbt2", L"Force Load cIOS"));
	m_btnMgr.setText(m_configLbl2, _t("cfgbt3", L"Force cIOS Revision"));
	m_btnMgr.setText(m_configLbl3, _t("cfgbt4", L"USB Port"));
	m_btnMgr.setText(m_configLbl4, _t("cfg719", L"Mount SD only"));

	m_btnMgr.setText(m_configBtn1, _optBoolToString(cur_load));
	if(cur_ios > 0)
		m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", cur_ios));
	else
		m_btnMgr.setText(m_configLbl2Val, L"AUTO");// cIOS 249 unless the user changed it via the meta.xml
	m_btnMgr.setText(m_configBtn3, wfmt(L"%i", set_port));
	m_btnMgr.setText(m_configBtn4, m_cfg.getBool("GENERAL", "sd_only") ? _t("yes", L"Yes") : _t("no", L"No"));

	m_btnMgr.show(m_configLbl1);
	m_btnMgr.show(m_configBtn1);

	m_btnMgr.show(m_configLbl2);
	m_btnMgr.show(m_configLbl2Val);
	m_btnMgr.show(m_configBtn2M);
	m_btnMgr.show(m_configBtn2P);

	m_btnMgr.show(m_configLbl3);
	m_btnMgr.show(m_configBtn3);

	m_btnMgr.show(m_configLbl4);
	m_btnMgr.show(m_configBtn4);
}

void CMenu::_Boot(void)
{
	if(isWiiVC)
	{
		error(_t("errboot7", L"Access denied in Wii VC mode."));
		return;
	}
	set_port = currentPort;
	bool prev_load = cur_load;
	u8 prev_ios = cur_ios;
	bool prev_sd = m_cfg.getBool("GENERAL", "sd_only");
	bool cur_sd = prev_sd;
	SetupInput();
	_showBoot();

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
			if(m_btnMgr.selected(m_bootBtnBack))
				break;
			else if(m_btnMgr.selected(m_configBtn1))
			{
				cur_load = !cur_load;
				m_btnMgr.setText(m_configBtn1, _optBoolToString(cur_load));
			}
			else if(m_btnMgr.selected(m_configBtn2M) || m_btnMgr.selected(m_configBtn2P))
			{
				bool increase = m_btnMgr.selected(m_configBtn2P);
				CIOSItr itr = _installed_cios.find(cur_ios);
				if(increase)
				{
					itr++;
					if(itr == _installed_cios.end())
						itr = _installed_cios.begin();
				}
				else
				{
					if(itr == _installed_cios.begin())
						itr = _installed_cios.end();
					itr--;
				}
				cur_ios = itr->first;
				if(cur_ios > 0)
					m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", cur_ios));
				else
					m_btnMgr.setText(m_configLbl2Val, L"AUTO");
			}
			else if(m_btnMgr.selected(m_configBtn3))
			{
				set_port = !set_port;
				m_btnMgr.setText(m_configBtn3, wfmt(L"%i", set_port));
			}
			else if (m_btnMgr.selected(m_configBtn4))
			{
				cur_sd = !cur_sd;
				m_btnMgr.setText(m_configBtn4, cur_sd ?  _t("yes", L"Yes") : _t("no", L"No"));
			}
		}
	}
	if(cur_sd != prev_sd)
	{
		InternalSave.SaveSDOnly(cur_sd);
		m_cfg.setBool("GENERAL", "sd_only", cur_sd);// backwards compatibity
	}
	if(prev_load != cur_load || prev_ios != cur_ios)
		InternalSave.SaveIOS();
	if(set_port != currentPort)
		InternalSave.SavePort(set_port);
	_hideBoot();

	if(prev_load != cur_load || prev_ios != cur_ios || set_port != currentPort || prev_sd != cur_sd)
	{
		error(_t("errboot8", L"Press 'A' to reload WiiFlow"));
		vector<string> arguments = _getMetaXML(fmt("%s/boot.dol", m_appDir.c_str()));
		_launchHomebrew(fmt("%s/boot.dol", m_appDir.c_str()), arguments);
	}
}

void CMenu::_initBoot(void)
{
	m_bootBg = _texture("BOOT/BG", "texture", theme.bg, false);
	
	_addUserLabels(m_bootLblUser, ARRAY_SIZE(m_bootLblUser), "BOOT");
	m_bootLblTitle = _addLabel("BOOT/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_bootBtnBack = _addButton("BOOT/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	
	_setHideAnim(m_bootLblTitle, "BOOT/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_bootBtnBack, "BOOT/BACK_BTN", 0, 0, 1.f, -1.f);
	
	_hideBoot(true);
	_textBoot();
}

void CMenu::_textBoot(void)
{
	m_btnMgr.setText(m_bootLblTitle, _t("cfgbt1", L"Startup Settings"));
	m_btnMgr.setText(m_bootBtnBack, _t("cfg10", L"Back"));
}
