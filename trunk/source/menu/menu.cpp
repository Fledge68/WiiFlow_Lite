
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
#include "fileOps/fileOps.h"
#include "gc/gc.hpp"
#include "gui/Gekko.h"
#include "gui/GameTDB.hpp"
#include "loader/alt_ios.h"
#include "loader/cios.h"
#include "loader/fs.h"
#include "loader/nk.h"
#include "loader/playlog.h"
#include "loader/sys.h"
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
// Pics
extern const u8 btnplus_png[];
extern const u8 btnpluss_png[];
extern const u8 btnminus_png[];
extern const u8 btnminuss_png[];
extern const u8 background_jpg[];
extern const u32 background_jpg_size;
extern const u8 butleft_png[];
extern const u8 butcenter_png[];
extern const u8 butright_png[];
extern const u8 butsleft_png[];
extern const u8 butscenter_png[];
extern const u8 butsright_png[];
extern const u8 buthleft_png[];
extern const u8 buthcenter_png[];
extern const u8 buthright_png[];
extern const u8 buthsleft_png[];
extern const u8 buthscenter_png[];
extern const u8 buthsright_png[];
extern const u8 pbarleft_png[];
extern const u8 pbarcenter_png[];
extern const u8 pbarright_png[];
extern const u8 pbarlefts_png[];
extern const u8 pbarcenters_png[];
extern const u8 pbarrights_png[];
extern const u8 butauon_png[];
extern const u8 butauons_png[];
extern const u8 butauoff_png[];
extern const u8 butauoffs_png[];
extern const u8 butenon_png[];
extern const u8 butenons_png[];
extern const u8 butenoff_png[];
extern const u8 butenoffs_png[];
extern const u8 butjaon_png[];
extern const u8 butjaons_png[];
extern const u8 butjaoff_png[];
extern const u8 butjaoffs_png[];
extern const u8 butfron_png[];
extern const u8 butfrons_png[];
extern const u8 butfroff_png[];
extern const u8 butfroffs_png[];
extern const u8 butdeon_png[];
extern const u8 butdeons_png[];
extern const u8 butdeoff_png[];
extern const u8 butdeoffs_png[];
extern const u8 buteson_png[];
extern const u8 butesons_png[];
extern const u8 butesoff_png[];
extern const u8 butesoffs_png[];
extern const u8 butiton_png[];
extern const u8 butitons_png[];
extern const u8 butitoff_png[];
extern const u8 butitoffs_png[];
extern const u8 butnlon_png[];
extern const u8 butnlons_png[];
extern const u8 butnloff_png[];
extern const u8 butnloffs_png[];
extern const u8 butpton_png[];
extern const u8 butptons_png[];
extern const u8 butptoff_png[];
extern const u8 butptoffs_png[];
extern const u8 butruon_png[];
extern const u8 butruons_png[];
extern const u8 butruoff_png[];
extern const u8 butruoffs_png[];
extern const u8 butkoon_png[];
extern const u8 butkoons_png[];
extern const u8 butkooff_png[];
extern const u8 butkooffs_png[];
extern const u8 butzhcnon_png[];
extern const u8 butzhcnons_png[];
extern const u8 butzhcnoff_png[];
extern const u8 butzhcnoffs_png[];
extern const u8 checkbox_png[];
extern const u8 checkboxs_png[];
extern const u8 checkboxhid_png[];
extern const u8 checkboxreq_png[];

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
	m_networkInit = false;
	m_thrdNetwork = false;
	m_mutex = 0;
	m_showtimer = 0;
	m_gameSoundThread = LWP_THREAD_NULL;
	m_soundThrdBusy = false;
	m_numCFVersions = 0;
	m_bgCrossFade = 0;
	m_bnrSndVol = 0;
	m_bnr_settings = true;
	m_directLaunch = false;
	m_exit = false;
	m_reload = false;
	m_gamesound_changed = false;
	m_video_playing = false;
	m_base_font = NULL;
	m_base_font_size = 0;
	m_wbf1_font = NULL;
	m_wbf2_font = NULL;
	m_current_view = COVERFLOW_USB;
	m_Emulator_boot = false;
	m_music_info = true;
}

void CMenu::init()
{
	SoundHandle.Init();
	m_gameSound.SetVoice(1);
	const char *drive = "empty";
	const char *check = "empty";
	struct stat dummy;

	/* Clear Playlog */
	Playlog_Delete();

	for(int i = SD; i <= USB8; i++) //Find the first partition with a wiiflow.ini
		if (DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) != PART_FS_WBFS && stat(fmt("%s:/%s/" CFG_FILENAME, DeviceName[i], APPDATA_DIR2), &dummy) == 0)
		{
			drive = DeviceName[i];
			break;
		}

	if(drive == check) //No wiiflow.ini found
		for(int i = SD; i <= USB8; i++) //Find the first partition with a boot.dol
			if (DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) != PART_FS_WBFS && stat(fmt("%s:/%s/boot.dol", DeviceName[i], APPDATA_DIR2), &dummy) == 0)
			{
				drive = DeviceName[i];
				break;
			}
			
	if(drive == check) //No boot.dol found
		for(int i = SD; i <= USB8; i++) //Find the first partition with apps/wiiflow folder
			if (DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) != PART_FS_WBFS && stat(fmt("%s:/%s", DeviceName[i], APPDATA_DIR2), &dummy) == 0)
			{
				drive = DeviceName[i];
				break;
			}

	if(drive == check) //No apps/wiiflow folder found
		for(int i = SD; i <= USB8; i++) // Find the first writable partition
			if (DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) != PART_FS_WBFS)
			{
				drive = DeviceName[i];
				fsop_MakeFolder((char *)fmt("%s:/%s", DeviceName[i], APPDATA_DIR2)); //Make the apps dir, so saving wiiflow.ini does not fail.
				break;
			}
	loadDefaultFont();

	if(drive == check) // Should not happen
	{
		_buildMenus();
		error(_fmt("errboot4", L"No available partitions found!"));
		m_exit = true;
		return;
	}

	m_appDir = sfmt("%s:/%s", drive, APPDATA_DIR2);
	m_cfg.load(sfmt("%s/" CFG_FILENAME, m_appDir.c_str()).c_str());
	m_use_wifi_gecko = m_cfg.getBool("DEBUG", "wifi_gecko");
	if (m_cfg.getBool("GENERAL", "async_network") || has_enabled_providers() || m_use_wifi_gecko)
		_reload_wifi_gecko();

	gprintf("Wiiflow boot.dol Location: %s\n", m_appDir.c_str());

	//Gecko Output to SD
	if(!WriteToSD)
	{
		WriteToSD = m_cfg.getBool("DEBUG", "sd_write_log", false);
		bufferMessages = WriteToSD;
	}
	int ForceIOS = min(m_cfg.getInt("GENERAL", "force_cios_rev", 0), 254);
	if(ForceIOS > 0)
	{
		gprintf("Using IOS%i instead of IOS%i as main cIOS.\n", ForceIOS, mainIOS);
		mainIOS = ForceIOS;
	}
	useMainIOS = m_cfg.getBool("GENERAL", "force_cios_load", false);
	bool onUSB = m_cfg.getBool("GENERAL", "data_on_usb", strncmp(drive, "usb", 3) == 0);

	drive = check; //reset the drive variable for the check

	if (onUSB)
	{
		for(int i = USB1; i <= USB8; i++) //Look for first partition with a wiiflow folder in root
			if (DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) != PART_FS_WBFS && stat(fmt("%s:/%s", DeviceName[i], APPDATA_DIR), &dummy) == 0)
			{
				drive = DeviceName[i];
				break;
			}
	}
	else if(DeviceHandle.IsInserted(SD)) drive = DeviceName[SD];

	if(drive == check && onUSB) //No wiiflow folder found in root of any usb partition, and data_on_usb=yes
		for(int i = USB1; i <= USB8; i++) // Try first USB partition with wbfs folder.
			if (DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) != PART_FS_WBFS && stat(fmt(GAMES_DIR, DeviceName[i]), &dummy) == 0)
			{
				drive = DeviceName[i];
				break;
			}

	if(drive == check && onUSB) // No wbfs folder found and data_on_usb=yes
		for(int i = USB1; i <= USB8; i++) // Try first available USB partition.
			if (DeviceHandle.IsInserted(i) && DeviceHandle.GetFSType(i) != PART_FS_WBFS)
			{
				drive = DeviceName[i];
				break;
			}

	if(drive == check)
	{	
		_buildMenus();
		if(DeviceHandle.IsInserted(SD))
		{
			error(_fmt("errboot5", L"data_on_usb=yes and No available usb partitions for data!\nUsing SD."));
			drive = DeviceName[SD];
		}
		else
		{
			error(_fmt("errboot6", L"No available usb partitions for data and no SD inserted!\nExiting."));
			m_exit = true;
			return;
		}
	}

	if(m_cfg.getBool(GC_DOMAIN, "always_show_button", false))
	{
		gprintf("Force enabling DML view\n");
		m_show_dml = true;
	}
	else
		m_show_dml = MIOSisDML();

	m_new_dml = m_cfg.getBool(GC_DOMAIN, "dml_r52+", true);
	m_new_dm_cfg = m_cfg.getBool(GC_DOMAIN, "dm_r2.1+", true);
	m_DMLgameDir = sfmt("%%s:/%s", m_cfg.getString(GC_DOMAIN, "dir_usb_games", "games").c_str());

	m_cfg.getString(CHANNEL_DOMAIN, "path", "");
	m_cfg.getInt(CHANNEL_DOMAIN, "partition", 1);
	m_cfg.getBool(CHANNEL_DOMAIN, "disable", true);

	_installed_cios.clear();
	if(!neek2o())
		_load_installed_cioses();
	else
		_installed_cios[CurrentIOS.Version] = CurrentIOS.Version;

	snprintf(m_app_update_drive, sizeof(m_app_update_drive), "%s:/", drive);
	m_dataDir = sfmt("%s:/%s", drive, APPDATA_DIR);
	gprintf("Data Directory: %s\n", m_dataDir.c_str());

	m_dol = sfmt("%s/boot.dol", m_appDir.c_str());
	m_ver = sfmt("%s/versions", m_appDir.c_str());
	m_app_update_zip = sfmt("%s/update.zip", m_appDir.c_str());
	m_data_update_zip = sfmt("%s/update.zip", m_dataDir.c_str());

	m_customBnrDir = m_cfg.getString("GENERAL", "dir_custom_banners", sfmt("%s/custom_banners", m_dataDir.c_str()));
	m_pluginsDir = m_cfg.getString("GENERAL", "dir_plugins", sfmt("%s/plugins", m_dataDir.c_str()));

	m_cacheDir = m_cfg.getString("GENERAL", "dir_cache", sfmt("%s/cache", m_dataDir.c_str()));
	m_listCacheDir = m_cfg.getString("GENERAL", "dir_list_cache", sfmt("%s/lists", m_cacheDir.c_str()));
	m_bnrCacheDir = m_cfg.getString("GENERAL", "dir_banner_cache", sfmt("%s/banners", m_cacheDir.c_str()));

	m_txtCheatDir = m_cfg.getString("GENERAL", "dir_txtcheat", sfmt("%s/codes", m_dataDir.c_str()));
	m_cheatDir = m_cfg.getString("GENERAL", "dir_cheat", sfmt("%s/gct", m_txtCheatDir.c_str()));
	m_wipDir = m_cfg.getString("GENERAL", "dir_wip", sfmt("%s/wip", m_txtCheatDir.c_str()));

	m_settingsDir = m_cfg.getString("GENERAL", "dir_settings", sfmt("%s/settings", m_dataDir.c_str()));
	m_languagesDir = m_cfg.getString("GENERAL", "dir_languages", sfmt("%s/languages", m_dataDir.c_str()));
	m_boxPicDir = m_cfg.getString("GENERAL", "dir_box_covers", sfmt("%s/boxcovers", m_dataDir.c_str()));
	m_picDir = m_cfg.getString("GENERAL", "dir_flat_covers", sfmt("%s/covers", m_dataDir.c_str()));
	m_themeDir = m_cfg.getString("GENERAL", "dir_themes", sfmt("%s/themes", m_dataDir.c_str()));
	m_musicDir = m_cfg.getString("GENERAL", "dir_music", sfmt("%s/music", m_dataDir.c_str())); 
	m_videoDir = m_cfg.getString("GENERAL", "dir_trailers", sfmt("%s/trailers", m_dataDir.c_str()));
	m_fanartDir = m_cfg.getString("GENERAL", "dir_fanart", sfmt("%s/fanart", m_dataDir.c_str()));
	m_screenshotDir = m_cfg.getString("GENERAL", "dir_screenshot", sfmt("%s/screenshots", m_dataDir.c_str()));
	m_helpDir = m_cfg.getString("GENERAL", "dir_help", sfmt("%s/help", m_dataDir.c_str()));
	
	//DeviceHandler::SetWatchdog(m_cfg.getUInt("GENERAL", "watchdog_timeout", 10));

	const char *domain = _domainFromView();
	const char *checkDir = m_current_view == COVERFLOW_HOMEBREW ? HOMEBREW_DIR : GAMES_DIR;

	u8 partition = m_cfg.getInt(domain, "partition", 0);  //Auto find a valid partition and fix old ini partition settings.
	if(m_current_view != COVERFLOW_CHANNEL && (partition > USB8 || !DeviceHandle.IsInserted(partition)))
	{
		m_cfg.remove(domain, "partition");
		for(int i = SD; i <= USB8+1; i++) // Find a usb partition with the wbfs folder or wbfs file system, else leave it blank (defaults to 1 later)
		{
			if(i > USB8)
			{
				m_current_view = COVERFLOW_CHANNEL;
				break;
			}
			if (DeviceHandle.IsInserted(i)
				&& ((m_current_view == COVERFLOW_USB && DeviceHandle.GetFSType(i) == PART_FS_WBFS)
				|| stat(fmt(checkDir, DeviceName[i]), &dummy) == 0))
			{
				gprintf("Setting Emu NAND to Partition: %i\n",currentPartition);
				m_cfg.setInt(domain, "partition", i);
				break;
			}
		}
	}
	CoverFlow.init(m_base_font, m_base_font_size, m_vid.vid_50hz());

	//Make important folders first.
	fsop_MakeFolder((char *)m_dataDir.c_str()); //D'OH!

	fsop_MakeFolder((char *)m_customBnrDir.c_str());
	fsop_MakeFolder((char *)m_pluginsDir.c_str());

	fsop_MakeFolder((char *)m_cacheDir.c_str());
	fsop_MakeFolder((char *)m_listCacheDir.c_str());
	fsop_MakeFolder((char *)m_bnrCacheDir.c_str());

	fsop_MakeFolder((char *)m_txtCheatDir.c_str());
	fsop_MakeFolder((char *)m_cheatDir.c_str());
	fsop_MakeFolder((char *)m_wipDir.c_str());

	fsop_MakeFolder((char *)m_settingsDir.c_str());
	fsop_MakeFolder((char *)m_languagesDir.c_str());
	fsop_MakeFolder((char *)m_boxPicDir.c_str());
	fsop_MakeFolder((char *)m_picDir.c_str());
	fsop_MakeFolder((char *)m_themeDir.c_str());
	fsop_MakeFolder((char *)m_musicDir.c_str());
	fsop_MakeFolder((char *)m_videoDir.c_str());
	fsop_MakeFolder((char *)m_fanartDir.c_str());
	fsop_MakeFolder((char *)m_screenshotDir.c_str());
	fsop_MakeFolder((char *)m_helpDir.c_str());

	// INI files
	m_cat.load(fmt("%s/" CAT_FILENAME, m_settingsDir.c_str()));
	string themeName = m_cfg.getString("GENERAL", "theme", "default");
	m_themeDataDir = sfmt("%s/%s", m_themeDir.c_str(), themeName.c_str());
	m_theme.load(fmt("%s.ini", m_themeDataDir.c_str()));
	m_plugin.init(m_pluginsDir);

	m_devo_installed = DEVO_Installed(m_dataDir.c_str());
	u8 defaultMenuLanguage = 7; //English
	switch (CONF_GetLanguage())
	{
		case CONF_LANG_JAPANESE:
			defaultMenuLanguage = 14; //Japanese
			break;
		case CONF_LANG_GERMAN:
			defaultMenuLanguage = 11; //German
			break;
		case CONF_LANG_FRENCH:
			defaultMenuLanguage = 9; //French
			break;
		case CONF_LANG_SPANISH:
			defaultMenuLanguage = 19; //Spanish
			break;
		case CONF_LANG_ITALIAN:
			defaultMenuLanguage = 13; //Italian
			break;
		case CONF_LANG_DUTCH:
			defaultMenuLanguage = 6; //Dutch
			break;
		case CONF_LANG_SIMP_CHINESE:
			defaultMenuLanguage = 3; //Chinese_S
			break;
		case CONF_LANG_TRAD_CHINESE:
			defaultMenuLanguage = 4; //Chinese_T
			break;
		case CONF_LANG_KOREAN:
			defaultMenuLanguage = 7; // No Korean translation has been done for wiiflow, so the menu will use english in this case.
			break;
	}
	if (CONF_GetArea() == CONF_AREA_BRA)
		defaultMenuLanguage = 2; //Brazilian

	m_curLanguage = CMenu::_translations[m_cfg.getInt("GENERAL", "language", defaultMenuLanguage)];
	if (!m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str())))
	{
		m_cfg.setInt("GENERAL", "language", 0);
		m_curLanguage = CMenu::_translations[0];
		m_loc.load(fmt("%s/%s.ini", m_languagesDir.c_str(), m_curLanguage.c_str()));
	}
	m_tempView = false;

	m_gameList.Init(m_settingsDir.c_str(), m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());

	m_aa = 3;

	CColor pShadowColor = m_theme.getColor("GENERAL", "pointer_shadow_color", CColor(0x3F000000));
	float pShadowX = m_theme.getFloat("GENERAL", "pointer_shadow_x", 3.f);
	float pShadowY = m_theme.getFloat("GENERAL", "pointer_shadow_y", 3.f);
	bool pShadowBlur = m_theme.getBool("GENERAL", "pointer_shadow_blur", false);

	for(int chan = WPAD_MAX_WIIMOTES-2; chan >= 0; chan--)
	{
		m_cursor[chan].init(fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString("GENERAL", fmt("pointer%i", chan+1)).c_str()),
			m_vid.wide(), pShadowColor, pShadowX, pShadowY, pShadowBlur, chan);
		WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
	}

	m_btnMgr.init();
	MusicPlayer.Init(m_cfg, m_musicDir, sfmt("%s/music", m_themeDataDir.c_str()));
	m_music_info = m_cfg.getBool("GENERAL", "display_music_info", true);

	_buildMenus();

	m_locked = m_cfg.getString("GENERAL", "parent_code", "").size() >= 4;
	m_btnMgr.setRumble(m_cfg.getBool("GENERAL", "rumble", true));

	int exit_to = m_cfg.getInt("GENERAL", "exit_to", 0);
	if(exit_to == EXIT_TO_BOOTMII && (!DeviceHandle.IsInserted(SD) || 
	stat(fmt("%s:/bootmii/armboot.bin",DeviceName[SD]), &dummy) != 0 || 
	stat(fmt("%s:/bootmii/ppcboot.elf", DeviceName[SD]), &dummy) != 0))
		exit_to = EXIT_TO_HBC;
	Sys_ExitTo(exit_to);

	LWP_MutexInit(&m_mutex, 0);

	CoverFlow.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_coverflow", 255));
	m_btnMgr.setSoundVolume(m_cfg.getInt("GENERAL", "sound_volume_gui", 255));
	m_bnrSndVol = m_cfg.getInt("GENERAL", "sound_volume_bnr", 255);
	m_bnr_settings = m_cfg.getBool("GENERAL", "banner_in_settings", true);

	m_cfg.setString("GAMERCARD", "gamercards", "wiinnertag|dutag");
	m_cfg.getString("GAMERCARD", "wiinnertag_url", WIINNERTAG_URL);
	m_cfg.getString("GAMERCARD", "wiinnertag_key", "");
	m_cfg.getString("GAMERCARD", "dutag_url", DUTAG_URL);
	m_cfg.getString("GAMERCARD", "dutag_key", "");
	if (m_cfg.getBool("GAMERCARD", "gamercards_enable", false))
	{
		vector<string> gamercards = stringToVector(m_cfg.getString("GAMERCARD", "gamercards"), '|');
		if (gamercards.size() == 0)
		{
			gamercards.push_back("wiinnertag");
			gamercards.push_back("dutag");
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
}

bool cleaned_up = false;

void CMenu::cleanup()
{
	if(cleaned_up)
		return;
	//gprintf("MEM1_freesize(): %i\nMEM2_freesize(): %i\n", MEM1_freesize(), MEM2_freesize());
	m_btnMgr.hide(m_mainLblCurMusic);
	_cleanupDefaultFont();
	m_banner.DeleteBanner();
	m_plugin.Cleanup();

	_stopSounds();
	_Theme_Cleanup();
	MusicPlayer.Cleanup();
	m_gameSound.FreeMemory();
	SoundHandle.Cleanup();
	soundDeinit();

	m_vid.cleanup();
	CoverFlow.shutdown();

	wiiLightOff();
	_deinitNetwork();
	Close_Inputs();

	LWP_MutexDestroy(m_mutex);
	m_mutex = 0;

	cleaned_up = true;
	//gprintf(" \nMemory cleaned up\n");
	gprintf("MEM1_freesize(): %i\nMEM2_freesize(): %i\n", MEM1_freesize(), MEM2_freesize());
}

void CMenu::_Theme_Cleanup(void)
{
	/* Backgrounds */
	theme.bg.Cleanup();
	m_prevBg.Cleanup();
	m_nextBg.Cleanup();
	m_curBg.Cleanup();
	m_lqBg.Cleanup();
	/* Buttons */
	theme.btnTexL.Cleanup();
	theme.btnTexR.Cleanup();
	theme.btnTexC.Cleanup();
	theme.btnTexLS.Cleanup();
	theme.btnTexRS.Cleanup();
	theme.btnTexCS.Cleanup();
	theme.btnTexLH.Cleanup();
	theme.btnTexRH.Cleanup();
	theme.btnTexCH.Cleanup();
	theme.btnTexLSH.Cleanup();
	theme.btnTexRSH.Cleanup();
	theme.btnTexCSH.Cleanup();
	theme.btnAUOn.Cleanup();
	theme.btnAUOns.Cleanup();
	theme.btnAUOff.Cleanup();
	theme.btnAUOffs.Cleanup();
	theme.btnENOn.Cleanup();
	theme.btnENOns.Cleanup();
	theme.btnENOff.Cleanup();
	theme.btnENOffs.Cleanup();
	theme.btnJAOn.Cleanup();
	theme.btnJAOns.Cleanup();
	theme.btnJAOff.Cleanup();
	theme.btnJAOffs.Cleanup();
	theme.btnFROn.Cleanup();
	theme.btnFROns.Cleanup();
	theme.btnFROff.Cleanup();
	theme.btnFROffs.Cleanup();
	theme.btnDEOn.Cleanup();
	theme.btnDEOns.Cleanup();
	theme.btnDEOff.Cleanup();
	theme.btnDEOffs.Cleanup();
	theme.btnESOn.Cleanup();
	theme.btnESOns.Cleanup();
	theme.btnESOff.Cleanup();
	theme.btnESOffs.Cleanup();
	theme.btnITOn.Cleanup();
	theme.btnITOns.Cleanup();
	theme.btnITOff.Cleanup();
	theme.btnITOffs.Cleanup();
	theme.btnNLOn.Cleanup();
	theme.btnNLOns.Cleanup();
	theme.btnNLOff.Cleanup();
	theme.btnNLOffs.Cleanup();
	theme.btnPTOn.Cleanup();
	theme.btnPTOns.Cleanup();
	theme.btnPTOff.Cleanup();
	theme.btnPTOffs.Cleanup();
	theme.btnRUOn.Cleanup();
	theme.btnRUOns.Cleanup();
	theme.btnRUOff.Cleanup();
	theme.btnRUOffs.Cleanup();
	theme.btnKOOn.Cleanup();
	theme.btnKOOns.Cleanup();
	theme.btnKOOff.Cleanup();
	theme.btnKOOffs.Cleanup();
	theme.btnZHCNOn.Cleanup();
	theme.btnZHCNOns.Cleanup();
	theme.btnZHCNOff.Cleanup();
	theme.btnZHCNOffs.Cleanup();
	theme.btnTexPlus.Cleanup();
	theme.btnTexPlusS.Cleanup();
	theme.btnTexMinus.Cleanup();
	theme.btnTexMinusS.Cleanup();
	/* Checkboxes */
	theme.checkboxoff.Cleanup();
	theme.checkboxoffs.Cleanup();
	theme.checkboxon.Cleanup();
	theme.checkboxons.Cleanup();
	theme.checkboxHid.Cleanup();
	theme.checkboxHids.Cleanup();
	theme.checkboxReq.Cleanup();
	theme.checkboxReqs.Cleanup();
	/* Progress Bars */
	theme.pbarTexL.Cleanup();
	theme.pbarTexR.Cleanup();
	theme.pbarTexC.Cleanup();
	theme.pbarTexLS.Cleanup();
	theme.pbarTexRS.Cleanup();
	theme.pbarTexCS.Cleanup();
	/* Other Theme Stuff */
	for(TexSet::iterator texture = theme.texSet.begin(); texture != theme.texSet.end(); ++texture)
		texture->second.Cleanup();
	for(FontSet::iterator font = theme.fontSet.begin(); font != theme.fontSet.end(); ++font)
		font->second.ClearData();
	for(SoundSet::iterator sound = theme.soundSet.begin(); sound != theme.soundSet.end(); ++sound)
		sound->second->FreeMemory();
	theme.texSet.clear();
	theme.fontSet.clear();
	theme.soundSet.clear();
}

void CMenu::_reload_wifi_gecko(void)
{
	if(m_use_wifi_gecko)
	{
		_initAsyncNetwork();
		while(net_get_status() == -EBUSY)
			usleep(100);
	}
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
	CoverFlow.setCachePath(m_cacheDir.c_str(), !m_cfg.getBool("GENERAL", "keep_png", true), m_cfg.getBool("GENERAL", "compress_cache", false));
	CoverFlow.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 20));
	// Coverflow Sounds
	CoverFlow.setSounds(
		new GuiSound(fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_flip").c_str())),
		_sound(theme.soundSet, domain, "sound_hover", hover_wav, hover_wav_size, "default_btn_hover", false),
		_sound(theme.soundSet, domain, "sound_select", click_wav, click_wav_size, "default_btn_click", false),
		new GuiSound(fmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "sound_cancel").c_str()))
	);
	// Textures
	string texLoading = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_box").c_str());
	string texNoCover = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_box").c_str());
	string texLoadingFlat = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "loading_cover_flat").c_str());
	string texNoCoverFlat = sfmt("%s/%s", m_themeDataDir.c_str(), m_theme.getString(domain, "missing_cover_flat").c_str());
	CoverFlow.setTextures(texLoading, texLoadingFlat, texNoCover, texNoCoverFlat);
	// Font
	CoverFlow.setFont(_font(theme.fontSet, domain, "font", TITLEFONT), m_theme.getColor(domain, "font_color", CColor(0xFFFFFFFF)));
	// Coverflow Count
	m_numCFVersions = min(max(2, m_theme.getInt("_COVERFLOW", "number_of_modes", 2)), 15);
}

Vector3D CMenu::_getCFV3D(const string &domain, const string &key, const Vector3D &def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_theme.has(domain, key169))
	{
		Vector3D v(m_theme.getVector3D(domain, key169));
		m_theme.getVector3D(domain, key43, v);
		return v;
	}
	return m_theme.getVector3D(domain, key169, m_theme.getVector3D(domain, key43, def));
}

int CMenu::_getCFInt(const string &domain, const string &key, int def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_theme.has(domain, key169))
	{
		int v = m_theme.getInt(domain, key169);
		m_theme.getInt(domain, key43, v);
		return v;
	}
	return m_theme.getInt(domain, key169, m_theme.getInt(domain, key43, def));
}

float CMenu::_getCFFloat(const string &domain, const string &key, float def, bool otherScrnFmt)
{
	string key169(key);
	string key43(key);

	key43 += "_4_3";
	if (m_vid.wide() == otherScrnFmt)
		swap(key169, key43);
	if (m_theme.has(domain, key169))
	{
		float v = m_theme.getFloat(domain, key169);
		m_theme.getFloat(domain, key43, v);
		return v;
	}
	return m_theme.getFloat(domain, key169, m_theme.getFloat(domain, key43, def));
}

void CMenu::_loadCFLayout(int version, bool forceAA, bool otherScrnFmt)
{
	bool homebrew = m_current_view == COVERFLOW_HOMEBREW;
	bool smallbox = (homebrew || m_current_view == COVERFLOW_PLUGIN) && m_cfg.getBool(_domainFromView(), "smallbox", true);
	string domain(homebrew ? fmt("_BREWFLOW_%i", version) : m_current_view == COVERFLOW_PLUGIN ? fmt("_EMUFLOW_%i", version) : fmt("_COVERFLOW_%i", version));
	string domainSel(homebrew ? fmt("_BREWFLOW_%i_S", version) : m_current_view == COVERFLOW_PLUGIN ? fmt("_EMUFLOW_%i_S", version) : fmt("_COVERFLOW_%i_S", version));
	bool sf = otherScrnFmt;

	int max_fsaa = m_theme.getInt(domain, "max_fsaa", 3);
	_setAA(forceAA ? max_fsaa : min(max_fsaa, m_cfg.getInt("GENERAL", "max_fsaa", 3)));

	CoverFlow.setTextureQuality(m_theme.getFloat(domain, "tex_lod_bias", -3.f),
		m_theme.getInt(domain, "tex_aniso", 2),
		m_theme.getBool(domain, "tex_edge_lod", true));

	CoverFlow.setRange(_getCFInt(domain, "rows", (smallbox && homebrew) ? 5 : 1, sf), _getCFInt(domain, "columns", 9, sf));

	CoverFlow.setCameraPos(false,
		_getCFV3D(domain, "camera_pos", Vector3D(0.f, 1.5f, 5.f), sf),
		_getCFV3D(domain, "camera_aim", Vector3D(0.f, 0.f, -1.f), sf));

	CoverFlow.setCameraPos(true,
		_getCFV3D(domainSel, "camera_pos", Vector3D(0.f, 1.5f, 5.f), sf),
		_getCFV3D(domainSel, "camera_aim", Vector3D(0.f, 0.f, -1.f), sf));

	CoverFlow.setCameraOsc(false,
		_getCFV3D(domain, "camera_osc_speed", Vector3D(2.f, 1.1f, 1.3f), sf),
		_getCFV3D(domain, "camera_osc_amp", Vector3D(0.1f, 0.2f, 0.1f), sf));

	CoverFlow.setCameraOsc(true,
		_getCFV3D(domainSel, "camera_osc_speed", Vector3D(), sf),
		_getCFV3D(domainSel, "camera_osc_amp", Vector3D(), sf));

	float def_cvr_posX = (smallbox && homebrew) ? 1.f : 1.6f;
	float def_cvr_posY = (smallbox && homebrew) ? -0.6f : 0.f;
	CoverFlow.setCoverPos(false,
		_getCFV3D(domain, "left_pos", Vector3D(-def_cvr_posX, def_cvr_posY, 0.f), sf),
		_getCFV3D(domain, "right_pos", Vector3D(def_cvr_posX, def_cvr_posY, 0.f), sf),
		_getCFV3D(domain, "center_pos", Vector3D(0.f, def_cvr_posY, 1.f), sf),
		_getCFV3D(domain, "row_center_pos", Vector3D(0.f, def_cvr_posY, 0.f), sf));

	def_cvr_posX = (smallbox && homebrew) ? 1.f : 4.6f;
	float def_cvr_posX1 = (smallbox && homebrew) ? 0.f : -0.6f;
	CoverFlow.setCoverPos(true,
		_getCFV3D(domainSel, "left_pos", Vector3D(-def_cvr_posX, def_cvr_posY, 0.f), sf),
		_getCFV3D(domainSel, "right_pos", Vector3D(def_cvr_posX, def_cvr_posY, 0.f), sf),
		_getCFV3D(domainSel, "center_pos", Vector3D(def_cvr_posX1, 0.f, 2.6f), sf),
		_getCFV3D(domainSel, "row_center_pos", Vector3D(0.f, def_cvr_posY, 0.f), sf));

	CoverFlow.setCoverAngleOsc(false,
		m_theme.getVector3D(domain, "cover_osc_speed", Vector3D(2.f, 2.f, 0.f)),
		m_theme.getVector3D(domain, "cover_osc_amp", Vector3D(5.f, 10.f, 0.f)));

	CoverFlow.setCoverAngleOsc(true,
		m_theme.getVector3D(domainSel, "cover_osc_speed", Vector3D(2.1f, 2.1f, 0.f)),
		m_theme.getVector3D(domainSel, "cover_osc_amp", Vector3D(2.f, 5.f, 0.f)));

	CoverFlow.setCoverPosOsc(false,
		m_theme.getVector3D(domain, "cover_pos_osc_speed"),
		m_theme.getVector3D(domain, "cover_pos_osc_amp"));

	CoverFlow.setCoverPosOsc(true,
		m_theme.getVector3D(domainSel, "cover_pos_osc_speed"),
		m_theme.getVector3D(domainSel, "cover_pos_osc_amp"));

	float spacerX = (smallbox && homebrew) ? 1.f : 0.35f;
	CoverFlow.setSpacers(false,
		m_theme.getVector3D(domain, "left_spacer", Vector3D(-spacerX, 0.f, 0.f)),
		m_theme.getVector3D(domain, "right_spacer", Vector3D(spacerX, 0.f, 0.f)));

	CoverFlow.setSpacers(true,
		m_theme.getVector3D(domainSel, "left_spacer", Vector3D(-spacerX, 0.f, 0.f)),
		m_theme.getVector3D(domainSel, "right_spacer", Vector3D(spacerX, 0.f, 0.f)));

	CoverFlow.setDeltaAngles(false,
		m_theme.getVector3D(domain, "left_delta_angle"),
		m_theme.getVector3D(domain, "right_delta_angle"));

	CoverFlow.setDeltaAngles(true,
		m_theme.getVector3D(domainSel, "left_delta_angle"),
		m_theme.getVector3D(domainSel, "right_delta_angle"));

	float angleY = (smallbox && homebrew) ? 0.f : 70.f;
	CoverFlow.setAngles(false,
		m_theme.getVector3D(domain, "left_angle", Vector3D(0.f, angleY, 0.f)),
		m_theme.getVector3D(domain, "right_angle", Vector3D(0.f, -angleY, 0.f)),
		m_theme.getVector3D(domain, "center_angle"),
		m_theme.getVector3D(domain, "row_center_angle"));

	angleY = (smallbox && homebrew) ? 0.f : 90.f;
	float angleY1 = (smallbox && homebrew) ? 0.f : 380.f;
	float angleX = (smallbox && homebrew) ? 0.f : -45.f;
	CoverFlow.setAngles(true,
		m_theme.getVector3D(domainSel, "left_angle", Vector3D(angleX, angleY, 0.f)),
		m_theme.getVector3D(domainSel, "right_angle", Vector3D(angleX, -angleY, 0.f)),
		m_theme.getVector3D(domainSel, "center_angle", Vector3D(0.f, angleY1, 0.f)),
		m_theme.getVector3D(domainSel, "row_center_angle"));

	angleX = smallbox ? 0.f : 55.f;
	CoverFlow.setTitleAngles(false,
		_getCFFloat(domain, "text_left_angle", -angleX, sf),
		_getCFFloat(domain, "text_right_angle", angleX, sf),
		_getCFFloat(domain, "text_center_angle", 0.f, sf));

	CoverFlow.setTitleAngles(true,
		_getCFFloat(domainSel, "text_left_angle", -angleX, sf),
		_getCFFloat(domainSel, "text_right_angle", angleX, sf),
		_getCFFloat(domainSel, "text_center_angle", 0.f, sf));

	CoverFlow.setTitlePos(false,
		_getCFV3D(domain, "text_left_pos", Vector3D(-4.f, 0.f, 1.3f), sf),
		_getCFV3D(domain, "text_right_pos", Vector3D(4.f, 0.f, 1.3f), sf),
		_getCFV3D(domain, "text_center_pos", Vector3D(0.f, 0.f, 2.6f), sf));

	CoverFlow.setTitlePos(true,
		_getCFV3D(domainSel, "text_left_pos", Vector3D(-4.f, 0.f, 1.3f), sf),
		_getCFV3D(domainSel, "text_right_pos", Vector3D(4.f, 0.f, 1.3f), sf),
		_getCFV3D(domainSel, "text_center_pos", Vector3D(1.7f, 1.8f, 1.6f), sf));

	CoverFlow.setTitleWidth(false,
		_getCFFloat(domain, "text_side_wrap_width", 500.f, sf),
		_getCFFloat(domain, "text_center_wrap_width", 500.f, sf));

	CoverFlow.setTitleWidth(true,
		_getCFFloat(domainSel, "text_side_wrap_width", 500.f, sf),
		_getCFFloat(domainSel, "text_center_wrap_width", 310.f, sf));

	CoverFlow.setTitleStyle(false,
		_textStyle(domain.c_str(), "text_side_style", FTGX_ALIGN_MIDDLE | FTGX_JUSTIFY_CENTER),
		_textStyle(domain.c_str(), "text_center_style", FTGX_ALIGN_MIDDLE | FTGX_JUSTIFY_CENTER));

	CoverFlow.setTitleStyle(true,
		_textStyle(domainSel.c_str(), "text_side_style", FTGX_ALIGN_MIDDLE | FTGX_JUSTIFY_CENTER),
		_textStyle(domainSel.c_str(), "text_center_style", FTGX_ALIGN_TOP | FTGX_JUSTIFY_RIGHT));

	CoverFlow.setColors(false,
		m_theme.getColor(domain, "color_beg", 0xCFFFFFFF),
		m_theme.getColor(domain, "color_end", 0x3FFFFFFF),
		m_theme.getColor(domain, "color_off", 0x7FFFFFFF));

	CoverFlow.setColors(true,
		m_theme.getColor(domainSel, "color_beg", 0x7FFFFFFF),
		m_theme.getColor(domainSel, "color_end", 0x1FFFFFFF),
		m_theme.getColor(domain, "color_off", 0x7FFFFFFF));	// Mouse not used once a selection has been made

	CoverFlow.setMirrorAlpha(m_theme.getFloat(domain, "mirror_alpha", 0.25f), m_theme.getFloat(domain, "title_mirror_alpha", 0.2f));	// Doesn't depend on selection

	CoverFlow.setMirrorBlur(m_theme.getBool(domain, "mirror_blur", true));	// Doesn't depend on selection

	CoverFlow.setShadowColors(false,
		m_theme.getColor(domain, "color_shadow_center", 0x00000000),
		m_theme.getColor(domain, "color_shadow_beg", 0x00000000),
		m_theme.getColor(domain, "color_shadow_end", 0x00000000),
		m_theme.getColor(domain, "color_shadow_off", 0x00000000));

	CoverFlow.setShadowColors(true,
		m_theme.getColor(domainSel, "color_shadow_center", 0x0000007F),
		m_theme.getColor(domainSel, "color_shadow_beg", 0x0000007F),
		m_theme.getColor(domainSel, "color_shadow_end", 0x0000007F),
		m_theme.getColor(domainSel, "color_shadow_off", 0x0000007F));

	CoverFlow.setShadowPos(m_theme.getFloat(domain, "shadow_scale", 1.1f),
		m_theme.getFloat(domain, "shadow_x"),
		m_theme.getFloat(domain, "shadow_y"));

	float spacerY = (smallbox && homebrew) ? 0.60f : 2.f; 
	CoverFlow.setRowSpacers(false,
		m_theme.getVector3D(domain, "top_spacer", Vector3D(0.f, spacerY, 0.f)),
		m_theme.getVector3D(domain, "bottom_spacer", Vector3D(0.f, -spacerY, 0.f)));

	CoverFlow.setRowSpacers(true,
		m_theme.getVector3D(domainSel, "top_spacer", Vector3D(0.f, spacerY, 0.f)),
		m_theme.getVector3D(domainSel, "bottom_spacer", Vector3D(0.f, -spacerY, 0.f)));

	CoverFlow.setRowDeltaAngles(false,
		m_theme.getVector3D(domain, "top_delta_angle"),
		m_theme.getVector3D(domain, "bottom_delta_angle"));

	CoverFlow.setRowDeltaAngles(true,
		m_theme.getVector3D(domainSel, "top_delta_angle"),
		m_theme.getVector3D(domainSel, "bottom_delta_angle"));

	CoverFlow.setRowAngles(false,
		m_theme.getVector3D(domain, "top_angle"),
		m_theme.getVector3D(domain, "bottom_angle"));

	CoverFlow.setRowAngles(true,
		m_theme.getVector3D(domainSel, "top_angle"),
		m_theme.getVector3D(domainSel, "bottom_angle"));

	Vector3D def_cvr_scale = 
	smallbox 
		? (homebrew 
			? Vector3D(0.667f, 0.25f, 1.f)
		: Vector3D(1.f, 0.5f, 1.f))
	: Vector3D(1.f, 1.f, 1.f);

	CoverFlow.setCoverScale(false,
		m_theme.getVector3D(domain, "left_scale", def_cvr_scale),
		m_theme.getVector3D(domain, "right_scale", def_cvr_scale),
		m_theme.getVector3D(domain, "center_scale", def_cvr_scale),
		m_theme.getVector3D(domain, "row_center_scale", def_cvr_scale));

	CoverFlow.setCoverScale(true,
		m_theme.getVector3D(domainSel, "left_scale", def_cvr_scale),
		m_theme.getVector3D(domainSel, "right_scale", def_cvr_scale),
		m_theme.getVector3D(domainSel, "center_scale", def_cvr_scale),
		m_theme.getVector3D(domainSel, "row_center_scale", def_cvr_scale));

	float flipX = (smallbox && homebrew) ? 359.f : 180.f;
	CoverFlow.setCoverFlipping(
		_getCFV3D(domainSel, "flip_pos", Vector3D(), sf),
		_getCFV3D(domainSel, "flip_angle", Vector3D(0.f, flipX, 0.f), sf),
		_getCFV3D(domainSel, "flip_scale", def_cvr_scale, sf));

	CoverFlow.setBlur(
		m_theme.getInt(domain, "blur_resolution", 1),
		m_theme.getInt(domain, "blur_radius", 2),
		m_theme.getFloat(domain, "blur_factor", 1.f));
}

void CMenu::_buildMenus(void)
{
	// Default fonts
	theme.btnFont = _font(theme.fontSet, "GENERAL", "button_font", BUTTONFONT);
	theme.btnFontColor = m_theme.getColor("GENERAL", "button_font_color", 0xD0BFDFFF);

	theme.lblFont = _font(theme.fontSet, "GENERAL", "label_font", LABELFONT);
	theme.lblFontColor = m_theme.getColor("GENERAL", "label_font_color", 0xD0BFDFFF);

	theme.titleFont = _font(theme.fontSet, "GENERAL", "title_font", TITLEFONT);
	theme.titleFontColor = m_theme.getColor("GENERAL", "title_font_color", 0xFFFFFFFF);

	theme.txtFont = _font(theme.fontSet, "GENERAL", "text_font", TEXTFONT);
	theme.txtFontColor = m_theme.getColor("GENERAL", "text_font_color", 0xFFFFFFFF);

	theme.selsbtnFontColor = m_theme.getColor("GENERAL", "selsbtn_font_color", 0xFA5882FF);
	theme.selubtnFontColor = m_theme.getColor("GENERAL", "selubtn_font_color", 0xD0BFDFFF);

	// Default Sounds
	theme.clickSound	= _sound(theme.soundSet, "GENERAL", "click_sound", click_wav, click_wav_size, "default_btn_click", false);
	theme.hoverSound	= _sound(theme.soundSet, "GENERAL", "hover_sound", hover_wav, hover_wav_size, "default_btn_hover", false);
	theme.cameraSound	= _sound(theme.soundSet, "GENERAL", "camera_sound", camera_wav, camera_wav_size, "default_camera", false);

	// Default textures
	theme.btnTexL.fromPNG(butleft_png);
	theme.btnTexL = _texture("GENERAL", "button_texture_left", theme.btnTexL); 
	theme.btnTexR.fromPNG(butright_png);
	theme.btnTexR = _texture("GENERAL", "button_texture_right", theme.btnTexR); 
	theme.btnTexC.fromPNG(butcenter_png);
	theme.btnTexC = _texture("GENERAL", "button_texture_center", theme.btnTexC); 
	theme.btnTexLS.fromPNG(butsleft_png);
	theme.btnTexLS = _texture("GENERAL", "button_texture_left_selected", theme.btnTexLS); 
	theme.btnTexRS.fromPNG(butsright_png);
	theme.btnTexRS = _texture("GENERAL", "button_texture_right_selected", theme.btnTexRS); 
	theme.btnTexCS.fromPNG(butscenter_png);
	theme.btnTexCS = _texture("GENERAL", "button_texture_center_selected", theme.btnTexCS); 

	theme.btnTexLH.fromPNG(buthleft_png);
	theme.btnTexLH = _texture("GENERAL", "button_texture_hlleft", theme.btnTexLH); 
	theme.btnTexRH.fromPNG(buthright_png);
	theme.btnTexRH = _texture("GENERAL", "button_texture_hlright", theme.btnTexRH); 
	theme.btnTexCH.fromPNG(buthcenter_png);
	theme.btnTexCH = _texture("GENERAL", "button_texture_hlcenter", theme.btnTexCH); 
	theme.btnTexLSH.fromPNG(buthsleft_png);
	theme.btnTexLSH = _texture("GENERAL", "button_texture_hlleft_selected", theme.btnTexLSH); 
	theme.btnTexRSH.fromPNG(buthsright_png);
	theme.btnTexRSH = _texture("GENERAL", "button_texture_hlright_selected", theme.btnTexRSH); 
	theme.btnTexCSH.fromPNG(buthscenter_png);
	theme.btnTexCSH = _texture("GENERAL", "button_texture_hlcenter_selected", theme.btnTexCSH); 

	theme.btnAUOn.fromPNG(butauon_png);
	theme.btnAUOn = _texture("GENERAL", "button_au_on", theme.btnAUOn);
	theme.btnAUOns.fromPNG(butauons_png);
	theme.btnAUOns = _texture("GENERAL", "button_au_on_selected", theme.btnAUOns);
	theme.btnAUOff.fromPNG(butauoff_png);
	theme.btnAUOff = _texture("GENERAL", "button_au_off", theme.btnAUOff);
	theme.btnAUOffs.fromPNG(butauoffs_png);
	theme.btnAUOffs = _texture("GENERAL", "button_au_off_selected", theme.btnAUOffs);

	theme.btnENOn.fromPNG(butenon_png);
	theme.btnENOn = _texture("GENERAL", "button_en_on", theme.btnENOn);
	theme.btnENOns.fromPNG(butenons_png);
	theme.btnENOns = _texture("GENERAL", "button_en_on_selected", theme.btnENOns);
	theme.btnENOff.fromPNG(butenoff_png);
	theme.btnENOff = _texture("GENERAL", "button_en_off", theme.btnENOff);
	theme.btnENOffs.fromPNG(butenoffs_png);
	theme.btnENOffs = _texture("GENERAL", "button_en_off_selected", theme.btnENOffs);

	theme.btnJAOn.fromPNG(butjaon_png);
	theme.btnJAOn = _texture("GENERAL", "button_ja_on", theme.btnJAOn);
	theme.btnJAOns.fromPNG(butjaons_png);
	theme.btnJAOns = _texture("GENERAL", "button_ja_on_selected", theme.btnJAOns);
	theme.btnJAOff.fromPNG(butjaoff_png);
	theme.btnJAOff = _texture("GENERAL", "button_ja_off", theme.btnJAOff);
	theme.btnJAOffs.fromPNG(butjaoffs_png);
	theme.btnJAOffs = _texture("GENERAL", "button_ja_off_selected", theme.btnJAOffs);

	theme.btnFROn.fromPNG(butfron_png);
	theme.btnFROn = _texture("GENERAL", "button_fr_on", theme.btnFROn);
	theme.btnFROns.fromPNG(butfrons_png);
	theme.btnFROns = _texture("GENERAL", "button_fr_on_selected", theme.btnFROns);
	theme.btnFROff.fromPNG(butfroff_png);
	theme.btnFROff = _texture("GENERAL", "button_fr_off", theme.btnFROff);
	theme.btnFROffs.fromPNG(butfroffs_png);
	theme.btnFROffs = _texture("GENERAL", "button_fr_off_selected", theme.btnFROffs);

	theme.btnDEOn.fromPNG(butdeon_png);
	theme.btnDEOn = _texture("GENERAL", "button_de_on", theme.btnDEOn);
	theme.btnDEOns.fromPNG(butdeons_png);
	theme.btnDEOns = _texture("GENERAL", "button_de_on_selected", theme.btnDEOns);
	theme.btnDEOff.fromPNG(butdeoff_png);
	theme.btnDEOff = _texture("GENERAL", "button_de_off", theme.btnDEOff);
	theme.btnDEOffs.fromPNG(butdeoffs_png);
	theme.btnDEOffs = _texture("GENERAL", "button_de_off_selected", theme.btnDEOffs);

	theme.btnESOn.fromPNG(buteson_png);
	theme.btnESOn = _texture("GENERAL", "button_es_on", theme.btnESOn);
	theme.btnESOns.fromPNG(butesons_png);
	theme.btnESOns = _texture("GENERAL", "button_es_on_selected", theme.btnESOns);
	theme.btnESOff.fromPNG(butesoff_png);
	theme.btnESOff = _texture("GENERAL", "button_es_off", theme.btnESOff);
	theme.btnESOffs.fromPNG(butesoffs_png);
	theme.btnESOffs = _texture("GENERAL", "button_es_off_selected", theme.btnESOffs);

	theme.btnITOn.fromPNG(butiton_png);
	theme.btnITOn = _texture("GENERAL", "button_it_on", theme.btnITOn);
	theme.btnITOns.fromPNG(butitons_png);
	theme.btnITOns = _texture("GENERAL", "button_it_on_selected", theme.btnITOns);
	theme.btnITOff.fromPNG(butitoff_png);
	theme.btnITOff = _texture("GENERAL", "button_it_off", theme.btnITOff);
	theme.btnITOffs.fromPNG(butitoffs_png);
	theme.btnITOffs = _texture("GENERAL", "button_it_off_selected", theme.btnITOffs);

	theme.btnNLOn.fromPNG(butnlon_png);
	theme.btnNLOn = _texture("GENERAL", "button_nl_on", theme.btnNLOn);
	theme.btnNLOns.fromPNG(butnlons_png);
	theme.btnNLOns = _texture("GENERAL", "button_nl_on_selected", theme.btnNLOns);
	theme.btnNLOff.fromPNG(butnloff_png);
	theme.btnNLOff = _texture("GENERAL", "button_nl_off", theme.btnNLOff);
	theme.btnNLOffs.fromPNG(butnloffs_png);
	theme.btnNLOffs = _texture("GENERAL", "button_nl_off_selected", theme.btnNLOffs);

	theme.btnPTOn.fromPNG(butpton_png);
	theme.btnPTOn = _texture("GENERAL", "button_pt_on", theme.btnPTOn);
	theme.btnPTOns.fromPNG(butptons_png);
	theme.btnPTOns = _texture("GENERAL", "button_pt_on_selected", theme.btnPTOns);
	theme.btnPTOff.fromPNG(butptoff_png);
	theme.btnPTOff = _texture("GENERAL", "button_pt_off", theme.btnPTOff);
	theme.btnPTOffs.fromPNG(butptoffs_png);
	theme.btnPTOffs = _texture("GENERAL", "button_pt_off_selected", theme.btnPTOffs);

	theme.btnRUOn.fromPNG(butruon_png);
	theme.btnRUOn = _texture("GENERAL", "button_ru_on", theme.btnRUOn);
	theme.btnRUOns.fromPNG(butruons_png);
	theme.btnRUOns = _texture("GENERAL", "button_ru_on_selected", theme.btnRUOns);
	theme.btnRUOff.fromPNG(butruoff_png);
	theme.btnRUOff = _texture("GENERAL", "button_ru_off", theme.btnRUOff);
	theme.btnRUOffs.fromPNG(butruoffs_png);
	theme.btnRUOffs = _texture("GENERAL", "button_ru_off_selected", theme.btnRUOffs);

	theme.btnKOOn.fromPNG(butkoon_png);
	theme.btnKOOn = _texture("GENERAL", "button_ko_on", theme.btnKOOn);
	theme.btnKOOns.fromPNG(butkoons_png);
	theme.btnKOOns = _texture("GENERAL", "button_ko_on_selected", theme.btnKOOns);
	theme.btnKOOff.fromPNG(butkooff_png);
	theme.btnKOOff = _texture("GENERAL", "button_ko_off", theme.btnKOOff);
	theme.btnKOOffs.fromPNG(butkooffs_png);
	theme.btnKOOffs = _texture("GENERAL", "button_ko_off_selected", theme.btnKOOffs);

	theme.btnZHCNOn.fromPNG(butzhcnon_png);
	theme.btnZHCNOn = _texture("GENERAL", "button_zhcn_on", theme.btnZHCNOn);
	theme.btnZHCNOns.fromPNG(butzhcnons_png);
	theme.btnZHCNOns = _texture("GENERAL", "button_zhcn_on_selected", theme.btnZHCNOns);
	theme.btnZHCNOff.fromPNG(butzhcnoff_png);
	theme.btnZHCNOff = _texture("GENERAL", "button_zhcn_off", theme.btnZHCNOff);
	theme.btnZHCNOffs.fromPNG(butzhcnoffs_png);
	theme.btnZHCNOffs = _texture("GENERAL", "button_zhcn_off_selected", theme.btnZHCNOffs);

	theme.checkboxoff.fromPNG(checkbox_png);
	theme.checkboxoff = _texture("GENERAL", "checkbox_off", theme.checkboxoff);
	theme.checkboxoffs.fromPNG(checkbox_png);
	theme.checkboxoffs = _texture("GENERAL", "checkbox_off_selected", theme.checkboxoffs);
	theme.checkboxon.fromPNG(checkboxs_png);
	theme.checkboxon = _texture("GENERAL", "checkbox_on", theme.checkboxon);
	theme.checkboxons.fromPNG(checkboxs_png);
	theme.checkboxons = _texture("GENERAL", "checkbox_on_selected", theme.checkboxons);
	theme.checkboxHid.fromPNG(checkboxhid_png);
	theme.checkboxHid = _texture("GENERAL", "checkbox_Hid", theme.checkboxHid);
	theme.checkboxHids.fromPNG(checkboxhid_png);
	theme.checkboxHids = _texture("GENERAL", "checkbox_Hid_selected", theme.checkboxHids);
	theme.checkboxReq.fromPNG(checkboxreq_png);
	theme.checkboxReq = _texture("GENERAL", "checkbox_Req", theme.checkboxReq);
	theme.checkboxReqs.fromPNG(checkboxreq_png);
	theme.checkboxReqs = _texture("GENERAL", "checkbox_Req_selected", theme.checkboxReqs);

	theme.pbarTexL.fromPNG(pbarleft_png);
	theme.pbarTexL = _texture("GENERAL", "progressbar_texture_left", theme.pbarTexL);
	theme.pbarTexR.fromPNG(pbarright_png);
	theme.pbarTexR = _texture("GENERAL", "progressbar_texture_right", theme.pbarTexR);
	theme.pbarTexC.fromPNG(pbarcenter_png);
	theme.pbarTexC = _texture("GENERAL", "progressbar_texture_center", theme.pbarTexC);
	theme.pbarTexLS.fromPNG(pbarlefts_png);
	theme.pbarTexLS = _texture("GENERAL", "progressbar_texture_left_selected", theme.pbarTexLS);
	theme.pbarTexRS.fromPNG(pbarrights_png);
	theme.pbarTexRS = _texture("GENERAL", "progressbar_texture_right_selected", theme.pbarTexRS);
	theme.pbarTexCS.fromPNG(pbarcenters_png);
	theme.pbarTexCS = _texture("GENERAL", "progressbar_texture_center_selected", theme.pbarTexCS);
	theme.btnTexPlus.fromPNG(btnplus_png);
	theme.btnTexPlus = _texture("GENERAL", "plus_button_texture", theme.btnTexPlus);
	theme.btnTexPlusS.fromPNG(btnpluss_png);
	theme.btnTexPlusS = _texture("GENERAL", "plus_button_texture_selected", theme.btnTexPlusS);
	theme.btnTexMinus.fromPNG(btnminus_png);
	theme.btnTexMinus = _texture("GENERAL", "minus_button_texture", theme.btnTexMinus);
	theme.btnTexMinusS.fromPNG(btnminuss_png);
	theme.btnTexMinusS = _texture("GENERAL", "minus_button_texture_selected", theme.btnTexMinusS);

	// Default background
	theme.bg.fromJPG(background_jpg, background_jpg_size);
	m_mainBgLQ.fromJPG(background_jpg, background_jpg_size, GX_TF_CMPR, 64, 64);
	m_gameBgLQ = m_mainBgLQ;

	// Build menus
	_initMainMenu();
	_initErrorMenu();
	_initConfigAdvMenu();
	_initConfigSndMenu();
	_initConfig4Menu();
	_initConfigScreenMenu();
	_initConfig3Menu();
	_initConfigMenu();
	_initGameMenu();
	_initDownloadMenu();
	_initCodeMenu();
	_initAboutMenu();
	_initWBFSMenu();
	_initCFThemeMenu();
	_initGameSettingsMenu();
	_initCheatSettingsMenu(); 
	_initSourceMenu();
	_initPluginSettingsMenu();
	_initCategorySettingsMenu();
	_initSystemMenu();
	_initGameInfoMenu();
	_initNandEmuMenu();
	_initHomeAndExitToMenu();

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

SFont CMenu::_font(CMenu::FontSet &fontSet, const char *domain, const char *key, u32 fontSize, u32 lineSpacing, u32 weight, u32 index, const char *genKey)
{
	string filename = "";
	bool general = strncmp(domain, "GENERAL", 7) == 0;
	FontHolder fonts[3] = {{ "_size", 6u, 300u, fontSize, 0 }, { "_line_height", 6u, 300u, lineSpacing, 0 }, { "_weight", 1u, 32u, weight, 0 }};

	if(!general)
		filename = m_theme.getString(domain, key);
	if(filename.empty())
		filename = m_theme.getString("GENERAL", genKey, genKey);
	bool useDefault = filename == genKey;

	for(u32 i = 0; i < 3; i++)
	{
		string defValue = genKey;
		defValue += fonts[i].ext;
		string value = key;
		value += fonts[i].ext;

		if(!general)
			fonts[i].res = (u32)m_theme.getInt(domain, value);
		if(fonts[i].res <= 0)
			fonts[i].res = (u32)m_theme.getInt("GENERAL", defValue);

		fonts[i].res = min(max(fonts[i].min, fonts[i].res <= 0 ? fonts[i].def : fonts[i].res), fonts[i].max);		
	}

	// Try to find the same font with the same size
	CMenu::FontSet::iterator i = fontSet.find(CMenu::FontDesc(upperCase(filename.c_str()), fonts[0].res));
	if (i != fontSet.end()) return i->second;

	// TTF not found in memory, load it to create a new font
	SFont retFont;
 	if (!useDefault && retFont.fromFile(fmt("%s/%s", m_themeDataDir.c_str(), filename.c_str()), fonts[0].res, fonts[1].res, fonts[2].res, index))
	{
		// Theme Font
		fontSet[CMenu::FontDesc(upperCase(filename.c_str()), fonts[0].res)] = retFont;
		return retFont;
	}
	/* Fallback to default font */
	if(retFont.fromBuffer(m_base_font, m_base_font_size, fonts[0].res, fonts[1].res, fonts[2].res, index))
	{
		// Default font
		fontSet[CMenu::FontDesc(upperCase(filename.c_str()), fonts[0].res)] = retFont;
		return retFont;
	}
	return retFont;
}

vector<STexture> CMenu::_textures(const char *domain, const char *key)
{
	vector<STexture> textures;

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
				STexture tex;
				if(tex.fromImageFile(fmt("%s/%s", m_themeDataDir.c_str(), filename.c_str())) == TE_OK)
				{
					theme.texSet[filename] = tex;
					textures.push_back(tex);
				}
			}
		}
	}
	return textures;
}

STexture CMenu::_texture(const char *domain, const char *key, STexture &def, bool freeDef)
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
			STexture tex;
			if(tex.fromImageFile(fmt("%s/%s", m_themeDataDir.c_str(), filename.c_str())) == TE_OK)
			{
				if(freeDef && def.data != NULL)
				{
					free(def.data);
					def.data = NULL;
				}
				theme.texSet[filename] = tex;
				return tex;
			}
		}
	}
	/* Fallback to default */
	theme.texSet[filename] = def;
	return def;
}

// Only for loading defaults and GENERAL domains!!
GuiSound *CMenu::_sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, const u8 * snd, u32 len, const char *name, bool isAllocated)
{
	const char *filename = m_theme.getString(domain, key, "").c_str();
	if(filename == NULL || filename[0] == '\0')
		filename = name;

	CMenu::SoundSet::iterator i = soundSet.find(upperCase(name));
	if(i == soundSet.end())
	{
		if(filename != name)
			soundSet[upperCase(filename)] = new GuiSound(fmt("%s/%s", m_themeDataDir.c_str(), filename));
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
		soundSet[upperCase(filename)] = new GuiSound(fmt("%s/%s", m_themeDataDir.c_str(), filename));
		return soundSet[upperCase(filename)];
	}
	return i->second;
}

u16 CMenu::_textStyle(const char *domain, const char *key, u16 def)
{
	u16 textStyle = 0;
	string style(m_theme.getString(domain, key));
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

	font = _font(theme.fontSet, domain, "font", BUTTONFONT);

	GuiSound *clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound->GetName());
	GuiSound *hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound->GetName());

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addButton(font, text, x, y, width, height, c, btnTexSet, clickSound, hoverSound);
}

s16 CMenu::_addSelButton(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color)
{
	SButtonTextureSet btnTexSet;
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	btnTexSet.left = _texture(domain, "texture_left", theme.btnTexLH, false);
	btnTexSet.right = _texture(domain, "texture_right", theme.btnTexRH, false);
	btnTexSet.center = _texture(domain, "texture_center", theme.btnTexCH, false);
	btnTexSet.leftSel = _texture(domain, "texture_left_selected", theme.btnTexLSH, false);
	btnTexSet.rightSel = _texture(domain, "texture_right_selected", theme.btnTexRSH, false);
	btnTexSet.centerSel = _texture(domain, "texture_center_selected", theme.btnTexCSH, false);

	font = _font(theme.fontSet, domain, "font", BUTTONFONT);

	GuiSound *clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound->GetName());
	GuiSound *hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound->GetName());

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addButton(font, text, x, y, width, height, c, btnTexSet, clickSound, hoverSound);
}

s16 CMenu::_addPicButton(const char *domain, STexture &texNormal, STexture &texSelected, int x, int y, u32 width, u32 height)
{
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	STexture tex1 = _texture(domain, "texture_normal", texNormal, false);
	STexture tex2 = _texture(domain, "texture_selected", texSelected, false);
	GuiSound *clickSound = _sound(theme.soundSet, domain, "click_sound", theme.clickSound->GetName());
	GuiSound *hoverSound = _sound(theme.soundSet, domain, "hover_sound", theme.hoverSound->GetName());

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addPicButton(tex1, tex2, x, y, width, height, clickSound, hoverSound);
}

s16 CMenu::_addTitle(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style)
{
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(theme.fontSet, domain, "font", TITLEFONT);
	style = _textStyle(domain, "style", style);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style);
}

s16 CMenu::_addText(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style)
{
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(theme.fontSet, domain, "font", TEXTFONT);
	style = _textStyle(domain, "style", style);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style);
}

s16 CMenu::_addLabel(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style)
{
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(theme.fontSet, domain, "font", LABELFONT);
	style = _textStyle(domain, "style", style);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addLabel(font, text, x, y, width, height, c, style);
}

s16 CMenu::_addLabel(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style, STexture &bg)
{
	CColor c(color);

	c = m_theme.getColor(domain, "color", c);
	x = m_theme.getInt(domain, "x", x);
	y = m_theme.getInt(domain, "y", y);
	width = m_theme.getInt(domain, "width", width);
	height = m_theme.getInt(domain, "height", height);
	font = _font(theme.fontSet, domain, "font", BUTTONFONT);
	STexture texBg = _texture(domain, "background_texture", bg, false);
	style = _textStyle(domain, "style", style);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

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

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
		x = m_vid.width() - x - width;
	if (btnPos & FTGX_ALIGN_BOTTOM)
		y = m_vid.height() - y - height;

	return m_btnMgr.addProgressBar(x, y, width, height, btnTexSet);
}

void CMenu::_setHideAnim(s16 id, const char *domain, int dx, int dy, float scaleX, float scaleY)
{
	dx = m_theme.getInt(domain, "effect_x", dx);
	dy = m_theme.getInt(domain, "effect_y", dy);
	scaleX = m_theme.getFloat(domain, "effect_scale_x", scaleX);
	scaleY = m_theme.getFloat(domain, "effect_scale_y", scaleY);

	int x, y;
	u32 width, height;
	m_btnMgr.getDimensions(id, x, y, width, height);

	u16 btnPos = _textStyle(domain, "elmstyle", FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	if (btnPos & FTGX_JUSTIFY_RIGHT)
	{
		dx = m_vid.width() - dx - width;
		scaleX = m_vid.width() - scaleX - width;
	}
	if (btnPos & FTGX_ALIGN_BOTTOM)
	{
		dy = m_vid.height() - dy - height;
		scaleY = m_vid.height() - scaleY - height;
	}

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
		string dom(sfmt("%s/USER%i", domain, i + 1));
		if (m_theme.hasDomain(dom))
		{
			STexture emptyTex;
			ids[i] = _addLabel(dom.c_str(), theme.lblFont, L"", 40, 200, 64, 64, CColor(0xFFFFFFFF), 0, emptyTex);
			_setHideAnim(ids[i], dom.c_str(), -50, 0, 0.f, 0.f);
		}
		else
			ids[i] = -1;
	}
}

void CMenu::_initCF(void)
{
	Config dump, gameAgeList;
	GameTDB gametdb;
	const char *domain = _domainFromView();
	
	CoverFlow.clear();
	CoverFlow.reserve(m_gameList.size());
	
	const vector<bool> &EnabledPlugins = m_plugin.GetEnabledPlugins(m_cfg);

 	bool dumpGameLst = m_cfg.getBool(domain, "dump_list", true);
	if(dumpGameLst) dump.load(fmt("%s/" TITLES_DUMP_FILENAME, m_settingsDir.c_str()));

	m_gcfg1.load(fmt("%s/" GAME_SETTINGS1_FILENAME, m_settingsDir.c_str()));
	
	int ageLock = m_cfg.getInt("GENERAL", "age_lock");
	if (ageLock < 2 || ageLock > 19)
		ageLock = 19;
	if (ageLock < 19)
	{
		gameAgeList.load(fmt("%s/" AGE_LOCK_FILENAME, m_settingsDir.c_str()));
		if (m_current_view == COVERFLOW_USB || m_current_view == COVERFLOW_CHANNEL)
		{
			gametdb.OpenFile(sfmt("%s/wiitdb.xml", m_settingsDir.c_str()).c_str());
			gametdb.SetLanguageCode(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
		}
	}
	// check for single plugin selected
	u8 pos = 0;
	u8 enabledPluginsCount = 0;
	if(m_current_view == COVERFLOW_PLUGIN && EnabledPlugins.size() != 0)
	{
		char PluginMagicWord[9];
		for(u8 i = 0; i < EnabledPlugins.size(); i++)
		{
			snprintf(PluginMagicWord, sizeof(PluginMagicWord), "%08x", m_plugin.getPluginMagic(i));
			if(m_cfg.getBool("PLUGIN", PluginMagicWord, true))
			{
				pos = i;
				enabledPluginsCount++;
			}
		}
	}
	for(vector<dir_discHdr>::iterator element = m_gameList.begin(); element != m_gameList.end(); ++element)
	{
		string id;
		string tempname = element->path;
		u64 chantitle = TITLE_ID(element->settings[0],element->settings[1]);
		if(element->type == TYPE_HOMEBREW)
		{
			tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
			id = tempname;
		}
		else if(element->type == TYPE_PLUGIN)
		{
			if(tempname.find(':') != string::npos)
			{
				if(tempname.empty() || tempname.find_first_of('/') == string::npos)
					continue;
				tempname.erase(0, tempname.find_first_of('/')+1);
				string dirName = tempname.substr(0, tempname.find_first_of('/')+1);
				if (tempname.find_first_of('/') == string::npos)
				{
					continue;
				}
				tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
				if(tempname.find_last_of('.') == string::npos)
				{
					continue;
				}
				tempname.erase(tempname.find_last_of('.'), tempname.size() - tempname.find_last_of('.'));
				id = dirName+tempname;
			}
			else
				id = tempname;
		}
		else
		{
			if(element->type == TYPE_CHANNEL && chantitle == HBC_108)
				strncpy(element->id, "JODI", 6);
			id = element->id;
		}
		bool ageLocked = false;
		if (ageLock < 19)
		{
			int ageRated = min(max(gameAgeList.getInt(domain, id), 0), 19);
			if(ageRated == 0 && gametdb.IsLoaded() && (element->type == TYPE_WII_GAME || element->type == TYPE_CHANNEL))
			{
				const char *RatingValue = NULL;
				if(gametdb.GetRatingValue(id.c_str(), RatingValue))
				{
					switch(gametdb.GetRating(id.c_str()))
					{
						case GAMETDB_RATING_TYPE_CERO:
							if(RatingValue[0] == 'A')
								ageRated = 3;
							else if(RatingValue[0] == 'B')
								ageRated = 12;
							else if(RatingValue[0] == 'D')
								ageRated = 15;
							else if(RatingValue[0] == 'C')
								ageRated = 17;
							else if(RatingValue[0] == 'Z')
								ageRated = 18;
							break;
						case GAMETDB_RATING_TYPE_ESRB:
							if(RatingValue[0] == 'E')
								ageRated = 6;
							else if(memcmp(RatingValue, "EC", 2) == 0)
								ageRated = 3;
							else if(memcmp(RatingValue, "E10+", 4) == 0)
								ageRated = 10;
							else if(RatingValue[0] == 'T')
								ageRated = 13;
							else if(RatingValue[0] == 'M')
								ageRated = 17;
							else if(memcmp(RatingValue, "AO", 2) == 0)
								ageRated = 18;
							break;
						case GAMETDB_RATING_TYPE_PEGI:
							if(RatingValue[0] == '3')
								ageRated = 3;
							else if(RatingValue[0] == '7')
								ageRated = 7;
							else if(memcmp(RatingValue, "12", 2) == 0)
								ageRated = 12;
							else if(memcmp(RatingValue, "16", 2) == 0)
								ageRated = 16;
							else if(memcmp(RatingValue, "18", 2) == 0)
								ageRated = 18;
							break;
						case GAMETDB_RATING_TYPE_GRB:
							if(RatingValue[0] == 'A')
								ageRated = 3;
							else if(memcmp(RatingValue, "12", 2) == 0)
								ageRated = 12;
							else if(memcmp(RatingValue, "15", 2) == 0)
								ageRated = 15;
							else if(memcmp(RatingValue, "18", 2) == 0)
								ageRated = 18;
							break;
						default:
							break;
					}
				}
			}
			if(ageRated == 0)
				ageRated = min(max(m_cfg.getInt("GENERAL", "age_lock_default", AGE_LOCK_DEFAULT), 2), 19);
			if(ageRated == 0)
				ageRated = AGE_LOCK_DEFAULT;
			if(ageRated > ageLock)
				ageLocked = true;
		}
		if((!m_favorites || m_gcfg1.getBool("FAVORITES", id, false))
			&& (!m_locked || !m_gcfg1.getBool("ADULTONLY", id, false))
			&& !ageLocked)
		{
			string catDomain;
			switch(element->type)
			{
				case TYPE_CHANNEL:
					catDomain = CHANNEL_DOMAIN;
					break;
				case TYPE_HOMEBREW:
					catDomain = HOMEBREW_DOMAIN;
					break;
				case TYPE_GC_GAME:
					catDomain = GC_DOMAIN;
					break;
				case TYPE_PLUGIN:
					catDomain = PLUGIN_DOMAIN;
					break;
				default:
					catDomain = WII_DOMAIN;
			}
			if(enabledPluginsCount == 1)
			{
				catDomain = (m_plugin.GetPluginName(pos)).toUTF8();
				if(element->settings[0] != m_plugin.getPluginMagic(pos))
					continue;
			}
			const char *requiredCats = m_cat.getString(fmt("%s/GENERAL", catDomain.c_str()), "required_categories").c_str();
			const char *selectedCats = m_cat.getString(fmt("%s/GENERAL", catDomain.c_str()), "selected_categories").c_str();
			const char *hiddenCats = m_cat.getString(fmt("%s/GENERAL", catDomain.c_str()), "hidden_categories").c_str();
			u8 numReqCats = strlen(requiredCats);
			u8 numSelCats = strlen(selectedCats);
			u8 numHidCats = strlen(hiddenCats);
			if(numReqCats != 0 || numSelCats != 0 || numHidCats != 0) // if all 0 skip checking cats and show all games
			{
				const char *idCats = m_cat.getString(catDomain, id).c_str();
				u8 numIdCats = strlen(idCats);
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
			int playcount = m_gcfg1.getInt("PLAYCOUNT", id, 0);
			unsigned int lastPlayed = m_gcfg1.getUInt("LASTPLAYED", id, 0);

			if(dumpGameLst)
				dump.setWString(domain, id, element->title);

			const char *blankCoverKey = NULL;
			switch(element->type)
			{
				case TYPE_CHANNEL:
					blankCoverKey = "channels";
					break;
				case TYPE_HOMEBREW:
					blankCoverKey = "homebrew";
					break;
				case TYPE_GC_GAME:
					blankCoverKey = "gamecube";
					break;
				case TYPE_PLUGIN:
					char PluginMagicWord[9];
					memset(PluginMagicWord, 0, sizeof(PluginMagicWord));
					strncpy(PluginMagicWord, fmt("%08x", element->settings[0]), 8);
					blankCoverKey = PluginMagicWord;
					break;
				default:
					blankCoverKey = "wii";
			}
			const string &blankCoverName = m_theme.getString("BLANK_COVERS", blankCoverKey, fmt("%s.jpg", blankCoverKey));
			if(element->type == TYPE_PLUGIN)
			{
				string tempname(element->path);
				if(tempname.find_last_of("/") != string::npos)
					tempname.assign(&tempname[tempname.find_last_of("/") + 1]);
				string coverFolder(m_plugin.GetCoverFolderName(element->settings[0]));
				if(EnabledPlugins.size() == 0) //all plugins
				{
					if(coverFolder.size() > 0)
						CoverFlow.addItem(&(*element), fmt("%s/%s/%s.png", m_picDir.c_str(), coverFolder.c_str(), tempname.c_str()), fmt("%s/%s/%s.png", m_boxPicDir.c_str(), coverFolder.c_str(), tempname.c_str()), fmt("%s/%s", m_boxPicDir.c_str(), blankCoverName.c_str()), playcount, lastPlayed);
					else
						CoverFlow.addItem(&(*element), fmt("%s/%s.png", m_picDir.c_str(), tempname.c_str()), fmt("%s/%s.png", m_boxPicDir.c_str(), tempname.c_str()), fmt("%s/%s", m_boxPicDir.c_str(), blankCoverName.c_str()), playcount, lastPlayed);
				}
				else
				{
					for(u8 j = 0; j < EnabledPlugins.size(); j++)
					{
						if(EnabledPlugins.at(j) == true && element->settings[0] == m_plugin.getPluginMagic(j))
						{
							if(coverFolder.size() > 0)
								CoverFlow.addItem(&(*element), fmt("%s/%s/%s.png", m_picDir.c_str(), coverFolder.c_str(), tempname.c_str()), fmt("%s/%s/%s.png", m_boxPicDir.c_str(), coverFolder.c_str(), tempname.c_str()), fmt("%s/%s", m_boxPicDir.c_str(), blankCoverName.c_str()), playcount, lastPlayed);
							else
								CoverFlow.addItem(&(*element), fmt("%s/%s.png", m_picDir.c_str(), tempname.c_str()), fmt("%s/%s.png", m_boxPicDir.c_str(), tempname.c_str()), fmt("%s/%s", m_boxPicDir.c_str(), blankCoverName.c_str()), playcount, lastPlayed);
							break;
						}
					}
				}
			}
			else if(element->type == TYPE_HOMEBREW)
				CoverFlow.addItem(&(*element), fmt("%s/icon.png", element->path), fmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str()), fmt("%s/%s", m_boxPicDir.c_str(), blankCoverName.c_str()), playcount, lastPlayed);
			else
				CoverFlow.addItem(&(*element), fmt("%s/%s.png", m_picDir.c_str(), id.c_str()), fmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str()), fmt("%s/%s", m_boxPicDir.c_str(), blankCoverName.c_str()), playcount, lastPlayed);
		}
	}
	m_gcfg1.unload();
 	if (dumpGameLst)
	{
		dump.save(true);
		m_cfg.setBool(domain, "dump_list", false);
	}
	CoverFlow.setBoxMode(m_cfg.getBool("GENERAL", "box_mode", true));
	CoverFlow.setCompression(m_cfg.getBool("GENERAL", "allow_texture_compression", true));
	CoverFlow.setBufferSize(m_cfg.getInt("GENERAL", "cover_buffer", 20));
	CoverFlow.setSorting((Sorting)m_cfg.getInt(domain, "sort", 0));
	CoverFlow.setHQcover(m_cfg.getBool("GENERAL", "cover_use_hq", false));

	CoverFlow.start();
	if (m_curGameId.empty() || !CoverFlow.findId(m_curGameId.c_str(), true))
		CoverFlow.findId(m_cfg.getString(domain, "current_item").c_str(), true);
}

void CMenu::_mainLoopCommon(bool withCF, bool adjusting)
{
	if(withCF)
		CoverFlow.tick();
	m_btnMgr.tick();
	m_fa.tick();
	m_fa.hideCover() ? 	CoverFlow.hideCover() : CoverFlow.showCover();
	CoverFlow.setFanartPlaying(m_fa.isLoaded());
	CoverFlow.setFanartTextColor(m_fa.getTextColor(m_theme.getColor("_COVERFLOW", "font_color", CColor(0xFFFFFFFF))));

	m_vid.prepare();
	m_vid.setup2DProjection();
	_updateBg();
	if(CoverFlow.getRenderTex())
		CoverFlow.RenderTex();
	if(withCF)
		CoverFlow.makeEffectTexture(m_lqBg);
	if(withCF && m_aa > 0)
	{
		m_vid.setAA(m_aa, true);
		for(int i = 0; i < m_aa; ++i)
		{
			m_vid.prepareAAPass(i);
			m_vid.setup2DProjection(false, true);
			_drawBg();
			m_fa.draw(false);
			CoverFlow.draw();
			m_vid.setup2DProjection(false, true);
			CoverFlow.drawEffect();
			if(!m_banner.GetSelectedGame())
				CoverFlow.drawText(adjusting);
			m_vid.renderAAPass(i);
		}
		m_vid.setup2DProjection();
		m_vid.drawAAScene();
	}
	else
	{
		_drawBg();
		m_fa.draw(false);
		if(withCF)
		{
			CoverFlow.draw();
			m_vid.setup2DProjection();
			CoverFlow.drawEffect();
			if(!m_banner.GetSelectedGame())
				CoverFlow.drawText(adjusting);
		}
	}
	if(m_fa.isLoaded())
		m_fa.draw();
	else if(m_banner.GetSelectedGame() && (!m_banner.GetInGameSettings() || (m_banner.GetInGameSettings() && m_bnr_settings)))
		m_banner.Draw();

	m_btnMgr.draw();
	ScanInput();
	if(!m_vid.showingWaitMessage())
		m_vid.render();

	if(Sys_Exiting())
		exitHandler(BUTTON_CALLBACK);

	if(withCF && m_gameSelected && m_gamesound_changed && !m_soundThrdBusy && !m_gameSound.IsPlaying() && MusicPlayer.GetVolume() == 0)
	{
		CheckGameSoundThread();
		m_gameSound.Play(m_bnrSndVol);
		m_gamesound_changed = false;
	}
	else if(!m_gameSelected)
		m_gameSound.Stop();

	MusicPlayer.Tick(m_video_playing || (m_gameSelected && 
		m_gameSound.IsLoaded()) ||  m_gameSound.IsPlaying());

	if(MusicPlayer.SongChanged() && m_music_info)
	{
		m_btnMgr.setText(m_mainLblCurMusic, MusicPlayer.GetFileName(), true);
		m_btnMgr.show(m_mainLblCurMusic);
		MusicPlayer.DisplayTime = time(NULL);
	}
	else if(MusicPlayer.DisplayTime > 0 && time(NULL) - MusicPlayer.DisplayTime > 3)
	{
		MusicPlayer.DisplayTime = 0;
		m_btnMgr.hide(m_mainLblCurMusic);
	}

	//Take Screenshot
	if(gc_btnsPressed & PAD_TRIGGER_Z)
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

#ifdef SHOWMEM
	m_btnMgr.setText(m_mem2FreeSize, wfmt(L"Mem2 Free:%u, Mem1 Free:%u", MEM2_freesize(), MEM1_freesize()), true);
#endif

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

void CMenu::_setBg(const STexture &tex, const STexture &lqTex)
{
	m_lqBg = lqTex;
	if(tex.data == m_nextBg.data)
		return;
	m_prevBg.CopyTexture(m_curBg);
	m_curBg.Cleanup();
	m_nextBg.CopyTexture(tex);
	m_bgCrossFade = 0xFF;
}

void CMenu::_updateBg(void)
{
	Mtx modelViewMtx;
	GXTexObj texObj;
	GXTexObj texObj2;

	if (m_bgCrossFade == 0) return;
	m_bgCrossFade = max(0, (int)m_bgCrossFade - 14);
	if (m_bgCrossFade == 0)
	{
		m_curBg.Cleanup();
		m_curBg = m_nextBg;
		return;
	}
	GX_ClearVtxDesc();
	GX_SetNumTevStages(m_prevBg.data == NULL ? 1 : 2);
	GX_SetNumChans(0);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(m_prevBg.data == NULL ? 1 : 2);
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
	GX_InitTexObj(&texObj, m_nextBg.data, m_nextBg.width, m_nextBg.height, m_nextBg.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	if(m_prevBg.data != NULL)
	{
		GX_InitTexObj(&texObj2, m_prevBg.data, m_prevBg.width, m_prevBg.height, m_prevBg.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
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
	if(m_curBg.data == NULL)
	{
		m_curBg.Cleanup();
		m_curBg = m_nextBg;
		m_bgCrossFade = 0;
	}
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
	_textMain();
	_textError();
	_textConfig();
	_textConfig3();
	_textConfigScreen();
	_textConfig4();
	_textConfigSnd();
	_textConfigAdv();
	_textDownload();
	_textGame();
	_textCode();
	_textWBFS();
	_textGameSettings();
	_textCategorySettings();
	_textSystem();
}

const wstringEx CMenu::_fmt(const char *key, const wchar_t *def)
{
	wstringEx ws = m_loc.getWString(m_curLanguage, key, def);
	if (checkFmt(def, ws)) return ws;
	return def;
}

bool CMenu::_loadChannelList(void)
{
	string emuPath;	
	int emuPartition = -1;

	bool disable_emu = (m_cfg.getBool(CHANNEL_DOMAIN, "disable", true) || neek2o());

	if(!disable_emu)
	{
		m_partRequest = m_cfg.getInt(CHANNEL_DOMAIN, "partition", 1);
		emuPartition = _FindEmuPart(&emuPath, m_partRequest, false);
	
		if(emuPartition < 0)
			emuPartition = _FindEmuPart(&emuPath, m_partRequest, true);

		if(emuPartition < 0)
			return false;
		else
			currentPartition = emuPartition;
	}

	if(!disable_emu)
	{
		char basepath[64];
		snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[currentPartition], emuPath.c_str());
		NandHandle.PreNandCfg(basepath, m_cfg.getBool(CHANNEL_DOMAIN, "real_nand_miis", false), m_cfg.getBool(CHANNEL_DOMAIN, "real_nand_config", false));
	}
	NandHandle.Disable_Emu();
	if(!disable_emu)
	{
		MusicPlayer.Stop();
		TempLoadIOS();
		DeviceHandle.UnMount(currentPartition);
		NandHandle.SetPaths(emuPath.c_str(), currentPartition, disable_emu);
		if(NandHandle.Enable_Emu() < 0)
			NandHandle.Disable_Emu();
	}
	if(!DeviceHandle.IsInserted(currentPartition))
		DeviceHandle.Mount(currentPartition);

	string cacheDir;
	if(!disable_emu)
		cacheDir = fmt("%s/%s_channels.db", m_listCacheDir.c_str(), DeviceName[currentPartition]);
	bool updateCache = m_cfg.getBool(_domainFromView(), "update_cache");
	vector<string> NullVector;
	m_gameList.CreateList(m_current_view, currentPartition, std::string(), 
				NullVector, cacheDir, updateCache);
	return m_gameList.size() > 0 ? true : false;
}

bool CMenu::_loadList(void)
{
	CoverFlow.clear();
	if((m_current_view == COVERFLOW_CHANNEL && m_cfg.getBool(CHANNEL_DOMAIN, "disable", true))
	|| (m_current_view != COVERFLOW_CHANNEL && NandHandle.EmulationEnabled()))
	{
		MusicPlayer.Stop();
		NandHandle.Disable_Emu();
		TempLoadIOS(IOS_TYPE_NORMAL_IOS);
	}
	gprintf("Switching View to %s\n", _domainFromView());

	bool retval;
	switch(m_current_view)
	{
		case COVERFLOW_CHANNEL:
			retval = _loadChannelList();
			break;
		case COVERFLOW_HOMEBREW:
			retval = _loadHomebrewList();
			break;
		case COVERFLOW_DML:
			retval = _loadDmlList();
			break;
		case COVERFLOW_PLUGIN:
			retval = _loadEmuList();
			break;
		default:
			retval = _loadGameList();
			break;
	}
	gprintf("Games found: %i\n", m_gameList.size());
	m_cfg.remove(_domainFromView(), "update_cache");

	return retval;
}

bool CMenu::_loadGameList(void)
{
	currentPartition = m_cfg.getInt(WII_DOMAIN, "partition", USB1);
	if(!DeviceHandle.IsInserted(currentPartition))
		return false;

	m_gameList.clear();
	DeviceHandle.OpenWBFS(currentPartition);
	string gameDir(fmt(GAMES_DIR, DeviceName[currentPartition]));
	string cacheDir(fmt("%s/%s_wii.db", m_listCacheDir.c_str(), DeviceName[currentPartition]));
	bool updateCache = m_cfg.getBool(WII_DOMAIN, "update_cache");
	m_gameList.CreateList(m_current_view, currentPartition, gameDir, stringToVector(".wbfs|.iso", '|'), cacheDir, updateCache);
	WBFS_Close();

	return m_gameList.size() > 0 ? true : false;
}

bool CMenu::_loadHomebrewList()
{
	currentPartition = m_cfg.getInt(HOMEBREW_DOMAIN, "partition", SD);
	if(!DeviceHandle.IsInserted(currentPartition))
		return false;

	m_gameList.clear();
	string gameDir(fmt(HOMEBREW_DIR, DeviceName[currentPartition]));
	string cacheDir(fmt("%s/%s_homebrew.db", m_listCacheDir.c_str(), DeviceName[currentPartition]));
	bool updateCache = m_cfg.getBool(HOMEBREW_DOMAIN, "update_cache");
	m_gameList.CreateList(m_current_view, currentPartition, gameDir, stringToVector(".dol|.elf", '|'), cacheDir, updateCache);

	return m_gameList.size() > 0 ? true : false;
}

bool CMenu::_loadDmlList()
{
	currentPartition = m_cfg.getInt(GC_DOMAIN, "partition", USB1);
	if(!DeviceHandle.IsInserted(currentPartition))
		return false;

	m_gameList.clear();
	string gameDir(fmt(currentPartition == SD ? DML_DIR : m_DMLgameDir.c_str(), DeviceName[currentPartition]));
	string cacheDir(fmt("%s/%s_gamecube.db", m_listCacheDir.c_str(), DeviceName[currentPartition]));
	bool updateCache = m_cfg.getBool(GC_DOMAIN, "update_cache");
	m_gameList.CreateList(m_current_view, currentPartition, gameDir,
			stringToVector(".iso|root", '|'),cacheDir, updateCache);

	return m_gameList.size() > 0 ? true : false;
}

static vector<string> INI_List;
static void GrabINIFiles(char *FullPath)
{
	//Just push back
	INI_List.push_back(FullPath);
}

bool CMenu::_loadEmuList()
{
	currentPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition", SD);
	if(!DeviceHandle.IsInserted(currentPartition))
		return false;
	bool updateCache = m_cfg.getBool(PLUGIN_DOMAIN, "update_cache");

	vector<dir_discHdr> emuList;
	Config m_plugin_cfg;

	INI_List.clear();
	m_gameList.clear();
	GetFiles(m_pluginsDir.c_str(), stringToVector(".ini", '|'), GrabINIFiles, false, 1);
	for(vector<string>::const_iterator Name = INI_List.begin(); Name != INI_List.end(); ++Name)
	{
		if(Name->find("scummvm.ini") != string::npos)
			continue;
		m_plugin_cfg.load(Name->c_str());
		if(m_plugin_cfg.loaded())
		{
			m_plugin.AddPlugin(m_plugin_cfg);
			const char *MagicNumber = m_plugin_cfg.getString(PLUGIN_INI_DEF,"magic").c_str();
			if(!m_cfg.getBool("PLUGIN", MagicNumber, false))
				continue;
			u32 MagicWord = strtoul(MagicNumber, NULL, 16);	
			if(m_plugin_cfg.getString(PLUGIN_INI_DEF,"romDir").find("scummvm.ini") == string::npos)
			{
				string gameDir(fmt("%s:/%s", DeviceName[currentPartition], m_plugin_cfg.getString(PLUGIN_INI_DEF,"romDir").c_str()));
				string cacheDir(fmt("%s/%s_%s.db", m_listCacheDir.c_str(), DeviceName[currentPartition], m_plugin_cfg.getString(PLUGIN_INI_DEF,"magic").c_str()));
				vector<string> FileTypes = stringToVector(m_plugin_cfg.getString(PLUGIN_INI_DEF,"fileTypes"), '|');
				m_gameList.Color = strtoul(m_plugin_cfg.getString(PLUGIN_INI_DEF,"coverColor").c_str(), NULL, 16);
				m_gameList.Magic = MagicWord;
				m_gameList.CreateList(m_current_view, currentPartition, gameDir, FileTypes, cacheDir, updateCache);
				for(vector<dir_discHdr>::iterator tmp_itr = m_gameList.begin(); tmp_itr != m_gameList.end(); tmp_itr++)
					emuList.push_back(*tmp_itr);
			}
			else
			{
				Config scummvm;
				vector<dir_discHdr> scummvmList;
				scummvm.load(fmt("%s/%s", m_pluginsDir.c_str(), "scummvm.ini"));
				scummvmList = m_plugin.ParseScummvmINI(scummvm, DeviceName[currentPartition], MagicWord);
				for(vector<dir_discHdr>::iterator tmp_itr = scummvmList.begin(); tmp_itr != scummvmList.end(); tmp_itr++)
					emuList.push_back(*tmp_itr);
			}
		}
		m_plugin_cfg.unload();
	}
	m_gameList.clear();
	for(vector<dir_discHdr>::iterator tmp_itr = emuList.begin(); tmp_itr != emuList.end(); tmp_itr++)
	{
		tmp_itr->index = m_gameList.size();
		m_gameList.push_back(*tmp_itr);
	}
	emuList.clear();
	//If we return to the coverflow before wiiflow quit we dont need to reload plugins
	m_plugin.EndAdd();

	return m_gameList.size() > 0 ? true : false;
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
	size = 0;
	FILE *fp = fopen(file == NULL ? path : fmt("%s/%s", path, file), "rb");
	if (fp == NULL)
		return false;

	fseek(fp, 0, SEEK_END);
	u32 fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	u8 *fileBuf = (u8*)MEM2_alloc(fileSize);
	if (fileBuf == NULL)
	{
		fclose(fp);
		return false;
	}
	if (fread(fileBuf, 1, fileSize, fp) != fileSize)
	{
		fclose(fp);
		return false;
	}
	fclose(fp);

	if(buffer != NULL)
		free(buffer);
	buffer = fileBuf;

	size = fileSize;
	return true;
}

void CMenu::_load_installed_cioses()
{
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
	m_vid.waitMessage(_textures("GENERAL", "waitmessage"), m_theme.getFloat("GENERAL", "waitmessage_delay", 0.f));
}

typedef struct map_entry
{
	char filename[8];
	u8 sha1[20];
} __attribute((packed)) map_entry_t;

void CMenu::loadDefaultFont(void)
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
				free(u8_font_archive);
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

				free(u8_font_archive);
			}
		}
	}
	if(!retry && m_base_font == NULL)
	{
		retry = true;
		goto retry;
	}
	free(content);
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

string CMenu::_getId()
{
	string id;
	if(!NoGameID(CoverFlow.getHdr()->type))
		id = CoverFlow.getId();
	else
	{
		dir_discHdr *hdr = CoverFlow.getHdr();
		string tempname(hdr->path);
		if(hdr->type == TYPE_HOMEBREW)
		{
			tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
			id = tempname;
		}
		else if(hdr->type == TYPE_PLUGIN)
		{
			if(tempname.find(':') != string::npos)
			{
				tempname.erase(0, tempname.find_first_of('/')+1);
				string dirName = tempname.substr(0, tempname.find_first_of('/')+1);
				tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
				if(tempname.find_last_of('.') != string::npos)
				tempname.erase(tempname.find_last_of('.'), tempname.size() - tempname.find_last_of('.'));
				id = dirName+tempname;
			}
			else
				id = tempname;
		}
	}
	return id;
}

const char *CMenu::_domainFromView()
{
	switch(m_current_view)
	{
		case COVERFLOW_CHANNEL:
			return CHANNEL_DOMAIN;
		case COVERFLOW_HOMEBREW:
			return HOMEBREW_DOMAIN;
		case COVERFLOW_DML:
			return GC_DOMAIN;
		case COVERFLOW_PLUGIN:
			return PLUGIN_DOMAIN;
		default:
			return WII_DOMAIN;
	}
	return "NULL";
}

void CMenu::UpdateCache(u32 view)
{
	if(view == COVERFLOW_MAX)
	{
		UpdateCache(COVERFLOW_USB);
		UpdateCache(COVERFLOW_HOMEBREW);
		UpdateCache(COVERFLOW_DML);
		UpdateCache(COVERFLOW_PLUGIN);
		UpdateCache(COVERFLOW_CHANNEL);
		return;
	}

	const char *domain;
	switch(view)
	{
		case COVERFLOW_CHANNEL:
			domain = CHANNEL_DOMAIN;
			break;
		case COVERFLOW_HOMEBREW:
			domain = HOMEBREW_DOMAIN;
			break;
		case COVERFLOW_DML:
			domain = GC_DOMAIN;
			break;
		case COVERFLOW_PLUGIN:
			domain = PLUGIN_DOMAIN;
			break;
		default:
			domain = WII_DOMAIN;
	}

	m_cfg.setBool(domain, "update_cache", true);
}

int CMenu::MIOSisDML()
{
	u32 size = 0;
	char ISFS_Filename[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	strcpy(ISFS_Filename, "/title/00000001/00000101/content/0000000c.app");
	u8 *appfile = ISFS_GetFile(ISFS_Filename, &size, -1);
	if(appfile)
	{
		for(u32 i = 0; i < size; ++i) 
		{
			if(*(vu32*)(appfile+i) == 0x44494F53)
			{
				for(u32 j = 0; j < size; ++j)
				{
					if(*(vu32*)(appfile+j) == 0x4C697465)
					{
						gprintf("DIOS-MIOS Lite is installed as MIOS\n");
						free(appfile);
						return 2;
					}
				}
				gprintf("DIOS-MIOS is installed as MIOS\n");
				free(appfile);
				return 1;
			}
		}
		free(appfile);
	}
	gprintf("DIOS-MIOS (Lite) not found\n");
	return 0;
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

void CMenu::TempLoadIOS(int IOS)
{
	/* Only temp reload in IOS58 mode */
	if(useMainIOS || neek2o() || Sys_DolphinMode())
		return;

	if(IOS == IOS_TYPE_NORMAL_IOS)
		IOS = 58;
	else if(IOS == 0)
		IOS = mainIOS;

	if(CurrentIOS.Version != IOS)
	{
		loadIOS(IOS, true);
		Open_Inputs();
		for(int chan = WPAD_MAX_WIIMOTES-2; chan >= 0; chan--)
			WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
	}
}
