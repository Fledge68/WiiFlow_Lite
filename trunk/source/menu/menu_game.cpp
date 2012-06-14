
#include "menu.hpp"
#include "loader/patchcode.h"

#include "loader/sys.h"
#include "loader/wdvd.h"
#include "loader/alt_ios.h"
#include "loader/playlog.h"
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <unistd.h>
#include <time.h>
#include "network/http.h"
#include "network/gcard.h"
#include "DeviceHandler.hpp"
#include "loader/wbfs.h"
#include "wip.h"
#include "channel_launcher.h"
#include "devicemounter/sdhc.h"

#include <network.h>
#include <errno.h>

#include "loader/frag.h"
#include "loader/fst.h"

#include "gui/WiiMovie.hpp"
#include "gui/GameTDB.hpp"
#include "channels.h"
#include "nand.hpp"
#include "alt_ios.h"
#include "gecko.h"
#include "homebrew.h"
#include "defines.h"
#include "gc/gc.h"
#include "Gekko.h"

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

//sounds
extern const u8 gc_ogg[];
extern const u32 gc_ogg_size;

extern u32 sector_size;
extern int mainIOS;
static u64 sm_title_id[8]  ATTRIBUTE_ALIGN(32);

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

const CMenu::SOption CMenu::_videoModes[7] = {
	{ "viddef", L"Default" },
	{ "vidp50", L"PAL 50Hz" },
	{ "vidp60", L"PAL 60Hz" },
	{ "vidntsc", L"NTSC" },
	{ "vidpatch", L"Auto Patch" },
	{ "vidsys", L"System" },	
	{ "vidprog", L"Progressive" }
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
	Banner *banner = NULL;
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->hdr.id, (char *) hdr->path);
	if (disc != NULL)
	{
		void *bnr = NULL;
		if (wbfs_extract_file(disc, (char *) "opening.bnr", &bnr) > 0)
		{
			banner = new Banner((u8 *) bnr);
		}
		WBFS_CloseDisc(disc);
	}
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

	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->hdr.id, (char *) hdr->path);
	if (!disc) return IOS;

	u8 *titleTMD = NULL;
	u32 tmd_size = wbfs_extract_file(disc, (char *) "TMD", (void **)&titleTMD);
	WBFS_CloseDisc(disc);

	if(!titleTMD) return IOS;

	if(tmd_size > 0x18B)
		IOS = titleTMD[0x18B];
	return IOS;
}

void CMenu::_hideGame(bool instant)
{
	m_gameSelected = false;
	m_fa.unload();
	m_cf.showCover();
	
	m_btnMgr.hide(m_gameBtnPlay, instant);
	m_btnMgr.hide(m_gameBtnDelete, instant);
	m_btnMgr.hide(m_gameBtnSettings, instant);
	m_btnMgr.hide(m_gameBtnBack, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOn, instant);
	m_btnMgr.hide(m_gameBtnFavoriteOff, instant);
	m_btnMgr.hide(m_gameBtnAdultOn, instant);
	m_btnMgr.hide(m_gameBtnAdultOff, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if (m_gameLblUser[i] != -1u)
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
		
	m_btnMgr.show(m_gameBtnPlay);
	m_btnMgr.show(m_gameBtnBack);
	for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
		if (m_gameLblUser[i] != -1u)
			m_btnMgr.show(m_gameLblUser[i]);
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

	s8 startGameSound = 1;
	while(true)
	{
		if(startGameSound < 1)
			startGameSound++;

		string id(m_cf.getId());
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
		if (BTN_B_PRESSED && (m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff)))
		{
			_hideGame();
			_CategorySettings(true);
			_showGame();
			continue;
		}
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			m_gameSound.FreeMemory();
			CheckGameSoundThread();
			ClearGameSoundThreadStack();
			break;
		}
		else if(BTN_PLUS_PRESSED && m_GameTDBLoaded && (m_cf.getHdr()->hdr.magic == WII_MAGIC || m_cf.getHdr()->hdr.gc_magic == GC_MAGIC || m_current_view == COVERFLOW_CHANNEL))
		{
			_hideGame();
			m_gameSelected = true;
			_gameinfo();
			_showGame();
			if (!m_gameSound.IsPlaying()) 
				startGameSound = -6;
		}
		else if(BTN_MINUS_PRESSED)
		{
			string videoPath = sfmt("%s/%.3s.thp", m_videoDir.c_str(), id.c_str());

			FILE *file = fopen(videoPath.c_str(), "rb");
			if(file)
			{
				fclose(file);
				_hideGame();
				WiiMovie movie(videoPath.c_str());
				movie.SetScreenSize(m_cfg.getInt("GENERAL", "tv_width", 640), m_cfg.getInt("GENERAL", "tv_height", 480), m_cfg.getInt("GENERAL", "tv_x", 0), m_cfg.getInt("GENERAL", "tv_y", 0));
				movie.SetVolume(m_cfg.getInt("GENERAL", "sound_volume_bnr", 255));
				//_stopSounds();		
				movie.Play();

				m_video_playing = true;

				STexture videoBg;
				while(!BTN_B_PRESSED && !BTN_A_PRESSED && !BTN_HOME_PRESSED && movie.GetNextFrame(&videoBg))
				{
					_setBg(videoBg, videoBg);
					m_bgCrossFade = 10;
					_mainLoopCommon(); // Redraw the background every frame
				}
				movie.Stop();
				_showGame();
				m_video_playing = false;
				//m_gameSound.play(m_bnrSndVol);
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
					if(_wbfsOp(CMenu::WO_REMOVE_GAME))
					{
						m_gameSound.Stop();
						CheckGameSoundThread();
						break;
					}
					_showGame();
				}
			}
			else if(m_btnMgr.selected(m_gameBtnFavoriteOn) || m_btnMgr.selected(m_gameBtnFavoriteOff))
				m_gcfg1.setBool("FAVORITES", id, !m_gcfg1.getBool("FAVORITES", id, false));
			else if(m_btnMgr.selected(m_gameBtnAdultOn) || m_btnMgr.selected(m_gameBtnAdultOff))
				m_gcfg1.setBool("ADULTONLY", id, !m_gcfg1.getBool("ADULTONLY", id, false));
			else if(m_btnMgr.selected(m_gameBtnBack))
			{
				m_gameSound.Stop();
				CheckGameSoundThread();
				break;
			}
			else if(m_btnMgr.selected(m_gameBtnSettings))
			{
				_hideGame();
				m_gameSelected = true;
				_gameSettings();
				_showGame();
				if(!m_gameSound.IsPlaying()) 
					startGameSound = -6;
			}
			else if(launch || m_btnMgr.selected(m_gameBtnPlay) || (!WPadIR_Valid(0) && !WPadIR_Valid(1) && !WPadIR_Valid(2) && !WPadIR_Valid(3) && m_btnMgr.selected((u32)-1)))
			{
				_hideGame();
				dir_discHdr *hdr = m_cf.getHdr();
				if(currentPartition != SD && hdr->hdr.gc_magic == GC_MAGIC)
				{
					char gcfolder[300];
					snprintf(gcfolder, sizeof(gcfolder), "%s [%s]", m_cf.getTitle().toUTF8().c_str(), (char *)hdr->hdr.id);
					if(GC_GameIsInstalled((char *)hdr->hdr.id, DeviceName[SD], DML_DIR))
					{
						memset(hdr->path, 0, sizeof(hdr->path));
						strncpy(hdr->path, (char*)hdr->hdr.id, sizeof(hdr->path));
					}
					else if(GC_GameIsInstalled(gcfolder, DeviceName[SD], DML_DIR))
					{
						memset(hdr->path, 0, sizeof(hdr->path));
						strncpy(hdr->path, gcfolder, sizeof(hdr->path));
					}
					else if(!GC_GameIsInstalled(hdr->path, DeviceName[SD], DML_DIR) && !_wbfsOp(CMenu::WO_COPY_GAME))
						break;
					currentPartition = SD;
				}

				m_cf.clear();
				_showWaitMessage();

				if(m_current_view != COVERFLOW_HOMEBREW && m_current_view != COVERFLOW_EMU)
				{
					// Get banner_title
					Banner * banner = m_current_view == COVERFLOW_CHANNEL ? _extractChannelBnr(chantitle) : (m_current_view == COVERFLOW_USB && hdr->hdr.gc_magic != GC_MAGIC) ? _extractBnr(hdr) : NULL;
					if(banner != NULL)
					{						
						if(banner->IsValid())
							_extractBannerTitle(banner, GetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str()));
						delete banner;
					}
					banner = NULL;

					if(Playlog_Update(id.c_str(), banner_title) < 0)
						Playlog_Delete();
				}

				gprintf("Launching game %s\n", id.c_str());
				_launch(hdr);

				if(m_exit)
					break;

				_hideWaitMessage();
				launch = false;

				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());

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
			m_cf.up();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_RIGHT_REPEAT || RIGHT_STICK_RIGHT))
		{
			m_cf.right();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_DOWN_REPEAT || RIGHT_STICK_DOWN))
		{
			m_cf.down();
			startGameSound = -10;
		}
		if((startGameSound == 1 || startGameSound < -8) && (BTN_LEFT_REPEAT || RIGHT_STICK_LEFT))
		{
			m_cf.left();
			startGameSound = -10;
		}
		if(startGameSound == -10)
		{
			m_gameSound.Stop();
			m_gameSelected = false;
			m_fa.unload();
			_setBg(m_mainBg, m_mainBgLQ);
		}
		if(m_show_zone_game)
		{
			bool b = m_gcfg1.getBool("FAVORITES", id, false);
			m_btnMgr.show(b ? m_gameBtnFavoriteOn : m_gameBtnFavoriteOff);
			m_btnMgr.hide(b ? m_gameBtnFavoriteOff : m_gameBtnFavoriteOn);
			m_btnMgr.show(m_gameBtnPlay);
			m_btnMgr.show(m_gameBtnBack);
			for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
				if (m_gameLblUser[i] != -1u)
					m_btnMgr.show(m_gameLblUser[i]);

			if (!m_locked)
			{
				b = m_gcfg1.getBool("ADULTONLY", id, false);
				m_btnMgr.show(b ? m_gameBtnAdultOn : m_gameBtnAdultOff);
				m_btnMgr.hide(b ? m_gameBtnAdultOff : m_gameBtnAdultOn);
				m_btnMgr.show(m_gameBtnSettings);
			}

			if ((m_current_view == COVERFLOW_USB || m_current_view == COVERFLOW_EMU || m_cf.getHdr()->hdr.gc_magic == GC_MAGIC) && !m_locked)
				m_btnMgr.show(m_gameBtnDelete);
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
			for (u32 i = 0; i < ARRAY_SIZE(m_gameLblUser); ++i)
				if (m_gameLblUser[i] != -1u)
					m_btnMgr.hide(m_gameLblUser[i]);
		}
	}
	m_gcfg1.save(true);
	_hideGame();
}

void CMenu::_directlaunch(const string &id)
{
	m_directLaunch = true;

	for (int i = USB1; i < USB8; i++)
	{
		if(!DeviceHandler::Instance()->IsInserted(i)) continue;

		DeviceHandler::Instance()->Open_WBFS(i);
		CList<dir_discHdr> list;
		string path = sfmt(GAMES_DIR, DeviceName[i]);
		vector<string> pathlist;
		list.GetPaths(pathlist, id.c_str(), path,
			strncasecmp(DeviceHandler::Instance()->PathToFSName(path.c_str()), "WBFS", 4) == 0);

		m_gameList.clear();
		Config nullCfg;
		list.GetHeaders(pathlist, m_gameList, m_settingsDir, m_curLanguage, m_DMLgameDir, nullCfg);
		if(m_gameList.size() > 0)
		{
			gprintf("Game found on partition #%i\n", i);
			_launch(&m_gameList[0]); // Launch will exit wiiflow
		}
	}
	error(sfmt("Cannot find the game with ID: %s", id.c_str()));
}

void CMenu::_launch(dir_discHdr *hdr)
{
	m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
	if(hdr->hdr.gc_magic == EMU_MAGIC)
	{
		string title(&hdr->path[string(hdr->path).find_last_of("/")+1]);
		string wiiflow_dol(m_dol);
		if(strstr(wiiflow_dol.c_str(), "sd:/") == NULL)
			wiiflow_dol.erase(3,1);
		string path((char*)hdr->path, size_t(strlen((char*)hdr->path) - title.size()));
		vector<string> arguments;
		gprintf("Game title: %s\n", title.c_str());
		if(m_plugin.isMplayerCE(hdr->hdr.magic))
			arguments = m_plugin.CreateMplayerCEArguments(string(hdr->path).c_str());
		else if(strstr(path.c_str(), ":/") != NULL)
		{
			if(strstr(path.c_str(), "sd:/") == NULL)
				path.erase(3,1);
			arguments.push_back(path);
			arguments.push_back(title);
			if(m_plugin.UseReturnLoader(hdr->hdr.magic))
				arguments.push_back(sfmt("%s/WiiFlowLoader.dol",m_pluginsDir.c_str()));
			else
				arguments.push_back(wiiflow_dol);
			m_cfg.setString("EMULATOR", "current_item", &hdr->path[string(hdr->path).find_last_of("/")]);
		}
		else
		{
			arguments.push_back(title);
			char gametitle[64];
			wcstombs(gametitle, hdr->title, sizeof(gametitle));
			m_cfg.setString("EMULATOR", "current_item", gametitle);
		}
		_launchHomebrew(fmt("%s/%s", m_pluginsDir.c_str(), m_plugin.GetDolName(hdr->hdr.magic)), arguments);
		return;
	}
	else if(hdr->hdr.gc_magic == GC_MAGIC)
	{
		_launchGC(hdr, true);
		return;
	}
	switch(m_current_view)
	{
		case COVERFLOW_HOMEBREW:
			_launchHomebrew((char *)hdr->path, m_homebrewArgs);
			break;
		case COVERFLOW_CHANNEL:
			_launchChannel(hdr);
			break;
		case COVERFLOW_DML:
			_launchGC(hdr, true);
			break;
		case COVERFLOW_EMU:
			_launchHomebrew((char *)hdr->path, m_homebrewArgs);
			break;
		case COVERFLOW_USB:
		default:
			_launchGame(hdr, false);
			break;
	}
}

extern "C" {extern void USBStorage_Deinit(void);}

void CMenu::_launchGC(dir_discHdr *hdr, bool DML)
{
	char* id = (char *)hdr->hdr.id;

	Nand::Instance()->Disable_Emu();

	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id);

	u8 GClanguage = min((u32)m_gcfg2.getInt(id, "gc_language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
	GClanguage = (GClanguage == 0) ? min((u32)m_cfg.getInt("DML", "game_language", 0), ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1u) : GClanguage-1;

	u8 DMLvideoMode = min((u32)m_gcfg2.getInt(id, "dml_video_mode", 0), ARRAY_SIZE(CMenu::_DMLvideoModes) - 1u);
	DMLvideoMode = (DMLvideoMode == 0) ? min((u32)m_cfg.getInt("DML", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalDMLvideoModes) - 1u) : DMLvideoMode-1;
	if((id[3] == 'P') && (DMLvideoMode == 0))
		DMLvideoMode = 1;
	else if((id[3] != 'P') && (DMLvideoMode == 0))
		DMLvideoMode = 2;

	if(DML)
	{
		m_cfg.setString("DML", "current_item", id);

		char CheatPath[256];
		char NewCheatPath[255];
		u8 NMM = min((u32)m_gcfg2.getInt(id, "dml_nmm", 0), ARRAY_SIZE(CMenu::_NMM) - 1u);
		NMM = (NMM == 0) ? m_cfg.getInt("DML", "dml_nmm", 0) : NMM-1;
		u8 nodisc = min((u32)m_gcfg2.getInt(id, "no_disc_patch", 0), ARRAY_SIZE(CMenu::_NoDVD) - 1u);
		nodisc = (nodisc == 0) ? m_cfg.getInt("DML", "no_disc_patch", 0) : nodisc-1;
		bool cheats = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("DML", "cheat", false));
		bool DML_debug = m_gcfg2.getBool(id, "debugger", false);

		if(cheats)
		{
			snprintf(CheatPath, sizeof(CheatPath), "%s/%s", m_cheatDir.c_str(), fmt("%s.gct", id));
			snprintf(NewCheatPath, sizeof(NewCheatPath), "%s/%s/%s", fmt(DML_DIR, "sd"), hdr->path, fmt("%s.gct", id));
		}

		if(m_new_dml)
			DML_New_SetOptions(hdr->path, CheatPath, NewCheatPath, cheats, DML_debug, NMM, nodisc, DMLvideoMode);
		else
			DML_Old_SetOptions(hdr->path, CheatPath, NewCheatPath, cheats);

		if(!nodisc || !m_new_dml)
		{
			WDVD_Init();
			WDVD_StopMotor();
			WDVD_Close();
		}
	}
	else
		DML_New_SetBootDiscOption();

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	cleanup();
	Close_Inputs();
	USBStorage_Deinit();
	SDHC_Init();

	GC_SetVideoMode(DMLvideoMode);
	GC_SetLanguage(GClanguage);
	DML_New_WriteOptions();

	WII_Initialize();
	if(WII_LaunchTitle(0x100000100LL) < 0)
		Sys_LoadMenu();
}

void CMenu::_launchHomebrew(const char *filepath, vector<string> arguments)
{
	Nand::Instance()->Disable_Emu();
	m_reload = true;

	Channels channel;
	u64 title = SYSTEM_MENU;
	if(channel.GetRequestedIOS(RETURN_CHANNEL) != 0)
		title = RETURN_CHANNEL;

	gprintf("Filepath of homebrew: %s\n",filepath);

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	while(net_get_status() == -EBUSY)
		usleep(100);

	m_vid.CheckWaitThread(true);
	Playlog_Delete();
	cleanup();
	// wifi and sd gecko doesnt work anymore after cleanup
	Close_Inputs();
	m_vid.cleanup();
	wiiLightOff();

	LoadHomebrew(filepath);
	DeviceHandler::DestroyInstance();
	USBStorage_Deinit();
	AddBootArgument(filepath);
	for(u32 i = 0; i < arguments.size(); ++i)
	{
		gprintf("Boot argument: %s\n", arguments[i].c_str());
		AddBootArgument(arguments[i].c_str());
	}
	gprintf("Return to Channel: %08x %08x\n", TITLE_UPPER(title), TITLE_LOWER(title));
	gprintf("Booting Homebrew application...\n");
	BootHomebrew(title);
}

int CMenu::_loadIOS(u8 gameIOS, int userIOS, string id)
{
	gprintf("Game ID# %s requested IOS %d.  User selected %d\n", id.c_str(), gameIOS, userIOS);

	if (userIOS > 0 && _installed_cios.size() > 0)
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

	// remap IOS to CIOS
	if(gameIOS < 0x64)
	{
		if(_installed_cios.size() <= 0)
		{
			error(sfmt("No cios found!"));
			Sys_LoadMenu();
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
		{
			error(sfmt("Couldn't find a cIOS using base %i, or 56/57", IOS[0]));
			return LOAD_IOS_FAILED;
		}
	}

	if(gameIOS != mainIOS)
	{
		gprintf("Reloading IOS into %d\n", gameIOS);
		if(!loadIOS(gameIOS, true))
		{
			_reload_wifi_gecko();
			error(sfmt("Couldn't load IOS %i", gameIOS));
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
	u32 ios = 0;
	u32 entry = 0;	
	Nand::Instance()->Disable_Emu();

	string id = string((const char *) hdr->hdr.id);

	bool forwarder = true;
	for (u8 num = 0; num < ARRAY_SIZE(systems); num++)
	{
		if(id[0] == systems[num])
		{
			forwarder = false;
			break;
		}
	}

	forwarder = m_gcfg2.getBool(id, "custom", forwarder) || strncmp(id.c_str(), "WIMC", 4) == 0;
	
	bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
	bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("NAND", "cheat", false));
	bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));
	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1u);
	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;
	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	int aspectRatio = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u)-1;
	
	if(!forwarder)
	{
		hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 0);
		debuggerselect = m_gcfg2.getBool(id, "debugger", false) ? 1 : 0;

		if ((debuggerselect || cheat) && hooktype == 0) 
			hooktype = 1;
		if (!debuggerselect && !cheat) 
			hooktype = 0;
		if (videoMode == 0)	
			videoMode = (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1);
		if (language == 0)	
			language = min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1);
	}

	m_cfg.setString("NAND", "current_item", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1); 
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(!forwarder && has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	string emuPath;
	m_partRequest = m_cfg.getInt("NAND", "partition", 0);
	int emuPartition = _FindEmuPart(&emuPath, m_partRequest, false);
	
	bool emu_disabled = m_cfg.getBool("NAND", "disable", true);
	int emulate_mode = min(max(0, m_cfg.getInt("NAND", "emulation", 1)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
	
	int userIOS = m_gcfg2.getInt(id, "ios", 0);

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);
	
	/*if(!emu_disabled && emulate_mode == 1)
	{
		char basepath[64];
		snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPartition], emuPath.c_str());
		Nand::Instance()->CreateConfig(basepath);
		Nand::Instance()->Do_Region_Change(id);
	}*/

	CheckGameSoundThread();
	m_vid.CheckWaitThread(true);
	cleanup();
	USBStorage_Deinit();
	Close_Inputs();
	
	if(!emu_disabled)
	{		
		Nand::Instance()->Init(emuPath.c_str(), emuPartition, false);
		DeviceHandler::Instance()->UnMount(emuPartition);

		if(emulate_mode == 1)
			Nand::Instance()->Set_FullMode(true);
		else
			Nand::Instance()->Set_FullMode(false);

		if(Nand::Instance()->Enable_Emu() < 0)
		{
			Nand::Instance()->Disable_Emu();
			error(L"Enabling emu failed!");

			return;
		}
	}		

	if(!forwarder)
	{
		entry = channel.Load(hdr->hdr.chantitle, &ios);
		setLanguage(language);

		SmartBuf cheatFile;
		u32 cheatSize = 0;
		if (cheat)
			_loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", hdr->hdr.id));
		
		ocarina_load_code(cheatFile.get(), cheatSize);

		int gameIOS = userIOS == 0 ? ios : userIOS;
		
		gprintf("%s IOS %u\n", userIOS == 0 ? "Game requested" : "User requested", gameIOS);
		
		if (gameIOS != mainIOS && gameIOS <= 0x50)
		{
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
			{
				error(sfmt("Couldn't find a cIOS using base %i, or 56/57", IOS[0]));
				return;
			}
		}

		if(gameIOS != mainIOS)
		{
			gprintf("Reloading IOS into %d\n", gameIOS);
			if(!loadIOS(gameIOS, false))
			{
				_reload_wifi_gecko();
				error(sfmt("Couldn't reload to cIOS %i", gameIOS));
				return;
			}
			if(!emu_disabled)
			{
				Nand::Instance()->Init(emuPath.c_str(), emuPartition, false);
				DeviceHandler::Instance()->UnMount(emuPartition);

				if(emulate_mode == 1)
					Nand::Instance()->Set_FullMode(true);
				else
					Nand::Instance()->Set_FullMode(false);

				if(Nand::Instance()->Enable_Emu() < 0)
				{
					Nand::Instance()->Disable_Emu();
					error(L"Enabling emu after reload failed!");
					Sys_LoadMenu();
					return;
				}
			}
		}		
	}	
	
	if(rtrn != NULL && strlen(rtrn) == 4)
	{			
		int rtrnID = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];
		
		static ioctlv vector[1]  ATTRIBUTE_ALIGN(32);
		sm_title_id[0] = (((u64)(0x00010001) << 32) | (rtrnID&0xFFFFFFFF));
		
		vector[0].data = sm_title_id;
		vector[0].len = 8;
		
		s32 ESHandle = IOS_Open("/dev/es", 0);
		gprintf("Return to channel %s. Using new d2x way\n", IOS_Ioctlv(ESHandle, 0xA1, 1, 0, vector) != -101 ? "Succeeded" : "Failed!" );
		IOS_Close(ESHandle);
	}

	if(forwarder)
	{
		WII_Initialize();
		if(WII_LaunchTitle(hdr->hdr.chantitle) < 0)
			Sys_LoadMenu();	
	}
	
	if(!BootChannel(entry, hdr->hdr.chantitle, ios, videoMode, vipatch, countryPatch, patchVidMode, aspectRatio))
		Sys_LoadMenu();
}

void CMenu::_launchGame(dir_discHdr *hdr, bool dvd)
{
	string id = string((const char *) hdr->hdr.id);
	Nand::Instance()->Disable_Emu();

	if (dvd)
	{
		u32 cover = 0;

		Disc_SetUSB(NULL);
		if (WDVD_GetCoverStatus(&cover) < 0)
		{
			error(L"WDVDGetCoverStatus Failed!");
			if (BTN_B_PRESSED) return;
		}
		if (!(cover & 0x2))
		{
			error(L"Please insert a game disc.");
			do {
				WDVD_GetCoverStatus(&cover);
				if (BTN_B_PRESSED) return;
			} while(!(cover & 0x2));
		}
		/* Open Disc */
		if (Disc_Open() < 0) 
		{
			error(L"Cannot Read DVD.");
			if (BTN_B_PRESSED) return;
		}

		/* Check disc */
		if (Disc_IsWii() < 0)
		{
			if (Disc_IsGC() < 0) 
			{
				error(L"This is not a Wii or GC disc");
				if (BTN_B_PRESSED) return;
			}
			else
			{
				/* Read GC disc header */
				struct gc_discHdr *gcHeader = (struct gc_discHdr *)MEM2_alloc(sizeof(struct gc_discHdr));
				Disc_ReadGCHeader(gcHeader);
				memcpy(hdr->hdr.id, gcHeader->id, 6);
				MEM2_free(gcHeader);
				/* Launching GC Game */
				_launchGC(hdr, false);
			}
		}

		/* Read header */
		struct discHdr *header = (struct discHdr *)MEM2_alloc(sizeof(struct discHdr));
		Disc_ReadHeader(header);
		for (int i = 0;i < 6; i++)
			id[i] = header->id[i];
		MEM2_free(header);
	}
	bool vipatch = m_gcfg2.testOptBool(id, "vipatch", m_cfg.getBool("GENERAL", "vipatch", false));
	bool cheat = m_gcfg2.testOptBool(id, "cheat", m_cfg.getBool("GAMES", "cheat", false));
	bool countryPatch = m_gcfg2.testOptBool(id, "country_patch", m_cfg.getBool("GENERAL", "country_patch", false));
	u8 videoMode = (u8)min((u32)m_gcfg2.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1u);
	int language = min((u32)m_gcfg2.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	const char *rtrn = m_gcfg2.getBool(id, "returnto", true) ? m_cfg.getString("GENERAL", "returnto").c_str() : NULL;
	int aspectRatio = min((u32)m_gcfg2.getInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u)-1;

	string emuPath;
	m_partRequest = m_cfg.getInt("GAMES", "savepartition", -1);
	if(m_partRequest == -1)
		m_partRequest = m_cfg.getInt("NAND", "partition", 0);
	int emuPartition = _FindEmuPart(&emuPath, m_partRequest, false);
	
	u8 emulate_mode = min((u32)m_gcfg2.getInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);

	if (emulate_mode == 0)
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
	
	if (!dvd && get_frag_list((u8 *) hdr->hdr.id, (char *) hdr->path, currentPartition == 0 ? 0x200 : sector_size) < 0)
		return;

	u8 patchVidMode = min((u32)m_gcfg2.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 0); // hooktype is defined in patchcode.h
	debuggerselect = m_gcfg2.getBool(id, "debugger", false) ? 1 : 0; // debuggerselect is defined in fst.h

	if ((debuggerselect || cheat) && hooktype == 0) hooktype = 1;
	if (!debuggerselect && !cheat) hooktype = 0;

	if (id == "RPWE41" || id == "RPWZ41" || id == "SPXP41") // Prince of Persia, Rival Swords
		debuggerselect = false;

	SmartBuf cheatFile, gameconfig;
	u32 cheatSize = 0, gameconfigSize = 0;

	CheckGameSoundThread();
	if(videoMode == 0)	videoMode = (u8)min((u32)m_cfg.getInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1);
	if(language == 0)	language = min((u32)m_cfg.getInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1);
	m_cfg.setString("GAMES", "current_item", id);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	int userIOS = 0;
	m_gcfg2.getInt(id, "ios", &userIOS);

	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	setLanguage(language);

	if(cheat)
		_loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", hdr->hdr.id));

	_loadFile(gameconfig, gameconfigSize, m_txtCheatDir.c_str(), "gameconfig.txt");

	load_wip_patches((u8 *) m_wipDir.c_str(), (u8 *) &hdr->hdr.id);	
	app_gameconfig_load((u8 *) &hdr->hdr.id, gameconfig.get(), gameconfigSize);
	ocarina_load_code(cheatFile.get(), cheatSize);

	if (!m_use_wifi_gecko)
		net_wc24cleanup();

	bool iosLoaded = false;

	if (!dvd)
	{
		int result = _loadIOS(GetRequestedGameIOS(hdr), userIOS, id);
		if (result == LOAD_IOS_FAILED)
			return;
		if (result == LOAD_IOS_SUCCEEDED)
			iosLoaded = true;
	}
	
	if(emulate_mode)
	{
		Nand::Instance()->Init(emuPath.c_str(), emuPartition, false);
		DeviceHandler::Instance()->UnMount(emuPartition);

		if(emulate_mode == 3)
			Nand::Instance()->Set_RCMode(true);
		else if(emulate_mode == 4)
			Nand::Instance()->Set_FullMode(true);
		else
			Nand::Instance()->Set_FullMode(false);

		if(Nand::Instance()->Enable_Emu() < 0)
		{
			Nand::Instance()->Disable_Emu();
			error(L"Enabling emu after reload failed!");
			if (iosLoaded) Sys_LoadMenu();
			return;
		}
		if(!DeviceHandler::Instance()->IsInserted(currentPartition))
			DeviceHandler::Instance()->Mount(currentPartition);
		DeviceHandler::Instance()->Mount(emuPartition);
	}

	if (!m_directLaunch)
	{
		if (rtrn != NULL && strlen(rtrn) == 4)
		{			
			int rtrnID = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];

			static ioctlv vector[1]  ATTRIBUTE_ALIGN(32);

			sm_title_id[0] = (((u64)(0x00010001) << 32) | (rtrnID&0xFFFFFFFF));

			vector[0].data = sm_title_id;
			vector[0].len = 8;

			s32 ESHandle = IOS_Open("/dev/es", 0);
			gprintf("Return to channel %s. Using new d2x way\n", IOS_Ioctlv(ESHandle, 0xA1, 1, 0, vector) != -101 ? "succeeded" : "failed!");
			IOS_Close(ESHandle);
		}
	}

	if (!dvd)
	{
		s32 ret = Disc_SetUSB((u8 *) hdr->hdr.id);
		if (ret < 0)
		{
			gprintf("Set USB failed: %d\n", ret);
			error(wfmt(L"Set USB failed: %d\n", ret));
			if (iosLoaded) Sys_LoadMenu();
			return;
		}

		if (Disc_Open() < 0)
		{
			error(L"Disc_Open failed");
			if (iosLoaded) Sys_LoadMenu();
			return;
		}
	}
	IOSReloadBlock(IOS_GetVersion(), true);

	while(net_get_status() == -EBUSY)
		usleep(100);

	m_vid.CheckWaitThread(true);
	cleanup();
	// wifi and sd gecko doesnt work anymore after cleanup
	Close_Inputs();
	USBStorage_Deinit();
	if(currentPartition == 0)
		SDHC_Init();

	/* Find game partition offset */
	u64 offset;
	s32 ret = Disc_FindPartition(&offset);
	if(ret < 0)
		return;

	RunApploader(offset, videoMode, vipatch, countryPatch, patchVidMode, aspectRatio);
	gprintf("Booting game\n");
	Disc_BootPartition();
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
	_addUserLabels(theme, m_gameLblUser, ARRAY_SIZE(m_gameLblUser), "GAME");
	m_gameBg = _texture(theme.texSet, "GAME/BG", "texture", theme.bg);
	if (m_theme.loaded() && STexture::TE_OK == bgLQ.fromPNGFile(fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GAME/BG", "texture").c_str()), GX_TF_CMPR, ALLOC_MEM2, 64, 64))
		m_gameBgLQ = bgLQ;

	m_gameBtnPlay = _addButton(theme, "GAME/PLAY_BTN", theme.btnFont, L"", 420, 344, 200, 56, theme.btnFontColor);
	m_gameBtnBack = _addButton(theme, "GAME/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_gameBtnFavoriteOn = _addPicButton(theme, "GAME/FAVORITE_ON", texFavOn, texFavOnSel, 460, 170, 48, 48);
	m_gameBtnFavoriteOff = _addPicButton(theme, "GAME/FAVORITE_OFF", texFavOff, texFavOffSel, 460, 170, 48, 48);
	m_gameBtnAdultOn = _addPicButton(theme, "GAME/ADULTONLY_ON", texAdultOn, texAdultOnSel, 532, 170, 48, 48);
	m_gameBtnAdultOff = _addPicButton(theme, "GAME/ADULTONLY_OFF", texAdultOff, texAdultOffSel, 532, 170, 48, 48);
	m_gameBtnSettings = _addPicButton(theme, "GAME/SETTINGS_BTN", texSettings, texSettingsSel, 460, 242, 48, 48);
	m_gameBtnDelete = _addPicButton(theme, "GAME/DELETE_BTN", texDelete, texDeleteSel, 532, 242, 48, 48);

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
	_hideGame(true);
	_textGame();
}

void CMenu::_textGame(void)
{
	m_btnMgr.setText(m_gameBtnPlay, _t("gm1", L"Play"));
	m_btnMgr.setText(m_gameBtnBack, _t("gm2", L"Back"));
}

struct IMD5Header
{
	u32 fcc;
	u32 filesize;
	u8 zeroes[8];
	u8 crypto[16];
} __attribute__((packed));

SmartBuf gameSoundThreadStack;
unsigned int gameSoundThreadStackSize = (unsigned int)32768;

void CMenu::_gameSoundThread(CMenu *m)
{
	if(m->m_cf.getHdr()->hdr.gc_magic == GC_MAGIC)
	{
		m->m_gameSound.Load(gc_ogg, gc_ogg_size, false);
		m->m_gamesound_changed = true;
		return;
	}
	else if(m->m_cf.getHdr()->hdr.gc_magic == EMU_MAGIC)
	{
		m->m_gameSound.Load(m->m_plugin.GetBannerSound(m->m_cf.getHdr()->hdr.magic), m->m_plugin.GetBannerSoundSize(), false);
		m->m_gamesound_changed = true;
		return;
	}

	m->m_gamesound_changed = false;
	u32 sndSize = 0;
	m->m_gameSoundHdr = m->m_cf.getHdr();

	Banner *banner = m->m_current_view == COVERFLOW_USB ?
		_extractBnr(m->m_gameSoundHdr) : m->m_current_view == COVERFLOW_CHANNEL ?
		_extractChannelBnr(m->m_gameSoundHdr->hdr.chantitle) : NULL;
	m->m_gameSoundHdr = NULL;

	if (banner == NULL || !banner->IsValid())
	{
		gprintf("no valid banner found\n");
		delete banner;
		return;
	}
	_extractBannerTitle(banner, GetLanguage(m->m_loc.getString(m->m_curLanguage, "gametdb_code", "EN").c_str()));

	const u8 *soundBin = banner->GetFile((char *) "sound.bin", &sndSize);
	delete banner;

	if (soundBin == NULL || (((IMD5Header *)soundBin)->fcc != 'IMD5' && ((IMD5Header *)soundBin)->fcc != 'RIFF'))
	{
		gprintf("Failed to load banner sound!\n\n");
		if(soundBin != NULL)
			delete soundBin;
		return;
	}

	m->m_gameSound.Load(soundBin, sndSize, false);
	m->m_gamesound_changed = true;
}

void CMenu::_playGameSound(void)
{
	m_gamesound_changed = false;
	if(m_bnrSndVol == 0) 
		return;

	CheckGameSoundThread();
	if(!gameSoundThreadStack.get())
		gameSoundThreadStack = smartMem2Alloc(gameSoundThreadStackSize);

	LWP_CreateThread(&m_gameSoundThread, (void *(*)(void *))CMenu::_gameSoundThread, (void *)this, gameSoundThreadStack.get(), gameSoundThreadStackSize, 60);
}

void CMenu::CheckGameSoundThread()
{
	if(LWP_ThreadIsSuspended(m_gameSoundThread))
		LWP_ResumeThread(m_gameSoundThread);

	LWP_JoinThread(m_gameSoundThread, NULL);
	m_gameSoundThread = LWP_THREAD_NULL;
}

void CMenu::ClearGameSoundThreadStack()
{
	if(gameSoundThreadStack.get())
		gameSoundThreadStack.release();
}
