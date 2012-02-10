
#include "menu.hpp"
#include "gecko.h"

extern const u8 error_png[];

void CMenu::error(const wstringEx &msg)
{
	SetupInput();
	_hideAbout();
	_hideCode();
	_hideConfig();
	_hideConfigAdv();
	_hideConfig3();
	_hideConfig4();
	_hideConfigScreen();
	_hideConfigSnd();
	_hideDownload();
	_hideGame();
	_hideMain();
	_hideWBFS();
	_hideCFTheme();
	_hideCategorySettings();
	_hideSystem();
	_hideGameInfo();
	_hideCheatDownload();
	_hideGameSettings();
	_hideWaitMessage();
	m_btnMgr.setText(m_errorLblMessage, msg, true);
	_showError();
	do
	{
		gprintf(msg.toUTF8().c_str());
		_mainLoopCommon();
	} while (!BTN_B_PRESSED && !BTN_A_PRESSED && !BTN_HOME_PRESSED);
	_hideError(false);
}

void CMenu::_hideError(bool instant)
{
	m_btnMgr.hide(m_errorLblIcon, instant);
	m_btnMgr.hide(m_errorLblMessage, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_errorLblUser); ++i)
		if (m_errorLblUser[i] != -1u)
			m_btnMgr.hide(m_errorLblUser[i], instant);
}

void CMenu::_showError(void)
{
	_setBg(m_errorBg, m_errorBg);
	m_btnMgr.show(m_errorLblMessage);
	m_btnMgr.show(m_errorLblIcon);
	for (u32 i = 0; i < ARRAY_SIZE(m_errorLblUser); ++i)
		if (m_errorLblUser[i] != -1u)
			m_btnMgr.show(m_errorLblUser[i]);
}

void CMenu::_initErrorMenu(CMenu::SThemeData &theme)
{
	STexture texIcon;

	texIcon.fromPNG(error_png);
	_addUserLabels(theme, m_errorLblUser, ARRAY_SIZE(m_errorLblUser), "ERROR");
	m_errorBg = _texture(theme.texSet, "ERROR/BG", "texture", theme.bg);
	m_errorLblMessage = _addLabel(theme, "ERROR/MESSAGE", theme.lblFont, L"", 112, 20, 500, 440, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_errorLblIcon = _addLabel(theme, "ERROR/ICON", theme.lblFont, L"", 40, 200, 64, 64, theme.lblFontColor, 0, texIcon);
	// 
	_setHideAnim(m_errorLblMessage, "ERROR/MESSAGE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_errorLblIcon, "ERROR/ICON", -50, 0, 0.f, 0.f);
	// 
	_hideError(true);
	_textError();
}

void CMenu::_textError(void)
{
}
