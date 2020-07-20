
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
	m_btnMgr.hide(m_mainBtnCategories, instant);
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

void CMenu::_getCustomBgTex()
{
	if(m_sourceflow)
		_getSFlowBgTex();
	else
	{
		curCustBg = loopNum(curCustBg + 1, 2);
		string fn = "";
		if(m_platform.loaded())
		{
			m_plugin.PluginMagicWord[0] = '\0';
			u8 i = 0;
			switch(m_current_view)
			{
				case COVERFLOW_CHANNEL:
					if(m_cfg.getInt(CHANNEL_DOMAIN, "channels_type") & CHANNELS_REAL)
						strncpy(m_plugin.PluginMagicWord, "4E414E44", 9);
					else
						strncpy(m_plugin.PluginMagicWord, "454E414E", 9);
					break;
				case COVERFLOW_HOMEBREW:
					strncpy(m_plugin.PluginMagicWord, "48425257", 9);
					break;
				case COVERFLOW_GAMECUBE:
					strncpy(m_plugin.PluginMagicWord, "4E47434D", 9);
					break;
				case COVERFLOW_PLUGIN:
					while(m_plugin.PluginExist(i) && !m_plugin.GetEnabledStatus(i)) { ++i; }
					if(m_plugin.PluginExist(i))
						strncpy(m_plugin.PluginMagicWord, fmt("%08x", m_plugin.GetPluginMagic(i)), 8);
					break;
				default:// wii
					strncpy(m_plugin.PluginMagicWord, "4E574949", 9);
			}
			if(strlen(m_plugin.PluginMagicWord) == 8)
				fn = m_platform.getString("PLUGINS", m_plugin.PluginMagicWord, "");
		}
		if(fn.length() > 0)
		{
			if(TexHandle.fromImageFile(m_mainCustomBg[curCustBg], fmt("%s/%s/%s.png", m_bckgrndsDir.c_str(), m_themeName.c_str(), fn.c_str())) != TE_OK)
			{	
				if(TexHandle.fromImageFile(m_mainCustomBg[curCustBg], fmt("%s/%s/%s.jpg", m_bckgrndsDir.c_str(), m_themeName.c_str(), fn.c_str())) != TE_OK)
				{
					if(TexHandle.fromImageFile(m_mainCustomBg[curCustBg], fmt("%s/%s.png", m_bckgrndsDir.c_str(), fn.c_str())) != TE_OK)
					{
						if(TexHandle.fromImageFile(m_mainCustomBg[curCustBg], fmt("%s/%s.jpg", m_bckgrndsDir.c_str(), fn.c_str())) != TE_OK)
						{
							curCustBg = loopNum(curCustBg + 1, 2);// reset it
							customBg = false;
							return;
						}
					}
				}
			}
			customBg = true;
		}
		else
		{
			curCustBg = loopNum(curCustBg + 1, 2);// reset it
			customBg = false;
		}
	}
}

void CMenu::_setMainBg()
{
	if(customBg)
		_setBg(m_mainCustomBg[curCustBg], m_mainCustomBg[curCustBg]);
	else
		_setBg(m_mainBg, m_mainBgLQ);
}

void CMenu::_showMain()
{
	_setMainBg();
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
		
		/* create gameList based on sources selected */
		_loadList();

		_hideWaitMessage();

		/* if game list is empty display message letting user know */
		wstringEx Msg;
		wstringEx Pth;
		if(m_gameList.empty())
		{
			cacheCovers = false;
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
						Pth = "";
						if(enabledPluginsCount == 0)
							Msg = _t("main6", L"No plugins selected.");
						else if(enabledPluginsCount > 1)
							Msg = _t("main5", L"No roms/items found.");
						else
						{
							Msg = _t("main2", L"No games found in ");
							u8 i = 0;
							while(m_plugin.PluginExist(i) && !m_plugin.GetEnabledStatus(i)){ ++i; }
							int romsPartition = m_plugin.GetRomPartition(i);
							if(romsPartition < 0)
								romsPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition", 0);
							Pth = wstringEx(fmt("%s:/%s", DeviceName[romsPartition], m_plugin.GetRomDir(i)));
						}
						break;
				}
			}
			Msg.append(Pth);
			m_btnMgr.setText(m_mainLblMessage, Msg);
			m_btnMgr.show(m_mainLblMessage);
			return;
		}
		
		/* if source menu button set to autoboot */
		if(m_source_autoboot == true)
		{	/* search game list for the requested title */
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
			/* title found - launch it */
			if(game_found == true) 
			{
				gprintf("Game found, autobooting...\n");
				_launch(&m_autoboot_hdr);
			}
			/* fail */
			m_source_autoboot = false;
		}
		if(cacheCovers)
		{
			cacheCovers = false;
			if(!m_sourceflow || _sfCacheCoversNeeded() > 0)
			{
				m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
				m_btnMgr.setText(m_wbfsLblMessage, L"0%");
				m_btnMgr.setText(m_wbfsLblDialog, L"");
				m_btnMgr.show(m_wbfsPBar);
				m_btnMgr.show(m_wbfsLblMessage);
				m_btnMgr.show(m_wbfsLblDialog);
			
				_start_pThread();
				_cacheCovers();
				_stop_pThread();
				m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg14", L"Done."));
				u8 pause = 150;
				if(m_sourceflow)
					pause = 1;
				do
				{
					_mainLoopCommon();
					pause--;
					if(pause == 0)
					{
						m_btnMgr.hide(m_wbfsPBar);
						m_btnMgr.hide(m_wbfsLblMessage);
						m_btnMgr.hide(m_wbfsLblDialog);
					}
				}while(!m_exit && pause > 0);
			}
		}
		/* setup categories and favorites for filtering the game list below */
		if(m_clearCats)// false on boot up and if a source menu button selects a category
		{
			if(m_autoboot_hdr.type == TYPE_PLUGIN && m_cat.hasDomain("PLUGINS"))
			{
				m_cat.remove("PLUGINS", "selected_categories");
				m_cat.remove("PLUGINS", "required_categories");
			}
			else
			{
				// do not clear hidden categories to keep games hidden
				m_cat.remove("GENERAL", "selected_categories");
				m_cat.remove("GENERAL", "required_categories");
			}
		}
		m_clearCats = true;// set to true for next source
		
		m_favorites = false;
		if(m_getFavs || m_cfg.getBool("GENERAL", "save_favorites_mode", false))
			m_favorites = m_cfg.getBool(_domainFromView(), "favorites", false);
		else
			m_cfg.setBool(_domainFromView(), "favorites", false);
		m_getFavs = false;
	}
	
	strcpy(cf_domain, "_COVERFLOW");
	if(!m_sourceflow && m_current_view == COVERFLOW_HOMEBREW && m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", true))
		strcpy(cf_domain, "_SMALLFLOW");
	if(m_sourceflow && m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox", true))
		strcpy(cf_domain, "_SMALLFLOW");
	if(m_current_view == COVERFLOW_PLUGIN && !m_sourceflow)
	{
		/* check if homebrew plugin */
		if(enabledPluginsCount == 1 && m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(strtoul("48425257", NULL, 16))) && m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox"))
			strcpy(cf_domain, "_SMALLFLOW");
		else if(enabledPluginsCount > 0 && m_platform.loaded())
		{
			/* get first plugin flow domain */
			u8 i = 0;
			while(m_plugin.PluginExist(i) && !m_plugin.GetEnabledStatus(i)){ ++i; }
			string flow_domain = m_platform.getString("FLOWS", m_platform.getString("PLUGINS", sfmt("%08x", m_plugin.GetPluginMagic(i)), ""), "_COVERFLOW");
			
			/* check if all plugin flow domains match */
			bool match = true;
			i++;
			while(m_plugin.PluginExist(i))
			{
				if(m_plugin.GetEnabledStatus(i) &&
					flow_domain != m_platform.getString("FLOWS", m_platform.getString("PLUGINS", sfmt("%08x", m_plugin.GetPluginMagic(i)), ""), "_COVERFLOW"))
				{
					match = false;
					break;
				}
				i++;
			}

			/* if all match we use that flow domain */
			if(match)
				snprintf(cf_domain, sizeof(cf_domain), "%s", flow_domain.c_str());
		}
	}

	/* get the number of layouts (modes) for the CoverFlow domain */
	m_numCFVersions = min(max(1, m_coverflow.getInt(cf_domain, "number_of_modes", 1)), 15);// max layouts is 15
	
	/* get the current cf layout number and use it to load the data used for that layout */
	_loadCFLayout(min(max(1, _getCFVersion()), (int)m_numCFVersions));
	
	/* filter game list to create the cf cover list and start coverflow coverloader */	
	_initCF();
	
	/* set the covers and titles to the positions and angles based on the cf layout */
	CoverFlow.applySettings();

	gprintf("Displaying covers\n");

	/* display game count if not sourceflow or homebrew */
	if(m_sourceflow || m_current_view == COVERFLOW_HOMEBREW)
		return;

	m_showtimer = 240;
	m_btnMgr.setText(m_mainLblNotice, wfmt(_fmt("main7", L"Total Games: %i"), CoverFlow.size()));
	m_btnMgr.show(m_mainLblNotice);
}

int CMenu::main(void)
{
	wstringEx curLetter;
	string prevTheme = m_themeName;
	bool show_channel = !m_cfg.getBool(CHANNEL_DOMAIN, "disable", false);
	bool show_plugin = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
	bool show_gamecube = !m_cfg.getBool(GC_DOMAIN, "disable", false);
	bool show_homebrew = !m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false);
	bool m_source_on_start = m_cfg.getBool("GENERAL", "source_on_start", false);
	bool bheld = false;// bheld to indicate btn b was pressed or held
	bool bUsed = false;// bused to indicate that it was actually used for something
	m_emuSaveNand = false;
	m_reload = false;
	CFLocked = m_cfg.getBool("GENERAL", "cf_locked", false);
	Auto_hide_icons = m_cfg.getBool("GENERAL", "auto_hide_icons", true);
	u32 disc_check = 0;

	m_prev_view = 0;
	m_current_view = m_cfg.getUInt("GENERAL", "sources", COVERFLOW_WII);
	m_source_cnt = 0;
	for(u8 i = 1; i < 32; i <<= 1)
		if(m_current_view & i)
			m_source_cnt++;
			
	if(m_source_cnt == 0)
	{
		m_current_view = COVERFLOW_WII;
		m_cfg.setUInt("GENERAL", "sources", m_current_view);
		m_source_cnt++;
	}
	
	m_catStartPage = m_cfg.getInt("GENERAL", "cat_startpage", 1);
	
	if(m_cfg.getBool("GENERAL", "update_cache", false))
	{
		m_cfg.setBool("GENERAL", "update_cache", false);
		fsop_deleteFolder(m_listCacheDir.c_str());
		fsop_MakeFolder(m_listCacheDir.c_str());
	}
	m_vid.set2DViewport(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480),
						m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));

	gprintf("Bootup completed!\n");

	if(!m_use_source || !m_source_on_start)
	{
		m_source_on_start = false;
		_getCustomBgTex();
		_setMainBg();
		_showCF(true);
	}

	if(show_mem)
	{
		m_btnMgr.show(m_mem1FreeSize);
		m_btnMgr.show(m_mem2FreeSize);
	}
	SetupInput(true);

	while(!m_exit)
	{
		/* IMPORTANT check if a disc is inserted */
		WDVD_GetCoverStatus(&disc_check);
		/* Main Loop */
		if(!m_source_on_start)
			_mainLoopCommon(true);
		//this will make the source menu/flow display. what happens when a sourceflow cover is selected is taken care of later.
		if(m_source_on_start || (bheld && !BTN_B_HELD))//if button b was held and now released
		{
			bheld = false;
			if(bUsed)//if b button used for something don't show souce menu or sourceflow
				bUsed = false;
			else
			{
				if(m_sourceflow)//back a tier or exit sourceflow
				{
					if(!_srcTierBack(false))// not back a tier - exit sourceflow and return to coverflow
					{
						_restoreSrcTiers();
						m_sourceflow = false;
					}
					_getCustomBgTex();
					_setMainBg();
					_showCF(true);//refresh coverflow or sourceflow list
					continue;
				}
				else if(m_use_source)//if source_menu enabled
				{
					_hideMain();
					if(m_cfg.getBool(SOURCEFLOW_DOMAIN, "enabled", false))//if sourceflow show it
					{
						sm_numbers_backup = m_cfg.getString(SOURCEFLOW_DOMAIN, "numbers");//backup for possible restore later
						sm_tiers_backup = m_cfg.getString(SOURCEFLOW_DOMAIN, "tiers");
						m_sourceflow = true;
						_getCustomBgTex();
						_setMainBg();
						_showCF(true);//refresh sourceflow list
					}
					else //show source menu
					{
						m_refreshGameList = _Source();
						if(BTN_B_HELD)
							bUsed = true;
						_getCustomBgTex();
						_setMainBg();
						_showCF(m_refreshGameList || m_source_on_start);//refresh coverflow list if new source selected
					}
					m_source_on_start = false;
					continue;
				}
			}
		}
		if(BTN_HOME_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_mainBtnHome)))
		{
			if(m_sourceflow)//back to base tier or exit sourceflow
			{
				if(!_srcTierBack(true))// if already on base tier exit sourceflow
				{
					_restoreSrcTiers();
					m_sourceflow = false;
				}
				_getCustomBgTex();
				_setMainBg();
				_showCF(true);//refresh coverflow or sourceflow list
			}
			else
			{
				_hideMain();
				/* Home menu */
				if(_Home())
					break;// exit wiiflow
				if(prevTheme != m_themeName)
				{
					/* new theme - exit wiiflow and reload */
					fsop_deleteFolder(fmt("%s/sourceflow", m_cacheDir.c_str()));
					m_cfg.remove(SOURCEFLOW_DOMAIN, "numbers");
					m_cfg.remove(SOURCEFLOW_DOMAIN, "tiers");
					m_reload = true;
					break;
				}
				if(BTN_B_HELD)
				{
					bheld = true;
					bUsed = true;
				}
				show_channel = !m_cfg.getBool(CHANNEL_DOMAIN, "disable", false);
				show_plugin = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
				show_gamecube = !m_cfg.getBool(GC_DOMAIN, "disable", false);
				show_homebrew = !m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false);
				_showMain();
			}
		}
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_mainBtnPrev))
				CoverFlow.pageUp();
		 	else if(m_btnMgr.selected(m_mainBtnNext))
				CoverFlow.pageDown();
			else if(m_btnMgr.selected(m_mainBtnChannel) || m_btnMgr.selected(m_mainBtnWii) || m_btnMgr.selected(m_mainBtnGamecube) 
					|| m_btnMgr.selected(m_mainBtnPlugin) || m_btnMgr.selected(m_mainBtnHomebrew))
			{
				/* change source via view button on main menu */
				if(m_current_view == COVERFLOW_WII) 
					m_current_view = show_gamecube ? COVERFLOW_GAMECUBE : (show_channel ? COVERFLOW_CHANNEL : 
									(show_plugin ? COVERFLOW_PLUGIN : (show_homebrew ? COVERFLOW_HOMEBREW : COVERFLOW_WII)));
				else if(m_current_view == COVERFLOW_GAMECUBE)
					m_current_view = show_channel ? COVERFLOW_CHANNEL : (show_plugin ? COVERFLOW_PLUGIN : (show_homebrew ? COVERFLOW_HOMEBREW : COVERFLOW_WII));
				else if(m_current_view == COVERFLOW_CHANNEL)
					m_current_view = show_plugin ? COVERFLOW_PLUGIN : (show_homebrew ? COVERFLOW_HOMEBREW : COVERFLOW_WII);
				else if(m_current_view == COVERFLOW_PLUGIN)
					m_current_view = show_homebrew ? COVERFLOW_HOMEBREW : COVERFLOW_WII;
				else if(m_current_view == COVERFLOW_HOMEBREW || m_source_cnt > 1)
					m_current_view = COVERFLOW_WII;
				if(m_use_source)
				{
					sm_numbers_backup = "0";
					sm_tiers_backup = SOURCE_FILENAME;
					_restoreSrcTiers();
				}
				m_source_cnt = 1;
				m_cfg.setUInt("GENERAL", "sources", m_current_view);
				m_catStartPage = 1;
				_getCustomBgTex();
				_setMainBg();
				_showCF(true);
			}
			else if(m_btnMgr.selected(m_mainBtnConfig))
			{
				// main menu global settings
				_hideMain();
				_config(1);
				if(prevTheme != m_themeName)
				{
					// new theme - exit wiiflow and reload
					fsop_deleteFolder(fmt("%s/sourceflow", m_cacheDir.c_str()));
					m_cfg.remove(SOURCEFLOW_DOMAIN, "numbers");
					m_cfg.remove(SOURCEFLOW_DOMAIN, "tiers");
					m_reload = true;
					break;
				}
				if(BTN_B_HELD)
				{
					bheld = true;
					bUsed = true;
				}
				show_channel = !m_cfg.getBool(CHANNEL_DOMAIN, "disable", false);
				show_plugin = !m_cfg.getBool(PLUGIN_DOMAIN, "disable", false);
				show_gamecube = !m_cfg.getBool(GC_DOMAIN, "disable", false);
				show_homebrew = !m_cfg.getBool(HOMEBREW_DOMAIN, "disable", false);
				_showMain();
			}
			else if(m_btnMgr.selected(m_mainBtnCategories))
			{
				_hideMain();
				_CategorySettings();
				if(BTN_B_HELD)// returned using the b btn
				{
					bheld = true;
					bUsed = true;
				}
				_setMainBg();
				if(m_refreshGameList)
				{
					m_refreshGameList = false;
					_initCF();
				}
			}
			else if(m_btnMgr.selected(m_mainBtnDVD))
			{
				if(disc_check & 0x2)
				{
					/* Boot DVD in drive */
					_hideMain(true);
					/* Create Fake Header */
					dir_discHdr hdr;
					memset(&hdr, 0, sizeof(dir_discHdr));
					memcpy(&hdr.id, "dvddvd", 6);//this must be set for neek2o
					/* Boot the Disc */
					_launchWii(&hdr, true, BTN_B_HELD);
					if(BTN_B_HELD)
					{
						bheld = true;
						bUsed = true;
					}
					_showCF(false);
				}
				else
				{
					error(_t("main8", L"No disc in drive!"));
					if(BTN_B_HELD)
					{
						bheld = true;
						bUsed = true;
					}
					_showMain();
				}
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
					_sourceFlow();// set the source selected or new source tier
					_getCustomBgTex();
					_setMainBg();
					_showCF(true);// refresh coverflow or sourceflow list
					continue;
				}
				else
				{
					_game(BTN_B_HELD);
					if(m_exit)
						break;
					if(BTN_B_HELD)
					{
						bheld = true;
						bUsed = true;
					}
					_setMainBg();
					if(m_refreshGameList)// if changes were made to favorites, parental lock, or categories
					{
						m_refreshGameList = false;
						_initCF();
					}
					else
						CoverFlow.cancel();
				}
			}
		}
		else if(BTN_B_PRESSED)
		{
			bheld = true;
			if(m_btnMgr.selected(m_mainBtnNext) || m_btnMgr.selected(m_mainBtnPrev))
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
		}
		else if(WROLL_RIGHT)
		{
			CoverFlow.right();
		}
		if(!BTN_B_HELD)
		{
			/* move coverflow */
			if(BTN_UP_REPEAT || RIGHT_STICK_UP)
				CoverFlow.up();
			else if(BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT)
				CoverFlow.right();
			else if(BTN_DOWN_REPEAT ||  RIGHT_STICK_DOWN)
				CoverFlow.down();
			else if(BTN_LEFT_REPEAT || RIGHT_STICK_LEFT)
				CoverFlow.left();
			else if(BTN_MINUS_PRESSED)
				CoverFlow.pageUp();
			else if(BTN_PLUS_PRESSED)
				CoverFlow.pageDown();
				
			/* change coverflow layout/mode */
			else if((BTN_1_PRESSED || BTN_2_PRESSED) && !CFLocked && !CoverFlow.empty())
			{
				u32 curPos = CoverFlow._currentPos();
				s8 direction = BTN_1_PRESSED ? 1 : -1;
				int cfVersion = 1 + loopNum((_getCFVersion() - 1) + direction, m_numCFVersions);
				_setCFVersion(cfVersion);
				_loadCFLayout(cfVersion);
				CoverFlow._setCurPos(curPos);
				CoverFlow.applySettings();
			}
		}
		else // Button B Held
		{
			bheld = true;
			const char *domain = _domainFromView();
			
			/* b+down or up = move to previous or next cover in sort order */
			if(!CoverFlow.empty() && (BTN_DOWN_PRESSED || BTN_UP_PRESSED))
			{
				bUsed = true;
				int sorting = m_cfg.getInt(domain, "sort", SORT_ALPHA);
				/* if for some reason domain sort value is not legal set it to alpha */
				if(sorting != SORT_ALPHA && sorting != SORT_PLAYERS && sorting != SORT_WIFIPLAYERS && sorting != SORT_GAMEID)
				{
					CoverFlow.setSorting((Sorting)SORT_ALPHA);
					m_cfg.setInt(domain, "sort", SORT_ALPHA);
					sorting = SORT_ALPHA;
				}
				/* move coverflow */
				wchar_t c[2] = {0, 0};
				BTN_UP_PRESSED ? CoverFlow.prevLetter(c) : CoverFlow.nextLetter(c);

				/* set sort text and display it */
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
			else if(BTN_LEFT_PRESSED)// b+left = previous song
			{
				bUsed = true;
				MusicPlayer.Previous();
			}
			else if(BTN_RIGHT_PRESSED)// b+right = next song
			{
				bUsed = true;
				MusicPlayer.Next();
			}
			/* b+plus = change sort mode */
			else if(!CoverFlow.empty() && BTN_PLUS_PRESSED && !m_locked && (m_current_view < 8 || m_sourceflow))// <8 excludes plugins and homebrew
			{
				bUsed = true;
				u8 sort = 0;
				if(m_sourceflow)// change sourceflow sort mode
				{
					sort = m_cfg.getInt(SOURCEFLOW_DOMAIN, "sort", SORT_ALPHA);
					if(sort == SORT_ALPHA)
						sort = SORT_BTN_NUMBERS;
					else
						sort = SORT_ALPHA;
				}
				else // change all other coverflow sort mode
					sort = loopNum((m_cfg.getInt(domain, "sort", 0)) + 1, SORT_MAX);
				m_cfg.setInt(domain, "sort", sort);
				
				/* set coverflow to new sorting */
				_initCF();
				/* set sort mode text and display it */
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
				else if(sort == SORT_BTN_NUMBERS)
					curSort = m_loc.getWString(m_curLanguage, "bybtnnumbers", L"By Button Numbers");
				m_showtimer = 120;
				m_btnMgr.setText(m_mainLblNotice, curSort);
				m_btnMgr.show(m_mainLblNotice);
			}
			/* b+minus = select random game or boot random game */ 
			else if(BTN_MINUS_PRESSED && !CoverFlow.empty())
			{
				_hideMain();
				srand(time(NULL));
				u16 place = (rand() + rand() + rand()) % CoverFlow.size();
				
				if(m_cfg.getBool("GENERAL", "random_select", false))// random select a game
				{
					CoverFlow.setSelected(place);
					_game(false);
					if(m_exit)
						break;
					if(BTN_B_HELD)
					{
						bheld = true;
						bUsed = true;
					}
					else
						bheld = false;
					if(m_refreshGameList)
					{
						/* if changes were made to favorites, parental lock, or categories */
						_initCF();
						m_refreshGameList = false;
					}
					else
						CoverFlow.cancel();
				}
				else // boot a random game
				{
					gprintf("Lets boot the random game number %u\n", place);
					const dir_discHdr *gameHdr = CoverFlow.getSpecificHdr(place);
					if(gameHdr != NULL)
						_launch(gameHdr);
					_showCF(false);// this shouldn't happen
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
		if(!m_gameList.empty() && m_show_zone_prev && !m_sourceflow)
			m_btnMgr.show(m_mainBtnPrev);
		else
			m_btnMgr.hide(m_mainBtnPrev);
			
		if(!m_gameList.empty() && m_show_zone_next && !m_sourceflow)
			m_btnMgr.show(m_mainBtnNext);
		else
			m_btnMgr.hide(m_mainBtnNext);
			
		if((!Auto_hide_icons || m_show_zone_main) && !m_sourceflow)
		{
			m_btnMgr.show(m_mainLblUser[0]);
			m_btnMgr.show(m_mainLblUser[1]);
			m_btnMgr.show(m_mainBtnCategories);
			m_btnMgr.show(m_mainBtnConfig);
			m_btnMgr.show(m_mainBtnHome);
			m_btnMgr.show(m_favorites ? m_mainBtnFavoritesOn : m_mainBtnFavoritesOff);
			m_btnMgr.hide(m_favorites ? m_mainBtnFavoritesOff : m_mainBtnFavoritesOn);
		}
		else
		{
			m_btnMgr.hide(m_mainLblUser[0]);
			m_btnMgr.hide(m_mainLblUser[1]);
			m_btnMgr.hide(m_mainBtnConfig);
			m_btnMgr.hide(m_mainBtnCategories);
			m_btnMgr.hide(m_mainBtnHome);
			m_btnMgr.hide(m_mainBtnFavoritesOn);
			m_btnMgr.hide(m_mainBtnFavoritesOff);
		}
		if(!m_cfg.getBool("GENERAL", "hideviews", false) && (!Auto_hide_icons || m_show_zone_main2) && !m_sourceflow)
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
					else if(show_homebrew)
						m_btnMgr.show(m_mainBtnHomebrew);
					else
						m_btnMgr.show(m_mainBtnWii);
					break;
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
			m_btnMgr.hide(m_mainBtnHomebrew);
			m_btnMgr.hide(m_mainLblUser[2]);
			m_btnMgr.hide(m_mainLblUser[3]);
		}
		if((!Auto_hide_icons || m_show_zone_main3) && !m_sourceflow)
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
		vector<string> arguments = _getMetaXML(fmt("%s/boot.dol", m_appDir.c_str()));
		_launchHomebrew(fmt("%s/boot.dol", m_appDir.c_str()), arguments);
		return 0;
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
	TexData texCategories;
	TexData texCategoriesS;
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

	TexHandle.fromImageFile(texCategories, fmt("%s/btncat.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texCategoriesS, fmt("%s/btncats.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texHome, fmt("%s/btnquit.png", m_imgsDir.c_str()));// home button
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
	TexHandle.fromImageFile(texFavOn, fmt("%s/gamefavon.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texFavOnS, fmt("%s/gamefavons.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texFavOff, fmt("%s/gamefavoff.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texFavOffS, fmt("%s/gamefavoffs.png", m_imgsDir.c_str()));

	_addUserLabels(m_mainLblUser, ARRAY_SIZE(m_mainLblUser), "MAIN");

	m_mainBtnCategories = _addPicButton("MAIN/CATEGORIES_BTN", texCategories, texCategoriesS, 126, 400, 48, 48);
	m_mainBtnFavoritesOn = _addPicButton("MAIN/FAVORITES_ON", texFavOn, texFavOnS, 194, 400, 48, 48);
	m_mainBtnFavoritesOff = _addPicButton("MAIN/FAVORITES_OFF", texFavOff, texFavOffS, 194, 400, 48, 48);
	m_mainBtnConfig = _addPicButton("MAIN/CONFIG_BTN", texConfig, texConfigS, 262, 400, 48, 48);
	m_mainBtnHome = _addPicButton("MAIN/QUIT_BTN", texHome, texHomeS, 330, 400, 48, 48);
	m_mainBtnChannel = _addPicButton("MAIN/CHANNEL_BTN", texChannel, texChannels, 398, 400, 48, 48);
	m_mainBtnHomebrew = _addPicButton("MAIN/HOMEBREW_BTN", texHomebrew, texHomebrews, 398, 400, 48, 48);
	m_mainBtnWii = _addPicButton("MAIN/USB_BTN", texWii, texWiis, 398, 400, 48, 48);
	m_mainBtnGamecube = _addPicButton("MAIN/DML_BTN", texGamecube, texGamecubes, 398, 400, 48, 48);
	m_mainBtnPlugin = _addPicButton("MAIN/EMU_BTN", texPlugin, texPlugins, 398, 400, 48, 48);
	m_mainBtnDVD = _addPicButton("MAIN/DVD_BTN", texDVD, texDVDs, 466, 400, 48, 48);
	
	m_mainBtnNext = _addPicButton("MAIN/NEXT_BTN", texNext, texNextS, 540, 146, 80, 80);
	m_mainBtnPrev = _addPicButton("MAIN/PREV_BTN", texPrev, texPrevS, 20, 146, 80, 80);

	m_mainLblMessage = _addLabel("MAIN/MESSAGE", theme.lblFont, L"", 40, 40, 560, 140, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_mainLblLetter = _addLabel("MAIN/LETTER", theme.titleFont, L"", 540, 40, 80, 80, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, emptyTex);
	m_mainLblNotice = _addLabel("MAIN/NOTICE", theme.txtFont, L"", 340, 40, 280, 80, theme.titleFontColor, FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE);
	m_mainLblCurMusic = _addLabel("MAIN/MUSIC", theme.txtFont, L"", 0, 10, 640, 32, theme.txtFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);

	m_mem1FreeSize = _addLabel("MEM1", theme.btnFont, L"", 40, 300, 480, 56, theme.btnFontColor, FTGX_JUSTIFY_LEFT, emptyTex);
	m_mem2FreeSize = _addLabel("MEM2", theme.btnFont, L"", 40, 356, 480, 56, theme.btnFontColor, FTGX_JUSTIFY_LEFT, emptyTex);
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
	_setHideAnim(m_mainBtnCategories, "MAIN/CATEGORIES_BTN", 0, 40, 0.f, 0.f);
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
	if(m_source_cnt > 1 && !m_emuSaveNand)
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
		if(!DeviceHandle.IsInserted(SD) || stat("sd:/bootmii/armboot.bin", &dummy) != 0 || stat("sd:/bootmii/ppcboot.elf", &dummy) != 0)
			ExitTo = EXIT_TO_HBC;
	}
	if(ExitTo != WIIFLOW_DEF)// if not using wiiflows exit option then go ahead and set the exit to
		Sys_ExitTo(ExitTo);
}

int CMenu::_getCFVersion()
{
	if(m_sourceflow)
		return _getSrcFlow();
	else if(m_current_view == COVERFLOW_PLUGIN)
	{
		int first = 0;
		for(u8 i = 0; m_plugin.PluginExist(i); ++i)
		{
			if(m_plugin.GetEnabledStatus(i))
			{
				string magic = sfmt("%08x", m_plugin.GetPluginMagic(i));
				if(m_cfg.has("PLUGIN_CFVERSION", magic))
				{
					if(first > 0 && m_cfg.getInt("PLUGIN_CFVERSION", magic, 1) != first)
						return m_cfg.getInt(_domainFromView(), "last_cf_mode", 1);
					else if(first == 0)	
						first = m_cfg.getInt("PLUGIN_CFVERSION", magic, 1);
				}
			}
		}
		if(first == 0)
			first++;
		return first;
	}
	return m_cfg.getInt(_domainFromView(), "last_cf_mode", 1);
}

void CMenu::_setCFVersion(int version)
{
	if(m_sourceflow)
		_setSrcFlow(version);
	else if(m_current_view == COVERFLOW_PLUGIN)
	{
		int first = 0;
		for(u8 i = 0; m_plugin.PluginExist(i); ++i)
		{
			if(m_plugin.GetEnabledStatus(i))
			{
				string magic = sfmt("%08x", m_plugin.GetPluginMagic(i));
				if(m_cfg.has("PLUGIN_CFVERSION", magic))
				{
					if(first > 0 && m_cfg.getInt("PLUGIN_CFVERSION", magic, 1) != first)
					{
						m_cfg.setInt(_domainFromView(), "last_cf_mode", version);
						return;
					}
					else if(first == 0)	
						first = m_cfg.getInt("PLUGIN_CFVERSION", magic, 1);
				}
			}
		}
		for(u8 i = 0; m_plugin.PluginExist(i); ++i)
		{
			if(m_plugin.GetEnabledStatus(i))
				m_cfg.setInt("PLUGIN_CFVERSION", sfmt("%08x", m_plugin.GetPluginMagic(i)), version);
		}
	}
	else
		m_cfg.setInt(_domainFromView(), "last_cf_mode", version);
}
