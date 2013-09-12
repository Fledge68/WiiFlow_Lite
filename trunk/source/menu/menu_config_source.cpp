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

s16 m_cfgsrcLblEnableSM;
s16 m_cfgsrcBtnEnableSM;

s16 m_cfgsrcLblSourceOnBoot;
s16 m_cfgsrcBtnSourceOnBoot;

s16 m_cfgsrcLblEnableSF;
s16 m_cfgsrcBtnEnableSF;

s16 m_cfgsrcLblSmallbox;
s16 m_cfgsrcBtnSmallbox;

s16 m_cfgsrcLblClearSF;
s16 m_cfgsrcBtnClearSF;

s16 m_cfgsrcLblMultisource;
s16 m_cfgsrcBtnMultisource;

s16 m_cfgsrcLblBonMode;
s16 m_cfgsrcBtnBonMode;

u8 cfgsrc_curPage = 1;
u8 cfgsrc_Pages = 2;

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
	m_btnMgr.hide(m_cfgsrcLblEnableSM, instant);
	m_btnMgr.hide(m_cfgsrcBtnEnableSM, instant);

	m_btnMgr.hide(m_cfgsrcLblSourceOnBoot, instant);
	m_btnMgr.hide(m_cfgsrcBtnSourceOnBoot, instant);

	m_btnMgr.hide(m_cfgsrcLblEnableSF, instant);
	m_btnMgr.hide(m_cfgsrcBtnEnableSF, instant);
	
	m_btnMgr.hide(m_cfgsrcLblSmallbox, instant);
	m_btnMgr.hide(m_cfgsrcBtnSmallbox, instant);
	/* page 2 */
	m_btnMgr.hide(m_cfgsrcLblClearSF, instant);
	m_btnMgr.hide(m_cfgsrcBtnClearSF, instant);
	
	m_btnMgr.hide(m_cfgsrcLblMultisource, instant);
	m_btnMgr.hide(m_cfgsrcBtnMultisource, instant);
	
	m_btnMgr.hide(m_cfgsrcLblBonMode, instant);
	m_btnMgr.hide(m_cfgsrcBtnBonMode, instant);
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
			else if (m_btnMgr.selected(m_cfgsrcBtnEnableSM))
			{
				m_cfg.setBool("GENERAL", "use_source", !m_cfg.getBool("GENERAL", "use_source", false));
				m_btnMgr.setText(m_cfgsrcBtnEnableSM, m_cfg.getBool("GENERAL", "use_source", false) ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnSourceOnBoot))
			{
				m_cfg.setBool("GENERAL", "source_on_start", !m_cfg.getBool("GENERAL", "source_on_start", false));
				m_btnMgr.setText(m_cfgsrcBtnSourceOnBoot, m_cfg.getBool("GENERAL", "source_on_start") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnEnableSF))
			{
				m_cfg.setBool("SOURCEFLOW", "enabled", !m_cfg.getBool("SOURCEFLOW", "enabled", false));
				m_btnMgr.setText(m_cfgsrcBtnEnableSF, m_cfg.getBool("SOURCEFLOW", "enabled") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnSmallbox))
			{
				fsop_deleteFolder(fmt("%s/sourceflow", m_cacheDir.c_str()));
				m_cfg.setBool("SOURCEFLOW", "smallbox", !m_cfg.getBool("SOURCEFLOW", "smallbox", false));
				m_btnMgr.setText(m_cfgsrcBtnSmallbox, m_cfg.getBool("SOURCEFLOW", "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnClearSF))
			{
				fsop_deleteFolder(fmt("%s/sourceflow", m_cacheDir.c_str()));
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnMultisource))
			{
				m_cfg.setBool("GENERAL", "multisource", !m_cfg.getBool("GENERAL", "multisource", false));
				m_btnMgr.setText(m_cfgsrcBtnMultisource, m_cfg.getBool("GENERAL", "multisource") ? _t("on", L"On") : _t("off", L"Off"));
				m_multisource = m_cfg.getBool("GENERAL", "multisource", false);
			}
			else if (m_btnMgr.selected(m_cfgsrcBtnBonMode))
			{
				m_cfg.setBool("GENERAL", "b_on_mode_to_source", !m_cfg.getBool("GENERAL", "b_on_mode_to_source", false));
				m_btnMgr.setText(m_cfgsrcBtnBonMode, m_cfg.getBool("GENERAL", "b_on_mode_to_source") ? _t("on", L"On") : _t("off", L"Off"));
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
		m_btnMgr.setText(m_cfgsrcBtnEnableSM, m_cfg.getBool("GENERAL", "use_source") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_cfgsrcBtnSourceOnBoot, m_cfg.getBool("GENERAL", "source_on_start") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_cfgsrcBtnEnableSF, m_cfg.getBool("SOURCEFLOW", "enabled") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_cfgsrcBtnSmallbox, m_cfg.getBool("SOURCEFLOW", "smallbox") ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_cfgsrcLblEnableSM);
		m_btnMgr.show(m_cfgsrcBtnEnableSM);

		m_btnMgr.show(m_cfgsrcLblSourceOnBoot);
		m_btnMgr.show(m_cfgsrcBtnSourceOnBoot);

		m_btnMgr.show(m_cfgsrcLblEnableSF);
		m_btnMgr.show(m_cfgsrcBtnEnableSF);
		
		m_btnMgr.show(m_cfgsrcLblSmallbox);
		m_btnMgr.show(m_cfgsrcBtnSmallbox);
	}
	else
	{
		m_btnMgr.setText(m_cfgsrcBtnMultisource, m_cfg.getBool("GENERAL", "multisource") ? _t("on", L"On") : _t("off", L"Off"));
		m_btnMgr.setText(m_cfgsrcBtnBonMode, m_cfg.getBool("GENERAL", "b_on_mode_to_source") ? _t("on", L"On") : _t("off", L"Off"));
		
		m_btnMgr.show(m_cfgsrcLblClearSF);
		m_btnMgr.show(m_cfgsrcBtnClearSF);
		
		m_btnMgr.show(m_cfgsrcLblMultisource);
		m_btnMgr.show(m_cfgsrcBtnMultisource);
		
		m_btnMgr.show(m_cfgsrcLblBonMode);
		m_btnMgr.show(m_cfgsrcBtnBonMode);
	}
}

void CMenu::_textCfgSrc(void)
{
	m_btnMgr.setText(m_cfgsrcLblTitle, _t("cfgsm1", L"Source Menu Settings"));
	m_btnMgr.setText(m_cfgsrcLblEnableSM, _t("cfgsm2", L"Enable Source Menu"));
	m_btnMgr.setText(m_cfgsrcLblSourceOnBoot, _t("cfgbt5", L"Show source menu on boot"));
	m_btnMgr.setText(m_cfgsrcLblEnableSF, _t("cfgsm3", L"Enable Sourceflow"));
	m_btnMgr.setText(m_cfgsrcLblSmallbox, _t("cfgsm4", L"Sourceflow Smallbox"));
	m_btnMgr.setText(m_cfgsrcLblClearSF, _t("cfgsm5", L"Clear Sourceflow Cache"));
	m_btnMgr.setText(m_cfgsrcBtnClearSF, _t("cfgc5", L"Go"));
	m_btnMgr.setText(m_cfgsrcLblMultisource, _t("cfgbt6", L"Enable Multisource Features"));
	m_btnMgr.setText(m_cfgsrcLblBonMode, _t("cfgsm6", L"B On Mode To Source"));
	m_btnMgr.setText(m_cfgsrcBtnBack, _t("cfg10", L"Back"));
}


void CMenu::_initCfgSrc(void)
{
	_addUserLabels(m_cfgsrcLblUser, ARRAY_SIZE(m_cfgsrcLblUser), "CFG_SRC");
	m_cfgsrcLblTitle = _addTitle("CFG_SRC/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnBack = _addButton("CFG_SRC/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_cfgsrcLblPage = _addLabel("CFG_SRC/PAGE_BTN", theme.btnFont, L"", 76, 400, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_cfgsrcBtnPageM = _addPicButton("CFG_SRC/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_cfgsrcBtnPageP = _addPicButton("CFG_SRC/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 400, 56, 56);
	
	m_cfgsrcLblEnableSM = _addLabel("CFG_SRC/ENABLE_SM", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnEnableSM = _addButton("CFG_SRC/ENABLE_SM_BTN", theme.btnFont, L"", 370, 130, 230, 56, theme.btnFontColor);

	m_cfgsrcLblSourceOnBoot = _addLabel("CFG_SRC/SOURCE_ON_START", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnSourceOnBoot = _addButton("CFG_SRC/SOURCE_ON_START_BTN", theme.btnFont, L"", 370, 190, 230, 56, theme.btnFontColor);
	
	m_cfgsrcLblEnableSF = _addLabel("CFG_SRC/ENABLE_SF", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnEnableSF = _addButton("CFG_SRC/ENABLE_SF_BTN", theme.btnFont, L"", 370, 250, 230, 56, theme.btnFontColor);

	m_cfgsrcLblSmallbox = _addLabel("CFG_SRC/SF_SMALLBOX", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnSmallbox = _addButton("CFG_SRC/SF_SMALLBOX_BTN", theme.btnFont, L"", 370, 310, 230, 56, theme.btnFontColor);
	/* page 2 */
	m_cfgsrcLblClearSF = _addLabel("CFG_SRC/CLEAR_SF", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnClearSF = _addButton("CFG_SRC/CLEAR_SF_BTN", theme.btnFont, L"", 370, 130, 230, 56, theme.btnFontColor);
	
	m_cfgsrcLblMultisource = _addLabel("CFG_SRC/MULTISOURCE", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnMultisource = _addButton("CFG_SRC/MULTISOURCE_BTN", theme.btnFont, L"", 370, 190, 230, 56, theme.btnFontColor);
	
	m_cfgsrcLblBonMode = _addLabel("CFG_SRC/B_ON_MODE", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cfgsrcBtnBonMode = _addButton("CFG_SRC/B_ON_MODE_BTN", theme.btnFont, L"", 370, 250, 230, 56, theme.btnFontColor);
	
	_setHideAnim(m_cfgsrcLblTitle, "CFG_SRC/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_cfgsrcBtnBack, "CFG_SRC/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cfgsrcLblPage, "CFG_SRC/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cfgsrcBtnPageM, "CFG_SRC/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cfgsrcBtnPageP, "CFG_SRC/PAGE_PLUS", 0, 0, 1.f, -1.f);
	
	_setHideAnim(m_cfgsrcLblEnableSM, "CFG_SRC/ENABLE_SM", -200, 0, 1.f, 0.f);
	_setHideAnim(m_cfgsrcBtnEnableSM, "CFG_SRC/ENABLE_SM_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_cfgsrcLblSourceOnBoot, "CFG_SRC/SOURCE_ON_START", -200, 0, 1.f, 0.f);
	_setHideAnim(m_cfgsrcBtnSourceOnBoot, "CFG_SRC/SOURCE_ON_START_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_cfgsrcLblEnableSF, "CFG_SRC/ENABLE_SF", -200, 0, 1.f, 0.f);
	_setHideAnim(m_cfgsrcBtnEnableSF, "CFG_SRC/ENABLE_SF_BTN", 200, 0, 1.f, 0.f);
	
	_setHideAnim(m_cfgsrcLblSmallbox, "CFG_SRC/SF_SMALLBOX", -200, 0, 1.f, 0.f);
	_setHideAnim(m_cfgsrcBtnSmallbox, "CFG_SRC/SF_SMALLBOX_BTN", 200, 0, 1.f, 0.f);
	
	_setHideAnim(m_cfgsrcLblClearSF, "CFG_SRC/CLEAR_SF", -200, 0, 1.f, 0.f);
	_setHideAnim(m_cfgsrcBtnClearSF, "CFG_SRC/CLEAR_SF_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_cfgsrcLblMultisource, "CFG_SRC/MULTISOURCE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_cfgsrcBtnMultisource, "CFG_SRC/MULTISOURCE_BTN", 200, 0, 1.f, 0.f);

	_setHideAnim(m_cfgsrcLblBonMode, "CFG_SRC/B_ON_MODE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_cfgsrcBtnBonMode, "CFG_SRC/B_ON_MODE_BTN", 200, 0, 1.f, 0.f);

	hideCfgSrc(true, true);
	_textCfgSrc();
}
