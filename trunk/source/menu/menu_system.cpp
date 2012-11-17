
#include "menu.hpp"
#include "const_str.hpp"
#include "lockMutex.hpp"
#include "loader/wbfs.h"

int version_num = 0, num_versions = 0, i;
const u32 SVN_REV_NUM = atoi(SVN_REV);
int CMenu::_version[9] = {0, SVN_REV_NUM, SVN_REV_NUM, SVN_REV_NUM, SVN_REV_NUM, SVN_REV_NUM, SVN_REV_NUM, SVN_REV_NUM, SVN_REV_NUM};

const int pixels_to_skip = 10;

void CMenu::_system()
{
	int msg = 0, newVer = SVN_REV_NUM;
	lwp_t thread = LWP_THREAD_NULL;
	wstringEx prevMsg;

	int amount_of_skips = 0;
	int update_x = 0, update_y = 0;
	u32 update_w = 0, update_h = 0;
	bool first = true;

	m_btnMgr.reset(m_systemLblInfo, true);

	SetupInput();
	m_btnMgr.setText(m_systemBtnBack, _t("dl1", L"Cancel"));
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	m_showtimer = -1;
	while(!m_exit)
	{
		_mainLoopCommon();
		if(amount_of_skips == 0) // Check dimensions in the loop, because the animation can have an effect
			m_btnMgr.getDimensions(m_systemLblInfo, update_x, update_y, update_w, update_h); // Get original dimensions
		if(first)
		{
			m_btnMgr.moveBy(m_systemLblInfo, 0, -(pixels_to_skip * 10));
			amount_of_skips++;
			first = false;
		}

		if (m_showtimer == -1)
		{
			m_showtimer = 120;
			m_btnMgr.show(m_downloadPBar);
			m_btnMgr.setProgress(m_downloadPBar, 0.f);
			m_thrdStop = false;
			m_thrdWorking = true;
			LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_versionTxtDownloaderInit, (void *)this, 0, 8192, 40);
		}
		if (m_showtimer > 0 && !m_thrdWorking)
		{
			if (thread != LWP_THREAD_NULL)
			{
				LWP_JoinThread(thread, NULL);
				thread = LWP_THREAD_NULL;
			}
			if (--m_showtimer == 0)
			{
				m_btnMgr.hide(m_downloadPBar);
				m_btnMgr.hide(m_downloadLblMessage[0], 0, 0, -2.f, 0.f);
				m_btnMgr.hide(m_downloadLblMessage[1], 0, 0, -2.f, 0.f);
				CMenu::_version[1] = m_version.getInt("GENERAL", "version", SVN_REV_NUM);
				num_versions = m_version.getInt("GENERAL", "num_versions", 1);
				for (i = 2; i < num_versions; i++)
				{
					CMenu::_version[i] = m_version.getInt(fmt("VERSION%i", i-1u), "version", SVN_REV_NUM);
					//add the changelog info here
				}
				if (num_versions > 1 && version_num == 0) version_num = 1;
				i = min((u32)version_num, ARRAY_SIZE(CMenu::_version) -1u);
				newVer = CMenu::_version[i];
				_showSystem();
			}
		}
		if ((BTN_DOWN_PRESSED || BTN_DOWN_HELD) && !(m_thrdWorking && m_thrdStop))
		{
			if (update_h - (amount_of_skips * pixels_to_skip) > (m_vid.height2D() - (35 + update_y)))
			{
				m_btnMgr.moveBy(m_systemLblInfo, 0, -pixels_to_skip);
				amount_of_skips++;
			}
		}
		else if ((BTN_UP_PRESSED || BTN_UP_HELD) && !(m_thrdWorking && m_thrdStop))
		{
			if (amount_of_skips > 1)
			{
				m_btnMgr.moveBy(m_systemLblInfo, 0, pixels_to_skip);
				amount_of_skips--;
			}
		}
		else if ((BTN_HOME_PRESSED || BTN_B_PRESSED || m_exit) && !m_thrdWorking)
			break;
		else if ((BTN_A_PRESSED) && !(m_thrdWorking && m_thrdStop))
		{
			if ((m_btnMgr.selected(m_systemBtnDownload)) && !m_thrdWorking)
			{
				// Download selected version
				_hideSystem();
				m_btnMgr.show(m_downloadPBar);
				m_btnMgr.setProgress(m_downloadPBar, 0.f);
				m_thrdStop = false;
				m_thrdWorking = true;
				gprintf("\nVersion to DL: %i\n", newVer);

				if(m_version.getInt("GENERAL", "version", 0) == newVer)
					m_app_update_size = m_version.getInt("GENERAL", "app_zip_size", 0);
				m_data_update_size = m_version.getInt("GENERAL", "data_zip_size", 0);

				m_app_update_url = fmt("%s/Wiiflow_Mod_svn_r%i.zip", m_version.getString("GENERAL", "update_url", "http://open-wiiflow-mod.googlecode.com/files").c_str(), newVer);
				m_data_update_url = fmt("%s/r%i/data.zip", m_version.getString("GENERAL", "update_url", "http://open-wiiflow-mod.googlecode.com/files").c_str(), newVer);

				m_showtimer = 120;
				LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_versionDownloaderInit, (void *)this, 0, 8192, 40);
				if (m_exit && !m_thrdWorking) 
				{
					m_thrdStop = true;
					break;
				}
			}
			else if (m_btnMgr.selected(m_systemBtnBack))
			{
				LockMutex lock(m_mutex);
				m_thrdStop = true;
				m_thrdMessageAdded = true;
				m_thrdMessage = _t("dlmsg6", L"Canceling...");
			}
			else if (m_btnMgr.selected(m_systemBtnVerSelectM))
			{
				if (version_num > 1)
					--version_num;
				else
					version_num = num_versions;
				i = min((u32)version_num, ARRAY_SIZE(CMenu::_version) -1u);
				{
					m_btnMgr.setText(m_systemLblVerSelectVal, wstringEx(sfmt("%i", CMenu::_version[i])));
					newVer = CMenu::_version[i];
					m_app_update_size = m_version.getInt(sfmt("VERSION%i", i - 1u), "app_zip_size", 0);
					if (i > 1 && i != num_versions)
						m_btnMgr.setText(m_systemLblInfo, m_version.getWString(sfmt("VERSION%i", i - 1u), "changes"), false);
					else 
						if (i == num_versions)
							m_btnMgr.setText(m_systemLblInfo, _t("sys7", L"Installed Version."), false);
						else
							m_btnMgr.setText(m_systemLblInfo, m_version.getWString("GENERAL", "changes"), false);
				}
			}
			else if (m_btnMgr.selected(m_systemBtnVerSelectP))
			{
				if (version_num < num_versions)
					++version_num;
				else
					version_num = 1;
				i = min((u32)version_num, ARRAY_SIZE(CMenu::_version) -1u);
				{
					m_btnMgr.setText(m_systemLblVerSelectVal, wstringEx(sfmt("%i", CMenu::_version[i])));
					newVer = CMenu::_version[i];
					m_app_update_size = m_version.getInt(sfmt("VERSION%i", i - 1u), "app_zip_size", 0);
					if (i > 1 && i != num_versions)
						m_btnMgr.setText(m_systemLblInfo, m_version.getWString(sfmt("VERSION%i", i - 1u), "changes"), false);
					else 
						if (i == num_versions)
							m_btnMgr.setText(m_systemLblInfo, _t("sys7", L"Installed Version."), false);
						else
							m_btnMgr.setText(m_systemLblInfo, m_version.getWString("GENERAL", "changes"), false);
				}
			}
		}
		if (Sys_Exiting())
		{
			LockMutex lock(m_mutex);
			m_thrdStop = true;
			m_thrdMessageAdded = true;
			m_thrdMessage = _t("dlmsg6", L"Canceling...");
		}
		// 
		if (m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			m_btnMgr.setProgress(m_downloadPBar, m_thrdProgress);
			if (m_thrdProgress == 1.f)
				m_btnMgr.setText(m_systemBtnBack, _t("dl2", L"Back"));
			if (prevMsg != m_thrdMessage)
			{
				prevMsg = m_thrdMessage;
				m_btnMgr.setText(m_downloadLblMessage[msg], m_thrdMessage, false);
				m_btnMgr.hide(m_downloadLblMessage[msg], -200, 0, 1.f, 0.5f, true);
				m_btnMgr.show(m_downloadLblMessage[msg]);
				msg ^= 1;
				m_btnMgr.hide(m_downloadLblMessage[msg], +400, 0, 1.f, 1.f);
			}
		}
		if (m_thrdStop && !m_thrdWorking)
			break;
	}
	if (thread != LWP_THREAD_NULL)
	{
		LWP_JoinThread(thread, NULL);
		thread = LWP_THREAD_NULL;
	}
	_hideSystem();
}

void CMenu::_hideSystem(bool instant)
{
	m_btnMgr.hide(m_systemLblTitle, instant);
	m_btnMgr.hide(m_systemLblVersionTxt, instant);
	m_btnMgr.hide(m_systemLblVersion, instant);
	m_btnMgr.hide(m_systemBtnBack, instant);
	m_btnMgr.hide(m_systemBtnDownload, instant);
	m_btnMgr.hide(m_downloadPBar, instant);
	m_btnMgr.hide(m_downloadLblMessage[0], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblMessage[1], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_systemLblInfo);
	m_btnMgr.hide(m_systemLblVerSelectVal);
	m_btnMgr.hide(m_systemBtnVerSelectM);
	m_btnMgr.hide(m_systemBtnVerSelectP);
	for(u8 i = 0; i < ARRAY_SIZE(m_systemLblUser); ++i)
		if(m_systemLblUser[i] != -1)
			m_btnMgr.hide(m_systemLblUser[i], instant);
}

void CMenu::_showSystem(void)
{
	_setBg(m_systemBg, m_systemBg);
	m_btnMgr.show(m_systemLblTitle);
	m_btnMgr.show(m_systemLblVersionTxt);
	m_btnMgr.show(m_systemLblVersion);
	m_btnMgr.show(m_systemBtnBack);
	m_btnMgr.show(m_systemLblInfo,false);
	m_btnMgr.show(m_systemLblVerSelectVal);
	m_btnMgr.show(m_systemBtnVerSelectM);
	m_btnMgr.show(m_systemBtnVerSelectP);
	m_btnMgr.show(m_systemBtnDownload);
	for(u8 i = 0; i < ARRAY_SIZE(m_systemLblUser); ++i)
		if(m_systemLblUser[i] != -1)
			m_btnMgr.show(m_systemLblUser[i]);
	_textSystem();
}

void CMenu::_initSystemMenu()
{
	STexture emptyTex;

	_addUserLabels(m_systemLblUser, ARRAY_SIZE(m_systemLblUser), "SYSTEM");		
	m_systemBg = _texture("SYSTEM/BG", "texture", theme.bg, false);
	m_systemLblTitle = _addTitle("SYSTEM/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_systemLblVersionTxt = _addLabel("SYSTEM/VERSION_TXT", theme.lblFont, L"", 40, 90, 220, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_systemLblVersion = _addLabel("SYSTEM/VERSION", theme.lblFont, L"", 260, 90, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_systemBtnDownload = _addButton("SYSTEM/DOWNLOAD_BTN", theme.btnFont, L"", 20, 410, 200, 56, theme.btnFontColor);
	m_systemBtnBack = _addButton("SYSTEM/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor); 

	m_systemLblInfo = _addText("SYSTEM/INFO", theme.txtFont, L"", 20, 300, 600, 280, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_systemLblVerSelectVal = _addLabel("SYSTEM/VER_SELECT_BTN", theme.btnFont, L"", 296, 150, 50, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_systemBtnVerSelectM = _addPicButton("SYSTEM/VER_SELECT_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 240, 150, 56, 56);
	m_systemBtnVerSelectP = _addPicButton("SYSTEM/VER_SELECT_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 346, 150, 56, 56);
	// 
	_setHideAnim(m_systemLblTitle, "SYSTEM/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_systemBtnDownload, "SYSTEM/DOWNLOAD_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_systemBtnBack, "SYSTEM/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_systemLblVersionTxt, "SYSTEM/VERSION_TXT", -100, 0, 0.f, 0.f);
	_setHideAnim(m_systemLblVersion, "SYSTEM/VERSION", 200, 0, 0.f, 0.f);

	_setHideAnim(m_systemLblInfo, "SYSTEM/INFO", 0, 100, 0.f, 0.f);
	_setHideAnim(m_systemLblVerSelectVal, "SYSTEM/VER_SELECT_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_systemBtnVerSelectM, "SYSTEM/VER_SELECT_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_systemBtnVerSelectP, "SYSTEM/VER_SELECT_PLUS", 0, 0, 1.f, -1.f);
	// 
	_hideSystem(true);
	_textSystem();
}

void CMenu::_textSystem(void)
{
	m_btnMgr.setText(m_systemLblTitle, _t("sys1", L"Update WiiFlow"));
	m_btnMgr.setText(m_systemLblVersionTxt, _t("sys2", L"WiiFlow Version:"));
	m_btnMgr.setText(m_systemLblVersion, SVN_REV_W);
	m_btnMgr.setText(m_systemBtnBack, _t("sys3", L"Cancel"));
	m_btnMgr.setText(m_systemBtnDownload, _t("sys4", L"Upgrade"));
	i = min((u32)version_num, ARRAY_SIZE(CMenu::_version) -1u);
	if (i == 0)
		m_btnMgr.setText(m_systemLblVerSelectVal, SVN_REV_W);
	else
	{
		m_btnMgr.setText(m_systemLblVerSelectVal, wstringEx(sfmt("%i", CMenu::_version[i])));
		if (i > 1 && i != num_versions)
			m_btnMgr.setText(m_systemLblInfo, m_version.getWString(sfmt("VERSION%i", i - 1u), "changes"), false);
		else 
			if (i == num_versions)
				m_btnMgr.setText(m_systemLblInfo, _t("sys7", L"Installed Version."), false);
			else
				m_btnMgr.setText(m_systemLblInfo, m_version.getWString("GENERAL", "changes"), false);
	}
}
