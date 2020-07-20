
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <wchar.h>
#include <network.h>
#include <errno.h>

#include "menu.hpp"
#include "types.h"
#include "fonts.h"
#include "banner/BannerWindow.hpp"
#include "channel/nand.hpp"
#include "channel/nand_save.hpp"
#include "gc/gc.hpp"
#include "hw/Gekko.h"
#include "gui/WiiMovie.hpp"
#include "loader/alt_ios.h"
#include "loader/cios.h"
#include "loader/fs.h"
#include "loader/nk.h"
#include "loader/playlog.h"
#include "loader/wbfs.h"
#include "music/SoundHandler.hpp"
#include "network/gcard.h"
#include "unzip/U8Archive.h"

// Sounds
extern const u8 click_wav[];
extern const u32 click_wav_size;
extern const u8 hover_wav[];
extern const u32 hover_wav_size;
extern const u8 camera_wav[];
extern const u32 camera_wav_size;

CMenu mainMenu;

u8 CMenu::downloadStack[8192] ATTRIBUTE_ALIGN(32);
const u32 CMenu::downloadStackSize = 8192;

CMenu::CMenu()
{
	m_aa = 0;
	m_thrdWorking = false;
	m_thrdStop = false;
	m_thrdProgress = 0.f;
	m_thrdStep = 0.f;
	m_thrdStepLen = 0.f;
	m_locked = false;
	m_favorites = false;
	m_thrdNetwork = false;
	m_mutex = 0;
	m_showtimer = 0;
	m_gameSoundThread = LWP_THREAD_NULL;
	m_soundThrdBusy = false;
	m_numCFVersions = 0;
	m_bgCrossFade = 0;
	m_bnrSndVol = 0;
	m_directLaunch = false;
	m_exit = false;
	m_reload = false;
	m_gamesound_changed = false;
	m_video_playing = false;
	m_base_font = NULL;
	m_base_font_size = 0;
	m_wbf1_font = NULL;
	m_wbf2_font = NULL;
	m_current_view = COVERFLOW_WII;
	m_prevBg = NULL;
	m_nextBg = NULL;
	m_lqBg = NULL;
	m_use_sd_logging = false;
	m_use_wifi_gecko = false;
	m_init_network = false;
	m_use_source = true;
	m_sourceflow = false;
	m_clearCats = false;
	m_getFavs = true;
	m_catStartPage = 1;
	cacheCovers = false;
	SF_cacheCovers = true;
	m_snapshot_loaded = false;
	curCustBg = 1;
	/* Explorer stuff */
	m_txt_view = false;
	m_txt_path = NULL;
	/* download stuff */
	m_file = NULL;
	m_buffer = NULL;
	m_filesize = 0;
	/* thread stuff */
	m_thrdPtr = LWP_THREAD_NULL;
	m_thrdInstalling = false;
	m_thrdUpdated = false;
	m_thrdDone = false;
	m_thrdWritten = 0;
	m_thrdTotal = 0;
	/* screensaver */
	no_input_time = 0;
	/* Autoboot stuff */
	m_source_autoboot = false;
}

bool CMenu::init(bool usb_mounted)
{
	SoundHandle.Init();
	m_gameSound.SetVoice(1);
	/* Clear Playlog */
	Playlog_Delete();

	/* Find the first partition with apps/wiiflow folder */
	const char *drive = "empty";
	const char *check = "empty";
	struct stat dummy;
	for(int i = SD; i <= USB8; i++)
	{
		if(DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) != PART_FS_WBFS && stat(fmt("%s:/%s", DeviceName[i], APPS_DIR), &dummy) == 0)
		{
			drive = DeviceName[i];
			break;
		}
	}
	if(drive == check) // Should not happen
	{
		/* Could not find a device to save configuration files on! */
		return false;
	}

	_loadDefaultFont();// load default font

	/* Handle apps dir first, so handling wiiflow.ini does not fail */
	m_appDir = fmt("%s:/%s", drive, APPS_DIR);
	gprintf("Wiiflow boot.dol Location: %s\n", m_appDir.c_str());
	fsop_MakeFolder(m_appDir.c_str());
	
	/* Load/Create wiiflow.ini so we can get settings to start Gecko and Network */
	m_cfg.load(fmt("%s/" CFG_FILENAME, m_appDir.c_str()));
	show_mem = m_cfg.getBool("DEBUG", "show_mem", false);
	/* Check if we want WiFi Gecko */
	m_use_wifi_gecko = m_cfg.getBool("DEBUG", "wifi_gecko", false);
	WiFiDebugger.SetBuffer(m_use_wifi_gecko);
	/* Check if we want SD Gecko */
	m_use_sd_logging = m_cfg.getBool("DEBUG", "sd_write_log", false);
	LogToSD_SetBuffer(m_use_sd_logging);
	/* Init gamer tags now in case we need to init network on boot */
	m_cfg.setString("GAMERCARD", "gamercards", "wiinnertag");
	m_cfg.getString("GAMERCARD", "wiinnertag_url", WIINNERTAG_URL);
	m_cfg.getString("GAMERCARD", "wiinnertag_key", "");
	if (m_cfg.getBool("GAMERCARD", "gamercards_enable", false))
	{
		vector<string> gamercards = stringToVector(m_cfg.getString("GAMERCARD", "gamercards"), '|');
		if (gamercards.size() == 0)
		{
			gamercards.push_back("wiinnertag");
		}

		for (vector<string>::iterator itr = gamercards.begin(); itr != gamercards.end(); itr++)
		{
			gprintf("Found gamercard provider: %s\n",(*itr).c_str());
			register_card_provider(
				m_cfg.getString("GAMERCARD", fmt("%s_url", (*itr).c_str())).c_str(),
				m_cfg.getString("GAMERCARD", fmt("%s_key", (*itr).c_str())).c_str()
			);
		}
	}
	/* Init Network if wanted */
	m_init_network = (has_enabled_providers() || m_use_wifi_gecko);
	_netInit();
	
	/* Set SD only to off if any usb device is attached and format is FAT, NTFS, WBFS, or LINUX */
	m_cfg.getBool("GENERAL", "sd_only", true);// will only set it true if this doesn't already exist
	for(int i = USB1; i <= USB8; i++)
	{
		if(DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) >= 0)
			m_cfg.setBool("GENERAL", "sd_only", false);
	}

	/* Set data folder on same device as the apps/wiiflow folder */
	m_dataDir = fmt("%s:/%s", drive, APP_DATA_DIR);
	gprintf("Data Directory: %s\n", m_dataDir.c_str());
	
	/* Our Wii games dir */
	memset(wii_games_dir, 0, 64);
	strncpy(wii_games_dir, m_cfg.getString(WII_DOMAIN, "wii_games_dir", GAMES_DIR).c_str(), 63);
	if(strncmp(wii_games_dir, "%s:/", 4) != 0)
		strcpy(wii_games_dir, GAMES_DIR);
	gprintf("Wii Games Directory: %s\n", wii_games_dir);
	if(m_cfg.getBool(WII_DOMAIN, "prefer_usb", false))
	{
		if(usb_mounted)
			m_cfg.setInt(WII_DOMAIN, "partition", USB1);
		else
			m_cfg.setInt(WII_DOMAIN, "partition", SD);
	}
	
	/* GameCube stuff */
	m_devo_installed = DEVO_Installed(m_dataDir.c_str());
	m_nintendont_installed = Nintendont_Installed();
	memset(gc_games_dir, 0, 64);
	strncpy(gc_games_dir, m_cfg.getString(GC_DOMAIN, "gc_games_dir", DF_GC_GAMES_DIR).c_str(), 63);
	if(strncmp(gc_games_dir, "%s:/", 4) != 0)
		strcpy(gc_games_dir, DF_GC_GAMES_DIR);
	gprintf("GameCube Games Directory: %s\n", gc_games_dir);
	m_gc_play_banner_sound = m_cfg.getBool(GC_DOMAIN, "play_banner_sound", true);
	m_gc_play_default_sound = m_cfg.getBool(GC_DOMAIN, "play_default_sound", true);
	if(m_cfg.getBool(GC_DOMAIN, "prefer_usb", false))
	{
		if(usb_mounted)
			m_cfg.setInt(GC_DOMAIN, "partition", USB1);
		else
			m_cfg.setInt(GC_DOMAIN, "partition", SD);
	}
	
	/* Load cIOS Map */
	_installed_cios.clear();
	_load_installed_cioses();

	m_imgsDir = fmt("%s/imgs", m_appDir.c_str());
	m_binsDir = fmt("%s/bins", m_appDir.c_str());

	m_cacheDir = m_cfg.getString("GENERAL", "dir_cache", fmt("%s/cache", m_dataDir.c_str()));
	m_listCacheDir = m_cfg.getString("GENERAL", "dir_list_cache", fmt("%s/lists", m_cacheDir.c_str()));
	m_bnrCacheDir = m_cfg.getString("GENERAL", "dir_banner_cache", fmt("%s/banners", m_cacheDir.c_str()));
	m_customBnrDir = m_cfg.getString("GENERAL", "dir_custom_banners", fmt("%s/custom_banners", m_dataDir.c_str()));

	m_txtCheatDir = m_cfg.getString("GENERAL", "dir_txtcheat", fmt("%s/codes", m_dataDir.c_str()));
	m_cheatDir = m_cfg.getString("GENERAL", "dir_cheat", fmt("%s/gct", m_txtCheatDir.c_str()));
	m_wipDir = m_cfg.getString("GENERAL", "dir_wip", fmt("%s/wip", m_txtCheatDir.c_str()));

	m_settingsDir = m_cfg.getString("GENERAL", "dir_settings", fmt("%s/settings", m_dataDir.c_str()));
	m_languagesDir = m_cfg.getString("GENERAL", "dir_languages", fmt("%s/languages", m_dataDir.c_str()));
	m_helpDir = m_cfg.getString("GENERAL", "dir_help", fmt("%s/help", m_dataDir.c_str()));
	m_screenshotDir = m_cfg.getString("GENERAL", "dir_screenshot", fmt("%s/screenshots", m_dataDir.c_str()));
	
	m_boxPicDir = m_cfg.getString("GENERAL", "dir_box_covers", fmt("%s/boxcovers", m_dataDir.c_str()));
	m_picDir = m_cfg.getString("GENERAL", "dir_flat_covers", fmt("%s/covers", m_dataDir.c_str()));
	m_themeDir = m_cfg.getString("GENERAL", "dir_themes_lite", fmt("%s/themes_lite", m_dataDir.c_str()));
	m_coverflowsDir = m_cfg.getString("GENERAL", "dir_coverflows", fmt("%s/coverflows", m_themeDir.c_str()));
	m_musicDir = m_cfg.getString("GENERAL", "dir_music", fmt("%s/music", m_dataDir.c_str())); 
	m_videoDir = m_cfg.getString("GENERAL", "dir_trailers", fmt("%s/trailers", m_dataDir.c_str()));
	m_fanartDir = m_cfg.getString("GENERAL", "dir_fanart", fmt("%s/fanart", m_dataDir.c_str()));
	m_bckgrndsDir = m_cfg.getString("GENERAL", "dir_backgrounds", fmt("%s/backgrounds", m_dataDir.c_str()));
	
	m_sourceDir = m_cfg.getString("GENERAL", "dir_Source", fmt("%s/source_menu", m_dataDir.c_str()));
	m_pluginsDir = m_cfg.getString("GENERAL", "dir_plugins", fmt("%s/plugins", m_dataDir.c_str()));
	m_pluginDataDir = m_cfg.getString("GENERAL", "dir_plugins_data", fmt("%s/plugins_data", m_dataDir.c_str()));
	m_cartDir = m_cfg.getString("GENERAL", "dir_cart", fmt("%s/cart_disk", m_dataDir.c_str()));
	m_snapDir = m_cfg.getString("GENERAL", "dir_snap", fmt("%s/snapshots", m_dataDir.c_str()));


	/* Create our Folder Structure */
	fsop_MakeFolder(m_dataDir.c_str()); //D'OH!

	fsop_MakeFolder(m_cacheDir.c_str());
	fsop_MakeFolder(m_listCacheDir.c_str());
	fsop_MakeFolder(m_bnrCacheDir.c_str());
	fsop_MakeFolder(m_customBnrDir.c_str());

	fsop_MakeFolder(m_txtCheatDir.c_str());
	fsop_MakeFolder(m_cheatDir.c_str());
	fsop_MakeFolder(m_wipDir.c_str());

	fsop_MakeFolder(m_settingsDir.c_str());
	fsop_MakeFolder(m_languagesDir.c_str());
	fsop_MakeFolder(m_screenshotDir.c_str());
	fsop_MakeFolder(m_helpDir.c_str());

	fsop_MakeFolder(m_boxPicDir.c_str());
	fsop_MakeFolder(m_picDir.c_str());
	fsop_MakeFolder(m_themeDir.c_str());
	fsop_MakeFolder(m_coverflowsDir.c_str());
	fsop_MakeFolder(m_musicDir.c_str());
	fsop_MakeFolder(m_videoDir.c_str());
	fsop_MakeFolder(m_fanartDir.c_str());
	fsop_MakeFolder(m_bckgrndsDir.c_str());

	fsop_MakeFolder(m_sourceDir.c_str());
	fsop_MakeFolder(m_pluginsDir.c_str());
	fsop_MakeFolder(m_pluginDataDir.c_str());
	fsop_MakeFolder(m_cartDir.c_str());
	fsop_MakeFolder(m_snapDir.c_str());
	
	/* set default wii games partition in case this is the first boot */
	int dp = -1;
	for(int i = SD; i <= USB8; i++) // Find a wbfs folder or a partition of wbfs file system
	{
		if(DeviceHandle.IsInserted(i) && (DeviceHandle.GetFSType(i) == PART_FS_WBFS || stat(fmt(GAMES_DIR, DeviceName[i]), &dummy) == 0))
		{
			dp = i;
			break;
		}
	}
	if(dp < 0)// not found 
	{
		if(DeviceHandle.IsInserted(SD))// set to sd if inserted otherwise USB1
			dp = 0;
		else
			dp = 1;
	}
	u8 partition = m_cfg.getInt(WII_DOMAIN, "partition", dp);
	gprintf("Setting Wii games partition to: %i\n", partition);
	
	/* Emu nands init even if not being used */
	memset(emu_nands_dir, 0, sizeof(emu_nands_dir));
	strncpy(emu_nands_dir, IsOnWiiU() ? "vwiinands" : "nands", sizeof(emu_nands_dir) - 1);
	_checkEmuNandSettings();
	
	CoverFlow.init(m_base_font, m_base_font_size, m_vid.vid_50hz());

	/* Load categories and theme INI files */
	m_cat.load(fmt("%s/" CAT_FILENAME, m_settingsDir.c_str()));
	m_gcfg1.load(fmt("%s/" GAME_SETTINGS1_FILENAME, m_settingsDir.c_str()));
	m_themeName = m_cfg.getString("GENERAL", "theme", "default");
	m_themeDataDir = fmt("%s/%s", m_themeDir.c_str(), m_themeName.c_str());
	m_theme.load(fmt("%s.ini", m_themeDataDir.c_str()));
	m_coverflow.load(fmt("%s/%s.ini", m_coverflowsDir.c_str(), m_themeName.c_str()));
	if(!m_coverflow.loaded())
		m_coverflow.load(fmt("%s/default.ini", m_coverflowsDir.c_str()));
	m_platform.load(fmt("%s/platform.ini", m_pluginDataDir.c_str()));
	
	/* Init plugins */
	m_plugin.init(m_pluginsDir);
	vector<string> magics = m_cfg.getStrings(PLUGIN_DOMAIN, "enabled_plugins", ',');
	if(magics.size() > 0)
	{
		enabledPluginsCount = 0;
		string enabledMagics;
		for(u8 i = 0; i < magics.size(); i++)
		{
			u8 pos = m_plugin.GetPluginPosition(strtoul(magics[i].c_str(), NULL, 16));
			if(pos < 255)
			{
				enabledPluginsCount++;
				m_plugin.SetEnablePlugin(pos, 2);
				if(i == 0)
					enabledMagics = magics[0];
				else
					enabledMagics.append(',' + magics[i]);
			}
		}
		m_cfg.setString(PLUGIN_DOMAIN, "enabled_plugins", enabledMagics);
		magics.clear();
	}
	
	/* Set wiiflow language */
	const char *defLang = "Default";
	switch (CONF_GetLanguage())
	{
		case CONF_LANG_JAPANESE:
			defLang = "japanese";
			break;
		case CONF_LANG_GERMAN:
			defLang = "german";
			break;
		case CONF_LANG_FRENCH:
			defLang = "french";
			break;
		case CONF_LANG_SPANISH:
			defLang = "spanish";
			break;
		case CONF_LANG_ITALIAN:
			defLang = "italian";
			break;
		case CONF_LANG_DUTCH:
			defLang = "dutch";
			break;
		case CONF_LANG_SIMP_CHINESE:
			defLang = "chinese_s";
			break;
		case CONF_LANG_TRAD_CHINESE:
			defLang = "chinese_t";
			break;
		case CONF_LANG_KOREAN:
			defLang = "korean";
			break;
		case CONF_LANG_ENGLISH:
			defLang = "english";
			break;
	}
	if(CONF_GetArea() == CONF_AREA_BRA)
		defLang = "brazilian";

	m_curLanguage = m_cfg.getString("GENERAL", "language", defLang);
	if(!m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str())))
	{
		m_curLanguage = "Default";
		m_cfg.setString("GENERAL", "language", m_curLanguage.c_str());
		m_loc.unload();
	}

	/* Init gametdb and custom titles for game list making */
	m_cacheList.Init(m_settingsDir.c_str(), m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str(), m_pluginDataDir.c_str());

	/* Init the onscreen pointer */
	m_aa = 3;
	CColor pShadowColor = m_theme.getColor("GENERAL", "pointer_shadow_color", CColor(0x3F000000));
	float pShadowX = m_theme.getFloat("GENERAL", "pointer_shadow_x", 3.f);
	float pShadowY = m_theme.getFloat("GENERAL", "pointer_shadow_y", 3.f);
	bool pShadowBlur = m_theme.getBool("GENERAL", "pointer_shadow_blur", false);

	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		m_cursor[chan].init(fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GENERAL", fmt("pointer%i", chan+1)).c_str()),
			m_vid.wide(), pShadowColor, pShadowX, pShadowY, pShadowBlur, chan);
		WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
	}

	/* Init background Music Player and song info */
	MusicPlayer.Init(m_cfg, m_musicDir, fmt("%s/music", m_themeDataDir.c_str()));
	m_music_info = m_cfg.getBool("GENERAL", "display_music_info", false);
	MusicPlayer.SetResampleSetting(m_cfg.getBool("general", "resample_to_48khz", false));

	/* Init Button Manager and build the menus */
	_buildMenus();

	/* Check if locked, set return to, set exit to, and init multi threading */
	m_locked = m_cfg.getString("GENERAL", "parent_code", "").size() >= 4;
	
	/* Switch WFLA to DWFA in case they were using old wiiflow lite */
	if(m_cfg.getString("GENERAL", "returnto") == "WFLA")
		m_cfg.setString("GENERAL", "returnto", "DWFA");

	/* set WIIFLOW_DEF exit to option */
	/* 0 thru 2 of exit to enum (EXIT_TO_MENU, EXIT_TO_HBC, EXIT_TO_WIIU) in sys.h */
	int exit_to = min(max(0, m_cfg.getInt("GENERAL", "exit_to", 0)), (int)ARRAY_SIZE(CMenu::_exitTo) - 1);
	Sys_ExitTo(exit_to);

	LWP_MutexInit(&m_mutex, 0);

	/* set sound volumes */
	CoverFlow.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255));
	m_btnMgr.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_gui", 255));
	m_bnrSndVol = m_cfg.getInt("GENERAL", "sound_volume_bnr", 255);
	m_bnr_settings = m_cfg.getBool("GENERAL", "banner_in_settings", true);

	return true;
}

bool cleaned_up = false;

void CMenu::cleanup()
{
	if(cleaned_up)
		return;
	//gprintf("MEM1_freesize(): %i\nMEM2_freesize(): %i\n", MEM1_freesize(), MEM2_freesize());
	m_btnMgr.hide(m_mainLblCurMusic);
	_stopSounds();
	MusicPlayer.Cleanup();
	_cleanupDefaultFont();
	CoverFlow.shutdown(); /* possibly plugin flow crash so cleanup early */
	m_banner.DeleteBanner();
	m_plugin.Cleanup();
	m_source.unload();
	m_platform.unload();
	m_loc.unload();
	
	_Theme_Cleanup();
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		m_cursor[chan].cleanup();
		
	m_gameSound.FreeMemory();
	SoundHandle.Cleanup();
	soundDeinit();

	m_vid.cleanup();

	wiiLightOff();
	LWP_MutexDestroy(m_mutex);
	m_mutex = 0;

	cleaned_up = true;
	//gprintf(" \nMemory cleaned up\n");
	gprintf("MEM1_freesize(): %i\nMEM2_freesize(): %i\n", MEM1_freesize(), MEM2_freesize());
}

void CMenu::_Theme_Cleanup(void)
{
	/* Backgrounds */
	TexHandle.Cleanup(theme.bg);
	m_prevBg = NULL;
	m_nextBg = NULL;
	TexHandle.Cleanup(m_curBg);
	m_lqBg = NULL;
	TexHandle.Cleanup(m_mainCustomBg[0]);
	TexHandle.Cleanup(m_mainCustomBg[1]);
	/* Buttons */
	TexHandle.Cleanup(theme.btnTexL);
	TexHandle.Cleanup(theme.btnTexR);
	TexHandle.Cleanup(theme.btnTexC);
	TexHandle.Cleanup(theme.btnTexLS);
	TexHandle.Cleanup(theme.btnTexRS);
	TexHandle.Cleanup(theme.btnTexCS);
	TexHandle.Cleanup(theme.btnAUOn);
	TexHandle.Cleanup(theme.btnAUOns);
	TexHandle.Cleanup(theme.btnAUOff);
	TexHandle.Cleanup(theme.btnAUOffs);
	TexHandle.Cleanup(theme.btnENOn);
	TexHandle.Cleanup(theme.btnENOns);
	TexHandle.Cleanup(theme.btnENOff);
	TexHandle.Cleanup(theme.btnENOffs);
	TexHandle.Cleanup(theme.btnJAOn);
	TexHandle.Cleanup(theme.btnJAOns);
	TexHandle.Cleanup(theme.btnJAOff);
	TexHandle.Cleanup(theme.btnJAOffs);
	TexHandle.Cleanup(theme.btnFROn);
	TexHandle.Cleanup(theme.btnFROns);
	TexHandle.Cleanup(theme.btnFROff);
	TexHandle.Cleanup(theme.btnFROffs);
	TexHandle.Cleanup(theme.btnDEOn);
	TexHandle.Cleanup(theme.btnDEOns);
	TexHandle.Cleanup(theme.btnDEOff);
	TexHandle.Cleanup(theme.btnDEOffs);
	TexHandle.Cleanup(theme.btnESOn);
	TexHandle.Cleanup(theme.btnESOns);
	TexHandle.Cleanup(theme.btnESOff);
	TexHandle.Cleanup(theme.btnESOffs);
	TexHandle.Cleanup(theme.btnITOn);
	TexHandle.Cleanup(theme.btnITOns);
	TexHandle.Cleanup(theme.btnITOff);
	TexHandle.Cleanup(theme.btnITOffs);
	TexHandle.Cleanup(theme.btnNLOn);
	TexHandle.Cleanup(theme.btnNLOns);
	TexHandle.Cleanup(theme.btnNLOff);
	TexHandle.Cleanup(theme.btnNLOffs);
	TexHandle.Cleanup(theme.btnPTOn);
	TexHandle.Cleanup(theme.btnPTOns);
	TexHandle.Cleanup(theme.btnPTOff);
	TexHandle.Cleanup(theme.btnPTOffs);
	TexHandle.Cleanup(theme.btnRUOn);
	TexHandle.Cleanup(theme.btnRUOns);
	TexHandle.Cleanup(theme.btnRUOff);
	TexHandle.Cleanup(theme.btnRUOffs);
	TexHandle.Cleanup(theme.btnKOOn);
	TexHandle.Cleanup(theme.btnKOOns);
	TexHandle.Cleanup(theme.btnKOOff);
	TexHandle.Cleanup(theme.btnKOOffs);
	TexHandle.Cleanup(theme.btnZHCNOn);
	TexHandle.Cleanup(theme.btnZHCNOns);
	TexHandle.Cleanup(theme.btnZHCNOff);
	TexHandle.Cleanup(theme.btnZHCNOffs);
	TexHandle.Cleanup(theme.btnTexPlus);
	TexHandle.Cleanup(theme.btnTexPlusS);
	TexHandle.Cleanup(theme.btnTexMinus);
	TexHandle.Cleanup(theme.btnTexMinusS);
	/* Checkboxes */
	TexHandle.Cleanup(theme.checkboxoff);
	TexHandle.Cleanup(theme.checkboxoffs);
	TexHandle.Cleanup(theme.checkboxon);
	TexHandle.Cleanup(theme.checkboxons);
	TexHandle.Cleanup(theme.checkboxHid);
	TexHandle.Cleanup(theme.checkboxHids);
	TexHandle.Cleanup(theme.checkboxReq);
	TexHandle.Cleanup(theme.checkboxReqs);
	/* Progress Bars */
	TexHandle.Cleanup(theme.pbarTexL);
	TexHandle.Cleanup(theme.pbarTexR);
	TexHandle.Cleanup(theme.pbarTexC);
	TexHandle.Cleanup(theme.pbarTexLS);
	TexHandle.Cleanup(theme.pbarTexRS);
	TexHandle.Cleanup(theme.pbarTexCS);
	/* Other Theme Stuff */
	for(TexSet::iterator texture = theme.texSet.begin(); texture != theme.texSet.end(); ++texture)
		TexHandle.Cleanup(texture->second);
	for(vector<SFont>::iterator font = theme.fontSet.begin(); font != theme.fontSet.end(); ++font)
		font->ClearData();
	for(SoundSet::iterator sound = theme.soundSet.begin(); sound != theme.soundSet.end(); ++sound)
		sound->second->FreeMemory();
	theme.texSet.clear();
	theme.fontSet.clear();
	theme.soundSet.clear();
	m_theme.unload();
	m_coverflow.unload();
}

void CMenu::_setAA(int aa)
{
	switch (aa)
	{
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 8:
			m_aa = aa;
			break;
		case 7:
			m_aa = 6;
			break;
		default:
			m_aa = 0;
	}
}

void CMenu::_loadCFCfg()
{
	const char *domain = "_COVERFLOW";

	//gprintf("Preparing to load sounds from %s\n", m_themeDataDir.c_str());
	CoverFlow.setCachePath(m_cacheDir.c_str(), m_cfg.getBool(PLUGIN_DOMAIN, "subfolder_cache", true));
	CoverFlow.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 20));
	// Coverflow Sounds
	CoverFlow.setSounds(
		new GuiSound(fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_flip").c_str())),
		_sound(theme.soundSet, m_theme.getString(domain, "sound_hover", "").c_str(),  hover_wav, hover_wav_size, "default_btn_hover", false),
		_sound(theme.soundSet, m_theme.getString(domain, "sound_select", "").c_str(), click_wav, click_wav_size, "default_btn_click", false),
		new GuiSound(fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_cancel").c_str()))
	);

	// Textures
	string texLoading = fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_box").c_str());
	string texNoCover = fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_box").c_str());
	string texLoadingFlat = fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_flat").c_str());
	string texNoCoverFlat = fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_flat").c_str());
	CoverFlow.setTextures(texLoading, texLoadingFlat, texNoCover, texNoCoverFlat);
	// Font
	CoverFlow.setFont(_font(domain, "font", theme.titleFont), m_theme.getColor(domain, "font_color", CColor(0xFFFFFFFF)));
}

Vector3D CMenu::_getCFV3D(const string &domain, const string &key, const Vector3D &def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_coverflow.has(domain, key169))
	{
		Vector3D v(m_coverflow.getVector3D(domain, key169));
		m_coverflow.getVector3D(domain, key43, v);
		return v;
	}
	return m_coverflow.getVector3D(domain, key169, m_coverflow.getVector3D(domain, key43, def));
}

int CMenu::_getCFInt(const string &domain, const string &key, int def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_coverflow.has(domain, key169))
	{
		int v = m_coverflow.getInt(domain, key169);
		m_coverflow.getInt(domain, key43, v);
		return v;
	}
	return m_coverflow.getInt(domain, key169, m_coverflow.getInt(domain, key43, def));
}

float CMenu::_getCFFloat(const string &domain, const string &key, float def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_coverflow.has(domain, key169))
	{
		float v = m_coverflow.getFloat(domain, key169);
		m_coverflow.getFloat(domain, key43, v);
		return v;
	}
	return m_coverflow.getFloat(domain, key169, m_coverflow.getFloat(domain, key43, def));
}

void CMenu::_loadCFLayout(int version, bool forceAA, bool otherScrnFmt)
{
	string domain = fmt("%s_%i", cf_domain, version);
	string domainSel = fmt("%s_%i_S", cf_domain, version);
	bool smallflow = strcasecmp(cf_domain, "_SMALLFLOW") == 0;
	bool sf = otherScrnFmt;

	int max_fsaa = m_coverflow.getInt(domain, "max_fsaa", 3);
	_setAA(forceAA ? max_fsaa : min(max_fsaa, m_cfg.getInt("GENERAL", "max_fsaa", 3)));

	CoverFlow.setTextureQuality(m_coverflow.getFloat(domain, "tex_lod_bias", -3.f),
		m_coverflow.getInt(domain, "tex_aniso", 2),
		m_coverflow.getBool(domain, "tex_edge_lod", true));

	CoverFlow.setRange(_getCFInt(domain, "rows", (smallflow ? 5 : 1), sf), _getCFInt(domain, "columns", 11, sf));

	CoverFlow.setCameraPos(false,
		_getCFV3D(domain, "camera_pos", Vector3D(0.f, 0.f, 5.f), sf),
		_getCFV3D(domain, "camera_aim", Vector3D(0.f, 0.f, 0.f), sf));

	CoverFlow.setCameraPos(true,
		_getCFV3D(domainSel, "camera_pos", Vector3D(0.f, 0.f, 5.f), sf),
		_getCFV3D(domainSel, "camera_aim", Vector3D(0.f, 0.f, 0.f), sf));

	CoverFlow.setCameraOsc(false,
		_getCFV3D(domain, "camera_osc_speed", Vector3D(2.f, 0.f, 0.f), sf),
		_getCFV3D(domain, "camera_osc_amp", Vector3D(0.1f, 0.f, 0.f), sf));

	CoverFlow.setCameraOsc(true,
		_getCFV3D(domainSel, "camera_osc_speed", Vector3D(), sf),
		_getCFV3D(domainSel, "camera_osc_amp", Vector3D(), sf));

	float def_cvr_posX = smallflow ? 1.f : 1.6f;
	float def_cvr_posY = smallflow ? -0.8f : -1.f;
	CoverFlow.setCoverPos(false,
		_getCFV3D(domain, "left_pos", Vector3D(-def_cvr_posX, def_cvr_posY, 0.f), sf),
		_getCFV3D(domain, "right_pos", Vector3D(def_cvr_posX, def_cvr_posY, 0.f), sf),
		_getCFV3D(domain, "center_pos", Vector3D(0.f, def_cvr_posY, 1.f), sf),
		_getCFV3D(domain, "row_center_pos", Vector3D(0.f, def_cvr_posY, 0.f), sf));

	if(smallflow)
		CoverFlow.setCoverPos(true, 
			_getCFV3D(domainSel, "left_pos", Vector3D(-4.05f, -0.6f, -1.f), sf),
			_getCFV3D(domainSel, "right_pos", Vector3D(3.35f, -0.6f, -1.f), sf),
			_getCFV3D(domainSel, "center_pos", Vector3D(-0.5f, -0.8f, 2.6f), sf),
			_getCFV3D(domainSel, "row_center_pos", Vector3D(-3.05f, -0.6f, -1.f), sf));
	else
		CoverFlow.setCoverPos(true,
			_getCFV3D(domainSel, "left_pos", Vector3D(-4.7f, -1.f, 0.f), sf),
			_getCFV3D(domainSel, "right_pos", Vector3D(4.7f, -1.f, 0.f), sf),
			_getCFV3D(domainSel, "center_pos", Vector3D(-0.6f, -1.f, 2.6f), sf),
			_getCFV3D(domainSel, "row_center_pos", Vector3D(0.f, 0.f, 0.f), sf));

	CoverFlow.setCoverAngleOsc(false,
		m_coverflow.getVector3D(domain, "cover_osc_speed", Vector3D(2.f, 2.f, 0.f)),
		m_coverflow.getVector3D(domain, "cover_osc_amp", Vector3D(5.f, 10.f, 0.f)));

	CoverFlow.setCoverAngleOsc(true,
		m_coverflow.getVector3D(domainSel, "cover_osc_speed", Vector3D(2.1f, 2.1f, 0.f)),
		m_coverflow.getVector3D(domainSel, "cover_osc_amp", Vector3D(2.f, 5.f, 0.f)));

	CoverFlow.setCoverPosOsc(false,
		m_coverflow.getVector3D(domain, "cover_pos_osc_speed"),
		m_coverflow.getVector3D(domain, "cover_pos_osc_amp"));

	CoverFlow.setCoverPosOsc(true,
		m_coverflow.getVector3D(domainSel, "cover_pos_osc_speed"),
		m_coverflow.getVector3D(domainSel, "cover_pos_osc_amp"));

	float spacerX = smallflow ? 1.f : 0.35f;
	CoverFlow.setSpacers(false,
		m_coverflow.getVector3D(domain, "left_spacer", Vector3D(-spacerX, 0.f, 0.f)),
		m_coverflow.getVector3D(domain, "right_spacer", Vector3D(spacerX, 0.f, 0.f)));

	CoverFlow.setSpacers(true,
		m_coverflow.getVector3D(domainSel, "left_spacer", Vector3D(-spacerX, 0.f, 0.f)),
		m_coverflow.getVector3D(domainSel, "right_spacer", Vector3D(spacerX, 0.f, 0.f)));

	CoverFlow.setDeltaAngles(false,
		m_coverflow.getVector3D(domain, "left_delta_angle"),
		m_coverflow.getVector3D(domain, "right_delta_angle"));

	CoverFlow.setDeltaAngles(true,
		m_coverflow.getVector3D(domainSel, "left_delta_angle"),
		m_coverflow.getVector3D(domainSel, "right_delta_angle"));

	float angleY = smallflow ? 0.f : 70.f;
	CoverFlow.setAngles(false,
		m_coverflow.getVector3D(domain, "left_angle", Vector3D(0.f, angleY, 0.f)),
		m_coverflow.getVector3D(domain, "right_angle", Vector3D(0.f, -angleY, 0.f)),
		m_coverflow.getVector3D(domain, "center_angle", Vector3D(0.f, 0.f, 0.f)),
		m_coverflow.getVector3D(domain, "row_center_angle"));

	angleY = smallflow ? 0.f : 90.f;
	float angleY1 = smallflow ? 0.f : 360.f;
	float angleX = smallflow ? 0.f : -45.f;
	CoverFlow.setAngles(true,
		m_coverflow.getVector3D(domainSel, "left_angle", Vector3D(angleX, angleY, 0.f)),
		m_coverflow.getVector3D(domainSel, "right_angle", Vector3D(angleX, -angleY, 0.f)),
		m_coverflow.getVector3D(domainSel, "center_angle", Vector3D(0.f, angleY1, 0.f)),
		m_coverflow.getVector3D(domainSel, "row_center_angle"));

	angleX = smallflow ? 0.f : 20.f;
	CoverFlow.setTitleAngles(false,
		_getCFFloat(domain, "text_left_angle", -angleX, sf),
		_getCFFloat(domain, "text_right_angle", angleX, sf),
		_getCFFloat(domain, "text_center_angle", 0.f, sf));

	CoverFlow.setTitleAngles(true,
		_getCFFloat(domainSel, "text_left_angle", -angleX, sf),
		_getCFFloat(domainSel, "text_right_angle", angleX, sf),
		_getCFFloat(domainSel, "text_center_angle", 0.f, sf));

	def_cvr_posX = smallflow ? 2.f : 1.f;
	CoverFlow.setTitlePos(false,
		_getCFV3D(domain, "text_left_pos", Vector3D(-def_cvr_posX, 0.8f, 2.6f), sf),
		_getCFV3D(domain, "text_right_pos", Vector3D(def_cvr_posX, 0.8f, 2.6f), sf),
		_getCFV3D(domain, "text_center_pos", Vector3D(0.f, 0.8f, 2.6f), sf));

	def_cvr_posX = smallflow ? .6f : 2.1f;
	CoverFlow.setTitlePos(true,
		_getCFV3D(domainSel, "text_left_pos", Vector3D(-4.f, 0.f, 1.3f), sf),
		_getCFV3D(domainSel, "text_right_pos", Vector3D(4.f, 0.f, 1.3f), sf),
		_getCFV3D(domainSel, "text_center_pos", Vector3D(def_cvr_posX, 1.f, 1.6f), sf));

	CoverFlow.setTitleWidth(false,
		_getCFFloat(domain, "text_side_wrap_width", 500.f, sf),
		_getCFFloat(domain, "text_center_wrap_width", 500.f, sf));

	CoverFlow.setTitleWidth(true,
		_getCFFloat(domainSel, "text_side_wrap_width", 500.f, sf),
		_getCFFloat(domainSel, "text_center_wrap_width", 390.f, sf));

	CoverFlow.setTitleStyle(false,
		_textStyle(domain.c_str(), "text_side_style", FTGX_ALIGN_MIDDLE | FTGX_JUSTIFY_CENTER, true),
		_textStyle(domain.c_str(), "text_center_style", FTGX_ALIGN_MIDDLE | FTGX_JUSTIFY_CENTER, true));

	CoverFlow.setTitleStyle(true,
		_textStyle(domainSel.c_str(), "text_side_style", FTGX_ALIGN_MIDDLE | FTGX_JUSTIFY_CENTER, true),
		_textStyle(domainSel.c_str(), "text_center_style", FTGX_ALIGN_TOP | FTGX_JUSTIFY_RIGHT, true));

	CoverFlow.setColors(false,
		m_coverflow.getColor(domain, "color_beg", 0xCFFFFFFF),
		m_coverflow.getColor(domain, "color_end", 0x3FFFFFFF),
		m_coverflow.getColor(domain, "color_off", 0x7FFFFFFF));

	CoverFlow.setColors(true,
		m_coverflow.getColor(domainSel, "color_beg", 0x7FFFFFFF),
		m_coverflow.getColor(domainSel, "color_end", 0x1FFFFFFF),
		m_coverflow.getColor(domain, "color_off", 0x7FFFFFFF));	// Mouse not used once a selection has been made

	CoverFlow.setMirrorAlpha(m_coverflow.getFloat(domain, "mirror_alpha", 0.15f), m_coverflow.getFloat(domain, "title_mirror_alpha", 0.03f));	// Doesn't depend on selection

	CoverFlow.setMirrorBlur(m_coverflow.getBool(domain, "mirror_blur", true));	// Doesn't depend on selection

	CoverFlow.setShadowColors(false,
		m_coverflow.getColor(domain, "color_shadow_center", 0x00000000),
		m_coverflow.getColor(domain, "color_shadow_beg", 0x00000000),
		m_coverflow.getColor(domain, "color_shadow_end", 0x00000000),
		m_coverflow.getColor(domain, "color_shadow_off", 0x00000000));

	CoverFlow.setShadowColors(true,
		m_coverflow.getColor(domainSel, "color_shadow_center", 0x0000007F),
		m_coverflow.getColor(domainSel, "color_shadow_beg", 0x0000007F),
		m_coverflow.getColor(domainSel, "color_shadow_end", 0x0000007F),
		m_coverflow.getColor(domainSel, "color_shadow_off", 0x0000007F));

	CoverFlow.setShadowPos(m_coverflow.getFloat(domain, "shadow_scale", 1.1f),
		m_coverflow.getFloat(domain, "shadow_x"),
		m_coverflow.getFloat(domain, "shadow_y"));
	
	float spacerY = smallflow ? 0.60f : 2.f; 
	CoverFlow.setRowSpacers(false,
		m_coverflow.getVector3D(domain, "top_spacer", Vector3D(0.f, spacerY, 0.f)),
		m_coverflow.getVector3D(domain, "bottom_spacer", Vector3D(0.f, -spacerY, 0.f)));

	CoverFlow.setRowSpacers(true,
		m_coverflow.getVector3D(domainSel, "top_spacer", Vector3D(0.f, spacerY, 0.f)),
		m_coverflow.getVector3D(domainSel, "bottom_spacer", Vector3D(0.f, -spacerY, 0.f)));

	CoverFlow.setRowDeltaAngles(false,
		m_coverflow.getVector3D(domain, "top_delta_angle"),
		m_coverflow.getVector3D(domain, "bottom_delta_angle"));

	CoverFlow.setRowDeltaAngles(true,
		m_coverflow.getVector3D(domainSel, "top_delta_angle"),
		m_coverflow.getVector3D(domainSel, "bottom_delta_angle"));

	CoverFlow.setRowAngles(false,
		m_coverflow.getVector3D(domain, "top_angle"),
		m_coverflow.getVector3D(domain, "bottom_angle"));

	CoverFlow.setRowAngles(true,
		m_coverflow.getVector3D(domainSel, "top_angle"),
		m_coverflow.getVector3D(domainSel, "bottom_angle"));

	Vector3D def_cvr_scale = smallflow ? Vector3D(0.66f, 0.25f, 1.f) : Vector3D(1.f, 1.f, 1.f);

	CoverFlow.setCoverScale(false,
		m_coverflow.getVector3D(domain, "left_scale", def_cvr_scale),
		m_coverflow.getVector3D(domain, "right_scale", def_cvr_scale),
		m_coverflow.getVector3D(domain, "center_scale", def_cvr_scale),
		m_coverflow.getVector3D(domain, "row_center_scale", def_cvr_scale));

	CoverFlow.setCoverScale(true,
		m_coverflow.getVector3D(domainSel, "left_scale", def_cvr_scale),
		m_coverflow.getVector3D(domainSel, "right_scale", def_cvr_scale),
		m_coverflow.getVector3D(domainSel, "center_scale", def_cvr_scale),
		m_coverflow.getVector3D(domainSel, "row_center_scale", def_cvr_scale));

	float flipX = smallflow ? 360.f : 180.f;
	CoverFlow.setCoverFlipping(
		_getCFV3D(domainSel, "flip_pos", Vector3D(), sf),
		_getCFV3D(domainSel, "flip_angle", Vector3D(0.f, flipX, 0.f), sf),
		_getCFV3D(domainSel, "flip_scale", Vector3D(1.f, 1.f, 1.f), sf));

	CoverFlow.setBlur(
		m_coverflow.getInt(domain, "blur_resolution", 1),
		m_coverflow.getInt(domain, "blur_radius", 2),
		m_coverflow.getFloat(domain, "blur_factor", 1.f));
}

void CMenu::_buildMenus(void)
{
	m_btnMgr.init();
	m_btnMgr.setRumble(m_cfg.getBool("GENERAL", "rumble", true));
	// Default fonts
	theme.btnFont = _dfltFont(BUTTONFONT);
	theme.btnFontColor = m_theme.getColor("GENERAL", "button_font_color", 0xD0BFDFFF);

	theme.lblFont = _dfltFont(LABELFONT);
	theme.lblFontColor = m_theme.getColor("GENERAL", "label_font_color", 0xD0BFDFFF);

	theme.titleFont = _dfltFont(TITLEFONT);
	theme.titleFontColor = m_theme.getColor("GENERAL", "title_font_color", 0xFFFFFFFF);

	theme.txtFont = _dfltFont(TEXTFONT);
	theme.txtFontColor = m_theme.getColor("GENERAL", "text_font_color", 0xFFFFFFFF);

	// Default Sounds
	theme.clickSound	= _sound(theme.soundSet, m_theme.getString("GENERAL", "click_sound", "").c_str(), click_wav, click_wav_size, "default_btn_click", false);
	theme.hoverSound	= _sound(theme.soundSet, m_theme.getString("GENERAL", "hover_sound", "").c_str(), hover_wav, hover_wav_size, "default_btn_hover", false);
	theme.cameraSound	= _sound(theme.soundSet, m_theme.getString("GENERAL", "camera_sound", "").c_str(), camera_wav, camera_wav_size, "default_camera", false);

	// Default textures
	TexHandle.fromImageFile(theme.btnTexL, fmt("%s/butleft.png", m_imgsDir.c_str()));
	theme.btnTexL = _texture("GENERAL", "button_texture_left", theme.btnTexL); 
	TexHandle.fromImageFile(theme.btnTexR, fmt("%s/butright.png", m_imgsDir.c_str()));
	theme.btnTexR = _texture("GENERAL", "button_texture_right", theme.btnTexR); 
	TexHandle.fromImageFile(theme.btnTexC, fmt("%s/butcenter.png", m_imgsDir.c_str()));
	theme.btnTexC = _texture("GENERAL", "button_texture_center", theme.btnTexC); 
	TexHandle.fromImageFile(theme.btnTexLS, fmt("%s/butsleft.png", m_imgsDir.c_str()));
	theme.btnTexLS = _texture("GENERAL", "button_texture_left_selected", theme.btnTexLS); 
	TexHandle.fromImageFile(theme.btnTexRS, fmt("%s/butsright.png", m_imgsDir.c_str()));
	theme.btnTexRS = _texture("GENERAL", "button_texture_right_selected", theme.btnTexRS); 
	TexHandle.fromImageFile(theme.btnTexCS, fmt("%s/butscenter.png", m_imgsDir.c_str()));
	theme.btnTexCS = _texture("GENERAL", "button_texture_center_selected", theme.btnTexCS); 

	/* Language Buttons */
	u32 img_buf_size = 0;
	u8 *img_buf = fsop_ReadFile(fmt("%s/butauon.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnAUOn, img_buf);
		TexHandle.fromPNG(theme.btnAUOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnAUOn = _texture("GENERAL", "button_au_on", theme.btnAUOn);
	theme.btnAUOff = _texture("GENERAL", "button_au_off", theme.btnAUOff);

	img_buf = fsop_ReadFile(fmt("%s/butauons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnAUOns, img_buf);
		TexHandle.fromPNG(theme.btnAUOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnAUOns = _texture("GENERAL", "button_au_on_selected", theme.btnAUOns);
	theme.btnAUOffs = _texture("GENERAL", "button_au_off_selected", theme.btnAUOffs);

	img_buf = fsop_ReadFile(fmt("%s/butenon.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnENOn, img_buf);
		TexHandle.fromPNG(theme.btnENOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnENOn = _texture("GENERAL", "button_en_on", theme.btnENOn);
	theme.btnENOff = _texture("GENERAL", "button_en_off", theme.btnENOff);

	img_buf = fsop_ReadFile(fmt("%s/butenons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnENOns, img_buf);
		TexHandle.fromPNG(theme.btnENOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnENOns = _texture("GENERAL", "button_en_on_selected", theme.btnENOns);
	theme.btnENOffs = _texture("GENERAL", "button_en_off_selected", theme.btnENOffs);

	img_buf = fsop_ReadFile(fmt("%s/butjaon.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnJAOn, img_buf);
		TexHandle.fromPNG(theme.btnJAOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnJAOn = _texture("GENERAL", "button_ja_on", theme.btnJAOn);
	theme.btnJAOff = _texture("GENERAL", "button_ja_off", theme.btnJAOff);

	img_buf = fsop_ReadFile(fmt("%s/butjaons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnJAOns, img_buf);
		TexHandle.fromPNG(theme.btnJAOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnJAOns = _texture("GENERAL", "button_ja_on_selected", theme.btnJAOns);
	theme.btnJAOffs = _texture("GENERAL", "button_ja_off_selected", theme.btnJAOffs);

	img_buf = fsop_ReadFile(fmt("%s/butfron.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnFROn, img_buf);
		TexHandle.fromPNG(theme.btnFROff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnFROn = _texture("GENERAL", "button_fr_on", theme.btnFROn);
	theme.btnFROff = _texture("GENERAL", "button_fr_off", theme.btnFROff);

	img_buf = fsop_ReadFile(fmt("%s/butfrons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnFROns, img_buf);
		TexHandle.fromPNG(theme.btnFROffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnFROns = _texture("GENERAL", "button_fr_on_selected", theme.btnFROns);
	theme.btnFROffs = _texture("GENERAL", "button_fr_off_selected", theme.btnFROffs);

	img_buf = fsop_ReadFile(fmt("%s/butdeon.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnDEOn, img_buf);
		TexHandle.fromPNG(theme.btnDEOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnDEOn = _texture("GENERAL", "button_de_on", theme.btnDEOn);
	theme.btnDEOff = _texture("GENERAL", "button_de_off", theme.btnDEOff);

	img_buf = fsop_ReadFile(fmt("%s/butdeons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnDEOns, img_buf);
		TexHandle.fromPNG(theme.btnDEOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnDEOns = _texture("GENERAL", "button_de_on_selected", theme.btnDEOns);
	theme.btnDEOffs = _texture("GENERAL", "button_de_off_selected", theme.btnDEOffs);

	img_buf = fsop_ReadFile(fmt("%s/buteson.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnESOn, img_buf);
		TexHandle.fromPNG(theme.btnESOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnESOn = _texture("GENERAL", "button_es_on", theme.btnESOn);
	theme.btnESOff = _texture("GENERAL", "button_es_off", theme.btnESOff);

	img_buf = fsop_ReadFile(fmt("%s/butesons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnESOns, img_buf);
		TexHandle.fromPNG(theme.btnESOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnESOns = _texture("GENERAL", "button_es_on_selected", theme.btnESOns);
	theme.btnESOffs = _texture("GENERAL", "button_es_off_selected", theme.btnESOffs);

	img_buf = fsop_ReadFile(fmt("%s/butiton.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnITOn, img_buf);
		TexHandle.fromPNG(theme.btnITOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnITOn = _texture("GENERAL", "button_it_on", theme.btnITOn);
	theme.btnITOff = _texture("GENERAL", "button_it_off", theme.btnITOff);

	img_buf = fsop_ReadFile(fmt("%s/butitons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnITOns, img_buf);
		TexHandle.fromPNG(theme.btnITOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnITOns = _texture("GENERAL", "button_it_on_selected", theme.btnITOns);
	theme.btnITOffs = _texture("GENERAL", "button_it_off_selected", theme.btnITOffs);

	img_buf = fsop_ReadFile(fmt("%s/butnlon.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnNLOn, img_buf);
		TexHandle.fromPNG(theme.btnNLOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnNLOn = _texture("GENERAL", "button_nl_on", theme.btnNLOn);
	theme.btnNLOff = _texture("GENERAL", "button_nl_off", theme.btnNLOff);

	img_buf = fsop_ReadFile(fmt("%s/butnlons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnNLOns, img_buf);
		TexHandle.fromPNG(theme.btnNLOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnNLOns = _texture("GENERAL", "button_nl_on_selected", theme.btnNLOns);
	theme.btnNLOffs = _texture("GENERAL", "button_nl_off_selected", theme.btnNLOffs);

	img_buf = fsop_ReadFile(fmt("%s/butpton.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnPTOn, img_buf);
		TexHandle.fromPNG(theme.btnPTOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnPTOn = _texture("GENERAL", "button_pt_on", theme.btnPTOn);
	theme.btnPTOff = _texture("GENERAL", "button_pt_off", theme.btnPTOff);

	img_buf = fsop_ReadFile(fmt("%s/butptons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnPTOns, img_buf);
		TexHandle.fromPNG(theme.btnPTOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnPTOns = _texture("GENERAL", "button_pt_on_selected", theme.btnPTOns);
	theme.btnPTOffs = _texture("GENERAL", "button_pt_off_selected", theme.btnPTOffs);

	img_buf = fsop_ReadFile(fmt("%s/butruon.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnRUOn, img_buf);
		TexHandle.fromPNG(theme.btnRUOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnRUOn = _texture("GENERAL", "button_ru_on", theme.btnRUOn);
	theme.btnRUOff = _texture("GENERAL", "button_ru_off", theme.btnRUOff);

	img_buf = fsop_ReadFile(fmt("%s/butruons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnRUOns, img_buf);
		TexHandle.fromPNG(theme.btnRUOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnRUOns = _texture("GENERAL", "button_ru_on_selected", theme.btnRUOns);
	theme.btnRUOffs = _texture("GENERAL", "button_ru_off_selected", theme.btnRUOffs);

	img_buf = fsop_ReadFile(fmt("%s/butkoon.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnKOOn, img_buf);
		TexHandle.fromPNG(theme.btnKOOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnKOOn = _texture("GENERAL", "button_ko_on", theme.btnKOOn);
	theme.btnKOOff = _texture("GENERAL", "button_ko_off", theme.btnKOOff);

	img_buf = fsop_ReadFile(fmt("%s/butkoons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnKOOns, img_buf);
		TexHandle.fromPNG(theme.btnKOOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnKOOns = _texture("GENERAL", "button_ko_on_selected", theme.btnKOOns);
	theme.btnKOOffs = _texture("GENERAL", "button_ko_off_selected", theme.btnKOOffs);

	img_buf = fsop_ReadFile(fmt("%s/butzhcnon.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnZHCNOn, img_buf);
		TexHandle.fromPNG(theme.btnZHCNOff, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnZHCNOn = _texture("GENERAL", "button_zhcn_on", theme.btnZHCNOn);
	theme.btnZHCNOff = _texture("GENERAL", "button_zhcn_off", theme.btnZHCNOff);

	img_buf = fsop_ReadFile(fmt("%s/butzhcnons.png", m_imgsDir.c_str()), &img_buf_size);
	if(img_buf !=  NULL)
	{
		TexHandle.fromPNG(theme.btnZHCNOns, img_buf);
		TexHandle.fromPNG(theme.btnZHCNOffs, img_buf, GX_TF_RGBA8, 0, 0, true);
		MEM2_free(img_buf);
	}
	theme.btnZHCNOns = _texture("GENERAL", "button_zhcn_on_selected", theme.btnZHCNOns);
	theme.btnZHCNOffs = _texture("GENERAL", "button_zhcn_off_selected", theme.btnZHCNOffs);

	/* Default textures */
	TexHandle.fromImageFile(theme.checkboxoff, fmt("%s/checkboxoff.png", m_imgsDir.c_str()));
	theme.checkboxoff = _texture("GENERAL", "checkbox_off", theme.checkboxoff);
	TexHandle.fromImageFile(theme.checkboxoffs, fmt("%s/checkboxoffs.png", m_imgsDir.c_str()));
	theme.checkboxoffs = _texture("GENERAL", "checkbox_off_selected", theme.checkboxoffs);
	TexHandle.fromImageFile(theme.checkboxon, fmt("%s/checkboxon.png", m_imgsDir.c_str()));
	theme.checkboxon = _texture("GENERAL", "checkbox_on", theme.checkboxon);
	TexHandle.fromImageFile(theme.checkboxons, fmt("%s/checkboxons.png", m_imgsDir.c_str()));
	theme.checkboxons = _texture("GENERAL", "checkbox_on_selected", theme.checkboxons);
	TexHandle.fromImageFile(theme.checkboxHid, fmt("%s/checkboxhid.png", m_imgsDir.c_str()));
	theme.checkboxHid = _texture("GENERAL", "checkbox_Hid", theme.checkboxHid);
	TexHandle.fromImageFile(theme.checkboxHids, fmt("%s/checkboxhids.png", m_imgsDir.c_str()));
	theme.checkboxHids = _texture("GENERAL", "checkbox_Hid_selected", theme.checkboxHids);
	TexHandle.fromImageFile(theme.checkboxReq, fmt("%s/checkboxreq.png", m_imgsDir.c_str()));
	theme.checkboxReq = _texture("GENERAL", "checkbox_Req", theme.checkboxReq);
	TexHandle.fromImageFile(theme.checkboxReqs, fmt("%s/checkboxreqs.png", m_imgsDir.c_str()));
	theme.checkboxReqs = _texture("GENERAL", "checkbox_Req_selected", theme.checkboxReqs);

	TexHandle.fromImageFile(theme.pbarTexL, fmt("%s/pbarleft.png", m_imgsDir.c_str()));
	theme.pbarTexL = _texture("GENERAL", "progressbar_texture_left", theme.pbarTexL);
	TexHandle.fromImageFile(theme.pbarTexR, fmt("%s/pbarright.png", m_imgsDir.c_str()));
	theme.pbarTexR = _texture("GENERAL", "progressbar_texture_right", theme.pbarTexR);
	TexHandle.fromImageFile(theme.pbarTexC, fmt("%s/pbarcenter.png", m_imgsDir.c_str()));
	theme.pbarTexC = _texture("GENERAL", "progressbar_texture_center", theme.pbarTexC);
	TexHandle.fromImageFile(theme.pbarTexLS, fmt("%s/pbarlefts.png", m_imgsDir.c_str()));
	theme.pbarTexLS = _texture("GENERAL", "progressbar_texture_left_selected", theme.pbarTexLS);
	TexHandle.fromImageFile(theme.pbarTexRS, fmt("%s/pbarrights.png", m_imgsDir.c_str()));
	theme.pbarTexRS = _texture("GENERAL", "progressbar_texture_right_selected", theme.pbarTexRS);
	TexHandle.fromImageFile(theme.pbarTexCS, fmt("%s/pbarcenters.png", m_imgsDir.c_str()));
	theme.pbarTexCS = _texture("GENERAL", "progressbar_texture_center_selected", theme.pbarTexCS);
	TexHandle.fromImageFile(theme.btnTexPlus, fmt("%s/btnplus.png", m_imgsDir.c_str()));
	theme.btnTexPlus = _texture("GENERAL", "plus_button_texture", theme.btnTexPlus);
	TexHandle.fromImageFile(theme.btnTexPlusS, fmt("%s/btnpluss.png", m_imgsDir.c_str()));
	theme.btnTexPlusS = _texture("GENERAL", "plus_button_texture_selected", theme.btnTexPlusS);
	TexHandle.fromImageFile(theme.btnTexMinus, fmt("%s/btnminus.png", m_imgsDir.c_str()));
	theme.btnTexMinus = _texture("GENERAL", "minus_button_texture", theme.btnTexMinus);
	TexHandle.fromImageFile(theme.btnTexMinusS, fmt("%s/btnminuss.png", m_imgsDir.c_str()));
	theme.btnTexMinusS = _texture("GENERAL", "minus_button_texture_selected", theme.btnTexMinusS);

	// Default background
	TexHandle.fromImageFile(theme.bg, fmt("%s/background.png", m_imgsDir.c_str()));
	TexHandle.fromImageFile(m_mainBgLQ, fmt("%s/background.png", m_imgsDir.c_str()), GX_TF_CMPR, 64, 64);
	m_gameBgLQ = m_mainBgLQ;

	// Build menus
	_initMainMenu();
	_initErrorMenu();
	_initWad();
	_initWBFSMenu();
	_initConfigAdvMenu();
	_initConfigSndMenu();
	_initConfig4Menu();
	_initConfigScreenMenu();
	_initConfig3Menu();
	_initConfigMenu();
	_initConfigGCMenu();
	_initConfig7Menu();
	_initPartitionsCfgMenu();
	_initGameMenu();
	_initDownloadMenu();
	_initCodeMenu();
	_initAboutMenu();
	_initCFThemeMenu();
	_initGameSettingsMenu();
	_initCheatSettingsMenu(); 
	_initSourceMenu();
	_initCfgSrc();
	_initCfgHB();
	_initPluginSettingsMenu();
	_initCategorySettingsMenu();
	_initGameInfoMenu();
	_initNandEmuMenu();
	_initHomeAndExitToMenu();
	_initCoverBanner();
	_initExplorer();
	_initBoot();
	_initPathsMenu();

	_loadCFCfg();
}

typedef struct
{
	string ext;
	u32 min;
	u32 max;
	u32 def;
	u32 res;
} FontHolder;

SFont CMenu::_dfltFont(u32 fontSize, u32 lineSpacing, u32 weight, u32 index, const char *genKey)
{
	/* get font info from theme.ini or use the default values */
	string filename;
	FontHolder fonts[3] = {{ "_size", 6u, 300u, fontSize, 0 }, { "_line_height", 6u, 300u, lineSpacing, 0 }, { "_weight", 1u, 32u, weight, 0 }};

	filename = m_theme.getString("GENERAL", genKey, genKey);
	bool useDefault = filename == genKey;

	/* get the resources - fontSize, lineSpacing, and weight */
	for(u32 i = 0; i < 3; i++)
	{
		string defValue = genKey;
		defValue += fonts[i].ext;// _size, _line_height, _weight
		fonts[i].res = (u32)m_theme.getInt("GENERAL", defValue);

		fonts[i].res = min(max(fonts[i].min, fonts[i].res <= 0 ? fonts[i].def : fonts[i].res), fonts[i].max);
	}

	/* check if font is already in memory */
	/* and the filename, size, spacing, and weight are the same */
	/* if so return this font */
	std::vector<SFont>::iterator font_itr;
	for(font_itr = theme.fontSet.begin(); font_itr != theme.fontSet.end(); ++font_itr)
	{
		if(strncmp(filename.c_str(), font_itr->name, 127) == 0 && font_itr->fSize == fonts[0].res &&
				font_itr->lineSpacing == fonts[1].res && font_itr->weight && fonts[2].res)
			break;
	}
	if (font_itr != theme.fontSet.end()) return *font_itr;

	/* font not found in memory, load it to create a new font */
	/* unless useDefault font is specified */
	SFont retFont;
	if(!useDefault && retFont.fromFile(fmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()), fonts[0].res, fonts[1].res, fonts[2].res, index, filename.c_str()))
	{
		// Theme Font
		theme.fontSet.push_back(retFont);
		return retFont;
	}
	
	/* try default font in imgs folder */
	if(retFont.fromFile(fmt("%s/font.ttf", m_imgsDir.c_str()), fonts[0].res, fonts[1].res, fonts[2].res, index, filename.c_str()))
	{
		// Default font
		theme.fontSet.push_back(retFont);
		return retFont;
	}
	
	/* Fallback to default font */
	/* default font is the wii's system font */
	if(retFont.fromBuffer(m_base_font, m_base_font_size, fonts[0].res, fonts[1].res, fonts[2].res, index, filename.c_str()))
	{
		// Default font
		theme.fontSet.push_back(retFont);
		return retFont;
	}
	return retFont;
}

SFont CMenu::_font(const char *domain, const char *key, SFont def_font)
{
	string filename;
	FontHolder fonts[3] = {{ "_size", 6u, 300u, 0, 0 }, { "_line_height", 6u, 300u, 0, 0 }, { "_weight", 1u, 32u, 0, 0 }};

	filename = m_theme.getString(domain, key);
	if(filename.empty())
		filename = def_font.name;

	/* get the resources - fontSize, lineSpacing, and weight */
	for(u32 i = 0; i < 3; i++)
	{
		string value = key;
		value += fonts[i].ext;// _size, _line_height, _weight

		fonts[i].res = (u32)m_theme.getInt(domain, value);
		if(fonts[i].res <= 0 && i == 0)
			fonts[i].res = def_font.fSize;
		else if(fonts[i].res <= 0 && i == 1)
			fonts[i].res = def_font.lineSpacing;
		else if(fonts[i].res <= 0 && i == 2)
			fonts[i].res = def_font.weight;

		fonts[i].res = min(max(fonts[i].min, fonts[i].res), fonts[i].max);
	}

	/* check if font is already in memory */
	/* and the filename, size, spacing, and weight are the same */
	/* if so return this font */
	std::vector<SFont>::iterator font_itr;
	for(font_itr = theme.fontSet.begin(); font_itr != theme.fontSet.end(); ++font_itr)
	{
		if(strncmp(filename.c_str(), font_itr->name, 127) == 0 && font_itr->fSize == fonts[0].res &&
				font_itr->lineSpacing == fonts[1].res && font_itr->weight && fonts[2].res)
			break;
	}
	if(font_itr != theme.fontSet.end()) return *font_itr;

	/* font not found in memory, load it to create a new font */
	/* unless useDefault font is specified */
	SFont retFont;
	if(retFont.fromFile(fmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()), fonts[0].res, fonts[1].res, fonts[2].res, 1, filename.c_str()))
	{
		// Theme Font
		theme.fontSet.push_back(retFont);
		return retFont;
	}
	return def_font;
}

vector<TexData> CMenu::_textures(const char *domain, const char *key)
{
	vector<TexData> textures;

	if (m_theme.loaded())
	{
		vector<string> filenames = m_theme.getStrings(domain, key);
		if (filenames.size() > 0)
		{
			for (vector<string>::iterator itr = filenames.begin(); itr != filenames.end(); itr++)
			{
				const string &filename = *itr;
				TexSet::iterator i = theme.texSet.find(filename);
				if (i != theme.texSet.end())
					textures.push_back(i->second);
				TexData themetex;
				if(TexHandle.fromImageFile(themetex, fmt("%s/%s", m_themeDataDir.c_str(), filename.c_str())) == TE_OK)
				{
					theme.texSet[filename] = themetex;
					textures.push_back(themetex);
				}
			}
		}
	}
	return textures;
}

TexData CMenu::_texture(const char *domain, const char *key, TexData &def, bool freeDef)
{
	string filename;

	if(m_theme.loaded())
	{
		/* Load from theme */
		filename = m_theme.getString(domain, key);
		if(!filename.empty())
		{
			TexSet::iterator i = theme.texSet.find(filename);
			if(i != theme.texSet.end())
				return i->second;
			/* Load from image file */
			TexData themetex;
			if(TexHandle.fromImageFile(themetex, fmt("%s/%s", m_themeDataDir.c_str(), filename.c_str())) == TE_OK)
			{
				if(freeDef && def.data != NULL)
				{
					MEM2_free(def.data);
					def.data = NULL;
				}
				theme.texSet[filename] = themetex;
				return themetex;
			}
		}
	}
	/* Fallback to default */
	return def;
}

// Only for loading defaults and GENERAL domains!!
GuiSound *CMenu::_sound(CMenu::SoundSet &soundSet, const char *filename, const u8 * snd, u32 len, const char *name, bool isAllocated)
{
	if(filename == NULL || filename[0] == '\0')
		filename = name;

	CMenu::SoundSet::iterator i = soundSet.find(upperCase(name));
	if(i == soundSet.end())
	{
		if(filename != name && fsop_FileExist(fmt("%s/%s", m_themeDataDir.c_str(), filename)))
		{
			u32 size = 0;
			u8 *mem = fsop_ReadFile(fmt("%s/%s", m_themeDataDir.c_str(), filename), &size);
			soundSet[upperCase(filename)] = new GuiSound(mem, size, filename, true);
		}
		else
			soundSet[upperCase(filename)] = new GuiSound(snd, len, filename, isAllocated);
		return soundSet[upperCase(filename)];
	}
	return i->second;
}

//For buttons and labels only!!
GuiSound *CMenu::_sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, const char *name)
{
	const char *filename = m_theme.getString(domain, key).c_str();
	if(filename == NULL || filename[0] == '\0')
	{
		if(strrchr(name, '/') != NULL)
			name = strrchr(name, '/') + 1;
		return soundSet[upperCase(name)];  // General/Default are already cached!
	}

	SoundSet::iterator i = soundSet.find(upperCase(filename));
	if(i == soundSet.end())
	{
		if(fsop_FileExist(fmt("%s/%s", m_themeDataDir.c_str(), filename)))
		{
			u32 size = 0;
			u8 *mem = fsop_ReadFile(fmt("%s/%s", m_themeDataDir.c_str(), filename), &size);
			soundSet[upperCase(filename)] = new GuiSound(mem, size, filename, true);
		}
		else
			soundSet[upperCase(filename)] = new GuiSound();
		return soundSet[upperCase(filename)];
	}
	return i->second;
}

u16 CMenu::_textStyle(const char *domain, const char *key, u16 def, bool coverflow)
{
	u16 textStyle = 0;
	string style;
	if(coverflow)
		style = m_coverflow.getString(domain, key);
	else
		style = m_theme.getString(domain, key);
	if (style.empty()) return def;

	if (style.find_first_of("Cc") != string::npos)
		textStyle |= FTGX_JUSTIFY_CENTER;
	else if (style.find_first_of("Rr") != string::npos)
		textStyle |= FTGX_JUSTIFY_RIGHT;
	else
		textStyle |= FTGX_JUSTIFY_LEFT;
	if (style.find_first_of("Mm") != string::npos)
		textStyle |= FTGX_ALIGN_MIDDLE;
	else if (style.find_first_of("Bb") != string::npos)
		textStyle |= FTGX_ALIGN_BOTTOM;
	else
		textStyle |= FTGX_ALIGN_TOP;
	return textStyle;
}

s16 CMenu::_addButton(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color)
{
	SButtonTextureSet btnTexSet;
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	btnTexSet.left = _texture(domain, "texture_left", theme.btnTexL, false);
	btnTexSet.right = _texture(domain, "texture_right", theme.btnTexR, false);
	btnTexSet.center = _texture(domain, "texture_center", theme.btnTexC, false);
	btnTexSet.leftSel = _texture(domain, "texture_left_selected", theme.btnTexLS, false);
	btnTexSet.rightSel = _texture(domain, "texture_right_selected", theme.btnTexRS, false);
	btnTexSet.centerSel = _texture(domain, "texture_center_selected", theme.btnTexCS, false);
	font = _font(domain, "font", font);
	GuiSound *clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound->GetName());
	GuiSound *hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound->GetName());
	return m_btnMgr.addButton(font, text, x, y, width, height, c, btnTexSet, clickSound, hoverSound);
}

s16 CMenu::_addPicButton(const char *domain, TexData &texNormal, TexData &texSelected, int x, int y, u32 width, u32 height)
{
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	TexData tex1 = _texture(domain, "texture_normal", texNormal, false);
	TexData tex2 = _texture(domain, "texture_selected", texSelected, false);
	GuiSound *clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound->GetName());
	GuiSound *hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound->GetName());
	return m_btnMgr.addPicButton(tex1, tex2, x, y, width, height, clickSound, hoverSound);
}

s16 CMenu::_addLabel(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style)
{
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(domain, "font", font);
	style = _textStyle(domain, "style", style);
	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style);
}

s16 CMenu::_addLabel(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style, TexData &bg)
{
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(domain, "font", font);
	TexData texBg = _texture(domain, "background_texture", bg, false);
	style = _textStyle(domain, "style", style);
	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style, texBg);
}

s16 CMenu::_addProgressBar(const char *domain, int x, int y, u32 width, u32 height)
{
	SButtonTextureSet btnTexSet;

	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	btnTexSet.left = _texture(domain, "texture_left", theme.pbarTexL, false);
	btnTexSet.right = _texture(domain, "texture_right", theme.pbarTexR, false);
	btnTexSet.center = _texture(domain, "texture_center", theme.pbarTexC, false);
	btnTexSet.leftSel = _texture(domain, "texture_left_selected", theme.pbarTexLS, false);
	btnTexSet.rightSel = _texture(domain, "texture_right_selected", theme.pbarTexRS, false);
	btnTexSet.centerSel = _texture(domain, "texture_center_selected", theme.pbarTexCS, false);
	return m_btnMgr.addProgressBar(x, y, width, height, btnTexSet);
}

void CMenu::_setHideAnim(s16 id, const char *domain, int dx, int dy, float scaleX, float scaleY)
{
	dx = m_theme.getInt(domain, "effect_x", dx);
	dy = m_theme.getInt(domain, "effect_y", dy);
	scaleX = m_theme.getFloat(domain, "effect_scale_x", scaleX);
	scaleY = m_theme.getFloat(domain, "effect_scale_y", scaleY);
	m_btnMgr.hide(id, dx, dy, scaleX, scaleY, true);
}

void CMenu::_addUserLabels(s16 *ids, u32 size, const char *domain)
{
	_addUserLabels(ids, 0, size, domain);
}

void CMenu::_addUserLabels(s16 *ids, u32 start, u32 size, const char *domain)
{

	for(u32 i = start; i < start + size; ++i)
	{
		string dom(fmt("%s/USER%i", domain, i + 1));
		if (m_theme.hasDomain(dom))
		{
			TexData emptyTex;
			ids[i] = _addLabel(dom.c_str(), theme.lblFont, L"", 40, 200, 64, 64, CColor(0xFFFFFFFF), 0, emptyTex);
			_setHideAnim(ids[i], dom.c_str(), -50, 0, 0.f, 0.f);
		}
		else
			ids[i] = -1;
	}
}

bool musicPaused = false;
void CMenu::_mainLoopCommon(bool withCF, bool adjusting)
{
	if(m_thrdWorking)
	{
		musicPaused = true;
		MusicPlayer.Pause();//note - bg music is paused but sound thread is still running. so banner gamesound still plays
		m_btnMgr.tick();
		m_vid.prepare();
		m_vid.setup2DProjection(false, true);// false = prepare() already set view port, true = no scaling - draw at 640x480
		_updateBg();
		if(CoverFlow.getRenderTex())
			CoverFlow.RenderTex();
		m_vid.setup2DProjection();// this time set the view port and allow scaling
		_drawBg();
		m_btnMgr.draw();
		m_vid.render();
		return;
	}
	if(musicPaused && !m_thrdWorking)
	{
		musicPaused = false;
		MusicPlayer.Resume();
	}
	
	/* ticks - for moving and scaling covers and gui buttons and text */
	if(withCF)
		CoverFlow.tick();
	m_btnMgr.tick();
	m_fa.tick();

	/* video setup */
	m_vid.prepare();
	m_vid.setup2DProjection(false, true);
	
	/* background and coverflow drawing */
	_updateBg();
	if(CoverFlow.getRenderTex())
		CoverFlow.RenderTex();
	if(withCF && m_lqBg != NULL)
		CoverFlow.makeEffectTexture(m_lqBg);
	if(withCF && m_aa > 0)
	{
		m_vid.setAA(m_aa, true);
		for(int i = 0; i < m_aa; ++i)
		{
			m_vid.prepareAAPass(i);
			m_vid.setup2DProjection(false, true);
			_drawBg();
			CoverFlow.draw();
			m_vid.setup2DProjection(false, true);
			CoverFlow.drawEffect();
			if(!m_soundThrdBusy && !m_banner.GetSelectedGame() && !m_snapshot_loaded)
				CoverFlow.drawText(adjusting);
			m_vid.renderAAPass(i);
		}
		m_vid.setup2DProjection();
		m_vid.drawAAScene();
	}
	else
	{
		m_vid.setup2DProjection();
		_drawBg();
		if(withCF)
		{
			CoverFlow.draw();
			m_vid.setup2DProjection();
			CoverFlow.drawEffect();
			if(!m_soundThrdBusy && !m_banner.GetSelectedGame() && !m_snapshot_loaded)
				CoverFlow.drawText(adjusting);
		}
	}
	
	/* game video or banner drawing */
	if(m_gameSelected)
	{
		if(m_fa.isLoaded())
			m_fa.draw();
		else if(m_video_playing)
		{
			if(movie.Frame != NULL)
			{
				DrawTexturePos(movie.Frame);
				movie.Frame->thread = false;
			}
		}
		else if(m_banner.GetSelectedGame() && (!m_banner.GetInGameSettings() || (m_banner.GetInGameSettings() && m_bnr_settings)))
			m_banner.Draw();
	}
	
	/* gui buttons and text drawing */
	m_btnMgr.draw();
	
	/* reading controller inputs and drawing cursor pointers*/	
	ScanInput();
	
	/* check if we want screensaver and if its idle long enuff, if so draw full screen black square with mild alpha */
	if(!m_cfg.getBool("GENERAL", "screensaver_disabled", true))
		m_vid.screensaver(NoInputTime(), m_cfg.getInt("GENERAL", "screensaver_idle_seconds", 60));

	/* render everything on screen */
	m_vid.render();
	
	// check if power button is pressed and exit wiiflow
	if(Sys_Exiting())
	{
		if(m_cfg.getBool("GENERAL", "idle_standby", false))
			exitHandler(SHUTDOWN_IDLE);
		else
			exitHandler(SHUTDOWN_STANDBY);
	}

	// check if we need to start playing the game/banner sound
	// m_gameSelected means we are on the game selected menu
	// m_gamesound_changed means a new game sound is loaded and ready to play
	// the previous game sound needs to stop before playing new sound
	// and the bg music volume needs to be 0 before playing game sound
	if(withCF && m_gameSelected && m_gamesound_changed && !m_gameSound.IsPlaying() && MusicPlayer.GetVolume() == 0)
	{
		_stopGameSoundThread();// stop game sound loading thread
		m_gameSound.Play(m_bnrSndVol);// play game sound
		m_gamesound_changed = false;
	}
	// stop game/banner sound from playing if we exited game selected menu or if we move to new game
	else if((withCF && m_gameSelected && m_gamesound_changed && m_gameSound.IsPlaying()) || (!m_gameSelected && m_gameSound.IsPlaying()))
		m_gameSound.Stop();

	/* decrease music volume to zero if any of these are true:
		trailer video playing or||
		game/banner sound is being loaded because we are switching to a new game or||
		game/banner sound is loaded and ready to play or||
		gamesound hasn't finished - when finishes music volume back to normal - some gamesounds don't loop continuously
		also this switches to next song if current song is done */
	MusicPlayer.Tick((withCF && (m_video_playing || (m_gameSelected && m_soundThrdBusy) || 
						(m_gameSelected && m_gamesound_changed))) ||  m_gameSound.IsPlaying());

	// set song title and display it if music info is allowed
	if(MusicPlayer.SongChanged() && m_music_info)
	{
		m_btnMgr.setText(m_mainLblCurMusic, MusicPlayer.GetFileName(), false);// false for word wrap
		m_btnMgr.show(m_mainLblCurMusic);
		MusicPlayer.DisplayTime = time(NULL);
	}
	// hide song title if it's displaying and been >3 seconds
	else if(MusicPlayer.DisplayTime > 0 && time(NULL) - MusicPlayer.DisplayTime > 3)
	{
		MusicPlayer.DisplayTime = 0;
		m_btnMgr.hide(m_mainLblCurMusic);
		if(MusicPlayer.OneSong) m_music_info = false;
	}

	//Take Screenshot
	if(WBTN_Z_PRESSED || GBTN_Z_PRESSED)
	{
		time_t rawtime;
		struct tm *timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer,80,"%b-%d-20%y-%Hh%Mm%Ss.png",timeinfo);
		gprintf("Screenshot taken and saved to: %s/%s\n", m_screenshotDir.c_str(), buffer);
		m_vid.TakeScreenshot(fmt("%s/%s", m_screenshotDir.c_str(), buffer));
		if(theme.cameraSound != NULL)
			theme.cameraSound->Play(255);
	}

	if(show_mem)
	{
		m_btnMgr.setText(m_mem1FreeSize, wfmt(L"Mem1 lo Free:%u, Mem1 Free:%u", MEM1_lo_freesize(), MEM1_freesize()), true);
		m_btnMgr.setText(m_mem2FreeSize, wfmt(L"Mem2 Free:%u", MEM2_freesize()), true);
	}

#ifdef SHOWMEMGECKO
	mem1 = MEM1_freesize();
	mem2 = MEM2_freesize();
	if(mem1 != mem1old)
	{
		mem1old = mem1;
		gprintf("Mem1 Free: %u\n", mem1);
	}
	if(mem2 != mem2old)
	{
		mem2old = mem2;
		gprintf("Mem2 Free: %u\n", mem2);
	}
#endif
}

void CMenu::_setBg(const TexData &bgTex, const TexData &bglqTex)
{
	/* Not setting same bg again */
	if(m_nextBg == &bgTex)
		return;
	m_lqBg = &bglqTex;
	/* before setting new next bg set previous */
	if(m_nextBg != NULL)
		m_prevBg = m_nextBg;
	m_nextBg = &bgTex;
	m_bgCrossFade = 0xFF;
}

void CMenu::_updateBg(void)
{
	if(m_bgCrossFade == 0)
		return;
	m_bgCrossFade = max(0, (int)m_bgCrossFade - 14);

	Mtx modelViewMtx;
	GXTexObj texObj;
	GXTexObj texObj2;

	/* last pass so remove previous bg */
	if(m_bgCrossFade == 0)
		m_prevBg = NULL;

	GX_ClearVtxDesc();
	GX_SetNumTevStages(m_prevBg == NULL ? 1 : 2);
	GX_SetNumChans(0);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(m_prevBg == NULL ? 1 : 2);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevKColor(GX_KCOLOR0, CColor(m_bgCrossFade, 0xFF - m_bgCrossFade, 0, 0));
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0_R);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_KONST, GX_CC_ZERO);
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0_G);
	GX_SetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLORNULL);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_FALSE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	if(m_nextBg != NULL && m_nextBg->data != NULL)
	{
		GX_InitTexObj(&texObj, m_nextBg->data, m_nextBg->width, m_nextBg->height, m_nextBg->format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
	}
	if(m_prevBg != NULL && m_prevBg->data != NULL)
	{
		GX_InitTexObj(&texObj2, m_prevBg->data, m_prevBg->width, m_prevBg->height, m_prevBg->format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj2, GX_TEXMAP1);
	}
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(0.f, 0.f, 0.f);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position3f32(640.f, 0.f, 0.f);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position3f32(640.f, 480.f, 0.f);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position3f32(0.f, 480.f, 0.f);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
	GX_SetNumTevStages(1);
	m_curBg.width = 640;
	m_curBg.height = 480;
	m_curBg.format = GX_TF_RGBA8;
	m_curBg.maxLOD = 0;
	m_vid.renderToTexture(m_curBg, true);
}

void CMenu::_drawBg(void)
{
	Mtx modelViewMtx;
	GXTexObj texObj;

	GX_ClearVtxDesc();
	GX_SetNumTevStages(1);
	GX_SetNumChans(0);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(1);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_FALSE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_InitTexObj(&texObj, m_curBg.data, m_curBg.width, m_curBg.height, m_curBg.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(0.f, 0.f, 0.f);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position3f32(640.f, 0.f, 0.f);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position3f32(640.f, 480.f, 0.f);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position3f32(0.f, 480.f, 0.f);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
}

void CMenu::_updateText(void)
{
	if(m_use_source)
		_textSource();
	_textPluginSettings();
	_textCategorySettings();
	_textCheatSettings();
	_textConfig();
	_textConfig3();
	_textConfigScreen();
	_textConfig4();
	_textConfigAdv();
	_textConfigSnd();
	_textConfigGC();
	_textPartitionsCfg();
	_textCfgHB();
	_textGame();
	_textDownload();
	_textCode();
	_textWBFS();
	_textGameSettings();
	_textNandEmu();
	_textHome();
	_textExitTo();
	_textShutdown();
	_textBoot();
	_textCoverBanner();
	_textExplorer();
	_textWad();
}

const wstringEx CMenu::_fmt(const char *key, const wchar_t *def)
{
	wstringEx ws = m_loc.getWString(m_curLanguage, key, def);
	if (checkFmt(def, ws)) return ws;
	return def;
}

void CMenu::_initCF(void)
{
	Config dump;
	bool dumpGameLst = m_cfg.getBool("GENERAL", "dump_list", true);
	if(dumpGameLst) dump.load(fmt("%s/" TITLES_DUMP_FILENAME, m_settingsDir.c_str()));

	CoverFlow.clear();
	CoverFlow.reserve(m_gameList.size());

	string requiredCats;
	string selectedCats;
	string hiddenCats;
	char id[74];
	char catID[64];
	
	for(vector<dir_discHdr>::iterator hdr = m_gameList.begin(); hdr != m_gameList.end(); ++hdr)
	{
		requiredCats = m_cat.getString("GENERAL", "required_categories", "");
		selectedCats = m_cat.getString("GENERAL", "selected_categories", "");
		hiddenCats = m_cat.getString("GENERAL", "hidden_categories", "");
		
		const char *favDomain = "FAVORITES";
		const char *adultDomain = "ADULTONLY";
		
		memset(id, 0, 74);
		memset(catID, 0, 64);
		
		if(m_sourceflow)
		{
			CoverFlow.addItem(&(*hdr), 0, 0);
			continue;
			/*strcpy(tmp1, "source/");
			wcstombs(tmp2, hdr->title, 64);
			strcat(tmp1, tmp2);
			id = tmp1;*/
		}
		else if(hdr->type == TYPE_HOMEBREW)
		{
			wcstombs(id, hdr->title, 63);
			strcpy(catID, id);
		}
		else if(hdr->type == TYPE_PLUGIN)
		{
			if(m_cat.hasDomain("PLUGINS"))// if using new style categories_lite.ini
			{
				requiredCats = m_cat.getString("PLUGINS", "required_categories", "");
				selectedCats = m_cat.getString("PLUGINS", "selected_categories", "");
				hiddenCats = m_cat.getString("PLUGINS", "hidden_categories", "");
			}
			
			strncpy(m_plugin.PluginMagicWord, fmt("%08x", hdr->settings[0]), 8);
			if(strrchr(hdr->path, '/') != NULL)
				wcstombs(catID, hdr->title, 63);
			else
				strncpy(catID, hdr->path, 63);// scummvm
			strcpy(id, m_plugin.PluginMagicWord);
			strcat(id, fmt("/%s", catID));
			favDomain = "FAVORITES_PLUGINS";
			adultDomain = "ADULTONLY_PLUGINS";
		}
		else // wii, gc, channels
		{
			strcpy(id, hdr->id);
			strcpy(catID, id);
		}
		
		u8 numReqCats = requiredCats.length();
		u8 numSelCats = selectedCats.length();
		u8 numHidCats = hiddenCats.length();
		
		if((!m_favorites || m_gcfg1.getBool(favDomain, id, false))
			&& (!m_locked || !m_gcfg1.getBool(adultDomain, id, false)))
		{
			string catDomain = "";
			if(hdr->type == TYPE_CHANNEL)
				catDomain = "NAND";
			else if(hdr->type == TYPE_EMUCHANNEL)
				catDomain = "CHANNELS";
			else if(hdr->type == TYPE_GC_GAME)
				catDomain = "GAMECUBE";
			else if(hdr->type == TYPE_WII_GAME)
				catDomain = "WII";
			else if(hdr->type == TYPE_HOMEBREW)
				catDomain = "HOMEBREW";
			else
				catDomain = m_plugin.PluginMagicWord;

			if(numReqCats != 0 || numSelCats != 0 || numHidCats != 0) // if all 0 skip checking cats and show all games
			{
				string idCats= m_cat.getString(catDomain, catID, "");
				u8 numIdCats = idCats.length();
				if(numIdCats == 0)
					m_cat.remove(catDomain, catID);
				bool inaCat = false;
				bool inHiddenCat = false;
				int reqMatch = 0;
				if(numIdCats != 0)
				{
					for(u8 j = 0; j < numIdCats; ++j)
					{
						int k = (static_cast<int>(idCats[j])) - 32;
						if(k <= 0)
							continue;
						bool match = false;
						if(numReqCats != 0)
						{
							for(u8 l = 0; l < numReqCats; ++l)
							{
								if(k == (static_cast<int>(requiredCats[l]) - 32))
								{
									match = true;
									reqMatch++;
									inaCat = true;
								}
							}
						}
						if(match)
							continue;
						if(numSelCats != 0)
						{
							for(u8 l = 0; l < numSelCats; ++l)
							{
								if(k == (static_cast<int>(selectedCats[l]) - 32))
								{
									match = true;
									inaCat = true;
								}
							}
						}
						if(match)
							continue;
						if(numHidCats != 0)
						{
							for(u8 l = 0; l < numHidCats; ++l)
							{
								if(k == (static_cast<int>(hiddenCats[l]) - 32))
									inHiddenCat = true;
							}
						}
					}
				}
				//continue; means don't add game to list (don't show)
				if(inHiddenCat)
					continue;
				if(numReqCats != reqMatch)
					continue;
				if(!inaCat)
				{
					if(numHidCats == 0)
						continue;
					else if(numSelCats > 0)
						continue;
				}
			}

			if(dumpGameLst && !NoGameID(hdr->type))
			{
				const char *domain = NULL;
				switch(hdr->type)
				{
					case TYPE_CHANNEL:
						domain = "NAND";
						break;
					case TYPE_EMUCHANNEL:
						domain = "CHANNELS";
						break;
					case TYPE_GC_GAME:
						domain = "GAMECUBE";
						break;
					default:
						domain = "WII";
						break;
				}
				dump.setWString(domain, id, hdr->title);
			}

			if(hdr->type == TYPE_PLUGIN && m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(hdr->settings[0])))
				CoverFlow.addItem(&(*hdr), 0, 0);
			else
			{
				int playcount = m_gcfg1.getInt("PLAYCOUNT", id, 0);
				unsigned int lastPlayed = m_gcfg1.getUInt("LASTPLAYED", id, 0);
				CoverFlow.addItem(&(*hdr), playcount, lastPlayed);
			}
		}
		/* remove them if false to keep file short */
		if(!m_gcfg1.getBool(favDomain, id))
			m_gcfg1.remove(favDomain, id);
		if(!m_gcfg1.getBool(adultDomain, id))
			m_gcfg1.remove(adultDomain, id);
	}

	if(CoverFlow.empty())
	{
		dump.unload();
		return;
	}
		
	if(dumpGameLst)
	{
		dump.save(true);
		m_cfg.setBool("GENERAL", "dump_list", false);
	}

	/*********************** sort coverflow list ***********************/
	CoverFlow.setSorting(m_source_cnt > 1 ? (Sorting)0 : (Sorting)m_cfg.getInt(_domainFromView(), "sort", 0));
	
	/*********************** set box mode and small box mode **************************/
	if(!m_sourceflow)
	{
		if(m_current_view == COVERFLOW_HOMEBREW)
		{
			CoverFlow.setBoxMode(m_cfg.getBool(HOMEBREW_DOMAIN, "box_mode", true));
			CoverFlow.setSmallBoxMode(m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", false));
		}
		else if(m_current_view == COVERFLOW_PLUGIN)
		{
			if(enabledPluginsCount == 1)// only one plugin enabled
			{
				if(m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(strtoul("48425257", NULL, 16))))// homebrew plugin
				{
					CoverFlow.setBoxMode(m_cfg.getBool(HOMEBREW_DOMAIN, "box_mode", true));
					CoverFlow.setSmallBoxMode(m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox", false));
				}
				else 
				{
					s8 bm = -1;
					for(u8 i = 0; m_plugin.PluginExist(i); ++i)
					{
						if(m_plugin.GetEnabledStatus(i))
						{
							bm = m_plugin.GetBoxMode(i);
							break;
						}
					}
					if(bm < 0)// if negative then use default setting
						CoverFlow.setBoxMode(m_cfg.getBool("GENERAL", "box_mode", true));
					else 
						CoverFlow.setBoxMode(bm == 0 ? false : true);
					CoverFlow.setSmallBoxMode(false);
				}
			}
			else // more than 1 plugin enabled
			{
				s8 bm1 = -1;
				s8 bm2 = -1;
				bool all_same = true;
				for(u8 i = 0; m_plugin.PluginExist(i); ++i)
				{
					if(m_plugin.GetEnabledStatus(i))
					{
						if(bm1 == -1)
						{
							bm1 = m_plugin.GetBoxMode(i);
							if(bm1 < 0)
								bm1 = m_cfg.getBool("GENERAL", "box_mode", true) ? 1 : 0;
						}
						else
						{
							bm2 = m_plugin.GetBoxMode(i);
							if(bm2 < 0)
								bm2 = m_cfg.getBool("GENERAL", "box_mode", true) ? 1 : 0;
							if(bm2 != bm1)
							{
								all_same = false;
								break;
							}
						}
					}
				}
				if(!all_same)
					CoverFlow.setBoxMode(m_cfg.getBool("GENERAL", "box_mode", true));
				else
					CoverFlow.setBoxMode(bm1 == 0 ? false : true);
				CoverFlow.setSmallBoxMode(false);
			}
		}
		else
		{
			CoverFlow.setBoxMode(m_cfg.getBool("GENERAL", "box_mode", true));
			CoverFlow.setSmallBoxMode(false);
		}
	}
	else // sourceflow
	{
		CoverFlow.setBoxMode(m_cfg.getBool(SOURCEFLOW_DOMAIN, "box_mode", true));
		CoverFlow.setSmallBoxMode(m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox", false));
	}
	
	/*********************** Setup coverflow covers settings ***********************/
	CoverFlow.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 20));
	CoverFlow.setHQcover(m_cfg.getBool("GENERAL", "cover_use_hq", true));
	CoverFlow.start(m_imgsDir);
	
	/*********************** Get and set game list current item to center cover **************************/
	if(!CoverFlow.empty())
	{
		/* get ID or filename or source number of center cover */
		string ID = "", filename = "";
		u32 sourceNumber = 0;
		if(m_current_view == COVERFLOW_PLUGIN && !m_sourceflow)
		{
			if(!m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(strtoul(m_cfg.getString(PLUGIN_DOMAIN, "cur_magic", "00000000").c_str(), NULL, 16))))
			{
				for(u8 i = 0; m_plugin.PluginExist(i); ++i)
				{
					if(m_plugin.GetEnabledStatus(i))
					{
						m_cfg.setString(PLUGIN_DOMAIN, "cur_magic", sfmt("%08x", m_plugin.GetPluginMagic(i)));
						break;
					}
				}
			}

			strncpy(m_plugin.PluginMagicWord, m_cfg.getString(PLUGIN_DOMAIN, "cur_magic").c_str(), 8);
			
			if(strncasecmp(m_plugin.PluginMagicWord, "4E47434D", 8) == 0)//NGCM
				ID = m_cfg.getString("plugin_item", m_plugin.PluginMagicWord, "");
			else if(strncasecmp(m_plugin.PluginMagicWord, "4E574949", 8) == 0)//NWII
				ID = m_cfg.getString("plugin_item", m_plugin.PluginMagicWord, "");
			else if(strncasecmp(m_plugin.PluginMagicWord, "4E414E44", 8) == 0)//NAND
				ID = m_cfg.getString("plugin_item", m_plugin.PluginMagicWord, "");
			else if(strncasecmp(m_plugin.PluginMagicWord, "454E414E", 8) == 0)//EMUNAND
				ID = m_cfg.getString("plugin_item", m_plugin.PluginMagicWord, "");
			else
				filename = m_cfg.getString("plugin_item", m_plugin.PluginMagicWord, "");// homebrew and plugins
		}
		else if(m_sourceflow && sm_numbers.size() > 0)
			sourceNumber = stoi(sm_numbers[sm_numbers.size() - 1]);
		else if(m_current_view == COVERFLOW_HOMEBREW || (m_source_cnt > 1 && NoGameID(m_cfg.getInt("MULTI", "current_item_type", TYPE_PLUGIN)))) 
			filename = m_cfg.getString(_domainFromView(), "current_item", "");
		else
			ID = m_cfg.getString(_domainFromView(), "current_item", "");
			
		/* set center cover as coverflow current position */
		if(!CoverFlow._setCurPosToCurItem(ID.c_str(), filename.c_str(), sourceNumber, true))
			CoverFlow._setCurPos(0);// if not found set first cover as coverflow current position
			
		/************************** create and start the cover loader thread *************************/
		CoverFlow.startCoverLoader();
	}
}

bool CMenu::_loadList(void)
{
	CoverFlow.clear();// clears filtered list (m_items), cover list (m_covers), and cover textures and stops coverloader
	m_gameList.clear();
	vector<dir_discHdr>().swap(m_gameList);
	NANDemuView = false;
	
	if(m_sourceflow)
	{
		m_cacheList.createSFList(m_max_source_btn, m_source, m_sourceDir);
		for(vector<dir_discHdr>::iterator tmp_itr = m_cacheList.begin(); tmp_itr != m_cacheList.end(); tmp_itr++)
			m_gameList.push_back(*tmp_itr);
		m_cacheList.Clear();
		if(SF_cacheCovers)
		{
			SF_cacheCovers = false;
			cacheCovers = true;
		}
		return true;
	}
	gprintf("Creating Gamelist\n");
	if(m_current_view & COVERFLOW_PLUGIN)
		_loadPluginList();
		
	if(m_current_view & COVERFLOW_WII)
		_loadWiiList();

	if(m_current_view & COVERFLOW_CHANNEL)
		_loadChannelList();

	if(m_current_view & COVERFLOW_GAMECUBE)
		_loadGamecubeList();

	if(m_current_view & COVERFLOW_HOMEBREW)
		_loadHomebrewList(HOMEBREW_DIR);

	m_cacheList.Clear();

	gprintf("Games found: %i\n", m_gameList.size());
	return m_gameList.size() > 0 ? true : false;
}

bool CMenu::_loadWiiList(void)
{
	currentPartition = m_cfg.getInt(WII_DOMAIN, "partition", USB1);
	if(!DeviceHandle.IsInserted(currentPartition))
		return false;

	gprintf("Adding wii list\n");
	DeviceHandle.OpenWBFS(currentPartition);
	string gameDir(fmt(wii_games_dir, DeviceName[currentPartition]));
	string cacheDir(fmt("%s/%s_wii.db", m_listCacheDir.c_str(), DeviceName[currentPartition]));
	bool updateCache = m_cfg.getBool(WII_DOMAIN, "update_cache");
	bool preCachedList = fsop_FileExist(cacheDir.c_str());
	m_cacheList.CreateList(COVERFLOW_WII, gameDir, stringToVector(".wbfs|.iso", '|'), cacheDir, updateCache);
	WBFS_Close();
	m_cfg.remove(WII_DOMAIN, "update_cache");
	for(vector<dir_discHdr>::iterator tmp_itr = m_cacheList.begin(); tmp_itr != m_cacheList.end(); tmp_itr++)
		m_gameList.push_back(*tmp_itr);
	if(updateCache || (!preCachedList && fsop_FileExist(cacheDir.c_str())))
		cacheCovers = true;
	return true;
}

bool CMenu::_loadHomebrewList(const char *HB_Dir)
{
	currentPartition = m_cfg.getInt(HOMEBREW_DOMAIN, "partition", SD);
	if(!DeviceHandle.IsInserted(currentPartition))
		return false;

	gprintf("Adding homebrew list\n");
	string gameDir(fmt("%s:/%s", DeviceName[currentPartition], HB_Dir));
	string cacheDir(fmt("%s/%s_%s.db", m_listCacheDir.c_str(), DeviceName[currentPartition], HB_Dir));
	bool updateCache = m_cfg.getBool(HOMEBREW_DOMAIN, "update_cache");
	bool preCachedList = fsop_FileExist(cacheDir.c_str());
	m_cacheList.CreateList(COVERFLOW_HOMEBREW, gameDir, stringToVector(".dol|.elf", '|'), cacheDir, updateCache);
	m_cfg.remove(HOMEBREW_DOMAIN, "update_cache");
	for(vector<dir_discHdr>::iterator tmp_itr = m_cacheList.begin(); tmp_itr != m_cacheList.end(); tmp_itr++)
		m_gameList.push_back(*tmp_itr);
	if(updateCache || (!preCachedList && fsop_FileExist(cacheDir.c_str())))
		cacheCovers = true;
	return true;
}

bool CMenu::_loadGamecubeList()
{
	currentPartition = m_cfg.getInt(GC_DOMAIN, "partition", USB1);
	if(!DeviceHandle.IsInserted(currentPartition))
		return false;

	gprintf("Adding gamecube list\n");
	string gameDir(fmt(gc_games_dir, DeviceName[currentPartition]));
	string cacheDir(fmt("%s/%s_gamecube.db", m_listCacheDir.c_str(), DeviceName[currentPartition]));
	bool updateCache = m_cfg.getBool(GC_DOMAIN, "update_cache");
	bool preCachedList = fsop_FileExist(cacheDir.c_str());
	m_cacheList.CreateList(COVERFLOW_GAMECUBE, gameDir, stringToVector(".iso|.gcm|.ciso|root", '|'), cacheDir, updateCache);
	m_cfg.remove(GC_DOMAIN, "update_cache");
	for(vector<dir_discHdr>::iterator tmp_itr = m_cacheList.begin(); tmp_itr != m_cacheList.end(); tmp_itr++)
	{
		if(tmp_itr->settings[0] == 1) /* disc 2 */
			continue;// skip gc disc 2 if its still part of the cached list
		m_gameList.push_back(*tmp_itr);
	}
	if(updateCache || (!preCachedList && fsop_FileExist(cacheDir.c_str())))
		cacheCovers = true;
	return true;
}

bool CMenu::_loadChannelList(void)
{
	u8 chantypes = m_cfg.getUInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL);
	if(chantypes < CHANNELS_REAL || chantypes > CHANNELS_BOTH)
	{
		m_cfg.setUInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL);
		chantypes = CHANNELS_REAL;
	}
	vector<string> NullVector;
	if(chantypes & CHANNELS_REAL)
	{
		gprintf("Adding real nand list\n");
		NANDemuView = false;
		bool updateCache = m_cfg.getBool(CHANNEL_DOMAIN, "update_cache");
		if(updateCache)
			cacheCovers = true;// real nand channels list is not cached but covers may still need to be updated
		m_cacheList.CreateList(COVERFLOW_CHANNEL, std::string(), NullVector, std::string(), false);
		m_cfg.remove(CHANNEL_DOMAIN, "update_cache");
		for(vector<dir_discHdr>::iterator tmp_itr = m_cacheList.begin(); tmp_itr != m_cacheList.end(); tmp_itr++)
			m_gameList.push_back(*tmp_itr);
	}
	if(chantypes & CHANNELS_EMU)
	{
		NANDemuView = true;
		int emuPartition = _FindEmuPart(EMU_NAND, false);// check if emunand folder exist and on FAT
		if(emuPartition >= 0)
		{
			gprintf("Adding emu nand list\n");
			currentPartition = emuPartition;
			string cacheDir = fmt("%s/%s_channels.db", m_listCacheDir.c_str(), DeviceName[currentPartition]);
			bool updateCache = m_cfg.getBool(CHANNEL_DOMAIN, "update_cache");
			bool preCachedList = fsop_FileExist(cacheDir.c_str());
			m_cacheList.CreateList(COVERFLOW_CHANNEL, std::string(), NullVector, cacheDir, updateCache);
			m_cfg.remove(CHANNEL_DOMAIN, "update_cache");
			for(vector<dir_discHdr>::iterator tmp_itr = m_cacheList.begin(); tmp_itr != m_cacheList.end(); tmp_itr++)
				m_gameList.push_back(*tmp_itr);
			if(updateCache || (!preCachedList && fsop_FileExist(cacheDir.c_str())))
				cacheCovers = true;
		}
	}
	return true;
}

bool CMenu::_loadPluginList()
{
	bool updateCache = m_cfg.getBool(PLUGIN_DOMAIN, "update_cache");
	int channels_type = min(max(1, m_cfg.getInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL)), (int)ARRAY_SIZE(CMenu::_ChannelsType));
	gprintf("Adding plugins list\n");
	for(u8 i = 0; m_plugin.PluginExist(i); ++i)
	{
		if(!m_plugin.GetEnabledStatus(i))
			continue;
		int romsPartition = m_plugin.GetRomPartition(i);
		if(romsPartition < 0)
			romsPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition", 0);
		currentPartition = romsPartition;
		if(!DeviceHandle.IsInserted(currentPartition))
			continue;
		strncpy(m_plugin.PluginMagicWord, fmt("%08x", m_plugin.GetPluginMagic(i)), 8);
		const char *romDir = m_plugin.GetRomDir(i);
		if(strcasecmp(romDir, "scummvm.ini") != 0)
		{
			if(strncasecmp(m_plugin.PluginMagicWord, "484252", 6) == 0)//HBRW
			{
				if(updateCache)
					m_cfg.setBool(HOMEBREW_DOMAIN, "update_cache", true);
				_loadHomebrewList(romDir);
			}
			else if(strncasecmp(m_plugin.PluginMagicWord, "4E47434D", 8) == 0)//NGCM
			{
				if(updateCache)
					m_cfg.setBool(GC_DOMAIN, "update_cache", true);
				_loadGamecubeList();
			}
			else if(strncasecmp(m_plugin.PluginMagicWord, "4E574949", 8) == 0)//NWII
			{
				if(updateCache)
					m_cfg.setBool(WII_DOMAIN, "update_cache", true);
				_loadWiiList();
			}
			else if(strncasecmp(m_plugin.PluginMagicWord, "4E414E44", 8) == 0)//NAND
			{
				if(updateCache)
					m_cfg.setBool(CHANNEL_DOMAIN, "update_cache", true);
				m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL);
				_loadChannelList();
			}
			else if(strncasecmp(m_plugin.PluginMagicWord, "454E414E", 8) == 0)//ENAN
			{
				if(updateCache)
					m_cfg.setBool(CHANNEL_DOMAIN, "update_cache", true);
				m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_EMU);
				_loadChannelList();
			}
			else
			{
				string romsDir(fmt("%s:/%s", DeviceName[currentPartition], romDir));
				string cachedListFile(fmt("%s/%s_%s.db", m_listCacheDir.c_str(), DeviceName[currentPartition], m_plugin.PluginMagicWord));
				bool preCachedList = fsop_FileExist(cachedListFile.c_str());
				vector<string> FileTypes = stringToVector(m_plugin.GetFileTypes(i), '|');
				m_cacheList.Color = m_plugin.GetCaseColor(i);
				m_cacheList.Magic = m_plugin.GetPluginMagic(i);
				m_cacheList.usePluginDBTitles = m_cfg.getBool(PLUGIN_DOMAIN, "database_titles", true);
				m_cacheList.CreateRomList(m_platform, romsDir, FileTypes, cachedListFile, updateCache);
				for(vector<dir_discHdr>::iterator tmp_itr = m_cacheList.begin(); tmp_itr != m_cacheList.end(); tmp_itr++)
					m_gameList.push_back(*tmp_itr);
				if(updateCache || (!preCachedList && fsop_FileExist(cachedListFile.c_str())))
					cacheCovers = true;
			}
		}
		else
		{
			string cachedListFile(fmt("%s/%s_%s.db", m_listCacheDir.c_str(), DeviceName[currentPartition], m_plugin.PluginMagicWord));
			bool preCachedList = fsop_FileExist(cachedListFile.c_str());
			
			Config scummvm;
			if(!scummvm.load(fmt("%s/scummvm.ini", m_pluginsDir.c_str())))
			{
				if(!scummvm.load(fmt("%s/scummvm/scummvm.ini", m_pluginsDir.c_str())))
					scummvm.load(fmt("%s:/apps/scummvm/scummvm.ini", DeviceName[currentPartition]));
			}
			string platformName = "";
			if(m_platform.loaded())/* convert plugin magic to platform name */
				platformName = m_platform.getString("PLUGINS", m_plugin.PluginMagicWord);
			m_cacheList.Color = m_plugin.GetCaseColor(i);
			m_cacheList.Magic = m_plugin.GetPluginMagic(i);
			m_cacheList.ParseScummvmINI(scummvm, DeviceName[currentPartition], m_pluginDataDir.c_str(), platformName.c_str(), cachedListFile, updateCache);
			for(vector<dir_discHdr>::iterator tmp_itr = m_cacheList.begin(); tmp_itr != m_cacheList.end(); tmp_itr++)
				m_gameList.push_back(*tmp_itr);
			if(updateCache || (!preCachedList && fsop_FileExist(cachedListFile.c_str())))
				cacheCovers = true;
			scummvm.unload();
		}
	}
	m_cfg.remove(PLUGIN_DOMAIN, "update_cache");
	m_cfg.setInt(CHANNEL_DOMAIN, "channels_type", channels_type);
	return true;
}

void CMenu::_stopSounds(void)
{
	// Fade out sounds
	int fade_rate = m_cfg.getInt("GENERAL", "music_fade_rate", 8);

	if(!MusicPlayer.IsStopped())
	{
		while(MusicPlayer.GetVolume() > 0 || m_gameSound.GetVolume() > 0)
		{
			MusicPlayer.Tick(true);
			if(m_gameSound.GetVolume() > 0)
				m_gameSound.SetVolume(m_gameSound.GetVolume() < fade_rate ? 0 : m_gameSound.GetVolume() - fade_rate);
			VIDEO_WaitVSync();
		}
	}
	m_btnMgr.stopSounds();
	CoverFlow.stopSound();
	m_gameSound.Stop();
}

bool CMenu::_loadFile(u8 * &buffer, u32 &size, const char *path, const char *file)
{
	u32 fileSize = 0;
	u8 *fileBuf = fsop_ReadFile(file == NULL ? path : fmt("%s/%s", path, file), &fileSize);
	if(fileBuf == NULL)
		return false;

	if(buffer != NULL)
		MEM2_free(buffer);
	buffer = fileBuf;
	size = fileSize;
	return true;
}

/* wiiflow creates a map<u8, u8> _installed_cios list for slots 200 to 253 and slot 0
the first u8 is the slot and the second u8 is the base if its a d2x cios otherwise the slot number again.
slot 0 is set to 1 - first = 0 and second = 1
game config only shows the first (slot) or auto if first = 0 */
void CMenu::_load_installed_cioses()
{
	if(isWiiVC)
		return;
	gprintf("Loading cIOS map\n");
	_installed_cios[0] = 1;

	for(u8 slot = 200; slot < 254; slot++)
	{
		u8 base = 1;
		if(IOS_D2X(slot, &base))
		{
			gprintf("Found d2x base %u in slot %u\n", base, slot);
			_installed_cios[slot] = base;
		}
		else if(CustomIOS(IOS_GetType(slot)))
		{
			gprintf("Found cIOS in slot %u\n", slot);
			_installed_cios[slot] = slot;
		}
	}
}

void CMenu::_hideWaitMessage()
{
	m_vid.hideWaitMessage();
}

void CMenu::_showWaitMessage()
{
	m_vid.waitMessage(0.15f);
}

typedef struct map_entry
{
	char filename[8];
	u8 sha1[20];
} ATTRIBUTE_PACKED map_entry_t;

void CMenu::_loadDefaultFont(void)
{
	if(m_base_font != NULL)
		return;

	u32 size = 0;
	bool retry = false;
	bool korean = (CONF_GetLanguage() == CONF_LANG_KOREAN);
	char ISFS_Filename[32] ATTRIBUTE_ALIGN(32);

	// Read content.map from ISFS
	strcpy(ISFS_Filename, "/shared1/content.map");
	u8 *content = ISFS_GetFile(ISFS_Filename, &size, -1);
	if(content == NULL)
		return;

	u32 items = size / sizeof(map_entry_t);
	//gprintf("Open content.map, size %d, items %d\n", size, items);
	map_entry_t *cm = (map_entry_t *)content;

retry:
	bool kor_font = (korean && !retry) || (!korean && retry);
	for(u32 i = 0; i < items; i++)
	{
		if(m_base_font != NULL && m_wbf1_font != NULL && m_wbf2_font != NULL)
			break;
		if(memcmp(cm[i].sha1, kor_font ? WIIFONT_HASH_KOR : WIIFONT_HASH, 20) == 0 && m_base_font == NULL)
		{
			sprintf(ISFS_Filename, "/shared1/%.8s.app", cm[i].filename);  //who cares about the few ticks more?
			u8 *u8_font_archive = ISFS_GetFile(ISFS_Filename, &size, -1);
			if(u8_font_archive != NULL)
			{
				const u8 *font_file = u8_get_file_by_index(u8_font_archive, 1, &size); // There is only one file in that app
				//gprintf("Extracted font: %d\n", size);
				m_base_font = (u8*)MEM1_lo_alloc(size);
				memcpy(m_base_font, font_file, size);
				DCFlushRange(m_base_font, size);
				m_base_font_size = size;
				MEM2_free(u8_font_archive);
			}
		}
		else if(memcmp(cm[i].sha1, WFB_HASH, 20) == 0 && m_wbf1_font == NULL && m_wbf2_font == NULL)
		{
			sprintf(ISFS_Filename, "/shared1/%.8s.app", cm[i].filename);  //who cares about the few ticks more?
			u8 *u8_font_archive = ISFS_GetFile(ISFS_Filename, &size, -1);
			if(u8_font_archive != NULL)
			{
				const u8 *font_file1 = u8_get_file(u8_font_archive, "wbf1.brfna", &size);
				m_wbf1_font = (u8*)MEM1_lo_alloc(size);
				memcpy(m_wbf1_font, font_file1, size);
				DCFlushRange(m_wbf1_font, size);
	
				const u8 *font_file2 = u8_get_file(u8_font_archive, "wbf2.brfna", &size);
				m_wbf2_font = (u8*)MEM1_lo_alloc(size);
				memcpy(m_wbf2_font, font_file2, size);
				DCFlushRange(m_wbf2_font, size);

				MEM2_free(u8_font_archive);
			}
		}
	}
	if(!retry && m_base_font == NULL)
	{
		retry = true;
		goto retry;
	}
	MEM2_free(content);
}

void CMenu::_cleanupDefaultFont()
{
	MEM1_lo_free(m_base_font);
	m_base_font = NULL;
	m_base_font_size = 0;
	MEM1_lo_free(m_wbf1_font);
	m_wbf1_font = NULL;
	MEM1_lo_free(m_wbf2_font);
	m_wbf2_font = NULL;
}

const char *CMenu::_domainFromView()
{
	if(m_sourceflow)
		return SOURCEFLOW_DOMAIN;
	if(m_source_cnt > 1)
		return "MULTI";
	switch(m_current_view)
	{
		case COVERFLOW_CHANNEL:
			return CHANNEL_DOMAIN;
		case COVERFLOW_HOMEBREW:
			return HOMEBREW_DOMAIN;
		case COVERFLOW_GAMECUBE:
			return GC_DOMAIN;
		case COVERFLOW_PLUGIN:
			return PLUGIN_DOMAIN;
		default:
			return WII_DOMAIN;
	}
	return "NULL";
}

void CMenu::RemoveCover(const char *id)
{
	const char *CoverPath = NULL;
	if(id == NULL)
		return;
	CoverPath = fmt("%s/%s.png", m_boxPicDir.c_str(), id);
	fsop_deleteFile(CoverPath);
	CoverPath = fmt("%s/%s.png", m_picDir.c_str(), id);
	fsop_deleteFile(CoverPath);
	CoverPath = fmt("%s/%s.wfc", m_cacheDir.c_str(), id);
	fsop_deleteFile(CoverPath);
}

/* if wiiflow using IOS58 this switches to cIOS for certain functions and back to IOS58 when done. */
/* if wiiflow using cIOS no need to temp switch */
void CMenu::TempLoadIOS(int IOS)
{
	/* Only temp reload in IOS58 mode */
	if(useMainIOS)
		return;

	if(IOS == IOS_TYPE_NORMAL_IOS)
		IOS = 58;
	else if(IOS == 0)
		IOS = mainIOS;

	if(CurrentIOS.Version != IOS)
	{
		loadIOS(IOS, true);// switch to new IOS
		Sys_Init();
		Open_Inputs();
		for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
			WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
		_netInit();
	}
}

static char blankCoverPath[MAX_FAT_PATH];
const char *CMenu::getBlankCoverPath(const dir_discHdr *element)
{
	string blankCoverTitle = "wii";
	if(m_platform.loaded())
	{
		switch(element->type)
		{
			case TYPE_CHANNEL:
				strncpy(m_plugin.PluginMagicWord, "4E414E44", 9);
				break;
			case TYPE_EMUCHANNEL:
				strncpy(m_plugin.PluginMagicWord, "454E414E", 9);
				break;
			case TYPE_HOMEBREW:
				strncpy(m_plugin.PluginMagicWord, "48425257", 9);
				break;
			case TYPE_GC_GAME:
				strncpy(m_plugin.PluginMagicWord, "4E47434D", 9);
				break;
			case TYPE_PLUGIN:
				strncpy(m_plugin.PluginMagicWord, fmt("%08x", element->settings[0]), 8);
				break;
			default:// wii
				strncpy(m_plugin.PluginMagicWord, "4E574949", 9);
		}
		blankCoverTitle = m_platform.getString("PLUGINS", m_plugin.PluginMagicWord, "wii");
	}
	snprintf(blankCoverPath, sizeof(blankCoverPath), "%s/blank_covers/%s.png", m_boxPicDir.c_str(), blankCoverTitle.c_str());
	if(!fsop_FileExist(blankCoverPath))
		snprintf(blankCoverPath, sizeof(blankCoverPath), "%s/blank_covers/%s.jpg", m_boxPicDir.c_str(), blankCoverTitle.c_str());
	return blankCoverPath;
}

const char *CMenu::getBoxPath(const dir_discHdr *element)
{
	if(element->type == TYPE_PLUGIN)
	{
		const char *tempname = element->path;
		if(strchr(element->path, '/') != NULL)
			tempname = strrchr(element->path, '/') + 1;
		const char *coverFolder = m_plugin.GetCoverFolderName(element->settings[0]);
		if(strlen(coverFolder) > 0)
			return fmt("%s/%s/%s.png", m_boxPicDir.c_str(), coverFolder, tempname);
		else
			return fmt("%s/%s.png", m_boxPicDir.c_str(), tempname);
	}
	else if(element->type == TYPE_HOMEBREW)
		return fmt("%s/homebrew/%s.png", m_boxPicDir.c_str(), strrchr(element->path, '/') + 1);
	else if(element->type == TYPE_SOURCE)//sourceflow
	{
		const char *coverImg = strrchr(element->path, '/') + 1;
		if(coverImg == NULL)
			return NULL;
		return fmt("%s/full_covers/%s", m_sourceDir.c_str(), coverImg);
	}
	return fmt("%s/%s.png", m_boxPicDir.c_str(), element->id);
}

const char *CMenu::getFrontPath(const dir_discHdr *element)
{
	if(element->type == TYPE_PLUGIN)
	{
		const char *tempname = element->path;
		if(strchr(element->path, '/') != NULL)
			tempname = strrchr(element->path, '/') + 1;
		const char *coverFolder = m_plugin.GetCoverFolderName(element->settings[0]);
		if(strlen(coverFolder) > 0)
			return fmt("%s/%s/%s.png", m_picDir.c_str(), coverFolder, tempname);
		else
			return fmt("%s/%s.png", m_picDir.c_str(), tempname);
	}
	else if(element->type == TYPE_HOMEBREW)
	{
		if(m_cfg.getBool(HOMEBREW_DOMAIN, "smallbox"))
			return fmt("%s/icon.png", element->path);
		else
			return fmt("%s/homebrew/%s.png", m_picDir.c_str(), strrchr(element->path, '/') + 1);
	}
	else if(element->type == TYPE_SOURCE)//sourceflow
	{
		const char *coverImg = strrchr(element->path, '/') + 1;
		if(coverImg == NULL)
			return NULL;
		const char *coverPath = fmt("%s/front_covers/%s", m_sourceDir.c_str(), coverImg);
		if(m_cfg.getBool(SOURCEFLOW_DOMAIN, "smallbox") || !fsop_FileExist(coverPath))
		{
			string themeName = m_cfg.getString("GENERAL", "theme", "default");
			coverPath = fmt("%s/small_covers/%s/%s", m_sourceDir.c_str(), themeName.c_str(), coverImg);
			if(!fsop_FileExist(coverPath))
			{
				coverPath = fmt("%s/small_covers/%s", m_sourceDir.c_str(), coverImg);
				if(!fsop_FileExist(coverPath))
					return element->path;
			}
		}
		return coverPath;
	}
	return fmt("%s/%s.png", m_picDir.c_str(), element->id);
}
