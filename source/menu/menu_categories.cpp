#include "menu.hpp"

#include <string.h>
#include <gccore.h>

void CMenu::_CategorySettings()
{
	SetupInput();
	bool exitloop = false;
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
		if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_categoryBtnBack))
				break;
			for (int i = 0; i < 12; ++i)
			{
				if (m_btnMgr.selected(m_categoryBtn[i]))
				{
					// handling code for clicked favorite
					m_category = i;
					m_cat.setInt("GENERAL", "category", i);
					exitloop = true;
					break;
				}
			}
		}
		if (exitloop == true)
			break;
	}
	_hideCategorySettings();
}

void CMenu::_hideCategorySettings(bool instant)
{
	m_btnMgr.hide(m_categoryBtnBack,instant);
	for (int i = 0; i < 12; ++i)
		m_btnMgr.hide(m_categoryBtn[i],instant);

	for (u32 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
		if (m_categoryLblUser[i] != -1u)
			m_btnMgr.hide(m_categoryLblUser[i], instant);
}

void CMenu::_showCategorySettings(void)
{
	_setBg(m_categoryBg, m_categoryBg);
	
	m_btnMgr.show(m_categoryBtnBack);
	for (int i = 0; i < m_max_categories+1; ++i)
		m_btnMgr.show(m_categoryBtn[i]);
		
	for (u32 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
		if (m_categoryLblUser[i] != -1u)
			m_btnMgr.show(m_categoryLblUser[i]);
}


void CMenu::_initCategorySettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_categoryLblUser, ARRAY_SIZE(m_categoryLblUser), "CATEGORY");
	m_categoryBg = _texture(theme.texSet, "CATEGORY/BG", "texture", theme.bg);
	m_categoryBtnBack = _addButton(theme, "CATEGORY/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);
	m_categoryBtn[0] = _addButton(theme, "CATEGORY/ALL_BTN", theme.btnFont, L"",  60, 40, 200, 50, theme.btnFontColor);
	m_categoryBtn[1] = _addButton(theme, "CATEGORY/1_BTN", theme.btnFont, L"", 340, 40, 200, 50, theme.btnFontColor);
	m_categoryBtn[2] = _addButton(theme, "CATEGORY/2_BTN", theme.btnFont, L"",  60, 100, 200, 50, theme.btnFontColor);
	m_categoryBtn[3] = _addButton(theme, "CATEGORY/3_BTN", theme.btnFont, L"", 340, 100, 200, 50, theme.btnFontColor);
	m_categoryBtn[4] = _addButton(theme, "CATEGORY/4_BTN", theme.btnFont, L"",  60, 160, 200, 50, theme.btnFontColor);
	m_categoryBtn[5] = _addButton(theme, "CATEGORY/5_BTN", theme.btnFont, L"", 340, 160, 200, 50, theme.btnFontColor);
	m_categoryBtn[6] = _addButton(theme, "CATEGORY/6_BTN", theme.btnFont, L"",  60, 220, 200, 50, theme.btnFontColor);
	m_categoryBtn[7] = _addButton(theme, "CATEGORY/7_BTN", theme.btnFont, L"", 340, 220, 200, 50, theme.btnFontColor);
	m_categoryBtn[8] = _addButton(theme, "CATEGORY/8_BTN", theme.btnFont, L"",  60, 280, 200, 50, theme.btnFontColor);
	m_categoryBtn[9] = _addButton(theme, "CATEGORY/9_BTN", theme.btnFont, L"", 340, 280, 200, 50, theme.btnFontColor);
	m_categoryBtn[10] = _addButton(theme, "CATEGORY/10_BTN", theme.btnFont, L"", 60, 340, 200, 50, theme.btnFontColor);
	m_categoryBtn[11] = _addButton(theme, "CATEGORY/11_BTN", theme.btnFont, L"",340, 340, 200, 50, theme.btnFontColor);

	_setHideAnim(m_categoryBtnBack, "CATEGORY/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_categoryBtn[0], "CATEGORY/ALL_BTN", 0, 0, 0.f, 0.f);
	for (int i = 1; i < 12; ++i)
		_setHideAnim(m_categoryBtn[i], sfmt("CATEGORY/%i_BTN", i).c_str(), 0, 0, 0.f, 0.f);
	
	_hideCategorySettings(true);
	_textCategorySettings();
}

void CMenu::_textCategorySettings(void)
{
	m_btnMgr.setText(m_categoryBtnBack, _t("cd1", L"Back"));
	m_btnMgr.setText(m_categoryBtn[0], _t("dl3", L"All"));
	for (int i = 1; i < 12; i++)
	m_btnMgr.setText(m_categoryBtn[i], m_cat.getWString("GENERAL", sfmt("cat%d",i).c_str(), wfmt(L"Category %i",i).c_str()));
}

