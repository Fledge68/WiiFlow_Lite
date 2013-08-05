
#include "menu.hpp"
#include "channel/nand.hpp"
#include "loader/alt_ios.h"
#include "loader/cios.h"
#include "const_str.hpp"

const int pixels_to_skip = 10;

extern const u8 english_txt[];
static const wstringEx ENGLISH_TXT_W((const char*)english_txt);

//About menu
s16 m_aboutLblTitle;
s16 m_aboutLblInfo;
s16 m_aboutLblUser[4];
s16 m_aboutLblIOS;
bool showHelp;

void CMenu::_about(bool help)
{
	showHelp = help;
	int amount_of_skips = 0;
	int thanks_x = 0, thanks_y = 0;
	u32 thanks_w = 0, thanks_h = 0;
	bool first = true;

	_textAbout();
	m_btnMgr.reset(m_aboutLblInfo, true);

	SetupInput();
	_showAbout();

	while(!m_exit)
	{
		_mainLoopCommon();
		if(amount_of_skips == 0) // Check dimensions in the loop, because the animation can have an effect
			m_btnMgr.getDimensions(m_aboutLblInfo, thanks_x, thanks_y, thanks_w, thanks_h); // Get original dimensions
		if(first)
		{
			m_btnMgr.moveBy(m_aboutLblInfo, 0, -1);
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
	}
	_hideAbout(false);
}

void CMenu::_hideAbout(bool instant)
{
	m_btnMgr.hide(m_aboutLblTitle, instant);
	m_btnMgr.hide(m_aboutLblIOS, instant);
	m_btnMgr.hide(m_aboutLblInfo, instant);
	for (u8 i = 0; i < ARRAY_SIZE(m_aboutLblUser); ++i)
	{
		if(m_aboutLblUser[i] != -1)
			m_btnMgr.hide(m_aboutLblUser[i], instant);
	}
}

void CMenu::_showAbout(void)
{
	_setBg(m_aboutBg, m_aboutBg);
	m_btnMgr.show(m_aboutLblTitle);
	if(m_txt_view == false)
		m_btnMgr.show(m_aboutLblIOS);
	m_btnMgr.show(m_aboutLblInfo,false);
	for(u8 i = 0; i < ARRAY_SIZE(m_aboutLblUser); ++i)
	{
		if(m_aboutLblUser[i] != -1)
			m_btnMgr.show(m_aboutLblUser[i]);
	}
}

void CMenu::_initAboutMenu()
{
	_addUserLabels(m_aboutLblUser, ARRAY_SIZE(m_aboutLblUser), "ABOUT");
	m_aboutBg = _texture("ABOUT/BG", "texture", theme.bg, false);
	m_aboutLblTitle = _addTitle("ABOUT/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_aboutLblInfo = _addText("ABOUT/INFO", theme.txtFont, L"", 40, 115, 560, 270, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_aboutLblIOS = _addLabel("ABOUT/IOS", theme.txtFont, L"", 240, 400, 360, 56, theme.txtFontColor, FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE);

	_setHideAnim(m_aboutLblTitle, "ABOUT/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_aboutLblInfo, "ABOUT/INFO", 0, 100, 0.f, 0.f);
	_setHideAnim(m_aboutLblIOS, "ABOUT/IOS", 0, 0, -2.f, 0.f);

	_hideAbout(true);
}

void CMenu::_textAbout(void)
{
	if(m_txt_view)
	{
		wstringEx txt_file_name;
		txt_file_name.fromUTF8(strrchr(m_txt_path, '/') + 1);
		m_btnMgr.setText(m_aboutLblTitle, txt_file_name);
		wstringEx txt_file_content;
		u32 txt_size = 0;
		char *txt_mem = (char*)fsop_ReadFile(m_txt_path, &txt_size);
		if(txt_mem != NULL)
		{
			txt_file_content.fromUTF8(txt_mem);
			m_btnMgr.setText(m_aboutLblInfo, txt_file_content);
			free(txt_mem);
		}
		txt_mem = NULL;
		return; /* no need for ios checks */
	}
	else if(showHelp)
	{
		m_btnMgr.setText(m_aboutLblTitle, _t("about10", L"Help Guide"));
		wstringEx help_text;
		FILE *f = fopen(fmt("%s/%s.txt", m_helpDir.c_str(), lowerCase(m_curLanguage).c_str()), "r");
		if(f)
		{
			fseek(f, 0, SEEK_END);
			u32 fsize = ftell(f);
			char *help = (char*)MEM2_alloc(fsize+1); //+1 for null character
			fseek(f, 0, SEEK_SET);
			fread(help, 1, fsize, f);
			help[fsize] = '\0';
			help_text.fromUTF8(help);
			MEM2_free(help);
			fclose(f);
			m_btnMgr.setText(m_aboutLblInfo, help_text);
		}
		else
			m_btnMgr.setText(m_aboutLblInfo, ENGLISH_TXT_W);
	}
	else
	{
		m_btnMgr.setText(m_aboutLblTitle, VERSION_STRING);

		wstringEx developers(wfmt(_fmt("about6", L"\nCurrent Developers:\n%s"), DEVELOPERS));
		wstringEx pDevelopers(wfmt(_fmt("about7", L"Past Developers:\n%s"), PAST_DEVELOPERS));

		wstringEx origLoader(wfmt(_fmt("about1", L"Original Loader By:\n%s"), LOADER_AUTHOR));
		wstringEx origGUI(wfmt(_fmt("about2", L"Original GUI By:\n%s"), GUI_AUTHOR));

		wstringEx codethx(wfmt(_fmt("about8", L"Bits of Code Obtained From:\n%s"), THANKS_CODE));
		wstringEx sites(wfmt(_fmt("about9", L"Supporting Websites:\n%s"), THANKS_SITES));

		wstringEx translator(wfmt(L", %s", m_loc.getWString(m_curLanguage, "translation_author").toUTF8().c_str()));
		wstringEx thanks(wfmt(_fmt("about4", L"Thanks To:\n%s"), THANKS));
		if(translator.size() > 3)
			thanks.append(translator);

		m_btnMgr.setText(m_aboutLblInfo,
			wfmt(L"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
			developers.toUTF8().c_str(),
			pDevelopers.toUTF8().c_str(),
			origLoader.toUTF8().c_str(),
			origGUI.toUTF8().c_str(),
			codethx.toUTF8().c_str(),
			sites.toUTF8().c_str(),
			thanks.toUTF8().c_str())
		);
	}
	const char *IOS_Name = NULL;
	switch(IOS_GetType(CurrentIOS.Version))
	{
		case IOS_TYPE_D2X:
			IOS_Name = "D2X";
			break;
		case IOS_TYPE_WANIN:
			IOS_Name = "Waninkoko";
			break;
		case IOS_TYPE_HERMES:
		case IOS_TYPE_KWIIRK:
			IOS_Name = "Hermes";
			break;
		default:
			break;
	}
	if(IOS_Name == NULL)
		m_btnMgr.setText(m_aboutLblIOS, wfmt(L"IOS%i v%i", CurrentIOS.Version, 
			CurrentIOS.Revision), true);
	else
		m_btnMgr.setText(m_aboutLblIOS, wfmt(L"%s IOS%i[%i] v%d.%d", IOS_Name, CurrentIOS.Version,
			CurrentIOS.Base, CurrentIOS.Revision, CurrentIOS.SubRevision), true);
}
