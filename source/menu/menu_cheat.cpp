
#include "menu.hpp"
#include "gui/text.hpp"
#include "lockMutex.hpp"
#include "network/https.h"

#define GECKOURL "https://codes.rc24.xyz/txt.php?txt=%s"
#define CHEATSPERPAGE 4

u8 m_cheatSettingsPage = 0;
int txtavailable;

int CMenu::_downloadCheatFileAsync()
{
	m_thrdTotal = 2;// download and save
	
	m_thrdMessage = _t("dlmsg1", L"Initializing network...");
	m_thrdMessageAdded = true;
	if(_initNetwork() < 0)
	{
		return -2;
	}
	m_thrdMessage = _t("dlmsg11", L"Downloading...");
	m_thrdMessageAdded = true;
	
	const char *id = CoverFlow.getId();
	struct download file = {};
	downloadfile(fmt(m_cfg.getString("general", "cheats_url", GECKOURL).c_str(), id), &file);
	if(file.size > 0 && file.data[0] != '<')
	{
		m_thrdMessage = _t("dlmsg13", L"Saving...");
		m_thrdMessageAdded = true;
		update_pThread(1);// its downloaded
		fsop_WriteFile(fmt("%s/%s.txt", m_txtCheatDir.c_str(), id), file.data, file.size);
		MEM2_free(file.data);
		return 0;
	}
	if(file.size > 0)// received a 301/302 redirect instead of a 404?
	{
		MEM2_free(file.data);
		return -4;// the file doesn't exist on the server
	}
	return -3;// download failed
}

void CMenu::_CheatSettings() 
{
	SetupInput();

	const char *id = CoverFlow.getId();

	m_cheatSettingsPage = 1;
	txtavailable = m_cheatfile.openTxtfile(fmt("%s/%s.txt", m_txtCheatDir.c_str(), id)); 
	
	_textCheatSettings();
	_showCheatSettings();

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if (txtavailable && (BTN_MINUS_PRESSED || BTN_LEFT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_cheatBtnPageM))))
		{
			_hideCheatSettings();
			if (m_cheatSettingsPage == 1)
				m_cheatSettingsPage = (m_cheatfile.getCnt()+CHEATSPERPAGE-1)/CHEATSPERPAGE;
			else if (m_cheatSettingsPage > 1)
				--m_cheatSettingsPage;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED) m_btnMgr.click(m_cheatBtnPageM);
			_showCheatSettings();
		}
		else if (txtavailable && (BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_cheatBtnPageP))))
		{
			_hideCheatSettings();
			if (m_cheatSettingsPage == (m_cheatfile.getCnt()+CHEATSPERPAGE-1)/CHEATSPERPAGE)
				m_cheatSettingsPage = 1;
			else if (m_cheatSettingsPage < (m_cheatfile.getCnt()+CHEATSPERPAGE-1)/CHEATSPERPAGE)
				++m_cheatSettingsPage;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_cheatBtnPageP);
			_showCheatSettings();
		}
		else if ((WBTN_2_HELD && WBTN_1_PRESSED) || (WBTN_1_HELD && WBTN_2_PRESSED))// pressing 1 and 2 deletes everything so cheats can be downloaded again.
		{
			fsop_deleteFile(fmt("%s/%s.gct", m_cheatDir.c_str(), id));
			fsop_deleteFile(fmt("%s/%s.txt", m_txtCheatDir.c_str(), id));
			m_gcfg2.remove(id, "cheat");
			m_gcfg2.remove(id, "hooktype");
			break;
		}
		else if (BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_cheatBtnBack))
				break;
			for (int i = 0; i < CHEATSPERPAGE; ++i)
				if (m_btnMgr.selected(m_cheatBtnItem[i]))
				{
					// handling code for clicked cheat
					m_cheatfile.sCheatSelected[(m_cheatSettingsPage-1)*CHEATSPERPAGE + i] = !m_cheatfile.sCheatSelected[(m_cheatSettingsPage-1)*CHEATSPERPAGE + i];
					_showCheatSettings();
				}
			
 			if (m_btnMgr.selected(m_cheatBtnApply))
			{
				bool selected = false;
				//checks if at least one cheat is selected
				for (unsigned int i=0; i < m_cheatfile.getCnt(); ++i)
				{
					if (m_cheatfile.sCheatSelected[i] == true) 
					{
						selected = true;
						break;
					}
				}
					
				if (selected)
				{
					m_cheatfile.createGCT(fmt("%s/%s.gct", m_cheatDir.c_str(), id)); 
					m_gcfg2.setOptBool(id, "cheat", 1);
					m_gcfg2.setInt(id, "hooktype", m_gcfg2.getInt(id, "hooktype", 1));
				}
				else
				{
					fsop_deleteFile(fmt("%s/%s.gct", m_cheatDir.c_str(), id));
					m_gcfg2.remove(id, "cheat");
					m_gcfg2.remove(id, "hooktype");
				}
				m_cheatfile.createTXT(fmt("%s/%s.txt", m_txtCheatDir.c_str(), id));
				break;
			}
			else if (m_btnMgr.selected(m_cheatBtnDownload))
			{
				_hideCheatSettings();
				bool dl_finished = false;
				while(!m_exit)
				{
					_mainLoopCommon();
					if((BTN_HOME_PRESSED || BTN_B_PRESSED) && dl_finished)
					{
						m_btnMgr.hide(m_wbfsPBar);
						m_btnMgr.hide(m_wbfsLblMessage);
						m_btnMgr.hide(m_wbfsLblDialog);
						break;
					}
					if(!dl_finished)
					{
						m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
						m_btnMgr.setText(m_wbfsLblMessage, L"0%");
						m_btnMgr.setText(m_wbfsLblDialog, L"");
						m_btnMgr.show(m_wbfsPBar);
						m_btnMgr.show(m_wbfsLblMessage);
						m_btnMgr.show(m_wbfsLblDialog);
						
						_start_pThread();
						int ret = _downloadCheatFileAsync();
						_stop_pThread();
						if(ret == -1)
							m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg27", L"Not enough memory!"));
						else if(ret == -2)
							m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg2", L"Network initialization failed!"));
						else if(ret == -3)
							m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg12", L"Download failed!"));
						else if(ret == -4)
							m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg36", L"No cheat file available to download."));
						else
							m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg14", L"Done."));
						dl_finished = true;
					}
				}
				txtavailable = m_cheatfile.openTxtfile(fmt("%s/%s.txt", m_txtCheatDir.c_str(), id));
				_showCheatSettings();
			}
		}
	}
	_hideCheatSettings();
}

void CMenu::_hideCheatSettings(bool instant)
{
	m_btnMgr.hide(m_cheatBtnBack, instant);
	m_btnMgr.hide(m_cheatBtnApply, instant);
	m_btnMgr.hide(m_cheatBtnDownload, instant);
	m_btnMgr.hide(m_cheatLblTitle, instant);

	m_btnMgr.hide(m_cheatLblPage, instant);
	m_btnMgr.hide(m_cheatBtnPageM, instant);
	m_btnMgr.hide(m_cheatBtnPageP, instant);
	
	for (int i=0;i<CHEATSPERPAGE;++i) {
		m_btnMgr.hide(m_cheatBtnItem[i], instant);
		m_btnMgr.hide(m_cheatLblItem[i], instant);
	}
	
	for(u8 i = 0; i < ARRAY_SIZE(m_cheatLblUser); ++i)
		if(m_cheatLblUser[i] != -1)
			m_btnMgr.hide(m_cheatLblUser[i], instant);
}

void CMenu::_showCheatSettings(void)
{
	if(txtavailable && m_cheatfile.getCnt() > 0)
		m_btnMgr.setText(m_cheatLblTitle, m_cheatfile.getGameName());
	else 
		m_btnMgr.setText(m_cheatLblTitle, L"");

	_setBg(m_cheatBg, m_cheatBg);
	m_btnMgr.show(m_cheatLblTitle);
	m_btnMgr.show(m_cheatBtnBack);

	for(u8 i = 0; i < ARRAY_SIZE(m_cheatLblUser); ++i)
		if(m_cheatLblUser[i] != -1)
			m_btnMgr.show(m_cheatLblUser[i]);

	if(m_cheatfile.getCnt() > 0)
	{
		// cheat found, show apply
		m_btnMgr.show(m_cheatBtnApply);
		m_btnMgr.show(m_cheatLblPage);
		m_btnMgr.show(m_cheatBtnPageM);
		m_btnMgr.show(m_cheatBtnPageP);
		m_btnMgr.setText(m_cheatLblPage, wfmt(L"%i / %i", m_cheatSettingsPage, (m_cheatfile.getCnt()+CHEATSPERPAGE-1)/CHEATSPERPAGE)); 
		
		// Show cheats if available, else hide
		for (u32 i=0; i < CHEATSPERPAGE; ++i)
		{
			// cheat in range?
			if (((m_cheatSettingsPage-1)*CHEATSPERPAGE + i + 1) <= m_cheatfile.getCnt()) 
			{
				wstringEx chtName;
				chtName.fromUTF8(m_cheatfile.getCheatName((m_cheatSettingsPage-1)*CHEATSPERPAGE + i));
				m_btnMgr.setText(m_cheatLblItem[i], chtName);
				m_btnMgr.setText(m_cheatBtnItem[i], _optBoolToString(m_cheatfile.sCheatSelected[(m_cheatSettingsPage-1)*CHEATSPERPAGE + i]));
				
				m_btnMgr.show(m_cheatLblItem[i], true);
				m_btnMgr.show(m_cheatBtnItem[i], true);
			}
			else
			{
				// cheat out of range, hide elements
				m_btnMgr.hide(m_cheatLblItem[i], true);
				m_btnMgr.hide(m_cheatBtnItem[i], true);
			}
		}
	}
	else if(!txtavailable)
	{
		// no cheat found, allow downloading
		m_btnMgr.show(m_cheatBtnDownload);
		m_btnMgr.setText(m_cheatLblItem[0], _t("cheat3", L"Cheat file for game not found."));
		m_btnMgr.show(m_cheatLblItem[0]);
	}
	else
	{
		m_btnMgr.setText(m_cheatLblItem[0], _t("dlmsg35", L"Downloaded cheat file has no cheats!"));
		m_btnMgr.show(m_cheatLblItem[0]);
	}
}


void CMenu::_initCheatSettingsMenu()
{
	_addUserLabels(m_cheatLblUser, ARRAY_SIZE(m_cheatLblUser), "CHEAT");
	m_cheatBg = _texture("CHEAT/BG", "texture", theme.bg, false);
	m_cheatLblTitle = _addLabel("CHEAT/TITLE", theme.titleFont, L"Cheats", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_cheatBtnBack = _addButton("CHEAT/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_cheatBtnApply = _addButton("CHEAT/APPLY_BTN", theme.btnFont, L"", 220, 400, 200, 48, theme.btnFontColor);
	m_cheatBtnDownload = _addButton("CHEAT/DOWNLOAD_BTN", theme.btnFont, L"", 470, 130, 150, 48, theme.btnFontColor);

	m_cheatLblPage = _addLabel("CHEAT/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_cheatBtnPageM = _addPicButton("CHEAT/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_cheatBtnPageP = _addPicButton("CHEAT/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);

	m_cheatLblItem[0] = _addLabel("CHEAT/ITEM_0", theme.lblFont, L"", 20, 125, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[0] = _addButton("CHEAT/ITEM_0_BTN", theme.btnFont, L"", 500, 130, 120, 48, theme.btnFontColor);
	m_cheatLblItem[1] = _addLabel("CHEAT/ITEM_1", theme.lblFont, L"", 20, 185, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[1] = _addButton("CHEAT/ITEM_1_BTN", theme.btnFont, L"", 500, 190, 120, 48, theme.btnFontColor);
	m_cheatLblItem[2] = _addLabel("CHEAT/ITEM_2", theme.lblFont, L"", 20, 245, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[2] = _addButton("CHEAT/ITEM_2_BTN", theme.btnFont, L"", 500, 250, 120, 48, theme.btnFontColor);
	m_cheatLblItem[3] = _addLabel("CHEAT/ITEM_3", theme.lblFont, L"", 20, 305, 460, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_cheatBtnItem[3] = _addButton("CHEAT/ITEM_3_BTN", theme.btnFont, L"", 500, 305, 120, 48, theme.btnFontColor);

	_setHideAnim(m_cheatLblTitle, "CHEAT/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_cheatBtnApply, "CHEAT/APPLY_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cheatBtnBack, "CHEAT/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cheatBtnDownload, "CHEAT/DOWNLOAD_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_cheatLblPage, "CHEAT/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cheatBtnPageM, "CHEAT/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_cheatBtnPageP, "CHEAT/PAGE_PLUS", 0, 0, 1.f, -1.f);
	
	for (int i=0;i<CHEATSPERPAGE;++i) {
		_setHideAnim(m_cheatLblItem[i], fmt("CHEAT/ITEM_%i", i), 50, 0, -2.f, 0.f);
		_setHideAnim(m_cheatBtnItem[i], fmt("CHEAT/ITEM_%i_BTN", i), -50, 0, 1.f, 0.f);
	}
	
	_hideCheatSettings();
	_textCheatSettings();
}

void CMenu::_textCheatSettings(void)
{
	m_btnMgr.setText(m_cheatBtnBack, _t("cheat1", L"Back"));
	m_btnMgr.setText(m_cheatBtnApply, _t("cheat2", L"Apply"));
	m_btnMgr.setText(m_cheatBtnDownload, _t("cfg4", L"Download"));
}
