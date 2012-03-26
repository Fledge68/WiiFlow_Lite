#include "menu.hpp"

#include <string.h>
#include <gccore.h>

u8 m_categories[20];
u32 C_curPage;
bool gameSet;

void CMenu::_hideCategorySettings(bool instant)
{
	m_btnMgr.hide(m_categoryLblTitle, instant);
	m_btnMgr.hide(m_categoryBtnBack, instant);
	m_btnMgr.hide(m_categoryLblPage, instant);
	m_btnMgr.hide(m_categoryBtnPageM, instant);
	m_btnMgr.hide(m_categoryBtnPageP, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
		if (m_categoryLblUser[i] != -1u) m_btnMgr.hide(m_categoryLblUser[i], instant);
	
	for (int i=0; i<20; ++i)
	{
		m_btnMgr.hide(m_categoryLblCat[i]);
		m_btnMgr.hide(m_categoryBtnCat[i]);
		m_btnMgr.hide(m_categoryBtnCats[i]);
	}
}

void CMenu::_showCategorySettings(void)
{
	_setBg(m_categoryBg, m_categoryBg);
	m_btnMgr.show(m_categoryLblTitle);
	m_btnMgr.show(m_categoryBtnBack);
	m_btnMgr.setText(m_categoryLblPage, wfmt(L"%i / 2", C_curPage));
	m_btnMgr.show(m_categoryLblPage);
	m_btnMgr.show(m_categoryBtnPageM);
	m_btnMgr.show(m_categoryBtnPageP);
	
	for (u32 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i) if (m_categoryLblUser[i] != -1u) m_btnMgr.show(m_categoryLblUser[i]);
	string id(m_cf.getId());
	const char *catflags;
	if (gameSet) catflags = m_cat.getString("CATEGORIES", id, "").c_str();
	else catflags = m_cat.getString("GENERAL", "categories", "10000000000000000000").c_str();
	memset(&m_categories, '0', sizeof(m_categories));
	if (strlen(catflags) == sizeof(m_categories)) memcpy(&m_categories, catflags, sizeof(m_categories));

	if (C_curPage == 1)
	{
		for (int i = 0; i < 10; ++i)
		{
			if (i == 0 && gameSet) continue;
			m_btnMgr.show(m_categoryLblCat[i]);
			if (catflags[i] == '1') m_btnMgr.show(m_categoryBtnCats[i]);
			else m_btnMgr.show(m_categoryBtnCat[i]);
			m_btnMgr.hide(m_categoryLblCat[i+10]);
			m_btnMgr.hide(m_categoryBtnCat[i+10]);		
			m_btnMgr.hide(m_categoryBtnCats[i+10]);	
		}
	}
	else
	{
		for (int i = 10; i < 20; ++i)
		{
			m_btnMgr.show(m_categoryLblCat[i]);
			if (catflags[i] == '1') m_btnMgr.show(m_categoryBtnCats[i]);
			else m_btnMgr.show(m_categoryBtnCat[i]);
			m_btnMgr.hide(m_categoryLblCat[i-10]);
			m_btnMgr.hide(m_categoryBtnCat[i-10]);		
			m_btnMgr.hide(m_categoryBtnCats[i-10]);		
		}
	}
}

void CMenu::_CategorySettings(bool fromGameSet)
{
	SetupInput();
	C_curPage = 1;
	gameSet = false;
	if (fromGameSet) gameSet=true;
	_showCategorySettings();
	while (true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if (BTN_MINUS_PRESSED || BTN_LEFT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnPageM)))
		{
			C_curPage = C_curPage == 1 ? 2 : 1;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_categoryBtnPageM);
			_hideCategorySettings();
			_showCategorySettings();
		}
		else if (BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnPageP)))
		{
			C_curPage = C_curPage == 1 ? 2 : 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_categoryBtnPageP);
			_hideCategorySettings();
			_showCategorySettings();
		}
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_categoryBtnBack))
				break;
			for (int i = 0; i < 20; ++i)
			{
				if (m_btnMgr.selected(m_categoryBtnCat[i]) || m_btnMgr.selected(m_categoryBtnCats[i]))
				{
					m_categories[i] = m_categories[i] == '1' ? '0' : '1';
					if (i == 0 && m_categories[i] == '1') for (int j=1; j<20; ++j) m_categories[j] = '0';
					else m_categories[0] = '0';
					char catflags[21];
					memset(&catflags, 0, sizeof(catflags));
					memcpy(&catflags, &m_categories, sizeof(m_categories));
					string id(m_cf.getId());
					if (gameSet) m_cat.setString("CATEGORIES", id, catflags);
					else m_cat.setString("GENERAL", "categories", catflags);
					_hideCategorySettings();
					_showCategorySettings();
					break;
				}
			}
		}
	}
	_hideCategorySettings();
}

void CMenu::_initCategorySettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_categoryLblUser, ARRAY_SIZE(m_categoryLblUser), "CATEGORY");
	m_categoryBg = _texture(theme.texSet, "CATEGORY/BG", "texture", theme.bg);
	m_categoryLblTitle = _addTitle(theme, "CATEGORY/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_categoryBtnBack = _addButton(theme, "CATEGORY/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_categoryLblPage = _addLabel(theme, "CATEGORY/PAGE_BTN", theme.btnFont, L"", 76, 400, 100, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_categoryBtnPageM = _addPicButton(theme, "CATEGORY/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_categoryBtnPageP = _addPicButton(theme, "CATEGORY/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 176, 400, 56, 56);
	
	for (int i = 0; i < 5; i++)
	{ // Page 1
	m_categoryLblCat[i] = _addLabel(theme, sfmt("CATEGORY/CAT_%i", i).c_str(), theme.lblFont, L"", 125, (100+i*58), 185, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_categoryBtnCat[i] = _addButton(theme, sfmt("CATEGORY/CAT_%i_BTN", i).c_str(), theme.btnFont, L"", 20, (100+i*58), 96, 48, theme.btnFontColor);
	m_categoryBtnCats[i] = _addButton(theme, sfmt("CATEGORY/CAT_%i_BTNS", i).c_str(), theme.btnFont, L"", 20, (100+i*58), 96, 48, theme.selsbtnFontColor);
	// right half
	m_categoryLblCat[i+5] = _addLabel(theme, sfmt("CATEGORY/CAT_%i", i+5).c_str(), theme.txtFont, L"", 425, (100+i*58), 185, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_categoryBtnCat[i+5] = _addButton(theme, sfmt("CATEGORY/CAT_%i_BTN", i+5).c_str(), theme.btnFont, L"", 320, (100+i*58), 96, 48, theme.btnFontColor);
	m_categoryBtnCats[i+5] = _addButton(theme, sfmt("CATEGORY/CAT_%i_BTNS", i+5).c_str(), theme.btnFont, L"", 320, (100+i*58), 96, 48, theme.selsbtnFontColor);
	// Page 2
	m_categoryLblCat[i+10] = _addLabel(theme, sfmt("CATEGORY/CAT_%i", i+10).c_str(), theme.lblFont, L"", 125, (100+i*58), 185, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_categoryBtnCat[i+10] = _addButton(theme, sfmt("CATEGORY/CAT_%i_BTN", i+10).c_str(), theme.btnFont, L"", 20, (100+i*58), 96, 48, theme.btnFontColor);
	m_categoryBtnCats[i+10] = _addButton(theme, sfmt("CATEGORY/CAT_%i_BTNS", i+10).c_str(), theme.btnFont, L"", 20, (100+i*58), 96, 48, theme.selsbtnFontColor);
	// right half
	m_categoryLblCat[i+15] = _addLabel(theme, sfmt("CATEGORY/CAT_%i", i+15).c_str(), theme.txtFont, L"", 425, (100+i*58), 185, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_categoryBtnCat[i+15] = _addButton(theme, sfmt("CATEGORY/CAT_%i_BTN", i+15).c_str(), theme.btnFont, L"", 320, (100+i*58), 96, 48, theme.btnFontColor);
	m_categoryBtnCats[i+15] = _addButton(theme, sfmt("CATEGORY/CAT_%i_BTNS", i+15).c_str(), theme.btnFont, L"", 320, (100+i*58), 96, 48, theme.selsbtnFontColor);	
	}
	_setHideAnim(m_categoryLblTitle, "CATEGORY/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_categoryLblPage, "CATEGORY/PAGE_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_categoryBtnPageM, "CATEGORY/PAGE_MINUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_categoryBtnPageP, "CATEGORY/PAGE_PLUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_categoryBtnBack, "CATEGORY/BACK_BTN", 0, 200, 1.f, 0.f);
	for (int i = 0; i < 20; ++i) {
		_setHideAnim(m_categoryBtnCat[i], sfmt("CATEGORY/CAT_%i_BTN", i).c_str(), 0, 0, 1.f, 0.f);
		_setHideAnim(m_categoryBtnCats[i], sfmt("CATEGORY/CAT_%i_BTNS", i).c_str(), 0, 0, 1.f, 0.f);
		_setHideAnim(m_categoryLblCat[i], sfmt("CATEGORY/CAT_%i", i).c_str(), 0, 0, 1.f, 0.f);
	}
	_hideCategorySettings(true);
	_textCategorySettings();
}

void CMenu::_textCategorySettings(void)
{
	m_btnMgr.setText(m_categoryLblTitle, _t("cfgg17", L"Categories"));
	m_btnMgr.setText(m_categoryBtnBack, _t("cd1", L"Back"));
	for (int i=0; i<20; ++i) 
	{
		if (i == 0) m_btnMgr.setText(m_categoryLblCat[i], _t("dl3", L"All"));
		else m_btnMgr.setText(m_categoryLblCat[i], m_cat.getWString("GENERAL", fmt("cat%d",i), wfmt(L"Category %i",i).c_str()));
		m_btnMgr.setText(m_categoryBtnCat[i], _t("off", L"OFF"));
		m_btnMgr.setText(m_categoryBtnCats[i], _t("on", L"ON"));
	}
}

