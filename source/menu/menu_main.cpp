
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

static inline int loopNum(int i, int s)
{
	return (i + s) % s;
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
	m_btnMgr.hide(m_mainBtnWii, instant);
	m_btnMgr.hide(m_mainBtnGamecube, instant);
	m_btnMgr.hide(m_mainBtnPlugin, instant);
	m_btnMgr.hide(m_mainBtnDVD, instant);
	m_btnMgr.hide(m_mainBtnInstall, instant);
	m_btnMgr.hide(m_mainBtnSelPart, instant);
	m_btnMgr.hide(m_mainLblMessage, instant);
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
static bool show_plugin = true;
static bool show_gamecube = true;

void CMenu::_showMain(void)
{
start_main:
	_hideWaitMessage();
#ifdef SHOWMEM
	m_btnMgr.show(m_mem1FreeSize);
	m_btnMgr.show(m_mem2FreeSize);
#endif
	m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480),
	m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
	_setBg(m_mainBg, m_mainBgLQ);
	m_btnMgr.show(m_mainBtnInfo);
	m_btnMgr.show(m_mainBtnConfig);
	m_btnMgr.show(m_mainBtnQuit);

	switch(m_current_view)
	{
		case COVERFLOW_GAMECUBE:
			if(show_channel)
				m_btnMgr.show(m_mainBtnChannel);
			else if(show_plugin)
				m_btnMgr.show(m_mainBtnPlugin);
			else if(show_homebrew)
				m_btnMgr.show(m_mainBtnHomebrew);
			else
				m_btnMgr.show(m_mainBtnWii);
			break;
		case COVERFLOW_CHANNEL:
			if(show_plugin)
				m_btnMgr.show(m_mainBtnPlugin);
			else if(show_homebrew)
				m_btnMgr.show(m_mainBtnHomebrew);
			else
				m_btnMgr.show(m_mainBtnWii);
			break;
		case COVERFLOW_MAX:
		case COVERFLOW_HOMEBREW:
			m_btnMgr.show(m_mainBtnWii);
			break;
		case COVERFLOW_PLUGIN:
			if(show_homebrew)
				m_btnMgr.show(m_mainBtnHomebrew);
			else
				m_btnMgr.show(m_mainBtnWii);
			break;
		default:
			if(show_gamecube)
				m_btnMgr.show(m_mainBtnGamecube);
			else if(show_channel)
				m_btnMgr.show(m_mainBtnChannel);
			else if(show_plugin)
				m_btnMgr.show(m_mainBtnPlugin);
			else if(show_homebrew)
				m_btnMgr.show(m_mainBtnHomebrew);
			else
				m_btnMgr.show(m_mainBtnWii);
			break;
	}

	for(u8 i = 0; i < ARRAY_SIZE(m_mainLblUser); ++i)
		if(m_mainLblUser[i] != -1)
			m_btnMgr.show(m_mainLblUser[i]);

	if(m_gameList.empty())
	{
		switch(m_current_view)
		{
			case COVERFLOW_WII:
			case COVERFLOW_GAMECUBE:
				m_btnMgr.setText(m_mainLblMessage, _t("main2", L"No games found! Please select partition to change the device/partition or click Install to install a game."));
			//	m_btnMgr.setText(m_mainLblMessage, _t("main2", L"Welcome to WiiFlow. I have not found any games. Click Install to install games, or Select partition to select your partition type."));
				m_btnMgr.show(m_mainBtnInstall);
				m_btnMgr.show(m_mainBtnSelPart);
				m_btnMgr.show(m_mainLblMessage);
				break;
			case COVERFLOW_CHANNEL:
				if(NANDemuView)
				{
					_hideMain();
					if(!_AutoCreateNand())
					{
						while(NANDemuView)//keep calling _setPartition till NANDemuView is false and CHANNEL_DOMAIN, "emu_nand", false
							_setPartition(1);
					}
					_loadList();
					goto start_main;
				}
				break;
			case COVERFLOW_HOMEBREW:
				m_btnMgr.setText(m_mainLblMessage, _t("main4", L"No homebrew apps found! Try changing the partition to select the correct device/partition."));
			//	m_btnMgr.setText(m_mainLblMessage, _t("main4", L"Welcome to WiiFlow. I have not found any homebrew apps. Select partition to select your partition type."));
				m_btnMgr.show(m_mainBtnSelPart);
				m_btnMgr.show(m_mainLblMessage);
				break;
			case COVERFLOW_PLUGIN:
				m_btnMgr.setText(m_mainLblMessage, _t("main5", L"No roms/items for your plugin found! Try changing the partition to select the correct device/partition."));
			//	m_btnMgr.setText(m_mainLblMessage, _t("main5", L"Welcome to WiiFlow. I have not found any plugins. Select partition to select your partition type."));
				m_btnMgr.show(m_mainBtnSelPart);
				m_btnMgr.show(m_mainLblMessage);
				break;
		}
	}
}

void CMenu::LoadView(void)
{
	m_load_view = false;
	_hideMain(true);
	CoverFlow.clear();
	if(!m_vid.showingWaitMessage())
		_showWaitMessage();
	if(m_clearCats)// clear categories unless a source menu btn has selected one
	{
		m_cat.remove("GENERAL", "selected_categories");
		m_cat.remove("GENERAL", "required_categories");
	}
	m_clearCats = true;
	m_favorites = false;
	if(m_cfg.getBool("GENERAL", "save_favorites_mode", false))
		m_favorites = m_cfg.getBool(_domainFromView(), "favorites", false);
	if(m_sourceflow)
	{
		m_gameList.clear();
		string cacheDir(fmt("%s/sourceflow.db", m_listCacheDir.c_str()));
		bool updateCache = m_cfg.getBool("SOURCEFLOW", "update_cache");
		u8 maxBtns = m_cfg.getInt("GENERAL", "max_source_buttons", 71);
		m_gameList.createSFList(maxBtns, m_source, show_homebrew, show_channel, show_plugin, show_gamecube, m_sourceDir, cacheDir, updateCache);
		m_cfg.remove("SOURCEFLOW", "update_cache");
	}
	else
		_loadList();

	if(m_source_autoboot == true)
	{	/* search for the requested file */
		bool game_found = false;
		for(vector<dir_discHdr>::iterator element = m_gameList.begin(); element != m_gameList.end(); ++element)
		{
			switch(m_autoboot_hdr.type)
			{
				case TYPE_CHANNEL:
				case TYPE_WII_GAME:
				case TYPE_GC_GAME:
					if(strcmp(m_autoboot_hdr.id, element->id) == 0)
						game_found = true;
					break;
				case TYPE_HOMEBREW:
				case TYPE_PLUGIN:
					if(wcsncmp(m_autoboot_hdr.title, element->title, 63) == 0)
						game_found = true;
					break;
				default:
					break;
			}
			if(game_found == true)
			{
				memcpy(&m_autoboot_hdr, &(*(element)), sizeof(dir_discHdr));
				break;
			}
		}
		if(game_found == true)
		{
			gprintf("Game found, autobooting...\n");
			_launch(&m_autoboot_hdr);
		}
		/* fail */
		m_source_autoboot = false;
	}
	_showMain();
	_initCF();
	_loadCFLayout(m_cfg.getInt(_domainFromView(), "last_cf_mode", 1));
	CoverFlow.applySettings();

	if(m_sourceflow)
		return;
	const char *mode = (m_current_view == COVERFLOW_CHANNEL && !m_cfg.getBool(CHANNEL_DOMAIN, "emu_nand", false)) 
		? CHANNEL_DOMAIN : DeviceName[currentPartition];

	m_showtimer = 120;
	m_btnMgr.setText(m_mainLblNotice, sfmt("%s (%u) [%s]", _domainFromView(), m_gameList.size(), upperCase(mode).c_str()));
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
	show_homebrew = (!m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false) && (parental_homebrew || !m_locked));
	show_channel = !m_cfg.getBool(CHANNEL_DOMAIN, "disable", false);
	show_plugin = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
	show_gamecube = m_show_gc;
	m_allow_random = m_cfg.getBool("GENERAL", "allow_b_on_questionmark", true);
	m_multisource = m_cfg.getBool("GENERAL", "multisource", false);
	m_use_source = m_cfg.getBool("GENERAL", "use_source", true);
	bool bheld = false;
	bool bUsed = false;
	m_emuSaveNand = false;
	m_reload = false;
	u32 disc_check = 0;

	SetupInput(true);
	GameTDB m_gametdb; 
 	m_gametdb.OpenFile(fmt("%s/wiitdb.xml", m_settingsDir.c_str()));
	m_GameTDBLoaded = false;
 	if(m_gametdb.IsLoaded())
	{
		m_GameTDBLoaded = true;
		m_gametdb.CloseFile();
	}
	m_last_view = max(0, min(m_cfg.getInt("GENERAL", "last_view", 6), 6));
	if(m_last_view == 6 || m_last_view == 0)
	{
		m_last_view = 0;
		_clearSources();
		m_cfg.setBool(WII_DOMAIN, "source", true);
	}
	m_current_view = m_last_view;
	m_catStartPage = m_cfg.getInt("GENERAL", "cat_startpage", 1);
	if(m_current_view != COVERFLOW_MAX)
	{
		m_cfg.remove("GENERAL", "last_view");
		m_cfg.remove("GENERAL", "cat_startpage");
	}
	else
		m_combined_view = true;
	m_cfg.save();
	if(m_cfg.getBool("GENERAL", "update_cache", false))
		UpdateCache();
	LoadView();

	gprintf("start\n");
	while(!m_exit)
	{
		/* IMPORTANT check if a disc is inserted */
		WDVD_GetCoverStatus(&disc_check);
		/* Main Loop */
		_mainLoopCommon(true);
		//this will make the source menu/flow display. what happens when a sourceflow cover is selected is taken care of later.
		if(bheld && !BTN_B_HELD)//if button b was held and now released
		{
			bheld = false;
			if(bUsed)//if b button used for something don't show souce menu or sourceflow
				bUsed = false;
			else
			{
				if(m_sourceflow)//if exiting sourceflow via b button
				{
					m_sourceflow = false;
					LoadView();
					continue;
				}
				if(m_use_source)//if source_menu enabled via b button
				{
					_hideMain();
					if(m_cfg.getBool("SOURCEFLOW", "enabled", false))//if sourceflow show it
					{
						m_sourceflow = true;
						LoadView();
					}
					else //show source menu
					{
						if(!_Source()) //if different source selected
							LoadView();
						else
						{
							if(BTN_B_HELD)
								bUsed = true;
							_showMain();
						}
					}
					continue;
				}
			}
		}
		if(BTN_HOME_PRESSED)
		{
			_hideMain();
			if(m_sourceflow)
			{
				_CfgSrc();
				_showMain();
				if(BTN_B_HELD)
					bUsed = true;
				if(m_load_view)
					LoadView();
				else
					_showMain();
			}
			else
			{
				if(_Home()) //exit wiiflow
					break;
				_showMain();
				if(BTN_B_HELD)
					bUsed = true;
			}
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
			else if(m_btnMgr.selected(m_mainBtnChannel) || m_btnMgr.selected(m_mainBtnWii) || m_btnMgr.selected(m_mainBtnGamecube) || m_btnMgr.selected(m_mainBtnHomebrew) || m_btnMgr.selected(m_mainBtnPlugin))
			{
				if(m_current_view == COVERFLOW_WII) 
					m_current_view = show_gamecube ? COVERFLOW_GAMECUBE : (show_channel ? COVERFLOW_CHANNEL : (show_plugin ? COVERFLOW_PLUGIN : (show_homebrew ? COVERFLOW_HOMEBREW : COVERFLOW_WII)));
				else if(m_current_view == COVERFLOW_GAMECUBE)
					m_current_view = show_channel ? COVERFLOW_CHANNEL : (show_plugin ? COVERFLOW_PLUGIN : (show_homebrew ? COVERFLOW_HOMEBREW : COVERFLOW_WII));
				else if(m_current_view == COVERFLOW_CHANNEL)
					m_current_view = show_plugin ? COVERFLOW_PLUGIN : (show_homebrew ? COVERFLOW_HOMEBREW : COVERFLOW_WII);
				else if(m_current_view == COVERFLOW_PLUGIN)
					m_current_view = show_homebrew ? COVERFLOW_HOMEBREW : COVERFLOW_WII;
				else if(m_current_view == COVERFLOW_HOMEBREW || m_current_view == COVERFLOW_MAX)
					m_current_view = COVERFLOW_WII;
				_clearSources();
				m_cfg.setBool(_domainFromView(), "source", true);
				m_catStartPage = 1;
				m_combined_view = false;
				LoadView();
			}
			else if(m_btnMgr.selected(m_mainBtnInstall))
			{
				if(!m_locked)
				{
					_hideMain();
					_wbfsOp(CMenu::WO_ADD_GAME);
					_showMain();
					if(BTN_B_HELD)
						bUsed = true;
				}
			}
			else if(m_btnMgr.selected(m_mainBtnSelPart))
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
				if(BTN_B_HELD)
					bUsed = true;
				//update show_homebrew because parental lock might have changed
				show_homebrew = (!m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false) && (parental_homebrew || !m_locked));
				if(m_load_view)
					LoadView();
				else
					_showMain();
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
				memcpy(&hdr.id, "dvddvd", 6);//only the id is used for a disc and dvddvd is changed in _launchGame.
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
				if(m_sourceflow)
				{
					_sourceFlow();// set the source selected
					LoadView();
					continue;
				}
				else
				{
					_game(BTN_B_HELD);
					if(m_exit)
						break;
					if(BTN_B_HELD)
						bUsed = true;
					CoverFlow.cancel();
					_showMain();
				}
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
			//Events to show source menu or sourceflow if B on mode icon
			else if(m_btnMgr.selected(m_mainBtnChannel) || m_btnMgr.selected(m_mainBtnWii) || m_btnMgr.selected(m_mainBtnGamecube)|| m_btnMgr.selected(m_mainBtnPlugin) || m_btnMgr.selected(m_mainBtnHomebrew))
			{
				if(!m_use_source)//only use if B to source menu not enabled
				{
					_hideMain();
					if(m_cfg.getBool("SOURCEFLOW", "enabled", false))//if sourceflow show it
					{
						m_sourceflow = true;
						LoadView();
					}
					else //show source menu
					{
						if(!_Source()) //if different source selected load it
							LoadView();
						else
						{
							if(BTN_B_HELD)
								bUsed = true;
							_showMain();
						}
					}
					continue;
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
					sorting = SORT_ALPHA;
				}
				wchar_t c[2] = {0, 0};
				m_btnMgr.selected(m_mainBtnPrev) ? CoverFlow.prevLetter(c) : CoverFlow.nextLetter(c);
				m_showtimer = 120;
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
			else if(m_btnMgr.selected(m_mainBtnInfo) && m_allow_random && !CoverFlow.empty())
			{
				/* WiiFlow should boot a random game */
				_hideMain();
				srand(time(NULL));
				u16 place = (rand() + rand() + rand()) % CoverFlow.size();
				gprintf("Lets boot the random game number %u\n", place);
				const dir_discHdr *gameHdr = CoverFlow.getSpecificHdr(place);
				if(gameHdr != NULL)
					_launch(gameHdr);
				/* Shouldnt happen */
				_showMain();
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
				const char *domain = _domainFromView();
				s8 direction = BTN_1_PRESSED ? 1 : -1;
				int cfVersion = 1+loopNum((m_cfg.getInt(domain, "last_cf_mode", 1)-1) + direction, m_numCFVersions);
				_loadCFLayout(cfVersion);
				CoverFlow.applySettings();
				m_cfg.setInt(domain, "last_cf_mode", cfVersion);
			}
			else if(BTN_MINUS_PRESSED)
				CoverFlow.pageUp();
			else if(BTN_PLUS_PRESSED)
				CoverFlow.pageDown();
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
					sorting = SORT_ALPHA;
				}
				wchar_t c[2] = {0, 0};
				BTN_UP_PRESSED ? CoverFlow.prevLetter(c) : CoverFlow.nextLetter(c);

				curLetter.clear();
				curLetter = wstringEx(c);

				m_showtimer = 120;
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
				MusicPlayer.Previous();
			}
			else if(BTN_RIGHT_PRESSED)
			{
				bUsed = true;
				MusicPlayer.Next();
			}
			else if(BTN_PLUS_PRESSED && !m_locked  && !m_sourceflow)
			{
				bUsed = true;
				u32 sort = 0;
				sort = loopNum((m_cfg.getInt(domain, "sort", 0)) + 1, SORT_MAX);
				if((m_current_view == COVERFLOW_HOMEBREW || m_current_view == COVERFLOW_PLUGIN) && sort > SORT_LASTPLAYED)
					sort = SORT_ALPHA;
				m_cfg.setInt(domain, "sort", sort);
				_initCF();
				wstringEx curSort ;
				if(sort == SORT_ALPHA)
					curSort = m_loc.getWString(m_curLanguage, "alphabetically", L"Alphabetically");
				else if(sort == SORT_PLAYCOUNT)
					curSort = m_loc.getWString(m_curLanguage, "byplaycount", L"By Play Count");
				else if(sort == SORT_LASTPLAYED)
					curSort = m_loc.getWString(m_curLanguage, "bylastplayed", L"By Last Played");
				else if(sort == SORT_GAMEID)
					curSort = m_loc.getWString(m_curLanguage, "bygameid", L"By Game I.D.");
				else if(sort == SORT_WIFIPLAYERS)
					curSort = m_loc.getWString(m_curLanguage, "bywifiplayers", L"By Wifi Players");
				else if(sort == SORT_PLAYERS)
					curSort = m_loc.getWString(m_curLanguage, "byplayers", L"By Players");
				m_showtimer = 120;
				m_btnMgr.setText(m_mainLblNotice, curSort);
				m_btnMgr.show(m_mainLblNotice);
			}
			else if(BTN_MINUS_PRESSED && !m_locked  && !m_sourceflow)
			{
				bUsed = true;
				const char *partition = NULL;
				_showWaitMessage();
				_hideMain();
				_setPartition(1);
				if(m_current_view == COVERFLOW_CHANNEL && (!m_cfg.getBool(CHANNEL_DOMAIN, "emu_nand", false) || neek2o()))
					partition = "NAND";
				else
					partition = DeviceName[currentPartition];
				//gprintf("Next item: %s\n", partition);
				_loadList();
				_showMain();
				_initCF();
				/* refresh AFTER reloading */
				m_showtimer = 120;
				m_btnMgr.setText(m_mainLblNotice, sfmt("%s (%u) [%s]", _domainFromView(), m_gameList.size(), upperCase(partition).c_str()));
				m_btnMgr.show(m_mainLblNotice);
			}
		}

		if(m_showtimer > 0)
		{
			if(--m_showtimer == 0)
			{
				m_btnMgr.hide(m_mainLblLetter);
				m_btnMgr.hide(m_mainLblNotice);
			}
		}
		//zones, showing and hiding buttons
		if(!m_gameList.empty() && m_show_zone_prev && !m_sourceflow)
			m_btnMgr.show(m_mainBtnPrev);
		else
			m_btnMgr.hide(m_mainBtnPrev);
		if(!m_gameList.empty() && m_show_zone_next && !m_sourceflow)
			m_btnMgr.show(m_mainBtnNext);
		else
			m_btnMgr.hide(m_mainBtnNext);
		if(!m_gameList.empty() && m_show_zone_main && !m_sourceflow)
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
		if((!m_cfg.getBool("GENERAL", "hideviews", false) && (m_gameList.empty() || m_show_zone_main2)) && !m_sourceflow)
		{
			switch(m_current_view)
			{
				case COVERFLOW_GAMECUBE:
					if(show_channel)
						m_btnMgr.show(m_mainBtnChannel);
					else if(show_plugin)
						m_btnMgr.show(m_mainBtnPlugin);
					else if(show_homebrew)
						m_btnMgr.show(m_mainBtnHomebrew);
					else 
						m_btnMgr.show(m_mainBtnWii);
					break;
				case COVERFLOW_CHANNEL:
					if(show_plugin)
						m_btnMgr.show(m_mainBtnPlugin);
					else if(show_homebrew)
						m_btnMgr.show(m_mainBtnHomebrew);
					else
						m_btnMgr.show(m_mainBtnWii);
					break;
				case COVERFLOW_PLUGIN:
					if(show_homebrew)
						m_btnMgr.show(m_mainBtnHomebrew);
					else
						m_btnMgr.show(m_mainBtnWii);
					break;
				case COVERFLOW_HOMEBREW:
				case COVERFLOW_MAX:
					m_btnMgr.show(m_mainBtnWii);
					break;
				default:
					if(show_gamecube)
						m_btnMgr.show(m_mainBtnGamecube);
					else if(show_channel)
						m_btnMgr.show(m_mainBtnChannel);
					else if(show_plugin)
						m_btnMgr.show(m_mainBtnPlugin);
					else if(show_homebrew)
						m_btnMgr.show(m_mainBtnHomebrew);
					else
						m_btnMgr.show(m_mainBtnWii);
			}
			m_btnMgr.show(m_mainLblUser[2]);
			m_btnMgr.show(m_mainLblUser[3]);
		}
		else
		{
			m_btnMgr.hide(m_mainBtnHomebrew);
			m_btnMgr.hide(m_mainBtnChannel);
			m_btnMgr.hide(m_mainBtnWii);
			m_btnMgr.hide(m_mainBtnGamecube);
			m_btnMgr.hide(m_mainBtnPlugin);
			m_btnMgr.hide(m_mainLblUser[2]);
			m_btnMgr.hide(m_mainLblUser[3]);
		}
		if(((disc_check & 0x2) && (m_gameList.empty() || m_show_zone_main3))  && !m_sourceflow)
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
		const char *ReturnPath = NULL;
		if(!m_cfg.getBool(CHANNEL_DOMAIN, "neek_return_default", false))
		{
			string emuPath;
			m_current_view = COVERFLOW_CHANNEL; /* So we get the path */
			_FindEmuPart(emuPath, false);
			ReturnPath = NandHandle.Get_NandPath();
		}
		Sys_SetNeekPath(ReturnPath);
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
	TexData texGamecube;
	TexData texGamecubes;
	TexData texPlugin;
	TexData texPlugins;
	TexData texDVD;
	TexData texDVDs;
	TexData texWii;
	TexData texWiis;
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

	TexHandle.fromImageFile(texQuit, fmt("%s/btnquit.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texQuitS, fmt("%s/btnquits.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texInfo, fmt("%s/btninfo.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texInfoS, fmt("%s/btninfos.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texConfig, fmt("%s/btnconfig.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texConfigS, fmt("%s/btnconfigs.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texDVD, fmt("%s/btndvd.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texDVDs, fmt("%s/btndvds.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texWii, fmt("%s/btnusb.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texWiis, fmt("%s/btnusbs.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texGamecube, fmt("%s/btndml.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texGamecubes, fmt("%s/btndmls.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texPlugin, fmt("%s/btnemu.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texPlugins, fmt("%s/btnemus.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texChannel, fmt("%s/btnchannel.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texChannels, fmt("%s/btnchannels.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texHomebrew, fmt("%s/btnhomebrew.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texHomebrews, fmt("%s/btnhomebrews.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texPrev, fmt("%s/btnprev.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texPrevS, fmt("%s/btnprevs.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texNext, fmt("%s/btnnext.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texNextS, fmt("%s/btnnexts.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texFavOn, fmt("%s/favoriteson.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texFavOnS, fmt("%s/favoritesons.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texFavOff, fmt("%s/favoritesoff.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texFavOffS, fmt("%s/favoritesoffs.png", m_imgsDir.c_str()));

	_addUserLabels(m_mainLblUser, ARRAY_SIZE(m_mainLblUser), "MAIN");

	m_mainBtnInfo = _addPicButton("MAIN/INFO_BTN", texInfo, texInfoS, 20, 400, 48, 48);
	m_mainBtnConfig = _addPicButton("MAIN/CONFIG_BTN", texConfig, texConfigS, 70, 400, 48, 48);
	m_mainBtnQuit = _addPicButton("MAIN/QUIT_BTN", texQuit, texQuitS, 570, 400, 48, 48);
	m_mainBtnChannel = _addPicButton("MAIN/CHANNEL_BTN", texChannel, texChannels, 520, 400, 48, 48);
	m_mainBtnHomebrew = _addPicButton("MAIN/HOMEBREW_BTN", texHomebrew, texHomebrews, 520, 400, 48, 48);
	m_mainBtnWii = _addPicButton("MAIN/USB_BTN", texWii, texWiis, 520, 400, 48, 48);
	m_mainBtnGamecube = _addPicButton("MAIN/DML_BTN", texGamecube, texGamecubes, 520, 400, 48, 48);
	m_mainBtnPlugin = _addPicButton("MAIN/EMU_BTN", texPlugin, texPlugins, 520, 400, 48, 48);
	m_mainBtnDVD = _addPicButton("MAIN/DVD_BTN", texDVD, texDVDs, 470, 400, 48, 48);
	m_mainBtnNext = _addPicButton("MAIN/NEXT_BTN", texNext, texNextS, 540, 146, 80, 80);
	m_mainBtnPrev = _addPicButton("MAIN/PREV_BTN", texPrev, texPrevS, 20, 146, 80, 80);
	m_mainBtnInstall = _addButton("MAIN/BIG_SETTINGS_BTN", theme.titleFont, L"", 72, 180, 496, 48, theme.titleFontColor);
	m_mainBtnSelPart = _addButton("MAIN/BIG_SETTINGS_BTN2", theme.titleFont, L"", 72, 290, 496, 48, theme.titleFontColor);
	m_mainLblMessage = _addLabel("MAIN/MESSAGE", theme.lblFont, L"", 40, 40, 560, 140, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_mainBtnFavoritesOn = _addPicButton("MAIN/FAVORITES_ON", texFavOn, texFavOnS, 288, 400, 64, 64);
	m_mainBtnFavoritesOff = _addPicButton("MAIN/FAVORITES_OFF", texFavOff, texFavOffS, 288, 400, 64, 64);
	m_mainLblLetter = _addLabel("MAIN/LETTER", theme.titleFont, L"", 540, 40, 80, 80, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, emptyTex);
	m_mainLblNotice = _addLabel("MAIN/NOTICE", theme.titleFont, L"", 340, 40, 280, 80, theme.titleFontColor, FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE, emptyTex);
	m_mainLblCurMusic = _addLabel("MAIN/MUSIC", theme.btnFont, L"", 0, 20, 640, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
#ifdef SHOWMEM
	m_mem1FreeSize = _addLabel("MEM1", theme.btnFont, L"", 0, 300, 480, 56, theme.btnFontColor, FTGX_JUSTIFY_LEFT, emptyTex);
	m_mem2FreeSize = _addLabel("MEM2", theme.btnFont, L"", 0, 356, 480, 56, theme.btnFontColor, FTGX_JUSTIFY_LEFT, emptyTex);
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
	_setHideAnim(m_mainBtnWii, "MAIN/USB_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnGamecube, "MAIN/DML_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnPlugin, "MAIN/EMU_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnDVD, "MAIN/DVD_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOn, "MAIN/FAVORITES_ON", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOff, "MAIN/FAVORITES_OFF", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnInstall, "MAIN/BIG_SETTINGS_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_mainBtnSelPart, "MAIN/BIG_SETTINGS_BTN2", 0, 0, -2.f, 0.f);
	_setHideAnim(m_mainLblMessage, "MAIN/MESSAGE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblLetter, "MAIN/LETTER", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblNotice, "MAIN/NOTICE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblCurMusic, "MAIN/MUSIC", 0, -100, 0.f, 0.f);
#ifdef SHOWMEM
	_setHideAnim(m_mem1FreeSize, "MEM1", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mem2FreeSize, "MEM2", 0, 0, 0.f, 0.f);
#endif
	_hideMain(true);
	_textMain();
}

void CMenu::_textMain(void)
{
	m_btnMgr.setText(m_mainBtnInstall, _t("main1", L"Install Game"));
	m_btnMgr.setText(m_mainBtnSelPart, _t("main3", L"Select Partition"));
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
	if(m_current_view == COVERFLOW_CHANNEL && neek2o())
		return;
	int FS_Type = 0;
	if(direction != 0)
	{
		bool switch_to_real = true;
		if(m_current_view == COVERFLOW_CHANNEL && !NANDemuView)
		{
			NANDemuView = true;
			m_cfg.setBool(CHANNEL_DOMAIN, "emu_nand", true);
			switch_to_real = false;
		}
		bool NeedFAT = m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_GAMECUBE;

		u8 limiter = 0;
		do
		{
			currentPartition = loopNum(currentPartition + direction, 10);
			FS_Type = DeviceHandle.GetFSType(currentPartition);
			if(m_current_view == COVERFLOW_CHANNEL && switch_to_real && FS_Type == -1)
				break;
			limiter++;
		}
		while(limiter < 12 && (!DeviceHandle.IsInserted(currentPartition) ||
			(m_current_view != COVERFLOW_WII && FS_Type == PART_FS_WBFS) ||
			(NeedFAT && FS_Type != PART_FS_FAT)));

		if(m_current_view == COVERFLOW_CHANNEL && FS_Type == -1)
		{
			NANDemuView = false;
			m_cfg.setBool(CHANNEL_DOMAIN, "emu_nand", false);
		}
	}
	if(m_emuSaveNand)
		m_cfg.setInt(WII_DOMAIN, "savepartition", currentPartition);
	else
	{
		if(direction == 0 || (direction != 0 && (m_current_view != COVERFLOW_CHANNEL || 
							(FS_Type != -1 && DeviceHandle.IsInserted(currentPartition)))))
			m_cfg.setInt(_domainFromView(), "partition", currentPartition);
		vector<bool> plugin_list = m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
		if(enabledPluginsCount == 1)
		{
			u8 i = 0;
			for(i = 0; i < plugin_list.size(); ++i)
			{
				if(plugin_list[i] == true)
					break;
			}
			strncpy(m_plugin.PluginMagicWord, fmt("%08x", m_plugin.getPluginMagic(i)), 8);
			m_cfg.setInt("PLUGINS_PARTITION", m_plugin.PluginMagicWord, currentPartition);
		}
	}
}
