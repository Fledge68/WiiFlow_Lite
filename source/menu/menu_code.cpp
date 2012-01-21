
#include "menu.hpp"


extern const u8 delete_png[];
extern const u8 deletes_png[];

void CMenu::_hideCode(bool instant)
{
	for (int i = 0; i < 10; ++i)
		m_btnMgr.hide(m_codeBtnKey[i], instant);
	m_btnMgr.hide(m_codeBtnBack, instant);
	m_btnMgr.hide(m_codeBtnErase, instant);
	m_btnMgr.hide(m_codeLblTitle, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_codeLblUser); ++i)
		if (m_codeLblUser[i] != -1u)
			m_btnMgr.hide(m_codeLblUser[i], instant);
}

void CMenu::_showCode(void)
{
	_setBg(m_codeBg, m_codeBg);
	for (int i = 0; i < 10; ++i)
		m_btnMgr.show(m_codeBtnKey[i]);
	m_btnMgr.show(m_codeBtnBack);
	m_btnMgr.show(m_codeLblTitle);
	for (u32 i = 0; i < ARRAY_SIZE(m_codeLblUser); ++i)
		if (m_codeLblUser[i] != -1u)
			m_btnMgr.show(m_codeLblUser[i]);
}

bool CMenu::_code(char code[4], bool erase)
{
	u32 n = 0;
	wchar_t codeLbl[] = L"_ _ _ _";

	SetupInput();
	memset(code, 0, sizeof code);
	m_btnMgr.setText(m_codeLblTitle, codeLbl);
	_showCode();
	if (erase)
		m_btnMgr.show(m_codeBtnErase);
	while (true)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED)
		{
			break;
		}
		else if (WPadIR_ANY())
		{
			if (BTN_B_PRESSED)
				break;
			else if (BTN_UP_PRESSED)
				m_btnMgr.up();
			else if (BTN_DOWN_PRESSED)
				m_btnMgr.down();
			if (BTN_A_PRESSED)
			{
				if (!m_locked && m_btnMgr.selected(m_codeBtnErase))
				{
					memset(code, 0, sizeof code);
					m_cfg.remove("GENERAL", "parent_code");
					n = 0;
					m_locked = false;
					break;
				}
				if (m_btnMgr.selected(m_codeBtnBack))
					break;
				else
					for (int i = 0; i < 10; ++i)
						if (m_btnMgr.selected(m_codeBtnKey[i]))
						{
							codeLbl[n * 2] = 'X';
							code[n++] = '0' + i;
							m_btnMgr.setText(m_codeLblTitle, codeLbl);
							break;
						}
			}
		}
		else
		{
			// Map buttons to numbers
			int c = -1;
			if (BTN_UP_PRESSED) c = 0;
			else if (BTN_LEFT_PRESSED) c = 1;
			else if (BTN_RIGHT_PRESSED) c = 2;
			else if (BTN_DOWN_PRESSED) c = 3;
			else if (BTN_MINUS_PRESSED) c = 4;
			else if (BTN_PLUS_PRESSED) c = 5;
			else if (BTN_A_PRESSED) c = 6;
			else if (BTN_B_PRESSED) c = 7;
			else if (BTN_1_PRESSED) c = 8;
			else if (BTN_2_PRESSED) c = 9;

			if (c != -1)
			{
				codeLbl[n * 2] = 'X';
				code[n++] = '0' + c;
				m_btnMgr.setText(m_codeLblTitle, codeLbl);
			}
		}
		if (n >= sizeof code)
			break;
	}
	_hideCode();
	return n == sizeof code;
}

void CMenu::_initCodeMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_codeLblUser, ARRAY_SIZE(m_codeLblUser), "CODE");
	m_codeBg = _texture(theme.texSet, "CODE/BG", "texture", theme.bg);
	m_codeLblTitle = _addLabel(theme, "CODE/CODE", theme.titleFont, L"_ _ _ _", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_codeBtnKey[7] = _addButton(theme, "CODE/7_BTN", theme.btnFont, L"7", 160, 100, 100, 50, theme.btnFontColor);
	m_codeBtnKey[8] = _addButton(theme, "CODE/8_BTN", theme.btnFont, L"8", 270, 100, 100, 50, theme.btnFontColor);
	m_codeBtnKey[9] = _addButton(theme, "CODE/9_BTN", theme.btnFont, L"9", 380, 100, 100, 50, theme.btnFontColor);
	m_codeBtnKey[4] = _addButton(theme, "CODE/4_BTN", theme.btnFont, L"4", 160, 180, 100, 50, theme.btnFontColor);
	m_codeBtnKey[5] = _addButton(theme, "CODE/5_BTN", theme.btnFont, L"5", 270, 180, 100, 50, theme.btnFontColor);
	m_codeBtnKey[6] = _addButton(theme, "CODE/6_BTN", theme.btnFont, L"6", 380, 180, 100, 50, theme.btnFontColor);
	m_codeBtnKey[1] = _addButton(theme, "CODE/1_BTN", theme.btnFont, L"1", 160, 260, 100, 50, theme.btnFontColor);
	m_codeBtnKey[2] = _addButton(theme, "CODE/2_BTN", theme.btnFont, L"2", 270, 260, 100, 50, theme.btnFontColor);
	m_codeBtnKey[3] = _addButton(theme, "CODE/3_BTN", theme.btnFont, L"3", 380, 260, 100, 50, theme.btnFontColor);
	m_codeBtnKey[0] = _addButton(theme, "CODE/0_BTN", theme.btnFont, L"0", 270, 340, 210, 50, theme.btnFontColor);
	m_codeBtnErase = _addButton(theme, "CODE/ERASE_BTN", theme.btnFont, L"", 20, 410, 200, 56, theme.btnFontColor);
	m_codeBtnBack = _addButton(theme, "CODE/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);
	// 
	for (int i = 0; i < 10; ++i)
		_setHideAnim(m_codeBtnKey[i], sfmt("CODE/%i_BTN", i).c_str(), 0, 0, 0.f, 0.f);
	_setHideAnim(m_codeBtnErase, "CODE/ERASE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_codeBtnBack, "CODE/BACK_BTN", 0, 0, -2.f, 0.f);
	// 
	_hideCode(true);
	_textCode();
}

void CMenu::_textCode(void)
{
	m_btnMgr.setText(m_codeBtnBack, _t("cd1", L"Back"));
	m_btnMgr.setText(m_codeBtnErase, _t("cd2", L"Erase"));
	m_btnMgr.setText(m_codeLblTitle, L"_ _ _ _");
}
