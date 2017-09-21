
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
bool m_banner_loaded = false;
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
	//{ "SaveRegG", L"Regionswitch" },
	{ "SaveFullG", L"Full" },
};

const CMenu::SOption CMenu::_SaveEmu[4] = {
	{ "SaveDef", L"Default" },
	{ "SaveOff", L"Off" },
	{ "SavePart", L"Game save" },
	//{ "SaveReg", L"Regionswitch" },
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

void CMenu::_setCurrentItem(const dir_discHdr *hdr)
{
	const char *title = CoverFlow.getPathId(hdr, true);// with extension
	m_cfg.setString(_domainFromView(), "current_item", title);
	if(m_source_cnt > 1)
		m_cfg.setInt(_domainFromView(), "current_item_type", hdr->type);
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
	return;
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
	char curId3[4];
	memset(curId3, 0, 4);
	const char *videoId = CoverFlow.getPathId(GameHdr);//title.ext
	if(!NoGameID(GameHdr->type))
	{	//id3
		memcpy(curId3, GameHdr->id, 3);
		videoId = curId3;
	}
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
	bool coverFlipped = false;
	int cf_version = 1;
	string domain;
	string key;
	Vector3D v;
	Vector3D savedv;
	
	dir_discHdr *hdr = (dir_discHdr*)MEM2_alloc(sizeof(dir_discHdr));
	memcpy(hdr, CoverFlow.getHdr(), sizeof(dir_discHdr));
	_setCurrentItem(hdr);
	
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

	if(m_banner.GetZoomSetting() != m_zoom_banner)
		m_banner.ToggleZoom();

	if(m_banner.GetInGameSettings())
		m_banner.ToggleGameSettings();
	m_gameSelected = true;
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
			/* if B on favorites btn goto game categories */
			else if(m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff))
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
					_CategorySettings(true);
					m_banner.ToggleGameSettings();//reset brightness
					m_banner.ToggleZoom();//de zoom to small
				}
				_showGame();
				if(m_newGame)
					startGameSound = -10;
			}
			/* exit game selected menu */
			else
			{
				_cleanupBanner();
				break;
			}
		}
		/* display game info screen */
		else if(BTN_PLUS_PRESSED && !NoGameID(hdr->type) && !coverFlipped && !m_video_playing)
		{
			_hideGame();// stops trailer movie too
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
		else if((BTN_1_PRESSED || BTN_2_PRESSED) && !coverFlipped && !m_video_playing)
		{
			s8 direction = BTN_1_PRESSED ? 1 : -1;
			int cfVersion = loopNum((_getCFVersion() - 1) + direction, m_numCFVersions) + 1;
			_loadCFLayout(cfVersion);
			CoverFlow.applySettings();
			_setCFVersion(cfVersion);
		}
		else if(launch || BTN_A_PRESSED)
		{
			/* delete button */
			if(m_btnMgr.selected(m_gameBtnDelete))
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
				if(m_newGame)
					startGameSound = -10;
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
			else if(m_btnMgr.selected(m_gameBtnAdultOn) || m_btnMgr.selected(m_gameBtnAdultOff))
			{
				if(m_locked)
				{
					m_banner.SetShowBanner(false);
					error(_t("errgame15", L"WiiFlow locked! Unlock WiiFlow to use this feature."));
					m_banner.SetShowBanner(true);
					_showGame();
				}
				else
				{
					if(hdr->type == TYPE_PLUGIN)
						m_gcfg1.setBool("ADULTONLY_PLUGINS", id, !m_gcfg1.getBool("ADULTONLY_PLUGINS", id, false));
					else
						m_gcfg1.setBool("ADULTONLY", id, !m_gcfg1.getBool("ADULTONLY", id, false));
					if(m_locked)
						m_refreshGameList = true;
				}
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
				if(m_newGame)
				{
					m_newGame = false;
					startGameSound = 1;
					_playGameSound();
				}
			}
		}
		
		/* showing and hiding buttons based on banner zoomed state */
		if(!m_zoom_banner)
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
				for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 2; ++i)// #4 is used for banner frame
					if(m_gameLblUser[i] != -1)
						m_btnMgr.show(m_gameLblUser[i]);
			}
			else
			{
				m_btnMgr.hide(m_gameBtnFavoriteOn);
				m_btnMgr.hide(m_gameBtnFavoriteOff);
				m_btnMgr.hide(m_gameBtnAdultOn);
				m_btnMgr.hide(m_gameBtnAdultOff);
				m_btnMgr.hide(m_gameBtnSettings);
				m_btnMgr.hide(m_gameBtnDelete);
				m_btnMgr.hide(m_gameBtnPlay);
				m_btnMgr.hide(m_gameBtnBack);
				for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 2; ++i)
					if (m_gameLblUser[i] != -1)
						m_btnMgr.hide(m_gameLblUser[i]);
			}
			
			/* show or hide small banner toggle btn and frame */
			if(m_banner_loaded && !m_soundThrdBusy && !coverFlipped && !m_video_playing)
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
		else // banner zoomed full screen
		{
			if(m_banner_loaded && !m_soundThrdBusy)// there is a banner
			{
				m_btnMgr.show(m_gameBtnPlayFull);
				m_btnMgr.show(m_gameBtnBackFull);
				m_btnMgr.show(m_gameBtnToggleFull);
				
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
					for(u8 i = 0; i < ARRAY_SIZE(m_gameLblUser) - 2; ++i)// don't include small banner frame
						if(m_gameLblUser[i] != -1)
							m_btnMgr.show(m_gameLblUser[i]);
				}
				else
				{
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
		/* get dol name and name length for music plugin */
		const char *plugin_dol_name = m_plugin.GetDolName(launchHdr.settings[0]);
		u8 plugin_dol_len = strlen(plugin_dol_name);
		/* check if music player plugin, if so set wiiflow's bckgrnd music player to play this song */
		if(plugin_dol_len == 5 && strcasecmp(plugin_dol_name, "music") == 0)
		{
			MusicPlayer.LoadFile(launchHdr.path, false);
			m_exit = false;
			return;
		}
		/* get title from hdr */
		u32 title_len_no_ext = 0;
		const char *title = CoverFlow.getPathId(hdr, true);// with extension
		//m_cfg.setString(_domainFromView(), "current_item", title);
		
		/* get path from hdr */
		// example rom path - dev:/roms/super mario bros.zip
		// example scummvm path - kq1-coco3		
		const char *path = NULL;
		if(strchr(launchHdr.path, ':') != NULL)//it's a rom path
		{
			// if there's extension get length of title without extension
			if(strchr(launchHdr.path, '.') != NULL)
				title_len_no_ext = strlen(title) - strlen(strrchr(title, '.'));
			// get path
			*strrchr(launchHdr.path, '/') = '\0'; //cut title off end of path
			path = strchr(launchHdr.path, '/') + 1; //cut dev:/ off of path
		}
		else // it's a scummvm game
			path = launchHdr.path;// kq1-coco3

		/* get device */
		const char *device = (currentPartition == 0 ? "sd" : (DeviceHandle.GetFSType(currentPartition) == PART_FS_NTFS ? "ntfs" : "usb"));
		
		/* get loader */
		// I believe the loader is set just in case the user is using a old plugin where the arguments line still requires loader
		const char *loader = fmt("%s:/%s/WiiFlowLoader.dol", device, strchr(m_pluginsDir.c_str(), '/') + 1);

		/* set arguments */
		vector<string> arguments = m_plugin.CreateArgs(device, path, title, loader, title_len_no_ext, launchHdr.settings[0]);
		
		/* find plugin dol - it does not have to be in dev:/wiiflow/plugins */
		const char *plugin_file = plugin_dol_name; // try full path
		if(strchr(plugin_file, ':') == NULL || !fsop_FileExist(plugin_file)) // if not found try wiiflow plugin folder
		{
			plugin_file = fmt("%s/%s", m_pluginsDir.c_str(), plugin_dol_name);
			if(!fsop_FileExist(plugin_file)) // not found - try device search
			{
				for(u8 i = SD; i < MAXDEVICES; ++i)
				{
					plugin_file = fmt("%s:/%s", DeviceName[i], plugin_dol_name);
					if(fsop_FileExist(plugin_file))
						break;
				}
			}
		}
		/* launch plugin with args */
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
	/* note for a disc boot hdr->id is set to the disc id before _launchGC is called */
	const char *id = hdr->id;
	
	/* Get loader choice*/
	u8 loader = min((u32)m_gcfg2.getInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
	loader = (loader == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "default_loader", 1), ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1u) : loader-1;
	
	if(disc)
		loader = NINTENDONT;

	/* Check if loader installed */
	if((loader == NINTENDONT && !m_nintendont_installed) || (loader == DEVOLUTION && !m_devo_installed))
	{
		error(_t("errgame11", L"GameCube Loader not found! Can't launch game."));
		gcLaunchFail = true;
		return;
	}
	
	/* clear coverflow, start wiiflow wait animation, set exit handler */	
	_launchShutdown();
	
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id);

	/* Get game settings */
	const char *path = NULL;
	if(disc)
		path = fmt("%s:/", DeviceName[currentPartition]);
	else
		path = hdr->path;
		
	u8 GClanguage = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
	GClanguage = (GClanguage == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "game_language", 0), ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1u) : GClanguage-1;
	// language selection only works for PAL games
	if(id[3] == 'E' || id[3] == 'J')
		GClanguage = 1; //=english
		
	u8 videoMode = min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_GCvideoModes) - 1u);
	videoMode = (videoMode == 0) ? min((u32)m_cfg.getInt(GC_DOMAIN, "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalGCvideoModes) - 1u) : videoMode-1;

	bool widescreen = m_gcfg2.getBool(id, "widescreen", false);
	bool activity_led = m_gcfg2.getBool(id, "led", false);
	
	if(loader == NINTENDONT)
	{
		/* might add here - if not a disc use path to check for disc2.iso - if so then we need to prompt disc 1 or disc 2? */
		u8 emuMC = min((u32)m_gcfg2.getInt(id, "emu_memcard", 0), ARRAY_SIZE(CMenu::_NinEmuCard) - 1u);
		emuMC = (emuMC == 0) ? m_cfg.getInt(GC_DOMAIN, "emu_memcard", 1) : emuMC - 1;
		
		// these 2 settings have global defaults in wfl main config
		bool cc_rumble = m_gcfg2.testOptBool(id, "cc_rumble", m_cfg.getBool(GC_DOMAIN, "cc_rumble", false));
		bool native_ctl = m_gcfg2.testOptBool(id, "native_ctl", m_cfg.getBool(GC_DOMAIN, "native_ctl", false));
		
		bool deflicker = m_gcfg2.getBool(id, "deflicker", false);
		bool tri_arcade = m_gcfg2.getBool(id, "triforce_arcade", false);
		bool ipl = m_gcfg2.getBool(id, "skip_ipl", false);
		bool patch_pal50 = m_gcfg2.getBool(id, "patch_pal50", false);
		bool NIN_Debugger = (m_gcfg2.getInt(id, "debugger", 0) == 2);
		bool wiiu_widescreen = m_gcfg2.getBool(id, "wiiu_widescreen", false);
		bool cheats = m_gcfg2.getBool(id, "cheat", false);
		
		s8 vidscale = m_gcfg2.getInt(id, "nin_width", 127);
		if(vidscale == 127)
			vidscale = m_cfg.getInt(GC_DOMAIN, "nin_width", 0);
		s8 vidoffset = m_gcfg2.getInt(id, "nin_pos", 127);
		if(vidoffset == 127)
			vidoffset = m_cfg.getInt(GC_DOMAIN, "nin_pos", 0);

		/* configs no longer needed */
		m_gcfg1.save(true);
		m_gcfg2.save(true);
		m_cat.save(true);
		m_cfg.save(true);

		bool ret = (Nintendont_GetLoader() && LoadAppBooter(fmt("%s/app_booter.bin", m_binsDir.c_str())));
		if(ret == false)
		{
			error(_t("errgame14", L"app_booter.bin not found!"));
			_exitWiiflow();
		}
		
		/* no more error msgs - remove btns and sounds */
		cleanup();
		
		//GameID for Video mode when booting a Disc
		memcpy((u8*)Disc_ID, id, 6);
		DCFlushRange((u8*)Disc_ID, 32);
		
		/* set nintendont conifg options */
		u32 n_config = 0;
		n_config |= NIN_CFG_AUTO_BOOT;

		if(DeviceHandle.PathToDriveType(path) != SD)
			n_config |= NIN_CFG_USB;

		char CheatPath[256];
		CheatPath[0] = '\0';// set NULL in case cheats are off
		if(cheats)
		{
			n_config |= NIN_CFG_CHEAT_PATH;
			n_config |= NIN_CFG_CHEATS;
			
			/* Generate Game Cheat path - usb1:/games/title [id]/ */
			char GC_Path[256];
			GC_Path[255] = '\0';
			strncpy(GC_Path, path, 255);
			if(strcasestr(path, "boot.bin") != NULL)//usb1:/games/title [id]/sys/boot.bin
			{
				*strrchr(GC_Path, '/') = '\0'; //erase /boot.bin
				*(strrchr(GC_Path, '/')+1) = '\0'; //erase sys folder
			}
			else //usb1:/games/title [id]/game.iso
				*(strrchr(GC_Path, '/')+1) = '\0'; //erase game.iso
			
			//use wiiflow cheat path if on same partition as game
			if(strncasecmp(m_cheatDir.c_str(), DeviceName[currentPartition], strlen(DeviceName[currentPartition])) == 0)
				snprintf(CheatPath, sizeof(CheatPath), "%s/%s", m_cheatDir.c_str(), fmt("%s.gct", id));
			else
			{
				// else copy cheat file to Game Cheat path above
				snprintf(CheatPath, sizeof(CheatPath), "%s%s.gct", GC_Path, id);
				fsop_CopyFile(fmt("%s/%s.gct", m_cheatDir.c_str(), id), CheatPath, NULL, NULL);
				//might add err msg here if copy error
			}
		}

		if(NIN_Debugger && IsOnWiiU() == false) //wii only
			n_config |= NIN_CFG_OSREPORT;

		if(native_ctl && IsOnWiiU() == false) //wii only
			n_config |= NIN_CFG_NATIVE_SI;

		if(wiiu_widescreen && IsOnWiiU() == true) //wii u vwii only
			n_config |= NIN_CFG_WIIU_WIDE;

		if(widescreen)
			n_config |= NIN_CFG_FORCE_WIDE;

		if(activity_led && IsOnWiiU() == false) //wii only
			n_config |= NIN_CFG_LED;

		if(tri_arcade)
			n_config |= NIN_CFG_ARCADE_MODE;

		if(cc_rumble)
			n_config |= NIN_CFG_BIT_CC_RUMBLE;

		if(ipl)
			n_config |= NIN_CFG_SKIP_IPL;
			
		/* set memcard options */
		if(emuMC > 0 || IsOnWiiU() == true) //force memcardemu for wii u vwii
			n_config |= NIN_CFG_MEMCARDEMU;

		if(emuMC > 1)
			n_config |= NIN_CFG_MC_MULTI;
			
		/* set nintendont video options */
		u32 n_videomode = 0;
		
		/* if video is not already forced and patch pal50 or deflicker are on then force to game video mode */
		if(videoMode == 0 && (patch_pal50 || deflicker))
		{
			if(id[3] == 'E' || id[3] == 'J')
				videoMode = 2; //NTSC 480i
			else if (CONF_GetEuRGB60() > 0)
				videoMode = 3; //PAL 480i
			else
				videoMode = 1; //PAL 576i
		}
		
		/* auto or forced */
		if(videoMode == 0)
			n_videomode |= NIN_VID_AUTO;
		else
			n_videomode |= NIN_VID_FORCE;

		// patch_pal50 only works on pal50 games forced to mpal, NTSC, or progressive.
		if(patch_pal50)
			n_videomode |= NIN_VID_PATCH_PAL50;

		// videomode must be forced to set deflicker
		if(deflicker)
			n_videomode |= NIN_VID_FORCE_DF;
			
		// progressive only works with NTSC - Nintendont auto forces to NTSC if progressive on
		if(videoMode == 5)
		{
			n_config |= NIN_CFG_FORCE_PROG;
			n_videomode |= NIN_VID_PROG;
		}

		// rmode and rmode_reg are set by Nintendont
		switch (videoMode)//if 0 nothing is forced
		{
			case 1:// PAL50
				n_videomode |= NIN_VID_FORCE_PAL50;
				break;
			case 2:// PAL60 480i
				n_videomode |= NIN_VID_FORCE_PAL60;
				break;
			case 3:// NTSC 480i
				n_videomode |= NIN_VID_FORCE_NTSC;
				break;
			case 4:// MPAL
				n_videomode |= NIN_VID_FORCE_MPAL;
				break;
		}

		Nintendont_SetOptions(path, id, CheatPath, GClanguage, n_config, n_videomode, vidscale, vidoffset);
		ShutdownBeforeExit();
		loadIOS(58, false); //nintendont NEEDS ios58 and AHBPROT disabled
		/* should be a check for error loading IOS58 and AHBPROT disabled */
		BootHomebrew(); //regular dol
	}
	else // loader == DEVOLUTION
	{
		// devolution does not allow force video and progressive mode
		// igonore video setting choice and use game region always
		if(id[3] =='E' || id[3] =='J')
		{
			// if game is NTSC then video is based on console video
			if(CONF_GetVideo() == CONF_VIDEO_PAL)
				videoMode = 2; //PAL 480i
			else
				videoMode = 3; //NTSC 480i
		}
		else
			videoMode = 1; //PAL 576i 50hz
			
		bool memcard_emu = m_gcfg2.testOptBool(id, "devo_memcard_emu", m_cfg.getBool(GC_DOMAIN, "devo_memcard_emu", false));
		
		/* configs no longer needed */
		m_gcfg1.save(true);
		m_gcfg2.save(true);
		m_cat.save(true);
		m_cfg.save(true);

		/* remove btns and sounds */
		cleanup();

		DEVO_GetLoader(m_dataDir.c_str());
		DEVO_SetOptions(path, id, videoMode, GClanguage, memcard_emu, widescreen, activity_led, m_use_wifi_gecko);
		
		if(AHBPROT_Patched())
			loadIOS(58, false);
		else //use cIOS instead to make sure Devolution works anyways
			loadIOS(mainIOS, false);
		ShutdownBeforeExit();
		DEVO_Boot();
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
	cleanup(); 

	AddBootArgument(filepath);
	for(u32 i = 0; i < arguments.size(); ++i)
	{
		gprintf("Argument: %s\n", arguments[i].c_str());
		AddBootArgument(arguments[i].c_str());
	}

	ShutdownBeforeExit();// wifi and sd gecko doesnt work anymore after
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
		/* doesn't use cIOS so we don't check userIOS */
		bool ret = loadIOS(gameIOS, false);//load game requested IOS and patch nothing
		_netInit();// needed after IOS change
		if(ret == false)
		{
			error(wfmt(_fmt("errgame4", L"Couldn't load IOS %i"), gameIOS));
			return LOAD_IOS_FAILED;
		}
		return LOAD_IOS_SUCCEEDED;
	}	
	
	if(userIOS)// if IOS is not 'auto' and set to a specific cIOS then set gameIOS to that cIOS if it's installed
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
	else if(gameIOS != 57)// if IOS is 'auto' but gameIOS is not IOS57 then set gameIOS to wiiflow's mainIOS
		gameIOS = mainIOS;// mainIOS is usually 249 unless changed by boot args or changed on startup settings menu
	gprintf("Changed requested IOS to %d.\n", gameIOS);

	// remap a gameIOS of IOS57 to a cIOS base 57 or if the specific cIOS selected is not installed then
	// remap game IOS to a CIOS with the same base IOS
	if(gameIOS < 0x64)// < 100
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
		// compare the base of each cios to the game ios
		// if no match then find the first cios with base 56
		// if no match then find the first cios with base 57
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
	/* at this point gameIOS is a cIOS */
	if(gameIOS != CurrentIOS.Version)
	{
		gprintf("Reloading IOS into %d\n", gameIOS);
		bool ret = loadIOS(gameIOS, true);// cIOS patch everything
		_netInit();// always seem to do netinit after changing IOS
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
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1); 
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	//interesting - there is only a global option for nand emulation - no per game choice
	int emulate_mode = min(max(0, m_cfg.getInt(CHANNEL_DOMAIN, "emulation", 1)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
	
	int userIOS = m_gcfg2.getInt(id, "ios", 0);
	u64 gameTitle = TITLE_ID(hdr->settings[0],hdr->settings[1]);
	bool useNK2o = (m_gcfg2.getBool(id, "useneek", false) && !neek2o());//if not in neek2o and use neek is set
	bool use_led = m_gcfg2.getBool(id, "led", false);
	u32 gameIOS = ChannelHandle.GetRequestedIOS(gameTitle);

	if(NANDemuView && !neek2o())
	{
		/* copy real NAND sysconf, settings.txt, & RFL_DB.dat if you want to, they are replaced if they already exist */
		NandHandle.PreNandCfg(m_cfg.getBool(CHANNEL_DOMAIN, "real_nand_miis", false), 
								m_cfg.getBool(CHANNEL_DOMAIN, "real_nand_config", false));
		m_cfg.setBool(CHANNEL_DOMAIN, "real_nand_miis", false); 
		m_cfg.setBool(CHANNEL_DOMAIN, "real_nand_config", false);
	}

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
				error(_t("errneek1", L"Cannot launch neek2o. Verify your neek2o setup"));//kernal.bin not found
				_exitWiiflow();
			}
			else 
			{
				const char *nkrtrn = NULL;
				if(IsOnWiiU())
					nkrtrn = "HCVA";// Return to WiiU system channel
				else
					nkrtrn = "NK2O";
				u32 nkreturnTo = nkrtrn[0] << 24 | nkrtrn[1] << 16 | nkrtrn[2] << 8 | nkrtrn[3];
				cleanup();
				ShutdownBeforeExit();
				if(IsOnWiiU())
					Launch_nk(gameTitle, NandHandle.Get_NandPath(), ((u64)(0x00010002) << 32) | (nkreturnTo & 0xFFFFFFFF));
				else
					Launch_nk(gameTitle, NandHandle.Get_NandPath(), ((u64)(0x00010001) << 32) | (nkreturnTo & 0xFFFFFFFF));
				while(1) usleep(500);
			}
		}
	}
	if(WII_Launch == false && ExternalBooter_LoadBins(m_binsDir.c_str()) == false)
	{
		error(_t("errgame15", L"Missing ext_loader.bin or ext_booter.bin!"));
		_exitWiiflow();
	}
	if(_loadIOS(gameIOS, userIOS, id, !NANDemuView) == LOAD_IOS_FAILED)//in neek2o this will only load the game IOS not a cIOS
	{
		/* error message already shown */
		_exitWiiflow();
	}

	if((CurrentIOS.Type == IOS_TYPE_D2X || neek2o()) && returnTo != 0)
	{
		if(D2X_PatchReturnTo(returnTo) >= 0)
			memset(&returnTo, 0, sizeof(u32));// not needed - always set to 0 in external booter below
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
		NandHandle.Patch_AHB(); /* Identify maybe uses it so keep AHBPROT disabled */
		PatchIOS(true); /* Patch for everything */
		Identify(gameTitle);

		ExternalBooter_ChannelSetup(gameTitle, use_dol);
		WiiFlow_ExternalBooter(videoMode, vipatch, countryPatch, patchVidMode, aspectRatio, 0, TYPE_CHANNEL, use_led);
	}
	Sys_Exit();
}

void CMenu::_launchGame(dir_discHdr *hdr, bool dvd, bool disc_cfg)
{
	string id(hdr->id);
	string path(hdr->path);
	if(neek2o())
	{
		int discID = id.c_str()[0] << 24 | id.c_str()[1] << 16 | id.c_str()[2] << 8 | id.c_str()[3];
		if(WDVD_NEEK_LoadDisc((discID&0xFFFFFFFF), 0x5D1C9EA3) > 0)//5D1C9EA3 is wii disc magic
		{
			dvd = true;
			sleep(3);
		}
	}

	if(dvd)
	{
		TempLoadIOS();// switch to cIOS if using IOS58 and not in neek2o
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
				hdr->type = TYPE_GC_GAME;
				
				/* Launching GC Game */
				if(m_nintendont_installed)
				{
					if(disc_cfg)
						_gameSettings(hdr, dvd);
					MusicPlayer.Stop();
					m_cfg.setInt("GENERAL", "cat_startpage", m_catStartPage);
					if(!disc_cfg)
						m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
					currentPartition = m_cfg.getInt(GC_DOMAIN, "partition", 1);
					_launchGC(hdr, dvd);
				}
				else
					error(_t("errgame12", L"Nintendont not found! Can't launch GC Disc."));
				return;
			}
		}
		else
		{
			/* Read header */
			Disc_ReadHeader(&wii_hdr);
			memcpy(hdr->id, wii_hdr.id, 6);
			id = string((const char*)wii_hdr.id, 6);
			hdr->type = TYPE_WII_GAME;
			if(disc_cfg)
				_gameSettings(hdr, dvd);
			MusicPlayer.Stop();
			m_cfg.setInt("GENERAL", "cat_startpage", m_catStartPage);
			if(!disc_cfg)
				m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
			currentPartition = m_cfg.getInt(WII_DOMAIN, "partition", 1);
		}
		gprintf("Game ID: %s\n", id.c_str());
	}
	
	/* clear coverflow, start wiiflow wait animation, set exit handler */	
	_launchShutdown();
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
	u8 gameEmuMode = emulate_mode;
	if(emulate_mode == 0)// default then use global
		emulate_mode = min(max(0, m_cfg.getInt(WII_DOMAIN, "save_emulation", 0)), (int)ARRAY_SIZE(CMenu::_GlobalSaveEmu) - 1);
	else
		emulate_mode--;
		
	if(emulate_mode && !dvd && !neek2o())
	{
		string emuPath;
		int emuPart = _FindEmuPart(emuPath, true, true);
		if(emuPart == -1)//if savepartition is unusable
		{
			_hideWaitMessage();
			error(_t("errgame13", L"EmuNAND for gamesave not found! Using real NAND."));
			emulate_mode = 0;
			_showWaitMessage();
		}
		else
		{
			bool need_config = false;
			bool need_miis = false;
			
			char testpath[MAX_FAT_PATH];
			char basepath[MAX_FAT_PATH];
			snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPart], emuPath.c_str());
			
			// does not check to see if actual tmd exist just if the folder exist
			if(!_checkSave(id, false))//if save is not on emunand
			{
				NandHandle.CreatePath("%s/import", basepath);
				NandHandle.CreatePath("%s/meta", basepath);
				NandHandle.CreatePath("%s/shared1", basepath);
				NandHandle.CreatePath("%s/shared2", basepath);
				NandHandle.CreatePath("%s/sys", basepath);
				NandHandle.CreatePath("%s/title", basepath);
				NandHandle.CreatePath("%s/ticket", basepath);
				NandHandle.CreatePath("%s/tmp", basepath);
				NandHandle.CreateTitleTMD(hdr);//setup emunand for wii gamesave
			}
			if(gameEmuMode == 3)//full per game setting - in case global is not full
			{
				//check config files
				snprintf(testpath, sizeof(testpath), "%s/shared2/sys/SYSCONF", basepath);
				if(!fsop_FileExist(testpath))
					need_config = true;
				snprintf(testpath, sizeof(testpath), "%s/title/00000001/00000002/data/setting.txt", basepath);
				if(!fsop_FileExist(testpath))
					need_config = true;
				// Check Mii's
				snprintf(testpath, sizeof(testpath), "%s/shared2/menu/FaceLib/RFL_DB.dat", basepath);
				if(!fsop_FileExist(testpath))
					need_miis = true;
					
				NandHandle.PreNandCfg(need_miis, need_config);//copy to emunand if needed
			}
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

	//this is a temp region change of real nand(rn) for gamesave or off or DVD if tempregionrn is set true
	bool patchregion = false;
	if(emulate_mode <= 1 && !neek2o() && m_cfg.getBool("GENERAL", "tempregionrn", false))
	{
		gprintf("Temp region change applied\n");
		// change real nand region to game ID[3] region. is reset when you turn wii off.
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
			memset(&returnTo, 0, sizeof(u32));//set to null to keep external booter from setting it again if using d2x 
		if(emulate_mode)
		{
			/* Enable our Emu NAND */
			DeviceHandle.UnMountAll();
			if(emulate_mode == 2)
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
	cleanup();

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

// loads game banner and sound to be played by mainloop
void CMenu::_gameSoundThread(CMenu *m)
{
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
	
	m_banner_loaded = false;
	const dir_discHdr *GameHdr = CoverFlow.getHdr();	
	if(GameHdr->type == TYPE_PLUGIN)
	{
		const char *coverDir  = NULL;
		coverDir = m_plugin.GetCoverFolderName(GameHdr->settings[0]);
		
		if(coverDir == NULL || strlen(coverDir) == 0)
			strncpy(custom_banner, fmt("%s/%s.bnr", m->m_customBnrDir.c_str(), CoverFlow.getPathId(GameHdr)), 255);
		else
			strncpy(custom_banner, fmt("%s/%s/%s.bnr", m->m_customBnrDir.c_str(), coverDir, CoverFlow.getPathId(GameHdr)), 255);
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
		else
		{
			m_banner.DeleteBanner();
			m->m_gameSound.Load(m_plugin.GetBannerSound(GameHdr->settings[0]), m_plugin.GetBannerSoundSize());
			if(m->m_gameSound.IsLoaded())
				m->m_gamesound_changed = true;
			m->m_soundThrdBusy = false;
			return;
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
		//get wiiflow gc ogg sound to play with banner
		m->m_gameSound.Load(gc_ogg, gc_ogg_size, false);
		if(m->m_gameSound.IsLoaded())
			m->m_gamesound_changed = true;
		m->m_soundThrdBusy = false;
		return;
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
		_extractBnr(GameHdr);
		m_banner_loaded = true;
	}
	else if(GameHdr->type == TYPE_CHANNEL || GameHdr->type == TYPE_EMUCHANNEL)
	{
		_extractChannelBnr(TITLE_ID(GameHdr->settings[0], GameHdr->settings[1]));
		m_banner_loaded = true;
	}
		
	if(!CurrentBanner.IsValid())
	{
		m_banner_loaded = false;
		m->m_gameSound.FreeMemory();
		m_banner.DeleteBanner();
		CurrentBanner.ClearBanner();
		m->m_soundThrdBusy = false;
		return;
	}
	//save new wii or channel banner to cache folder, gc and custom banners are not cached
	if(cached_bnr_file == NULL && custom_bnr_file == NULL)
		fsop_WriteFile(cached_banner, CurrentBanner.GetBannerFile(), CurrentBanner.GetBannerFileSize());

	//load and init banner
	m_banner.LoadBanner(m->m_wbf1_font, m->m_wbf2_font);
	
	//get sound from wii, channel, or custom banner and load it to play with the banner
	u32 sndSize = 0;
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
void CMenu::_playGameSound(void)// starts banner and gamesound loading thread
{
	_cleanupBanner(true);
	m_gamesound_changed = false;
	if(m_bnrSndVol == 0) 
		return;

	if(m_gameSoundThread != LWP_THREAD_NULL)
		_stopGameSoundThread();
	GameSoundStack = (u8*)MEM2_lo_alloc(GameSoundSize);
	LWP_CreateThread(&m_gameSoundThread, (void *(*)(void *))CMenu::_gameSoundThread, (void*)this, GameSoundStack, GameSoundSize, 60);
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
