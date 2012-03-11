
#include "menu.hpp"
#include "nand.hpp"
#include "svnrev.h"

#include "sys.h"
#include "alt_ios.h"
#include "defines.h"
#include "cios.hpp"

const int pixels_to_skip = 10;

void CMenu::_about(void)
{
	int amount_of_skips = 0;
	int thanks_x = 0, thanks_y = 0;
	u32 thanks_w = 0, thanks_h = 0;
	bool first = true;

	m_btnMgr.reset(m_aboutLblInfo, true);

	SetupInput();
	_showAbout();

	do
	{
		_mainLoopCommon();

		if (amount_of_skips == 0)
		{
			// Check dimensions in the loop, because the animation can have an effect
			m_btnMgr.getDimensions(m_aboutLblInfo, thanks_x, thanks_y, thanks_w, thanks_h); // Get original dimensions
		}	
		if(first)
		{
			m_btnMgr.moveBy(m_aboutLblInfo, 0, -(pixels_to_skip * 10));
			amount_of_skips++;
			first = false;
		}

		if ((BTN_DOWN_PRESSED || BTN_DOWN_HELD) && !(m_thrdWorking && m_thrdStop))
		{
			if (thanks_h - (amount_of_skips * pixels_to_skip) > (m_vid.height2D() - (35 + thanks_y)))
			{
				m_btnMgr.moveBy(m_aboutLblInfo, 0, -pixels_to_skip);
				amount_of_skips++;
			}
		}
		else if ((BTN_UP_PRESSED || BTN_UP_HELD) && !(m_thrdWorking && m_thrdStop))
		{
			if (amount_of_skips > 1)
			{
				m_btnMgr.moveBy(m_aboutLblInfo, 0, pixels_to_skip);
				amount_of_skips--;
			}
		}
		else if (BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if (BTN_A_PRESSED && !(m_thrdWorking && m_thrdStop))
		{
			if (!m_locked && m_btnMgr.selected(m_aboutBtnSystem))
			{
				// show system menu
				m_cf.stopCoverLoader(true);
				_hideAbout(false);
				_system();
				remove(m_ver.c_str());
				if(m_exit)
				{
					_launchHomebrew(m_dol.c_str(), m_homebrewArgs);
					break;
				}
				_showAbout();
				m_cf.startCoverLoader();
			}
		}
	} while (true);
	_hideAbout(false);
}

void CMenu::_hideAbout(bool instant)
{
	m_btnMgr.hide(m_aboutLblTitle, instant);
	m_btnMgr.hide(m_aboutLblIOS, instant);
	m_btnMgr.hide(m_aboutLblInfo, instant);
	m_btnMgr.hide(m_aboutBtnSystem, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_aboutLblUser); ++i)
		if (m_aboutLblUser[i] != -1u)
			m_btnMgr.hide(m_aboutLblUser[i], instant);
}

void CMenu::_showAbout(void)
{
	_setBg(m_aboutBg, m_aboutBg);
	m_btnMgr.show(m_aboutLblTitle);
	m_btnMgr.show(m_aboutLblIOS);
	m_btnMgr.show(m_aboutLblInfo,false,true);
	m_btnMgr.show(m_aboutBtnSystem);
	for (u32 i = 0; i < ARRAY_SIZE(m_aboutLblUser); ++i)
		if (m_aboutLblUser[i] != -1u)
			m_btnMgr.show(m_aboutLblUser[i]);
}

void CMenu::_initAboutMenu(CMenu::SThemeData &theme)
{
	STexture emptyTex;
	_addUserLabels(theme, m_aboutLblUser, ARRAY_SIZE(m_aboutLblUser), "ABOUT");
	m_aboutBg = _texture(theme.texSet, "ABOUT/BG", "texture", theme.bg);
	m_aboutLblTitle = _addTitle(theme, "ABOUT/TITLE", theme.titleFont, L"", 20, 30, 600, 75, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_aboutLblInfo = _addText(theme, "ABOUT/INFO", theme.txtFont, L"", 20, 200, 600, 280, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_aboutBtnSystem = _addButton(theme, "ABOUT/SYSTEM_BTN", theme.btnFont, L"", 20, 400, 200, 56, theme.btnFontColor);
	m_aboutLblIOS = _addLabel(theme, "ABOUT/IOS", theme.txtFont, L"", 240, 400, 360, 56, theme.txtFontColor, FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE);
	// 
	_setHideAnim(m_aboutLblTitle, "ABOUT/TITLE", 0, 100, 0.f, 0.f);
	_setHideAnim(m_aboutLblInfo, "ABOUT/INFO", 0, 100, 0.f, 0.f);
	_setHideAnim(m_aboutBtnSystem, "ABOUT/SYSTEM_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_aboutLblIOS, "ABOUT/IOS", 0, 100, 0.f, 0.f);
	// 
	_hideAbout(true);
	_textAbout();
}

void CMenu::_textAbout(void)
{
	m_btnMgr.setText(m_aboutBtnSystem, _t("sys4", L"Update"));
	m_btnMgr.setText(m_aboutLblTitle, wfmt(_fmt("appname", L"%s (%s-r%s)"), APP_NAME, APP_VERSION, SVN_REV), false);

	wstringEx developers(wfmt(_fmt("about6", L"\nCurrent Developers:\n%s"), DEVELOPERS));
	wstringEx pDevelopers(wfmt(_fmt("about7", L"Past Developers:\n%s"), PAST_DEVELOPERS));

	wstringEx origLoader(wfmt(_fmt("about1", L"Original Loader By:\n%s"), LOADER_AUTHOR));
	wstringEx origGUI(wfmt(_fmt("about2", L"Original GUI By:\n%s"), GUI_AUTHOR));

	wstringEx codethx(wfmt(_fmt("about8", L"Bits of Code Obtained From:\n%s"), THANKS_CODE));
	wstringEx sites(wfmt(_fmt("about9", L"Supporting Websites:\n%s"), THANKS_SITES));

	wstringEx translator(wfmt(L", %s", m_loc.getWString(m_curLanguage, "translation_author").toUTF8().c_str()));
	wstringEx thanks(wfmt(_fmt("about4", L"Thanks To:\n%s"), THANKS));
	if(translator.size() > 3) thanks.append(translator);

	m_btnMgr.setText(m_aboutLblInfo,
			wfmt(L"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
			developers.toUTF8().c_str(),
			pDevelopers.toUTF8().c_str(),
			origLoader.toUTF8().c_str(),
			origGUI.toUTF8().c_str(),
			codethx.toUTF8().c_str(),
			sites.toUTF8().c_str(),
			thanks.toUTF8().c_str()),
			false
		);

	Nand::Instance()->Disable_Emu();
	iosinfo_t * iosInfo = cIOSInfo::GetInfo(mainIOS);
	if(iosInfo != NULL)
		m_btnMgr.setText(m_aboutLblIOS, wfmt(_fmt("ios", L"IOS%i base %i v%i"), mainIOS, iosInfo->baseios, iosInfo->version), true);
	SAFE_FREE(iosInfo);
	if(m_current_view == COVERFLOW_CHANNEL && m_cfg.getInt("NAND", "emulation", 0) > 0)
		Nand::Instance()->Enable_Emu();
}
