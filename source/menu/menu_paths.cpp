
#include "menu.hpp"

using namespace std;

s16 m_pathsLblTitle;
s16 m_pathsLblPage;
s16 m_pathsBtnPageM;
s16 m_pathsBtnPageP;
s16 m_pathsBtnBack;
	
s16 m_pathsLbl1;
s16 m_pathsLbl2;
s16 m_pathsLbl3;
s16 m_pathsLbl4;

s16 m_pathsBtn1;
s16 m_pathsBtn2;
s16 m_pathsBtn3;
s16 m_pathsBtn4;

s16 m_pathsLblUser[4];

TexData m_pathsBg;

u8 paths_curPage = 1;
u8 paths_Pages = 2;

void CMenu::_hidePaths(bool instant)
{
	m_btnMgr.hide(m_pathsLblTitle, instant);
	m_btnMgr.hide(m_pathsBtnBack, instant);
	m_btnMgr.hide(m_pathsLblPage, instant);
	m_btnMgr.hide(m_pathsBtnPageM, instant);
	m_btnMgr.hide(m_pathsBtnPageP, instant);
	m_btnMgr.hide(m_pathsLbl1, instant);
	m_btnMgr.hide(m_pathsBtn1, instant);
	m_btnMgr.hide(m_pathsLbl2, instant);
	m_btnMgr.hide(m_pathsBtn2, instant);
	m_btnMgr.hide(m_pathsLbl3, instant);
	m_btnMgr.hide(m_pathsBtn3, instant);
	m_btnMgr.hide(m_pathsLbl4, instant);
	m_btnMgr.hide(m_pathsBtn4, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_pathsLblUser); ++i)
		if(m_pathsLblUser[i] != -1)
			m_btnMgr.hide(m_pathsLblUser[i], instant);
}

void CMenu::_showPaths(void)
{
	m_btnMgr.hide(m_pathsLbl1, true);
	m_btnMgr.hide(m_pathsBtn1, true);
	m_btnMgr.hide(m_pathsLbl2, true);
	m_btnMgr.hide(m_pathsBtn2, true);
	m_btnMgr.hide(m_pathsLbl3, true);
	m_btnMgr.hide(m_pathsBtn3, true);
	m_btnMgr.hide(m_pathsLbl4, true);
	m_btnMgr.hide(m_pathsBtn4, true);
	_setBg(m_pathsBg, m_pathsBg);
	m_btnMgr.show(m_pathsLblTitle);
	m_btnMgr.show(m_pathsBtnBack);
	m_btnMgr.show(m_pathsLblPage);
	m_btnMgr.show(m_pathsBtnPageM);
	m_btnMgr.show(m_pathsBtnPageP);
	for(u32 i = 0; i < ARRAY_SIZE(m_pathsLblUser); ++i)
		if(m_pathsLblUser[i] != -1)
			m_btnMgr.show(m_pathsLblUser[i]);
	m_btnMgr.setText(m_pathsLblPage, wfmt(L"%i / %i", paths_curPage, paths_Pages));
	m_btnMgr.show(m_pathsLbl1);
	m_btnMgr.show(m_pathsBtn1);
	m_btnMgr.show(m_pathsLbl2);
	m_btnMgr.show(m_pathsBtn2);
	m_btnMgr.show(m_pathsLbl3);
	m_btnMgr.show(m_pathsBtn3);
	if(paths_curPage == 1)
	{
		m_btnMgr.show(m_pathsLbl4);
		m_btnMgr.show(m_pathsBtn4);
		m_btnMgr.setText(m_pathsLbl1, _t("cfgp8", L"Box Covers"));
		m_btnMgr.setText(m_pathsLbl2, _t("cfgp2", L"Flat Covers"));
		m_btnMgr.setText(m_pathsLbl3, _t("cfgp9", L"Custom Banners"));
		m_btnMgr.setText(m_pathsLbl4, _t("cfgp4", L"Banners Cache"));
	}
	else
	{
		m_btnMgr.setText(m_pathsLbl1, _t("cfgp5", L"Wii Games"));
		m_btnMgr.setText(m_pathsLbl2, _t("cfgp6", L"GameCube Games"));
		m_btnMgr.setText(m_pathsLbl3, _t("cfgp7", L"Music"));
	}
}

void CMenu::_Paths(void)
{
	const char *path;
	paths_curPage = 1;
	SetupInput();
	_showPaths();
	while(!m_exit)
	{
		_mainLoopCommon();
		if (BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_pathsBtnBack)))
			break;
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if (BTN_LEFT_PRESSED || BTN_MINUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_pathsBtnPageM)))
		{
			paths_curPage--;
			if(paths_curPage == 0) paths_curPage = paths_Pages;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_pathsBtnPageM);
			_showPaths();
		}
		else if (BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_pathsBtnPageP)))
		{
			paths_curPage++;
			if(paths_curPage > paths_Pages) paths_curPage = 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED) m_btnMgr.click(m_pathsBtnPageP);
			_showPaths();
		}
		else if (BTN_A_PRESSED && paths_curPage == 1)
		{
			if (m_btnMgr.selected(m_pathsBtn1))
			{
				_hidePaths();
				path = _FolderExplorer(m_cfg.getString("GENERAL", "dir_box_covers").c_str());
				if(strlen(path) > 0)
				{
					m_cfg.setString("GENERAL", "dir_box_covers", path);
					m_boxPicDir = path;
					m_load_view = true;
				}
				_showPaths();
			}
			else if (m_btnMgr.selected(m_pathsBtn2))
			{
				_hidePaths();
				path = _FolderExplorer(m_cfg.getString("GENERAL", "dir_flat_covers").c_str());
				if(strlen(path) > 0)
				{
					m_cfg.setString("GENERAL", "dir_flat_covers", path);
					m_picDir = path;
					m_load_view = true;
				}
				_showPaths();
			}
			else if (m_btnMgr.selected(m_pathsBtn3))
			{
				_hidePaths();
				path = _FolderExplorer(m_cfg.getString("GENERAL", "dir_custom_banners").c_str());
				if(strlen(path) > 0)
				{
					m_cfg.setString("GENERAL", "dir_custom_banners", path);
					m_customBnrDir = path;
				}
				_showPaths();
			}
			else if (m_btnMgr.selected(m_pathsBtn4))
			{
				_hidePaths();
				path = _FolderExplorer(m_cfg.getString("GENERAL", "dir_banner_cache").c_str());
				if(strlen(path) > 0)
				{
					m_cfg.setString("GENERAL", "dir_banner_cache", path);
					m_bnrCacheDir = path;
				}
				_showPaths();
			}
		}
		else if (BTN_A_PRESSED && paths_curPage == 2)
		{
			if (m_btnMgr.selected(m_pathsBtn1))
			{
				_hidePaths();
				currentPartition = m_cfg.getInt(WII_DOMAIN, "partition", USB1);
				string gameDir(fmt(wii_games_dir, DeviceName[currentPartition]));
				path = _FolderExplorer(gameDir.c_str());
				if(strlen(path) > 0)
				{
					if(strncmp(path, "sd:/", 4) == 0)
						m_cfg.setInt(WII_DOMAIN, "partition", 0);
					else
					{
						const char *partval = &path[3];
						m_cfg.setInt(WII_DOMAIN, "partition", atoi(partval));
					}
					char tmpPath[MAX_FAT_PATH];
					strcpy(tmpPath, "%s");
					strcat(tmpPath, strchr(path, ':'));
					m_cfg.setString(WII_DOMAIN, "wii_games_dir", tmpPath);
					memset(wii_games_dir, 0, 64);
					strncpy(wii_games_dir, tmpPath, 64);
					m_cfg.setBool(WII_DOMAIN, "update_cache", true);
					if(m_cfg.getBool(WII_DOMAIN, "source"))
						m_load_view = true;
				}
				_showPaths();
			}
			else if (m_btnMgr.selected(m_pathsBtn2))
			{
				_hidePaths();
				currentPartition = m_cfg.getInt(GC_DOMAIN, "partition", USB1);
				string gameDir(fmt(currentPartition == SD ? DML_DIR : m_DMLgameDir.c_str(), DeviceName[currentPartition]));
				path = _FolderExplorer(gameDir.c_str());
				if(strlen(path) > 0)
				{
					if(strncmp(path, "sd:/", 4) == 0)
						m_cfg.setInt(GC_DOMAIN, "partition", 0);
					else
					{
						const char *partval = &path[3];
						m_cfg.setInt(GC_DOMAIN, "partition", atoi(partval));
					}
					char tmpPath[MAX_FAT_PATH];
					strncpy(tmpPath, strchr(path, '/')+1, MAX_FAT_PATH-1);
					m_cfg.setString(GC_DOMAIN, "dir_usb_games", tmpPath);
					m_DMLgameDir = fmt("%%s:/%s", m_cfg.getString(GC_DOMAIN, "dir_usb_games", "games").c_str());
					m_cfg.setBool(GC_DOMAIN, "update_cache", true);
					if(m_cfg.getBool(GC_DOMAIN, "source"))
						m_load_view = true;
				}
				_showPaths();
			}
			else if (m_btnMgr.selected(m_pathsBtn3))
			{
				_hidePaths();
				path = _FolderExplorer(m_cfg.getString("GENERAL", "dir_music").c_str());
				if(strlen(path) > 0)
				{
					m_cfg.setString("GENERAL", "dir_music", path);
					m_musicDir = path;
					MusicPlayer.Init(m_cfg, m_musicDir, fmt("%s/music", m_themeDataDir.c_str()));

				}
				_showPaths();
			}
		}
	}
	_hidePaths();
}

void CMenu::_initPathsMenu()
{
	_addUserLabels(m_pathsLblUser, ARRAY_SIZE(m_pathsLblUser), "PATHS");
	m_pathsBg = _texture("PATHS/BG", "texture", theme.bg, false);	
	m_pathsLblTitle = _addTitle("PATHS/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_pathsLblPage = _addLabel("PATHS/PAGE_BTN", theme.btnFont, L"", 76, 400, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_pathsBtnPageM = _addPicButton("PATHS/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_pathsBtnPageP = _addPicButton("PATHS/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 400, 56, 56);
	m_pathsBtnBack = _addButton("PATHS/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	
	m_pathsLbl1 = _addLabel("PATHS/DIR1", theme.lblFont, L"", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_pathsBtn1 = _addButton("PATHS/DIR1_BTN", theme.btnFont, L"", 370, 130, 230, 56, theme.btnFontColor);
	m_pathsLbl2 = _addLabel("PATHS/DIR2", theme.lblFont, L"", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_pathsBtn2 = _addButton("PATHS/DIR2_BTN", theme.btnFont, L"", 370, 190, 230, 56, theme.btnFontColor);
	m_pathsLbl3 = _addLabel("PATHS/DIR3", theme.lblFont, L"", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_pathsBtn3 = _addButton("PATHS/DIR3_BTN", theme.btnFont, L"", 370, 250, 230, 56, theme.btnFontColor);
	m_pathsLbl4 = _addLabel("PATHS/DIR4", theme.lblFont, L"", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_pathsBtn4 = _addButton("PATHS/DIR4_BTN", theme.btnFont, L"", 370, 310, 230, 56, theme.btnFontColor);

	_setHideAnim(m_pathsLblTitle, "PATHS/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_pathsBtnBack, "PATHS/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_pathsLblPage, "PATHS/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pathsBtnPageM, "PATHS/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pathsBtnPageP, "PATHS/PAGE_PLUS", 0, 0, 1.f, -1.f);
	
	_setHideAnim(m_pathsLbl1, "PATHS/DIR1", 100, 0, -2.f, 0.f);
	_setHideAnim(m_pathsBtn1, "PATHS/DIR1_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pathsLbl2, "PATHS/DIR2", 100, 0, -2.f, 0.f);
	_setHideAnim(m_pathsBtn2, "PATHS/DIR2_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pathsLbl3, "PATHS/DIR3", 100, 0, -2.f, 0.f);
	_setHideAnim(m_pathsBtn3, "PATHS/DIR3_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pathsLbl4, "PATHS/DIR4", 100, 0, -2.f, 0.f);
	_setHideAnim(m_pathsBtn4, "PATHS/DIR4_BTN", 0, 0, 1.f, -1.f);
	
	_hidePaths(true);
	_textPaths();
}

void CMenu::_textPaths(void)
{
	m_btnMgr.setText(m_pathsLblTitle, _t("cfgd4", L"Path Manager"));
	m_btnMgr.setText(m_pathsBtnBack, _t("cfg10", L"Back"));
	m_btnMgr.setText(m_pathsBtn1, _t("dl16", L"Set"));
	m_btnMgr.setText(m_pathsBtn2, _t("dl16", L"Set"));
	m_btnMgr.setText(m_pathsBtn3, _t("dl16", L"Set"));
	m_btnMgr.setText(m_pathsBtn4, _t("dl16", L"Set"));
}
