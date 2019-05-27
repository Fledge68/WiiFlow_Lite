
#include "menu.hpp"
#include "banner/BannerWindow.hpp"
#include "gc/gcdisc.hpp"
#include "gui/WiiMovie.hpp"
#include "loader/playlog.h"

//sounds
extern const u8 gc_ogg[];
extern const u32 gc_ogg_size;

bool m_zoom_banner = false;
bool m_banner_loaded = false;
bool m_set_bg = false;
s16 m_gameBtnPlayFull;
s16 m_gameBtnBackFull;
s16 m_gameBtnToggle;
s16 m_gameBtnToggleFull;

const CMenu::SOption CMenu::_GlobalVideoModes[6] = {
	{ "vidgame", L"Game" },
	{ "vidsys", L"System" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "vidprog", L"Progressive" },
};

const CMenu::SOption CMenu::_VideoModes[7] = {
	{ "viddef", L"Default" },
	{ "vidgame", L"Game" },
	{ "vidsys", L"System" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "vidprog", L"Progressive" },
};

const CMenu::SOption CMenu::_GlobalGCvideoModes[6] = {
	{ "vidgame", L"Game" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "DMLmpal", L"MPAL" },
	{ "vidprog", L"Progressive" },
};

const CMenu::SOption CMenu::_GCvideoModes[7] = {
	{ "viddef", L"Default" },
	{ "vidgame", L"Game" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "DMLmpal", L"MPAL" },
	{ "vidprog", L"Progressive" },
};

const CMenu::SOption CMenu::_languages[11] = {
	{ "lngdef", L"Default" },// next should be console
	{ "lngjap", L"Japanese" },
	{ "lngeng", L"English" },
	{ "lngger", L"German" },
	{ "lngfre", L"French" },
	{ "lngspa", L"Spanish" },
	{ "lngita", L"Italian" },
	{ "lngdut", L"Dutch" },
	{ "lngsch", L"S. Chinese" },
	{ "lngtch", L"T. Chinese" },
	{ "lngkor", L"Korean" }
};

const CMenu::SOption CMenu::_GlobalGClanguages[7] = {
	{ "lngsys", L"System" },
	{ "lngeng", L"English" },
	{ "lngger", L"German" },
	{ "lngfre", L"French" },
	{ "lngspa", L"Spanish" },
	{ "lngita", L"Italian" },
	{ "lngdut", L"Dutch" }
};

const CMenu::SOption CMenu::_GClanguages[8] = {
	{ "lngdef", L"Default" },
	{ "lngsys", L"System" },
	{ "lngeng", L"English" },
	{ "lngger", L"German" },
	{ "lngfre", L"French" },
	{ "lngspa", L"Spanish" },
	{ "lngita", L"Italian" },
	{ "lngdut", L"Dutch" }
};

const CMenu::SOption CMenu::_ChannelsType[3] = {
	{ "ChanReal", L"Real NAND" },
	{ "ChanEmu", L"Emu NAND" },
	{ "ChanBoth", L"Both" },
};

const CMenu::SOption CMenu::_NandEmu[2] = {
	{ "NANDpart", L"Partial" },
	{ "NANDfull", L"Full" },
};

const CMenu::SOption CMenu::_GlobalSaveEmu[3] = {
	{ "SaveOffG", L"Off" },
	{ "SavePartG", L"Game save" },
	{ "SaveFullG", L"Full" },
};

const CMenu::SOption CMenu::_SaveEmu[4] = {
	{ "SaveDef", L"Default" },
	{ "SaveOff", L"Off" },
	{ "SavePart", L"Game save" },
	{ "SaveFull", L"Full" },
};

const CMenu::SOption CMenu::_AspectRatio[3] = {
	{ "aspectDef", L"Default" },
	{ "aspect43", L"Force 4:3" },
	{ "aspect169", L"Force 16:9" },
};

const CMenu::SOption CMenu::_NinEmuCard[5] = {
	{ "NinMCDef", L"Default" },
	{ "NinMCOff", L"Disabled" },
	{ "NinMCon", L"Enabled" },
	{ "NinMCMulti", L"Multi Saves" },
	{ "NinMCdebug", L"Debug" },
};

const CMenu::SOption CMenu::_GlobalGCLoaders[2] = {
	{ "GC_Devo", L"Devolution" },
	{ "GC_Nindnt", L"Nintendont" },
};

const CMenu::SOption CMenu::_GCLoader[3] = {
	{ "GC_Def", L"Default" },
	{ "GC_Devo", L"Devolution" },
	{ "GC_Nindnt", L"Nintendont" },
};

const CMenu::SOption CMenu::_vidModePatch[4] = {
	{ "vmpnone", L"None" },
	{ "vmpnormal", L"Normal" },
	{ "vmpmore", L"More" },
	{ "vmpall", L"All" }
};

const CMenu::SOption CMenu::_hooktype[8] = {
	{ "hook_auto", L"AUTO" },
	{ "hooktype1", L"VBI" },
	{ "hooktype2", L"KPAD read" },
	{ "hooktype3", L"Joypad" },
	{ "hooktype4", L"GXDraw" },
	{ "hooktype5", L"GXFlush" },
	{ "hooktype6", L"OSSleepThread" },
	{ "hooktype7", L"AXNextFrame" },
};

const CMenu::SOption CMenu::_debugger[3] = {
	{ "disabled", L"Disabled" },
	{ "dbg_gecko", L"Gecko" },
	{ "dbgfwrite", L"OSReport" },
};

static inline int loopNum(int i, int s)
{
	return (i + s) % s;
}

static int GetLanguage(const char *lang)
{
	if (strncmp(lang, "JP", 2) == 0) return CONF_LANG_JAPANESE;
	else if (strncmp(lang, "EN", 2) == 0) return CONF_LANG_ENGLISH;
	else if (strncmp(lang, "DE", 2) == 0) return CONF_LANG_GERMAN;
	else if (strncmp(lang, "FR", 2) == 0) return CONF_LANG_FRENCH;
	else if (strncmp(lang, "ES", 2) == 0) return CONF_LANG_SPANISH;
	else if (strncmp(lang, "IT", 2) == 0) return CONF_LANG_ITALIAN;
	else if (strncmp(lang, "NL", 2) == 0) return CONF_LANG_DUTCH;
	else if (strncmp(lang, "ZHTW", 4) == 0) return CONF_LANG_TRAD_CHINESE;
	else if (strncmp(lang, "ZH", 2) == 0) return CONF_LANG_SIMP_CHINESE;
	else if (strncmp(lang, "KO", 2) == 0) return CONF_LANG_KOREAN;
	
	return CONF_LANG_ENGLISH; // Default to EN
}

void CMenu::_extractBnr(const dir_discHdr *hdr)
{
	u32 size = 0;
	DeviceHandle.OpenWBFS(currentPartition);
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->id, (char *) hdr->path);
	if(disc != NULL)
	{
		void *bnr = NULL;
		size = wbfs_extract_file(disc, (char*)"opening.bnr", &bnr);
		if(size > 0)
			CurrentBanner.SetBanner((u8*)bnr, size, false, true);
		WBFS_CloseDisc(disc);
	}
	WBFS_Close();
}

void CMenu::_setCurrentItem(const dir_discHdr *hdr)
{
	const char *title = CoverFlow.getFilenameId(hdr);
	if(m_current_view == COVERFLOW_PLUGIN)
	{
		if(hdr->type == TYPE_PLUGIN)
			strncpy(m_plugin.PluginMagicWord, fmt("%08x", hdr->settings[0]), 8);
		else
		{
			if(hdr->type == TYPE_WII_GAME)
				strncpy(m_plugin.PluginMagicWord, "4E574949", 9);
			else if(hdr->type == TYPE_GC_GAME)
				strncpy(m_plugin.PluginMagicWord, "4E47434D", 9);
			else if(hdr->type == TYPE_CHANNEL)
				strncpy(m_plugin.PluginMagicWord, "4E414E44", 9);
			else if(hdr->type == TYPE_EMUCHANNEL)
				strncpy(m_plugin.PluginMagicWord, "454E414E", 9);
			else //HOMEBREW
				strncpy(m_plugin.PluginMagicWord, "48425257", 9);
		}
		m_cfg.setString(PLUGIN_DOMAIN, "cur_magic", m_plugin.PluginMagicWord);
		m_cfg.setString("plugin_item", m_plugin.PluginMagicWord, title);
	}
	else
	{
		m_cfg.setString(_domainFromView(), "current_item", title);
		if(m_source_cnt > 1)
			m_cfg.setInt("MULTI", "current_item_type", hdr->type);
	}
}

void CMenu::_hideGame(bool instant)
{
	_cleanupVideo();
	m_fa.unload();
	CoverFlow.showCover();

	m_btnMgr.hide(m_gameBtnPlay, instant);
	m_btnMgr.hide(m_gameBtnBack, instant);
	m_btnMgr.hide(m_gameBtnPlayFull, instant);
	m_btnMgr.hide(m_gameBtnBackFull, instant);
	m_btnMgr.hide(m_gameBtnDelete, instant);
	m_btnMgr.hide(m_gameBtnSettings, instant);
	m_btnMgr.hide(m_gameBtnToggle, instant);
	m_btnMgr.hide(m_gameBtnToggleFull, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOn, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOff, instant);
	m_btnMgr.hide(m_gameBtnCategories, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if(m_gameLblUser[i] != -1)
			m_btnMgr.hide(m_gameLblUser[i], instant);
}

void CMenu::_showGame(void)
{
	const dir_discHdr *GameHdr = CoverFlow.getHdr();
	const char *coverDir = NULL;
	const char *Path = NULL;
	if(GameHdr->type == TYPE_PLUGIN)
		coverDir = m_plugin.GetCoverFolderName(GameHdr->settings[0]);
	
	if(coverDir == NULL || strlen(coverDir) == 0)
		Path = fmt("%s", m_fanartDir.c_str());
	else
		Path = fmt("%s/%s", m_fanartDir.c_str(), coverDir);
	if(m_fa.load(m_cfg, Path, CoverFlow.getHdr()))
	{
		const TexData *bg = NULL;
		const TexData *bglq = NULL;
		m_fa.getBackground(bg, bglq);
		_setBg(*bg, *bglq);
		CoverFlow.hideCover();
	}
	else if(m_set_bg)
	{
		CoverFlow.showCover();		
		_setMainBg();
	}
	m_set_bg = true;
}

void CMenu::_cleanupBanner(bool gamechange)
{
	//banner
	m_gameSound.FreeMemory();
	_stopGameSoundThread();// stop banner and gamesound loading
	m_banner.DeleteBanner(gamechange);
	//movie
	_cleanupVideo();
}

void CMenu::_cleanupVideo()
{
	m_video_playing = false;
	movie.DeInit();
}

static const char *getVideoPath(const string &videoDir, const char *videoId)
{
	const char *coverDir = NULL;
	const char *videoPath = NULL;
	if(CoverFlow.getHdr()->type == TYPE_PLUGIN)
		coverDir = m_plugin.GetCoverFolderName(CoverFlow.getHdr()->settings[0]);
	
	if(coverDir == NULL || strlen(coverDir) == 0)
		videoPath = fmt("%s/%s", videoDir.c_str(), videoId);
	else
		videoPath = fmt("%s/%s/%s", videoDir.c_str(), coverDir, videoId);
	return videoPath;
}

static const char *getVideoDefaultPath(const string &videoDir)
{
	//strncpy(m_plugin.PluginMagicWord, fmt("%08x", CoverFlow.getHdr()->settings[0]), 8);
	const char *videoPath = fmt("%s/%s", videoDir.c_str(), m_plugin.PluginMagicWord);
	return videoPath;
}

bool CMenu::_startVideo()
{
	const dir_discHdr *GameHdr = CoverFlow.getHdr();
	
	const char *videoId = NULL;
	char curId3[4];
	memset(curId3, 0, 4);
	if(!NoGameID(GameHdr->type))
	{	//id3
		memcpy(curId3, GameHdr->id, 3);
		videoId = curId3;
	}
	else
		videoId = CoverFlow.getFilenameId(GameHdr);//title.ext

	//dev:/wiiflow/trailers/{coverfolder}/title.ext.thp or dev:/wiiflow/trailers/id3.thp
	const char *videoPath = getVideoPath(m_videoDir, videoId);
	const char *THP_Path = fmt("%s.thp", videoPath);
	if(!fsop_FileExist(THP_Path))
	{
		if(GameHdr->type == TYPE_PLUGIN)
		{
			//dev:/wiiflow/trailers/magic#.thp
			videoPath = getVideoDefaultPath(m_videoDir);
			THP_Path = fmt("%s.thp", videoPath);
		}
		else if(!NoGameID(GameHdr->type))
		{
			//id6
			videoPath = getVideoPath(m_videoDir, GameHdr->id);
			THP_Path = fmt("%s.thp", videoPath);
		}
	}
	if(fsop_FileExist(THP_Path))
	{
		m_gameSound.FreeMemory();
		_stopGameSoundThread();
		m_banner.SetShowBanner(false);
		/* Lets play the movie */
		movie.Init(THP_Path);
		m_gameSound.Load(fmt("%s.ogg", videoPath));
		m_gameSound.SetVolume(m_cfg.getInt("GENERAL", "sound_volume_bnr", 255));
		m_video_playing = true;
		m_gameSound.Play();
		movie.Play(true); //video loops sound doesnt
		return true;
	}
	return false;
}

void CMenu::_game(bool launch)
{
	m_banner_loaded = false;
	m_set_bg = false;
	bool coverFlipped = false;
	int cf_version = 1;
	string domain;
	string key;
	Vector3D v;
	Vector3D savedv;
	
	dir_discHdr *hdr = (dir_discHdr*)MEM2_alloc(sizeof(dir_discHdr));
	memcpy(hdr, CoverFlow.getHdr(), sizeof(dir_discHdr));
	_setCurrentItem(hdr);
	
	char id[74];
	char catID[64];
	memset(id, 0, 74);
	memset(catID, 0, 64);
	
	if(hdr->type == TYPE_HOMEBREW)
		wcstombs(id, hdr->title, 63);
	else if(hdr->type == TYPE_PLUGIN)
	{
		strncpy(m_plugin.PluginMagicWord, fmt("%08x", hdr->settings[0]), 8);
		if(strrchr(hdr->path, '/') != NULL)
			wcstombs(catID, hdr->title, 63);
		else
			strncpy(catID, hdr->path, 63);// scummvm
		strcpy(id, m_plugin.PluginMagicWord);
		strcat(id, fmt("/%s", catID));
	}
	else
	{
		strcpy(id, hdr->id);
		if(hdr->type == TYPE_GC_GAME && hdr->settings[0] == 1) /* disc 2 */
			strcat(id, "_2");
	}

	m_zoom_banner = m_cfg.getBool(_domainFromView(), "show_full_banner", false);

	if(m_banner.GetZoomSetting() != m_zoom_banner)
		m_banner.ToggleZoom();

	if(m_banner.GetInGameSettings())
		m_banner.ToggleGameSettings();
	m_gameSelected = true;
	s8 startGameSound = -7;
	SetupInput();

	while(!m_exit)
	{
		if(m_fa.isLoaded() && m_fa.isAnimationComplete())
		{
			if(m_fa.noLoop())
			{
				m_fa.unload();
				CoverFlow.showCover();
				_setMainBg();
			}
			else //loop fanart
				m_fa.reset();
		}
		if(startGameSound < 1)
			startGameSound++;

		if(startGameSound == -5)
			_showGame();// this also starts fanart with unloading previous fanart.
			
		if(!launch)	
			_mainLoopCommon(true);
			
		if(startGameSound == 0)
		{
			startGameSound = 1;
			_playGameSound();
		}
		/* move and zoom flipped cover */
		if(coverFlipped)
		{
			float step = 0.05f;
			if(BTN_PLUS_PRESSED || BTN_MINUS_PRESSED)
			{
				if(BTN_MINUS_PRESSED)
					step = -step;
				v.z = min(max(-15.f, v.z + step), 15.f);
			}
			else if(BTN_LEFT_PRESSED || BTN_RIGHT_PRESSED)
			{
				if(BTN_RIGHT_PRESSED)
					step = -step;
				v.x = min(max(-15.f, v.x + step), 15.f);
			}
			else if(BTN_UP_PRESSED || BTN_DOWN_PRESSED)
			{
				if(BTN_UP_PRESSED)
					step = -step;
				v.y = min(max(-15.f, v.y + step), 15.f);
			}
			CoverFlow.setCoverFlipPos(v);
		}
		/* exit game menu */
		if(BTN_HOME_PRESSED)
		{
			_cleanupBanner();// also cleans up trailer movie including trailer sound
			break;
		}
		else if(BTN_B_PRESSED)
		{
			/* de flip cover */
			if(coverFlipped)
			{
				CoverFlow.flip();
				coverFlipped = false;
				m_banner.SetShowBanner(true);
			}
			/* exit game selected menu */
			else
			{
				_cleanupBanner();
				break;
			}
		}
		/* display game info screen */
		else if(BTN_PLUS_PRESSED && hdr->type != TYPE_HOMEBREW && hdr->type != TYPE_SOURCE && !coverFlipped && !m_video_playing)
		{
			_hideGame();
			m_banner.SetShowBanner(false);
			_gameinfo();
			_showGame();
			m_banner.SetShowBanner(true);
		}
		/* play or stop a trailer video */
		else if(BTN_MINUS_PRESSED && !coverFlipped)
		{
			if(m_video_playing)
			{
				m_video_playing = false;
				movie.DeInit();
				m_gameSound.FreeMemory();
				m_banner.SetShowBanner(true);
				if(!m_gameSound.IsPlaying()) 
					startGameSound = -6;
			}
			else
				_startVideo();
		}
		/* switch coverflow layout */
		else if((BTN_1_PRESSED || BTN_2_PRESSED) && !CFLocked && !coverFlipped && !m_video_playing)
		{
			u32 curPos = CoverFlow._currentPos();
			s8 direction = BTN_1_PRESSED ? 1 : -1;
			int cfVersion = loopNum((_getCFVersion() - 1) + direction, m_numCFVersions) + 1;
			_setCFVersion(cfVersion);
			_loadCFLayout(cfVersion);
			CoverFlow._setCurPos(curPos);
			CoverFlow.applySettings();
		}
		else if(launch || BTN_A_PRESSED)
		{
			if(m_fa.isLoaded() && ShowPointer())// stop and unload fanart
			{
				m_fa.unload();
				CoverFlow.showCover();
				_setMainBg();
				continue;
			}
			/* delete button */
			else if(m_btnMgr.selected(m_gameBtnDelete) && hdr->type != TYPE_HOMEBREW)
			{
				_hideGame();
				m_banner.SetShowBanner(false);
				if(m_locked)
					error(_t("errgame15", L"WiiFlow locked! Unlock WiiFlow to use this feature."));
				else if(hdr->type == TYPE_CHANNEL)
					error(_t("errgame17", L"Can not delete real NAND Channels!"));
				else /* delete wii, gamecube, emunand game or plugin rom */
				{
					if(_wbfsOp(WO_REMOVE_GAME))
					{
						m_refreshGameList = false;
						_setCurrentItem(CoverFlow.getNextHdr());
						_cleanupBanner();
						_loadList();
						_initCF();
						CoverFlow.select();
						CoverFlow.applySettings();
						startGameSound = -10;
					}
				}
				_showGame();
				m_banner.SetShowBanner(true);
			}
			else if(m_btnMgr.selected(m_gameBtnSettings))
			{
				_hideGame();
				if(m_locked)
				{
					m_banner.SetShowBanner(false);
					error(_t("errgame15", L"WiiFlow locked! Unlock WiiFlow to use this feature."));
					m_banner.SetShowBanner(true);
				}
				else
				{
					// the mainloop handles drawing banner while in settings
					m_banner.ToggleZoom();//zoom to full
					m_banner.ToggleGameSettings();// dim brightness
					_gameSettings(hdr);
					m_banner.ToggleGameSettings();//reset brightness
					m_banner.ToggleZoom();//de zoom to small
				}
				_showGame();
			}
			else if(m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff))
			{
				if(hdr->type == TYPE_PLUGIN)
					m_gcfg1.setBool("FAVORITES_PLUGINS", id, !m_gcfg1.getBool("FAVORITES_PLUGINS", id, false));
				else
					m_gcfg1.setBool("FAVORITES", id, !m_gcfg1.getBool("FAVORITES", id, false));
				if(m_favorites)
					m_refreshGameList = true;
			}
			else if(m_btnMgr.selected(m_gameBtnCategories))
			{
				if(m_locked)
				{
					m_banner.SetShowBanner(false);
					error(_t("errgame15", L"WiiFlow locked! Unlock WiiFlow to use this feature."));
					m_banner.SetShowBanner(true);
				}
				else
				{
					_hideGame();
					// the mainloop handles drawing banner while in settings
					m_banner.ToggleZoom();//zoom to full
					m_banner.ToggleGameSettings();// dim brightness
					_CategorySettings(true);
					m_banner.ToggleGameSettings();//reset brightness
					m_banner.ToggleZoom();//de zoom to small
				}
				_showGame();
				if(m_newGame)
					startGameSound = -10;
			}
			else if(m_btnMgr.selected(m_gameBtnBack) || m_btnMgr.selected(m_gameBtnBackFull))
			{
				_cleanupBanner();
				break;
			}
			else if(m_btnMgr.selected(m_gameBtnToggle) || m_btnMgr.selected(m_gameBtnToggleFull))
			{
				m_zoom_banner = m_banner.ToggleZoom();
				m_cfg.setBool(_domainFromView(), "show_full_banner", m_zoom_banner);
				m_show_zone_game = false;
				m_btnMgr.hide(m_gameBtnPlayFull);
				m_btnMgr.hide(m_gameBtnBackFull);
				m_btnMgr.hide(m_gameBtnToggleFull);
			}
			else if(launch || m_btnMgr.selected(m_gameBtnPlay) || m_btnMgr.selected(m_gameBtnPlayFull) || 
					(!ShowPointer() && !m_video_playing && !coverFlipped))
			{
				_hideGame();
				if(isWiiVC && (hdr->type == TYPE_WII_GAME || hdr->type == TYPE_EMUCHANNEL))
				{
					error(_t("errgame19", L"Can't launch in Wii virtual console mode!"));
					launch = false;
					_showGame();
				}
				else
				{
					MusicPlayer.Stop();
					_cleanupBanner();
					m_cfg.setInt("GENERAL", "cat_startpage", m_catStartPage);
					m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
					/* change to current game's partition */
					switch(hdr->type)
					{
						case TYPE_CHANNEL:
						case TYPE_EMUCHANNEL:
							currentPartition = m_cfg.getInt(CHANNEL_DOMAIN, "partition", 1);
							break;
						case TYPE_HOMEBREW:
							currentPartition = m_cfg.getInt(HOMEBREW_DOMAIN, "partition", 1);
							break;
						case TYPE_GC_GAME:
							currentPartition = m_cfg.getInt(GC_DOMAIN, "partition", 1);
							break;
						case TYPE_WII_GAME:
							currentPartition = m_cfg.getInt(WII_DOMAIN, "partition", 1);
							break;
						default:
							int romsPartition = m_plugin.GetRomPartition(m_plugin.GetPluginPosition(hdr->settings[0]));
							if(romsPartition < 0)
								romsPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition", 0);
							currentPartition = romsPartition;
							break;
					}

					/* Get Banner Title for Playlog */
					if(hdr->type == TYPE_WII_GAME || hdr->type == TYPE_CHANNEL || hdr->type == TYPE_EMUCHANNEL)
					{
						NANDemuView = hdr->type == TYPE_EMUCHANNEL;
						CurrentBanner.ClearBanner();
						if(hdr->type == TYPE_CHANNEL || hdr->type == TYPE_EMUCHANNEL)
						{
							u64 chantitle = CoverFlow.getChanTitle();
							ChannelHandle.GetBanner(chantitle);
						}
						else if(hdr->type == TYPE_WII_GAME)
							_extractBnr(hdr);
						u8 banner_title[84];
						memset(banner_title, 0, 84);
						if(CurrentBanner.IsValid())
							CurrentBanner.GetName(banner_title, GetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str()));
						if(Playlog_Update(hdr->id, banner_title) < 0)
							Playlog_Delete();
						CurrentBanner.ClearBanner();
					}

					/* Finally boot it */
					gprintf("Launching game %s\n", hdr->id);
					if(hdr->type == TYPE_EMUCHANNEL)
						gprintf("from emu nand\n");
					else if(hdr->type == TYPE_CHANNEL)
						gprintf("from real nand\n");
					_launch(hdr);

					if(m_exit)
						break;

					_hideWaitMessage();
					launch = false;

					for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
						WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
					m_gcfg2.unload();
					_showGame();
				}
			}
			else if(!coverFlipped)
			{
				/* flip cover if mouse over */
				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
				{
					if(CoverFlow.mouseOver(m_cursor[chan].x(), m_cursor[chan].y()))
					{
						cf_version = _getCFVersion();
						domain = fmt("%s_%i_S", cf_domain, cf_version);
						key = "flip_pos";
						if(!m_vid.wide())
							key += "_4_3";
						v = m_coverflow.getVector3D(domain, key);
						coverFlipped = true;
						CoverFlow.flip();
						m_banner.SetShowBanner(false);
					}
				}
			}
		}
		if(!coverFlipped && !m_video_playing)
		{
			/* move to new cover if needed */
			if((startGameSound == 1 || startGameSound < -8) && (BTN_UP_REPEAT || RIGHT_STICK_UP))
			{
				CoverFlow.up();
				startGameSound = -10;
			}
			if((startGameSound == 1 || startGameSound < -8) && (BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT))
			{
				CoverFlow.right();
				startGameSound = -10;
			}
			if((startGameSound == 1 || startGameSound < -8) && (BTN_DOWN_REPEAT || RIGHT_STICK_DOWN))
			{
				CoverFlow.down();
				startGameSound = -10;
			}
			if((startGameSound == 1 || startGameSound < -8) && (BTN_LEFT_REPEAT || RIGHT_STICK_LEFT))
			{
				CoverFlow.left();
				startGameSound = -10;
			}
			if(startGameSound == -10)// if -10 then we moved to new cover
			{
				memcpy(hdr, CoverFlow.getHdr(), sizeof(dir_discHdr));// get new game header
				_setCurrentItem(hdr);
				
				memset(id, 0, 74);
				memset(catID, 0, 64);
	
				if(hdr->type == TYPE_HOMEBREW)
					wcstombs(id, hdr->title, 64);
				else if(hdr->type == TYPE_PLUGIN)
				{
					strncpy(m_plugin.PluginMagicWord, fmt("%08x", hdr->settings[0]), 8);
					if(strrchr(hdr->path, '/') != NULL)
						wcstombs(catID, hdr->title, 63);
					else
						strncpy(catID, hdr->path, 63);// scummvm
					strcpy(id, m_plugin.PluginMagicWord);
					strcat(id, fmt("/%s", catID));
				}
				else
				{
					strcpy(id, hdr->id);
					if(hdr->type == TYPE_GC_GAME && hdr->settings[0] == 1) /* disc 2 */
						strcat(id, "_2");
				}
				if(m_newGame)
				{
					m_newGame = false;
					startGameSound = 1;
					_playGameSound();
				}
			}
		}
		
		/* showing and hiding buttons based on banner zoomed state */
		if(!m_zoom_banner && !m_fa.isLoaded())
		{
			/* always hide full banner buttons */
			m_btnMgr.hide(m_gameBtnPlayFull);
			m_btnMgr.hide(m_gameBtnBackFull);
			m_btnMgr.hide(m_gameBtnToggleFull);
			
			if(m_show_zone_game && !coverFlipped && !m_video_playing)
			{
				m_btnMgr.show(m_gameBtnPlay);
				m_btnMgr.show(m_gameBtnBack);
				m_btnMgr.show(m_gameBtnSettings);
				m_btnMgr.show(m_gameBtnDelete);
				m_btnMgr.show(m_gameBtnCategories);
				bool b;
				if(hdr->type == TYPE_PLUGIN)
					b = m_gcfg1.getBool("FAVORITES_PLUGINS", id, false);
				else
					b = m_gcfg1.getBool("FAVORITES", id, false);
				m_btnMgr.show(b ? m_gameBtnFavoriteOn : m_gameBtnFavoriteOff);
				m_btnMgr.hide(b ? m_gameBtnFavoriteOff : m_gameBtnFavoriteOn);
				for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 2; ++i)// #4 is used for banner frame
					if(m_gameLblUser[i] != -1)
						m_btnMgr.show(m_gameLblUser[i]);
			}
			else
			{
				m_btnMgr.hide(m_gameBtnFavoriteOn);
				m_btnMgr.hide(m_gameBtnFavoriteOff);
				m_btnMgr.hide(m_gameBtnCategories);
				m_btnMgr.hide(m_gameBtnSettings);
				m_btnMgr.hide(m_gameBtnDelete);
				m_btnMgr.hide(m_gameBtnPlay);
				m_btnMgr.hide(m_gameBtnBack);
				for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 2; ++i)
					if (m_gameLblUser[i] != -1)
						m_btnMgr.hide(m_gameLblUser[i]);
			}
			
			/* show or hide small banner toggle btn and frame */
			if(m_banner_loaded && !m_soundThrdBusy && !coverFlipped && !m_video_playing && !m_fa.isLoaded())
			{
				/* show only if the game has a banner */
				m_btnMgr.show(m_gameBtnToggle);
				if(m_gameLblUser[4] != -1)
					m_btnMgr.show(m_gameLblUser[4]);
			}
			else
			{
				m_btnMgr.hide(m_gameBtnToggle);
				if(m_gameLblUser[4] != -1)
					m_btnMgr.hide(m_gameLblUser[4]);
			}
		}
		else if(!m_fa.isLoaded())// banner zoomed full screen
		{
			if(m_banner_loaded && !m_soundThrdBusy)// there is a banner
			{
				m_btnMgr.show(m_gameBtnPlayFull);
				m_btnMgr.show(m_gameBtnBackFull);
				m_btnMgr.show(m_gameBtnToggleFull);
				
				m_btnMgr.hide(m_gameBtnFavoriteOn);
				m_btnMgr.hide(m_gameBtnFavoriteOff);
				m_btnMgr.hide(m_gameBtnCategories);
				m_btnMgr.hide(m_gameBtnSettings);
				m_btnMgr.hide(m_gameBtnDelete);
				m_btnMgr.hide(m_gameBtnPlay);
				m_btnMgr.hide(m_gameBtnBack);
				
				m_btnMgr.hide(m_gameBtnToggle);
				for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
					if(m_gameLblUser[i] != -1)
						m_btnMgr.hide(m_gameLblUser[i]);
			}
			else // no banner for whatever reason
			{
				if(!m_soundThrdBusy)
				{
					m_btnMgr.hide(m_gameBtnPlayFull);
					m_btnMgr.hide(m_gameBtnBackFull);
					m_btnMgr.hide(m_gameBtnToggleFull);
				}
					
				if(m_show_zone_game && !m_soundThrdBusy)
				{
					m_btnMgr.show(m_gameBtnPlay);
					m_btnMgr.show(m_gameBtnBack);
					m_btnMgr.show(m_gameBtnSettings);
					m_btnMgr.show(m_gameBtnDelete);
					m_btnMgr.show(m_gameBtnCategories);
					bool b;
					if(hdr->type == TYPE_PLUGIN)
						b = m_gcfg1.getBool("FAVORITES_PLUGINS", id, false);
					else
						b = m_gcfg1.getBool("FAVORITES", id, false);
					m_btnMgr.show(b ? m_gameBtnFavoriteOn : m_gameBtnFavoriteOff);
					m_btnMgr.hide(b ? m_gameBtnFavoriteOff : m_gameBtnFavoriteOn);
					for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 2; ++i)// don't include small banner frame
						if(m_gameLblUser[i] != -1)
							m_btnMgr.show(m_gameLblUser[i]);
				}
				else
				{
					m_btnMgr.hide(m_gameBtnFavoriteOn);
					m_btnMgr.hide(m_gameBtnFavoriteOff);
					m_btnMgr.hide(m_gameBtnCategories);
					m_btnMgr.hide(m_gameBtnSettings);
					m_btnMgr.hide(m_gameBtnDelete);
					m_btnMgr.hide(m_gameBtnPlay);
					m_btnMgr.hide(m_gameBtnBack);
					
					m_btnMgr.hide(m_gameBtnToggle);
					for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 1; ++i)
						if (m_gameLblUser[i] != -1)
							m_btnMgr.hide(m_gameLblUser[i]);
				}
			}
		}
	}
	if(coverFlipped)
	{
		m_coverflow.setVector3D(domain, key, savedv);
		_loadCFLayout(cf_version, true);// true?
		CoverFlow.applySettings();
	}
	m_gameSelected = false;
	MEM2_free(hdr);
	_hideGame();
}

void CMenu::_initGameMenu()
{
	//CColor fontColor(0xD0BFDFFF);
	TexData texGameFavOn;
	TexData texGameFavOnSel;
	TexData texGameFavOff;
	TexData texGameFavOffSel;
	TexData texCategories;
	TexData texCategoriesSel;
	TexData texDelete;
	TexData texDeleteSel;
	TexData texSettings;
	TexData texSettingsSel;
	TexData texToggleBanner;
	TexData bgLQ;

	TexHandle.fromImageFile(texGameFavOn, fmt("%s/gamefavon.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texGameFavOnSel, fmt("%s/gamefavons.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texGameFavOff, fmt("%s/gamefavoff.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texGameFavOffSel, fmt("%s/gamefavoffs.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texCategories, fmt("%s/btncat.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texCategoriesSel, fmt("%s/btncats.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texDelete, fmt("%s/delete.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texDeleteSel, fmt("%s/deletes.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texSettings, fmt("%s/btnconfig.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texSettingsSel, fmt("%s/btnconfigs.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texToggleBanner, fmt("%s/blank.png", m_imgsDir.c_str()));

	_addUserLabels(m_gameLblUser, ARRAY_SIZE(m_gameLblUser), "GAME");
	m_gameBg = _texture("GAME/BG", "texture", theme.bg, false);
	if(m_theme.loaded() && TexHandle.fromImageFile(bgLQ, fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GAME/BG", "texture").c_str()), GX_TF_CMPR, 64, 64) == TE_OK)
		m_gameBgLQ = bgLQ;

	m_gameBtnPlay = _addButton("GAME/PLAY_BTN", theme.btnFont, L"", 420, 344, 200, 48, theme.btnFontColor);
	m_gameBtnBack = _addButton("GAME/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_gameBtnFavoriteOn = _addPicButton("GAME/FAVORITE_ON", texGameFavOn, texGameFavOnSel, 460, 200, 48, 48);
	m_gameBtnFavoriteOff = _addPicButton("GAME/FAVORITE_OFF", texGameFavOff, texGameFavOffSel, 460, 200, 48, 48);
	m_gameBtnCategories = _addPicButton("GAME/CATEGORIES_BTN", texCategories, texCategoriesSel, 532, 200, 48, 48);
	m_gameBtnSettings = _addPicButton("GAME/SETTINGS_BTN", texSettings, texSettingsSel, 460, 272, 48, 48);
	m_gameBtnDelete = _addPicButton("GAME/DELETE_BTN", texDelete, texDeleteSel, 532, 272, 48, 48);
	m_gameBtnBackFull = _addButton("GAME/BACK_FULL_BTN", theme.btnFont, L"", 100, 390, 200, 56, theme.btnFontColor);
	m_gameBtnPlayFull = _addButton("GAME/PLAY_FULL_BTN", theme.btnFont, L"", 340, 390, 200, 56, theme.btnFontColor);
	m_gameBtnToggle = _addPicButton("GAME/TOOGLE_BTN", texToggleBanner, texToggleBanner, 385, 31, 236, 127);
	m_gameBtnToggleFull = _addPicButton("GAME/TOOGLE_FULL_BTN", texToggleBanner, texToggleBanner, 20, 12, 608, 344);

	m_gameButtonsZone.x = m_theme.getInt("GAME/ZONES", "buttons_x", 380);
	m_gameButtonsZone.y = m_theme.getInt("GAME/ZONES", "buttons_y", 0);
	m_gameButtonsZone.w = m_theme.getInt("GAME/ZONES", "buttons_w", 640);
	m_gameButtonsZone.h = m_theme.getInt("GAME/ZONES", "buttons_h", 480);
	m_gameButtonsZone.hide = m_theme.getBool("GAME/ZONES", "buttons_hide", true);

	_setHideAnim(m_gameBtnPlay, "GAME/PLAY_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnBack, "GAME/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnFavoriteOn, "GAME/FAVORITE_ON", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnFavoriteOff, "GAME/FAVORITE_OFF", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnCategories, "GAME/CATEGORIES_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnSettings, "GAME/SETTINGS_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnDelete, "GAME/DELETE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnPlayFull, "GAME/PLAY_FULL_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnBackFull, "GAME/BACK_FULL_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnToggle, "GAME/TOOGLE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnToggleFull, "GAME/TOOGLE_FULL_BTN", 200, 0, 1.f, 0.f);
	_hideGame(true);
	_textGame();
}

void CMenu::_textGame(void)
{
	m_btnMgr.setText(m_gameBtnPlay, _t("gm1", L"Play"));
	m_btnMgr.setText(m_gameBtnBack, _t("gm2", L"Back"));
	m_btnMgr.setText(m_gameBtnPlayFull, _t("gm1", L"Play"));
	m_btnMgr.setText(m_gameBtnBackFull, _t("gm2", L"Back"));
}

struct IMD5Header
{
	u32 fcc;
	u32 filesize;
	u8 zeroes[8];
	u8 crypto[16];
} ATTRIBUTE_PACKED;

// loads game banner and sound to be played by mainloop
void * CMenu::_gameSoundThread(void *obj)
{
	CMenu *m = (CMenu*)obj;
	m->m_soundThrdBusy = true;
	m->m_gamesound_changed = false;
	CurrentBanner.ClearBanner();//clear current banner from memory

	u8 *custom_bnr_file = NULL;
	u32 custom_bnr_size = 0;
	char custom_banner[256];
	custom_banner[255] = '\0';

	u8 *cached_bnr_file = NULL;
	u32 cached_bnr_size = 0;
	char cached_banner[256];
	cached_banner[255] = '\0';
	
	/* plugin individual game sound */
	char game_sound[256];
	game_sound[255] = '\0';
	
	m_banner_loaded = false;
	const dir_discHdr *GameHdr = CoverFlow.getHdr();	
	if(GameHdr->type == TYPE_PLUGIN)
	{
		const char *coverDir  = NULL;
		coverDir = m_plugin.GetCoverFolderName(GameHdr->settings[0]);
		
		if(coverDir == NULL || strlen(coverDir) == 0)
		{
			strncpy(custom_banner, fmt("%s/%s.bnr", m->m_customBnrDir.c_str(), CoverFlow.getFilenameId(GameHdr)), 255);
			strncpy(game_sound, fmt("%s/gamesounds/%s", m->m_dataDir.c_str(), CoverFlow.getFilenameId(GameHdr)), 251);//save for .ext
		}
		else
		{
			strncpy(custom_banner, fmt("%s/%s/%s.bnr", m->m_customBnrDir.c_str(), coverDir, CoverFlow.getFilenameId(GameHdr)), 255);
			strncpy(game_sound, fmt("%s/gamesounds/%s/%s", m->m_dataDir.c_str(), coverDir, CoverFlow.getFilenameId(GameHdr)), 251);
		}
		
		/* get plugin rom custom banner */
		fsop_GetFileSizeBytes(custom_banner, &custom_bnr_size);
		if(custom_bnr_size > 0)
		{
			custom_bnr_file = (u8*)MEM2_lo_alloc(custom_bnr_size);
			if(custom_bnr_file != NULL)
			{
				fsop_ReadFileLoc(custom_banner, custom_bnr_size, (void*)custom_bnr_file);
				m_banner_loaded = true;
			}
		}
		else // if no banner get plugin rom gamesound or just the default plugin gamesound
		{
			m_banner.DeleteBanner();
			bool found = false;
			if(fsop_FileExist(fmt("%s.mp3", game_sound)))
			{
				strcat(game_sound, ".mp3");
				found = true;
			}
			else if(fsop_FileExist(fmt("%s.wav", game_sound)))
			{
				strcat(game_sound, ".wav");
				found = true;
			}
			else if(fsop_FileExist(fmt("%s.ogg", game_sound)))
			{
				strcat(game_sound, ".ogg");
				found = true;
			}
			if(found)
			{
				u32 size = 0;
				u8 *sf = fsop_ReadFile(game_sound, &size);
				m->m_gameSound.Load(sf, size);
			}
			else
				m->m_gameSound.Load(m_plugin.GetBannerSound(GameHdr->settings[0]), m_plugin.GetBannerSoundSize());
			if(m->m_gameSound.IsLoaded())
				m->m_gamesound_changed = true;
			m->m_soundThrdBusy = false;
			return NULL;
		}
	}
	else
	{
		/* check custom ID6 first */
		strncpy(custom_banner, fmt("%s/%s.bnr", m->m_customBnrDir.c_str(), GameHdr->id), 255);
		fsop_GetFileSizeBytes(custom_banner, &custom_bnr_size);
		if(custom_bnr_size > 0)
		{
			custom_bnr_file = (u8*)MEM2_lo_alloc(custom_bnr_size);
			if(custom_bnr_file != NULL)
			{
				fsop_ReadFileLoc(custom_banner, custom_bnr_size, (void*)custom_bnr_file);
				m_banner_loaded = true;
			}
		}
		else /* no custom ID6 or too big, try ID3 */
		{
			strncpy(custom_banner, fmt("%s/%.3s.bnr", m->m_customBnrDir.c_str(), GameHdr->id), 255);
			fsop_GetFileSizeBytes(custom_banner, &custom_bnr_size);
			if(custom_bnr_size > 0)
			{
				custom_bnr_file = (u8*)MEM2_lo_alloc(custom_bnr_size);
				if(custom_bnr_file != NULL)
				{
					fsop_ReadFileLoc(custom_banner, custom_bnr_size, (void*)custom_bnr_file);
					m_banner_loaded = true;
				}
			}
		}
	}
	if(GameHdr->type == TYPE_GC_GAME && custom_bnr_file == NULL)
	{
		//get the gc game's opening.bnr from ISO - a 96x32 image to add to the gc banner included with wiiflow
		GC_Disc_Reader.init(GameHdr->path);
		u8 *opening_bnr = GC_Disc_Reader.GetGameCubeBanner();
		if(opening_bnr != NULL)
		{
			//creategcbanner adds the opening.bnr image and game title to the wiiflow gc banner
			m_banner.CreateGCBanner(opening_bnr, m->m_wbf1_font, m->m_wbf2_font, GameHdr->title);
			m_banner_loaded = true;
		}
		else
			m_banner.DeleteBanner();
		GC_Disc_Reader.clear();
		if(m->m_gc_play_default_sound)
		{
			//get wiiflow gc ogg sound to play with banner
			m->m_gameSound.Load(gc_ogg, gc_ogg_size, false);
			if(m->m_gameSound.IsLoaded())
				m->m_gamesound_changed = true;
		}
		m->m_soundThrdBusy = false;
		return NULL;
	}
	if(custom_bnr_file == NULL)/* no custom and not GC game try cached banner id6 only*/
	{
		strncpy(cached_banner, fmt("%s/%s.bnr", m->m_bnrCacheDir.c_str(), GameHdr->id), 255);
		fsop_GetFileSizeBytes(cached_banner, &cached_bnr_size);
		if(cached_bnr_size > 0)
		{
			cached_bnr_file = (u8*)MEM2_lo_alloc(cached_bnr_size);
			if(cached_bnr_file != NULL)
			{
				fsop_ReadFileLoc(cached_banner, cached_bnr_size, (void*)cached_bnr_file);
				m_banner_loaded = true;
			}
		}
	}

	if(custom_bnr_file != NULL)
		CurrentBanner.SetBanner(custom_bnr_file, custom_bnr_size, true, true);
	else if(cached_bnr_file != NULL)
		CurrentBanner.SetBanner(cached_bnr_file, cached_bnr_size, false, true);
	else if(GameHdr->type == TYPE_WII_GAME)
	{
		m->_extractBnr(GameHdr);
		m_banner_loaded = true;
	}
	else if(GameHdr->type == TYPE_CHANNEL || GameHdr->type == TYPE_EMUCHANNEL)
	{
		ChannelHandle.GetBanner(TITLE_ID(GameHdr->settings[0], GameHdr->settings[1]));
		m_banner_loaded = true;
	}
		
	if(!CurrentBanner.IsValid())
	{
		m_banner_loaded = false;
		m->m_gameSound.FreeMemory();
		m_banner.DeleteBanner();
		CurrentBanner.ClearBanner();
		m->m_soundThrdBusy = false;
		return NULL;
	}
	//save new wii or channel banner to cache folder, gc and custom banners are not cached
	if(cached_bnr_file == NULL && custom_bnr_file == NULL)
		fsop_WriteFile(cached_banner, CurrentBanner.GetBannerFile(), CurrentBanner.GetBannerFileSize());

	//load and init banner
	m_banner.LoadBanner(m->m_wbf1_font, m->m_wbf2_font);
	
	//get sound from wii, channel, or custom banner and load it to play with the banner
	u32 sndSize = 0;
	u8 *soundBin = CurrentBanner.GetFile("sound.bin", &sndSize);
	CurrentBanner.ClearBanner();// got sound.bin and banner for displaying is loaded so no longer need current banner.

	if(soundBin != NULL && (GameHdr->type != TYPE_GC_GAME || m->m_gc_play_banner_sound))
	{
		if(memcmp(&((IMD5Header *)soundBin)->fcc, "IMD5", 4) == 0)
		{
			u32 newSize = 0;
			u8 *newSound = DecompressCopy(soundBin, sndSize, &newSize);
			free(soundBin);// no longer needed, now using decompressed newSound
			if(newSound == NULL || newSize == 0 || !m->m_gameSound.Load(newSound, newSize))
			{
				m->m_gameSound.FreeMemory();// frees newSound
				m_banner.DeleteBanner();// the same as UnloadBanner
				m->m_soundThrdBusy = false;
				return NULL;
			}
		}
		else
			m->m_gameSound.Load(soundBin, sndSize);

		if(m->m_gameSound.IsLoaded())
			m->m_gamesound_changed = true;
		else
		{
			m->m_gameSound.FreeMemory();// frees soundBin
			m_banner.DeleteBanner();
		}
	}
	else
	{
		if(soundBin != NULL)
			free(soundBin);
		//gprintf("WARNING: No sound found in banner!\n");
		m->m_gamesound_changed = true;
		m->m_gameSound.FreeMemory();// frees previous game sound
	}
	m->m_soundThrdBusy = false;
	return NULL;
}

u8 *GameSoundStack = NULL;
u32 GameSoundSize = 0x10000; //64kb
void CMenu::_playGameSound(void)// starts banner and gamesound loading thread
{
	_cleanupBanner(true);
	m_gamesound_changed = false;
	if(m_bnrSndVol == 0) 
		return;

	if(m_gameSoundThread != LWP_THREAD_NULL)
		_stopGameSoundThread();
	GameSoundStack = (u8*)MEM2_lo_alloc(GameSoundSize);
	LWP_CreateThread(&m_gameSoundThread, _gameSoundThread, this, GameSoundStack, GameSoundSize, 60);
}

void CMenu::_stopGameSoundThread()//stops banner and gamesound loading thread
{
	if(m_gameSoundThread == LWP_THREAD_NULL)
		return;

	if(LWP_ThreadIsSuspended(m_gameSoundThread))
		LWP_ResumeThread(m_gameSoundThread);

	while(m_soundThrdBusy)
		usleep(500);

	LWP_JoinThread(m_gameSoundThread, NULL);
	m_gameSoundThread = LWP_THREAD_NULL;

	if(GameSoundStack)
		MEM2_lo_free(GameSoundStack);
	GameSoundStack = NULL;
}
