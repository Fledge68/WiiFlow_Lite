
#include "menu.hpp"
#include "channel/nand_save.hpp"

s16 m_bootLblTitle;
s16 m_bootBtnBack;
s16 m_bootLblUser[4];

s16 m_bootLblLoadCIOS;
s16 m_bootBtnLoadCIOS;

s16 m_bootLblCIOSrev;
s16 m_bootLblCurCIOSrev;
s16 m_bootLblCIOSrevM;
s16 m_bootLblCIOSrevP;

s16 m_bootLblUSBPort;
s16 m_bootBtnUSBPort;

s16 m_bootLblAsyncNet;
s16 m_bootBtnAsyncNet;

u8 set_port = 0;

void CMenu::_hideBoot(bool instant)
{
	m_btnMgr.hide(m_bootLblTitle, instant);
	m_btnMgr.hide(m_bootBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_bootLblUser); ++i)
		if(m_bootLblUser[i] != -1)
			m_btnMgr.hide(m_bootLblUser[i], instant);
				
	m_btnMgr.hide(m_bootLblLoadCIOS, instant);
	m_btnMgr.hide(m_bootBtnLoadCIOS, instant);

	m_btnMgr.hide(m_bootLblCIOSrev, instant);
	m_btnMgr.hide(m_bootLblCurCIOSrev, instant);
	m_btnMgr.hide(m_bootLblCIOSrevM, instant);
	m_btnMgr.hide(m_bootLblCIOSrevP, instant);

	m_btnMgr.hide(m_bootLblUSBPort, instant);
	m_btnMgr.hide(m_bootBtnUSBPort, instant);
	
	m_btnMgr.hide(m_bootLblAsyncNet, instant);
	m_btnMgr.hide(m_bootBtnAsyncNet, instant);
}

void CMenu::_showBoot()
{
	m_btnMgr.show(m_bootLblTitle);
	m_btnMgr.show(m_bootBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_bootLblUser); ++i)
		if(m_bootLblUser[i] != -1)
			m_btnMgr.show(m_bootLblUser[i]);

	m_btnMgr.setText(m_bootBtnLoadCIOS, _optBoolToString(cur_load));
	m_btnMgr.setText(m_bootBtnUSBPort, wfmt(L"%i", set_port));
	if(cur_ios > 0)
		m_btnMgr.setText(m_bootLblCurCIOSrev, wfmt(L"%i", cur_ios));
	else
		m_btnMgr.setText(m_bootLblCurCIOSrev, L"AUTO");
	
	m_btnMgr.show(m_bootLblLoadCIOS);
	m_btnMgr.show(m_bootBtnLoadCIOS);

	m_btnMgr.show(m_bootLblCIOSrev);
	m_btnMgr.show(m_bootLblCurCIOSrev);
	m_btnMgr.show(m_bootLblCIOSrevM);
	m_btnMgr.show(m_bootLblCIOSrevP);

	m_btnMgr.show(m_bootLblUSBPort);
	m_btnMgr.show(m_bootBtnUSBPort);

	m_btnMgr.setText(m_bootBtnAsyncNet, m_cfg.getBool("GENERAL", "async_network", false) ? _t("on", L"On") : _t("off", L"Off"));
	m_btnMgr.show(m_bootLblAsyncNet);
	m_btnMgr.show(m_bootBtnAsyncNet);
}

bool CMenu::_Boot(void)
{
	if(isWiiVC)
	{
		error(_t("errboot7", L"Access denied in Wii VC mode."));
		return false;
	}
	set_port = currentPort;
	bool prev_load = cur_load;
	u8 prev_ios = cur_ios;
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
			else if(m_btnMgr.selected(m_bootBtnLoadCIOS))
			{
				cur_load = !cur_load;
				m_btnMgr.setText(m_bootBtnLoadCIOS, _optBoolToString(cur_load));
			}
			else if(m_btnMgr.selected(m_bootLblCIOSrevM) || m_btnMgr.selected(m_bootLblCIOSrevP))
			{
				bool increase = m_btnMgr.selected(m_bootLblCIOSrevP);
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
					m_btnMgr.setText(m_bootLblCurCIOSrev, wfmt(L"%i", cur_ios));
				else
					m_btnMgr.setText(m_bootLblCurCIOSrev, L"AUTO");
			}
			else if(m_btnMgr.selected(m_bootBtnUSBPort))
			{
				set_port = !set_port;
				m_btnMgr.setText(m_bootBtnUSBPort, wfmt(L"%i", set_port));
			}
			else if (m_btnMgr.selected(m_bootBtnAsyncNet))
			{
				m_cfg.setBool("GENERAL", "async_network", !m_cfg.getBool("GENERAL", "async_network", false));
				m_btnMgr.setText(m_bootBtnAsyncNet, m_cfg.getBool("GENERAL", "async_network", false) ? _t("on", L"On") : _t("off", L"Off"));
			}
		}
	}
	if(prev_load != cur_load || prev_ios != cur_ios)
		InternalSave.SaveIOS();
	if(set_port != currentPort)
		InternalSave.SavePort(set_port);
	_hideBoot();

	if(prev_load != cur_load || prev_ios != cur_ios || set_port != currentPort)
	{
		m_exit = true;
		m_reload = true;
		return 1;
	}
	return 0;
}

void CMenu::_initBoot(void)
{
	_addUserLabels(m_bootLblUser, ARRAY_SIZE(m_bootLblUser), "BOOT");
	m_bootLblTitle = _addTitle("BOOT/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_bootBtnBack = _addButton("BOOT/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	
	m_bootLblLoadCIOS = _addLabel("BOOT/LOAD_CIOS", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnLoadCIOS = _addButton("BOOT/LOAD_CIOS_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_bootLblCIOSrev = _addLabel("BOOT/CIOS_REV", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootLblCurCIOSrev = _addLabel("BOOT/CIOS_REV_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_bootLblCIOSrevM = _addPicButton("BOOT/CIOS_REV_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_bootLblCIOSrevP = _addPicButton("BOOT/CIOS_REV_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);

	m_bootLblUSBPort = _addLabel("BOOT/USB_PORT", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnUSBPort = _addButton("BOOT/USB_PORT_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	
	m_bootLblAsyncNet = _addLabel("BOOT/ASYNCNET", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnAsyncNet = _addButton("BOOT/ASYNCNET_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

	_setHideAnim(m_bootLblTitle, "BOOT/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_bootBtnBack, "BOOT/BACK_BTN", 0, 0, 1.f, -1.f);

	_setHideAnim(m_bootLblLoadCIOS, "BOOT/LOAD_CIOS", 50, 0, -2.f, 0.f);
	_setHideAnim(m_bootBtnLoadCIOS, "BOOT/LOAD_CIOS_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblCIOSrev, "BOOT/CIOS_REV", 50, 0, -2.f, 0.f);
	_setHideAnim(m_bootLblCurCIOSrev, "BOOT/CIOS_REV_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_bootLblCIOSrevM, "BOOT/CIOS_REV_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_bootLblCIOSrevP, "BOOT/CIOS_REV_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblUSBPort, "BOOT/USB_PORT", 50, 0, -2.f, 0.f);
	_setHideAnim(m_bootBtnUSBPort, "BOOT/USB_PORT_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblAsyncNet, "BOOT/ASYNCNET", 50, 0, -2.f, 0.f);
	_setHideAnim(m_bootBtnAsyncNet, "BOOT/ASYNCNET_BTN", -50, 0, 1.f, 0.f);
	
	_hideBoot(true);
	_textBoot();
}

void CMenu::_textBoot(void)
{
	m_btnMgr.setText(m_bootLblTitle, _t("cfgbt1", L"Startup Settings"));
	m_btnMgr.setText(m_bootLblLoadCIOS, _t("cfgbt2", L"Force Load cIOS"));
	m_btnMgr.setText(m_bootLblCIOSrev, _t("cfgbt3", L"Force cIOS Revision"));
	m_btnMgr.setText(m_bootLblUSBPort, _t("cfgbt4", L"USB Port"));
	m_btnMgr.setText(m_bootLblAsyncNet, _t("cfgp3", L"Init network on boot"));
	m_btnMgr.setText(m_bootBtnBack, _t("cfg10", L"Back"));
}
