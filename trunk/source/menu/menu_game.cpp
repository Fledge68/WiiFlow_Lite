
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <unistd.h>
#include <time.h>
#include <network.h>
#include <errno.h>

#include "menu.hpp"
#include "types.h"
#include "banner/BannerWindow.hpp"
#include "booter/external_booter.hpp"
#include "channel/channel_launcher.h"
#include "channel/channels.h"
#include "channel/nand.hpp"
#include "channel/identify.h"
#include "devicemounter/DeviceHandler.hpp"
#include "devicemounter/sdhc.h"
#include "devicemounter/usbstorage.h"
#include "gc/gc.hpp"
#include "gc/gcdisc.hpp"
#include "gui/WiiMovie.hpp"
#include "gui/GameTDB.hpp"
#include "homebrew/homebrew.h"
#include "loader/alt_ios.h"
#include "loader/wdvd.h"
#include "loader/alt_ios.h"
#include "loader/playlog.h"
#include "loader/wbfs.h"
#include "loader/wip.h"
#include "loader/frag.h"
#include "loader/fst.h"
#include "loader/cios.h"
#include "loader/nk.h"
#include "memory/memory.h"
#include "network/http.h"
#include "network/gcard.h"

//sounds
extern const u8 gc_ogg[];
extern const u32 gc_ogg_size;

bool m_zoom_banner = false;
s16 m_gameBtnPlayFull;
s16 m_gameBtnBackFull;
s16 m_gameBtnToogle;
s16 m_gameBtnToogleFull;

const CMenu::SOption CMenu::_languages[11] = {
	{ "lngdef", L"Default" },
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

const CMenu::SOption CMenu::_GlobalGCvideoModes[8] = {
	{ "DMLdefG", L"Game" },
	{ "DMLpal", L"PAL 576i" },
	{ "DMLntsc", L"NTSC 480i" },
	{ "DMLpal60", L"PAL 480i" },
	{ "DMLprog", L"NTSC 480p" },
	{ "DMLprogP", L"PAL 480p" },
	{ "DMLmpal", L"MPAL" },
	{ "DMLmpalP", L"MPAL-P" }
};

const CMenu::SOption CMenu::_GCvideoModes[9] = {
	{ "DMLdef", L"Default" },
	{ "DMLdefG", L"Game" },
	{ "DMLpal", L"PAL 576i" },
	{ "DMLntsc", L"NTSC 480i" },
	{ "DMLpal60", L"PAL 480i" },
	{ "DMLprog", L"NTSC 480p" },
	{ "DMLprogP", L"PAL 480p" },
	{ "DMLmpal", L"MPAL" },
	{ "DMLmpalP", L"MPAL-P" }
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

const CMenu::SOption CMenu::_NandEmu[2] = {
	{ "NANDpart", L"Partial" },
	{ "NANDfull", L"Full" },
};

const CMenu::SOption CMenu::_GlobalSaveEmu[4] = {
	{ "SaveOffG", L"Off" },
	{ "SavePartG", L"Game save" },
	{ "SaveRegG", L"Regionswitch" },
	{ "SaveFullG", L"Full" },
};

const CMenu::SOption CMenu::_SaveEmu[5] = {
	{ "SaveDef", L"Default" },
	{ "SaveOff", L"Off" },
	{ "SavePart", L"Game save" },
	{ "SaveReg", L"Regionswitch" },
	{ "SaveFull", L"Full" },
};

const CMenu::SOption CMenu::_AspectRatio[3] = {
	{ "aspectDef", L"Default" },
	{ "aspect43", L"Force 4:3" },
	{ "aspect169", L"Force 16:9" },
};

const CMenu::SOption CMenu::_NMM[4] = {
	{ "NMMDef", L"Default" },
	{ "NMMOff", L"Disabled" },
	{ "NMMon", L"Enabled" },
	{ "NMMdebug", L"Debug" },
};

const CMenu::SOption CMenu::_NinEmuCard[5] = {
	{ "NMMDef", L"Default" },
	{ "NMMOff", L"Disabled" },
	{ "NMMon", L"Enabled" },
	{ "NMMMulti", L"Multi Saves" },
	{ "NMMdebug", L"Debug" },
};

const CMenu::SOption CMenu::_NoDVD[3] = {
	{ "NoDVDDef", L"Default" },
	{ "NoDVDOff", L"Disabled" },
	{ "NoDVDon", L"Enabled" },
};

const CMenu::SOption CMenu::_GlobalGCLoaders[3] = {
	{ "GC_Auto", L"Auto MIOS" },
	{ "GC_Devo", L"Devolution" },
	{ "GC_Nindnt", L"Nintendont" },
};

const CMenu::SOption CMenu::_GCLoader[4] = {
	{ "GC_Def", L"Default" },
	{ "GC_Auto", L"Auto MIOS" },
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

/*
0 No Hook
1 VBI
2 KPAD read
3 Joypad Hook
4 GXDraw Hook
5 GXFlush Hook
6 OSSleepThread Hook
7 AXNextFrame Hook
*/

map<u8, u8> CMenu::_installed_cios;
u8 banner_title[84];

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}


static void _extractBannerTitle(int language)
{
	memset(banner_title, 0, 84);
	CurrentBanner.GetName(banner_title, language);
}

static void _extractChannelBnr(const u64 chantitle)
{
	ChannelHandle.GetBanner(chantitle);
}

static void _extractBnr(const dir_discHdr *hdr)
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

static u8 GetRequestedGameIOS(dir_discHdr *hdr)
{
	u8 IOS = 0;

	DeviceHandle.OpenWBFS(currentPartition);
	wbfs_disc_t *disc = WBFS_OpenDisc((u8*)&hdr->id, hdr->path);
	if(disc != NULL)
	{
		void *titleTMD = NULL;
		u32 tmd_size = wbfs_extract_file(disc, (char*)"TMD", &titleTMD);
		if(titleTMD != NULL)
		{
			if(tmd_size > 0x18B)
				IOS = *((u8*)titleTMD + 0x18B);
			MEM2_free(titleTMD);
		}
		WBFS_CloseDisc(disc);
	}
	WBFS_Close();
	return IOS;
}

void CMenu::_hideGame(bool instant)
{
	m_gameSelected = false;
	_cleanupVideo();
	m_fa.unload();
	CoverFlow.showCover();
	m_btnMgr.hide(m_gameBtnPlay, instant);
	m_btnMgr.hide(m_gameBtnBack, instant);
	m_btnMgr.hide(m_gameBtnPlayFull, instant);
	m_btnMgr.hide(m_gameBtnBackFull, instant);
	m_btnMgr.hide(m_gameBtnDelete, instant);
	m_btnMgr.hide(m_gameBtnSettings, instant);
	m_btnMgr.hide(m_gameBtnToogle, instant);
	m_btnMgr.hide(m_gameBtnToogleFull, instant);

	m_btnMgr.hide(m_gameBtnFavoriteOn, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOff, instant);
	m_btnMgr.hide(m_gameBtnAdultOn, instant);
	m_btnMgr.hide(m_gameBtnAdultOff, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if(m_gameLblUser[i] != -1)
			m_btnMgr.hide(m_gameLblUser[i], instant);
}

void CMenu::_showGame(void)
{
	CoverFlow.showCover();

	if(m_fa.load(m_cfg, m_fanartDir.c_str(), CoverFlow.getId()))
	{
		const TexData *bg = NULL;
		const TexData *bglq = NULL;
		m_fa.getBackground(bg, bglq);
		if(bg != NULL && bglq != NULL)
			_setBg(*bg, *bglq);
		if (m_fa.hideCover())
			CoverFlow.hideCover();
	}
	else
		_setBg(m_gameBg, m_gameBgLQ);

	if(!m_zoom_banner)
	{
		for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 1; ++i)
		{
			if(m_gameLblUser[i] != -1)
				m_btnMgr.show(m_gameLblUser[i]);
		}
		m_btnMgr.show(m_gameBtnPlay);
		m_btnMgr.show(m_gameBtnBack);
		m_btnMgr.show(m_gameBtnToogle);
	}
	else
	{
		m_btnMgr.show(m_gameBtnPlayFull);
		m_btnMgr.show(m_gameBtnBackFull);
		m_btnMgr.show(m_gameBtnToogleFull);
	}
}

static void setLanguage(int l)
{
	if (l > 0 && l <= 10)
		configbytes[0] = l - 1;
	else
		configbytes[0] = 0xCD;
}

void CMenu::_cleanupBanner(bool gamechange)
{
	//banner
	m_gameSound.FreeMemory();
	CheckGameSoundThread();
	m_banner.DeleteBanner(gamechange);
	//movie
	_cleanupVideo();
	//fanart
	m_fa.unload();
}

void CMenu::_cleanupVideo()
{
	m_video_playing = false;
	movie.DeInit();
}

static const char *getVideoPath(const string &videoDir, const char *videoId)
{
	const char *coverDir = m_plugin.GetCoverFolderName(CoverFlow.getHdr()->settings[0]);
	const char *videoPath = NULL;
	if(coverDir == NULL || strlen(coverDir) == 0)
		videoPath = fmt("%s/%s", videoDir.c_str(), videoId);
	else
		videoPath = fmt("%s/%s/%s", videoDir.c_str(), coverDir, videoId);
	return videoPath;
}

static const char *getVideoDefaultPath(const string &videoDir)
{
	char PluginMagicWord[9];
	memset(PluginMagicWord, 0, sizeof(PluginMagicWord));
	strncpy(PluginMagicWord, fmt("%08x", CoverFlow.getHdr()->settings[0]), 8);
	const char *videoPath = fmt("%s/%s", videoDir.c_str(), PluginMagicWord);
	return videoPath;
}

bool CMenu::_startVideo()
{
	char curId3[4];
	memset(curId3, 0, 4);
	const char *videoId = CoverFlow.getPathId(CoverFlow.getHdr());
	if(!NoGameID(CoverFlow.getHdr()->type))
	{	//id3
		memcpy(curId3, CoverFlow.getId(), 3);
		videoId = curId3;
	}
	const char *videoPath = getVideoPath(m_videoDir, videoId);
	const char *THP_Path = fmt("%s.thp", videoPath);
	if(!fsop_FileExist(THP_Path))
	{
		if(m_current_view == COVERFLOW_PLUGIN)
		{
			videoPath = getVideoDefaultPath(m_videoDir);
			THP_Path = fmt("%s.thp", videoPath);
		}
		else if(!NoGameID(CoverFlow.getHdr()->type))
		{
			videoPath = getVideoPath(m_videoDir, CoverFlow.getId());
			THP_Path = fmt("%s.thp", videoPath);
		}
	}
	if(fsop_FileExist(THP_Path))
	{
		m_gameSound.FreeMemory();
		CheckGameSoundThread();
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
	m_gcfg1.load(fmt("%s/" GAME_SETTINGS1_FILENAME, m_settingsDir.c_str()));
	m_zoom_banner = m_cfg.getBool(_domainFromView(), "show_full_banner", false);
	if(NoGameID(CoverFlow.getHdr()->type))
	{
		bool video_available = (m_current_view == COVERFLOW_PLUGIN && fsop_FileExist(fmt("%s.thp", getVideoDefaultPath(m_videoDir)))) ||
								fsop_FileExist(fmt("%s.thp", getVideoPath(m_videoDir, CoverFlow.getPathId(CoverFlow.getHdr()))));
		m_zoom_banner = m_zoom_banner && video_available;
		m_cfg.setBool(_domainFromView(), "show_full_banner", m_zoom_banner);
	}
	currentMoviePos = (m_zoom_banner ? zoomedMoviePos : normalMoviePos);
	if(m_banner.GetZoomSetting() != m_zoom_banner)
		m_banner.ToogleZoom();

	if(!launch)
	{
		SetupInput();
		_showGame();
		m_gameSelected = true;
		_playGameSound();
	}

	s8 startGameSound = 1;
	while(!m_exit)
	{
		if(startGameSound < 1)
			startGameSound++;

		if(startGameSound == -5)
			_showGame();
		_mainLoopCommon(true);

		if(startGameSound == 0)
		{
			m_gameSelected = true;
			startGameSound = 1;
			_playGameSound();
		}
		if(BTN_B_PRESSED && !m_locked && (m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff)))
		{
			_hideGame();
			m_banner.SetShowBanner(false);
			_CategorySettings(true);
			_showGame();
			m_banner.SetShowBanner(true);
			if(!m_gameSound.IsPlaying()) 
				startGameSound = -6;
			continue;
		}
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			_cleanupBanner();
			break;
		}
		else if(BTN_PLUS_PRESSED && m_GameTDBLoaded && (CoverFlow.getHdr()->type == TYPE_WII_GAME || CoverFlow.getHdr()->type == TYPE_GC_GAME || CoverFlow.getHdr()->type == TYPE_CHANNEL))
		{
			_hideGame();
			m_banner.SetShowBanner(false);
			m_gameSelected = true;
			_gameinfo();
			_showGame();
			m_banner.SetShowBanner(true);
			if(!m_gameSound.IsPlaying())
				startGameSound = -6;
		}
		else if(BTN_MINUS_PRESSED)
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
		else if((BTN_1_PRESSED) || (BTN_2_PRESSED))
		{
			s8 direction = BTN_1_PRESSED ? 1 : -1;
			const char *domain = _domainFromView();
			int cfVersion = 1+loopNum((m_cfg.getInt(domain, "last_cf_mode", 1)-1) + direction, m_numCFVersions);
			_loadCFLayout(cfVersion);
			CoverFlow.applySettings();
			m_cfg.setInt(domain, "last_cf_mode" , cfVersion);
		}
		else if(launch || BTN_A_PRESSED)
		{
			if (m_btnMgr.selected(m_mainBtnQuit))
				break;
			else if(m_btnMgr.selected(m_gameBtnDelete))
			{
				if(!m_locked)
				{
					_hideGame();
					m_banner.SetShowBanner(false);
					if(_wbfsOp(WO_REMOVE_GAME))
					{
						_cleanupBanner();
						break;
					}
					m_banner.SetShowBanner(true);
					if(!m_gameSound.IsPlaying())
						startGameSound = -6;
					_showGame();
				}
			}
			else if(m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff))
				m_gcfg1.setBool("FAVORITES", _getId(), !m_gcfg1.getBool("FAVORITES", _getId(), false));
			else if(m_btnMgr.selected(m_gameBtnAdultOn) || m_btnMgr.selected(m_gameBtnAdultOff))
				m_gcfg1.setBool("ADULTONLY", _getId(), !m_gcfg1.getBool("ADULTONLY", _getId(), false));
			else if(m_btnMgr.selected(m_gameBtnBack) || m_btnMgr.selected(m_gameBtnBackFull))
			{
				_cleanupBanner();
				break;
			}
			else if((m_btnMgr.selected(m_gameBtnToogle) || m_btnMgr.selected(m_gameBtnToogleFull)) 
					&& (!NoGameID(CoverFlow.getHdr()->type) || m_video_playing))
			{
				m_zoom_banner = m_banner.ToogleZoom();
				m_cfg.setBool(_domainFromView(), "show_full_banner", m_zoom_banner);
				currentMoviePos = (m_zoom_banner ? zoomedMoviePos : normalMoviePos);
				m_show_zone_game = false;
				m_btnMgr.hide(m_gameBtnPlayFull);
				m_btnMgr.hide(m_gameBtnBackFull);
				m_btnMgr.hide(m_gameBtnToogleFull);
			}
			else if(m_btnMgr.selected(m_gameBtnSettings))
			{
				_hideGame();
				m_gameSelected = true;

				m_banner.ToogleGameSettings();
				_gameSettings();
				m_banner.ToogleGameSettings();

				_showGame();
				if(!m_gameSound.IsPlaying()) 
					startGameSound = -6;
			}
			else if(launch || m_btnMgr.selected(m_gameBtnPlay) || m_btnMgr.selected(m_gameBtnPlayFull) || !ShowPointer())
			{
				_hideGame();
				MusicPlayer.Stop();
				_cleanupBanner();
				dir_discHdr *hdr = (dir_discHdr*)MEM2_alloc(sizeof(dir_discHdr));
				memcpy(hdr, CoverFlow.getHdr(), sizeof(dir_discHdr));
				m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
				// change to current games partition and set last_view for recall later
				m_cfg.setInt("GENERAL", "last_view", m_current_view);
				m_cfg.setInt("GENERAL", "cat_startpage", m_catStartPage);
				switch(hdr->type)
				{
					case TYPE_CHANNEL:
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
						m_plugin.GetEnabledPlugins(m_cfg, &enabledPluginsCount);
						if(enabledPluginsCount == 1)
						{
							char PluginMagicWord[9];
							memset(PluginMagicWord, 0, sizeof(PluginMagicWord));
							strncpy(PluginMagicWord, fmt("%08x", hdr->settings[0]), 8);
							currentPartition = m_cfg.getInt("PLUGINS_PARTITION", PluginMagicWord, 1);
							m_cfg.setInt(PLUGIN_DOMAIN, "partition", currentPartition);
						}
						currentPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition", 1);
						break;
				}
				/* Get Banner Title for Playlog */
				CurrentBanner.ClearBanner();
				if(hdr->type == TYPE_CHANNEL)
				{
					u64 chantitle = CoverFlow.getChanTitle();
					_extractChannelBnr(chantitle);
				}
				else if(hdr->type == TYPE_WII_GAME)
					_extractBnr(hdr);
				if(CurrentBanner.IsValid())
					_extractBannerTitle(GetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str()));
				if(hdr->type != TYPE_HOMEBREW && hdr->type != TYPE_PLUGIN)
				{
					if(Playlog_Update(hdr->id, banner_title) < 0)
						Playlog_Delete();
				}
				gprintf("Launching game %s\n", hdr->id);
				CurrentBanner.ClearBanner();

				/* Finally boot it */
				_launch(hdr);

				if(m_exit)
					break;

				_hideWaitMessage();
				launch = false;

				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
				m_gcfg2.unload();
				_showGame();
				//_initCF();
				//CoverFlow.select();
			}
			else 
			{
				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					if (CoverFlow.mouseOver(m_cursor[chan].x(), m_cursor[chan].y()))
						CoverFlow.flip();
			}
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_UP_REPEAT || RIGHT_STICK_UP))
		{
			_cleanupBanner(true);
			CoverFlow.up();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT))
		{
			_cleanupBanner(true);
			CoverFlow.right();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_DOWN_REPEAT || RIGHT_STICK_DOWN))
		{
			_cleanupBanner(true);
			CoverFlow.down();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_LEFT_REPEAT || RIGHT_STICK_LEFT))
		{
			_cleanupBanner(true);
			CoverFlow.left();
			startGameSound = -10;
		}
		if(startGameSound == -10)
		{
			m_gameSelected = false;
			_setBg(m_gameBg, m_gameBgLQ);
		}
		if(m_show_zone_game && !m_zoom_banner)
		{
			bool b = m_gcfg1.getBool("FAVORITES", _getId(), false);
			m_btnMgr.show(b ? m_gameBtnFavoriteOn : m_gameBtnFavoriteOff);
			m_btnMgr.hide(b ? m_gameBtnFavoriteOff : m_gameBtnFavoriteOn);
			m_btnMgr.show(m_gameBtnPlay);
			m_btnMgr.show(m_gameBtnBack);
			if(!m_fa.isLoaded())
				m_btnMgr.show(m_gameBtnToogle);
			else
				m_btnMgr.hide(m_gameBtnToogle);
			m_btnMgr.hide(m_gameBtnPlayFull);
			m_btnMgr.hide(m_gameBtnBackFull);
			m_btnMgr.hide(m_gameBtnToogleFull);
			if(m_gameLblUser[4] != -1 && !NoGameID(CoverFlow.getHdr()->type) && !m_fa.isLoaded())
				m_btnMgr.show(m_gameLblUser[4]);
			else
				m_btnMgr.hide(m_gameLblUser[4]);
			for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 1; ++i)
			{
				if(m_gameLblUser[i] != -1)
					m_btnMgr.show(m_gameLblUser[i]);
			}
			if(!m_locked)
			{
				b = m_gcfg1.getBool("ADULTONLY", _getId(), false);
				m_btnMgr.show(b ? m_gameBtnAdultOn : m_gameBtnAdultOff);
				m_btnMgr.hide(b ? m_gameBtnAdultOff : m_gameBtnAdultOn);
				m_btnMgr.show(m_gameBtnSettings);
			}
			if((CoverFlow.getHdr()->type != TYPE_HOMEBREW && (CoverFlow.getHdr()->type != TYPE_CHANNEL || 
				(!m_cfg.getBool(CHANNEL_DOMAIN, "disable", true) && CoverFlow.getHdr()->type == TYPE_CHANNEL))) && !m_locked)
				m_btnMgr.show(m_gameBtnDelete);
		}
		else
		{
			if(m_zoom_banner && !m_fa.isLoaded())
			{
				m_btnMgr.show(m_gameBtnPlayFull);
				m_btnMgr.show(m_gameBtnBackFull);
				m_btnMgr.show(m_gameBtnToogleFull);
			}
			m_btnMgr.hide(m_gameBtnFavoriteOn);
			m_btnMgr.hide(m_gameBtnFavoriteOff);
			m_btnMgr.hide(m_gameBtnAdultOn);
			m_btnMgr.hide(m_gameBtnAdultOff);
			m_btnMgr.hide(m_gameBtnSettings);
			m_btnMgr.hide(m_gameBtnDelete);
			m_btnMgr.hide(m_gameBtnPlay);
			m_btnMgr.hide(m_gameBtnBack);
			m_btnMgr.hide(m_gameBtnToogle);
			if(m_gameLblUser[4] != -1)
			{
				if(!NoGameID(CoverFlow.getHdr()->type) && !m_zoom_banner && !m_fa.isLoaded())
					m_btnMgr.show(m_gameLblUser[4]);
				else
					m_btnMgr.hide(m_gameLblUser[4], true);
			}
			for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 1; ++i)
				if (m_gameLblUser[i] != -1)
					m_btnMgr.hide(m_gameLblUser[i]);
		}
	}
	m_gcfg1.save(true);
	_hideGame();
}

void CMenu::directlaunch(const char *GameID)
{
	m_directLaunch = true;
	for(currentPartition = SD; currentPartition < USB8; currentPartition++)
	{
		if(!DeviceHandle.IsInserted(currentPartition))
			continue;
		DeviceHandle.OpenWBFS(currentPartition);
		string gameDir(fmt(wii_games_dir, DeviceName[currentPartition]));
		string cacheDir(fmt("%s/%s_wii.db", m_listCacheDir.c_str(), DeviceName[currentPartition]));
		m_gameList.CreateList(COVERFLOW_WII, currentPartition, gameDir,
				stringToVector(".wbfs|.iso", '|'), cacheDir, false);
		WBFS_Close();
		for(u32 i = 0; i < m_gameList.size(); i++)
		{
			if(strncasecmp(GameID, m_gameList[i].id, 6) == 0)
			{
				_launchGame(&m_gameList[i], false); // Launch will exit wiiflow
				break;
			}
		}
	}
	error(sfmt("errgame1", L"Cannot find the game with ID: %s", GameID));
}

void CMenu::_launchShutdown()
{
	CoverFlow.clear();
	_showWaitMessage();
	exitHandler(PRIILOADER_DEF); //Making wiiflow ready to boot something
}

void CMenu::_launch(const dir_discHdr *hdr)
{
	dir_discHdr launchHdr;
	memcpy(&launchHdr, hdr, sizeof(dir_discHdr));
	/* Lets boot that shit */
	if(launchHdr.type == TYPE_WII_GAME)
		_launchGame(&launchHdr, false);
	else if(launchHdr.type == TYPE_GC_GAME)
		_launchGC(&launchHdr, false);
	else if(launchHdr.type == TYPE_CHANNEL)
		_launchChannel(&launchHdr);
	else if(launchHdr.type == TYPE_PLUGIN)
	{
		//get dol name and length
		const char *plugin_dol_name = m_plugin.GetDolName(launchHdr.settings[0]);
		u8 plugin_dol_len = strlen(plugin_dol_name);
		//get title and path from hdr
		char title[101];
		memset(&title, 0, sizeof(title));
		u32 title_len_no_ext = 0;
		const char *path = NULL;
		//example rom path - sd:/roms/super mario bros.zip
		//example scummvm path - games/monkey island.exe
		if(strchr(launchHdr.path, ':') != NULL)//it's a rom path
		{
			//check if music player plugin, if so launch wiiflow's music player
			if(plugin_dol_len == strlen(MUSIC_DOMAIN) && strcmp(plugin_dol_name, MUSIC_DOMAIN) == 0)
			{
				MusicPlayer.LoadFile(launchHdr.path, false);
				m_exit = false;
				return;
			}
			//get rom title after last '/'
			strncpy(title, strrchr(launchHdr.path, '/') + 1, 100);
			if(strchr(launchHdr.path, '.') != NULL) // if there's extension get length of title without extension
				title_len_no_ext = strlen(title) - strlen(strrchr(title, '.'));
			//get path
			*strrchr(launchHdr.path, '/') = '\0'; //cut title off end of path
			path = strchr(launchHdr.path, '/') + 1; //cut sd:/ off of path
		}
		else //it's a scummvm game
		{
			path = launchHdr.path;
			wcstombs(title, launchHdr.title, 63);
		}
		m_cfg.setString(PLUGIN_DOMAIN, "current_item", title);
		// check if quadforce plugin if so then set proper quadforce mios
		const char *mios_wad = NULL;
		u32 magic = strtoul("444d4c62", NULL, 16);
		if(m_plugin.GetEnableStatus(m_cfg, magic))
		{
			if(currentPartition == SD && (m_mios_ver != 2 || m_sd_dm == false))
				mios_wad = fmt("%s/qfsd.wad", m_miosDir.c_str());
			else if(currentPartition != SD && (m_mios_ver != 2 || m_sd_dm == true))
				mios_wad = fmt("%s/qfusb.wad", m_miosDir.c_str());
			if(mios_wad != NULL && fsop_FileExist(mios_wad))
				_Wad(mios_wad, true);//install mios
		}
		const char *device = (currentPartition == 0 ? "sd" : (DeviceHandle.GetFSType(currentPartition) == PART_FS_NTFS ? "ntfs" : "usb"));
		/* this is weird - he sets the return to loader to be dev:/wiiflow/plugins/WiiFlowLoader.dol
		but there is no such dol in the plugins directory and WiiFlowLoader.dol is really a hidden channel
		on the system menu. This is set because some plugins require a arg for loader dol and even though
		this isn't real it fills the argument needed. */
		const char *loader = fmt("%s:/%s/WiiFlowLoader.dol", device, strchr(m_pluginsDir.c_str(), '/') + 1);

		vector<string> arguments = m_plugin.CreateArgs(device, path, title, loader, title_len_no_ext, launchHdr.settings[0]);
		const char *plugin_file = plugin_dol_name; /* try full path */
		if(strchr(plugin_file, ':') == NULL || !fsop_FileExist(plugin_file)) /* try universal plugin folder */
		{
			plugin_file = fmt("%s/%s", m_pluginsDir.c_str(), plugin_dol_name);
			if(!fsop_FileExist(plugin_file)) /* try device search */
			{
				for(u8 i = SD; i < MAXDEVICES; ++i)
				{
					plugin_file = fmt("%s:/%s", DeviceName[i], plugin_dol_name);
					if(fsop_FileExist(plugin_file))
						break;
				}
			}
		}
		_launchHomebrew(plugin_file, arguments);
	}
	else if(launchHdr.type == TYPE_HOMEBREW)
	{
		const char *gamepath = fmt("%s/boot.dol", launchHdr.path);
		if(!fsop_FileExist(gamepath))
			gamepath = fmt("%s/boot.elf", launchHdr.path);
		if(fsop_FileExist(gamepath))
		{
			m_cfg.setString(HOMEBREW_DOMAIN, "current_item", strrchr(launchHdr.path, '/') + 1);
			_launchHomebrew(gamepath, m_homebrewArgs);
			/*m_homebrewArgs is basically an empty vector string not needed for homebrew
			but used by plugins when _launchHomebrew is called */
		}
	}
	ShutdownBeforeExit();
	Sys_Exit();
}

// taken from Postloader by Stfour
#define QFIDN 9
static const char qfid[QFIDN][7] = {
"RELSAB", //Sample Header?
"GGPE01", //Mario Kart Arcade GP
"MKAGP1", //Mario Kart Arcade GP (Alt ID)
"GGPE02", //Mario Kart Arcade GP 2
"MKAGP2", //Mario Kart Arcade GP 2 (Alt ID)
"GFZJ8P", //F-Zero AX
"GVSJ8P", // Virtua Striker 4 Ver.2006 (PAL)
"GVS46E", // Virtua Striker 4 Ver.2006 (NTSC)
"GVS46J", // Virtua Striker 4 Ver.2006 (JAP)
};

bool CMenu::_QF_Game(const char *game_id)
{
	if(game_id == NULL)
		return false;
	for(u8 i = 0; i < QFIDN; i++)
	{
		if(memcmp(game_id, qfid[i], 7) == 0)
			return true;
	}
	return false;
}

void CMenu::_launchGC(dir_discHdr *hdr, bool disc)
{
	const char *id = hdr->id;
	memcpy((u8*)Disc_ID, id, 6);
	DCFlushRange((u8*)Disc_ID, 32);

	const char *path = hdr->path;
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id);

	u8 videoSetting = min(m_cfg.getInt(GC_DOMAIN, "video_setting", 1), 2);

	u8 GClanguage = min((u32)m_gcfg2.getInt(id, "gc_language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
	GClanguage = (GClanguage == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "game_language", 0), ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1u) : GClanguage-1;
	if(id[3] == 'E' || id[3] == 'J')
		GClanguage = 1; //=english
	u8 videoMode = min((u32)m_gcfg2.getInt(id, "dml_video_mode", 0), ARRAY_SIZE(CMenu::_GCvideoModes) - 1u);
	videoMode = (videoMode == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalGCvideoModes) - 1u) : videoMode-1;
	if(disc || videoMode == 0)
	{
		if(id[3] == 'E' || id[3] == 'J')
			videoMode = 2; //NTSC 480i
		else
			videoMode = 1; //PAL 576i
	}

	u8 loader = min((u32)m_gcfg2.getInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
	loader = (loader == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "default_loader", 2), ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1u) : loader-1;
	bool memcard_emu = m_gcfg2.getBool(id, "devo_memcard_emu", false);
	bool widescreen = m_gcfg2.getBool(id, "dm_widescreen", false);
	bool activity_led = m_gcfg2.getBool(id, "led", false);
	bool usb_hid = m_gcfg2.getBool(id, "USB_HID", m_cfg.getBool(GC_DOMAIN, "USB_HID", false));
	bool native_ctl = m_gcfg2.getBool(id, "NATIVE_CTL", m_cfg.getBool(GC_DOMAIN, "NATIVE_CTL", false));
	bool deflicker = m_gcfg2.getBool(id, "Deflicker", m_cfg.getBool(GC_DOMAIN, "Deflicker", false));

	if(loader == 2 && m_nintendont_installed == false)
		loader = 0;

	//always enable for nintendont
	u8 NMM = min((u32)m_gcfg2.getInt(id, "dml_nmm", m_cfg.getInt(GC_DOMAIN, "dml_nmm", 1)), ARRAY_SIZE(CMenu::_NMM) - 1u);
	NMM = (NMM == 0) ? m_cfg.getInt(GC_DOMAIN, "dml_nmm", 1) : NMM-1;

	m_cfg.setString(GC_DOMAIN, "current_item", id);

	if(loader == 0) //auto selected
	{
		if(IsOnWiiU())
		{
			if(m_nintendont_installed)
				loader = 2;
			else if(m_devo_installed)
				loader = 1;
		}
		else
		{
			gprintf("Auto installing MIOS\n");
			const char *mios_wad = NULL;
			_showWaitMessage();
			if(_QF_Game(id) == true)
			{
				if(currentPartition == SD && (m_mios_ver != 2 || m_sd_dm == false))
					mios_wad = fmt("%s/qfsd.wad", m_miosDir.c_str());
				else if(currentPartition != SD && (m_mios_ver != 2 || m_sd_dm == true))
					mios_wad = fmt("%s/qfusb.wad", m_miosDir.c_str());
			}
			else if(disc == false)
			{
				if(currentPartition == SD && (m_mios_ver != 1 || m_sd_dm == false))
					mios_wad = fmt("%s/dml.wad", m_miosDir.c_str());
				else if(currentPartition != SD && (m_mios_ver != 1 || m_sd_dm == true))
					mios_wad = fmt("%s/dm.wad", m_miosDir.c_str());
			}
			else if(m_mios_ver != 0)
				mios_wad = fmt("%s/mios.wad", m_miosDir.c_str());
			if(mios_wad != NULL && fsop_FileExist(mios_wad))
				_Wad(mios_wad, true);//install mios
		}
	}
	//copy DML game from USB to SD if needed for DML
	if(disc == false && loader == 0 && currentPartition != SD && m_sd_dm == true && strcasestr(hdr->path, ".iso") == NULL)
	{
		bool foundOnSD = false;
		ListGenerator SD_List;
		string gameDir(fmt(DML_DIR, DeviceName[SD]));
		string cacheDir(fmt("%s/%s_gamecube.db", m_listCacheDir.c_str(), DeviceName[SD]));
		SD_List.CreateList(COVERFLOW_GAMECUBE, SD, gameDir,
				stringToVector(".iso|root", '|'), cacheDir, false);
		for(vector<dir_discHdr>::iterator List = SD_List.begin(); List != SD_List.end(); List++)
		{
			if(strncasecmp(hdr->id, List->id, 6) == 0)
			{
				foundOnSD = true;
				memset(hdr->path, 0, sizeof(hdr->path));
				strncpy(hdr->path, List->path, sizeof(hdr->path));
				break;
			}
		}
		SD_List.clear();
		if(!foundOnSD)
		{
			if(_wbfsOp(CMenu::WO_COPY_GAME))
			{
				char folder[50];
				string GC_Path(hdr->path);
				if(strcasestr(GC_Path.c_str(), "boot.bin") != NULL)
					GC_Path.erase(GC_Path.end() - 13, GC_Path.end());
				else
					GC_Path.erase(GC_Path.end() - 9, GC_Path.end());
				u32 Place = GC_Path.find_last_of("/");
				GC_Path = hdr->path;
				memset(hdr->path, 0, sizeof(hdr->path));
				snprintf(folder, sizeof(folder), DML_DIR, DeviceName[SD]);
				snprintf(hdr->path, sizeof(hdr->path), "%s/%s", folder, &GC_Path[Place]+1);
			}
			else
				return;
		}
		currentPartition = SD;
	}
	_launchShutdown();
	if(disc == true)
	{
		if(loader == 0)
			DML_New_SetBootDiscOption(m_new_dm_cfg);
		else if(loader == 2)
		{
			NMM = m_cfg.getInt(GC_DOMAIN, "dml_nmm", 1);
			Nintendont_BootDisc(NMM, widescreen, usb_hid, native_ctl, deflicker);
		}
	}
	else if(loader == 0)
	{
		char CheatPath[256];
		u8 nodisc = min((u32)m_gcfg2.getInt(id, "no_disc_patch", 0), ARRAY_SIZE(CMenu::_NoDVD) - 1u);
		nodisc = (nodisc == 0) ? m_cfg.getInt(GC_DOMAIN, "no_disc_patch", 0) : nodisc-1;
		bool cheats = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool(GC_DOMAIN, "cheat", false));
		bool DML_debug = (m_gcfg2.getInt(id, "debugger", 0) == 1);
		bool screenshot = m_gcfg2.getBool(id, "screenshot", false);
		/* Generate gct path */
		char GC_Path[1024];
		GC_Path[1023] = '\0';
		strncpy(GC_Path, path, 1023);
		if(strcasestr(path, "boot.bin") != NULL)
		{
			*strrchr(GC_Path, '/') = '\0'; //boot.bin
			*(strrchr(GC_Path, '/')+1) = '\0'; //sys
		}
		else
			*(strrchr(GC_Path, '/')+1) = '\0'; //iso path
		const char *NewCheatPath = fmt("%s%s.gct", GC_Path, id);
		if(cheats)
			snprintf(CheatPath, sizeof(CheatPath), "%s/%s", m_cheatDir.c_str(), fmt("%s.gct", id));
		const char *newPath = strcasestr(path, "boot.bin") == NULL ? strchr(path, '/') : strchr(GC_Path, '/');
		if(m_new_dml)
			DML_New_SetOptions(newPath, CheatPath, NewCheatPath, DeviceName[currentPartition],
				cheats, DML_debug, NMM, nodisc, videoMode, videoSetting, widescreen, m_new_dm_cfg, activity_led, screenshot);
		else
			DML_Old_SetOptions(newPath);
		if(!nodisc || !m_new_dml)
			WDVD_StopMotor();
	}
	else if(loader == 1)
		DEVO_GetLoader(m_dataDir.c_str());
	else if(loader == 2)
	{
		bool NIN_Debugger = (m_gcfg2.getInt(id, "debugger", 0) == 2);
		bool screenshot = m_gcfg2.getBool(id, "screenshot", false);
		bool cheats = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool(GC_DOMAIN, "cheat", false));	
		/* Generate gct path */
		char GC_Path[256];
		GC_Path[255] = '\0';
		strncpy(GC_Path, path, 255);
		if(strcasestr(path, "boot.bin") != NULL)
		{
			*strrchr(GC_Path, '/') = '\0'; //boot.bin
			*(strrchr(GC_Path, '/')+1) = '\0'; //sys
		}
		else
			*(strrchr(GC_Path, '/')+1) = '\0'; //iso path
		//const char *NewPath = fmt("%s%s.gct", GC_Path, id);
		char CheatPath[256];
		char NewCheatPath[256];
		snprintf(CheatPath, sizeof(CheatPath), "%s/%s", m_cheatDir.c_str(), fmt("%s.gct", id));	
		snprintf(NewCheatPath, sizeof(NewCheatPath), "%s%s.gct",GC_Path,id);
				
		Nintendont_SetOptions(path, id, CheatPath,NewCheatPath,DeviceName[currentPartition],
			cheats, NMM, videoMode,videoSetting, widescreen,usb_hid,native_ctl,deflicker,screenshot,NIN_Debugger);
	}			
	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	cleanup();
	
 	GC_SetVideoMode(videoMode, (disc ? 1 : videoSetting), loader);
	GC_SetLanguage(GClanguage, loader);
	/* NTSC-J Patch by FIX94 */
	if(id[3] == 'J')
		*HW_PPCSPEED = 0x0002A9E0;

	if(loader == 0)
	{
		DML_New_WriteOptions();
		ShutdownBeforeExit();
		WII_Initialize();
		WII_LaunchTitle(0x100000100LL);
	}
	else if(loader == 1)
	{
		if(AHBRPOT_Patched())
			loadIOS(58, false);
		else //use cIOS instead to make sure Devolution works anyways
			loadIOS(mainIOS, false);
		ShutdownBeforeExit();
		DEVO_SetOptions(path, id, memcard_emu, 
			widescreen, activity_led, m_use_wifi_gecko);
		DEVO_Boot();
	}
	else
	{
		bool ret = (Nintendont_GetLoader() && LoadAppBooter(fmt("%s/app_booter.bin", m_binsDir.c_str())));
		Nintendont_WriteOptions();
		ShutdownBeforeExit();
		if(ret == true)
		{
			loadIOS(58, false); //nintendont NEEDS ios58
			BootHomebrew(); //regular dol
		}
	}
	Sys_Exit();
}

void CMenu::_launchHomebrew(const char *filepath, vector<string> arguments)
{
	_launchShutdown();
	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	Playlog_Delete();
	cleanup(); // wifi and sd gecko doesnt work anymore after cleanup

	bool ret = (LoadHomebrew(filepath) && LoadAppBooter(fmt("%s/app_booter.bin", m_binsDir.c_str())));

	AddBootArgument(filepath);
	for(u32 i = 0; i < arguments.size(); ++i)
	{
		gprintf("Argument: %s\n", arguments[i].c_str());
		AddBootArgument(arguments[i].c_str());
	}

	ShutdownBeforeExit();
	if(ret == true)
	{
		loadIOS(58, false);
		BootHomebrew();
	}
	Sys_Exit();
}

int CMenu::_loadIOS(u8 gameIOS, int userIOS, string id, bool RealNAND_Channels)
{
	gprintf("Game ID# %s requested IOS %d.  User selected %d\n", id.c_str(), gameIOS, userIOS);
	if(neek2o() || (RealNAND_Channels && IOS_GetType(mainIOS) == IOS_TYPE_STUB))
	{
		bool ret = loadIOS(gameIOS, false);
		_netInit();
		if(ret == false)
		{
			error(sfmt("errgame4", L"Couldn't load IOS %i", gameIOS));
			return LOAD_IOS_FAILED;
		}
		return LOAD_IOS_SUCCEEDED;
	}	
	
	if(userIOS)
	{
		for(CIOSItr itr = _installed_cios.begin(); itr != _installed_cios.end(); itr++)
		{
			if(itr->second == userIOS || itr->first == userIOS)
			{
				gameIOS = itr->first;
				break;
			}
		}
	}
	else if(gameIOS != 57)
		gameIOS = mainIOS;
	gprintf("Changed requested IOS to %d.\n", gameIOS);

	// remap IOS to CIOS
	if(gameIOS < 0x64)
	{
		if(_installed_cios.size() <= 0)
		{
			error(sfmt("errgame2", L"No cIOS found!"));
			Sys_Exit();
		}
		u8 IOS[3];
		IOS[0] = gameIOS;
		IOS[1] = 56;
		IOS[2] = 57;
		bool found = false;
		for(u8 num = 0; num < 3; num++)
		{
			if(found)
				break;
			if(IOS[num] == 0)
				continue;
			for(CIOSItr itr = _installed_cios.begin(); itr != _installed_cios.end(); itr++)
			{
				if(itr->second == IOS[num] || itr->first == IOS[num])
				{
					gameIOS = itr->first;
					found = true;
					break;
				}
			}
		}
		if(!found)
			return LOAD_IOS_FAILED;
	}

	if(gameIOS != CurrentIOS.Version)
	{
		gprintf("Reloading IOS into %d\n", gameIOS);
		bool ret = loadIOS(gameIOS, true);
		_netInit();
		if(ret == false)
		{
			error(sfmt("errgame4", L"Couldn't load IOS %i", gameIOS));
			return LOAD_IOS_FAILED;
		}
		return LOAD_IOS_SUCCEEDED;
	}
	return LOAD_IOS_NOT_NEEDED;
}

void CMenu::_launchChannel(dir_discHdr *hdr)
{
	_launchShutdown();
	string id = string(hdr->id);

	bool NAND_Emu = !m_cfg.getBool(CHANNEL_DOMAIN, "disable", true);
	bool WII_Launch = (m_gcfg2.getBool(id, "custom", false) && (!NAND_Emu || neek2o()));
	bool use_dol = !m_gcfg2.getBool(id, "apploader", false);

	bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
	bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool(CHANNEL_DOMAIN, "cheat", false));
	bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));

	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
	videoMode = (videoMode == 0) ? (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1) : videoMode-1;

	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	language = (language == 0) ? min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1) : language;

	const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;
	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	int aspectRatio = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u)-1;
	u32 returnTo = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];

	u8 *cheatFile = NULL;
	u32 cheatSize = 0;
	if(!WII_Launch)
	{
		hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 0);
		debuggerselect = m_gcfg2.getInt(id, "debugger", 0);
		if((cheat || debuggerselect == 1) && hooktype == 0)
			hooktype = 1;
		else if(!cheat && debuggerselect != 1)
			hooktype = 0;

		if(cheat)
			_loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", id.c_str()));
		if(has_enabled_providers() && _initNetwork() == 0)
			add_game_to_card(id.c_str());
	}
	m_cfg.setString(CHANNEL_DOMAIN, "current_item", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1); 
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	string emuPath = m_cfg.getString(CHANNEL_DOMAIN, "path");
	int emulate_mode = min(max(0, m_cfg.getInt(CHANNEL_DOMAIN, "emulation", 1)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
	
	int userIOS = m_gcfg2.getInt(id, "ios", 0);
	u64 gameTitle = TITLE_ID(hdr->settings[0],hdr->settings[1]);
	bool useNK2o = (m_gcfg2.getBool(id, "useneek", false) && !neek2o());
	bool use_led = m_gcfg2.getBool(id, "led", false);
	u32 gameIOS = ChannelHandle.GetRequestedIOS(gameTitle);

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	cleanup();

	if(NAND_Emu && !neek2o())
	{
		NANDemuView = true;
		if(useNK2o)
		{
			if(!Load_Neek2o_Kernel())
			{
				error(_t("errneek1", L"Cannot launch neek2o. Verify your neek2o setup"));
				Sys_Exit();
			}
			ShutdownBeforeExit();
			Launch_nk(gameTitle, NandHandle.Get_NandPath(), 
				returnTo ? (((u64)(0x00010001) << 32) | (returnTo & 0xFFFFFFFF)) : 0);
			while(1) usleep(500);
		}
	}
	if(WII_Launch == false && ExternalBooter_LoadBins(m_binsDir.c_str()) == false)
		Sys_Exit();
	if(_loadIOS(gameIOS, userIOS, id, !NAND_Emu) == LOAD_IOS_FAILED)
		Sys_Exit();

	if((CurrentIOS.Type == IOS_TYPE_D2X || neek2o()) && returnTo != 0)
	{
		if(D2X_PatchReturnTo(returnTo) >= 0)
			memset(&returnTo, 0, sizeof(u32));
	}
	if(NAND_Emu && !neek2o())
	{
		/* Enable our Emu NAND */
		DeviceHandle.UnMountAll();
		if(emulate_mode == 1)
			NandHandle.Set_FullMode(true);
		else
			NandHandle.Set_FullMode(false);
		if(NandHandle.Enable_Emu() < 0)
		{
			NandHandle.Disable_Emu();
			error(_t("errgame5", L"Enabling emu failed!"));
			Sys_Exit();
		}
		DeviceHandle.MountAll();
	}
	if(WII_Launch)
	{
		ShutdownBeforeExit();
		WII_Initialize();
		WII_LaunchTitle(gameTitle);
	}
	else
	{
		setLanguage(language);
		ocarina_load_code(cheatFile, cheatSize);
		NandHandle.Patch_AHB(); /* Identify may takes it */
		PatchIOS(true); /* Patch for everything */
		Identify(gameTitle);

		ExternalBooter_ChannelSetup(gameTitle, use_dol);
		WiiFlow_ExternalBooter(videoMode, vipatch, countryPatch, patchVidMode, aspectRatio, 0, TYPE_CHANNEL, use_led);
	}
	Sys_Exit();
}

void CMenu::_launchGame(dir_discHdr *hdr, bool dvd)
{
	_launchShutdown();
	string id(hdr->id);
	string path(hdr->path);
	if(neek2o())
	{
		int discID = id.c_str()[0] << 24 | id.c_str()[1] << 16 | id.c_str()[2] << 8 | id.c_str()[3];
		if(WDVD_NEEK_LoadDisc((discID&0xFFFFFFFF), 0x5D1C9EA3) > 0)
		{
			dvd = true;
			sleep(3);
		}
	}

	if(dvd)
	{
		u32 cover = 0;
		if(!neek2o() && !Sys_DolphinMode())
		{
			if(WDVD_GetCoverStatus(&cover) < 0)
			{
				error(_t("errgame7", L"WDVDGetCoverStatus Failed!"));
				Sys_Exit();
			}
			if(!(cover & 0x2))
			{
				error(_t("errgame8", L"Please insert a game disc."));
				do
				{
					WDVD_GetCoverStatus(&cover);
					if(BTN_B_PRESSED)
						return;
				} while(!(cover & 0x2));
			}
		}
		TempLoadIOS();
		/* Open Disc */
		if(Disc_Open(true) < 0)
		{
			error(_t("wbfsoperr2", L"Disc_Open failed"));
			Sys_Exit();
		}
		/* Check disc */
		if(Disc_IsWii() < 0)
		{
			if (Disc_IsGC() < 0) 
			{
				error(_t("errgame9", L"This is not a Wii or GC disc"));
				Sys_Exit();
			}
			else
			{
				/* Read GC disc header */
				Disc_ReadGCHeader(&gc_hdr);
				memcpy(hdr->id, gc_hdr.id, 6);
				/* Launching GC Game */
				_launchGC(hdr, true);
				return;
			}
		}
		else
		{
			/* Read header */
			Disc_ReadHeader(&wii_hdr);
			id = string((const char*)wii_hdr.id, 6);
		}
		gprintf("Game ID: %s\n", id.c_str());
	}

	bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
	bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));
	bool private_server = m_gcfg2.testOptBool(id, "private_server", m_cfg.getBool("GENERAL", "private_server", false));

	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
	videoMode = (videoMode == 0) ? (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1) : videoMode-1;

	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	language = (language == 0) ? min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1) : language;

	const char *rtrn = m_cfg.getString("GENERAL", "returnto", "").c_str();
	int aspectRatio = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u)-1;

	string emuPath;
	int emuPartition = 0;

	u8 emulate_mode = min((u32)m_gcfg2.getInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);
	if(emulate_mode == 0)
	{
		emulate_mode = min(max(0, m_cfg.getInt(WII_DOMAIN, "save_emulation", 0)), (int)ARRAY_SIZE(CMenu::_GlobalSaveEmu) - 1);
		if(emulate_mode != 0)
			emulate_mode++;
	}
	else if(emulate_mode == 1)
		emulate_mode = 0;
	m_current_view = COVERFLOW_WII; // used for _FindEmuPart()
	if(emulate_mode && !dvd && !neek2o())
	{
		emuPartition = _FindEmuPart(emuPath, false);
		if(emuPartition < 0)
		{
			if(emulate_mode == 4)
			{
				_hideWaitMessage();
				while(true)
				{
					_AutoCreateNand();
					if(_TestEmuNand(m_cfg.getInt(WII_DOMAIN, "savepartition", 0), emuPath.c_str(), true))
					{
						emuPartition = m_cfg.getInt(WII_DOMAIN, "savepartition", -1);
						emuPath = m_cfg.getString(WII_DOMAIN, "savepath", m_cfg.getString(CHANNEL_DOMAIN, "path", ""));
						break;
					}
				}
				_showWaitMessage();
			}
			else
			{
				emuPartition = _FindEmuPart(emuPath, true);
				NandHandle.CreatePath(fmt("%s:/%s", DeviceName[emuPartition], APPDATA_DIR));
				NandHandle.CreatePath(fmt("%s:/%s/nandemu", DeviceName[emuPartition], APPDATA_DIR));
			}
		}
		/* Set them */
		NANDemuView = true;
		m_cfg.setInt(WII_DOMAIN, "savepartition", emuPartition);
		m_cfg.setString(WII_DOMAIN, "savepath", emuPath);
		if(emulate_mode == 2)
		{
			m_forceext = false;
			_hideWaitMessage();
			if(!_AutoExtractSave(id))
				NandHandle.CreateTitleTMD(hdr);
			_showWaitMessage();
		}
		else if(emulate_mode > 2)
		{
			NandHandle.CreateConfig();
			NandHandle.Do_Region_Change(id, false);
		}
	}
	else
		emulate_mode = 0;

	bool use_led = m_gcfg2.getBool(id, "led", false);
	bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool(WII_DOMAIN, "cheat", false));
	debuggerselect = m_gcfg2.getInt(id, "debugger", 0); // debuggerselect is defined in fst.h
	if((id == "RPWE41" || id == "RPWZ41" || id == "SPXP41") && debuggerselect == 1) // Prince of Persia, Rival Swords
		debuggerselect = 0;
	hooktype = (u32)m_gcfg2.getInt(id, "hooktype", 0); // hooktype is defined in patchcode.h
	if((cheat || debuggerselect == 1) && hooktype == 0)
		hooktype = 1;
	else if(!cheat && debuggerselect != 1)
		hooktype = 0;

	u8 *cheatFile = NULL;
	u8 *gameconfig = NULL;
	u32 cheatSize = 0, gameconfigSize = 0, returnTo = 0;

	m_cfg.setString(WII_DOMAIN, "current_item", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	setLanguage(language);

	load_wip_patches((u8 *)m_wipDir.c_str(), (u8 *) &id);
	if(cheat)
		_loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", id.c_str()));
	_loadFile(gameconfig, gameconfigSize, m_txtCheatDir.c_str(), "gameconfig.txt");

	if(strlen(rtrn) == 4)
		returnTo = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];
	int userIOS = m_gcfg2.getInt(id, "ios", 0);
	int gameIOS = dvd && !neek2o() ? userIOS : GetRequestedGameIOS(hdr);

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	cleanup(); // wifi and sd gecko doesnt work anymore after cleanup


	bool patchregion = false;
	if(emulate_mode <= 2 && !neek2o() && m_cfg.getBool("GENERAL", "tempregionrn", false))
	{
		gprintf("Check\n");
		patchregion = NandHandle.Do_Region_Change(id, true);
	}
	if(ExternalBooter_LoadBins(m_binsDir.c_str()) == false)
		Sys_Exit();
	if((!dvd || neek2o()) && !Sys_DolphinMode())
	{
		if(_loadIOS(gameIOS, userIOS, id) == LOAD_IOS_FAILED)
			Sys_Exit();
	}

	if(CurrentIOS.Type == IOS_TYPE_D2X)
	{
		if(returnTo != 0 && !m_directLaunch && D2X_PatchReturnTo(returnTo) >= 0)
			memset(&returnTo, 0, sizeof(u32));
		if(emulate_mode)
		{
			/* Enable our Emu NAND */
			DeviceHandle.UnMountAll();
			if(emulate_mode == 3)
				NandHandle.Set_RCMode(true);
			else if(emulate_mode == 4)
				NandHandle.Set_FullMode(true);
			else
				NandHandle.Set_FullMode(false);
			if(NandHandle.Enable_Emu() < 0)
			{
				NandHandle.Disable_Emu();
				error(_t("errgame6", L"Enabling emu after reload failed!"));
				Sys_Exit();
			}
			DeviceHandle.MountAll();
		}
	}
	bool wbfs_partition = false;
	if(!dvd)
	{
		DeviceHandle.OpenWBFS(currentPartition);
		wbfs_partition = (DeviceHandle.GetFSType(currentPartition) == PART_FS_WBFS);
		if(!wbfs_partition && get_frag_list((u8 *)id.c_str(), (char*)path.c_str(), currentPartition == 0 ? 0x200 : USBStorage2_GetSectorSize()) < 0)
			Sys_Exit();
		WBFS_Close();
	}
	if(cheatFile != NULL)
	{
		ocarina_load_code(cheatFile, cheatSize);
		free(cheatFile);
	}
	if(gameconfig != NULL)
	{
		app_gameconfig_load(id.c_str(), gameconfig, gameconfigSize);
		free(gameconfig);
	}

	ExternalBooter_WiiGameSetup(wbfs_partition, dvd, patchregion, private_server, id.c_str());
	WiiFlow_ExternalBooter(videoMode, vipatch, countryPatch, patchVidMode, aspectRatio, returnTo, TYPE_WII_GAME, use_led);

	Sys_Exit();
}

void CMenu::_initGameMenu()
{
	CColor fontColor(0xD0BFDFFF);
	TexData texGameFavOn;
	TexData texGameFavOnSel;
	TexData texGameFavOff;
	TexData texGameFavOffSel;
	TexData texAdultOn;
	TexData texAdultOnSel;
	TexData texAdultOff;
	TexData texAdultOffSel;
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
	TexHandle.fromImageFile(texAdultOn, fmt("%s/stopkidon.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texAdultOnSel, fmt("%s/stopkidons.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texAdultOff, fmt("%s/stopkidoff.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texAdultOffSel, fmt("%s/stopkidoffs.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texDelete, fmt("%s/delete.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texDeleteSel, fmt("%s/deletes.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texSettings, fmt("%s/btngamecfg.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texSettingsSel, fmt("%s/btngamecfgs.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(texToggleBanner, fmt("%s/blank.png", m_imgsDir.c_str()));

	_addUserLabels(m_gameLblUser, ARRAY_SIZE(m_gameLblUser), "GAME");
	m_gameBg = _texture("GAME/BG", "texture", theme.bg, false);
	if(m_theme.loaded() && TexHandle.fromImageFile(bgLQ, fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GAME/BG", "texture").c_str()), GX_TF_CMPR, 64, 64) == TE_OK)
		m_gameBgLQ = bgLQ;

	m_gameBtnPlay = _addButton("GAME/PLAY_BTN", theme.btnFont, L"", 420, 344, 200, 48, theme.btnFontColor);
	m_gameBtnBack = _addButton("GAME/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_gameBtnFavoriteOn = _addPicButton("GAME/FAVORITE_ON", texGameFavOn, texGameFavOnSel, 460, 200, 48, 48);
	m_gameBtnFavoriteOff = _addPicButton("GAME/FAVORITE_OFF", texGameFavOff, texGameFavOffSel, 460, 200, 48, 48);
	m_gameBtnAdultOn = _addPicButton("GAME/ADULTONLY_ON", texAdultOn, texAdultOnSel, 532, 200, 48, 48);
	m_gameBtnAdultOff = _addPicButton("GAME/ADULTONLY_OFF", texAdultOff, texAdultOffSel, 532, 200, 48, 48);
	m_gameBtnSettings = _addPicButton("GAME/SETTINGS_BTN", texSettings, texSettingsSel, 460, 272, 48, 48);
	m_gameBtnDelete = _addPicButton("GAME/DELETE_BTN", texDelete, texDeleteSel, 532, 272, 48, 48);
	m_gameBtnBackFull = _addButton("GAME/BACK_FULL_BTN", theme.btnFont, L"", 100, 390, 200, 56, theme.btnFontColor);
	m_gameBtnPlayFull = _addButton("GAME/PLAY_FULL_BTN", theme.btnFont, L"", 340, 390, 200, 56, theme.btnFontColor);
	m_gameBtnToogle = _addPicButton("GAME/TOOGLE_BTN", texToggleBanner, texToggleBanner, 385, 31, 236, 127);
	m_gameBtnToogleFull = _addPicButton("GAME/TOOGLE_FULL_BTN", texToggleBanner, texToggleBanner, 20, 12, 608, 344);

	m_gameButtonsZone.x = m_theme.getInt("GAME/ZONES", "buttons_x", 0);
	m_gameButtonsZone.y = m_theme.getInt("GAME/ZONES", "buttons_y", 0);
	m_gameButtonsZone.w = m_theme.getInt("GAME/ZONES", "buttons_w", 640);
	m_gameButtonsZone.h = m_theme.getInt("GAME/ZONES", "buttons_h", 480);
	m_gameButtonsZone.hide = m_theme.getBool("GAME/ZONES", "buttons_hide", true);

	_setHideAnim(m_gameBtnPlay, "GAME/PLAY_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnBack, "GAME/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnFavoriteOn, "GAME/FAVORITE_ON", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnFavoriteOff, "GAME/FAVORITE_OFF", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnAdultOn, "GAME/ADULTONLY_ON", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnAdultOff, "GAME/ADULTONLY_OFF", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnSettings, "GAME/SETTINGS_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnDelete, "GAME/DELETE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_gameBtnPlayFull, "GAME/PLAY_FULL_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnBackFull, "GAME/BACK_FULL_BTN", 0, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnToogle, "GAME/TOOGLE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnToogleFull, "GAME/TOOGLE_FULL_BTN", 200, 0, 1.f, 0.f);
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

void CMenu::_gameSoundThread(CMenu *m)
{
	m->m_soundThrdBusy = true;
	m->m_gamesound_changed = false;
	CurrentBanner.ClearBanner();

	const dir_discHdr *GameHdr = CoverFlow.getHdr();
	if(GameHdr->type == TYPE_PLUGIN)
	{
		m_banner.DeleteBanner();
		m->m_gameSound.Load(m_plugin.GetBannerSound(GameHdr->settings[0]), m_plugin.GetBannerSoundSize());
		if(m->m_gameSound.IsLoaded())
			m->m_gamesound_changed = true;
		m->m_soundThrdBusy = false;
		return;
	}
	u8 *custom_bnr_file = NULL;
	u32 custom_bnr_size = 0;

	u8 *cached_bnr_file = NULL;
	u32 cached_bnr_size = 0;

	char cached_banner[256];
	cached_banner[255] = '\0';
	char custom_banner[256];
	custom_banner[255] = '\0';
	/* check custom ID6 first */
	strncpy(custom_banner, fmt("%s/%s.bnr", m->m_customBnrDir.c_str(), GameHdr->id), 255);
	fsop_GetFileSizeBytes(custom_banner, &custom_bnr_size);
	if(custom_bnr_size > 0)
	{
		custom_bnr_file = (u8*)MEM2_lo_alloc(custom_bnr_size);
		if(custom_bnr_file != NULL)
			fsop_ReadFileLoc(custom_banner, custom_bnr_size, (void*)custom_bnr_file);
	}
	else /* no custom ID6 or too big, try ID3 */
	{
		strncpy(custom_banner, fmt("%s/%.3s.bnr", m->m_customBnrDir.c_str(), GameHdr->id), 255);
		fsop_GetFileSizeBytes(custom_banner, &custom_bnr_size);
		if(custom_bnr_size > 0)
		{
			custom_bnr_file = (u8*)MEM2_lo_alloc(custom_bnr_size);
			if(custom_bnr_file != NULL)
				fsop_ReadFileLoc(custom_banner, custom_bnr_size, (void*)custom_bnr_file);
		}
	}
	if(custom_bnr_file == NULL && GameHdr->type == TYPE_GC_GAME)
	{
		if(m->_QF_Game(GameHdr->id) == false)
		{
			GC_Disc_Reader.init(GameHdr->path);
			u8 *opening_bnr = GC_Disc_Reader.GetGameCubeBanner();
			if(opening_bnr != NULL)
				m_banner.CreateGCBanner(opening_bnr, m->m_wbf1_font, m->m_wbf2_font, GameHdr->title);
			GC_Disc_Reader.clear();
		}
		m->m_gameSound.Load(gc_ogg, gc_ogg_size, false);
		if(m->m_gameSound.IsLoaded())
			m->m_gamesound_changed = true;
		m->m_soundThrdBusy = false;
		return;
	}
	if(custom_bnr_file == NULL)/* no custom and not GC game try cached banner*/
	{
		strncpy(cached_banner, fmt("%s/%s.bnr", m->m_bnrCacheDir.c_str(), GameHdr->id), 255);
		fsop_GetFileSizeBytes(cached_banner, &cached_bnr_size);
		if(cached_bnr_size > 0)
		{
			cached_bnr_file = (u8*)MEM2_lo_alloc(cached_bnr_size);
			if(cached_bnr_file != NULL)
				fsop_ReadFileLoc(cached_banner, cached_bnr_size, (void*)cached_bnr_file);
		}
	}

	if(custom_bnr_file != NULL)
		CurrentBanner.SetBanner(custom_bnr_file, custom_bnr_size, true, true);
	else if(cached_bnr_file != NULL)
		CurrentBanner.SetBanner(cached_bnr_file, cached_bnr_size, false, true);
	else if(GameHdr->type == TYPE_WII_GAME)
		_extractBnr(GameHdr);
	else if(GameHdr->type == TYPE_CHANNEL)
		_extractChannelBnr(TITLE_ID(GameHdr->settings[0],
									GameHdr->settings[1]));
	if(!CurrentBanner.IsValid())
	{
		m->m_gameSound.FreeMemory();
		m_banner.DeleteBanner();
		CurrentBanner.ClearBanner();
		m->m_soundThrdBusy = false;
		return;
	}
	if(cached_bnr_file == NULL && custom_bnr_file == NULL)
		fsop_WriteFile(cached_banner, CurrentBanner.GetBannerFile(), CurrentBanner.GetBannerFileSize());

	u32 sndSize = 0;
	m_banner.LoadBanner(m->m_wbf1_font, m->m_wbf2_font);
	u8 *soundBin = CurrentBanner.GetFile("sound.bin", &sndSize);
	CurrentBanner.ClearBanner();

	if(soundBin != NULL)
	{
		if(memcmp(&((IMD5Header *)soundBin)->fcc, "IMD5", 4) == 0)
		{
			u32 newSize = 0;
			u8 *newSound = DecompressCopy(soundBin, sndSize, &newSize);
			if(newSound == NULL || newSize == 0 || !m->m_gameSound.Load(newSound, newSize))
			{
				free(soundBin);
				m->m_gameSound.FreeMemory();
				m_banner.DeleteBanner();
				m->m_soundThrdBusy = false;
				return;
			}
		}
		else
			m->m_gameSound.Load(soundBin, sndSize);

		if(m->m_gameSound.IsLoaded())
			m->m_gamesound_changed = true;
		else
		{
			m->m_gameSound.FreeMemory();
			m_banner.DeleteBanner();
		}
	}
	else
	{
		gprintf("WARNING: No sound found in banner!\n");
		m->m_gamesound_changed = true;
		m->m_gameSound.FreeMemory();
	}
	m->m_soundThrdBusy = false;
}

u8 *GameSoundStack = NULL;
u32 GameSoundSize = 0x10000; //64kb
void CMenu::_playGameSound(void)
{
	if(NoGameID(CoverFlow.getHdr()->type))
	{
		if(_startVideo())
			return;
		if(m_zoom_banner == true)
		{
			m_zoom_banner = m_banner.ToogleZoom();
			m_cfg.setBool(_domainFromView(), "show_full_banner", m_zoom_banner);
			currentMoviePos = normalMoviePos;
		}
	}
	m_gamesound_changed = false;
	if(m_bnrSndVol == 0) 
		return;

	if(m_gameSoundThread != LWP_THREAD_NULL)
		CheckGameSoundThread();
	GameSoundStack = (u8*)MEM2_lo_alloc(GameSoundSize);
	LWP_CreateThread(&m_gameSoundThread, (void *(*)(void *))CMenu::_gameSoundThread, (void*)this, GameSoundStack, GameSoundSize, 60);
}

void CMenu::CheckGameSoundThread()
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
