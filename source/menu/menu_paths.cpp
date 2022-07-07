
#include "menu.hpp"

s16 m_pathsLblTitle;
s16 m_pathsLblPage;
s16 m_pathsBtnPageM;
s16 m_pathsBtnPageP;
s16 m_pathsBtnBack;	
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
	for(u8 i = 0; i < ARRAY_SIZE(m_pathsLblUser); ++i)
		if(m_pathsLblUser[i] != -1)
			m_btnMgr.hide(m_pathsLblUser[i], instant);
	
	_hideConfigButtons(instant);	
}

void CMenu::_showPaths(void)
{
	_setBg(m_pathsBg, m_pathsBg);
	m_btnMgr.show(m_pathsLblTitle);
	m_btnMgr.show(m_pathsBtnBack);
	for(u32 i = 0; i < ARRAY_SIZE(m_pathsLblUser); ++i)
		if(m_pathsLblUser[i] != -1)
			m_btnMgr.show(m_pathsLblUser[i]);
	
	m_btnMgr.setText(m_pathsLblPage, wfmt(L"%i / %i", paths_curPage, paths_Pages));
	m_btnMgr.show(m_pathsLblPage);
	m_btnMgr.show(m_pathsBtnPageM);
	m_btnMgr.show(m_pathsBtnPageP);
	
	_hideConfigButtons(true);

	m_btnMgr.setText(m_configBtn1, _t("dl16", L"Set"));
	m_btnMgr.setText(m_configBtn2, _t("dl16", L"Set"));
	m_btnMgr.setText(m_configBtn3, _t("dl16", L"Set"));
	m_btnMgr.setText(m_configBtn4, _t("dl16", L"Set"));
	
	m_btnMgr.show(m_configLbl1);
	m_btnMgr.show(m_configBtn1);
	m_btnMgr.show(m_configLbl2);
	m_btnMgr.show(m_configBtn2);
	m_btnMgr.show(m_configLbl3);
	m_btnMgr.show(m_configBtn3);
	if(paths_curPage == 1)
	{
		m_btnMgr.show(m_configLbl4);
		m_btnMgr.show(m_configBtn4);
		m_btnMgr.setText(m_configLbl1, _t("cfgp8", L"Box Covers"));
		m_btnMgr.setText(m_configLbl2, _t("cfgp2", L"Flat Covers"));
		m_btnMgr.setText(m_configLbl3, _t("cfgp9", L"Custom Banners"));
		m_btnMgr.setText(m_configLbl4, _t("cfgp4", L"Banners Cache"));
	}
	else
	{
		m_btnMgr.setText(m_configLbl1, _t("cfgp5", L"Wii Games"));
		m_btnMgr.setText(m_configLbl2, _t("cfgp6", L"GameCube Games"));
		m_btnMgr.setText(m_configLbl3, _t("cfgp7", L"Music"));
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
			if (m_btnMgr.selected(m_configBtn1))
			{
				_hidePaths();
				path = _FolderExplorer(m_cfg.getString("GENERAL", "dir_box_covers").c_str());
				if(strlen(path) > 0)
				{
					m_cfg.setString("GENERAL", "dir_box_covers", path);
					m_boxPicDir = path;
					//m_refreshGameList = true;
					_initCF();
				}
				_showPaths();
			}
			else if (m_btnMgr.selected(m_configBtn2))
			{
				_hidePaths();
				path = _FolderExplorer(m_cfg.getString("GENERAL", "dir_flat_covers").c_str());
				if(strlen(path) > 0)
				{
					m_cfg.setString("GENERAL", "dir_flat_covers", path);
					m_picDir = path;
					//m_refreshGameList = true;
					_initCF();
				}
				_showPaths();
			}
			else if (m_btnMgr.selected(m_configBtn3))
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
			else if (m_btnMgr.selected(m_configBtn4))
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
			if (m_btnMgr.selected(m_configBtn1))
			{
				_hidePaths();
				currentPartition = m_cfg.getInt(WII_DOMAIN, "partition", USB1);
				path = _FolderExplorer(fmt(wii_games_dir, DeviceName[currentPartition]));
				if(strchr(path, '/') != NULL)// path includes a folder and not just sd: or usb:
				{
					if(strncmp(path, "sd:", 3) == 0)
						m_cfg.setInt(WII_DOMAIN, "partition", 0);
					else
					{
						const char *partval = &path[3];
						m_cfg.setInt(WII_DOMAIN, "partition", atoi(partval));
					}
					string tmpPath = "%s" + string(strchr(path, ':'));
					m_cfg.setString(WII_DOMAIN, "wii_games_dir", tmpPath);
					memset(wii_games_dir, 0, sizeof(wii_games_dir));
					strncpy(wii_games_dir, tmpPath.c_str(), sizeof(wii_games_dir) - 1);
					m_cfg.setBool(WII_DOMAIN, "update_cache", true);
					if(m_current_view & COVERFLOW_WII)
						m_refreshGameList = true;
				}
				_showPaths();
			}
			else if (m_btnMgr.selected(m_configBtn2))
			{
				_hidePaths();
				currentPartition = m_cfg.getInt(GC_DOMAIN, "partition", USB1);
				path = _FolderExplorer(fmt(gc_games_dir, DeviceName[currentPartition]));
				if(strchr(path, '/') != NULL)// path includes a folder and not just sd: or usb:
				{
					if(strncmp(path, "sd:", 3) == 0)
						m_cfg.setInt(GC_DOMAIN, "partition", 0);
					else
					{
						//might add error check to make sure its first partition and FAT only
						const char *partval = &path[3];
						m_cfg.setInt(GC_DOMAIN, "partition", atoi(partval));
					}
					string tmpPath = "%s" + string(strchr(path, ':'));
					m_cfg.setString(GC_DOMAIN, "gc_games_dir", tmpPath);
					memset(gc_games_dir, 0, sizeof(gc_games_dir));
					strncpy(gc_games_dir, tmpPath.c_str(), sizeof(gc_games_dir) - 1);
					m_cfg.setBool(GC_DOMAIN, "update_cache", true);
					if(m_current_view & COVERFLOW_GAMECUBE)
						m_refreshGameList = true;
				}
				_showPaths();
			}
			if (m_btnMgr.selected(m_configBtn3))
			{
				_hidePaths();
				path = _FolderExplorer(m_cfg.getString("GENERAL", "dir_music").c_str());
				if(strlen(path) > 0)
				{
					m_cfg.setString("GENERAL", "dir_music", path);
					m_musicDir = path;
					MusicPlayer.Init(m_cfg, m_musicDir, fmt("%s/music", m_themeDataDir.c_str()));
					m_music_info = m_cfg.getBool("GENERAL", "display_music_info", false);
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
	m_pathsLblTitle = _addLabel("PATHS/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_pathsLblPage = _addLabel("PATHS/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_pathsBtnPageM = _addPicButton("PATHS/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_pathsBtnPageP = _addPicButton("PATHS/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_pathsBtnBack = _addButton("PATHS/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	
	_setHideAnim(m_pathsLblTitle, "PATHS/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_pathsBtnBack, "PATHS/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pathsLblPage, "PATHS/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pathsBtnPageM, "PATHS/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_pathsBtnPageP, "PATHS/PAGE_PLUS", 0, 0, 1.f, -1.f);

	_hidePaths(true);
	_textPaths();
}

void CMenu::_textPaths(void)
{
	m_btnMgr.setText(m_pathsLblTitle, _t("cfgd4", L"Path Manager"));
	m_btnMgr.setText(m_pathsBtnBack, _t("cfg10", L"Back"));
}
