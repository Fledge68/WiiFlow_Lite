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
#include "fileOps/fileOps.h"
#include "channel/nand_save.hpp"

s16 m_bootLblTitle;
s16 m_bootLblLoadCIOS;
s16 m_bootBtnLoadCIOS;

s16 m_bootLblCIOSrev;
s16 m_bootLblCurCIOSrev;
s16 m_bootLblCIOSrevM;
s16 m_bootLblCIOSrevP;

static void showBoot(void)
{
	m_btnMgr.show(m_bootLblTitle);
	m_btnMgr.show(m_bootLblLoadCIOS);
	m_btnMgr.show(m_bootBtnLoadCIOS);

	m_btnMgr.show(m_bootLblCIOSrev);
	m_btnMgr.show(m_bootLblCurCIOSrev);
	m_btnMgr.show(m_bootLblCIOSrevM);
	m_btnMgr.show(m_bootLblCIOSrevP);
}

static void hideBoot(bool instant)
{
	m_btnMgr.hide(m_bootLblTitle, instant);
	m_btnMgr.hide(m_bootLblLoadCIOS, instant);
	m_btnMgr.hide(m_bootBtnLoadCIOS, instant);

	m_btnMgr.hide(m_bootLblCIOSrev, instant);
	m_btnMgr.hide(m_bootLblCurCIOSrev, instant);
	m_btnMgr.hide(m_bootLblCIOSrevM, instant);
	m_btnMgr.hide(m_bootLblCIOSrevP, instant);
}

void CMenu::_Boot(void)
{
	SetupInput();
	_refreshBoot();
	bool prev_load = m_cfg.getBool("GENERAL", "force_cios_load", false);
	u8 prev_ios = min(m_cfg.getInt("GENERAL", "force_cios_rev", 0), 254);

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;	
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_bootBtnLoadCIOS))
			{
				bool old = m_cfg.getBool("GENERAL", "force_cios_load", false);
				m_cfg.setBool("GENERAL", "force_cios_load", !old);
				_refreshBoot();
			}
			else if(m_btnMgr.selected(m_bootLblCIOSrevM) || m_btnMgr.selected(m_bootLblCIOSrevP))
			{
				bool increase = m_btnMgr.selected(m_bootLblCIOSrevP);
				CIOSItr itr = _installed_cios.find(min(m_cfg.getInt("GENERAL", "force_cios_rev", 0), 254));
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
				m_cfg.setInt("GENERAL", "force_cios_rev", itr->first);
			}
			_refreshBoot();
		}
	}
	bool cur_load = m_cfg.getBool("GENERAL", "force_cios_load", false);
	u8 cur_ios = min(m_cfg.getInt("GENERAL", "force_cios_rev", 0), 254);
	if(prev_load != cur_load || prev_ios != cur_ios)
		InternalSave.SaveIOS(cur_ios, cur_load);
	hideBoot(false);
}

void CMenu::_refreshBoot(void)
{
	m_btnMgr.setText(m_bootBtnLoadCIOS, _optBoolToString(m_cfg.getBool("GENERAL", "force_cios_load", false)));
	u8 IOS_Revision = min(m_cfg.getInt("GENERAL", "force_cios_rev", 0), 254);
	if(IOS_Revision > 0)
		m_btnMgr.setText(m_bootLblCurCIOSrev, wfmt(L"%i", IOS_Revision));
	else
		m_btnMgr.setText(m_bootLblCurCIOSrev, L"AUTO");
	showBoot();
}

void CMenu::_textBoot(void)
{
	m_btnMgr.setText(m_bootLblTitle, _t("cfgbt1", L"Startup Settings"));
	m_btnMgr.setText(m_bootLblLoadCIOS, _t("cfgbt2", L"Force Load cIOS"));
	m_btnMgr.setText(m_bootLblCIOSrev, _t("cfgbt3", L"Force cIOS Revision"));
}


void CMenu::_initBoot(void)
{
	m_bootLblTitle = _addTitle("BOOT/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	m_bootLblLoadCIOS = _addLabel("BOOT/LOAD_CIOS", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootBtnLoadCIOS = _addButton("BOOT/LOAD_CIOS_BTN", theme.btnFont, L"", 330, 130, 270, 56, theme.btnFontColor);

	m_bootLblCIOSrev = _addLabel("BOOT/CIOS_REV", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_bootLblCurCIOSrev = _addLabel("BOOT/CIOS_REV_BTN", theme.btnFont, L"", 386, 190, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_bootLblCIOSrevM = _addPicButton("BOOT/CIOS_REV_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 190, 56, 56);
	m_bootLblCIOSrevP = _addPicButton("BOOT/CIOS_REV_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);

	_setHideAnim(m_bootLblTitle, "BOOT/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_bootLblLoadCIOS, "BOOT/LOAD_CIOS", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootBtnLoadCIOS, "BOOT/LOAD_CIOS_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_bootLblCIOSrev, "BOOT/CIOS_REV", -200, 0, 1.f, 0.f);
	_setHideAnim(m_bootLblCurCIOSrev, "BOOT/CIOS_REV_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_bootLblCIOSrevM, "BOOT/CIOS_REV_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_bootLblCIOSrevP, "BOOT/CIOS_REV_PLUS", 200, 0, 1.f, 0.f);

	hideBoot(true);
	_textBoot();
}
