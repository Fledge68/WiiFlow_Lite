
#include "menu.hpp"

s16 m_errorLblMessage;
s16 m_errorLblIcon;
s16 m_errorLblUser[4];

void CMenu::_hideError(bool instant)
{
	m_btnMgr.hide(m_errorLblIcon, instant);
	m_btnMgr.hide(m_errorLblMessage, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_errorLblUser); ++i)
		if(m_errorLblUser[i] != -1)
			m_btnMgr.hide(m_errorLblUser[i], instant);
}

void CMenu::_showError(void)
{
	_setBg(m_errorBg, m_errorBg);
	m_btnMgr.show(m_errorLblMessage);
	m_btnMgr.show(m_errorLblIcon);
	for(u8 i = 0; i < ARRAY_SIZE(m_errorLblUser); ++i)
		if(m_errorLblUser[i] != -1)
			m_btnMgr.show(m_errorLblUser[i]);
}

void CMenu::_error(const wstringEx &msg)
{
	SetupInput();
	_hideAbout();
	_hideCode();
	_hideConfigMain();
	_hideConfigGCGame();
	_hideDownload();
	_hideExitTo();
	_hideGame();
	_hideMain();
	_hideWBFS();
	_hideCFTheme();
	_hideCategorySettings();
	_hideGameInfo();
	_hideConfigGame();
	_hideWaitMessage();
	m_btnMgr.setText(m_errorLblMessage, msg, true);
	_showError();

	gprintf("error msg: %s\n", msg.toUTF8().c_str());
	do
	{
		_mainLoopCommon();
	} while (!m_exit && !BTN_B_PRESSED && !BTN_A_PRESSED && !BTN_HOME_PRESSED);
	_hideError(false);
}

void CMenu::_initErrorMenu()
{
	_addUserLabels(m_errorLblUser, ARRAY_SIZE(m_errorLblUser), "ERROR");
	m_errorBg = _texture("ERROR/BG", "texture", theme.bg, false);
	m_errorLblMessage = _addLabel("ERROR/MESSAGE", theme.lblFont, L"", 112, 20, 500, 440, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	TexData texIcon;
	TexHandle.fromImageFile(texIcon, fmt("%s/error.png", m_imgsDir.c_str()));
	m_errorLblIcon = _addLabel("ERROR/ICON", theme.lblFont, L"", 40, 200, 64, 64, theme.lblFontColor, 0, texIcon);
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
