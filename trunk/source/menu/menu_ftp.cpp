/****************************************************************************
 * Copyright (C) 2013 FIX94
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
#include "network/net.h"
#include "network/ftp.h"
#include "network/http.h"
#include "network/gcard.h"
#include "network/FTP_Dir.hpp"

s16 m_ftpLblTitle;
s16 m_ftpBtnBack;
s16 m_ftpBtnToggle;
s16 m_ftpLblInfo;
s16 m_ftpLblUser[4];

void CMenu::_updateFTP(void)
{
	_hideFTP(true);
	if(m_ftp_inited == true)
	{
		in_addr addr;
		addr.s_addr = net_gethostip();
		m_btnMgr.hide(m_wbfsLblDialog, true);
		m_btnMgr.setText(m_ftpLblTitle, wfmt(_t("dlmsg28", L"Running FTP Server on %s:%d"), inet_ntoa(addr), ftp_server_port));
		m_btnMgr.show(m_ftpLblTitle);
		m_btnMgr.setText(m_ftpBtnToggle, _t("ftp2", L"Stop"));
		m_btnMgr.show(m_ftpLblInfo);
	}
	else
	{
		m_btnMgr.hide(m_ftpLblTitle, true);
		m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg29", L"FTP Server is currently stopped."));
		m_btnMgr.setText(m_ftpBtnToggle, _t("ftp1", L"Start"));
		m_btnMgr.show(m_wbfsLblDialog);
	}
	_showFTP();
}

void CMenu::_FTP(void)
{
	_updateFTP();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_ftpBtnBack))
				break;
			else if(m_btnMgr.selected(m_ftpBtnToggle))
			{
				if(m_ftp_inited == true)
				{
					ftp_endTread();
					m_init_ftp = false;
					m_ftp_inited = false;
					init_network = (m_cfg.getBool("GENERAL", "async_network") || has_enabled_providers() || m_use_wifi_gecko);
					ftp_dbg_print_update();
				}
				else
				{
					m_init_ftp = true;
					init_network = true;
					if(networkInit == false)
					{
						gprintf("no net init yet\n");
						_netInit();
					}
					else
					{
						gprintf("net already inited, just start server\n");
						m_ftp_inited = ftp_startThread();
					}
				}
				_updateFTP();
			}
		}
		if(ftp_dbg_print_update())
		{
			m_btnMgr.setText(m_ftpLblInfo, wfmt(L"%s%s%s%s%s%s", ftp_get_prints(5), ftp_get_prints(4), 
				ftp_get_prints(3), ftp_get_prints(2), ftp_get_prints(1), ftp_get_prints(0)));
		}
	}
	m_btnMgr.hide(m_wbfsLblDialog);
	_hideFTP();
}

void CMenu::_showFTP(void)
{
	m_btnMgr.show(m_ftpBtnToggle);
	m_btnMgr.show(m_ftpBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_ftpLblUser); ++i)
		if(m_ftpLblUser[i] != -1)
			m_btnMgr.show(m_ftpLblUser[i]);
}

void CMenu::_hideFTP(bool instant)
{
	m_btnMgr.hide(m_ftpLblTitle, instant);
	m_btnMgr.hide(m_ftpBtnToggle, instant);
	m_btnMgr.hide(m_ftpBtnBack, instant);
	m_btnMgr.hide(m_ftpLblInfo, instant);
		for(u8 i = 0; i < ARRAY_SIZE(m_ftpLblUser); ++i)
			if(m_ftpLblUser[i] != -1)
				m_btnMgr.hide(m_ftpLblUser[i], instant);
}

void CMenu::_initFTP(void)
{
	_addUserLabels(m_ftpLblUser, ARRAY_SIZE(m_ftpLblUser), "FTP");

	m_ftpLblTitle = _addTitle("FTP/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_ftpBtnToggle = _addButton("FTP/TOGGLE_BTN", theme.btnFont, L"", 20, 400, 200, 56, theme.btnFontColor);
	m_ftpBtnBack = _addButton("FTP/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_ftpLblInfo = _addText("FTP/INFO", theme.txtFont, L"", 40, 115, 560, 270, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);

	_setHideAnim(m_ftpLblTitle, "FTP/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_ftpBtnToggle, "FTP/TOGGLE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_ftpBtnBack, "FTP/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_ftpLblInfo, "FTP/INFO", 0, 100, 0.f, 0.f);

	_textFTP();
	_hideFTP(true);
}

void CMenu::_textFTP(void)
{
	m_btnMgr.setText(m_ftpLblTitle, L"");
	m_btnMgr.setText(m_ftpBtnToggle, L"");
	m_btnMgr.setText(m_ftpBtnBack, _t("cfg10", L"Back"));
	m_btnMgr.setText(m_ftpLblInfo, L"");
}

