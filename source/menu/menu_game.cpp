
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <unistd.h>
#include <time.h>
#include <network.h>
#include <errno.h>

#include "menu.hpp"
#include "types.h"
#include "banner/BannerWindow.hpp"
#include "channel/channel_launcher.h"
#include "channel/channels.h"
#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "devicemounter/sdhc.h"
#include "devicemounter/usbstorage.h"
#include "fileOps/fileOps.h"
#include "gc/gc.hpp"
#include "gc/gcdisc.hpp"
#include "gecko/gecko.h"
#include "gui/WiiMovie.hpp"
#include "gui/GameTDB.hpp"
#include "gui/Gekko.h"
#include "homebrew/homebrew.h"
#include "loader/alt_ios.h"
#include "loader/external_booter.hpp"
#include "loader/sys.h"
#include "loader/wdvd.h"
#include "loader/alt_ios.h"
#include "loader/playlog.h"
#include "loader/wbfs.h"
#include "loader/wip.h"
#include "loader/frag.h"
#include "loader/fst.h"
#include "loader/cios.h"
#include "loader/nk.h"
#include "network/http.h"
#include "network/gcard.h"

extern const u8 btngamecfg_png[];
extern const u8 btngamecfgs_png[];
extern const u8 stopkidon_png[];
extern const u8 stopkidons_png[];
extern const u8 stopkidoff_png[];
extern const u8 stopkidoffs_png[];
extern const u8 favoriteson_png[];
extern const u8 favoritesons_png[];
extern const u8 favoritesoff_png[];
extern const u8 favoritesoffs_png[];
extern const u8 delete_png[];
extern const u8 deletes_png[];
extern const u8 blank_png[];

//sounds
extern const u8 gc_ogg[];
extern const u32 gc_ogg_size;

extern u32 boot2version;
static u64 sm_title_id[8]  ATTRIBUTE_ALIGN(32);

bool m_zoom_banner = false;
s16 m_gameBtnPlayFull;
s16 m_gameBtnBackFull;
s16 m_gameBtnToogle;
s16 m_gameBtnToogleFull;

const string CMenu::_translations[23] = {
	"Default",
	"Arab",
	"Brazilian",
	"Chinese_S",
	"Chinese_T",
	"Danish",
	"Dutch",
	"English",
	"Finnish",
	"French",
	"Gallego",
	"German",
	"Hungarian",
	"Italian",
	"Japanese",
	"Norwegian",
	"Polish",
	"Portuguese",
	"Russian",
	"Spanish",
	"Swedish",
	"Tagalog",
	"Turkish"
};

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

const CMenu::SOption CMenu::_GlobalDMLvideoModes[6] = {
	{ "DMLdefG", L"Game" },
	{ "DMLpal", L"PAL 576i" },
	{ "DMLntsc", L"NTSC 480i" },
	{ "DMLpal60", L"PAL 480i" },
	{ "DMLprog", L"NTSC 480p" },
	{ "DMLprogP", L"PAL 480p" }
};

const CMenu::SOption CMenu::_DMLvideoModes[7] = {
	{ "DMLdef", L"Default" },
	{ "DMLdefG", L"Game" },
	{ "DMLpal", L"PAL 576i" },
	{ "DMLntsc", L"NTSC 480i" },
	{ "DMLpal60", L"PAL 480i" },
	{ "DMLprog", L"NTSC 480p" },
	{ "DMLprogP", L"PAL 480p" }
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

const CMenu::SOption CMenu::_NoDVD[3] = {
	{ "NoDVDDef", L"Default" },
	{ "NoDVDOff", L"Disabled" },
	{ "NoDVDon", L"Enabled" },
};

const CMenu::SOption CMenu::_GCLoader[3] = {
	{ "GC_Def", L"Default" },
	{ "GC_DM", L"DIOS-MIOS" },
	{ "GC_Devo", L"Devolution" },
};

const CMenu::SOption CMenu::_vidModePatch[4] = {
	{ "vmpnone", L"None" },
	{ "vmpnormal", L"Normal" },
	{ "vmpmore", L"More" },
	{ "vmpall", L"All" }
};

const CMenu::SOption CMenu::_hooktype[8] = {
	{ "disabled", L"Disabled" },
	{ "hooktype1", L"VBI" },
	{ "hooktype2", L"KPAD read" },
	{ "hooktype3", L"Joypad" },
	{ "hooktype4", L"GXDraw" },
	{ "hooktype5", L"GXFlush" },
	{ "hooktype6", L"OSSleepThread" },
	{ "hooktype7", L"AXNextFrame" },
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


static void _extractBannerTitle(Banner *bnr, int language)
{
	if (bnr != NULL)
	{
		memset(banner_title, 0, 84);
		bnr->GetName(banner_title, language);
	}
}

static Banner *_extractChannelBnr(const u64 chantitle)
{
	return Channels::GetBanner(chantitle);
}

static Banner *_extractBnr(dir_discHdr *hdr)
{
	u32 size = 0;
	Banner *banner = NULL;
	DeviceHandle.OpenWBFS(currentPartition);
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->id, (char *) hdr->path);
	if(disc != NULL)
	{
		void *bnr = NULL;
		size = wbfs_extract_file(disc, (char *) "opening.bnr", &bnr);
		if(size > 0)
			banner = new Banner((u8 *)bnr, size);
		WBFS_CloseDisc(disc);
	}
	WBFS_Close();
	return banner;
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
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->id, (char *) hdr->path);
	if(disc != NULL)
	{
		u8 *titleTMD = NULL;
		u32 tmd_size = wbfs_extract_file(disc, (char *) "TMD", (void **)&titleTMD);
		if(titleTMD != NULL && tmd_size > 0x18B)
			IOS = titleTMD[0x18B];
		WBFS_CloseDisc(disc);
	}
	WBFS_Close();
	return IOS;
}

void CMenu::_hideGame(bool instant)
{
	m_gameSelected = false;
	m_fa.unload();
	m_cf.showCover();
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
	m_cf.showCover();
	
	if (m_fa.load(m_cfg, m_fanartDir.c_str(), m_cf.getId().c_str()))
	{
		STexture bg, bglq;
		m_fa.getBackground(bg, bglq);
		_setBg(bg, bglq);

		if (m_fa.hideCover())
			m_cf.hideCover();
	}
	else
		_setBg(m_mainBg, m_mainBgLQ);

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

void CMenu::_game(bool launch)
{
	m_gcfg1.load(fmt("%s/" GAME_SETTINGS1_FILENAME, m_settingsDir.c_str()));
	if(!launch)
	{
		SetupInput();
		_playGameSound();
		_showGame();
		m_gameSelected = true;
	}

	m_zoom_banner = m_cfg.getBool(_domainFromView(), "show_full_banner", false) && !NoGameID(m_cf.getHdr()->type);
	if(m_banner->GetZoomSetting() != m_zoom_banner)
		m_banner->ToogleZoom();

	s8 startGameSound = 1;
	while(!m_exit)
	{
		if(startGameSound < 1)
			startGameSound++;

		u64 chantitle = m_cf.getChanTitle();

		if(startGameSound == -5)
		{
			_playGameSound();
			_showGame();
		}
		_mainLoopCommon(true);

		if(startGameSound == 0)
		{
			m_gameSelected = true;
			startGameSound = 1;
		}
		if(BTN_B_PRESSED && (m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff)))
		{
			_hideGame();
			m_banner->SetShowBanner(false);
			_CategorySettings(true);
			_showGame();
			m_banner->SetShowBanner(true);
			if(!m_gameSound.IsPlaying()) 
				startGameSound = -6;
			continue;
		}
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			m_gameSound.FreeMemory();
			CheckGameSoundThread();
			ClearGameSoundThreadStack();
			m_banner->DeleteBanner();
			break;
		}
		else if(BTN_PLUS_PRESSED && m_GameTDBLoaded && (m_cf.getHdr()->type == TYPE_WII_GAME || m_cf.getHdr()->type == TYPE_GC_GAME || m_cf.getHdr()->type == TYPE_CHANNEL))
		{
			_hideGame();
			m_banner->SetShowBanner(false);
			m_gameSelected = true;
			_gameinfo();
			_showGame();
			m_banner->SetShowBanner(true);
			if(!m_gameSound.IsPlaying())
				startGameSound = -6;
		}
		else if(BTN_MINUS_PRESSED)
		{
			const char *videoPath = fmt("%s/%.3s.thp", m_videoDir.c_str(), m_cf.getId().c_str());

			FILE *file = fopen(videoPath, "r");
			if(file)
			{
				MusicPlayer.Stop();
				m_gameSound.Stop();
				m_banner->SetShowBanner(false);
				fclose(file);
				_hideGame();
				/* Backup Background */
				STexture Current_LQ_BG = m_lqBg;
				STexture Current_HQ_BG = m_curBg;
				STexture EmptyBG;
				_setBg(EmptyBG, EmptyBG);
				/* Lets play the movie */
				WiiMovie movie(videoPath);
				movie.SetScreenSize(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
				movie.SetVolume(m_cfg.getInt("GENERAL", "sound_volume_bnr", 255));
				m_video_playing = true;
				movie.Play();
				m_banner->ReSetup_GX();
				m_vid.setup2DProjection();
				while(!BTN_B_PRESSED && !BTN_A_PRESSED && !BTN_HOME_PRESSED && movie.GetNextFrame(&m_curBg))
				{
					/* Draw movie BG and render */
					_drawBg();
					m_vid.render();
					m_curBg.data.release();
					/* Check if we want to stop */
					WPAD_ScanPads();
					PAD_ScanPads();
					ButtonsPressed();
				}
				movie.Stop();
				m_curBg.data.release();
				/* Finished, so lets re-setup the background */
				_setBg(Current_HQ_BG, Current_LQ_BG);
				_updateBg();
				/* Get back into our coverflow */
				_showGame();
				m_video_playing = false;
				m_banner->SetShowBanner(true);
				if(!m_gameSound.IsPlaying())
					startGameSound = -6;
			}
		}
		else if((BTN_1_PRESSED) || (BTN_2_PRESSED))
		{
			s8 direction = BTN_1_PRESSED ? 1 : -1;
			const char *domain = _domainFromView();
			int cfVersion = 1+loopNum((m_cfg.getInt(domain, "last_cf_mode", 1)-1) + direction, m_numCFVersions);
			_loadCFLayout(cfVersion);
			m_cf.applySettings();
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
					m_banner->SetShowBanner(false);
					if(_wbfsOp(CMenu::WO_REMOVE_GAME))
					{
						m_gameSound.FreeMemory();
						CheckGameSoundThread();
						ClearGameSoundThreadStack();
						m_banner->DeleteBanner();
						break;
					}
					m_banner->SetShowBanner(true);
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
				m_gameSound.FreeMemory();
				CheckGameSoundThread();
				ClearGameSoundThreadStack();
				m_banner->DeleteBanner();
				break;
			}
			else if((m_btnMgr.selected(m_gameBtnToogle) || m_btnMgr.selected(m_gameBtnToogleFull)) && !NoGameID(m_cf.getHdr()->type))
			{
				m_zoom_banner = m_banner->ToogleZoom();
				m_cfg.setBool(_domainFromView(), "show_full_banner", m_zoom_banner);
				m_show_zone_game = false;
			}
			else if(m_btnMgr.selected(m_gameBtnSettings))
			{
				_hideGame();
				m_gameSelected = true;

				m_banner->ToogleGameSettings();
				_gameSettings();
				m_banner->ToogleGameSettings();

				_showGame();
				if(!m_gameSound.IsPlaying()) 
					startGameSound = -6;
			}
			else if(launch || m_btnMgr.selected(m_gameBtnPlay) || m_btnMgr.selected(m_gameBtnPlayFull) || !ShowPointer())
			{
				_hideGame();
				MusicPlayer.Stop();
				m_gameSound.FreeMemory();
				CheckGameSoundThread();
				ClearGameSoundThreadStack();
				m_banner->DeleteBanner();
				dir_discHdr *hdr = m_cf.getHdr();
				m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
				if(currentPartition != SD && hdr->type == TYPE_GC_GAME && m_show_dml == 2 && (strstr(hdr->path, ".iso") == NULL ||
				!m_devo_installed || min((u32)m_gcfg2.getInt(hdr->id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u) == 1))
				{
					bool foundOnSD = false;
					CList<dir_discHdr> tmplist;
					vector<string> pathlist;
					tmplist.GetPaths(pathlist, ".iso|.bin", "sd:/games", false, true);
					vector<dir_discHdr> tmpGameList;
					Config nullCfg;
					tmplist.GetHeaders(pathlist, tmpGameList, m_settingsDir, m_curLanguage, m_DMLgameDir, nullCfg);
					for(u8 i = 0; i < tmpGameList.size(); i++)
					{
						if(strncasecmp(tmpGameList.at(i).id, hdr->id, 6) == 0)
						{
							foundOnSD = true;
							memset(hdr->path, 0, sizeof(hdr->path));
							strncpy(hdr->path, tmpGameList.at(i).path, sizeof(hdr->path));
							break;
						}
					}
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
							break;
					}
					currentPartition = SD;
				}

				m_cf.clear();
				_showWaitMessage();
				exitHandler(PRIILOADER_DEF); //Making wiiflow ready to boot something

				if(hdr->type != TYPE_HOMEBREW && hdr->type != TYPE_PLUGIN)
				{
					// Get banner_title
					Banner *banner = hdr->type == TYPE_CHANNEL ? _extractChannelBnr(chantitle) : (hdr->type == TYPE_WII_GAME ? _extractBnr(hdr) : NULL);
					if(banner != NULL)
					{
						if(banner->IsValid())
							_extractBannerTitle(banner, GetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str()));
						delete banner;
					}
					banner = NULL;

					if(Playlog_Update(m_cf.getId().c_str(), banner_title) < 0)
						Playlog_Delete();
				}

				gprintf("Launching game %s\n", m_cf.getId().c_str());
				_launch(hdr);

				if(m_exit)
					break;

				_hideWaitMessage();
				launch = false;

				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
				m_gcfg2.unload();
				_showGame();
				_initCF();
				m_cf.select();
			}
			else 
			{
				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					if (m_cf.mouseOver(m_vid, m_cursor[chan].x(), m_cursor[chan].y()))
						m_cf.flip();
			}
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_UP_REPEAT || RIGHT_STICK_UP))
		{
			if(m_gameSoundThread != LWP_THREAD_NULL)
				CheckGameSoundThread();
			m_cf.up();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT))
		{
			if(m_gameSoundThread != LWP_THREAD_NULL)
				CheckGameSoundThread();
			m_cf.right();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_DOWN_REPEAT || RIGHT_STICK_DOWN))
		{
			if(m_gameSoundThread != LWP_THREAD_NULL)
				CheckGameSoundThread();
			m_cf.down();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_LEFT_REPEAT || RIGHT_STICK_LEFT))
		{
			if(m_gameSoundThread != LWP_THREAD_NULL)
				CheckGameSoundThread();
			m_cf.left();
			startGameSound = -10;
		}
		if(startGameSound == -10)
		{
			m_gameSound.Stop();
			m_gameSelected = false;
			m_fa.unload();
			m_banner->DeleteBanner(true);
			_setBg(m_mainBg, m_mainBgLQ);
		}
		if(m_show_zone_game && !m_zoom_banner)
		{
			bool b = m_gcfg1.getBool("FAVORITES", _getId(), false);
			m_btnMgr.show(b ? m_gameBtnFavoriteOn : m_gameBtnFavoriteOff);
			m_btnMgr.hide(b ? m_gameBtnFavoriteOff : m_gameBtnFavoriteOn);
			m_btnMgr.show(m_gameBtnPlay);
			m_btnMgr.show(m_gameBtnBack);
			m_btnMgr.show(m_gameBtnToogle);
			m_btnMgr.hide(m_gameBtnPlayFull);
			m_btnMgr.hide(m_gameBtnBackFull);
			m_btnMgr.hide(m_gameBtnToogleFull);
			if(m_gameLblUser[4] != -1 && !NoGameID(m_cf.getHdr()->type))
				m_btnMgr.show(m_gameLblUser[4]);
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
			if ((m_cf.getHdr()->type != TYPE_HOMEBREW && m_cf.getHdr()->type != TYPE_CHANNEL) && !m_locked)
				m_btnMgr.show(m_gameBtnDelete);
		}
		else
		{
			if(m_zoom_banner)
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
				if(!NoGameID(m_cf.getHdr()->type) && !m_zoom_banner)
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

void CMenu::directlaunch(const string &id)
{
	m_directLaunch = true;

	for (int i = USB1; i < USB8; i++)
	{
		if(!DeviceHandle.IsInserted(i)) continue;

		DeviceHandle.OpenWBFS(i);
		CList<dir_discHdr> list;
		string path = sfmt(GAMES_DIR, DeviceName[i]);
		vector<string> pathlist;
		list.GetPaths(pathlist, id.c_str(), path, strncasecmp(DeviceHandle.PathToFSName(path.c_str()), "WBFS", 4) == 0);
		m_gameList.clear();
		Config nullCfg;
		list.GetHeaders(pathlist, m_gameList, m_settingsDir, m_curLanguage, m_DMLgameDir, nullCfg);
		if(m_gameList.size() > 0)
		{
			gprintf("Game found on partition #%i\n", i);
			_launch(&m_gameList[0]); // Launch will exit wiiflow
		}
		WBFS_Close();
	}
	error(sfmt("errgame1", L"Cannot find the game with ID: %s", id.c_str()));
}

void CMenu::_launch(dir_discHdr *hdr)
{
	/* No need to do that separate */
	Nand::Instance()->Disable_Emu();
	/* Lets boot that shit */
	if(hdr->type == TYPE_WII_GAME)
		_launchGame(hdr, false);
	else if(hdr->type == TYPE_GC_GAME)
		_launchGC(hdr, false);
	else if(hdr->type == TYPE_CHANNEL)
		_launchChannel(hdr);
	else if(hdr->type == TYPE_PLUGIN)
	{
		string title(&hdr->path[string(hdr->path).find_last_of("/")+1]);
		string wiiflow_dol(m_dol);
		if(strstr(wiiflow_dol.c_str(), "sd:/") == NULL)
			wiiflow_dol.erase(3,1);
		string path((char*)hdr->path, size_t(strlen((char*)hdr->path) - title.size()));
		vector<string> arguments;
		gprintf("Game title: %s\n", title.c_str());
		if(m_plugin.isMplayerCE(hdr->settings[0]))
			arguments = m_plugin.CreateMplayerCEArguments(string(hdr->path).c_str());
		else if(strstr(path.c_str(), ":/") != NULL)
		{
			if(strstr(path.c_str(), "sd:/") == NULL)
				path.erase(3,1);
			arguments.push_back(path);
			arguments.push_back(title);
			if(m_plugin.UseReturnLoader(hdr->settings[0]))
				arguments.push_back(sfmt("%s/WiiFlowLoader.dol",m_pluginsDir.c_str()));
			else
				arguments.push_back(wiiflow_dol);
			m_cfg.setString("EMULATOR", "current_item", title);
		}
		else
		{
			arguments.push_back(title);
			char gametitle[64];
			wcstombs(gametitle, hdr->title, sizeof(gametitle));
			m_cfg.setString("EMULATOR", "current_item", gametitle);
		}
		_launchHomebrew(fmt("%s/%s", m_pluginsDir.c_str(), m_plugin.GetDolName(hdr->settings[0])), arguments);
	}
	else if(hdr->type == TYPE_HOMEBREW)
	{
		string title(&hdr->path[string(hdr->path).find_last_of("/")+1]);
		char gamepath[128];
		snprintf(gamepath, sizeof(gamepath), "%s/boot.dol", hdr->path);
		if(!fsop_FileExist((const char*)gamepath))
			snprintf(gamepath, sizeof(gamepath), "%s/boot.elf", hdr->path);
		m_cfg.setString("HOMEBREW", "current_item", title);
		_launchHomebrew(gamepath, m_homebrewArgs);
	}
}

void CMenu::_launchGC(dir_discHdr *hdr, bool disc)
{
	string id(hdr->id);
	string path(hdr->path);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	u8 videoSetting = min(m_cfg.getInt("DML", "video_setting", 1), 2);

	u8 GClanguage = min((u32)m_gcfg2.getInt(id, "gc_language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
	GClanguage = (GClanguage == 0) ? min((u32)m_cfg.getInt("DML", "game_language", 0), ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1u) : GClanguage-1;

	u8 videoMode = min((u32)m_gcfg2.getInt(id, "dml_video_mode", 0), ARRAY_SIZE(CMenu::_DMLvideoModes) - 1u);
	videoMode = (videoMode == 0) ? min((u32)m_cfg.getInt("DML", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalDMLvideoModes) - 1u) : videoMode-1;
	if(videoMode == 0)
	{
		if(id.c_str()[3] == 'E' || id.c_str()[3] == 'J')
			videoMode = 2; //NTSC 480i
		else
			videoMode = 1; //PAL 576i
	}

	u8 loader = min((u32)m_gcfg2.getInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
	bool memcard_emu = m_gcfg2.getBool(id, "devo_memcard_emu", false);

	if(disc)
	{
		loader = 0;
		DML_New_SetBootDiscOption(m_new_dm_cfg);
	}
	else if(loader == 1 || (loader == 0 && (strcasestr(path.c_str(), "boot.bin") != NULL || !m_devo_installed)))
	{
		loader = 1;
		m_cfg.setString("DML", "current_item", id);
		char CheatPath[256];
		u8 NMM = min((u32)m_gcfg2.getInt(id, "dml_nmm", 0), ARRAY_SIZE(CMenu::_NMM) - 1u);
		NMM = (NMM == 0) ? m_cfg.getInt("DML", "dml_nmm", 0) : NMM-1;
		u8 nodisc = min((u32)m_gcfg2.getInt(id, "no_disc_patch", 0), ARRAY_SIZE(CMenu::_NoDVD) - 1u);
		nodisc = (nodisc == 0) ? m_cfg.getInt("DML", "no_disc_patch", 0) : nodisc-1;
		bool cheats = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("DML", "cheat", false));
		string NewCheatPath;
		bool DML_debug = m_gcfg2.getBool(id, "debugger", false);
		bool DM_Widescreen = m_gcfg2.getBool(id, "dm_widescreen", false);
		if(strcasestr(path.c_str(), "boot.bin") != NULL)
		{
			path.erase(path.end() - 12, path.end());
			NewCheatPath = sfmt("%s%s", path.c_str(), fmt("%s.gct", id.c_str()));
		}
		else
		{
			NewCheatPath = sfmt("%s/%s", path.c_str(), fmt("%s.gct", id.c_str()));
			NewCheatPath.erase(NewCheatPath.end() - 19, NewCheatPath.end() - 10);
		}
		if(cheats)
			snprintf(CheatPath, sizeof(CheatPath), "%s/%s", m_cheatDir.c_str(), fmt("%s.gct", id.c_str()));
		string newPath = &path[path.find_first_of(":/")+1];
		if(m_new_dml)
			DML_New_SetOptions(newPath.c_str(), CheatPath, NewCheatPath.c_str(), DeviceName[currentPartition], cheats, DML_debug, NMM, nodisc, videoMode, videoSetting, DM_Widescreen, m_new_dm_cfg);
		else
			DML_Old_SetOptions(newPath.c_str());

		if(!nodisc || !m_new_dml)
			WDVD_StopMotor();
	}
	else if(loader == 2 || (loader == 0 && m_devo_installed && strcasestr(path.c_str(), "boot.bin") == NULL))
	{
		loader = 2;
		DEVO_GetLoader(m_dataDir.c_str());
	}

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	cleanup();

	GC_SetVideoMode(videoMode, videoSetting);
	GC_SetLanguage(GClanguage);

	if(loader == 2 && !disc)
	{
		if(AHBRPOT_Patched())
			loadIOS(58, false);
		else //use cIOS instead to make sure Devolution works anyways
			loadIOS(mainIOS, false);
		ShutdownBeforeExit();
		DEVO_SetOptions(path.c_str(), currentPartition, id.c_str(), memcard_emu);
		writeStub();
		DEVO_Boot();
	}
	else
	{
		DML_New_WriteOptions();
		ShutdownBeforeExit();
		WII_Initialize();
		WII_LaunchTitle(0x100000100LL);
	}
	Sys_Exit();
}

void CMenu::_launchHomebrew(const char *filepath, vector<string> arguments)
{
	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	Playlog_Delete();
	cleanup(); // wifi and sd gecko doesnt work anymore after cleanup

	LoadHomebrew(filepath);
	AddBootArgument(filepath);
	for(u32 i = 0; i < arguments.size(); ++i)
		AddBootArgument(arguments[i].c_str());

	loadIOS(58, false);
	ShutdownBeforeExit();
	writeStub();
	BootHomebrew();
}

int CMenu::_loadIOS(u8 gameIOS, int userIOS, string id)
{
	gprintf("Game ID# %s requested IOS %d.  User selected %d\n", id.c_str(), gameIOS, userIOS);
	if(neek2o())
	{
		if(!loadIOS(gameIOS, false))
		{
			_reload_wifi_gecko();
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
		if(!loadIOS(gameIOS, true))
		{
			_reload_wifi_gecko();
			error(sfmt("errgame4", L"Couldn't load IOS %i", gameIOS));
			return LOAD_IOS_FAILED;
		}
		return LOAD_IOS_SUCCEEDED;
	}
	return LOAD_IOS_NOT_NEEDED;
}

static const char systems[11] = { 'C', 'E', 'F', 'J', 'L', 'M', 'N', 'P', 'Q', 'W', 'H' };

void CMenu::_launchChannel(dir_discHdr *hdr)
{
	Channels channel;
	u32 gameIOS = 0;
	string id = string(hdr->id);

	bool forwarder = true;
	for (u8 num = 0; num < ARRAY_SIZE(systems); num++)
	{
		if(id[0] == systems[num])
		{
			forwarder = false;
			break;
		}
	}
	forwarder = m_gcfg2.getBool(id, "custom", forwarder) || strncmp(id.c_str(), "WIMC", 4) == 0 || neek2o();
	bool emu_disabled = (m_cfg.getBool("NAND", "disable", true) || neek2o());
	if(!emu_disabled && !neek2o())
		forwarder = false;

	bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
	bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("NAND", "cheat", false));
	bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));

	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
	videoMode = (videoMode == 0) ? (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1) : videoMode-1;

	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	language = (language == 0) ? min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1) : language;

	const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;
	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	int aspectRatio = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u)-1;

	SmartBuf cheatFile;
	u32 cheatSize = 0;
	if(!forwarder)
	{
		hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 0);
		debuggerselect = m_gcfg2.getBool(id, "debugger", false) ? 1 : 0;

		if((debuggerselect || cheat) && hooktype == 0) 
			hooktype = 1;
		if(!debuggerselect && !cheat) 
			hooktype = 0;
		if(cheat && hooktype)
			_loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", id.c_str()));
	}

	m_cfg.setString("NAND", "current_item", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1); 
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(!forwarder && has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	string emuPath;
	m_partRequest = m_cfg.getInt("NAND", "partition", 0);
	int emuPartition = _FindEmuPart(&emuPath, m_partRequest, false);
	int emulate_mode = min(max(0, m_cfg.getInt("NAND", "emulation", 1)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
	
	int userIOS = m_gcfg2.getInt(id, "ios", 0);
	u64 gameTitle = TITLE_ID(hdr->settings[0],hdr->settings[1]);
	bool useNK2o = (m_gcfg2.getBool(id, "useneek", false) && !neek2o());

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	cleanup();

	/*if(useNK2o && emuPartition != 1)
		useNK2o = false;*/

	if(useNK2o && !emu_disabled)
	{
		if(!Load_Neek2o_Kernel())
		{
			error(_t("errneek1", L"Cannot launch neek2o. Verify your neek2o setup"));
			Sys_Exit();
		}
		
		int rtrnID = 0;
		if(rtrn != NULL && strlen(rtrn) == 4)
			rtrnID = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];

		ShutdownBeforeExit();
		Launch_nk(gameTitle, emuPath.size() > 1 ? emuPath.c_str() : NULL, rtrnID ? (((u64)(0x00010001) << 32) | (rtrnID & 0xFFFFFFFF)) : rtrnID);
	}
	if(!forwarder || neek2o())
	{
		if(!emu_disabled)
		{
			DeviceHandle.UnMount(emuPartition);
			Nand::Instance()->Init(emuPath.c_str(), emuPartition, false);
			Nand::Instance()->Enable_Emu();
		}
		gameIOS = channel.GetRequestedIOS(gameTitle);
		if(!emu_disabled)
			Nand::Instance()->Disable_Emu();
		if(_loadIOS(gameIOS, userIOS, id) == LOAD_IOS_FAILED)
			Sys_Exit();
	}
	if((CurrentIOS.Type == IOS_TYPE_D2X || neek2o()) && rtrn != NULL && strlen(rtrn) == 4)
	{
		int rtrnID = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];

		static ioctlv vector[1]  ATTRIBUTE_ALIGN(32);
		sm_title_id[0] = (((u64)(0x00010001) << 32) | (rtrnID & 0xFFFFFFFF));

		vector[0].data = sm_title_id;
		vector[0].len = 8;

		s32 ESHandle = IOS_Open("/dev/es", 0);
		gprintf("Return to channel %s %s. Using new d2x way\n", rtrn, IOS_Ioctlv(ESHandle, 0xA1, 1, 0, vector) != -101 ? "Succeeded" : "Failed!" );
		IOS_Close(ESHandle);
	}
	if(!emu_disabled)
	{
		Nand::Instance()->Init(emuPath.c_str(), emuPartition, false);
		if(emulate_mode == 1)
			Nand::Instance()->Set_FullMode(true);
		else
			Nand::Instance()->Set_FullMode(false);

		if(Nand::Instance()->Enable_Emu() < 0)
		{
			Nand::Instance()->Disable_Emu();
			error(_t("errgame5", L"Enabling emu failed!"));
			Sys_Exit();
		}
	}
	if(forwarder)
	{
		ShutdownBeforeExit();
		WII_Initialize();
		WII_LaunchTitle(gameTitle);
	}
	else 
	{
		setLanguage(language);
		ocarina_load_code(cheatFile.get(), cheatSize);
		Identify(gameTitle);
		ExternalBooter_ChannelSetup(gameTitle);
		BootChannel(gameTitle, gameIOS, videoMode, vipatch, countryPatch, patchVidMode, aspectRatio);
	}
	Sys_Exit();
}

void CMenu::_launchGame(dir_discHdr *hdr, bool dvd)
{
	string id(hdr->id);
	string path(hdr->path);
	if(neek2o())
	{
		int discID = id.c_str()[0] << 24 | id.c_str()[1] << 16 | id.c_str()[2] << 8 | id.c_str()[3];
		WDVD_NEEK_LoadDisc((discID&0xFFFFFFFF), 0x5D1C9EA3);
		dvd = true;
		sleep(1);
	}

	if(dvd)
	{
		u32 cover = 0;
		if(!neek2o() && !Sys_DolphinMode())
		{
			Disc_SetUSB(NULL, false);
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
		_TempLoadIOS();
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
				struct gc_discHdr *gcHeader = (struct gc_discHdr *)memalign(32, sizeof(struct gc_discHdr));
				Disc_ReadGCHeader(gcHeader);
				strncpy(hdr->id, (char*)gcHeader->id, 6);
				free(gcHeader);
				/* Launching GC Game */
				_launchGC(hdr, true);
			}
		}
		else
		{
			/* Read header */
			struct discHdr *header = (struct discHdr *)memalign(32, sizeof(struct discHdr));
			Disc_ReadHeader(header);
			id = string((const char*)header->id);
			free(header);
		}
		gprintf("Game ID: %s\n", id.c_str());
	}

	bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
	bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("GAMES", "cheat", false));
	bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));

	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
	videoMode = (videoMode == 0) ? (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1) : videoMode-1;

	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	language = (language == 0) ? min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1) : language;

	const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;
	int aspectRatio = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u)-1;

	string emuPath;
	m_partRequest = m_cfg.getInt("GAMES", "savepartition", -1);
	if(m_partRequest == -1)
		m_partRequest = m_cfg.getInt("NAND", "partition", 0);
	int emuPartition = _FindEmuPart(&emuPath, m_partRequest, false);
	
	u8 emulate_mode = min((u32)m_gcfg2.getInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);

	if(emulate_mode == 0)
	{
		emulate_mode = min(max(0, m_cfg.getInt("GAMES", "save_emulation", 0)), (int)ARRAY_SIZE(CMenu::_GlobalSaveEmu) - 1);
		if(emulate_mode != 0)
			emulate_mode++;
	}
	else if(emulate_mode == 1)
		emulate_mode = 0;
	if(!dvd && emulate_mode)
	{
		if(emuPartition < 0)
		{
			if(emulate_mode == 4)
			{
				_hideWaitMessage();
				while(true)
				{
					_AutoCreateNand();
					if(_TestEmuNand(m_cfg.getInt("GAMES", "savepartition", 0), emuPath.c_str(), true))
					{
						emuPartition = m_cfg.getInt("GAMES", "savepartition", -1);
						string emuPath = m_cfg.getString("GAMES", "savepath", m_cfg.getString("NAND", "path", ""));						
						break;
					}
				}
				_showWaitMessage();
			}
			else
			{
				emuPartition = _FindEmuPart(&emuPath, 1, true);
				Nand::Instance()->CreatePath("%s:/wiiflow", DeviceName[emuPartition]);
				Nand::Instance()->CreatePath("%s:/wiiflow/nandemu", DeviceName[emuPartition]);
			}
		}
		
		m_cfg.setInt("GAMES", "savepartition", emuPartition);
		m_cfg.setString("GAMES", "savepath", emuPath);
		m_cfg.save();
		
		char basepath[64];
		snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPartition], emuPath.c_str());
		
		if(emulate_mode == 2 || emulate_mode > 3)
		{
			if(emulate_mode == 2)
			{
				m_forceext = false;
				_hideWaitMessage();
				if(!_AutoExtractSave(id))
					Nand::Instance()->CreateTitleTMD(basepath, hdr);
				_showWaitMessage();
			}
		}
		if(emulate_mode > 2)
		{
			Nand::Instance()->CreateConfig(basepath);
			Nand::Instance()->Do_Region_Change(id);
		}
	}

	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 0); // hooktype is defined in patchcode.h
	debuggerselect = m_gcfg2.getBool(id, "debugger", false) ? 1 : 0; // debuggerselect is defined in fst.h

	if ((debuggerselect || cheat) && hooktype == 0) hooktype = 1;
	if (!debuggerselect && !cheat) hooktype = 0;

	if(id == "RPWE41" || id == "RPWZ41" || id == "SPXP41") // Prince of Persia, Rival Swords
		debuggerselect = false;

	SmartBuf cheatFile, gameconfig;
	u32 cheatSize = 0, gameconfigSize = 0, returnTo = 0;

	m_cfg.setString("GAMES", "current_item", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	setLanguage(language);

	if(cheat)
		_loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", id.c_str()));
	_loadFile(gameconfig, gameconfigSize, m_txtCheatDir.c_str(), "gameconfig.txt");
	load_wip_patches((u8 *)m_wipDir.c_str(), (u8 *) &id);

	if(rtrn != NULL && strlen(rtrn) == 4)
		returnTo = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];
	int userIOS = m_gcfg2.getInt(id, "ios", 0);
	int gameIOS = GetRequestedGameIOS(hdr);

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	cleanup(); // wifi and sd gecko doesnt work anymore after cleanup

	if((!dvd || neek2o()) && !Sys_DolphinMode())
	{
		if(_loadIOS(gameIOS, userIOS, id) == LOAD_IOS_FAILED)
			Sys_Exit();
	}
	if(CurrentIOS.Type == IOS_TYPE_D2X)
	{
		/* Open ES Module */
		s32 ESHandle = IOS_Open("/dev/es", 0);
		/* IOS Reload Block */
		static ioctlv block_vector[2] ATTRIBUTE_ALIGN(32);
		static u32 mode ATTRIBUTE_ALIGN(32);
		static u32 ios ATTRIBUTE_ALIGN(32);
		mode = 2;
		block_vector[0].data = &mode;
		block_vector[0].len  = sizeof(u32);
		ios = IOS_GetVersion();
		block_vector[1].data = &ios;
		block_vector[1].len  = sizeof(u32);
		gprintf("Block IOS Reload for %i %s\n", ios, IOS_Ioctlv(ESHandle, 0xA0, 2, 0, block_vector) < 0 ? "failed!" : "succeeded");
		/* Return to */
		if(!m_directLaunch && returnTo)
		{
			static ioctlv rtn_vector[1]  ATTRIBUTE_ALIGN(32);
			sm_title_id[0] = (((u64)(0x00010001) << 32) | (returnTo & 0xFFFFFFFF));
			rtn_vector[0].data = sm_title_id;
			rtn_vector[0].len = sizeof(u64);
			gprintf("Return to channel %s %s. Using new d2x way\n", rtrn, IOS_Ioctlv(ESHandle, 0xA1, 1, 0, rtn_vector) != -101 ? "succeeded" : "failed!");
			returnTo = 0;
		}
		/* Close ES Module */
		IOS_Close(ESHandle);
	}
	if(emulate_mode && !neek2o())
	{
		Nand::Instance()->Init(emuPath.c_str(), emuPartition, false);
		DeviceHandle.UnMount(emuPartition);

		if(emulate_mode == 3)
			Nand::Instance()->Set_RCMode(true);
		else if(emulate_mode == 4)
			Nand::Instance()->Set_FullMode(true);
		else
			Nand::Instance()->Set_FullMode(false);
		if(Nand::Instance()->Enable_Emu() < 0)
		{
			Nand::Instance()->Disable_Emu();
			error(_t("errgame6", L"Enabling emu after reload failed!"));
			Sys_Exit();
		}
		if(!DeviceHandle.IsInserted(currentPartition))
			DeviceHandle.Mount(currentPartition);
		DeviceHandle.Mount(emuPartition);
	}
	if(!dvd)
	{
		DeviceHandle.OpenWBFS(currentPartition);
		bool wbfs_partition = (DeviceHandle.GetFSType(currentPartition) == PART_FS_WBFS);
		if(!wbfs_partition && get_frag_list((u8 *)id.c_str(), (char*)path.c_str(), currentPartition == 0 ? 0x200 : USBStorage2_GetSectorSize()) < 0)
			Sys_Exit();
		s32 ret = Disc_SetUSB((u8*)id.c_str(), !wbfs_partition);
		if(ret < 0)
		{
			gprintf("Set USB failed: %d\n", ret);
			error(wfmt(_fmt("errgame10", L"Set USB failed: %d"), ret));
			Sys_Exit();
		}
		if(Disc_Open(false) < 0)
		{
			error(_t("wbfsoperr2", L"Disc_Open failed"));
			Sys_Exit();
		}
		WBFS_Close();
	}
	if(gameconfig.get() != NULL)
	{
		app_gameconfig_load((u8*)&id, gameconfig.get(), gameconfigSize);
		gameconfig.release();
	}
	if(cheatFile.get() != NULL)
	{
		ocarina_load_code(cheatFile.get(), cheatSize);
		cheatFile.release();
	}
	if(CurrentIOS.Type == IOS_TYPE_HERMES)
	{
		if(dvd)
			Hermes_Disable_EHC();
		else
			Hermes_shadow_mload();
	}
	WiiFlow_ExternalBooter(videoMode, vipatch, countryPatch, patchVidMode, aspectRatio, returnTo, TYPE_WII_GAME);
}

void CMenu::_initGameMenu(CMenu::SThemeData &theme)
{
	CColor fontColor(0xD0BFDFFF);
	STexture texFavOn;
	STexture texFavOnSel;
	STexture texFavOff;
	STexture texFavOffSel;
	STexture texAdultOn;
	STexture texAdultOnSel;
	STexture texAdultOff;
	STexture texAdultOffSel;
	STexture texDelete;
	STexture texDeleteSel;
	STexture texSettings;
	STexture texSettingsSel;
	STexture texToogleBanner;
	STexture bgLQ;

	texFavOn.fromPNG(favoriteson_png);
	texFavOnSel.fromPNG(favoritesons_png);
	texFavOff.fromPNG(favoritesoff_png);
	texFavOffSel.fromPNG(favoritesoffs_png);
	texAdultOn.fromPNG(stopkidon_png);
	texAdultOnSel.fromPNG(stopkidons_png);
	texAdultOff.fromPNG(stopkidoff_png);
	texAdultOffSel.fromPNG(stopkidoffs_png);
	texDelete.fromPNG(delete_png);
	texDeleteSel.fromPNG(deletes_png);
	texSettings.fromPNG(btngamecfg_png);
	texSettingsSel.fromPNG(btngamecfgs_png);
	texToogleBanner.fromPNG(blank_png);

	_addUserLabels(theme, m_gameLblUser, ARRAY_SIZE(m_gameLblUser), "GAME");
	m_gameBg = _texture(theme.texSet, "GAME/BG", "texture", theme.bg);
	if (m_theme.loaded() && STexture::TE_OK == bgLQ.fromImageFile(fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GAME/BG", "texture").c_str()), GX_TF_CMPR, ALLOC_MEM2, 64, 64))
		m_gameBgLQ = bgLQ;

	m_gameBtnPlay = _addButton(theme, "GAME/PLAY_BTN", theme.btnFont, L"", 420, 344, 200, 56, theme.btnFontColor);
	m_gameBtnBack = _addButton(theme, "GAME/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_gameBtnFavoriteOn = _addPicButton(theme, "GAME/FAVORITE_ON", texFavOn, texFavOnSel, 460, 200, 48, 48);
	m_gameBtnFavoriteOff = _addPicButton(theme, "GAME/FAVORITE_OFF", texFavOff, texFavOffSel, 460, 200, 48, 48);
	m_gameBtnAdultOn = _addPicButton(theme, "GAME/ADULTONLY_ON", texAdultOn, texAdultOnSel, 532, 200, 48, 48);
	m_gameBtnAdultOff = _addPicButton(theme, "GAME/ADULTONLY_OFF", texAdultOff, texAdultOffSel, 532, 200, 48, 48);
	m_gameBtnSettings = _addPicButton(theme, "GAME/SETTINGS_BTN", texSettings, texSettingsSel, 460, 272, 48, 48);
	m_gameBtnDelete = _addPicButton(theme, "GAME/DELETE_BTN", texDelete, texDeleteSel, 532, 272, 48, 48);
	m_gameBtnBackFull = _addButton(theme, "GAME/BACK_FULL_BTN", theme.btnFont, L"", 100, 390, 200, 56, theme.btnFontColor);
	m_gameBtnPlayFull = _addButton(theme, "GAME/PLAY_FULL_BTN", theme.btnFont, L"", 340, 390, 200, 56, theme.btnFontColor);
	m_gameBtnToogle = _addPicButton(theme, "GAME/TOOGLE_BTN", texToogleBanner, texToogleBanner, 385, 31, 236, 127);
	m_gameBtnToogleFull = _addPicButton(theme, "GAME/TOOGLE_FULL_BTN", texToogleBanner, texToogleBanner, 20, 12, 608, 344);

	m_gameButtonsZone.x = m_theme.getInt("GAME/ZONES", "buttons_x", 0);
	m_gameButtonsZone.y = m_theme.getInt("GAME/ZONES", "buttons_y", 0);
	m_gameButtonsZone.w = m_theme.getInt("GAME/ZONES", "buttons_w", 640);
	m_gameButtonsZone.h = m_theme.getInt("GAME/ZONES", "buttons_h", 480);
	m_gameButtonsZone.hide = m_theme.getBool("GAME/ZONES", "buttons_hide", true);

	_setHideAnim(m_gameBtnPlay, "GAME/PLAY_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnBack, "GAME/BACK_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameBtnFavoriteOn, "GAME/FAVORITE_ON", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnFavoriteOff, "GAME/FAVORITE_OFF", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnAdultOn, "GAME/ADULTONLY_ON", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnAdultOff, "GAME/ADULTONLY_OFF", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnSettings, "GAME/SETTINGS_BTN", 0, 0, -1.5f, -1.5f);
	_setHideAnim(m_gameBtnDelete, "GAME/DELETE_BTN", 0, 0, -1.5f, -1.5f);
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
} __attribute__((packed));

SmartBuf gameSoundThreadStack;
u32 gameSoundThreadStackSize = (u32)32768;
void CMenu::_gameSoundThread(CMenu *m)
{
	m->m_gameSoundHdr = m->m_cf.getHdr();
	m->m_gamesound_changed = false;

	if(m->m_cf.getHdr()->type == TYPE_PLUGIN)
	{
		m_banner->DeleteBanner();
		m->m_gameSound.Load(m->m_plugin.GetBannerSound(m->m_cf.getHdr()->settings[0]), m->m_plugin.GetBannerSoundSize(), false);
		m->m_gamesound_changed = true;
		m->m_gameSoundHdr = NULL;
		return;
	}
	bool custom = false;
	u8 *custom_bnr_file = NULL;
	u32 custom_bnr_size = 0;

	bool cached = false;
	u8 *cached_bnr_file = NULL;
	u32 cached_bnr_size = 0;

	char cached_banner[256];
	snprintf(cached_banner, sizeof(cached_banner), "%s/%.6s.bnr", m->m_bnrCacheDir.c_str(), m->m_cf.getHdr()->id);
	FILE *fp = fopen(cached_banner, "rb");
	if(fp)
	{
		cached = true;
		fseek(fp, 0, SEEK_END);
		cached_bnr_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		cached_bnr_file = (u8*)malloc(cached_bnr_size);
		if(cached_bnr_file == NULL)
		{
			m->m_gameSound.FreeMemory();
			m_banner->DeleteBanner();
			m->m_gameSoundHdr = NULL;
			return;
		}
		fread(cached_bnr_file, 1, cached_bnr_size, fp);
		fclose(fp);
	}
	else
	{
		char custom_banner[256];
		snprintf(custom_banner, sizeof(custom_banner), "%s/%.6s.bnr", m->m_customBnrDir.c_str(), m->m_cf.getHdr()->id);
		FILE *fp = fopen(custom_banner, "rb");
		if(!fp)
		{
			snprintf(custom_banner, sizeof(custom_banner), "%s/%.3s.bnr", m->m_customBnrDir.c_str(), m->m_cf.getHdr()->id);
			fp = fopen(custom_banner, "rb");
			if(!fp && m->m_cf.getHdr()->type == TYPE_GC_GAME)
			{
				GC_Disc disc;
				disc.init(m->m_cf.getHdr()->path);
				u8 *opening_bnr = disc.GetGameCubeBanner();
				if(opening_bnr != NULL)
					m_banner->CreateGCBanner(opening_bnr, &m->m_vid, m->m_wbf1_font, m->m_wbf2_font, m->m_cf.getHdr()->title);
				m->m_gameSound.Load(gc_ogg, gc_ogg_size, false);
				m->m_gamesound_changed = true;
				m->m_gameSoundHdr = NULL;
				disc.clear();
				return;
			}
		}
		if(fp)
		{
			custom = true;
			fseek(fp, 0, SEEK_END);
			custom_bnr_size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			custom_bnr_file = (u8*)malloc(custom_bnr_size);
			if(custom_bnr_file == NULL)
			{
				m->m_gameSound.FreeMemory();
				m_banner->DeleteBanner();
				m->m_gameSoundHdr = NULL;
				return;
			}
			fread(custom_bnr_file, 1, custom_bnr_size, fp);
			fclose(fp);
		}
	}
	u32 sndSize = 0;
	u8 *soundBin = NULL;

	Banner *banner = cached ? new Banner(cached_bnr_file, cached_bnr_size) : 
		(custom ? new Banner((u8 *)custom_bnr_file, custom_bnr_size, 0, true) : 
		(m->m_gameSoundHdr->type == TYPE_WII_GAME ? _extractBnr(m->m_gameSoundHdr) : (m->m_gameSoundHdr->type == TYPE_CHANNEL ?
		_extractChannelBnr(TITLE_ID(m->m_gameSoundHdr->settings[0],m->m_gameSoundHdr->settings[1])) : NULL)));
	if(banner != NULL && banner->IsValid())
	{
		m_banner->LoadBanner(banner, &m->m_vid, m->m_wbf1_font, m->m_wbf2_font);
		soundBin = banner->GetFile((char *)"sound.bin", &sndSize);
	}
	else
	{
		m->m_gameSound.FreeMemory();
		m_banner->DeleteBanner();
		m->m_gameSoundHdr = NULL;
		delete banner;
		return;
	}
	if(!custom && !cached)
	{
		FILE *fp = fopen(cached_banner, "wb");
		fwrite(banner->GetBannerFile(), 1, banner->GetBannerFileSize(), fp);
		fclose(fp);
	}
	delete banner;
	if(soundBin != NULL)
	{
		if(((IMD5Header *)soundBin)->fcc == 'IMD5')
		{
			u32 newSize = 0;
			u8 *newSound = DecompressCopy(soundBin, sndSize, &newSize);
			if(newSound == NULL || newSize == 0 || !m->m_gameSound.Load(newSound, newSize))
			{
				m->m_gameSound.FreeMemory();
				m_banner->DeleteBanner();
				m->m_gameSoundHdr = NULL;
				return;
			}
			free(soundBin);
		}
		else
			m->m_gameSound.Load(soundBin, sndSize);

		if(m->m_gameSound.IsLoaded())
			m->m_gamesound_changed = true;
		else
		{
			m->m_gameSound.FreeMemory();
			m_banner->DeleteBanner();
		}
	}
	else
	{
		gprintf("WARNING: No sound found in banner!\n");
		m->m_gamesound_changed = true;
		m->m_gameSound.FreeMemory();
	}
	m->m_gameSoundHdr = NULL;
}

void CMenu::_playGameSound(void)
{
	m_gamesound_changed = false;
	if(m_bnrSndVol == 0) 
		return;

	if(m_gameSoundThread != LWP_THREAD_NULL)
		CheckGameSoundThread();
	if(!gameSoundThreadStack.get())
		gameSoundThreadStack = smartMem2Alloc(gameSoundThreadStackSize);
	LWP_CreateThread(&m_gameSoundThread, (void *(*)(void *))CMenu::_gameSoundThread, (void *)this, gameSoundThreadStack.get(), gameSoundThreadStackSize, 60);
}

void CMenu::CheckGameSoundThread()
{
	if(m_gameSoundThread == LWP_THREAD_NULL)
		return;

	if(LWP_ThreadIsSuspended(m_gameSoundThread))
		LWP_ResumeThread(m_gameSoundThread);

	while(m_gameSoundHdr != NULL)
		usleep(50);

	LWP_JoinThread(m_gameSoundThread, NULL);
	m_gameSoundThread = LWP_THREAD_NULL;
}

void CMenu::ClearGameSoundThreadStack()
{
	if(gameSoundThreadStack.get())
		gameSoundThreadStack.release();
}
