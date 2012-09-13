
#include "menu.hpp"

static const int g_curPage = 6;

void CMenu::_hideConfigScreen(bool instant)
{
	_hideConfigCommon(instant);

	m_btnMgr.hide(m_configScreenLblTVHeight, instant);
	m_btnMgr.hide(m_configScreenLblTVHeightVal, instant);
	m_btnMgr.hide(m_configScreenBtnTVHeightP, instant);
	m_btnMgr.hide(m_configScreenBtnTVHeightM, instant);
	m_btnMgr.hide(m_configScreenLblTVWidth, instant);
	m_btnMgr.hide(m_configScreenLblTVWidthVal, instant);
	m_btnMgr.hide(m_configScreenBtnTVWidthP, instant);
	m_btnMgr.hide(m_configScreenBtnTVWidthM, instant);
	m_btnMgr.hide(m_configScreenLblTVX, instant);
	m_btnMgr.hide(m_configScreenLblTVXVal, instant);
	m_btnMgr.hide(m_configScreenBtnTVXM, instant);
	m_btnMgr.hide(m_configScreenBtnTVXP, instant);
	m_btnMgr.hide(m_configScreenLblTVY, instant);
	m_btnMgr.hide(m_configScreenLblTVYVal, instant);
	m_btnMgr.hide(m_configScreenBtnTVYM, instant);
	m_btnMgr.hide(m_configScreenBtnTVYP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_configScreenLblUser); ++i)
		if(m_configScreenLblUser[i] != -1)
			m_btnMgr.hide(m_configScreenLblUser[i], instant);
}

void CMenu::_showConfigScreen(void)
{
	_showConfigCommon(m_configScreenBg, g_curPage);

	m_btnMgr.show(m_configScreenLblTVHeight);
	m_btnMgr.show(m_configScreenLblTVHeightVal);
	m_btnMgr.show(m_configScreenBtnTVHeightP);
	m_btnMgr.show(m_configScreenBtnTVHeightM);
	m_btnMgr.show(m_configScreenLblTVWidth);
	m_btnMgr.show(m_configScreenLblTVWidthVal);
	m_btnMgr.show(m_configScreenBtnTVWidthP);
	m_btnMgr.show(m_configScreenBtnTVWidthM);
	m_btnMgr.show(m_configScreenLblTVX);
	m_btnMgr.show(m_configScreenLblTVXVal);
	m_btnMgr.show(m_configScreenBtnTVXM);
	m_btnMgr.show(m_configScreenBtnTVXP);
	m_btnMgr.show(m_configScreenLblTVY);
	m_btnMgr.show(m_configScreenLblTVYVal);
	m_btnMgr.show(m_configScreenBtnTVYM);
	m_btnMgr.show(m_configScreenBtnTVYP);
	for(u8 i = 0; i < ARRAY_SIZE(m_configScreenLblUser); ++i)
		if(m_configScreenLblUser[i] != -1)
			m_btnMgr.show(m_configScreenLblUser[i]);

	m_btnMgr.setText(m_configScreenLblTVWidthVal, wfmt(L"%i", 640 * 640 / max(1, m_cfg.getInt("GENERAL", "tv_width", 640))));
	m_btnMgr.setText(m_configScreenLblTVHeightVal, wfmt(L"%i", 480 * 480 / max(1, m_cfg.getInt("GENERAL", "tv_height", 480))));
	m_btnMgr.setText(m_configScreenLblTVXVal, wfmt(L"%i", -m_cfg.getInt("GENERAL", "tv_x", 0)));
	m_btnMgr.setText(m_configScreenLblTVYVal, wfmt(L"%i", m_cfg.getInt("GENERAL", "tv_y", 0)));
}

int CMenu::_configScreen(void)
{
	int change = CONFIG_PAGE_NO_CHANGE;
	SetupInput();

	_showConfigScreen();
	while(!m_exit)
	{
		change = _configCommon();
		if (change != CONFIG_PAGE_NO_CHANGE)
			break;
		if (BTN_A_REPEAT)
		{
			if (m_btnMgr.selected(m_configScreenBtnTVWidthP) || m_btnMgr.selected(m_configScreenBtnTVWidthM)
				|| m_btnMgr.selected(m_configScreenBtnTVHeightP) || m_btnMgr.selected(m_configScreenBtnTVHeightM)
				|| m_btnMgr.selected(m_configScreenBtnTVXP) || m_btnMgr.selected(m_configScreenBtnTVXM)
				|| m_btnMgr.selected(m_configScreenBtnTVYP) || m_btnMgr.selected(m_configScreenBtnTVYM))
			{
				int step = 0;
				if (m_btnMgr.selected(m_configScreenBtnTVWidthM) || m_btnMgr.selected(m_configScreenBtnTVHeightM))
					step = 2;
				else if (m_btnMgr.selected(m_configScreenBtnTVWidthP) || m_btnMgr.selected(m_configScreenBtnTVHeightP))
					step = -2;
				else if (m_btnMgr.selected(m_configScreenBtnTVXP) || m_btnMgr.selected(m_configScreenBtnTVYM))
					step = -1;
				else if (m_btnMgr.selected(m_configScreenBtnTVXM) || m_btnMgr.selected(m_configScreenBtnTVYP))
					step = 1;
				if (m_btnMgr.selected(m_configScreenBtnTVWidthM) || m_btnMgr.selected(m_configScreenBtnTVWidthP))
					m_cfg.setInt("GENERAL", "tv_width", min(max(512, m_cfg.getInt("GENERAL", "tv_width", 640) + step), 800));
				else if (m_btnMgr.selected(m_configScreenBtnTVHeightM) || m_btnMgr.selected(m_configScreenBtnTVHeightP))
					m_cfg.setInt("GENERAL", "tv_height", min(max(384, m_cfg.getInt("GENERAL", "tv_height", 480) + step), 600));
				else if (m_btnMgr.selected(m_configScreenBtnTVXP) || m_btnMgr.selected(m_configScreenBtnTVXM))
					m_cfg.setInt("GENERAL", "tv_x", min(max(-50, m_cfg.getInt("GENERAL", "tv_x", 0) + step), 50));
				else if (m_btnMgr.selected(m_configScreenBtnTVYP) || m_btnMgr.selected(m_configScreenBtnTVYM))
					m_cfg.setInt("GENERAL", "tv_y", min(max(-30, m_cfg.getInt("GENERAL", "tv_y", 0) + step), 30));
				_showConfigScreen();
				m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
			}
		}
	}
	_hideConfigScreen();
	return change;
}

void CMenu::_initConfigScreenMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_configScreenLblUser, ARRAY_SIZE(m_configScreenLblUser), "SCREEN");
	m_configScreenBg = _texture(theme.texSet, "SCREEN/BG", "texture", theme.bg);
	m_configScreenLblTVWidth = _addLabel(theme, "SCREEN/TVWIDTH", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configScreenLblTVWidthVal = _addLabel(theme, "SCREEN/TVWIDTH_BTN", theme.btnFont, L"", 426, 130, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configScreenBtnTVWidthM = _addPicButton(theme, "SCREEN/TVWIDTH_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 130, 56, 56);
	m_configScreenBtnTVWidthP = _addPicButton(theme, "SCREEN/TVWIDTH_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_configScreenLblTVHeight = _addLabel(theme, "SCREEN/TVHEIGHT", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configScreenLblTVHeightVal = _addLabel(theme, "SCREEN/TVHEIGHT_BTN", theme.btnFont, L"", 426, 190, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configScreenBtnTVHeightM = _addPicButton(theme, "SCREEN/TVHEIGHT_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 190, 56, 56);
	m_configScreenBtnTVHeightP = _addPicButton(theme, "SCREEN/TVHEIGHT_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 190, 56, 56);
	m_configScreenLblTVX = _addLabel(theme, "SCREEN/TVX", theme.lblFont, L"", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configScreenLblTVXVal = _addLabel(theme, "SCREEN/TVX_BTN", theme.btnFont, L"", 426, 250, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configScreenBtnTVXM = _addPicButton(theme, "SCREEN/TVX_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 250, 56, 56);
	m_configScreenBtnTVXP = _addPicButton(theme, "SCREEN/TVX_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);
	m_configScreenLblTVY = _addLabel(theme, "SCREEN/TVY", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configScreenLblTVYVal = _addLabel(theme, "SCREEN/TVY_BTN", theme.btnFont, L"", 426, 310, 118, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configScreenBtnTVYM = _addPicButton(theme, "SCREEN/TVY_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 370, 310, 56, 56);
	m_configScreenBtnTVYP = _addPicButton(theme, "SCREEN/TVY_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
	// 
	_setHideAnim(m_configScreenLblTVWidth, "SCREEN/TVWIDTH", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configScreenLblTVWidthVal, "SCREEN/TVWIDTH_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenBtnTVWidthM, "SCREEN/TVWIDTH_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenBtnTVWidthP, "SCREEN/TVWIDTH_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenLblTVHeight, "SCREEN/TVHEIGHT", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configScreenLblTVHeightVal, "SCREEN/TVHEIGHT_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenBtnTVHeightM, "SCREEN/TVHEIGHT_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenBtnTVHeightP, "SCREEN/TVHEIGHT_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenLblTVX, "SCREEN/TVX", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configScreenLblTVXVal, "SCREEN/TVX_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenBtnTVXM, "SCREEN/TVX_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenBtnTVXP, "SCREEN/TVX_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenLblTVY, "SCREEN/TVY", 100, 0, -2.f, 0.f);
	_setHideAnim(m_configScreenLblTVYVal, "SCREEN/TVY_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenBtnTVYM, "SCREEN/TVY_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_configScreenBtnTVYP, "SCREEN/TVY_PLUS", 0, 0, 1.f, -1.f);
	_hideConfigScreen(true);
	_textConfigScreen();
}

void CMenu::_textConfigScreen(void)
{
	m_btnMgr.setText(m_configScreenLblTVWidth, _t("cfgc2", L"Adjust TV width"));
	m_btnMgr.setText(m_configScreenLblTVHeight, _t("cfgc3", L"Adjust TV height"));
	m_btnMgr.setText(m_configScreenLblTVX, _t("cfgc6", L"Horizontal offset"));
	m_btnMgr.setText(m_configScreenLblTVY, _t("cfgc7", L"Vertical offset"));
}
