
#include "menu.hpp"

static const int g_curPage = 5;

void CMenu::_hideConfigSnd(bool instant)
{
	_hideConfigCommon(instant);

	m_btnMgr.hide(m_configSndLblBnrVol, instant);
	m_btnMgr.hide(m_configSndLblBnrVolVal, instant);
	m_btnMgr.hide(m_configSndBtnBnrVolP, instant);
	m_btnMgr.hide(m_configSndBtnBnrVolM, instant);
	m_btnMgr.hide(m_configSndLblMusicVol, instant);
	m_btnMgr.hide(m_configSndLblMusicVolVal, instant);
	m_btnMgr.hide(m_configSndBtnMusicVolP, instant);
	m_btnMgr.hide(m_configSndBtnMusicVolM, instant);
	m_btnMgr.hide(m_configSndLblGuiVol, instant);
	m_btnMgr.hide(m_configSndLblGuiVolVal, instant);
	m_btnMgr.hide(m_configSndBtnGuiVolP, instant);
	m_btnMgr.hide(m_configSndBtnGuiVolM, instant);
	m_btnMgr.hide(m_configSndLblCFVol, instant);
	m_btnMgr.hide(m_configSndLblCFVolVal, instant);
	m_btnMgr.hide(m_configSndBtnCFVolP, instant);
	m_btnMgr.hide(m_configSndBtnCFVolM, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_configSndLblUser); ++i)
		if(m_configSndLblUser[i] != -1)
			m_btnMgr.hide(m_configSndLblUser[i], instant);
}

void CMenu::_showConfigSnd(void)
{
	_showConfigCommon(m_configSndBg, g_curPage);

	m_btnMgr.show(m_configSndLblBnrVol);
	m_btnMgr.show(m_configSndLblBnrVolVal);
	m_btnMgr.show(m_configSndBtnBnrVolP);
	m_btnMgr.show(m_configSndBtnBnrVolM);
	m_btnMgr.show(m_configSndLblMusicVol);
	m_btnMgr.show(m_configSndLblMusicVolVal);
	m_btnMgr.show(m_configSndBtnMusicVolP);
	m_btnMgr.show(m_configSndBtnMusicVolM);
	m_btnMgr.show(m_configSndLblGuiVol);
	m_btnMgr.show(m_configSndLblGuiVolVal);
	m_btnMgr.show(m_configSndBtnGuiVolP);
	m_btnMgr.show(m_configSndBtnGuiVolM);
	m_btnMgr.show(m_configSndLblCFVol);
	m_btnMgr.show(m_configSndLblCFVolVal);
	m_btnMgr.show(m_configSndBtnCFVolP);
	m_btnMgr.show(m_configSndBtnCFVolM);
	for(u8 i = 0; i < ARRAY_SIZE(m_configSndLblUser); ++i)
		if(m_configSndLblUser[i] != -1)
			m_btnMgr.show(m_configSndLblUser[i]);

	m_btnMgr.setText(m_configSndLblGuiVolVal, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_gui", 255)));
	m_btnMgr.setText(m_configSndLblCFVolVal, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255)));
	m_btnMgr.setText(m_configSndLblMusicVolVal, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_music", 255)));
	m_btnMgr.setText(m_configSndLblBnrVolVal, wfmt(L"%i", m_cfg.getInt("GENERAL", "sound_volume_bnr", 255)));
}

int CMenu::_configSnd(void)
{
	int change = CONFIG_PAGE_NO_CHANGE;
	SetupInput();
	int step = 1;

	_showConfigSnd();
	while(!m_exit)
	{
		change = _configCommon();
		if (change != CONFIG_PAGE_NO_CHANGE)
			break;
		if (BTN_A_REPEAT || BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_configSndBtnBnrVolP))
			{
				m_cfg.setInt("GENERAL", "sound_volume_bnr", min(m_cfg.getInt("GENERAL", "sound_volume_bnr", 255) + step, 255));
				_showConfigSnd();
				m_bnrSndVol = m_cfg.getInt("GENERAL", "sound_volume_bnr", 255);
			}
			else if (m_btnMgr.selected(m_configSndBtnBnrVolM))
			{
				m_cfg.setInt("GENERAL", "sound_volume_bnr", max(m_cfg.getInt("GENERAL", "sound_volume_bnr", 255) - step, 0));
				_showConfigSnd();
				m_bnrSndVol = m_cfg.getInt("GENERAL", "sound_volume_bnr", 255);
			}
			else if (m_btnMgr.selected(m_configSndBtnGuiVolP))
			{
				m_cfg.setInt("GENERAL", "sound_volume_gui", min(m_cfg.getInt("GENERAL", "sound_volume_gui", 255) + step, 255));
				_showConfigSnd();
				m_btnMgr.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_gui", 255));
			}
			else if (m_btnMgr.selected(m_configSndBtnGuiVolM))
			{
				m_cfg.setInt("GENERAL", "sound_volume_gui", max(m_cfg.getInt("GENERAL", "sound_volume_gui", 255) - step, 0));
				_showConfigSnd();
				m_btnMgr.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_gui", 255));
			}
			else if (m_btnMgr.selected(m_configSndBtnMusicVolP))
			{
				int musicVol = min(m_cfg.getInt("GENERAL", "sound_volume_music", 255) + step, 255);
				m_cfg.setInt("GENERAL", "sound_volume_music", musicVol);
				_showConfigSnd();
				MusicPlayer.SetMaxVolume(musicVol);
			}
			else if (m_btnMgr.selected(m_configSndBtnMusicVolM))
			{
				int musicVol = max(m_cfg.getInt("GENERAL", "sound_volume_music", 255) - step, 0);
				m_cfg.setInt("GENERAL", "sound_volume_music", musicVol);
				_showConfigSnd();
				MusicPlayer.SetMaxVolume(musicVol);
			}
			else if (m_btnMgr.selected(m_configSndBtnCFVolP))
			{
				m_cfg.setInt("GENERAL", "sound_volume_coverflow", min(m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255) + step, 255));
				_showConfigSnd();
				CoverFlow.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255));
			}
			else if (m_btnMgr.selected(m_configSndBtnCFVolM))
			{
				m_cfg.setInt("GENERAL", "sound_volume_coverflow", max(m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255) - step, 0));
				_showConfigSnd();
				CoverFlow.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255));
			}
		}
	}
	_hideConfigSnd();
	return change;
}

void CMenu::_initConfigSndMenu()
{
	_addUserLabels(m_configSndLblUser, ARRAY_SIZE(m_configSndLblUser), "CONFIGSND");
	m_configSndBg = _texture("CONFIGSND/BG", "texture", theme.bg, false);
	m_configSndLblMusicVol = _addLabel("CONFIGSND/MUSIC_VOL", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configSndLblMusicVolVal = _addLabel("CONFIGSND/MUSIC_VOL_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configSndBtnMusicVolM = _addPicButton("CONFIGSND/MUSIC_VOL_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_configSndBtnMusicVolP = _addPicButton("CONFIGSND/MUSIC_VOL_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	m_configSndLblGuiVol = _addLabel("CONFIGSND/GUI_VOL", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configSndLblGuiVolVal = _addLabel("CONFIGSND/GUI_VOL_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configSndBtnGuiVolM = _addPicButton("CONFIGSND/GUI_VOL_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_configSndBtnGuiVolP = _addPicButton("CONFIGSND/GUI_VOL_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	m_configSndLblCFVol = _addLabel("CONFIGSND/CF_VOL", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configSndLblCFVolVal = _addLabel("CONFIGSND/CF_VOL_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configSndBtnCFVolM = _addPicButton("CONFIGSND/CF_VOL_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_configSndBtnCFVolP = _addPicButton("CONFIGSND/CF_VOL_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);
	m_configSndLblBnrVol = _addLabel("CONFIGSND/BNR_VOL", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_configSndLblBnrVolVal = _addLabel("CONFIGSND/BNR_VOL_BTN", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_configSndBtnBnrVolM = _addPicButton("CONFIGSND/BNR_VOL_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_configSndBtnBnrVolP = _addPicButton("CONFIGSND/BNR_VOL_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);
	// 
	_setHideAnim(m_configSndLblMusicVol, "CONFIGSND/MUSIC_VOL", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configSndLblMusicVolVal, "CONFIGSND/MUSIC_VOL_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndBtnMusicVolM, "CONFIGSND/MUSIC_VOL_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndBtnMusicVolP, "CONFIGSND/MUSIC_VOL_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndLblGuiVol, "CONFIGSND/GUI_VOL", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configSndLblGuiVolVal, "CONFIGSND/GUI_VOL_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndBtnGuiVolM, "CONFIGSND/GUI_VOL_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndBtnGuiVolP, "CONFIGSND/GUI_VOL_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndLblCFVol, "CONFIGSND/CF_VOL", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configSndLblCFVolVal, "CONFIGSND/CF_VOL_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndBtnCFVolM, "CONFIGSND/CF_VOL_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndBtnCFVolP, "CONFIGSND/CF_VOL_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndLblBnrVol, "CONFIGSND/BNR_VOL", 50, 0, -2.f, 0.f);
	_setHideAnim(m_configSndLblBnrVolVal, "CONFIGSND/BNR_VOL_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndBtnBnrVolM, "CONFIGSND/BNR_VOL_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_configSndBtnBnrVolP, "CONFIGSND/BNR_VOL_PLUS", -50, 0, 1.f, 0.f);
	_hideConfigSnd(true);
	_textConfigSnd();
}

void CMenu::_textConfigSnd(void)
{
	m_btnMgr.setText(m_configSndLblMusicVol, _t("cfgs1", L"Music volume"));
	m_btnMgr.setText(m_configSndLblGuiVol, _t("cfgs2", L"GUI sound volume"));
	m_btnMgr.setText(m_configSndLblCFVol, _t("cfgs3", L"Coverflow sound volume"));
	m_btnMgr.setText(m_configSndLblBnrVol, _t("cfgs4", L"Game sound volume"));
}
