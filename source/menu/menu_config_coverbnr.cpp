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
#include "defines.h"

s16 m_coverbnrLblTitle;
s16 m_coverbnrBtnBack;

s16 m_coverbnrLblDlCover;
s16 m_coverbnrLblDeleteCover;
s16 m_coverbnrLblDlBanner;
s16 m_coverbnrLblDeleteBanner;

s16 m_coverbnrBtnDlCover;
s16 m_coverbnrBtnDeleteCover;
s16 m_coverbnrBtnDlBanner;
s16 m_coverbnrBtnDeleteBanner;

s16 m_coverbnrLblUser[4];
TexData m_coverbnrBg;

void CMenu::_hideCoverBanner(bool instant)
{
	m_btnMgr.hide(m_coverbnrLblTitle, instant);
	m_btnMgr.hide(m_coverbnrBtnBack, instant);
	m_btnMgr.hide(m_coverbnrLblDlCover, instant);
	m_btnMgr.hide(m_coverbnrLblDeleteCover, instant);
	m_btnMgr.hide(m_coverbnrLblDlBanner, instant);
	m_btnMgr.hide(m_coverbnrLblDeleteBanner, instant);

	m_btnMgr.hide(m_coverbnrBtnDlCover, instant);
	m_btnMgr.hide(m_coverbnrBtnDeleteCover, instant);
	m_btnMgr.hide(m_coverbnrBtnDlBanner, instant);
	m_btnMgr.hide(m_coverbnrBtnDeleteBanner, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_coverbnrLblUser); ++i)
		if(m_coverbnrLblUser[i] != -1)
			m_btnMgr.hide(m_coverbnrLblUser[i], instant);
}

void CMenu::_showCoverBanner(void)
{
	_setBg(m_coverbnrBg, m_coverbnrBg);
	m_btnMgr.show(m_coverbnrLblTitle);
	m_btnMgr.show(m_coverbnrBtnBack);
	
	m_btnMgr.show(m_coverbnrLblDlCover);
	m_btnMgr.show(m_coverbnrLblDeleteCover);
	m_btnMgr.show(m_coverbnrLblDlBanner);
	m_btnMgr.show(m_coverbnrLblDeleteBanner);

	m_btnMgr.show(m_coverbnrBtnDlCover);
	m_btnMgr.show(m_coverbnrBtnDeleteCover);
	m_btnMgr.show(m_coverbnrBtnDlBanner);
	m_btnMgr.show(m_coverbnrBtnDeleteBanner);

	for(u8 i = 0; i < ARRAY_SIZE(m_coverbnrLblUser); ++i)
		if(m_coverbnrLblUser[i] != -1)
			m_btnMgr.show(m_coverbnrLblUser[i]);
}

void CMenu::_CoverBanner(void)
{
	const char *id = CoverFlow.getId();
	SetupInput();
	_showCoverBanner();
	while(!m_exit)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_coverbnrBtnBack)))
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_coverbnrBtnDlCover))
			{
				_hideCoverBanner();
				_download(id);
				_showCoverBanner();
			}
			else if(m_btnMgr.selected(m_coverbnrBtnDeleteCover))
			{
				_hideCoverBanner();
				RemoveCover(id);
				_showCoverBanner();
			}
			else if(m_btnMgr.selected(m_coverbnrBtnDlBanner))
			{
				_hideCoverBanner();
				fsop_deleteFile(fmt("%s/%s.bnr", m_bnrCacheDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%s.bnr", m_customBnrDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%.3s.bnr", m_bnrCacheDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%.3s.bnr", m_customBnrDir.c_str(), id));
				_downloadBnr(id);
				_showCoverBanner();
			}
			else if(m_btnMgr.selected(m_coverbnrBtnDeleteBanner))
			{
				_hideCoverBanner();
				fsop_deleteFile(fmt("%s/%s.bnr", m_bnrCacheDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%s.bnr", m_customBnrDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%.3s.bnr", m_bnrCacheDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%.3s.bnr", m_customBnrDir.c_str(), id));
				_showCoverBanner();
			}
		}
	}
	_hideCoverBanner();
}

void CMenu::_initCoverBanner()
{
	m_coverbnrBg = _texture("COVERBNR/BG", "texture", theme.bg, false);
	_addUserLabels(m_coverbnrLblUser, ARRAY_SIZE(m_coverbnrLblUser), "COVERBNR");
	m_coverbnrLblTitle = _addTitle("COVERBNR/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_coverbnrBtnBack = _addButton("COVERBNR/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	m_coverbnrLblDlCover = _addLabel("COVERBNR/DLCOVER", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_coverbnrBtnDlCover = _addButton("COVERBNR/DLCOVER_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);
	m_coverbnrLblDeleteCover = _addLabel("COVERBNR/DELCOVER", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_coverbnrBtnDeleteCover = _addButton("COVERBNR/DELCOVER_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	m_coverbnrLblDlBanner = _addLabel("COVERBNR/DLBNR", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_coverbnrBtnDlBanner = _addButton("COVERBNR/DLBNR_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	m_coverbnrLblDeleteBanner = _addLabel("COVERBNR/DELBNR", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_coverbnrBtnDeleteBanner = _addButton("COVERBNR/DELBNR_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

	_setHideAnim(m_coverbnrLblTitle, "COVERBNR/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_coverbnrBtnBack, "COVERBNR/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_coverbnrLblDlCover, "COVERBNR/DLCOVER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_coverbnrBtnDlCover, "COVERBNR/DLCOVER_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_coverbnrLblDeleteCover, "COVERBNR/DELCOVER", 50, 0, -2.f, 0.f);
	_setHideAnim(m_coverbnrBtnDeleteCover, "COVERBNR/DELCOVER_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_coverbnrLblDlBanner, "COVERBNR/DLBNR", 50, 0, -2.f, 0.f);
	_setHideAnim(m_coverbnrBtnDlBanner, "COVERBNR/DLBNR_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_coverbnrLblDeleteBanner, "COVERBNR/DELBNR", 50, 0, -2.f, 0.f);
	_setHideAnim(m_coverbnrBtnDeleteBanner, "COVERBNR/DELBNR_BTN", -50, 0, 1.f, 0.f);
	_hideCoverBanner(true);
	_textCoverBanner();
}

void CMenu::_textCoverBanner(void)
{
	m_btnMgr.setText(m_coverbnrLblTitle, _t("cfgg40", L"Manage Cover and Banner"));
	m_btnMgr.setText(m_coverbnrLblDlCover, _t("cfgbnr1", L"Download Cover"));
	m_btnMgr.setText(m_coverbnrLblDeleteCover, _t("cfgbnr2", L"Delete Cover"));
	m_btnMgr.setText(m_coverbnrLblDlBanner, _t("cfgbnr3", L"Download Custom Banner"));
	m_btnMgr.setText(m_coverbnrLblDeleteBanner, _t("cfgbnr4", L"Delete Banner"));

	m_btnMgr.setText(m_coverbnrBtnDlCover, _t("cfgbnr5", L"Download"));
	m_btnMgr.setText(m_coverbnrBtnDeleteCover, _t("cfgbnr6", L"Delete"));
	m_btnMgr.setText(m_coverbnrBtnDlBanner, _t("cfgbnr5", L"Download"));
	m_btnMgr.setText(m_coverbnrBtnDeleteBanner, _t("cfgbnr6", L"Delete"));
	m_btnMgr.setText(m_coverbnrBtnBack, _t("cfg10", L"Back"));
}
