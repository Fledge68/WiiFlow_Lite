
#include "menu.hpp"


extern const u8 delete_png[];
extern const u8 deletes_png[];

void CMenu::_hideCode(bool instant)
{
	for(u8 i = 0; i < 10; ++i)
		m_btnMgr.hide(m_codeBtnKey[i], instant);
	m_btnMgr.hide(m_codeBtnBack, instant);
	m_btnMgr.hide(m_codeBtnErase, instant);
	m_btnMgr.hide(m_codeBtnAge, instant);
	m_btnMgr.hide(m_codeLblTitle, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_codeLblUser); ++i)
		if(m_codeLblUser[i] != (u16)-1)
			m_btnMgr.hide(m_codeLblUser[i], instant);
	m_btnMgr.hide(m_codeLblAge, true);
}

void CMenu::_showCode(void)
{
	_setBg(m_codeBg, m_codeBg);
	for(u8 i = 0; i < 10; ++i)
		m_btnMgr.show(m_codeBtnKey[i]);
	m_btnMgr.show(m_codeBtnBack);
	m_btnMgr.show(m_codeLblTitle);
	for(u8 i = 0; i < ARRAY_SIZE(m_codeLblUser); ++i)
		if(m_codeLblUser[i] != (u16)-1)
			m_btnMgr.show(m_codeLblUser[i]);
	m_btnMgr.hide(m_codeLblAge, true);
}


void CMenu::_code(void)
{
	char code[4];
	_hideConfig();

	u32 n = 0;
	wchar_t codeLbl[] = L"_ _ _ _";

	SetupInput();
	memset(code, 0, sizeof code);
	m_btnMgr.setText(m_codeLblTitle, codeLbl);
	_showCode();
	bool ageLockMode = false;
	bool modeChanged = false;
	bool goBack = false;
	if (!m_locked)
	{
		m_btnMgr.show(m_codeBtnAge);
		m_btnMgr.show(m_codeBtnErase);
	}
	while (true)
	{
		int c = -1;
		_mainLoopCommon();
		if (BTN_HOME_PRESSED)
			goBack = true;

		if (WPadIR_ANY())
		{
			if (BTN_B_PRESSED)
				goBack = true;
			else if (BTN_UP_PRESSED)
				m_btnMgr.up();
			else if (BTN_DOWN_PRESSED)
				m_btnMgr.down();
			else if (BTN_A_PRESSED)
			{
				if (m_btnMgr.selected(m_codeBtnBack))
					goBack = true;
				else if (m_btnMgr.selected(m_codeBtnErase))
				{
					goBack = true;
					_cfNeedsUpdate();
					if (ageLockMode)
						m_cfg.remove("GENERAL", "age_lock");
					else
					{
						m_cfg.remove("GENERAL", "parent_code");
						m_locked = false;
					}
				}
				else if (m_btnMgr.selected(m_codeBtnAge))
					modeChanged = true;
				else
					for (int i = 0; i < 10; ++i)
						if (m_btnMgr.selected(m_codeBtnKey[i]))
						{
							c = i;
							break;
						}
			}
		}
		else if (!ageLockMode)
		{
			// Map buttons to numbers
			c = -1;
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
		}

		if  (goBack)
		{
			if (!ageLockMode)
				break;
			modeChanged = true;
			goBack = false;
		}
		// ageLockMode allows entry of numbers 2 - 19
		// a first digit of 0 is ignored
		// a first digit of 2 - 9 is taken to mean a single digit number
		// a first digit of 1 will be the start of a 2 digit number
		else if (c != -1 && !(ageLockMode && (n == 0 && c == 0)))
		{
			codeLbl[n * 2] = ageLockMode ? '0' + c : 'X';
			code[n++] = '0' + c;
			m_btnMgr.setText(m_codeLblTitle, codeLbl);
		}
		
		if (modeChanged)
		{
			modeChanged = false;
			memset(code, 0, sizeof code);
			n = 0;
			ageLockMode = !ageLockMode;
			
			if (ageLockMode)
			{
				int ageLockM = m_cfg.getInt("GENERAL", "age_lock");
				if (ageLockM < 2 || ageLockM > 19)
					ageLockM = 19;
				m_btnMgr.hide(m_codeBtnAge, true);
				wchar_t ageLbl[40];
				wcsncpy(ageLbl, (_t("cd3", L"Age Lock")).c_str(), 35);
				ageLbl[35] = 0;
				swprintf(ageLbl, 40, L"%ls: %d", ageLbl, ageLockM);
				m_btnMgr.setText(m_codeLblAge, ageLbl);
				m_btnMgr.show(m_codeLblAge);
			}
			else if (!m_locked)
			{
				m_btnMgr.show(m_codeBtnAge);
				m_btnMgr.hide(m_codeLblAge, true);
			}

			for (u32 i = 0; i < sizeof code; i++)
				codeLbl[i*2] = (ageLockMode && i > 1) ? ' ' : '_';
			m_btnMgr.setText(m_codeLblTitle, codeLbl);
		}
		else
		{
			if (ageLockMode)
			{
				if ((n >= 2) || (n == 1 && c > 1))
				{
					modeChanged = true;
					m_cfg.setString("GENERAL", "age_lock", string(code, 2).c_str());
					_cfNeedsUpdate();				}
			}
			else if (n >= sizeof code)
			{
				if (m_locked)
				{
					if (memcmp(code, m_cfg.getString("GENERAL", "parent_code").c_str(), 4) == 0)
					{
						m_locked = false;
						_cfNeedsUpdate();
					}
					else
						error(_t("cfgg25", L"Password incorrect."));
				}
				else
				{
					m_cfg.setString("GENERAL", "parent_code", string(code, 4).c_str());
					m_locked = true;
					_cfNeedsUpdate();
				}
				break;
			}
		}
	}
	_hideCode();
	_showConfig();
}

void CMenu::_initCodeMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_codeLblUser, ARRAY_SIZE(m_codeLblUser), "CODE");
	m_codeBg = _texture(theme.texSet, "CODE/BG", "texture", theme.bg);
	m_codeLblTitle = _addLabel(theme, "CODE/CODE", theme.titleFont, L"_ _ _ _", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_codeBtnKey[0] = _addButton(theme, "CODE/0_BTN", theme.btnFont, L"0", 270, 340, 210, 50, theme.btnFontColor);
	m_codeBtnErase = _addButton(theme, "CODE/ERASE_BTN", theme.btnFont, L"", 20, 400, 200, 56, theme.btnFontColor);
	m_codeBtnBack = _addButton(theme, "CODE/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_codeBtnAge = _addButton(theme, "CODE/AGE_BTN", theme.btnFont, L"", 20, 340, 200, 56, theme.btnFontColor);
	m_codeLblAge = _addTitle(theme, "CODE/AGE", theme.titleFont, L"", 20, 340, 200, 20, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	for (int i = 0; i < 10; ++i)
	{
		const char *codeText = fmt("CODE/%i_BTN", i);
		if (i > 0)
		{
			int x = i - 1;
			int y = x / 3;
			x %= 3;
			x = 160 + x * 110;
			y = 260 - y * 80;
			m_codeBtnKey[i] = _addButton(theme, codeText, theme.btnFont, wfmt(L"%i", i), x, y, 100, 50, theme.btnFontColor);
		}
		_setHideAnim(m_codeBtnKey[i], codeText, 0, 0, 0.f, 0.f);
	}
	_setHideAnim(m_codeBtnErase, "CODE/ERASE_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_codeBtnBack, "CODE/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_codeBtnAge, "CODE/AGE_BTN", 0, 0, -2.f, 0.f);

	_hideCode(true);
	_textCode();
}

void CMenu::_textCode(void)
{
	m_btnMgr.setText(m_codeBtnBack, _t("cd1", L"Back"));
	m_btnMgr.setText(m_codeBtnErase, _t("cd2", L"Erase"));
	m_btnMgr.setText(m_codeBtnAge, _t("cd3", L"Age Lock"));
//	m_btnMgr.setText(m_codeLblTitle, L"_ _ _ _");
}
