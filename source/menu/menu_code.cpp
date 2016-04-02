
#include "menu.hpp"

void CMenu::_hideCode(bool instant)
{
	for(u8 i = 0; i < 10; ++i)
		m_btnMgr.hide(m_codeBtnKey[i], instant);
	m_btnMgr.hide(m_codeBtnBack, instant);
	m_btnMgr.hide(m_codeBtnErase, instant);
	m_btnMgr.hide(m_codeLblTitle, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_codeLblUser); ++i)
		if(m_codeLblUser[i] != -1)
			m_btnMgr.hide(m_codeLblUser[i], instant);
}

void CMenu::_showCode(void)
{
	_setBg(m_codeBg, m_codeBg);
	for(u8 i = 0; i < 10; ++i)
		m_btnMgr.show(m_codeBtnKey[i]);
	m_btnMgr.show(m_codeBtnBack);
	m_btnMgr.show(m_codeLblTitle);
	for(u8 i = 0; i < ARRAY_SIZE(m_codeLblUser); ++i)
		if(m_codeLblUser[i] != -1)
			m_btnMgr.show(m_codeLblUser[i]);
}


bool CMenu::_code(char code[4], bool erase)
{
	u32 n = 0;
	wchar_t codeLbl[] = L"_ _ _ _";

	SetupInput();
	memset(code, 0, 4);
	m_btnMgr.setText(m_codeLblTitle, codeLbl);
	_showCode();
	if(erase)
		m_btnMgr.show(m_codeBtnErase);

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED)
			break;
		else if(WPadIR_ANY())
		{
			if(BTN_B_PRESSED)
				break;
			else if(BTN_UP_PRESSED)
				m_btnMgr.up();
			else if(BTN_DOWN_PRESSED)
				m_btnMgr.down();
			if(BTN_A_PRESSED)
			{
				if(!m_locked && m_btnMgr.selected(m_codeBtnErase))
				{
					memset(code, 0, 4);
					m_cfg.remove("GENERAL", "parent_code");
					n = 0;
					m_locked = false;
					break;
				}
				if(m_btnMgr.selected(m_codeBtnBack))
					break;
				else
					for(int i = 0; i < 10; ++i)
						if(m_btnMgr.selected(m_codeBtnKey[i]))
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
			if(BTN_UP_PRESSED) c = 0;
			else if(BTN_LEFT_PRESSED) c = 1;
			else if(BTN_RIGHT_PRESSED) c = 2;
			else if(BTN_DOWN_PRESSED) c = 3;
			else if(BTN_MINUS_PRESSED) c = 4;
			else if(BTN_PLUS_PRESSED) c = 5;
			else if(BTN_A_PRESSED) c = 6;
			else if(BTN_B_PRESSED) c = 7;
			else if(BTN_1_PRESSED) c = 8;
			else if(BTN_2_PRESSED) c = 9;
			
			if(c != -1)
			{
				codeLbl[n * 2] = 'X';
				code[n++] = '0' + c;
				m_btnMgr.setText(m_codeLblTitle, codeLbl);
			}
		}
	}
	_hideCode();
	return n == sizeof code;
}

void CMenu::_initCodeMenu()
{
	_addUserLabels(m_codeLblUser, ARRAY_SIZE(m_codeLblUser), "CODE");
	m_codeBg = _texture("CODE/BG", "texture", theme.bg, false);
	m_codeLblTitle = _addLabel("CODE/CODE", theme.titleFont, L"_ _ _ _", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_codeBtnKey[0] = _addButton("CODE/0_BTN", theme.btnFont, L"0", 270, 320, 100, 50, theme.btnFontColor);
	m_codeBtnErase = _addButton("CODE/ERASE_BTN", theme.btnFont, L"", 20, 400, 200, 48, theme.btnFontColor);
	m_codeBtnBack = _addButton("CODE/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	for(int i = 0; i < 10; ++i)
	{
		char *codeText = fmt_malloc("CODE/%i_BTN", i);
		if(codeText == NULL) continue;
		if(i > 0)
		{
			int x = i - 1;
			int y = x / 3;
			x %= 3;
			x = 160 + x * 110;
			y = 240 - y * 80;
			m_codeBtnKey[i] = _addButton(codeText, theme.btnFont, wfmt(L"%i", i), x, y, 100, 50, theme.btnFontColor);
		}
		_setHideAnim(m_codeBtnKey[i], codeText, 0, 0, 0.f, 0.f);
		MEM2_free(codeText);
	}
	_setHideAnim(m_codeBtnErase, "CODE/ERASE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_codeBtnBack, "CODE/BACK_BTN", 0, 0, 1.f, -1.f);

	_hideCode(true);
	_textCode();
}

void CMenu::_textCode(void)
{
	m_btnMgr.setText(m_codeBtnBack, _t("cd1", L"Back"));
	m_btnMgr.setText(m_codeBtnErase, _t("cd2", L"Erase"));
	m_btnMgr.setText(m_codeLblTitle, L"_ _ _ _");
}
