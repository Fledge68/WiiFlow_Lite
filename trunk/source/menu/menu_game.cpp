
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
s16 m_gameBtnToggle;
s16 m_gameBtnToggleFull;

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

const CMenu::SOption CMenu::_ChannelsType[3] = {
	{ "ChanReal", L"Real NAND" },
	{ "ChanEmu", L"Emu NAND" },
	{ "ChanBoth", L"Both" },
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

//still need to change the NMM's
const CMenu::SOption CMenu::_NinEmuCard[5] = {
	{ "NMMDef", L"Default" },
	{ "NMMOff", L"Disabled" },
	{ "NMMon", L"Enabled" },
	{ "NMMMulti", L"Multi Saves" },
	{ "NMMdebug", L"Debug" },
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

map<u8, u8> CMenu::_installed_cios;
u8 banner_title[84];

static inline int loopNum(int i, int s)
{
	return (i + s) % s;
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

static void setLanguage(int l)
{
	if (l > 0 && l <= 10)
		configbytes[0] = l - 1;
	else
		configbytes[0] = 0xCD;
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
	_cleanupVideo();
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
	m_btnMgr.hide(m_gameBtnAdultOn, instant);
	m_btnMgr.hide(m_gameBtnAdultOff, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if(m_gameLblUser[i] != -1)
			m_btnMgr.hide(m_gameLblUser[i], instant);
}

void CMenu::_showGame(void)
{
	_setBg(m_gameBg, m_gameBgLQ);

	if(!m_zoom_banner)
	{
		for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 1; ++i)
			if(m_gameLblUser[i] != -1)
				m_btnMgr.show(m_gameLblUser[i]);
	
		m_btnMgr.show(m_gameBtnPlay);
		m_btnMgr.show(m_gameBtnBack);
		m_btnMgr.show(m_gameBtnToggle);
	}
	else
	{
		m_btnMgr.show(m_gameBtnPlayFull);
		m_btnMgr.show(m_gameBtnBackFull);
		m_btnMgr.show(m_gameBtnToggleFull);
	}
}

void CMenu::_cleanupBanner(bool gamechange)
{
	//banner
	m_gameSound.FreeMemory();
	CheckGameSoundThread();
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
	//strncpy(m_plugin.PluginMagicWord, fmt("%08x", CoverFlow.getHdr()->settings[0]), 8);
	const char *videoPath = fmt("%s/%s", videoDir.c_str(), m_plugin.PluginMagicWord);
	return videoPath;
}

bool CMenu::_startVideo()
{
	const dir_discHdr *GameHdr = CoverFlow.getHdr();
	char curId3[4];
	memset(curId3, 0, 4);
	const char *videoId = CoverFlow.getPathId(GameHdr);
	if(!NoGameID(GameHdr->type))
	{	//id3
		memcpy(curId3, GameHdr->id, 3);
		videoId = curId3;
	}
	const char *videoPath = getVideoPath(m_videoDir, videoId);
	const char *THP_Path = fmt("%s.thp", videoPath);
	if(!fsop_FileExist(THP_Path))
	{
		if(GameHdr->type == TYPE_PLUGIN)
		{
			videoPath = getVideoDefaultPath(m_videoDir);
			THP_Path = fmt("%s.thp", videoPath);
		}
		else if(!NoGameID(GameHdr->type))
		{
			videoPath = getVideoPath(m_videoDir, GameHdr->id);
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
	bool coverFlipped = false;
	int cf_version = 1;
	string domain;
	string key;
	Vector3D v;
	Vector3D savedv;
	
	dir_discHdr *hdr = (dir_discHdr*)MEM2_alloc(sizeof(dir_discHdr));
	memcpy(hdr, CoverFlow.getHdr(), sizeof(dir_discHdr));
	
	const char *id = NULL;
	char tmp1[74];// title/magic#
	memset(tmp1, 0, 74);
	char tmp2[64];
	memset(tmp2, 0, 64);
	if(hdr->type == TYPE_PLUGIN)
	{
		strncpy(m_plugin.PluginMagicWord, fmt("%08x", hdr->settings[0]), 8);
		wcstombs(tmp2, hdr->title, 64);
		strcat(tmp1, m_plugin.PluginMagicWord);
		strcat(tmp1, fmt("/%s", tmp2));
		id = tmp1;
	}
	else
	{
		id = hdr->id;
		if(hdr->type == TYPE_GC_GAME && hdr->settings[0] == 1) /* disc 2 */
		{
			strcat(tmp1, fmt("%.6s_2", hdr->id));
			id = tmp1;
		}
	}

	m_zoom_banner = m_cfg.getBool(_domainFromView(), "show_full_banner", false);
	if(NoGameID(hdr->type))
	{
		bool video_available = (hdr->type == TYPE_PLUGIN && 
								(fsop_FileExist(fmt("%s.thp", getVideoPath(m_videoDir, CoverFlow.getPathId(hdr)))) || 
								fsop_FileExist(fmt("%s.thp", getVideoDefaultPath(m_videoDir)))));
		m_zoom_banner = m_zoom_banner && video_available;
	}
	currentMoviePos = (m_zoom_banner ? zoomedMoviePos : normalMoviePos);
	if(m_banner.GetZoomSetting() != m_zoom_banner)
		m_banner.ToggleZoom();

	s8 startGameSound = -7;
	SetupInput();

	while(!m_exit)
	{
		if(startGameSound < 1)
			startGameSound++;

		if(startGameSound == -5)
			_showGame();
			
		if(!launch)	
			_mainLoopCommon(true);

		if(startGameSound == 0)
		{
			m_gameSelected = true;// mark game selected and load sound/banner for main loop to start playing
			startGameSound = 1;
			_playGameSound();
		}
		/* move and zoom flipped cover */
		if(coverFlipped && 
			(BTN_PLUS_PRESSED || BTN_MINUS_PRESSED ||
			BTN_LEFT_PRESSED || BTN_RIGHT_PRESSED ||
			BTN_UP_PRESSED || BTN_DOWN_PRESSED))
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
		/* if B on favorites btn goto game categories */
		if(BTN_B_PRESSED && (m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff)))
		{
			_hideGame();
			m_banner.SetShowBanner(false);
			if(m_locked)
				error(_t("errgame15", L"WiiFlow locked! Unlock WiiFlow to use this feature."));
			else
				_CategorySettings(true);
			_showGame();
			m_banner.SetShowBanner(true);
			if(!m_gameSound.IsPlaying()) 
				startGameSound = -6;
			continue;
		}
		/* exit game menu or reset flipped cover */
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			if(BTN_B_PRESSED && coverFlipped)
			{
				CoverFlow.flip();
				coverFlipped = false;
			}
			else
			{
				_cleanupBanner();
				break;
			}
		}
		/* press + for game info screen */
		else if(BTN_PLUS_PRESSED && !NoGameID(hdr->type) && !coverFlipped)
		{
			GameTDB m_gametdb; 
			m_gametdb.OpenFile(fmt("%s/wiitdb.xml", m_settingsDir.c_str()));
			if(m_gametdb.IsLoaded())
			{
				_hideGame();
				m_banner.SetShowBanner(false);
				_gameinfo();
				m_gametdb.CloseFile();
				_showGame();
				m_banner.SetShowBanner(true);
				if(!m_gameSound.IsPlaying())// this makes the game sound play when we return
					startGameSound = -6;
			}
		}
		/* play or stop a video (this is the old video play for non plugin games) */
		else if(BTN_MINUS_PRESSED && !coverFlipped && !NoGameID(hdr->type))
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
		else if((BTN_1_PRESSED || BTN_2_PRESSED) && !coverFlipped)
		{
			s8 direction = BTN_1_PRESSED ? 1 : -1;
			int cfVersion = loopNum((_getCFVersion() - 1) + direction, m_numCFVersions) + 1;
			_loadCFLayout(cfVersion);
			CoverFlow.applySettings();
			_setCFVersion(cfVersion);
		}
		else if(launch || BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_gameBtnDelete))
			{
				_hideGame();
				m_banner.SetShowBanner(false);
				if(m_locked)
					error(_t("errgame15", L"WiiFlow locked! Unlock WiiFlow to use this feature."));
				else if(hdr->type == TYPE_CHANNEL)
					error(_t("errgame17", L"Can not delete real NAND Channels!"));
				else if(hdr->type == TYPE_PLUGIN)
				{
					const char *wfcPath = NULL;
					const char *coverDir = NULL;
					if(m_cfg.getBool(PLUGIN_DOMAIN, "subfolder_cache"))
						coverDir = m_plugin.GetCoverFolderName(hdr->settings[0]);
					if(coverDir == NULL || strlen(coverDir) == 0)
						wfcPath = fmt("%s/%s.wfc", m_cacheDir.c_str(), CoverFlow.getPathId(hdr, true));
					else
						wfcPath = fmt("%s/%s/%s.wfc", m_cacheDir.c_str(), coverDir, CoverFlow.getPathId(hdr, true));
					fsop_deleteFile(wfcPath);
					CoverFlow.stopCoverLoader(true);
					CoverFlow.startCoverLoader();
				}
				else
				{
					if(_wbfsOp(WO_REMOVE_GAME))
					{
						_cleanupBanner();
						break;
					}
				}
				_showGame();
				m_banner.SetShowBanner(true);
				if(!m_gameSound.IsPlaying())
					startGameSound = -6;
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
				/*else if(hdr->type == TYPE_PLUGIN)
				{
					m_banner.SetShowBanner(false);
					error(_t("errgame16", L"Not available for plugin games!"));
					m_banner.SetShowBanner(true);
				}*/
				else
				{
					m_banner.ToggleGameSettings();
					_gameSettings();
					m_banner.ToggleGameSettings();
				}
				_showGame();
				if(!m_gameSound.IsPlaying()) 
					startGameSound = -6;
			}
			else if(m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff))
			{
				if(hdr->type == TYPE_PLUGIN)
					m_gcfg1.setBool("FAVORITES_PLUGINS", id, !m_gcfg1.getBool("FAVORITES_PLUGINS", id, false));
				else
					m_gcfg1.setBool("FAVORITES", id, !m_gcfg1.getBool("FAVORITES", id, false));
			}
			else if(m_btnMgr.selected(m_gameBtnAdultOn) || m_btnMgr.selected(m_gameBtnAdultOff))
			{
				if(m_locked)
				{
					m_banner.SetShowBanner(false);
					error(_t("errgame15", L"WiiFlow locked! Unlock WiiFlow to use this feature."));
					m_banner.SetShowBanner(true);
					_showGame();
					if(!m_gameSound.IsPlaying()) 
						startGameSound = -6;
				}
				else
				{
					if(hdr->type == TYPE_PLUGIN)
						m_gcfg1.setBool("ADULTONLY_PLUGINS", id, !m_gcfg1.getBool("ADULTONLY_PLUGINS", id, false));
					else
						m_gcfg1.setBool("ADULTONLY", id, !m_gcfg1.getBool("ADULTONLY", id, false));
				}
			}
			else if(m_btnMgr.selected(m_gameBtnBack) || m_btnMgr.selected(m_gameBtnBackFull))
			{
				_cleanupBanner();
				break;
			}
			else if((m_btnMgr.selected(m_gameBtnToggle) || m_btnMgr.selected(m_gameBtnToggleFull)) 
					&& (!NoGameID(hdr->type) || m_video_playing))
			{
				m_zoom_banner = m_banner.ToggleZoom();
				m_cfg.setBool(_domainFromView(), "show_full_banner", m_zoom_banner);
				currentMoviePos = (m_zoom_banner ? zoomedMoviePos : normalMoviePos);
				m_show_zone_game = false;
				m_btnMgr.hide(m_gameBtnPlayFull);
				m_btnMgr.hide(m_gameBtnBackFull);
				m_btnMgr.hide(m_gameBtnToggleFull);
			}
			else if(launch || m_btnMgr.selected(m_gameBtnPlay) || m_btnMgr.selected(m_gameBtnPlayFull) || !ShowPointer())
			{
				_hideGame();
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
				CurrentBanner.ClearBanner();
				NANDemuView = hdr->type == TYPE_EMUCHANNEL;
				if(hdr->type == TYPE_CHANNEL || hdr->type == TYPE_EMUCHANNEL)
				{
					u64 chantitle = CoverFlow.getChanTitle();
					_extractChannelBnr(chantitle);
				}
				else if(hdr->type == TYPE_WII_GAME)
					_extractBnr(hdr);
				if(CurrentBanner.IsValid())
					_extractBannerTitle(GetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str()));
				if(hdr->type == TYPE_WII_GAME || hdr->type == TYPE_CHANNEL || hdr->type == TYPE_EMUCHANNEL)
				{
					if(Playlog_Update(hdr->id, banner_title) < 0)
						Playlog_Delete();
				}
				CurrentBanner.ClearBanner();

				/* Finally boot it */
				gprintf("Launching game %s\n", hdr->id);
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
			/* flip cover if mouse over */
			else if(!coverFlipped)
			{
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
					}
				}
			}
		}
		if(!coverFlipped)
		{
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
			if(startGameSound == -10)// if -10 then we moved to new cover
			{
				m_gameSelected = false; // deselect game if moved to new cover
				memcpy(hdr, CoverFlow.getHdr(), sizeof(dir_discHdr));// get new game header
				memset(tmp1, 0, 74);
				memset(tmp2, 0, 64);
				if(hdr->type == TYPE_PLUGIN)
				{
					strncpy(m_plugin.PluginMagicWord, fmt("%08x", hdr->settings[0]), 8);
					wcstombs(tmp2, hdr->title, 64);
					strcat(tmp1, m_plugin.PluginMagicWord);
					strcat(tmp1, fmt("/%s", tmp2));
					id = tmp1;
				}
				else
				{
					id = hdr->id;
					if(hdr->type == TYPE_GC_GAME && hdr->settings[0] == 1) /* disc 2 */
					{
						strcat(tmp1, fmt("%.6s_2", hdr->id));
						id = tmp1;
					}
				}
				m_zoom_banner = m_cfg.getBool(_domainFromView(), "show_full_banner", false);
				if(NoGameID(hdr->type))
				{
					bool video_available = (hdr->type == TYPE_PLUGIN && 
											(fsop_FileExist(fmt("%s.thp", getVideoPath(m_videoDir, CoverFlow.getPathId(hdr)))) || 
											fsop_FileExist(fmt("%s.thp", getVideoDefaultPath(m_videoDir)))));
					m_zoom_banner = m_zoom_banner && video_available;
				}
				currentMoviePos = (m_zoom_banner ? zoomedMoviePos : normalMoviePos);
				if(m_banner.GetZoomSetting() != m_zoom_banner)
					m_banner.ToggleZoom();
			}
		}
		/* show small banner frame if available */
		if(m_gameLblUser[4] != -1 && !NoGameID(hdr->type)  && !m_zoom_banner)
			m_btnMgr.show(m_gameLblUser[4]);
		else
			m_btnMgr.hide(m_gameLblUser[4]);
		if(m_show_zone_game && !m_zoom_banner)
		{
			m_btnMgr.hide(m_gameBtnPlayFull);
			m_btnMgr.hide(m_gameBtnBackFull);
			m_btnMgr.hide(m_gameBtnToggleFull);
			m_btnMgr.show(m_gameBtnPlay);
			m_btnMgr.show(m_gameBtnBack);
			m_btnMgr.show(m_gameBtnToggle);
			
			for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 1; ++i)
				if(m_gameLblUser[i] != -1)
					m_btnMgr.show(m_gameLblUser[i]);
					
			m_btnMgr.show(m_gameBtnSettings);
			m_btnMgr.show(m_gameBtnDelete);
			bool b;
			if(hdr->type == TYPE_PLUGIN)
				b = m_gcfg1.getBool("FAVORITES_PLUGINS", id, false);
			else
				b = m_gcfg1.getBool("FAVORITES", id, false);
			m_btnMgr.show(b ? m_gameBtnFavoriteOn : m_gameBtnFavoriteOff);
			m_btnMgr.hide(b ? m_gameBtnFavoriteOff : m_gameBtnFavoriteOn);
			if(hdr->type == TYPE_PLUGIN)
				b = m_gcfg1.getBool("ADULTONLY_PLUGINS", id, false);
			else
				b = m_gcfg1.getBool("ADULTONLY", id, false);
			m_btnMgr.show(b ? m_gameBtnAdultOn : m_gameBtnAdultOff);
			m_btnMgr.hide(b ? m_gameBtnAdultOff : m_gameBtnAdultOn);
		}
		else
		{
			if(m_zoom_banner)
			{
				m_btnMgr.show(m_gameBtnPlayFull);
				m_btnMgr.show(m_gameBtnBackFull);
				m_btnMgr.show(m_gameBtnToggleFull);
			}
			m_btnMgr.hide(m_gameBtnFavoriteOn);
			m_btnMgr.hide(m_gameBtnFavoriteOff);
			m_btnMgr.hide(m_gameBtnAdultOn);
			m_btnMgr.hide(m_gameBtnAdultOff);
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
	if(coverFlipped)
	{
		m_coverflow.setVector3D(domain, key, savedv);
		_loadCFLayout(cf_version, true);
		CoverFlow.applySettings();
	}
	m_gameSelected = false;
	MEM2_free(hdr);
	_hideGame();
}

void CMenu::directlaunch(const char *GameID)// from boot arg for wii game only
{
	m_directLaunch = true;
	for(currentPartition = SD; currentPartition < USB8; currentPartition++)
	{
		if(!DeviceHandle.IsInserted(currentPartition))
			continue;
		DeviceHandle.OpenWBFS(currentPartition);
		string gameDir(fmt(wii_games_dir, DeviceName[currentPartition]));
		string cacheDir(fmt("%s/%s_wii.db", m_listCacheDir.c_str(), DeviceName[currentPartition]));
		m_cacheList.CreateList(COVERFLOW_WII, currentPartition, gameDir,
				stringToVector(".wbfs|.iso", '|'), cacheDir, false);
		WBFS_Close();
		for(u32 i = 0; i < m_cacheList.size(); i++)
		{
			if(strncasecmp(GameID, m_cacheList[i].id, 6) == 0)
			{
				_launchGame(&m_cacheList[i], false); // Launch will exit wiiflow
				break;
			}
		}
	}
	error(wfmt(_fmt("errgame1", L"Cannot find the game with ID: %s"), GameID));
}

void CMenu::_launchShutdown()
{
	CoverFlow.clear();
	_showWaitMessage();
	exitHandler(PRIILOADER_DEF); //Making wiiflow ready to boot something
}

bool gcLaunchFail = false;
void CMenu::_launch(const dir_discHdr *hdr)
{
	dir_discHdr launchHdr;
	memcpy(&launchHdr, hdr, sizeof(dir_discHdr));
	/* Lets boot that shit */
	if(launchHdr.type == TYPE_WII_GAME)
		_launchGame(&launchHdr, false);
	else if(launchHdr.type == TYPE_GC_GAME)
	{
		gcLaunchFail = false;
		_launchGC(&launchHdr, false);
		if(gcLaunchFail) return;
	}
	else if(launchHdr.type == TYPE_CHANNEL || launchHdr.type == TYPE_EMUCHANNEL)
		_launchChannel(&launchHdr);
	else if(launchHdr.type == TYPE_PLUGIN)
	{
		//get dol name and name length for music plugin
		const char *plugin_dol_name = m_plugin.GetDolName(launchHdr.settings[0]);
		u8 plugin_dol_len = strlen(plugin_dol_name);
		//get title and path from hdr
		//example rom path - sd:/roms/super mario bros.zip
		//example scummvm path - kq1-coco3
		char title[101];
		memset(&title, 0, sizeof(title));
		u32 title_len_no_ext = 0;
		const char *path = NULL;
		if(strchr(launchHdr.path, ':') != NULL)//it's a rom path
		{
			//check if music player plugin, if so set wiiflow's bckgrnd music player to play this song
			if(plugin_dol_len == 5 && strcasecmp(plugin_dol_name, "music") == 0)
			{
				MusicPlayer.LoadFile(launchHdr.path, false);
				m_exit = false;
				return;
			}
			//get rom title after last '/'
			strncpy(title, strrchr(launchHdr.path, '/') + 1, 100);
			//if there's extension get length of title without extension
			if(strchr(launchHdr.path, '.') != NULL)
				title_len_no_ext = strlen(title) - strlen(strrchr(title, '.'));
			//get path
			*strrchr(launchHdr.path, '/') = '\0'; //cut title off end of path
			path = strchr(launchHdr.path, '/') + 1; //cut sd:/ off of path
		}
		else //it's a scummvm game
		{
			path = launchHdr.path;// kq1-coco3
			wcstombs(title, launchHdr.title, 63);// King's Quest I: Quest for the Crown (CoCo3/English)
		}
		m_cfg.setString(_domainFromView(), "current_item", title);

		const char *device = (currentPartition == 0 ? "sd" : (DeviceHandle.GetFSType(currentPartition) == PART_FS_NTFS ? "ntfs" : "usb"));
		// I believe the loader is set just in case the user is using a old plugin where the arguments line still requires loader
		const char *loader = fmt("%s:/%s/WiiFlowLoader.dol", device, strchr(m_pluginsDir.c_str(), '/') + 1);

		vector<string> arguments = m_plugin.CreateArgs(device, path, title, loader, title_len_no_ext, launchHdr.settings[0]);
		// find plugin dol - it does not have to be in dev:/wiiflow/plugins
		const char *plugin_file = plugin_dol_name; /* try full path */
		if(strchr(plugin_file, ':') == NULL || !fsop_FileExist(plugin_file)) /* if not found try wiiflow plugin folder */
		{
			plugin_file = fmt("%s/%s", m_pluginsDir.c_str(), plugin_dol_name);
			if(!fsop_FileExist(plugin_file)) /* not found - try device search */
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

void CMenu::_launchGC(dir_discHdr *hdr, bool disc)
{
	const char *id = hdr->id;
	memcpy((u8*)Disc_ID, id, 6);
	DCFlushRange((u8*)Disc_ID, 32);

	const char *path = hdr->path;

	u8 loader = min((u32)m_gcfg2.getInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
	loader = (loader == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "default_loader", 1), ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1u) : loader-1;
	
	if(disc)
		loader = NINTENDONT;
	if((loader == NINTENDONT && !m_nintendont_installed) || (loader == DEVOLUTION && !m_devo_installed))
	{
		error(_t("errgame11", L"GameCube Loader not found! Can't launch game."));
		gcLaunchFail = true;
		return;
	}
	/* GC Loader Found we can go ahead with launchShutdown() */
	/* clear coverflow, start wiiflow wait animation, set exit handler */	
	_launchShutdown();
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));
	m_cfg.setString(_domainFromView(), "current_item", id);

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id);

	u8 GClanguage = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
	GClanguage = (GClanguage == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "game_language", 0), ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1u) : GClanguage-1;
	if(id[3] == 'E' || id[3] == 'J')
		GClanguage = 1; //=english
		
	u8 videoMode = min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_GCvideoModes) - 1u);
	videoMode = (videoMode == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalGCvideoModes) - 1u) : videoMode-1;
	if(disc || videoMode == 0)
	{
		if(id[3] == 'E' || id[3] == 'J')
			videoMode = 2; //NTSC 480i
		else
			videoMode = 1; //PAL 576i
	}

	bool widescreen = m_gcfg2.getBool(id, "widescreen", false);

	if(loader == DEVOLUTION)
	{
		bool memcard_emu = m_gcfg2.testOptBool(id, "devo_memcard_emu", m_cfg.getBool(GC_DOMAIN, "devo_memcard_emu", false));
		bool activity_led = m_gcfg2.getBool(id, "led", false);
		DEVO_GetLoader(m_dataDir.c_str());
		DEVO_SetOptions(path, id, memcard_emu, widescreen, activity_led, m_use_wifi_gecko);
	}
	else if(loader == NINTENDONT)
	{
		u8 emuMC = min((u32)m_gcfg2.getInt(id, "emu_memcard", 0), ARRAY_SIZE(CMenu::_NinEmuCard) - 1u);
		emuMC = (emuMC == 0) ? m_cfg.getInt(GC_DOMAIN, "emu_memcard", 1) : emuMC - 1;
		
		// these 2 settings have global defaults in wfl main config
		bool cc_rumble = m_gcfg2.testOptBool(id, "cc_rumble", m_cfg.getBool(GC_DOMAIN, "cc_rumble", false));
		bool native_ctl = m_gcfg2.testOptBool(id, "native_ctl", m_cfg.getBool(GC_DOMAIN, "native_ctl", false));
		
		bool deflicker = m_gcfg2.getBool(id, "deflicker", false);
		bool tri_arcade = m_gcfg2.getBool(id, "triforce_arcade", false);
		bool activity_led = m_gcfg2.getBool(id, "led", false);
		bool ipl = m_gcfg2.getBool(id, "skip_ipl", false);
		if(IsOnWiiU())
		{
			native_ctl = false;
			activity_led = false;
		}
		if(disc == true)
		{
			/*funny, in order for these settings to work they would have to be entered in gameconfig2 manually under the gameID
			or the game will have to already be on USB or SD but then why launch via disc?*/
			Nintendont_BootDisc(emuMC, widescreen, cc_rumble, native_ctl, deflicker);
		}
		else
		{
			bool NIN_Debugger = (m_gcfg2.getInt(id, "debugger", 0) == 2);
			bool wiiu_widescreen = m_gcfg2.getBool(id, "wiiu_widescreen", false);
			bool cheats = m_gcfg2.getBool(id, "cheat", false);
			/* Generate gct path */
			char GC_Path[256];
			GC_Path[255] = '\0';
			strncpy(GC_Path, path, 255);
			if(strcasestr(path, "boot.bin") != NULL)//games/title [id]/sys/boot.bin
			{
				*strrchr(GC_Path, '/') = '\0'; //erase /boot.bin
				*(strrchr(GC_Path, '/')+1) = '\0'; //erase sys folder
			}
			else //games/title [id]/game.iso
				*(strrchr(GC_Path, '/')+1) = '\0'; //erase game.iso
			char CheatPath[256];// wiiflow cheats path - sd:/wiiflow/cheats/id.gct
			char NewCheatPath[256];// nintendont cheat path - games/title [id]/id.gct
			snprintf(CheatPath, sizeof(CheatPath), "%s/%s", m_cheatDir.c_str(), fmt("%s.gct", id));	
			snprintf(NewCheatPath, sizeof(NewCheatPath), "%s%s.gct",GC_Path,id);
					
			Nintendont_SetOptions(path, id, CheatPath, NewCheatPath, DeviceName[currentPartition],
				cheats, emuMC, videoMode, widescreen, activity_led, native_ctl, deflicker, wiiu_widescreen, 
				NIN_Debugger, tri_arcade, cc_rumble, ipl);
		}
	}
	/* configs no longer needed */
	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	if(loader == NINTENDONT)
	{
		bool ret = (Nintendont_GetLoader() && LoadAppBooter(fmt("%s/app_booter.bin", m_binsDir.c_str())));
		if(ret == false)
		{
			error(_t("errgame14", L"app_booter.bin not found!"));
			_exitWiiflow();
		}
	}
	/* no more error msgs - remove btns and sounds */
	cleanup();
	
 	GC_SetVideoMode(videoMode, loader);
	GC_SetLanguage(GClanguage, loader);
	/* NTSC-J Patch by FIX94 */
	if(id[3] == 'J')
		*HW_PPCSPEED = 0x0002A9E0;

	if(loader == DEVOLUTION)
	{
		if(AHBRPOT_Patched())
			loadIOS(58, false);
		else //use cIOS instead to make sure Devolution works anyways
			loadIOS(mainIOS, false);
		ShutdownBeforeExit();
		DEVO_Boot();
	}
	else
	{
		Nintendont_WriteOptions();
		ShutdownBeforeExit();
		loadIOS(58, false); //nintendont NEEDS ios58
		BootHomebrew(); //regular dol
	}
	Sys_Exit();
}

void CMenu::_launchHomebrew(const char *filepath, vector<string> arguments)
{
	/* clear coverflow, start wiiflow wait animation, set exit handler */	
	_launchShutdown();
	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	Playlog_Delete();

	bool ret = (LoadHomebrew(filepath) && LoadAppBooter(fmt("%s/app_booter.bin", m_binsDir.c_str())));
	if(ret == false)
	{
		error(_t("errgame14", L"app_booter.bin not found!"));
		_exitWiiflow();
	}
	/* no more error msgs - remove btns and sounds */
	cleanup(); // wifi and sd gecko doesnt work anymore after cleanup

	AddBootArgument(filepath);
	for(u32 i = 0; i < arguments.size(); ++i)
	{
		gprintf("Argument: %s\n", arguments[i].c_str());
		AddBootArgument(arguments[i].c_str());
	}

	ShutdownBeforeExit();
	loadIOS(58, false);
	BootHomebrew();
	Sys_Exit();
}

/* dont confuse loadIOS with _loadIOS */
int CMenu::_loadIOS(u8 gameIOS, int userIOS, string id, bool RealNAND_Channels)
{
	gprintf("Game ID# %s requested IOS %d.  User selected %d\n", id.c_str(), gameIOS, userIOS);
	if(neek2o() || (RealNAND_Channels && IOS_GetType(mainIOS) == IOS_TYPE_STUB))
	{
		bool ret = loadIOS(gameIOS, false);
		_netInit();
		if(ret == false)
		{
			error(wfmt(_fmt("errgame4", L"Couldn't load IOS %i"), gameIOS));
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
			error(_t("errgame2", L"No cIOS found!"));
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
			error(wfmt(_fmt("errgame4", L"Couldn't load IOS %i"), gameIOS));
			return LOAD_IOS_FAILED;
		}
		return LOAD_IOS_SUCCEEDED;
	}
	return LOAD_IOS_NOT_NEEDED;
}

void CMenu::_launchChannel(dir_discHdr *hdr)
{
	/* clear coverflow, start wiiflow wait animation, set exit handler */	
	_launchShutdown();
	string id = string(hdr->id);

	/* WII_Launch is used for launching real nand channels */
	bool WII_Launch = (m_gcfg2.getBool(id, "custom", false) && (!NANDemuView || neek2o()));
	/* use_dol = true to use the channels dol or false to use the old apploader method to boot channel */
	bool use_dol = !m_gcfg2.getBool(id, "apploader", false);

	bool vipatch = m_gcfg2.getBool(id, "vipatch", false);
	bool cheat = m_gcfg2.getBool(id, "cheat", false);
	bool countryPatch = m_gcfg2.getBool(id, "country_patch", false);

	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1);
	videoMode = (videoMode == 0) ? (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1) : videoMode - 1;

	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1);
	language = (language == 0) ? min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1) : language;

	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1);
	int aspectRatio = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1) - 1;// -1,0,1
	const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;
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
	m_cfg.setString(_domainFromView(), "current_item", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1); 
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	string emuPath = fmt("/%s/%s", EMU_NANDS_DIR, m_cfg.getString(CHANNEL_DOMAIN, "current_emunand", "default").c_str());
	int emulate_mode = min(max(0, m_cfg.getInt(CHANNEL_DOMAIN, "emulation", 1)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
	
	int userIOS = m_gcfg2.getInt(id, "ios", 0);
	u64 gameTitle = TITLE_ID(hdr->settings[0],hdr->settings[1]);
	bool useNK2o = (m_gcfg2.getBool(id, "useneek", false) && !neek2o());
	bool use_led = m_gcfg2.getBool(id, "led", false);
	u32 gameIOS = ChannelHandle.GetRequestedIOS(gameTitle);

	/* configs no longer needed */
	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	if(NANDemuView && !neek2o())
	{
		if(useNK2o)
		{
			if(!Load_Neek2o_Kernel())
			{
				error(_t("errneek1", L"Cannot launch neek2o. Verify your neek2o setup"));
				_exitWiiflow();
			}
			else 
			{
				cleanup();
				ShutdownBeforeExit();
				Launch_nk(gameTitle, NandHandle.Get_NandPath(), returnTo ? (((u64)(0x00010001) << 32) | (returnTo & 0xFFFFFFFF)) : 0);
				while(1) usleep(500);
			}
		}
	}
	if(WII_Launch == false && ExternalBooter_LoadBins(m_binsDir.c_str()) == false)
	{
		error(_t("errgame15", L"Missing ext_loader.bin or ext_booter.bin!"));
		_exitWiiflow();
	}
	if(_loadIOS(gameIOS, userIOS, id, !NANDemuView) == LOAD_IOS_FAILED)
	{
		/* error message already shown */
		_exitWiiflow();
	}

	if((CurrentIOS.Type == IOS_TYPE_D2X || neek2o()) && returnTo != 0)
	{
		if(D2X_PatchReturnTo(returnTo) >= 0)
			memset(&returnTo, 0, sizeof(u32));
	}
	if(NANDemuView && !neek2o())
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
			_exitWiiflow();
		}
		DeviceHandle.MountAll();
	}
	
	cleanup();//done with error menu can now cleanup
	
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
	/* clear coverflow, start wiiflow wait animation, set exit handler */	
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
			if(Disc_IsGC() < 0) 
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
				if(m_nintendont_installed)
					_launchGC(hdr, true);
				else
					error(_t("errgame12", L"Nintendont not found! Can't launch GC Disc."));
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

	bool vipatch = m_gcfg2.getBool(id, "vipatch", false);
	bool countryPatch = m_gcfg2.getBool(id, "country_patch", false);
	bool private_server = m_gcfg2.getBool(id, "private_server", false);

	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
	videoMode = (videoMode == 0) ? (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1) : videoMode-1;

	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	language = (language == 0) ? min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1) : language;

	const char *rtrn = m_cfg.getString("GENERAL", "returnto", "").c_str();
	int aspectRatio = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u) - 1;
	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);

	u8 emulate_mode = min((u32)m_gcfg2.getInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);
	if(emulate_mode == 0)// default then use global
		emulate_mode = min(max(0, m_cfg.getInt(WII_DOMAIN, "save_emulation", 0)), (int)ARRAY_SIZE(CMenu::_GlobalSaveEmu) - 1);
	else
		emulate_mode--;
		
	string emuPath;
	int emuPartition = 0;
	
	if(emulate_mode && !dvd && !neek2o())
	{
		emuPartition = _FindEmuPart(emuPath, false, true);
		if(emuPartition < 0)//if savepartition and/or nand folder no good
		{
			_hideWaitMessage();
			error(_t("errgame13", L"EmuNAND for gamesave not found! Using real NAND."));
			emulate_mode = 0;
			_showWaitMessage();
		}
		if(emulate_mode == 1)//gamesave
		{
			m_forceext = false;
			_hideWaitMessage();
			m_exit = false;
			/*_AutoExtractSave(id) returns
			true if the save is already on emu nand (then no extraction)
			true if the save was successfully extracted
			false if save is not on real nand (nothing to extract)
			false if user chooses to have game create new save*/
			if(!_AutoExtractSave(id))
				NandHandle.CreateTitleTMD(hdr);//setup emu nand for gamesave
			_showWaitMessage();
			m_exit = true;
		}
		else if(emulate_mode > 1)//region switch or full
		{
			// copy real nand config files to emuNAND
			NandHandle.CreateConfig();
			// do region change on emuNAND even if region switch is not selected
			NandHandle.Do_Region_Change(id, false);
			// region change is always done because if it was changed last time then this time it may need to be set back
			// region switch is done later when D2X cIOS is detected
		}
	}
	else
		emulate_mode = 0;//sets to off we are using neek2o or launching a DVD game

	bool use_led = m_gcfg2.getBool(id, "led", false);
	bool cheat = m_gcfg2.getBool(id, "cheat", false);
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

	m_cfg.setString(_domainFromView(), "current_item", id);
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
	//cleanup(); wifi and sd gecko doesnt work anymore after cleanup

	//this is a temp region change of real nand(rn) for gamesave or off or DVD if tempregionrn is set true
	bool patchregion = false;
	if(emulate_mode <= 1 && !neek2o() && m_cfg.getBool("GENERAL", "tempregionrn", false))
	{
		gprintf("Check\n");
		patchregion = NandHandle.Do_Region_Change(id, true);
	}
	//load external booter bin file
	if(ExternalBooter_LoadBins(m_binsDir.c_str()) == false)
	{
		error(_t("errgame15", L"Missing ext_loader.bin or ext_booter.bin!"));
		_exitWiiflow();
	}
	if((!dvd || neek2o()) && !Sys_DolphinMode())
	{
		if(_loadIOS(gameIOS, userIOS, id) == LOAD_IOS_FAILED)
		{
			/* error message already shown */
			_exitWiiflow();
		}
	}

	if(CurrentIOS.Type == IOS_TYPE_D2X)
	{
		if(returnTo != 0 && !m_directLaunch && D2X_PatchReturnTo(returnTo) >= 0)
			memset(&returnTo, 0, sizeof(u32));
		if(emulate_mode)
		{
			/* Enable our Emu NAND */
			DeviceHandle.UnMountAll();
			if(emulate_mode == 2)
				NandHandle.Set_RCMode(true);
			else if(emulate_mode == 3)
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
	
	/* no more error msgs - clear btns and snds */
	cleanup(); //wifi and sd gecko doesnt work anymore after cleanup

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

void CMenu::_exitWiiflow()
{
	//Exit WiiFlow, no game booted...
	cleanup();// cleanup and clear memory
	ShutdownBeforeExit();// unmount devices and close inputs
	Sys_Exit();
}


void CMenu::_initGameMenu()
{
	//CColor fontColor(0xD0BFDFFF);
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
	m_gameBtnToggle = _addPicButton("GAME/TOOGLE_BTN", texToggleBanner, texToggleBanner, 385, 31, 236, 127);
	m_gameBtnToggleFull = _addPicButton("GAME/TOOGLE_FULL_BTN", texToggleBanner, texToggleBanner, 20, 12, 608, 344);

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
		GC_Disc_Reader.init(GameHdr->path);
		u8 *opening_bnr = GC_Disc_Reader.GetGameCubeBanner();
		if(opening_bnr != NULL)
			m_banner.CreateGCBanner(opening_bnr, m->m_wbf1_font, m->m_wbf2_font, GameHdr->title);
		GC_Disc_Reader.clear();

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
	else if(GameHdr->type == TYPE_CHANNEL || GameHdr->type == TYPE_EMUCHANNEL)
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
			m_zoom_banner = m_banner.ToggleZoom();
			//m_cfg.setBool(_domainFromView(), "show_full_banner", m_zoom_banner);
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
