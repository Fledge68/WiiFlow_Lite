
#include "menu.hpp"

s16 m_coverbnrLblTitle;
s16 m_coverbnrBtnBack;
s16 m_coverbnrLblUser[4];

TexData m_coverbnrBg;

void CMenu::_hideCoverBanner(bool instant)
{
	m_btnMgr.hide(m_coverbnrLblTitle, instant);
	m_btnMgr.hide(m_coverbnrBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_coverbnrLblUser); ++i)
		if(m_coverbnrLblUser[i] != -1)
			m_btnMgr.hide(m_coverbnrLblUser[i], instant);

	_hideConfigButtons(instant);
}

void CMenu::_showCoverBanner(void)
{
	_setBg(m_coverbnrBg, m_coverbnrBg);
	m_btnMgr.show(m_coverbnrLblTitle);
	m_btnMgr.show(m_coverbnrBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_coverbnrLblUser); ++i)
		if(m_coverbnrLblUser[i] != -1)
			m_btnMgr.show(m_coverbnrLblUser[i]);

	m_btnMgr.setText(m_configLbl1, _t("cfgbnr1", L"Download Cover"));
	m_btnMgr.setText(m_configLbl2, _t("cfgbnr2", L"Delete Cover"));
	m_btnMgr.setText(m_configLbl3, _t("cfgbnr3", L"Download Custom Banner"));
	m_btnMgr.setText(m_configLbl4, _t("cfgbnr4", L"Delete Banner"));

	m_btnMgr.setText(m_configBtn1, _t("cfgbnr5", L"Download"));
	m_btnMgr.setText(m_configBtn2, _t("cfgbnr6", L"Delete"));
	m_btnMgr.setText(m_configBtn3, _t("cfgbnr5", L"Download"));
	m_btnMgr.setText(m_configBtn4, _t("cfgbnr6", L"Delete"));
	
	m_btnMgr.show(m_configLbl1);
	m_btnMgr.show(m_configLbl2);
	m_btnMgr.show(m_configLbl3);
	m_btnMgr.show(m_configLbl4);

	m_btnMgr.show(m_configBtn1);
	m_btnMgr.show(m_configBtn2);
	m_btnMgr.show(m_configBtn3);
	m_btnMgr.show(m_configBtn4);
}

void CMenu::_CoverBanner(void)
{
	const char *id = CoverFlow.getId();
	SetupInput();
	_showCoverBanner();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_coverbnrBtnBack)))
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_configBtn1))
			{
				_hideCoverBanner();
				_download(id, 1);
				_showCoverBanner();
			}
			else if(m_btnMgr.selected(m_configBtn2))
			{
				_hideCoverBanner();
				RemoveCover(id);
				error(_t("deltcover", L"Cover is deleted."));
				_showCoverBanner();
			}
			else if(m_btnMgr.selected(m_configBtn3))
			{
				_hideCoverBanner();
				fsop_deleteFile(fmt("%s/%s.bnr", m_bnrCacheDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%s.bnr", m_customBnrDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%.3s.bnr", m_bnrCacheDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%.3s.bnr", m_customBnrDir.c_str(), id));
				_download(id, 2);
				m_newGame = true;
				_showCoverBanner();
			}
			else if(m_btnMgr.selected(m_configBtn4))
			{
				_hideCoverBanner();
				fsop_deleteFile(fmt("%s/%s.bnr", m_bnrCacheDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%s.bnr", m_customBnrDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%.3s.bnr", m_bnrCacheDir.c_str(), id));
				fsop_deleteFile(fmt("%s/%.3s.bnr", m_customBnrDir.c_str(), id));
				error(_t("deltbanner", L"Banner is deleted."));
				m_newGame = true;
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
	m_coverbnrLblTitle = _addLabel("COVERBNR/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_coverbnrBtnBack = _addButton("COVERBNR/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	_setHideAnim(m_coverbnrLblTitle, "COVERBNR/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_coverbnrBtnBack, "COVERBNR/BACK_BTN", 0, 0, 1.f, -1.f);

	_hideCoverBanner(true);
	_textCoverBanner();
}

void CMenu::_textCoverBanner(void)
{
	m_btnMgr.setText(m_coverbnrLblTitle, _t("cfgg40", L"Manage Cover and Banner"));
	m_btnMgr.setText(m_coverbnrBtnBack, _t("cfg10", L"Back"));
}
