#include "menu.hpp"

#include <string.h>
#include <gccore.h>

u8 m_categories[21];
u32 C_curPage;
bool gameSet;
u8 lastBtn;

void CMenu::_hideCategorySettings(bool instant)
{
	m_btnMgr.hide(m_categoryLblTitle, instant);
	m_btnMgr.hide(m_categoryBtnBack, instant);
	m_btnMgr.hide(m_categoryLblPage, instant);
	m_btnMgr.hide(m_categoryBtnPageM, instant);
	m_btnMgr.hide(m_categoryBtnPageP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
	{
		if(m_categoryLblUser[i] != (u16)-1u)
			m_btnMgr.hide(m_categoryLblUser[i], instant);
	}

	for(int i = 0; i < 21; ++i)
	{
		m_btnMgr.hide(m_categoryLblCat[i]);
		m_btnMgr.hide(m_categoryBtnCat[i]);
		m_btnMgr.hide(m_categoryBtnCats[i]);
	}
}

void CMenu::_showCategorySettings(void)
{
	_setBg(m_categoryBg, m_categoryBg);
	for(u8 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
	{
		if(m_categoryLblUser[i] != (u16)-1)
			m_btnMgr.show(m_categoryLblUser[i]);
	}
	m_btnMgr.show(m_categoryLblTitle);
	m_btnMgr.show(m_categoryBtnBack);
	_updateCheckboxes();
}

void CMenu::_updateCheckboxes(void)
{
	if(m_max_categories > 10)
	{
		m_btnMgr.setText(m_categoryLblPage, wfmt(L"%i / 2", C_curPage));
		m_btnMgr.show(m_categoryLblPage);
		m_btnMgr.show(m_categoryBtnPageM);
		m_btnMgr.show(m_categoryBtnPageP);
	}
	for(u8 i = 0; i < 21; ++i)
	{
		m_btnMgr.hide(m_categoryBtnCat[i]);
		m_btnMgr.hide(m_categoryBtnCats[i]);
		m_btnMgr.hide(m_categoryLblCat[i]);
	}
	const char *catflags;
	if (gameSet)
	{
		string id;
		if(m_current_view != COVERFLOW_EMU)
			id = m_cf.getId();
		else
		{
			dir_discHdr *hdr = m_cf.getHdr();
			string tempname(hdr->path);
			tempname.erase(0, tempname.find_first_of('/')+1);
			string dirName = tempname.substr(0, tempname.find_first_of('/')+1);
			tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
			if(tempname.find_last_of('.') != string::npos)
			tempname.erase(tempname.find_last_of('.'), tempname.size() - tempname.find_last_of('.'));
			id = dirName+tempname;
		}
		catflags = m_cat.getString("CATEGORIES", id, "").c_str();
	}
	else
		catflags = m_cat.getString(_domainFromView(), "categories", "100000000000000000000").c_str();
	memset(&m_categories, '0', sizeof(m_categories));
	if(strlen(catflags) == sizeof(m_categories))
		memcpy(&m_categories, catflags, sizeof(m_categories));

	if(C_curPage == 1)
	{
		int j = 11;
		if(m_max_categories < 11)
			j = m_max_categories;
		for(int i = 0; i < j; ++i)
		{
			if(i == 0 && gameSet)
				continue;
			if(catflags[i] == '1')
				m_btnMgr.show(m_categoryBtnCats[i]);
			else
				m_btnMgr.show(m_categoryBtnCat[i]);
			m_btnMgr.show(m_categoryLblCat[i]);
		}
	}
	else
	{
		for(int i = 11; i < m_max_categories; ++i)
		{
			if(catflags[i] == '1')
				m_btnMgr.show(m_categoryBtnCats[i]);
			else
				m_btnMgr.show(m_categoryBtnCat[i]);
			m_btnMgr.show(m_categoryLblCat[i]);
		}
	}
}

void CMenu::_CategorySettings(bool fromGameSet)
{
	SetupInput();
	C_curPage = 1;
	gameSet = fromGameSet;
	_showCategorySettings();
	while(true)
	{
		_mainLoopCommon();
		if(!m_btnMgr.selected(lastBtn))
			m_btnMgr.noHover(false);
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			m_cat.save();
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) && m_max_categories>11) || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnPageM)))
		{
			lastBtn = m_categoryBtnPageM;
			m_btnMgr.noHover(true);
			C_curPage = C_curPage == 1 ? 2 : 1;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_categoryBtnPageM);
			_updateCheckboxes();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED) && m_max_categories>11) || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnPageP)))
		{
			lastBtn = m_categoryBtnPageP;
			m_btnMgr.noHover(true);
			C_curPage = C_curPage == 1 ? 2 : 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_categoryBtnPageP);
			_updateCheckboxes();
		}
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_categoryBtnBack))
			{
				m_cat.save();
				break;
			}
			for(int i = 0; i < 21; ++i)
			{
				if(m_btnMgr.selected(m_categoryBtnCat[i]) || m_btnMgr.selected(m_categoryBtnCats[i]))
				{
					lastBtn = m_categoryBtnCat[i];
					if(m_btnMgr.selected(m_categoryBtnCats[i]))
						lastBtn = m_categoryBtnCats[i];
					m_btnMgr.noHover(true);
					m_categories[i] = m_categories[i] == '1' ? '0' : '1';
					if(i == 0 && m_categories[i] == '1')
					{
						for(int j = 1; j < 21; ++j)
							m_categories[j] = '0';
					}
					else
						m_categories[0] = '0';
					char catflags[22];
					memset(&catflags, 0, sizeof(catflags));
					memcpy(&catflags, &m_categories, sizeof(m_categories));
					if(string (catflags) == "000000000000000000000")
						catflags[0] = '1';
					if (gameSet)
					{
						string id;
						if(m_current_view != COVERFLOW_EMU)
							id = m_cf.getId();
						else
						{
							dir_discHdr *hdr = m_cf.getHdr();
							string tempname(hdr->path);
							tempname.erase(0, tempname.find_first_of('/')+1);
							string dirName = tempname.substr(0, tempname.find_first_of('/')+1);
							tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
							if(tempname.find_last_of('.') != string::npos)
								tempname.erase(tempname.find_last_of('.'), tempname.size() - tempname.find_last_of('.'));
							id = dirName+tempname;
						}
					m_cat.setString("CATEGORIES", id, catflags);
					}
					else
						m_cat.setString(_domainFromView(), "categories", catflags);
					_updateCheckboxes();
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
	m_categoryLblPage = _addLabel(theme, "CATEGORY/PAGE_BTN", theme.btnFont, L"", 256, 400, 100, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_categoryBtnPageM = _addPicButton(theme, "CATEGORY/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 200, 400, 56, 56);
	m_categoryBtnPageP = _addPicButton(theme, "CATEGORY/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 356, 400, 56, 56);
	m_categoryBtnCat[0] = _addPicButton(theme, "CATEGORY/CAT_0_BTN", theme.checkboxoff, theme.checkboxoffs, 30, 390, 44, 48);
	m_categoryBtnCats[0] = _addPicButton(theme, "CATEGORY/CAT_0_BTNS", theme.checkboxon, theme.checkboxons, 30, 390, 44, 48);
	m_categoryLblCat[0] = _addLabel(theme, "CATEGORY/CAT_0", theme.lblFont, L"", 85, 390, 100, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	for(int i = 1; i < 6; ++i)
	{ 	// Page 1
		m_categoryBtnCat[i] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTN", i), theme.checkboxoff, theme.checkboxoffs, 30, (42+i*58), 44, 48);
		m_categoryBtnCats[i] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNS", i), theme.checkboxon, theme.checkboxons, 30, (42+i*58), 44, 48);
		m_categoryLblCat[i] = _addLabel(theme, fmt("CATEGORY/CAT_%i", i), theme.lblFont, L"", 85, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// right half
		m_categoryBtnCat[i+5] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTN", i+5), theme.checkboxoff, theme.checkboxoffs, 325, (42+i*58), 44, 48);
		m_categoryBtnCats[i+5] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNS", i+5), theme.checkboxon, theme.checkboxons, 325, (42+i*58), 44, 48);
		m_categoryLblCat[i+5] = _addLabel(theme, fmt("CATEGORY/CAT_%i", i+5), theme.txtFont, L"", 380, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// Page 2
		m_categoryBtnCat[i+10] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTN", i+10), theme.checkboxoff, theme.checkboxoffs, 30, (42+i*58), 44, 48);
		m_categoryBtnCats[i+10] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNS", i+10), theme.checkboxon, theme.checkboxons, 30, (42+i*58), 44, 48);
		m_categoryLblCat[i+10] = _addLabel(theme, fmt("CATEGORY/CAT_%i", i+10), theme.lblFont, L"", 85, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// right half
		m_categoryBtnCat[i+15] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTN", i+15), theme.checkboxoff, theme.checkboxoffs, 325, (42+i*58), 44, 48);
		m_categoryBtnCats[i+15] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNS", i+15), theme.checkboxon, theme.checkboxons, 325, (42+i*58), 44, 48);
		m_categoryLblCat[i+15] = _addLabel(theme, fmt("CATEGORY/CAT_%i", i+15), theme.txtFont, L"", 380, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	}
	_setHideAnim(m_categoryLblTitle, "CATEGORY/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_categoryLblPage, "CATEGORY/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnPageM, "CATEGORY/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnPageP, "CATEGORY/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnBack, "CATEGORY/BACK_BTN", 0, 0, 1.f, -1.f);
	for(int i = 0; i < 21; ++i)
	{
		_setHideAnim(m_categoryBtnCat[i], fmt("CATEGORY/CAT_%i_BTN", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_categoryBtnCats[i], fmt("CATEGORY/CAT_%i_BTNS", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_categoryLblCat[i], fmt("CATEGORY/CAT_%i", i), 0, 0, 1.f, 0.f);
	}
	_hideCategorySettings(true);
	_textCategorySettings();
}

void CMenu::_textCategorySettings(void)
{
	m_btnMgr.setText(m_categoryLblTitle, _t("cfgg17", L"Select Categories"));
	m_btnMgr.setText(m_categoryBtnBack, _t("cd1", L"Back"));
	for(int i = 0; i < 21; ++i)
	{
		if(i == 0)
			m_btnMgr.setText(m_categoryLblCat[i], _t("dl3", L"All"));
		else
			m_btnMgr.setText(m_categoryLblCat[i], m_cat.getWString("GENERAL", fmt("cat%d",i), wfmt(L"Category %i",i).c_str()));
	}
}
