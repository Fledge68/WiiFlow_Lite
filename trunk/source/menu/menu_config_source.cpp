/****************************************************************************
 * Copyright (C) 2012 Fledge68
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

s16 m_cfgsrcLblTitle;
s16 m_cfgsrcBtnBack;
s16 m_cfgsrcLblPage;
s16 m_cfgsrcBtnPageM;
s16 m_cfgsrcBtnPageP;
s16 m_cfgsrcLblUser[4];

s16 m_cfgsrcLblEnableSF;
s16 m_cfgsrcBtnEnableSF;

s16 m_cfgsrcLblSmallbox;
s16 m_cfgsrcBtnSmallbox;

s16 m_cfgsrcLblClearSF;
s16 m_cfgsrcBtnClearSF;

s16 m_cfgsrcLblMultisource;
s16 m_cfgsrcBtnMultisource;

u8 cfgsrc_curPage = 1;
u8 cfgsrc_Pages = 1;

static void showCfgSrc(void)
{
	m_btnMgr.show(m_cfgsrcLblTitle);
	m_btnMgr.show(m_cfgsrcBtnBack);
	m_btnMgr.show(m_cfgsrcLblPage);
	m_btnMgr.show(m_cfgsrcBtnPageM);
	m_btnMgr.show(m_cfgsrcBtnPageP);
	for(u8 i = 0; i < ARRAY_SIZE(m_cfgsrcLblUser); ++i)
		if(m_cfgsrcLblUser[i] != -1)
			m_btnMgr.show(m_cfgsrcLblUser[i]);
}

static void hideCfgSrc(bool instant, bool common)
{
	if(common)
	{
		m_btnMgr.hide(m_cfgsrcLblTitle, instant);
		m_btnMgr.hide(m_cfgsrcBtnBack, instant);
		m_btnMgr.hide(m_cfgsrcLblPage, instant);
		m_btnMgr.hide(m_cfgsrcBtnPageM, instant);
		m_btnMgr.hide(m_cfgsrcBtnPageP, instant);
		for(u8 i = 0; i < ARRAY_SIZE(m_cfgsrcLblUser); ++i)
			if(m_cfgsrcLblUser[i] != -1)
				m_btnMgr.hide(m_cfgsrcLblUser[i], instant);
	}

	m_btnMgr.hide(m_cfgsrcLblEnableSF, instant);
	m_btnMgr.hide(m_cfgsrcBtnEnableSF, instant);
	
	m_btnMgr.hide(m_cfgsrcLblSmallbox, instant);
	m_btnMgr.hide(m_cfgsrcBtnSmallbox, instant);

	m_btnMgr.hide(m_cfgsrcLblClearSF, instant);
	m_btnMgr.hide(m_cfgsrcBtnClearSF, instant);
	
	m_btnMgr.hide(m_cfgsrcLblMultisource, instant);
	m_btnMgr.hide(m_cfgsrcBtnMultisource, instant);
}

void CMenu::_CfgSrc(void)
{
	cfgsrc_curPage = 1;
	SetupInput();
	showCfgSrc();
	_refreshCfgSrc();

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) || (BTN_A_PRESSED && m_btnMgr.selected(m_cfgsrcBtnPageM)))
		{
			cfgsrc_curPage--;
			if(cfgsrc_curPage == 0) cfgsrc_curPage = cfgsrc_Pages;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_cfgsrcBtnPageM);
			_refreshCfgSrc();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED)) || (BTN_A_PRESSED && m_btnMgr.selected(m_cfgsrcBtnPageP)))
		{
			cfgsrc_curPage++;
			if(cfgsrc_curPage > cfgsrc_Pages) cfgsrc_curPage = 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_cfgsrcBtnPageP);
			_refreshCfgSrc();
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_cfgsrcBtnBack))
				break;
			else if (m_btnMgr.selected(m_cfgsrcBtnEnableSF))
			{
				m_cfg.setBool("SOURCEFLOW", "enabled", !m_cfg.getBool("SOURCEFLOW", "enabled", false));
				m_btnMgr.setText(m_cfgsrcBtnEnableSF, m_cfg.getBool("SOURCEFLOW", "enabled") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnSmallbox) && m_sourceflow)
			{
				m_load_view = true;
				fsop_deleteFolder(fmt("%s/sourceflow", m_cacheDir.c_str()));
				m_cfg.setBool("SOURCEFLOW", "smallbox", !m_cfg.getBool("SOURCEFLOW", "smallbox", false));
				m_btnMgr.setText(m_cfgsrcBtnSmallbox, m_cfg.getBool("SOURCEFLOW", "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnClearSF) && m_sourceflow)
			{
				m_cfg.setBool("SOURCEFLOW", "update_cache", true);
				m_load_view = true;
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnMultisource) && !m_sourceflow)
			{
				m_cfg.setBool("GENERAL", "multisource", !m_cfg.getBool("GENERAL", "multisource", false));
				m_btnMgr.setText(m_cfgsrcBtnMultisource, m_cfg.getBool("GENERAL", "multisource") ? _t("on", L"On") : _t("off", L"Off"));
				m_multisource = m_cfg.getBool("GENERAL", "multisource", false);
			}
		}
	}
	hideCfgSrc(false, true);
}

void CMenu::_refreshCfgSrc()
{
	hideCfgSrc(true, false);
	m_btnMgr.setText(m_cfgsrcLblPage, wfmt(L"%i / %i", cfgsrc_curPage, cfgsrc_Pages));
	if(cfgsrc_curPage == 1)
	{
		m_btnMgr.setText(m_cfgsrcBtnEnableSF, m_cfg.getBool("SOURCEFLOW", "enabled") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_cfgsrcBtnSmallbox, m_cfg.getBool("SOURCEFLOW", "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_cfgsrcBtnMultisource, m_cfg.getBool("GENERAL", "multisource") ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_cfgsrcLblEnableSF);
		m_btnMgr.show(m_cfgsrcBtnEnableSF);
		
		m_btnMgr.show(m_cfgsrcLblSmallbox);
		m_btnMgr.show(m_cfgsrcBtnSmallbox);

		m_btnMgr.show(m_cfgsrcLblClearSF);
		m_btnMgr.show(m_cfgsrcBtnClearSF);
		
		m_btnMgr.show(m_cfgsrcLblMultisource);
		m_btnMgr.show(m_cfgsrcBtnMultisource);
	}
}

void CMenu::_textCfgSrc(void)
{
	m_btnMgr.setText(m_cfgsrcLblTitle, _t("cfgsm1", L"Source Menu Settings"));
	m_btnMgr.setText(m_cfgsrcLblEnableSF, _t("cfgsm3", L"Enable Sourceflow"));
	m_btnMgr.setText(m_cfgsrcLblSmallbox, _t("cfgsm4", L"Sourceflow Smallbox"));
	m_btnMgr.setText(m_cfgsrcLblClearSF, _t("cfgsm5", L"Clear Sourceflow Cache"));
	m_btnMgr.setText(m_cfgsrcBtnClearSF, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_cfgsrcLblMultisource, _t("cfgbt6", L"Enable Multisource Features"));
	m_btnMgr.setText(m_cfgsrcBtnBack, _t("cfg10", L"Back"));
}


void CMenu::_initCfgSrc(void)
{
	_addUserLabels(m_cfgsrcLblUser, ARRAY_SIZE(m_cfgsrcLblUser), "CFG_SRC");
	m_cfgsrcLblTitle = _addTitle("CFG_SRC/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnBack = _addButton("CFG_SRC/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_cfgsrcLblPage = _addLabel("CFG_SRC/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_cfgsrcBtnPageM = _addPicButton("CFG_SRC/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_cfgsrcBtnPageP = _addPicButton("CFG_SRC/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	
	m_cfgsrcLblEnableSF = _addLabel("CFG_SRC/ENABLE_SF", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnEnableSF = _addButton("CFG_SRC/ENABLE_SF_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);

	m_cfgsrcLblSmallbox = _addLabel("CFG_SRC/SF_SMALLBOX", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnSmallbox = _addButton("CFG_SRC/SF_SMALLBOX_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);

	m_cfgsrcLblClearSF = _addLabel("CFG_SRC/CLEAR_SF", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnClearSF = _addButton("CFG_SRC/CLEAR_SF_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	
	m_cfgsrcLblMultisource = _addLabel("CFG_SRC/MULTISOURCE", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnMultisource = _addButton("CFG_SRC/MULTISOURCE_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);
	
	_setHideAnim(m_cfgsrcLblTitle, "CFG_SRC/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnBack, "CFG_SRC/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cfgsrcLblPage, "CFG_SRC/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cfgsrcBtnPageM, "CFG_SRC/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cfgsrcBtnPageP, "CFG_SRC/PAGE_PLUS", 0, 0, 1.f, -1.f);
	
	_setHideAnim(m_cfgsrcLblEnableSF, "CFG_SRC/ENABLE_SF", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnEnableSF, "CFG_SRC/ENABLE_SF_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_cfgsrcLblSmallbox, "CFG_SRC/SF_SMALLBOX", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnSmallbox, "CFG_SRC/SF_SMALLBOX_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_cfgsrcLblClearSF, "CFG_SRC/CLEAR_SF", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnClearSF, "CFG_SRC/CLEAR_SF_BTN", -50, 0, 1.f, 0.f);

	_setHideAnim(m_cfgsrcLblMultisource, "CFG_SRC/MULTISOURCE", -50, 0, -2.f, 0.f);
	_setHideAnim(m_cfgsrcBtnMultisource, "CFG_SRC/MULTISOURCE_BTN", -50, 0, 1.f, 0.f);

	hideCfgSrc(true, true);
	_textCfgSrc();
}
