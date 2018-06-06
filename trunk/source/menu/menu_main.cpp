
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>

#include "menu.hpp"
#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
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
	m_btnMgr.hide(m_mainBtnHome, instant);
	m_btnMgr.hide(m_mainBtnHomebrew, instant);
	m_btnMgr.hide(m_mainBtnChannel, instant);
	m_btnMgr.hide(m_mainBtnWii, instant);
	m_btnMgr.hide(m_mainBtnGamecube, instant);
	m_btnMgr.hide(m_mainBtnPlugin, instant);
	m_btnMgr.hide(m_mainBtnDVD, instant);
	m_btnMgr.hide(m_mainLblMessage, instant);
	m_btnMgr.hide(m_mainBtnFavoritesOn, instant);
	m_btnMgr.hide(m_mainBtnFavoritesOff, instant);
	m_btnMgr.hide(m_mainLblLetter, instant);
	m_btnMgr.hide(m_mainLblNotice, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_mainLblUser); ++i)
		if(m_mainLblUser[i] != -1)
			m_btnMgr.hide(m_mainLblUser[i], instant);
}

void CMenu::_showMain()
{
	_setBg(m_mainBg, m_mainBgLQ);
	if(m_refreshGameList)
		_showCF(m_refreshGameList);
}

void CMenu::_showCF(bool refreshList)
{
	m_refreshGameList = false;
	_hideMain(true);

	if(refreshList)
	{
		if(!m_vid.showingWaitMessage())
			_showWaitMessage();
		
		/* create gameList */
		_loadList();

		/* autoboot stuff */
		if(m_source_autoboot == true)
		{	/* search for the requested file */
			bool game_found = false;
			for(vector<dir_discHdr>::iterator element = m_gameList.begin(); element != m_gameList.end(); ++element)
			{
				switch(m_autoboot_hdr.type)
				{
					case TYPE_CHANNEL:
					//case TYPE_EMUCHANNEL:
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
		
		_hideWaitMessage();
	}

	wstringEx Msg;
	wstringEx Pth;
	if(m_gameList.empty())
	{
		if(m_source_cnt > 1)
		{
			Msg = _t("main8", L"game list empty!");
			Pth = "";
		}
		else
		{
			switch(m_current_view)
			{
				case COVERFLOW_WII:
					Msg = _t("main2", L"No games found in ");
					Pth = wstringEx(fmt(wii_games_dir, DeviceName[currentPartition]));
					break;
				case COVERFLOW_GAMECUBE:
					Msg = _t("main2", L"No games found in ");
					Pth = wstringEx(fmt(gc_games_dir, DeviceName[currentPartition]));
					break;
				case COVERFLOW_CHANNEL:
					Msg = _t("main3", L"No titles found in ");
					Pth = wstringEx(fmt("%s:/%s/%s", DeviceName[currentPartition],  emu_nands_dir, m_cfg.getString(CHANNEL_DOMAIN, "current_emunand").c_str()));
					break;
				case COVERFLOW_HOMEBREW:
					Msg = _t("main4", L"No apps found in ");
					Pth = wstringEx(fmt(HOMEBREW_DIR, DeviceName[currentPartition]));
					break;
				case COVERFLOW_PLUGIN:
					m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
					if(enabledPluginsCount == 0)
						Msg = _t("main6", L"No plugins selected.");
					else
						Msg = _t("main5", L"No roms/items found.");
					Pth = "";
					break;
			}
		}
		Msg.append(Pth);
		m_btnMgr.setText(m_mainLblMessage, Msg);
		m_btnMgr.show(m_mainLblMessage);
		return;
	}
	
	/* setup for filter list and coverflow stuff */
	if(refreshList && m_clearCats)// clear categories unless a source menu btn has selected one
	{
		// do not clear hidden categories to keep games hidden
		m_cat.remove("GENERAL", "selected_categories");
		m_cat.remove("GENERAL", "required_categories");
	}
	m_clearCats = true;
	
	m_favorites = false;
	if(m_cfg.getBool("GENERAL", "save_favorites_mode", false))
		m_favorites = m_cfg.getBool(_domainFromView(), "favorites", false);
	
	cf_domain = "_COVERFLOW";
	if(!m_sourceflow && m_current_view == COVERFLOW_HOMEBREW && m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", true))
		cf_domain = "_SMALLFLOW";
	if(m_sourceflow && m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox", true))
		cf_domain = "_SMALLFLOW";
	if(m_current_view == COVERFLOW_PLUGIN && !m_sourceflow)
	{
		m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
		if(enabledPluginsCount > 0)
		{
			int sdc = 0;
			int shc = 0;
			for(u8 i = 0; m_plugin.PluginExist(i); ++i)
			{
				if(m_plugin.GetEnableStatus(m_cfg, m_plugin.getPluginMagic(i)))
				{
					if(_sideCover(m_plugin.PluginMagicWord))
						sdc++;
					else if(_shortCover(m_plugin.PluginMagicWord))
						shc++;
				}
			}
			if(sdc == enabledPluginsCount)
				cf_domain = "_SIDEFLOW";
			else if(shc == enabledPluginsCount)
				cf_domain = "_SHORTFLOW";
		}
	}

	m_numCFVersions = min(max(1, m_coverflow.getInt(cf_domain, "number_of_modes", 1)), 15);
	
	/* filter list and start coverflow coverloader */	
	_loadCFLayout(min(max(1, _getCFVersion()), (int)m_numCFVersions));
	_initCF();
	CoverFlow.applySettings();

	/* display game count unless sourceflow or homebrew */
	if(m_sourceflow || m_current_view == COVERFLOW_HOMEBREW)
		return;

	m_showtimer = 240;
	m_btnMgr.setText(m_mainLblNotice, wfmt(_fmt("main7", L"Total Games: %u"), CoverFlow.size()));
	m_btnMgr.show(m_mainLblNotice);
}

int CMenu::main(void)
{
	wstringEx curLetter;
	const char *prevTheme = m_cfg.getString("GENERAL", "theme", "default").c_str();
	bool show_channel = !m_cfg.getBool(CHANNEL_DOMAIN, "disable", false);
	bool show_plugin = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
	bool show_gamecube = !m_cfg.getBool(GC_DOMAIN, "disable", false);
	m_multisource = m_cfg.getBool("GENERAL", "multisource", false);
	bool m_source_on_start = m_cfg.getBool("GENERAL", "source_on_start", false);
	bool bheld = false;
	bool bUsed = false;
	m_emuSaveNand = false;
	m_reload = false;
	u32 disc_check = 0;

	m_current_view = m_cfg.getUInt("GENERAL", "sources", COVERFLOW_WII);
	m_source_cnt = 0;
	for(u8 i = 1; i < 16; i <<= 1)//not including coverflow_homebrew
		if(m_current_view & i)
			m_source_cnt++;
			
	if(m_source_cnt == 0 || m_current_view == COVERFLOW_HOMEBREW)
	{
		m_current_view = COVERFLOW_WII;
		m_cfg.setUInt("GENERAL", "sources", m_current_view);
		m_source_cnt++;
	}
	
	m_catStartPage = m_cfg.getInt("GENERAL", "cat_startpage", 1);
	//if(m_source_cnt == 1)
	//	m_cfg.remove("GENERAL", "cat_startpage");
	
	if(m_cfg.getBool("GENERAL", "update_cache", false))
	{
		m_cfg.setBool("GENERAL", "update_cache", false);
		fsop_deleteFolder(m_listCacheDir.c_str());
		fsop_MakeFolder(m_listCacheDir.c_str());
	}
	m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480),
						m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));

	m_refreshGameList = true;
	_showMain();
	if(show_mem)
	{
		m_btnMgr.show(m_mem1FreeSize);
		m_btnMgr.show(m_mem2FreeSize);
	}
	SetupInput(true);

	gprintf("start wiiflow main screen\n");
	while(!m_exit)
	{
		/* IMPORTANT check if a disc is inserted */
		WDVD_GetCoverStatus(&disc_check);
		/* Main Loop */
		_mainLoopCommon(true);
		//this will make the source menu/flow display. what happens when a sourceflow cover is selected is taken care of later.
		if((bheld && !BTN_B_HELD) || m_source_on_start)//if button b was held and now released
		{
			m_source_on_start = false;
			bheld = false;
			if(bUsed)//if b button used for something don't show souce menu or sourceflow
				bUsed = false;
			else
			{
				if(m_sourceflow)//if exiting sourceflow via b button
				{
					m_sourceflow = false;
					_showCF(true);
					continue;
				}
				if(m_current_view == COVERFLOW_HOMEBREW)
				{
					m_current_view = m_prev_view;
					m_cfg.setUInt("GENERAL", "sources", m_current_view);
					_showCF(true);
					continue;
				}
				if(m_use_source)//if source_menu enabled
				{
					_hideMain();
					if(m_cfg.getBool(SOURCEFLOW_DOMAIN, "enabled", false))//if sourceflow show it
					{
						m_sourceflow = true;
						_showCF(true);
					}
					else //show source menu
					{
						
						m_refreshGameList = _Source();
						if(BTN_B_HELD)
							bUsed = true;
						_showMain();
					}
					continue;
				}
			}
		}
		if(BTN_HOME_PRESSED)
		{
			_hideMain();
			/* sourceflow config menu */
			if(m_sourceflow)
			{
				_CfgSrc();
				if(BTN_B_HELD)
				{
					bheld = true;
					bUsed = true;
				}
				if(!m_cfg.getBool(SOURCEFLOW_DOMAIN, "enabled"))
				{
					m_sourceflow = false;
					m_refreshGameList = true;
					//_showMain();
					//continue;
				}
				_showMain();
			}
			/* homebrew flow config menu */
			else if(m_current_view == COVERFLOW_HOMEBREW)
			{
				_CfgHB();
				if(BTN_B_HELD)
				{
					bheld = true;
					bUsed = true;
				}
				_showMain();
			}
			/* Home menu */
			else
			{
				if(_Home())
					break;// exit wiiflow
				if(BTN_B_HELD)
					bUsed = true;
				_showMain();
			}
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_mainBtnPrev))
				CoverFlow.pageUp();
		 	else if(m_btnMgr.selected(m_mainBtnNext))
				CoverFlow.pageDown();
			else if(m_btnMgr.selected(m_mainBtnHome))
			{
				/* home menu via main menu button */
				_hideMain();
				if(_Home())
					break;// exit wiiflow
				if(BTN_B_HELD)
					bUsed = true;
				_showMain();
			}
			else if(m_btnMgr.selected(m_mainBtnChannel) || m_btnMgr.selected(m_mainBtnWii) || m_btnMgr.selected(m_mainBtnGamecube) || m_btnMgr.selected(m_mainBtnPlugin))
			{
				/* change source via view button on main menu */
				if(m_current_view == COVERFLOW_WII) 
					m_current_view = show_gamecube ? COVERFLOW_GAMECUBE : (show_channel ? COVERFLOW_CHANNEL : (show_plugin ? COVERFLOW_PLUGIN : COVERFLOW_WII));
				else if(m_current_view == COVERFLOW_GAMECUBE)
					m_current_view = show_channel ? COVERFLOW_CHANNEL : (show_plugin ? COVERFLOW_PLUGIN : COVERFLOW_WII);
				else if(m_current_view == COVERFLOW_CHANNEL)
					m_current_view = show_plugin ? COVERFLOW_PLUGIN : COVERFLOW_WII;
				else if(m_current_view == COVERFLOW_PLUGIN || m_source_cnt > 1)
					m_current_view = COVERFLOW_WII;
				m_source_cnt = 1;
				m_cfg.setUInt("GENERAL", "sources", m_current_view);
				m_catStartPage = 1;
				_showCF(true);
			}
			else if(m_btnMgr.selected(m_mainBtnConfig))
			{
				/* main menu global settings */
				_hideMain();
				_config(1);
				if(strcmp(prevTheme, m_cfg.getString("GENERAL", "theme").c_str()) != 0)
				{
					/* new theme - exit wiiflow and reload */
					m_reload = true;
					break;
				}
				if(BTN_B_HELD)
					bUsed = true;
				_showMain();
			}
			else if(m_btnMgr.selected(m_mainBtnHomebrew))
			{
				/* launch homebrew flow */
				if(m_locked && m_cfg.getBool(HOMEBREW_DOMAIN, "parental", false))
				{
					error(_t("errgame15", L"WiiFlow locked! Unlock WiiFlow to use this feature."));
					_showMain();
				}
				else
				{
					m_prev_view = m_current_view;
					m_current_view = COVERFLOW_HOMEBREW;
					_showCF(true);
				}
				if(BTN_B_HELD)
					bUsed = true;
			}
			else if(m_btnMgr.selected(m_mainBtnDVD))
			{
				/* Boot DVD in drive */
				_hideMain(true);
				/* Create Fake Header */
				dir_discHdr hdr;
				memset(&hdr, 0, sizeof(dir_discHdr));
				memcpy(&hdr.id, "dvddvd", 6);//this must be set for neek2o
				/* Boot the Disc */
				_launchGame(&hdr, true, BTN_B_HELD);
				if(BTN_B_HELD)
					bUsed = true;
				_showCF(false);
			}
			else if(m_btnMgr.selected(m_mainBtnFavoritesOn) || m_btnMgr.selected(m_mainBtnFavoritesOff))
			{
				/* switch favorite games only on/off */
				m_favorites = !m_favorites;
				m_cfg.setBool(_domainFromView(), "favorites", m_favorites);
				_initCF();
			}
			else if(!CoverFlow.empty() && CoverFlow.select())
			{
				/* select game cover or sourceflow cover */
				_hideMain();
				if(m_sourceflow)
				{
					_sourceFlow();// set the source selected
					_showCF(true);
					continue;
				}
				else
				{
					_game(BTN_B_HELD || m_current_view == COVERFLOW_HOMEBREW);
					if(m_exit)
						break;
					if(BTN_B_HELD)
						bUsed = true;
					if(m_refreshGameList)
					{
						/* if changes were made to favorites, parental lock, or categories */
						_initCF();
						m_refreshGameList = false;
					}
					else
						CoverFlow.cancel();
				}
			}
		}
		else if(BTN_B_PRESSED)
		{
			/* Show Categories */
			if(m_btnMgr.selected(m_mainBtnFavoritesOn) || m_btnMgr.selected(m_mainBtnFavoritesOff))
			{
				_hideMain();
				_CategorySettings();
				if(BTN_B_HELD)
					bUsed = true;
				_setBg(m_mainBg, m_mainBgLQ);
				if(m_refreshGameList)
				{
					m_refreshGameList = false;
					_initCF();
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
				s8 direction = BTN_1_PRESSED ? 1 : -1;
				int cfVersion = 1 + loopNum((_getCFVersion() - 1) + direction, m_numCFVersions);
				_setCFVersion(cfVersion);
				_loadCFLayout(cfVersion);
				CoverFlow.applySettings();
			}
			else if(BTN_MINUS_PRESSED)
				CoverFlow.pageUp();
			else if(BTN_PLUS_PRESSED)
				CoverFlow.pageDown();
		}
		else // Button B Held
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
			/* change sorting with btn B and + */
			else if(BTN_PLUS_PRESSED && !m_locked  && !m_sourceflow && m_current_view < 8)
			{
				bUsed = true;
				u32 sort = 0;
				sort = loopNum((m_cfg.getInt(domain, "sort", 0)) + 1, SORT_MAX);
				//if((m_current_view == COVERFLOW_HOMEBREW || m_current_view == COVERFLOW_PLUGIN) && sort > SORT_LASTPLAYED)
				//	sort = SORT_ALPHA;
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
			else if(BTN_MINUS_PRESSED && !CoverFlow.empty())
			{
				_hideMain();
				srand(time(NULL));
				u16 place = (rand() + rand() + rand()) % CoverFlow.size();
				if(m_cfg.getBool("GENERAL", "random_select", false))
				{
					CoverFlow.setSelected(place);
					_game(false);
					if(m_exit)
						break;
					if(BTN_B_HELD)
						bUsed = true;
					if(m_refreshGameList)
					{
						/* if changes were made to favorites, parental lock, or categories */
						_initCF();
						m_refreshGameList = false;
					}
					else
						CoverFlow.cancel();
				}
				else /* WiiFlow should boot a random game */
				{
					gprintf("Lets boot the random game number %u\n", place);
					const dir_discHdr *gameHdr = CoverFlow.getSpecificHdr(place);
					if(gameHdr != NULL)
						_launch(gameHdr);
					/* Shouldnt happen */
					_showCF(false);
				}
			}
		}
		/* Hide Notice or Letter if times up */	
		if(m_showtimer > 0)
		{
			if(--m_showtimer == 0)
			{
				m_btnMgr.hide(m_mainLblLetter);
				m_btnMgr.hide(m_mainLblNotice);
			}
		}
		/*zones, showing and hiding buttons */
		if(!m_gameList.empty() && m_show_zone_prev && !m_sourceflow && m_current_view != COVERFLOW_HOMEBREW)
			m_btnMgr.show(m_mainBtnPrev);
		else
			m_btnMgr.hide(m_mainBtnPrev);
			
		if(!m_gameList.empty() && m_show_zone_next && !m_sourceflow && m_current_view != COVERFLOW_HOMEBREW)
			m_btnMgr.show(m_mainBtnNext);
		else
			m_btnMgr.hide(m_mainBtnNext);
			
		if(m_show_zone_main && !m_sourceflow && m_current_view != COVERFLOW_HOMEBREW)
		{
			m_btnMgr.show(m_mainLblUser[0]);
			m_btnMgr.show(m_mainLblUser[1]);
			m_btnMgr.show(m_mainBtnHomebrew);
			m_btnMgr.show(m_mainBtnConfig);
			m_btnMgr.show(m_mainBtnHome);
			static bool change = m_favorites;
			m_btnMgr.show(m_favorites ? m_mainBtnFavoritesOn : m_mainBtnFavoritesOff, change != m_favorites);
			m_btnMgr.hide(m_favorites ? m_mainBtnFavoritesOff : m_mainBtnFavoritesOn, change != m_favorites);
			change = m_favorites;
		}
		else
		{
			m_btnMgr.hide(m_mainLblUser[0]);
			m_btnMgr.hide(m_mainLblUser[1]);
			m_btnMgr.hide(m_mainBtnConfig);
			m_btnMgr.hide(m_mainBtnHomebrew);
			m_btnMgr.hide(m_mainBtnHome);
			m_btnMgr.hide(m_mainBtnFavoritesOn);
			m_btnMgr.hide(m_mainBtnFavoritesOff);
		}
		if(!m_cfg.getBool("GENERAL", "hideviews", false) && m_show_zone_main2 && !m_sourceflow && m_current_view != COVERFLOW_HOMEBREW)
		{
			switch(m_current_view)
			{
				case COVERFLOW_WII:
					if(show_gamecube)
						m_btnMgr.show(m_mainBtnGamecube);
					else if(show_channel)
						m_btnMgr.show(m_mainBtnChannel);
					else if(show_plugin)
						m_btnMgr.show(m_mainBtnPlugin);
					else
						m_btnMgr.show(m_mainBtnWii);
					break;
				case COVERFLOW_GAMECUBE:
					if(show_channel)
						m_btnMgr.show(m_mainBtnChannel);
					else if(show_plugin)
						m_btnMgr.show(m_mainBtnPlugin);
					else 
						m_btnMgr.show(m_mainBtnWii);
					break;
				case COVERFLOW_CHANNEL:
					if(show_plugin)
						m_btnMgr.show(m_mainBtnPlugin);
					else
						m_btnMgr.show(m_mainBtnWii);
					break;
				default:
					m_btnMgr.show(m_mainBtnWii);
					break;
			}
			m_btnMgr.show(m_mainLblUser[2]);
			m_btnMgr.show(m_mainLblUser[3]);
		}
		else
		{
			m_btnMgr.hide(m_mainBtnChannel);
			m_btnMgr.hide(m_mainBtnWii);
			m_btnMgr.hide(m_mainBtnGamecube);
			m_btnMgr.hide(m_mainBtnPlugin);
			m_btnMgr.hide(m_mainLblUser[2]);
			m_btnMgr.hide(m_mainLblUser[3]);
		}
		if((disc_check & 0x2) && m_show_zone_main3 && !m_sourceflow && m_current_view != COVERFLOW_HOMEBREW)
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
	if(m_reload || BTN_B_HELD)// rebooting wiiflow
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
			if(_FindEmuPart(emuPath, false, false) >= 0)// make sure emunand folder exists
				ReturnPath = NandHandle.Get_NandPath();
		}
		Sys_SetNeekPath(ReturnPath);
	}
	cleanup();
	//gprintf("Saving configuration files\n");
	m_gcfg1.save(true);// save configs on power off or exit wiiflow
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	return 0;
}

void CMenu::_initMainMenu()
{
	TexData texHome;
	TexData texHomeS;
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

	TexHandle.fromImageFile(texHome, fmt("%s/btnquit.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texHomeS, fmt("%s/btnquits.png", m_imgsDir.c_str()));
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

	m_mainBtnConfig = _addPicButton("MAIN/CONFIG_BTN", texConfig, texConfigS, 70, 400, 48, 48);
	m_mainBtnHome = _addPicButton("MAIN/QUIT_BTN", texHome, texHomeS, 570, 400, 48, 48);
	m_mainBtnChannel = _addPicButton("MAIN/CHANNEL_BTN", texChannel, texChannels, 520, 400, 48, 48);
	m_mainBtnHomebrew = _addPicButton("MAIN/HOMEBREW_BTN", texHomebrew, texHomebrews, 20, 400, 48, 48);
	m_mainBtnWii = _addPicButton("MAIN/USB_BTN", texWii, texWiis, 520, 400, 48, 48);
	m_mainBtnGamecube = _addPicButton("MAIN/DML_BTN", texGamecube, texGamecubes, 520, 400, 48, 48);
	m_mainBtnPlugin = _addPicButton("MAIN/EMU_BTN", texPlugin, texPlugins, 520, 400, 48, 48);
	m_mainBtnDVD = _addPicButton("MAIN/DVD_BTN", texDVD, texDVDs, 470, 400, 48, 48);
	m_mainBtnNext = _addPicButton("MAIN/NEXT_BTN", texNext, texNextS, 540, 146, 80, 80);
	m_mainBtnPrev = _addPicButton("MAIN/PREV_BTN", texPrev, texPrevS, 20, 146, 80, 80);
	m_mainLblMessage = _addLabel("MAIN/MESSAGE", theme.lblFont, L"", 40, 40, 560, 140, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_mainBtnFavoritesOn = _addPicButton("MAIN/FAVORITES_ON", texFavOn, texFavOnS, 288, 400, 64, 64);
	m_mainBtnFavoritesOff = _addPicButton("MAIN/FAVORITES_OFF", texFavOff, texFavOffS, 288, 400, 64, 64);
	m_mainLblLetter = _addLabel("MAIN/LETTER", theme.titleFont, L"", 540, 40, 80, 80, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, emptyTex);
	m_mainLblNotice = _addLabel("MAIN/NOTICE", theme.titleFont, L"", 340, 40, 280, 80, theme.titleFontColor, FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE, emptyTex);
	m_mainLblCurMusic = _addLabel("MAIN/MUSIC", theme.btnFont, L"", 0, 20, 640, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
//#ifdef SHOWMEM
	m_mem1FreeSize = _addLabel("MEM1", theme.btnFont, L"", 40, 300, 480, 56, theme.btnFontColor, FTGX_JUSTIFY_LEFT, emptyTex);
	m_mem2FreeSize = _addLabel("MEM2", theme.btnFont, L"", 40, 356, 480, 56, theme.btnFontColor, FTGX_JUSTIFY_LEFT, emptyTex);
//#endif
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
	_setHideAnim(m_mainBtnHome, "MAIN/QUIT_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnChannel, "MAIN/CHANNEL_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnHomebrew, "MAIN/HOMEBREW_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnWii, "MAIN/USB_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnGamecube, "MAIN/DML_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnPlugin, "MAIN/EMU_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnDVD, "MAIN/DVD_BTN", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOn, "MAIN/FAVORITES_ON", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainBtnFavoritesOff, "MAIN/FAVORITES_OFF", 0, 40, 0.f, 0.f);
	_setHideAnim(m_mainLblMessage, "MAIN/MESSAGE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblLetter, "MAIN/LETTER", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblNotice, "MAIN/NOTICE", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mainLblCurMusic, "MAIN/MUSIC", 0, -100, 0.f, 0.f);
//#ifdef SHOWMEM
	_setHideAnim(m_mem1FreeSize, "MEM1", 0, 0, 0.f, 0.f);
	_setHideAnim(m_mem2FreeSize, "MEM2", 0, 0, 0.f, 0.f);
//#endif
	_hideMain(true);
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
	if((m_current_view == COVERFLOW_CHANNEL && neek2o()) || (m_source_cnt > 1 && !m_emuSaveNand))
		return;
	int FS_Type = 0;
	/* change partition if direction is not zero */
	if(direction != 0)
	{
		bool NeedFAT = m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_GAMECUBE || m_emuSaveNand == true;
		u8 limiter = 0;
		do
		{
			currentPartition = loopNum(currentPartition + direction, 9);
			FS_Type = DeviceHandle.GetFSType(currentPartition);
			limiter++;
		}
		while(limiter < 9 && (!DeviceHandle.IsInserted(currentPartition) ||
			(m_current_view != COVERFLOW_WII && FS_Type == PART_FS_WBFS) ||
			(NeedFAT && FS_Type != PART_FS_FAT)));
	}
	/* set partition to currentPartition */
	if(m_emuSaveNand)
		m_cfg.setInt(WII_DOMAIN, "savepartition", currentPartition);
	else if(direction == 0 || (direction != 0 && (m_current_view != COVERFLOW_CHANNEL || 
							(FS_Type != -1 && DeviceHandle.IsInserted(currentPartition)))))
		m_cfg.setInt(_domainFromView(), "partition", currentPartition);
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

int CMenu::_getCFVersion()
{
	if(m_current_view == COVERFLOW_PLUGIN && !m_sourceflow)
	{
		m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
		if(enabledPluginsCount == 1)
		{
			for(u8 i = 0; m_plugin.PluginExist(i); ++i)
			{
				if(m_plugin.GetEnableStatus(m_cfg, m_plugin.getPluginMagic(i)))
					return m_cfg.getInt("PLUGIN_CFVERSION", m_plugin.PluginMagicWord, 1);
			}
		}
		else if(strlen(single_sourcebtn))
			return m_cfg.getInt("PLUGIN_CFVERSION", single_sourcebtn, 1);
	}
	return m_cfg.getInt(_domainFromView(), "last_cf_mode", 1);
}

void CMenu::_setCFVersion(int version)
{
	if(m_current_view == COVERFLOW_PLUGIN && !m_sourceflow)
	{
		m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
		if(enabledPluginsCount == 1)
		{
			for(u8 i = 0; m_plugin.PluginExist(i); ++i)
			{
				if(m_plugin.GetEnableStatus(m_cfg, m_plugin.getPluginMagic(i)))
				{
					m_cfg.setInt("PLUGIN_CFVERSION", m_plugin.PluginMagicWord, version);
					return;
				}
			}
		}
		else if(strlen(single_sourcebtn))
		{
			m_cfg.setInt("PLUGIN_CFVERSION", single_sourcebtn, version);
			return;
		}
	}
	m_cfg.setInt(_domainFromView(), "last_cf_mode", version);
}
