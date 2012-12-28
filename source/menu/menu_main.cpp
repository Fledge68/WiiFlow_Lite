
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>

#include "menu.hpp"
#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gui/GameTDB.hpp"
#include "loader/alt_ios.h"
#include "loader/cios.h"
#include "loader/disc.h"
#include "loader/nk.h"
#include "loader/wbfs.h"
#include "loader/wdvd.h"
#include "network/gcard.h"

extern const u8 btnconfig_png[];
extern const u8 btnconfigs_png[];
extern const u8 btninfo_png[];
extern const u8 btninfos_png[];
extern const u8 btnquit_png[];
extern const u8 btnquits_png[];
extern const u8 btnnext_png[];
extern const u8 btnnexts_png[];
extern const u8 btnprev_png[];
extern const u8 btnprevs_png[];
extern const u8 btnchannel_png[];
extern const u8 btnchannels_png[];
extern const u8 btnusb_png[];
extern const u8 btnusbs_png[];
extern const u8 btndml_png[];
extern const u8 btndmls_png[];
extern const u8 btnemu_png[];
extern const u8 btnemus_png[];
extern const u8 btndvd_png[];
extern const u8 btndvds_png[];
extern const u8 favoriteson_png[];
extern const u8 favoritesons_png[];
extern const u8 favoritesoff_png[];
extern const u8 favoritesoffs_png[];

extern const u8 btnhomebrew_png[];
extern const u8 btnhomebrews_png[];

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

void CMenu::_hideMain(bool instant)
{
	m_btnMgr.hide(m_mainBtnNext, instant);
	m_btnMgr.hide(m_mainBtnPrev, instant);
	m_btnMgr.hide(m_mainBtnConfig, instant);
	m_btnMgr.hide(m_mainBtnInfo, instant);
	m_btnMgr.hide(m_mainBtnQuit, instant);
	m_btnMgr.hide(m_mainBtnHomebrew, instant);
	m_btnMgr.hide(m_mainBtnChannel, instant);
	m_btnMgr.hide(m_mainBtnUsb, instant);
	m_btnMgr.hide(m_mainBtnDML, instant);
	m_btnMgr.hide(m_mainBtnEmu, instant);
	m_btnMgr.hide(m_mainBtnDVD, instant);
	m_btnMgr.hide(m_mainBtnInit, instant);
	m_btnMgr.hide(m_mainBtnInit2, instant);
	m_btnMgr.hide(m_mainLblInit, instant);
	m_btnMgr.hide(m_mainBtnFavoritesOn, instant);
	m_btnMgr.hide(m_mainBtnFavoritesOff, instant);
	m_btnMgr.hide(m_mainLblLetter, instant);
	m_btnMgr.hide(m_mainLblNotice, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_mainLblUser); ++i)
		if(m_mainLblUser[i] != -1)
			m_btnMgr.hide(m_mainLblUser[i], instant);
}

static bool show_homebrew = true;
static bool parental_homebrew = false;
static bool show_channel = true;
static bool show_emu = true;

void CMenu::_showMain(void)
{
	_hideWaitMessage();
#ifdef SHOWMEM	
	m_btnMgr.show(m_mem2FreeSize);
#endif
	m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480),
	m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
	_setBg(m_gameBg, m_gameBgLQ);
	m_btnMgr.show(m_mainBtnInfo);
	m_btnMgr.show(m_mainBtnConfig);
	m_btnMgr.show(m_mainBtnQuit);

	switch(m_current_view)
	{
		case COVERFLOW_DML:
			if(show_channel)
				m_btnMgr.show(m_mainBtnChannel);
			else if(show_emu)
				m_btnMgr.show(m_mainBtnEmu);
			else if(show_homebrew)
				m_btnMgr.show(m_mainBtnHomebrew);
			else
				m_btnMgr.show(m_mainBtnUsb);
			break;
		case COVERFLOW_CHANNEL:
			if(show_emu)
				m_btnMgr.show(m_mainBtnEmu);
			else if (show_homebrew && (parental_homebrew || !m_locked))
				m_btnMgr.show(m_mainBtnHomebrew);
			else
				m_btnMgr.show(m_mainBtnUsb);
			break;
		case COVERFLOW_HOMEBREW:
			m_btnMgr.show(m_mainBtnUsb);
			break;
		case COVERFLOW_PLUGIN:
			if (show_homebrew && (parental_homebrew || !m_locked))
				m_btnMgr.show(m_mainBtnHomebrew);
			else
				m_btnMgr.show(m_mainBtnUsb);
			break;
		default:
			if(m_show_dml || m_devo_installed)
				m_btnMgr.show(m_mainBtnDML);
			else if (show_channel)
				m_btnMgr.show(m_mainBtnChannel);
			else if(show_emu)
				m_btnMgr.show(m_mainBtnEmu);
			else if(show_homebrew && (parental_homebrew || !m_locked))
				m_btnMgr.show(m_mainBtnHomebrew);
			else
				m_btnMgr.show(m_mainBtnUsb);
			break;
	}

	for(u8 i = 0; i < ARRAY_SIZE(m_mainLblUser); ++i)
		if(m_mainLblUser[i] != -1)
			m_btnMgr.show(m_mainLblUser[i]);

	if(m_gameList.empty())
	{
		switch(m_current_view)
		{
			case COVERFLOW_USB:
			case COVERFLOW_DML:
				m_btnMgr.setText(m_mainLblInit, _t("main2", L"Welcome to WiiFlow. I have not found any games. Click Install to install games, or Select partition to select your partition type."), true);
				m_btnMgr.show(m_mainBtnInit);
				m_btnMgr.show(m_mainBtnInit2);
				m_btnMgr.show(m_mainLblInit);
				break;
			case COVERFLOW_CHANNEL:
				if(NANDemuView)
				{
					_hideMain();
					if(!_AutoCreateNand())
						m_cfg.setBool(CHANNEL_DOMAIN, "disable", true);
					_loadList();
					_showMain();
					_initCF();
				}
				break;
			case COVERFLOW_HOMEBREW:
				m_btnMgr.setText(m_mainLblInit, _t("main4", L"Welcome to WiiFlow. I have not found any homebrew apps. Select partition to select your partition type."), true);
				m_btnMgr.show(m_mainBtnInit2);
				m_btnMgr.show(m_mainLblInit);
				break;
			case COVERFLOW_PLUGIN:
				m_btnMgr.setText(m_mainLblInit, _t("main5", L"Welcome to WiiFlow. I have not found any plugins. Select partition to select your partition type."), true);
				m_btnMgr.show(m_mainBtnInit2);
				m_btnMgr.show(m_mainLblInit);
				break;
		}
	}
}

void CMenu::LoadView(void)
{
	_hideMain(true);
	CoverFlow.clear();
	if(!m_vid.showingWaitMessage())
		_showWaitMessage();
	m_favorites = false;
	if (m_cfg.getBool("GENERAL", "save_favorites_mode", false))
		m_favorites = m_cfg.getBool(_domainFromView(), "favorites", false);
	_loadList();
	_showMain();
	_initCF();
	_loadCFLayout(m_cfg.getInt(_domainFromView(), "last_cf_mode", 1));
	CoverFlow.applySettings();

	const char *mode = (m_current_view == COVERFLOW_CHANNEL && m_cfg.getBool(CHANNEL_DOMAIN, "disable", true)) 
		? CHANNEL_DOMAIN : DeviceName[currentPartition];

	m_showtimer=60;
	char gui_name[20];
	snprintf(gui_name, sizeof(gui_name), "%s [%s]", _domainFromView(), upperCase(mode).c_str());
	m_btnMgr.setText(m_mainLblNotice, (string)gui_name);
	m_btnMgr.show(m_mainLblNotice);
}

void CMenu::exitHandler(int ExitTo)
{
	m_exit = true;
	if(ExitTo == EXIT_TO_BOOTMII) //Bootmii, check that the files are there, or ios will hang.
	{
		struct stat dummy;
		if(!DeviceHandle.IsInserted(SD) || 
		stat("sd:/bootmii/armboot.bin", &dummy) != 0 || 
		stat("sd:/bootmii/ppcboot.elf", &dummy) != 0)
			ExitTo = EXIT_TO_HBC;
	}
	if(ExitTo != WIIFLOW_DEF)
		Sys_ExitTo(ExitTo);
}

int CMenu::main(void)
{
	wstringEx curLetter;
	string prevTheme = m_cfg.getString("GENERAL", "theme", "default");
	parental_homebrew = m_cfg.getBool(HOMEBREW_DOMAIN, "parental", false);	
	show_homebrew = !m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false);
	show_channel = !m_cfg.getBool("GENERAL", "hidechannel", false);
	show_emu = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
	bool dpad_mode = m_cfg.getBool("GENERAL", "dpad_mode", false);
	bool b_lr_mode = m_cfg.getBool("GENERAL", "b_lr_mode", false);
	bool use_grab = m_cfg.getBool("GENERAL", "use_grab", false);
	bool bheld = false;
	bool bUsed = false;

	m_reload = false;
	u32 disc_check = 0;
	int done = 0;

	SetupInput(true);
	GameTDB m_gametdb; 
 	m_gametdb.OpenFile(fmt("%s/wiitdb.xml", m_settingsDir.c_str()));
	m_GameTDBLoaded = false;
 	if(m_gametdb.IsLoaded())
	{
		m_GameTDBLoaded = true;
		m_gametdb.CloseFile();
	}
	if(m_Emulator_boot)
		m_current_view = COVERFLOW_PLUGIN;

	if(m_cfg.getBool("GENERAL", "update_cache", false))
	{
		UpdateCache();
		//m_gameList.Update();
	}
	LoadView();
	if(m_cfg.getBool("GENERAL", "startup_menu", false)) 
	{
		_hideMain();
		if(!_Source())
			LoadView();
		else
			_showMain();
		if(BTN_B_HELD)
			bUsed = true;
	}

	while(!m_exit)
	{
		/* IMPORTANT check if a disc is inserted */
		WDVD_GetCoverStatus(&disc_check);
		/* Main Loop */
		_mainLoopCommon(true);
		if(bheld && !BTN_B_HELD)
		{
			bheld = false;
			if(bUsed)
			{
				bUsed = false;
			}
			else
			{
				_hideMain();
				if(!_Source()) //Different source selected
					LoadView();
				else
					_showMain();
				if(BTN_B_HELD)
					bUsed = true;
				continue;
			}
		}
		if(dpad_mode && (BTN_UP_PRESSED || BTN_DOWN_PRESSED || BTN_LEFT_PRESSED || BTN_RIGHT_PRESSED) && (m_btnMgr.selected(m_mainBtnChannel) || m_btnMgr.selected(m_mainBtnUsb) || m_btnMgr.selected(m_mainBtnDML) || m_btnMgr.selected(m_mainBtnHomebrew) || m_btnMgr.selected(m_mainBtnEmu)))
		{
			u32 lastView = m_current_view;
			if(BTN_UP_PRESSED) 
				m_current_view = COVERFLOW_USB;
			else if(BTN_DOWN_PRESSED && (m_show_dml ||m_devo_installed))
				m_current_view = COVERFLOW_DML;
			else if(BTN_LEFT_PRESSED && show_emu)
				m_current_view =  COVERFLOW_PLUGIN;
			else if(BTN_RIGHT_PRESSED && show_channel)
				m_current_view = COVERFLOW_CHANNEL;
			if(lastView == m_current_view) 
				m_current_view = COVERFLOW_HOMEBREW;
			LoadView();
			continue;
		}
		if(BTN_HOME_PRESSED)
		{
			_hideMain();
			if(_Home()) //exit wiiflow
				break;
			_showMain();
			if(BTN_B_HELD)
				bUsed = true;
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_mainBtnPrev))
				CoverFlow.pageUp();
		 	else if(m_btnMgr.selected(m_mainBtnNext))
				CoverFlow.pageDown();
			else if(m_btnMgr.selected(m_mainBtnQuit))
			{
				_hideMain();
				if(_Home()) //exit wiiflow
					break;
				_showMain();
				if(BTN_B_HELD)
					bUsed = true;
			}
			else if(m_btnMgr.selected(m_mainBtnChannel) || m_btnMgr.selected(m_mainBtnUsb) || m_btnMgr.selected(m_mainBtnDML) || m_btnMgr.selected(m_mainBtnHomebrew) || m_btnMgr.selected(m_mainBtnEmu))
			{
				if(m_current_view == COVERFLOW_USB) 
					m_current_view = (m_show_dml || m_devo_installed) ? COVERFLOW_DML : (show_channel ? COVERFLOW_CHANNEL : (show_emu ? COVERFLOW_PLUGIN : ((show_homebrew && (parental_homebrew || !m_locked)) ? COVERFLOW_HOMEBREW : COVERFLOW_USB)));
				else if(m_current_view == COVERFLOW_DML)
					m_current_view = show_channel ? COVERFLOW_CHANNEL : ((show_emu ? COVERFLOW_PLUGIN : (show_homebrew && (parental_homebrew || !m_locked)) ? COVERFLOW_HOMEBREW : COVERFLOW_USB));
				else if(m_current_view == COVERFLOW_CHANNEL)
					m_current_view = (show_emu ? COVERFLOW_PLUGIN : (show_homebrew && (parental_homebrew || !m_locked)) ? COVERFLOW_HOMEBREW : COVERFLOW_USB);
				else if(m_current_view == COVERFLOW_PLUGIN)
					m_current_view = (show_homebrew && (parental_homebrew || !m_locked)) ? COVERFLOW_HOMEBREW : COVERFLOW_USB;
				else if(m_current_view == COVERFLOW_HOMEBREW)
					m_current_view = COVERFLOW_USB;
				LoadView();
			}
			else if(m_btnMgr.selected(m_mainBtnInit))
			{
				if(!m_locked)
				{
					_hideMain();
					_wbfsOp(CMenu::WO_ADD_GAME);
					if(prevTheme != m_cfg.getString("GENERAL", "theme"))
					{
						m_reload = true;
						break;
					}
					_showMain();
					if(BTN_B_HELD)
						bUsed = true;
				}
			}
			else if(m_btnMgr.selected(m_mainBtnInit2))
			{
				_hideMain();
				_config(1);
				if(prevTheme != m_cfg.getString("GENERAL", "theme"))
				{
					m_reload = true;
					break;
				}
				_showMain();
				if(BTN_B_HELD)
					bUsed = true;
			}
			else if(m_btnMgr.selected(m_mainBtnConfig))
			{
				_hideMain();
				_config(1);
				if(prevTheme != m_cfg.getString("GENERAL", "theme"))
				{
					m_reload = true;
					break;
				}
				_showMain();
				if(BTN_B_HELD)
					bUsed = true;
			}
			else if(m_btnMgr.selected(m_mainBtnInfo))
			{
				_hideMain();
				_about(true);
				_showMain();
				if(BTN_B_HELD)
					bUsed = true;
			}
			else if(m_btnMgr.selected(m_mainBtnDVD))
			{
				/* Cleanup for Disc Booter */
				_hideMain(true);
				CoverFlow.clear();
				_showWaitMessage();
				m_gameSound.Stop();
				CheckGameSoundThread();
				/* Create Fake Header */
				dir_discHdr hdr;
				memset(&hdr, 0, sizeof(dir_discHdr));
				memcpy(&hdr.id, "dvddvd", 6);
				/* Boot the Disc */
				_launchGame(&hdr, true);
				_showMain();
				if(BTN_B_HELD)
					bUsed = true;
			}
			else if(m_btnMgr.selected(m_mainBtnFavoritesOn) || m_btnMgr.selected(m_mainBtnFavoritesOff))
			{
				m_favorites = !m_favorites;
				m_cfg.setBool(_domainFromView(), "favorites", m_favorites);
				_initCF();
			}
			else if(!CoverFlow.empty() && CoverFlow.select())
			{
				_hideMain();
				_game(BTN_B_HELD);
				if(m_exit)
					break;
				if(BTN_B_HELD)
					bUsed = true;
				CoverFlow.cancel();
				_showMain();
			}
		}
		else if(BTN_B_PRESSED)
		{
			//Events to Show Categories
			if(m_btnMgr.selected(m_mainBtnFavoritesOn) || m_btnMgr.selected(m_mainBtnFavoritesOff))
			{
				// Event handler to show categories for selection
				_hideMain();
				_CategorySettings();
				if(BTN_B_HELD)
					bUsed = true;
				_showMain();
				_initCF();
			}
			//Events to Switch off/on nand emu
			else if(m_btnMgr.selected(m_mainBtnChannel) || m_btnMgr.selected(m_mainBtnUsb) || m_btnMgr.selected(m_mainBtnDML)|| m_btnMgr.selected(m_mainBtnEmu) || m_btnMgr.selected(m_mainBtnHomebrew))
			{
				if(m_cfg.getBool("GENERAL", "b_on_mode_to_source", false))
				{
					_hideMain();
					if(!_Source()) //Different source selected
						LoadView();
					else
						_showMain();
					if(BTN_B_HELD)
						bUsed = true;
					continue;
				}
				else if(!neek2o())
				{
					bUsed = true;
					m_cfg.setBool(CHANNEL_DOMAIN, "disable", !m_cfg.getBool(CHANNEL_DOMAIN, "disable", true));
					gprintf("EmuNand is %s\n", m_cfg.getBool(CHANNEL_DOMAIN, "disable", true) ? "Disabled" : "Enabled");
					m_current_view = COVERFLOW_CHANNEL;
					LoadView();
				}
			}
			else if(m_btnMgr.selected(m_mainBtnNext) || m_btnMgr.selected(m_mainBtnPrev))
			{
				bUsed = true;
				const char *domain = _domainFromView();
				int sorting = m_cfg.getInt(domain, "sort", SORT_ALPHA);
				if (sorting != SORT_ALPHA && sorting != SORT_PLAYERS && sorting != SORT_WIFIPLAYERS && sorting != SORT_GAMEID)
				{
					CoverFlow.setSorting((Sorting)SORT_ALPHA);
					m_cfg.setInt(domain, "sort", SORT_ALPHA);
				}
				wchar_t c[2] = {0, 0};
				m_btnMgr.selected(m_mainBtnPrev) ? CoverFlow.prevLetter(c) : CoverFlow.nextLetter(c);
				m_showtimer = 60;
				curLetter.clear();
				curLetter = wstringEx(c);

				if(sorting == SORT_ALPHA)
				{
					m_btnMgr.setText(m_mainLblLetter, curLetter);
					m_btnMgr.show(m_mainLblLetter);
				}
				else
				{
					curLetter = _getNoticeTranslation(sorting, curLetter);
					m_btnMgr.setText(m_mainLblNotice, curLetter);
					m_btnMgr.show(m_mainLblNotice);
				}
			}
		}
		else if(WROLL_LEFT)
		{
			CoverFlow.left();
			bUsed = true;
		}
		else if(WROLL_RIGHT)
		{
			CoverFlow.right();
			bUsed = true;
		}
		if(!BTN_B_HELD)
		{
			if(BTN_UP_REPEAT || RIGHT_STICK_UP)
				CoverFlow.up();
			else if(BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT)
				CoverFlow.right();
			else if(BTN_DOWN_REPEAT ||  RIGHT_STICK_DOWN)
				CoverFlow.down();
			else if(BTN_LEFT_REPEAT || RIGHT_STICK_LEFT)
				CoverFlow.left();
			else if(BTN_1_PRESSED || BTN_2_PRESSED)
			{
				if (!m_btnMgr.selected(m_mainBtnQuit))
				{
					const char *domain = _domainFromView();
					s8 direction = BTN_1_PRESSED ? 1 : -1;
					int cfVersion = 1+loopNum((m_cfg.getInt(domain, "last_cf_mode", 1)-1) + direction, m_numCFVersions);
					_loadCFLayout(cfVersion);
					CoverFlow.applySettings();
					m_cfg.setInt(domain, "last_cf_mode", cfVersion);
				}
			}
			else if(BTN_MINUS_PRESSED)
			{
				if(b_lr_mode)
					MusicPlayer.Previous();
				else
					CoverFlow.pageUp();
			}
			else if(BTN_PLUS_PRESSED)
			{
				if(b_lr_mode)
					MusicPlayer.Next();
				else
					CoverFlow.pageDown();
			}
		}
		else
		{
			bheld = true;
			const char *domain = _domainFromView();
			//Search by Alphabet
			if(BTN_DOWN_PRESSED || BTN_UP_PRESSED)
			{
				bUsed = true;
				int sorting = m_cfg.getInt(domain, "sort", SORT_ALPHA);
				if(sorting != SORT_ALPHA && sorting != SORT_PLAYERS && sorting != SORT_WIFIPLAYERS && sorting != SORT_GAMEID)
				{
					CoverFlow.setSorting((Sorting)SORT_ALPHA);
					m_cfg.setInt(domain, "sort", SORT_ALPHA);
				}
				wchar_t c[2] = {0, 0};
				BTN_UP_PRESSED ? CoverFlow.prevLetter(c) : CoverFlow.nextLetter(c);

				curLetter.clear();
				curLetter = wstringEx(c);

				m_showtimer = 60;
				if(sorting == SORT_ALPHA)
				{
					m_btnMgr.setText(m_mainLblLetter, curLetter);
					m_btnMgr.show(m_mainLblLetter);
				}
				else
				{
					curLetter = _getNoticeTranslation(sorting, curLetter);
					m_btnMgr.setText(m_mainLblNotice, curLetter);
					m_btnMgr.show(m_mainLblNotice);
				}
			}
			else if(BTN_LEFT_PRESSED)
			{
				bUsed = true;
				if(b_lr_mode)
					CoverFlow.pageUp();
				else
					MusicPlayer.Previous();
			}
			else if(BTN_RIGHT_PRESSED)
			{
				bUsed = true;
				if(b_lr_mode)
					CoverFlow.pageDown();
				else
					MusicPlayer.Next();
			}
			else if(BTN_PLUS_PRESSED && !m_locked)
			{
				bUsed = true;
				u32 sort = 0;
				sort = loopNum((m_cfg.getInt(domain, "sort", 0)) + 1, SORT_MAX - 1);
				CoverFlow.setSorting((Sorting)sort);
				m_cfg.setInt(domain, "sort", sort);
				wstringEx curSort ;
				if(sort == SORT_ALPHA)
					curSort = m_loc.getWString(m_curLanguage, "alphabetically", L"Alphabetically");
				else if(sort == SORT_PLAYCOUNT)
					curSort = m_loc.getWString(m_curLanguage, "byplaycount", L"By Play Count");
				else if(sort == SORT_LASTPLAYED)
					curSort = m_loc.getWString(m_curLanguage, "bylastplayed", L"By Last Played");
				else if(sort == SORT_GAMEID)
					curSort = m_loc.getWString(m_curLanguage, "bygameid", L"By Game I.D.");
				else if(sort == SORT_ESRB)
					curSort = m_loc.getWString(m_curLanguage, "byesrb", L"By ESRB");
				else if(sort == SORT_WIFIPLAYERS)
					curSort = m_loc.getWString(m_curLanguage, "bywifiplayers", L"By Wifi Players");
				else if(sort == SORT_PLAYERS)
					curSort = m_loc.getWString(m_curLanguage, "byplayers", L"By Players");
				else if(sort == SORT_CONTROLLERS)
					curSort = m_loc.getWString(m_curLanguage, "bycontrollers", L"By Controllers");
				m_showtimer=60; 
				m_btnMgr.setText(m_mainLblNotice, curSort);
				m_btnMgr.show(m_mainLblNotice);
			}
			else if(BTN_MINUS_PRESSED && !m_locked)
			{
				bUsed = true;
				bool block = m_current_view == COVERFLOW_CHANNEL && (m_cfg.getBool(CHANNEL_DOMAIN, "disable", true) || neek2o());
				const char *partition = NULL;
				if(!block)
				{
					_showWaitMessage();
					_hideMain();
					_setPartition(1);
					partition = DeviceName[currentPartition];
				}
				else
					partition = "NAND";
				//gprintf("Next item: %s\n", partition);
				m_showtimer=60; 
				char gui_name[20];
				snprintf(gui_name, sizeof(gui_name), "%s [%s]", _domainFromView(), upperCase(partition).c_str());
				m_btnMgr.setText(m_mainLblNotice, (string)gui_name);
				m_btnMgr.show(m_mainLblNotice);
				if(!block)
				{
					_loadList();
					_showMain();
					_initCF();
				}
			}
		}

		if(done==0 && m_cfg.getBool("GENERAL", "category_on_start", false))
		{
			done = 1; //set done so it doesnt keep doing it
			// show categories menu
			_hideMain();
			_CategorySettings();
			if(BTN_B_HELD)
				bUsed = true;
			_showMain();
			_initCF();
		}
		if(use_grab)
			_getGrabStatus();
		if(m_showtimer > 0)
		{
			if(--m_showtimer == 0)
			{
				m_btnMgr.hide(m_mainLblLetter);
				m_btnMgr.hide(m_mainLblNotice);
			}
		}
		//zones, showing and hiding buttons
		if(!m_gameList.empty() && m_show_zone_prev)
			m_btnMgr.show(m_mainBtnPrev);
		else
			m_btnMgr.hide(m_mainBtnPrev);
		if(!m_gameList.empty() && m_show_zone_next)
			m_btnMgr.show(m_mainBtnNext);
		else
			m_btnMgr.hide(m_mainBtnNext);
		if(!m_gameList.empty() && m_show_zone_main)
		{
			m_btnMgr.show(m_mainLblUser[0]);
			m_btnMgr.show(m_mainLblUser[1]);
			m_btnMgr.show(m_mainBtnInfo);
			m_btnMgr.show(m_mainBtnConfig);
			m_btnMgr.show(m_mainBtnQuit);
			static bool change = m_favorites;
			m_btnMgr.show(m_favorites ? m_mainBtnFavoritesOn : m_mainBtnFavoritesOff, change != m_favorites);
			m_btnMgr.hide(m_favorites ? m_mainBtnFavoritesOff : m_mainBtnFavoritesOn, change != m_favorites);
			change = m_favorites;
		}
		else
		{
			m_btnMgr.hide(m_mainLblUser[0]);
			m_btnMgr.hide(m_mainLblUser[1]);
			if(!m_gameList.empty())
				m_btnMgr.hide(m_mainBtnConfig);
			m_btnMgr.hide(m_mainBtnInfo);
			m_btnMgr.hide(m_mainBtnQuit);
			m_btnMgr.hide(m_mainBtnFavoritesOn);
			m_btnMgr.hide(m_mainBtnFavoritesOff);
		}
		if(!m_cfg.getBool("GENERAL", "hideviews", false) && (m_gameList.empty() || m_show_zone_main2))
		{
			switch(m_current_view)
			{
				case COVERFLOW_DML:
					if(show_channel)
						m_btnMgr.show(m_mainBtnChannel);
					else if(show_emu)
						m_btnMgr.show(m_mainBtnEmu);
					else if(show_homebrew)
						m_btnMgr.show(m_mainBtnHomebrew);
					else 
						m_btnMgr.show(m_mainBtnUsb);
					break;
				case COVERFLOW_CHANNEL:
					if(show_emu)
						m_btnMgr.show(m_mainBtnEmu);
					else if(show_homebrew && (parental_homebrew || !m_locked))
						m_btnMgr.show(m_mainBtnHomebrew);
					else
						m_btnMgr.show(m_mainBtnUsb);
					break;
				case COVERFLOW_PLUGIN:
					if(show_homebrew && (parental_homebrew || !m_locked))
						m_btnMgr.show(m_mainBtnHomebrew);
					else
						m_btnMgr.show(m_mainBtnUsb);
					break;
				case COVERFLOW_HOMEBREW:
					m_btnMgr.show(m_mainBtnUsb);
					break;
				default:
					if(m_show_dml || m_devo_installed)
						m_btnMgr.show(m_mainBtnDML);
					else if(show_channel)
						m_btnMgr.show(m_mainBtnChannel);
					else if(show_emu)
						m_btnMgr.show(m_mainBtnEmu);
					else if(show_homebrew && (parental_homebrew || !m_locked))
						m_btnMgr.show(m_mainBtnHomebrew);
					else
						m_btnMgr.show(m_mainBtnUsb);
			}
			m_btnMgr.show(m_mainLblUser[2]);
			m_btnMgr.show(m_mainLblUser[3]);
		}
		else
		{
			m_btnMgr.hide(m_mainBtnHomebrew);
			m_btnMgr.hide(m_mainBtnChannel);
			m_btnMgr.hide(m_mainBtnUsb);
			m_btnMgr.hide(m_mainBtnDML);
			m_btnMgr.hide(m_mainBtnEmu);
			m_btnMgr.hide(m_mainLblUser[2]);
			m_btnMgr.hide(m_mainLblUser[3]);
		}
		if((disc_check & 0x2) && (m_gameList.empty() || m_show_zone_main3))
		{
			m_btnMgr.show(m_mainBtnDVD);
			m_btnMgr.show(m_mainLblUser[4]);
			m_btnMgr.show(m_mainLblUser[5]);
		}
		else
		{
			m_btnMgr.hide(m_mainBtnDVD);
			m_btnMgr.hide(m_mainLblUser[4]);
			m_btnMgr.hide(m_mainLblUser[5]);
		}
		for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		{
			if(WPadIR_Valid(chan) || (m_show_pointer[chan] && !WPadIR_Valid(chan)))
				CoverFlow.mouse(chan, m_cursor[chan].x(), m_cursor[chan].y());
			else
				CoverFlow.mouse(chan, -1, -1);
		}
	}
	ScanInput();
	if(m_reload || BTN_B_HELD)
	{
		CoverFlow.clear();
		_showWaitMessage();
		exitHandler(PRIILOADER_DEF); //Making wiiflow ready to boot something
		_launchHomebrew(fmt("%s/boot.dol", m_appDir.c_str()), m_homebrewArgs);
		return 0;
	}
	else if(Sys_GetExitTo() == EXIT_TO_SMNK2O || Sys_GetExitTo() == EXIT_TO_WFNK2O)
	{
		string emuPath;
		_FindEmuPart(emuPath, false);
		Sys_SetNeekPath(emuPath.size() > 1 ? emuPath.c_str() : NULL);
	}
	//gprintf("Saving configuration files\n");
	m_cfg.save();
	m_cat.save();
//	m_loc.save();
	return 0;
}

void CMenu::_initMainMenu()
{
	TexData texQuit;
	TexData texQuitS;
	TexData texInfo;
	TexData texInfoS;
	TexData texConfig;
	TexData texConfigS;
	TexData texDML;
	TexData texDMLs;
	TexData texEmu;
	TexData texEmus;
	TexData texDVD;
	TexData texDVDs;
	TexData texUsb;
	TexData texUsbs;
	TexData texChannel;
	TexData texChannels;
	TexData texHomebrew;
	TexData texHomebrews;
	TexData texPrev;
	TexData texPrevS;
	TexData texNext;
	TexData texNextS;
	TexData texFavOn;
	TexData texFavOnS;
	TexData texFavOff;
	TexData texFavOffS;
	TexData bgLQ;
	TexData emptyTex;

	m_mainBg = _texture("MAIN/BG", "texture", theme.bg, false);
	if(m_theme.loaded() && TexHandle.fromImageFile(bgLQ, fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("MAIN/BG", "texture").c_str()), GX_TF_CMPR, 64, 64) == TE_OK)
		m_mainBgLQ = bgLQ;

	TexHandle.fromPNG(texQuit, btnquit_png);
	TexHandle.fromPNG(texQuitS, btnquits_png);
	TexHandle.fromPNG(texInfo, btninfo_png);
	TexHandle.fromPNG(texInfoS, btninfos_png);
	TexHandle.fromPNG(texConfig, btnconfig_png);
	TexHandle.fromPNG(texConfigS, btnconfigs_png);
	TexHandle.fromPNG(texDVD, btndvd_png);
	TexHandle.fromPNG(texDVDs, btndvds_png);
	TexHandle.fromPNG(texUsb, btnusb_png);
	TexHandle.fromPNG(texUsbs, btnusbs_png);
	TexHandle.fromPNG(texDML, btndml_png);
	TexHandle.fromPNG(texDMLs, btndmls_png);
	TexHandle.fromPNG(texEmu, btnemu_png);
	TexHandle.fromPNG(texEmus, btnemus_png);
	TexHandle.fromPNG(texChannel, btnchannel_png);
	TexHandle.fromPNG(texChannels, btnchannels_png);
	TexHandle.fromPNG(texHomebrew, btnhomebrew_png);
	TexHandle.fromPNG(texHomebrews, btnhomebrews_png);
	TexHandle.fromPNG(texPrev, btnprev_png);
	TexHandle.fromPNG(texPrevS, btnprevs_png);
	TexHandle.fromPNG(texNext, btnnext_png);
	TexHandle.fromPNG(texNextS, btnnexts_png);
	TexHandle.fromPNG(texFavOn, favoriteson_png);
	TexHandle.fromPNG(texFavOnS, favoritesons_png);
	TexHandle.fromPNG(texFavOff, favoritesoff_png);
	TexHandle.fromPNG(texFavOffS, favoritesoffs_png);

	_addUserLabels(m_mainLblUser, ARRAY_SIZE(m_mainLblUser), "MAIN");

	m_mainBtnInfo = _addPicButton("MAIN/INFO_BTN", texInfo, texInfoS, 20, 400, 48, 48);
	m_mainBtnConfig = _addPicButton("MAIN/CONFIG_BTN", texConfig, texConfigS, 70, 400, 48, 48);
	m_mainBtnQuit = _addPicButton("MAIN/QUIT_BTN", texQuit, texQuitS, 570, 400, 48, 48);
	m_mainBtnChannel = _addPicButton("MAIN/CHANNEL_BTN", texChannel, texChannels, 520, 400, 48, 48);
	m_mainBtnHomebrew = _addPicButton("MAIN/HOMEBREW_BTN", texHomebrew, texHomebrews, 520, 400, 48, 48);
	m_mainBtnUsb = _addPicButton("MAIN/USB_BTN", texUsb, texUsbs, 520, 400, 48, 48);
	m_mainBtnDML = _addPicButton("MAIN/DML_BTN", texDML, texDMLs, 520, 400, 48, 48);
	m_mainBtnEmu = _addPicButton("MAIN/EMU_BTN", texEmu, texEmus, 520, 400, 48, 48);
	m_mainBtnDVD = _addPicButton("MAIN/DVD_BTN", texDVD, texDVDs, 470, 400, 48, 48);
	m_mainBtnNext = _addPicButton("MAIN/NEXT_BTN", texNext, texNextS, 540, 146, 80, 80);
	m_mainBtnPrev = _addPicButton("MAIN/PREV_BTN", texPrev, texPrevS, 20, 146, 80, 80);
	m_mainBtnInit = _addButton("MAIN/BIG_SETTINGS_BTN", theme.titleFont, L"", 72, 180, 496, 56, theme.titleFontColor);
	m_mainBtnInit2 = _addButton("MAIN/BIG_SETTINGS_BTN2", theme.titleFont, L"", 72, 290, 496, 56, theme.titleFontColor);
	m_mainLblInit = _addLabel("MAIN/MESSAGE", theme.lblFont, L"", 40, 40, 560, 140, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_mainBtnFavoritesOn = _addPicButton("MAIN/FAVORITES_ON", texFavOn, texFavOnS, 300, 400, 56, 56);
	m_mainBtnFavoritesOff = _addPicButton("MAIN/FAVORITES_OFF", texFavOff, texFavOffS, 300, 400, 56, 56);
	m_mainLblLetter = _addLabel("MAIN/LETTER", theme.titleFont, L"", 540, 40, 80, 80, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, emptyTex);
	m_mainLblNotice = _addLabel("MAIN/NOTICE", theme.titleFont, L"", 340, 40, 280, 80, theme.titleFontColor, FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE, emptyTex);
	m_mainLblCurMusic = _addLabel("MAIN/MUSIC", theme.btnFont, L"", 0, 20, 640, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
#ifdef SHOWMEM	
	m_mem2FreeSize = _addLabel("MEM2", theme.titleFont, L"", 40, 300, 480, 80, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, emptyTex);
#endif
	// 
	m_mainPrevZone.x = m_theme.getInt("MAIN/ZONES", "prev_x", -32);
	m_mainPrevZone.y = m_theme.getInt("MAIN/ZONES", "prev_y", -32);
	m_mainPrevZone.w = m_theme.getInt("MAIN/ZONES", "prev_w", 182);
	m_mainPrevZone.h = m_theme.getInt("MAIN/ZONES", "prev_h", 382);
	m_mainPrevZone.hide = m_theme.getBool("MAIN/ZONES", "prev_hide", true);
	
	m_mainNextZone.x = m_theme.getInt("MAIN/ZONES", "next_x", 490);
	m_mainNextZone.y = m_theme.getInt("MAIN/ZONES", "next_y", -32);
	m_mainNextZone.w = m_theme.getInt("MAIN/ZONES", "next_w", 182);
	m_mainNextZone.h = m_theme.getInt("MAIN/ZONES", "next_h", 382);
	m_mainNextZone.hide = m_theme.getBool("MAIN/ZONES", "next_hide", true);
	
	m_mainButtonsZone.x = m_theme.getInt("MAIN/ZONES", "buttons_x", -32);
	m_mainButtonsZone.y = m_theme.getInt("MAIN/ZONES", "buttons_y", 350);
	m_mainButtonsZone.w = m_theme.getInt("MAIN/ZONES", "buttons_w", 704);
	m_mainButtonsZone.h = m_theme.getInt("MAIN/ZONES", "buttons_h", 162);
	m_mainButtonsZone.hide = m_theme.getBool("MAIN/ZONES", "buttons_hide", true);

	m_mainButtonsZone2.x = m_theme.getInt("MAIN/ZONES", "buttons2_x", -32);
	m_mainButtonsZone2.y = m_theme.getInt("MAIN/ZONES", "buttons2_y", 350);
	m_mainButtonsZone2.w = m_theme.getInt("MAIN/ZONES", "buttons2_w", 704);
	m_mainButtonsZone2.h = m_theme.getInt("MAIN/ZONES", "buttons2_h", 162);
	m_mainButtonsZone2.hide = m_theme.getBool("MAIN/ZONES", "buttons2_hide", true);
	
	m_mainButtonsZone3.x = m_theme.getInt("MAIN/ZONES", "buttons3_x", -32);
	m_mainButtonsZone3.y = m_theme.getInt("MAIN/ZONES", "buttons3_y", 350);
	m_mainButtonsZone3.w = m_theme.getInt("MAIN/ZONES", "buttons3_w", 704);
	m_mainButtonsZone3.h = m_theme.getInt("MAIN/ZONES", "buttons3_h", 162);
	m_mainButtonsZone3.hide = m_theme.getBool("MAIN/ZONES", "buttons3_hide", true);
	//
	_setHideAnim(m_mainBtnNext, "MAIN/NEXT_BTN", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainBtnPrev, "MAIN/PREV_BTN", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainBtnConfig, "MAIN/CONFIG_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnInfo, "MAIN/INFO_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnQuit, "MAIN/QUIT_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnChannel, "MAIN/CHANNEL_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnHomebrew, "MAIN/HOMEBREW_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnUsb, "MAIN/USB_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnDML, "MAIN/DML_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnEmu, "MAIN/EMU_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnDVD, "MAIN/DVD_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOn, "MAIN/FAVORITES_ON", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOff, "MAIN/FAVORITES_OFF", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnInit, "MAIN/BIG_SETTINGS_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_mainBtnInit2, "MAIN/BIG_SETTINGS_BTN2", 0, 0, -2.f, 0.f);
	_setHideAnim(m_mainLblInit, "MAIN/MESSAGE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblLetter, "MAIN/LETTER", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblNotice, "MAIN/NOTICE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblCurMusic, "MAIN/MUSIC", 0, -100, 0.f, 0.f);
#ifdef SHOWMEM
	_setHideAnim(m_mem2FreeSize, "MEM2", 0, 0, 0.f, 0.f);
#endif
	_hideMain(true);
	_textMain();
}

void CMenu::_textMain(void)
{
	m_btnMgr.setText(m_mainBtnInit, _t("main1", L"Install Game"));
	m_btnMgr.setText(m_mainBtnInit2, _t("main3", L"Select Partition"));
}

wstringEx CMenu::_getNoticeTranslation(int sorting, wstringEx curLetter)
{
	if(sorting == SORT_PLAYERS)
		curLetter += m_loc.getWString(m_curLanguage, "players", L" Players");
	else if(sorting == SORT_WIFIPLAYERS)
		curLetter += m_loc.getWString(m_curLanguage, "wifiplayers", L" Wifi Players");
	else if(sorting == SORT_GAMEID)
	{
		switch(curLetter[0])
		{
			case L'C':
			{
				if(m_current_view != COVERFLOW_CHANNEL)
					curLetter = m_loc.getWString(m_curLanguage, "custom", L"Custom");
				else
					curLetter = m_loc.getWString(m_curLanguage, "commodore", L"Commodore 64");
				break;
			}
			case L'E':
			{
				curLetter = m_loc.getWString(m_curLanguage, "neogeo", L"Neo-Geo");
				break;
			}
			case L'F':
			{
				curLetter = m_loc.getWString(m_curLanguage, "nes", L"Nintendo");
				break;
			}
			case L'J':
			{
				curLetter = m_loc.getWString(m_curLanguage, "snes", L"Super Nintendo");
				break;
			}
			case L'L':
			{
				curLetter = m_loc.getWString(m_curLanguage, "mastersystem", L"Sega Master System");
				break;
			}
			case L'M':
			{
				curLetter = m_loc.getWString(m_curLanguage, "genesis", L"Sega Genesis");
				break;
			}
			case L'N':
			{
				curLetter = m_loc.getWString(m_curLanguage, "nintendo64", L"Nintendo64");
				break;
			}
			case L'P':
			{
				curLetter = m_loc.getWString(m_curLanguage, "turbografx16", L"TurboGrafx-16");
				break;
			}
			case L'Q':
			{
				curLetter = m_loc.getWString(m_curLanguage, "turbografxcd", L"TurboGrafx-CD");
				break;
			}
			case L'W':
			{
				curLetter = m_loc.getWString(m_curLanguage, "wiiware", L"WiiWare");
				break;
			}
			case L'H':
			{
				curLetter = m_loc.getWString(m_curLanguage, "wiichannels", L"Offical Wii Channels");
				break;
			}
			case L'R':
			case L'S':
			{
				curLetter = m_loc.getWString(m_curLanguage, "wii", L"Wii");
				break;
			}
			case L'D':
			{
				curLetter = m_loc.getWString(m_curLanguage, "homebrew", L"Homebrew");
				break;
			}
			default:
			{
				curLetter = m_loc.getWString(m_curLanguage, "unknown", L"Unknown");
				break;
			}
		}
	}
	
	return curLetter;
}

void CMenu::_setPartition(s8 direction)
{
	if(m_current_view == COVERFLOW_CHANNEL && NANDemuView == false)
		return;
	_cfNeedsUpdate();
	if(direction != 0)
	{
		u8 limiter = 0;
		bool NeedFAT = m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_DML;
		currentPartition = loopNum(currentPartition + direction, 8);
		int FS_Type = DeviceHandle.GetFSType(currentPartition);
		while(!DeviceHandle.IsInserted(currentPartition) || 
			(m_current_view != COVERFLOW_USB && FS_Type == PART_FS_WBFS) ||
			(NeedFAT && FS_Type != PART_FS_FAT))
		{
			currentPartition = loopNum(currentPartition + direction, 8);
			FS_Type = DeviceHandle.GetFSType(currentPartition);
			if(limiter > 10)
				break;
			limiter++;
		}
	}
	if(m_tempView)
		m_cfg.setInt(WII_DOMAIN, "savepartition", currentPartition);
	else
	{
		m_cfg.setInt(_domainFromView(), "partition", currentPartition);
		_checkForSinglePlugin();
		if(enabledPluginsCount == 1)
			m_cfg.setInt("PLUGINS/PARTITION", PluginMagicWord, currentPartition);
	}
}
