/****************************************************************************
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "menu.hpp"
#include "const_str.hpp"
#include "channel/nand_save.hpp"

s16 m_bootLblTitle;
s16 m_bootBtnBack;
s16 m_bootLblPage;
s16 m_bootBtnPageM;
s16 m_bootBtnPageP;
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

s16 m_bootLblCategoryOnBoot;
s16 m_bootBtnCategoryOnBoot;

s16 m_bootLblSourceOnBoot;
s16 m_bootBtnSourceOnBoot;

s16 m_bootLblMultisource;
s16 m_bootBtnMultisource;

s16 m_bootLblFtpOnBoot;
s16 m_bootBtnFtpOnBoot;

u8 set_port = 0;
u8 boot_curPage = 1;
u8 boot_Pages = 2;

static void showBoot(void)
{
	m_btnMgr.show(m_bootLblTitle);
	m_btnMgr.show(m_bootBtnBack);
	m_btnMgr.show(m_bootLblPage);
	m_btnMgr.show(m_bootBtnPageM);
	m_btnMgr.show(m_bootBtnPageP);
	for(u8 i = 0; i < ARRAY_SIZE(m_bootLblUser); ++i)
		if(m_bootLblUser[i] != -1)
			m_btnMgr.show(m_bootLblUser[i]);
}

static void hideBoot(bool instant, bool common)
{
	if(common)
	{
		m_btnMgr.hide(m_bootLblTitle, instant);
		m_btnMgr.hide(m_bootBtnBack, instant);
		m_btnMgr.hide(m_bootLblPage, instant);
		m_btnMgr.hide(m_bootBtnPageM, instant);
		m_btnMgr.hide(m_bootBtnPageP, instant);
		for(u8 i = 0; i < ARRAY_SIZE(m_bootLblUser); ++i)
			if(m_bootLblUser[i] != -1)
				m_btnMgr.hide(m_bootLblUser[i], instant);
	}
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
	/* page 2 */
	m_btnMgr.hide(m_bootLblCategoryOnBoot, instant);
	m_btnMgr.hide(m_bootBtnCategoryOnBoot, instant);
	
	m_btnMgr.hide(m_bootLblSourceOnBoot, instant);
	m_btnMgr.hide(m_bootBtnSourceOnBoot, instant);
	
	m_btnMgr.hide(m_bootLblMultisource, instant);
	m_btnMgr.hide(m_bootBtnMultisource, instant);
	
	m_btnMgr.hide(m_bootLblFtpOnBoot, instant);
	m_btnMgr.hide(m_bootBtnFtpOnBoot, instant);
}

bool CMenu::_Boot(void)
{
	boot_curPage = 1;
	SetupInput();
	set_port = currentPort;
	bool prev_load = cur_load;
	u8 prev_ios = cur_ios;
	showBoot();
	_refreshBoot();

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) || (BTN_A_PRESSED && m_btnMgr.selected(m_bootBtnPageM)))
		{
			boot_curPage--;
			if(boot_curPage == 0) boot_curPage = boot_Pages;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_bootBtnPageM);
			_refreshBoot();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED)) || (BTN_A_PRESSED && m_btnMgr.selected(m_bootBtnPageP)))
		{
			boot_curPage++;
			if(boot_curPage > boot_Pages) boot_curPage = 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_bootBtnPageP);
			_refreshBoot();
		}
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
			else if (m_btnMgr.selected(m_bootBtnCategoryOnBoot))
			{
				m_cfg.setBool("GENERAL", "category_on_start", !m_cfg.getBool("GENERAL", "category_on_start", false));
				m_btnMgr.setText(m_bootBtnCategoryOnBoot, m_cfg.getBool("GENERAL", "category_on_start") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_bootBtnSourceOnBoot))
			{
				m_cfg.setBool("GENERAL", "source_on_start", !m_cfg.getBool("GENERAL", "source_on_start", false));
				m_btnMgr.setText(m_bootBtnSourceOnBoot, m_cfg.getBool("GENERAL", "source_on_start") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_bootBtnMultisource))
			{
				m_cfg.setBool("GENERAL", "multisource", !m_cfg.getBool("GENERAL", "multisource", false));
				m_btnMgr.setText(m_bootBtnMultisource, m_cfg.getBool("GENERAL", "multisource") ? _t("on", L"On") : _t("off", L"Off"));
				m_multisource = m_cfg.getBool("GENERAL", "multisource", false);
			}
			else if (m_btnMgr.selected(m_bootBtnFtpOnBoot))
			{
				m_cfg.setBool(FTP_DOMAIN, "auto_start", !m_cfg.getBool(FTP_DOMAIN, "auto_start", false));
				m_btnMgr.setText(m_bootBtnFtpOnBoot, m_cfg.getBool(FTP_DOMAIN, "auto_start") ? _t("on", L"On") : _t("off", L"Off"));
			}
		}
	}
	if(prev_load != cur_load || prev_ios != cur_ios)
		InternalSave.SaveIOS();
	if(set_port != currentPort)
		InternalSave.SavePort(set_port);
	hideBoot(false, true);

	if(prev_load != cur_load || prev_ios != cur_ios || set_port != currentPort)
	{
		m_exit = true;
		m_reload = true;
		return 1;
	}
	return 0;
}

void CMenu::_refreshBoot()
{
	hideBoot(true, false);
	m_btnMgr.setText(m_bootLblPage, wfmt(L"%i / %i", boot_curPage, boot_Pages));
	if(boot_curPage == 1)
	{
		m_btnMgr.setText(m_bootBtnLoadCIOS, _optBoolToString(cur_load));
		m_btnMgr.setText(m_bootBtnUSBPort, wfmt(L"%i", set_port));
		if(cur_ios > 0)
			m_btnMgr.setText(m_bootLblCurCIOSrev, wfmt(L"%i", cur_ios));
		else
			m_btnMgr.setText(m_bootLblCurCIOSrev, L"AUTO");
		m_btnMgr.setText(m_bootBtnAsyncNet, m_cfg.getBool("GENERAL", "async_network", false) ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_bootLblLoadCIOS);
		m_btnMgr.show(m_bootBtnLoadCIOS);

		m_btnMgr.show(m_bootLblCIOSrev);
		m_btnMgr.show(m_bootLblCurCIOSrev);
		m_btnMgr.show(m_bootLblCIOSrevM);
		m_btnMgr.show(m_bootLblCIOSrevP);

		m_btnMgr.show(m_bootLblUSBPort);
		m_btnMgr.show(m_bootBtnUSBPort);
		
		m_btnMgr.show(m_bootLblAsyncNet);
		m_btnMgr.show(m_bootBtnAsyncNet);
	}
	else
	{
		m_btnMgr.setText(m_bootBtnCategoryOnBoot, m_cfg.getBool("GENERAL", "category_on_start") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_bootBtnSourceOnBoot, m_cfg.getBool("GENERAL", "source_on_start") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_bootBtnMultisource, m_cfg.getBool("GENERAL", "multisource") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_bootBtnFtpOnBoot, m_cfg.getBool(FTP_DOMAIN, "auto_start") ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_bootLblCategoryOnBoot);
		m_btnMgr.show(m_bootBtnCategoryOnBoot);
		
		m_btnMgr.show(m_bootLblSourceOnBoot);
		m_btnMgr.show(m_bootBtnSourceOnBoot);
		
		m_btnMgr.show(m_bootLblMultisource);
		m_btnMgr.show(m_bootBtnMultisource);
		
		m_btnMgr.show(m_bootLblFtpOnBoot);
		m_btnMgr.show(m_bootBtnFtpOnBoot);
	}
}

void CMenu::_textBoot(void)
{
	m_btnMgr.setText(m_bootLblTitle, _t("cfgbt1", L"Startup Settings"));
	m_btnMgr.setText(m_bootLblLoadCIOS, _t("cfgbt2", L"Force Load cIOS"));
	m_btnMgr.setText(m_bootLblCIOSrev, _t("cfgbt3", L"Force cIOS Revision"));
	m_btnMgr.setText(m_bootLblUSBPort, _t("cfgbt4", L"USB Port"));
	m_btnMgr.setText(m_bootLblAsyncNet, _t("cfgp3", L"Init network on boot"));
	m_btnMgr.setText(m_bootLblCategoryOnBoot, _t("cfgd7", L"Show categories on boot"));
	m_btnMgr.setText(m_bootLblSourceOnBoot, _t("cfgbt5", L"Show source menu on boot"));
	m_btnMgr.setText(m_bootLblMultisource, _t("cfgbt6", L"Enable Multisource Features"));
	m_btnMgr.setText(m_bootLblFtpOnBoot, _t("cfgbt7", L"Start FTP Server on boot"));
	m_btnMgr.setText(m_bootBtnBack, _t("cfg10", L"Back"));
}


void CMenu::_initBoot(void)
{
	_addUserLabels(m_bootLblUser, ARRAY_SIZE(m_bootLblUser), "BOOT");
	m_bootLblTitle = _addTitle("BOOT/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_bootBtnBack = _addButton("BOOT/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_bootLblPage = _addLabel("BOOT/PAGE_BTN", theme.btnFont, L"", 76, 400, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_bootBtnPageM = _addPicButton("BOOT/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_bootBtnPageP = _addPicButton("BOOT/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 400, 56, 56);
	
	m_bootLblLoadCIOS = _addLabel("BOOT/LOAD_CIOS", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnLoadCIOS = _addButton("BOOT/LOAD_CIOS_BTN", theme.btnFont, L"", 370, 130, 230, 56, theme.btnFontColor);

	m_bootLblCIOSrev = _addLabel("BOOT/CIOS_REV", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootLblCurCIOSrev = _addLabel("BOOT/CIOS_REV_BTN", theme.btnFont, L"", 426, 190, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_bootLblCIOSrevM = _addPicButton("BOOT/CIOS_REV_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 190, 56, 56);
	m_bootLblCIOSrevP = _addPicButton("BOOT/CIOS_REV_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);

	m_bootLblUSBPort = _addLabel("BOOT/USB_PORT", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnUSBPort = _addButton("BOOT/USB_PORT_BTN", theme.btnFont, L"", 370, 250, 230, 56, theme.btnFontColor);

	m_bootLblAsyncNet = _addLabel("BOOT/ASYNCNET", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnAsyncNet = _addButton("BOOT/ASYNCNET_BTN", theme.btnFont, L"", 370, 310, 230, 56, theme.btnFontColor);
	/* page 2 */
	m_bootLblCategoryOnBoot = _addLabel("BOOT/CAT_ON_START", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnCategoryOnBoot = _addButton("BOOT/CAT_ON_START_BTN", theme.btnFont, L"", 370, 130, 230, 56, theme.btnFontColor);
	
	m_bootLblSourceOnBoot = _addLabel("BOOT/SOURCE_ON_START", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnSourceOnBoot = _addButton("BOOT/SOURCE_ON_START_BTN", theme.btnFont, L"", 370, 190, 230, 56, theme.btnFontColor);
	
	m_bootLblMultisource = _addLabel("BOOT/MULTISOURCE", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnMultisource = _addButton("BOOT/MULTISOURCE_BTN", theme.btnFont, L"", 370, 250, 230, 56, theme.btnFontColor);
	
	m_bootLblFtpOnBoot = _addLabel("BOOT/FTP", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnFtpOnBoot = _addButton("BOOT/FTP_BTN", theme.btnFont, L"", 370, 310, 230, 56, theme.btnFontColor);
	
	_setHideAnim(m_bootLblTitle, "BOOT/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_bootBtnBack, "BOOT/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_bootLblPage, "BOOT/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_bootBtnPageM, "BOOT/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_bootBtnPageP, "BOOT/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_bootLblLoadCIOS, "BOOT/LOAD_CIOS", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootBtnLoadCIOS, "BOOT/LOAD_CIOS_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblCIOSrev, "BOOT/CIOS_REV", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootLblCurCIOSrev, "BOOT/CIOS_REV_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_bootLblCIOSrevM, "BOOT/CIOS_REV_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_bootLblCIOSrevP, "BOOT/CIOS_REV_PLUS", 200, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblUSBPort, "BOOT/USB_PORT", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootBtnUSBPort, "BOOT/USB_PORT_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblAsyncNet, "BOOT/ASYNCNET", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootBtnAsyncNet, "BOOT/ASYNCNET_BTN", 200, 0, 1.f, 0.f);
	
	_setHideAnim(m_bootLblCategoryOnBoot, "BOOT/CAT_ON_START", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootBtnCategoryOnBoot, "BOOT/CAT_ON_START_BTN", 200, 0, 1.f, 0.f);
	
	_setHideAnim(m_bootLblSourceOnBoot, "BOOT/SOURCE_ON_START", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootBtnSourceOnBoot, "BOOT/SOURCE_ON_START_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblMultisource, "BOOT/MULTISOURCE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootBtnMultisource, "BOOT/MULTISOURCE_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblFtpOnBoot, "BOOT/FTP", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootBtnFtpOnBoot, "BOOT/FTP_BTN", 200, 0, 1.f, 0.f);

	hideBoot(true, true);
	_textBoot();
}
