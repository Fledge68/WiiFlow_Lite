
#include <fcntl.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_watchdog.h>

#include "menu.hpp"
#include "types.h"
#include "booter/external_booter.hpp"
#include "channel/channel_launcher.h"
#include "channel/channels.h"
#include "channel/nand.hpp"
#include "channel/identify.h"
#include "devicemounter/DeviceHandler.hpp"
#include "devicemounter/sdhc.h"
#include "devicemounter/usbstorage.h"
#include "gc/gc.hpp"
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
#include "network/gcard.h"

static void setLanguage(int l)
{
	if (l > 0 && l <= 10)
		configbytes[0] = l - 1;
	else
		configbytes[0] = 0xCD;
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

void CMenu::directlaunch(const char *GameID)// from boot arg for wii game only
{
	m_directLaunch = true;
	u8 chantypes = m_cfg.getUInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_REAL);
	m_cfg.setUInt(CHANNEL_DOMAIN, "channels_type", CHANNELS_EMU);
	m_current_view = COVERFLOW_WII | COVERFLOW_GAMECUBE | COVERFLOW_CHANNEL;
	_loadList();
	m_cfg.setUInt(CHANNEL_DOMAIN, "channels_type", chantypes);
	
	for(u32 i = 0; i < m_gameList.size(); i++)
	{
		if(strncasecmp(GameID, m_gameList[i].id, 6) == 0)
		{
			_launch(&m_gameList[i]); // Launch will exit wiiflow
			break;
		}
	}
	
	_error(wfmt(_fmt("errgame1", L"Cannot find the game with ID: %s"), GameID));
}

void CMenu::_launchShutdown()
{
	CoverFlow.clear();
	if(!m_directLaunch)
		_showWaitMessage();
	exitHandler(PRIILOADER_DEF); //Making wiiflow ready to boot something
}

void CMenu::_launch(const dir_discHdr *hdr)
{
	dir_discHdr launchHdr;
	memcpy(&launchHdr, hdr, sizeof(dir_discHdr));
	
	MusicPlayer.Stop();
	m_cfg.setInt("GENERAL", "cat_startpage", m_catStartPage);
	m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));

	/* change to current game's partition */
	switch(launchHdr.type)
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
			int romsPartition = m_plugin.GetRomPartition(m_plugin.GetPluginPosition(launchHdr.settings[0]));
			if(romsPartition < 0)
				romsPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition", 0);
			currentPartition = romsPartition;
			break;
	}

	/* Get Banner Title for Playlog */
	if(launchHdr.type == TYPE_WII_GAME || launchHdr.type == TYPE_CHANNEL || launchHdr.type == TYPE_EMUCHANNEL)
	{
		NANDemuView = launchHdr.type == TYPE_EMUCHANNEL;
		CurrentBanner.ClearBanner();
		if(launchHdr.type == TYPE_CHANNEL || launchHdr.type == TYPE_EMUCHANNEL)
		{
			u64 chantitle = CoverFlow.getChanTitle();
			ChannelHandle.GetBanner(chantitle);
		}
		else if(launchHdr.type == TYPE_WII_GAME)
			_extractBnr(&launchHdr);
		u8 banner_title[84];
		memset(banner_title, 0, 84);
		if(CurrentBanner.IsValid())
			CurrentBanner.GetName(banner_title, GetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str()));
		if(Playlog_Update(launchHdr.id, banner_title) < 0)
			Playlog_Delete();
		CurrentBanner.ClearBanner();
	}

	gprintf("Launching game %s\n", launchHdr.id);
	if(launchHdr.type == TYPE_EMUCHANNEL)
		gprintf("from emu nand\n");
	else if(launchHdr.type == TYPE_CHANNEL)
		gprintf("from real nand\n");

	/* Lets boot that shit */
	if(launchHdr.type == TYPE_WII_GAME)
		_launchWii(&launchHdr, false);
	else if(launchHdr.type == TYPE_GC_GAME)
		_launchGC(&launchHdr, false);
	else if(launchHdr.type == TYPE_CHANNEL || launchHdr.type == TYPE_EMUCHANNEL)
		_launchChannel(&launchHdr);
	else if(launchHdr.type == TYPE_PLUGIN)
		_launchPlugin(&launchHdr);
	else if(launchHdr.type == TYPE_HOMEBREW)
	{
		const char *bootpath = fmt("%s/boot.dol", launchHdr.path);
		if(!fsop_FileExist(bootpath))
			bootpath = fmt("%s/boot.elf", launchHdr.path);
		if(fsop_FileExist(bootpath))
		{
			m_cfg.setString(HOMEBREW_DOMAIN, "current_item", strrchr(launchHdr.path, '/') + 1);
			vector<string> arguments = _getMetaXML(bootpath);
			gprintf("launching homebrew app\n");
			_launchHomebrew(bootpath, arguments);
		}
	}
	// if we make it here it means the launch failed.
	//Exit WiiFlow, no game booted...
	cleanup();// cleanup and clear memory
	ShutdownBeforeExit();// unmount devices and close inputs. launch game failed.
	Sys_Exit();
}

void CMenu::_launchPlugin(dir_discHdr *hdr)
{
	if(strcmp(hdr->id, "PLUGIN") != 0)
	{
		strncpy(m_plugin.PluginMagicWord, fmt("%08x", hdr->settings[0]), 8);
		string gcfg1Key = sfmt("%s/%s", m_platform.getString("PLUGINS", m_plugin.PluginMagicWord).c_str(), hdr->id);
		m_gcfg1.setInt("PLAYCOUNT_PLUGINS", gcfg1Key, m_gcfg1.getInt("PLAYCOUNT_PLUGINS", gcfg1Key, 0) + 1);
		m_gcfg1.setUInt("LASTPLAYED_PLUGINS", gcfg1Key, time(NULL));
	}

	/* get dol name and name length for music plugin */
	const char *plugin_dol_name = m_plugin.GetDolName(hdr->settings[0]);
	u8 plugin_dol_len = strlen(plugin_dol_name);
	
	/* check if music player plugin, if so set wiiflow's bckgrnd music player to play this song or playlist */
	if(plugin_dol_len == 5 && strcasecmp(plugin_dol_name, "music") == 0)
	{
		if(strstr(hdr->path, ".pls") == NULL && strstr(hdr->path, ".m3u") == NULL)
			MusicPlayer.LoadFile(hdr->path, false);
		else
		{
			m_music_info = m_cfg.getBool("GENERAL", "display_music_info", false);
			MusicPlayer.InitPlaylist(m_cfg, hdr->path, currentPartition);// maybe error msg if trouble loading playlist
		}
		return;
	}
	
	/* get title from hdr */
	u32 title_len_no_ext = 0;
	const char *title = CoverFlow.getFilenameId(hdr);// with extension
	
	/* get path from hdr */
	// example rom path - dev:/roms/super mario bros.zip
	// example scummvm path - kq1-coco3		
	const char *path = NULL;
	if(strchr(hdr->path, ':') != NULL)//it's a rom path
	{
		// if there's extension get length of title without extension
		if(strchr(hdr->path, '.') != NULL)
			title_len_no_ext = strlen(title) - strlen(strrchr(title, '.'));
		// get path
		*strrchr(hdr->path, '/') = '\0'; //cut title off end of path
		path = strchr(hdr->path, '/') + 1; //cut dev:/ off of path
	}
	else // it's a scummvm game
		path = hdr->path;// kq1-coco3

	/* get device */
	const char *device = (currentPartition == 0 ? "sd" : (DeviceHandle.GetFSType(currentPartition) == PART_FS_NTFS ? "ntfs" : "usb"));
	
	/* get loader */
	// the loader arg was used and added to plugin mods that fix94 made.
	// it was used because postloader 4 also used the wiiflow plugins and the emus needed to know which loader to return to.
	// the wiimednafen plugin mod still requires this loader arg. most others don't use it.
	const char *loader = fmt("%s:/%s/WiiFlowLoader.dol", device, strchr(m_pluginsDir.c_str(), '/') + 1);

	/* set arguments */
	vector<string> arguments = m_plugin.CreateArgs(device, path, title, loader, title_len_no_ext, hdr->settings[0]);
	
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
	
	/* date fixes for specific plugins */
	if(hdr->settings[0] == 1414875969) //wiituka
		settime(637962048000000000);// Aug 16, 2022
	
	/* launch plugin with args */
	gprintf("launching plugin app\n");
	_launchHomebrew(plugin_file, arguments);
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

	/* load boot.dol into memory and load app_booter.bin into memory */
	bool ret = (LoadHomebrew(filepath) && LoadAppBooter(fmt("%s/app_booter.bin", m_binsDir.c_str())));
	if(ret == false)
	{
		_error(_t("errgame14", L"app_booter.bin not found!"));
		return;
	}
	/* no more error msgs - remove btns and sounds */
	cleanup(); 

	AddBootArgument(filepath);
	for(u32 i = 0; i < arguments.size(); ++i)
	{
		gprintf("app argument: %s\n", arguments[i].c_str());
		AddBootArgument(arguments[i].c_str());
	}

	ShutdownBeforeExit();// before launching homebrew or plugin dol
	NandHandle.Patch_AHB();
	IOS_ReloadIOS(58);
	WII_Initialize();
	BootHomebrew();
	Sys_Exit();
}

vector<string> CMenu::_getMetaXML(const char *bootpath)
{
	char *meta_buf = NULL;
	vector<string> args;
	char meta_path[200];
	char *p;
	char *e, *end;
	struct stat st;
	
	/* load meta.xml */
	
	p = strrchr(bootpath, '/');
	snprintf(meta_path, sizeof(meta_path), "%.*smeta.xml", p ? p-bootpath+1 : 0, bootpath);

	if (stat(meta_path, &st) != 0)
		return args;
	if (st.st_size > 64*1024)
		return args;

	meta_buf = (char *) MEM2_alloc(st.st_size + 1);// +1 so that the buf is 0 terminated
	if (!meta_buf)
		return args;
	memset(meta_buf, 0, st.st_size + 1);
	
	int fd = open(meta_path, O_RDONLY);
	if (fd < 0)
	{
		MEM2_free(meta_buf); 
		meta_buf = NULL; 
		return args;
	}
	read(fd, meta_buf, st.st_size);
	close(fd);

	/* strip comments */

	p = meta_buf;
	int len;
	while (p && *p) 
	{
		p = strstr(p, "<!--");
		if (!p) 
			break;
		e = strstr(p, "-->");
		if (!e) 
		{
			*p = 0; // terminate
			break;
		}
		e += 3;
		len = strlen(e);
		memmove(p, e, len + 1); // +1 for 0 termination
	}
	
	/* parse meta */

	if (strstr(meta_buf, "<app") && strstr(meta_buf, "</app>") && strstr(meta_buf, "<arguments>") && strstr(meta_buf, "</arguments>"))
	{
		p = strstr(meta_buf, "<arguments>");
		end = strstr(meta_buf, "</arguments>");

		do 
		{
			p = strstr(p, "<arg>");
			if (!p) 
				break;
				
			p += 5; //strlen("<arg>");
			e = strstr(p, "</arg>");
			if (!e) 
				break;

			string arg(p, e-p);
			args.push_back(arg);
			p = e + 6;
		} 
		while (p < end);
	}

	MEM2_free(meta_buf); 
	meta_buf = NULL; 
	return args;
}

void CMenu::_launchGC(dir_discHdr *hdr, bool disc)
{
	/* note for a disc boot hdr->id is set to the disc id before _launchGC is called */
	const char *id = hdr->id;
	
	/* Get loader choice*/
	u8 loader = min(m_gcfg2.getUInt(id, "gc_loader", 0), ARRAY_SIZE(CMenu::_GCLoader) - 1u);
	loader = (loader == 0) ? min(m_cfg.getUInt(GC_DOMAIN, "default_loader", 1), ARRAY_SIZE(CMenu::_GlobalGCLoaders) - 1u) : loader-1;
	
	if(disc)
		loader = NINTENDONT;

	/* Check if loader installed */
	if((loader == NINTENDONT && !m_nintendont_installed) || (loader == DEVOLUTION && !m_devo_installed))
	{
		_error(_t("errgame11", L"GameCube Loader not found! Can't launch game."));
		return;
	}
	
	/* Set game path */
	char path[256];
	if(disc)
		strcpy(path, "di");
	else
		strcpy(path, hdr->path);
	path[255] = '\0';
		
	if(loader == NINTENDONT && !disc)// Check if game has multi Discs
	{
		char disc2Path[256];
		strcpy(disc2Path, path);
		disc2Path[255] = '\0';
		*strrchr(disc2Path, '/') = '\0';
		strcat(disc2Path, "/disc2.iso");
		// note fst extracted /boot.bin paths will not have disc2.iso
		if(fsop_FileExist(disc2Path))
		{
			SetupInput();
			_setBg(m_promptBg, m_promptBg);
			m_btnMgr.show(m_promptLblQuestion);
			m_btnMgr.show(m_promptBtnChoice1);
			m_btnMgr.show(m_promptBtnChoice2);
			m_btnMgr.setText(m_promptLblQuestion, _t("discq", L"This game has multiple discs. Please select the disc to launch."));
			m_btnMgr.setText(m_promptBtnChoice1, _t("disc1", L"Disc 1"));
			m_btnMgr.setText(m_promptBtnChoice2, _t("disc2", L"Disc 2"));
			int choice = -1;
			while(!m_exit)
			{
				_mainLoopCommon();
				if(BTN_UP_PRESSED)
					m_btnMgr.up();
				else if(BTN_DOWN_PRESSED)
					m_btnMgr.down();
				else if(BTN_A_PRESSED)
				{
					if(m_btnMgr.selected(m_promptBtnChoice1))
					{
						choice = 0;
						break;
					}
					else if(m_btnMgr.selected(m_promptBtnChoice2))
					{
						choice = 1;
						break;
					}
				}
			}
			m_btnMgr.hide(m_promptLblQuestion);
			m_btnMgr.hide(m_promptBtnChoice1);
			m_btnMgr.hide(m_promptBtnChoice2);
			if(choice < 0)
				return;
			if(choice == 1)
				snprintf(path, sizeof(path), "%s", disc2Path);
		}
	}
	
	/* clear coverflow, start wiiflow wait animation, set exit handler */	
	_launchShutdown();
	
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id);

	/* Get game settings */
	u8 GClanguage = min(m_gcfg2.getUInt(id, "language", 0), ARRAY_SIZE(CMenu::_GClanguages) - 1u);
	GClanguage = (GClanguage == 0) ? min(m_cfg.getUInt(GC_DOMAIN, "game_language", 0), ARRAY_SIZE(CMenu::_GlobalGClanguages) - 1u) : GClanguage-1;
	// language selection only works for PAL games. E and J are always set to english.
	if(id[3] == 'E' || id[3] == 'J')
		GClanguage = 1; //=english
		
	u8 videoMode = min(m_gcfg2.getUInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_GCvideoModes) - 1u);
	videoMode = (videoMode == 0) ? min(m_cfg.getUInt(GC_DOMAIN, "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalGCvideoModes) - 1u) : videoMode-1;

	bool widescreen = m_gcfg2.getBool(id, "widescreen", false);
	bool activity_led = m_gcfg2.getBool(id, "led", false);
	
	if(loader == NINTENDONT)
	{
		/* might add here - if not a disc use path to check for disc2.iso - if so then we need to prompt disc 1 or disc 2? */
		u8 emuMC = min(m_gcfg2.getUInt(id, "emu_memcard", 0), ARRAY_SIZE(CMenu::_NinEmuCard) - 1u);
		emuMC = (emuMC == 0) ? m_cfg.getUInt(GC_DOMAIN, "emu_memcard", 1) : emuMC - 1;
		
		// these 3 settings have global defaults in wfl main config
		bool cc_rumble = m_gcfg2.testOptBool(id, "cc_rumble", m_cfg.getBool(GC_DOMAIN, "cc_rumble", false));
		bool native_ctl = m_gcfg2.testOptBool(id, "native_ctl", m_cfg.getBool(GC_DOMAIN, "native_ctl", false));
		bool wiiu_widescreen = m_gcfg2.testOptBool(id, "wiiu_widescreen", m_cfg.getBool(GC_DOMAIN, "wiiu_widescreen", false));

		bool deflicker = m_gcfg2.getBool(id, "deflicker", false);
		bool tri_arcade = m_gcfg2.getBool(id, "triforce_arcade", false);
		bool ipl = m_gcfg2.getBool(id, "skip_ipl", false);
		bool bba = m_gcfg2.getBool(id, "bba_emu", false);
		bool patch_pal50 = m_gcfg2.getBool(id, "patch_pal50", false);
		bool NIN_Debugger = (m_gcfg2.getInt(id, "debugger", 0) == 2);
		bool cheats = m_gcfg2.getBool(id, "cheat", false);
		
		s8 vidscale = m_gcfg2.getInt(id, "nin_width", 127);
		if(vidscale == 127)
			vidscale = m_cfg.getInt(GC_DOMAIN, "nin_width", 0);
		s8 vidoffset = m_gcfg2.getInt(id, "nin_pos", 127);
		if(vidoffset == 127)
			vidoffset = m_cfg.getInt(GC_DOMAIN, "nin_pos", 0);
		u8 netprofile = 0;
		if(!IsOnWiiU())
			netprofile = m_gcfg2.getUInt(id, "net_profile", 0);
		// project slippi is a mod of nintendont to play patched version of smash bros melee
		bool use_slippi = (m_cfg.getBool(GC_DOMAIN, "use_slippi", false) && strncasecmp(hdr->id, "GAL", 3) == 0);
		gprintf("use slippi %s\n", use_slippi ? "yes" : "no");

		/* configs no longer needed */
		m_gcfg1.save(true);
		m_gcfg2.save(true);
		m_cat.save(true);
		m_cfg.save(true);

		bool ret = (Nintendont_GetLoader(use_slippi) && LoadAppBooter(fmt("%s/app_booter.bin", m_binsDir.c_str())));
		if(ret == false)
		{
			_error(_t("errgame14", L"app_booter.bin not found!"));
			return;
		}
		
		/* no more error msgs - remove btns and sounds */
		cleanup();
		
		//GameID for Video mode when booting a Disc
		memcpy((u8*)Disc_ID, id, 6);
		DCFlushRange((u8*)Disc_ID, 6);
		
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
			
			//use wiiflow cheat folder if is a disc or is on same partition as game folder
			if(disc || strncasecmp(m_cheatDir.c_str(), DeviceName[currentPartition], strlen(DeviceName[currentPartition])) == 0)
				snprintf(CheatPath, sizeof(CheatPath), "%s/%s", m_cheatDir.c_str(), fmt("%s.gct", id));
			else
			{
				/* Generate Game Cheat path - usb1:/games/title [id]/ */
				char GC_Path[256];
				strcpy(GC_Path, path);
				GC_Path[255] = '\0';
				if(strcasestr(path, "boot.bin") != NULL)//usb1:/games/title [id]/sys/boot.bin
				{
					*strrchr(GC_Path, '/') = '\0'; //erase /boot.bin
					*(strrchr(GC_Path, '/')+1) = '\0'; //erase sys folder
				}
				else //usb1:/games/title [id]/game.iso
					*(strrchr(GC_Path, '/')+1) = '\0'; //erase game.iso
			
				// copy cheat file from wiiflow cheat folder to Game folder
				char GC_game_dir[strlen(GC_Path) + 11];
				snprintf(GC_game_dir, sizeof(GC_game_dir), "%s%s.gct", GC_Path, id);
				fsop_CopyFile(fmt("%s/%s.gct", m_cheatDir.c_str(), id), GC_game_dir, NULL, NULL);
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

		if(bba)
			n_config |= NIN_CFG_BBA_EMU;

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

		Nintendont_SetOptions(path, id, CheatPath, GClanguage, n_config, n_videomode, vidscale, vidoffset, netprofile);
		ShutdownBeforeExit();// before launching nintendont
		NandHandle.Patch_AHB();
		IOS_ReloadIOS(58);
		BootHomebrew(); //regular dol
	}
	else // loader == DEVOLUTION
	{
		// devolution does not allow force video
		// ignore video setting choice and use game region always
		if(id[3] =='E' || id[3] =='J')// if game is NTSC then video is based on console video
		{
			if(CONF_GetVideo() == CONF_VIDEO_PAL)// get console video
				videoMode = 2; //PAL 480i 60hz
			else
				videoMode = 3; //NTSC 480i 60hz
		}
		else
			videoMode = 1; //PAL 528i 50hz

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
		ShutdownBeforeExit();// before launching devolution
		NandHandle.Patch_AHB();
		IOS_ReloadIOS(58);
		DEVO_Boot();
	}
	Sys_Exit();
}

/* used to load gameconfig.txt and cheats .gct */
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

/* used by wii and channel games to load the cIOS to use for the game */
/* plugins, apps, and gamecube games don't use cIOS */
int CMenu::_loadGameIOS(u8 gameIOS, int userIOS, const char *id, bool RealNAND_Channels)
{
	gprintf("Game ID %s requested IOS %d.\n", id, gameIOS);
	if(!userIOS)
		gprintf("User selected %d\n", userIOS);
	else
		gprintf("User selected AUTO\n");
	
	// this if seems to have been used if wiiflow was in neek2o mode
	// or cios 249 is a stub and wiiflow runs on ios58
	if(RealNAND_Channels && IOS_GetType(mainIOS) == IOS_TYPE_STUB)
	{
		/* doesn't use cIOS so we don't check userIOS */
		bool ret = loadIOS(gameIOS, false);//load game requested IOS and do not remount sd and USB 
		if(has_enabled_providers() || m_use_wifi_gecko)
			_initAsyncNetwork();// needed after IOS change
		if(ret == false)
		{
			_error(wfmt(_fmt("errgame4", L"Couldn't load IOS %i"), gameIOS));
			return LOAD_IOS_FAILED;
		}
		return LOAD_IOS_SUCCEEDED;
	}	
	
	u8 slot = 0;

	// check if the user wants to use a specific cios and if it's installed.
	if(userIOS && _installed_cios.find(userIOS) != _installed_cios.end())
		slot = userIOS;
	else // auto find a cios base match
	{
		// Workaround for SpongeBobs Boating Bash
		if(strncasecmp(id, "SBV", 3) == 0)
		{
			slot = _cios_base[gameIOS];// try the games'IOS 53
			if(!slot)
				slot = _cios_base[58];
			if(!slot && !IsOnWiiU())
				slot = _cios_base[38];
		}
		else
			slot = _cios_base[gameIOS];
		if(!slot)// no direct match so we get the first cios with a greater base
		{
			for(CIOSItr itr = _cios_base.begin(); itr != _cios_base.end(); itr++)
			{
				if(itr->first > gameIOS && itr->first >= 56)//compare bases
				{
					slot = itr->second;// set to cios slot
					break;
				}
			}
		}
	}
	if(!slot)// shouldn't happen but just in case
		slot = mainIOS;// set to wiiflow's cios
	gprintf("cIOS slot %d chosen.\n", slot);

	// now we reload to this cios slot if we need to
	if(slot != CurrentIOS.Version)
	{
		bool ret = loadIOS(slot, true);//load cIOS requested and then remount sd and USB devices
		if(has_enabled_providers() || m_use_wifi_gecko)
			_initAsyncNetwork();// always seem to do netinit after changing IOS
		if(ret == false)
		{
			_error(wfmt(_fmt("errgame4", L"Couldn't load IOS %i"), slot));
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

	NANDemuView = hdr->type == TYPE_EMUCHANNEL;
	string id = string(hdr->id);
	u64 gameTitle = TITLE_ID(hdr->settings[0],hdr->settings[1]);
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1); 
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));

	bool hbc = false;
	if(gameTitle == HBC_OHBC || gameTitle == HBC_LULZ || gameTitle == HBC_108 || gameTitle == HBC_JODI || gameTitle == HBC_HAXX)
		hbc = true;
		
	/* WII_Launch is used only for launching real nand channels */
	/* note: no patches, cheats, or cIOS settings allowed */
	bool WII_Launch = (m_gcfg2.getBool(id, "custom", false) && !NANDemuView);
	if(WII_Launch || (hbc && !NANDemuView))
	{
		/* configs no longer needed */
		m_gcfg1.save(true);
		m_gcfg2.save(true);
		m_cat.save(true);
		m_cfg.save(true);
		cleanup();//no more error messages we can now cleanup
		ShutdownBeforeExit();// before wii_launch channel
		WII_Initialize();
		WII_LaunchTitle(gameTitle);
	}
		
	/* use_dol = true to use the channels dol or false to use the old apploader method to boot channel */
	bool use_dol = !m_gcfg2.getBool(id, "apploader", false);
	bool use_led = m_gcfg2.getBool(id, "led", false);

	int language = min(m_gcfg2.getUInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	language = (language == 0) ? min(m_cfg.getUInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1u) : language;

	bool vipatch = m_gcfg2.getBool(id, "vipatch", false);
	bool countryPatch = m_gcfg2.getBool(id, "country_patch", false);

	u8 videoMode = min(m_gcfg2.getUInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
	videoMode = (videoMode == 0) ? min(m_cfg.getUInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1u) : videoMode - 1;
	
	u8 patchVidMode = min(m_gcfg2.getUInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);

	s8 aspectRatio = min(m_gcfg2.getUInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1) - 1;// -1,0,1

	int fix480pVal = m_gcfg2.getOptBool(id, "fix480p", 2);
	bool fix480p = fix480pVal == 0 ? false : (fix480pVal == 1 ? true : m_cfg.getBool(WII_DOMAIN, "fix480p", false));

	u8 wiiuWidescreen = min(m_gcfg2.getUInt(id, "widescreen_wiiu", 0), ARRAY_SIZE(CMenu::_WidescreenWiiu) - 1u);

	u8 deflicker = min(m_gcfg2.getUInt(id, "deflicker_wii", 0), ARRAY_SIZE(CMenu::_DeflickerOptions) - 1u);
	deflicker = (deflicker == 0) ? min(m_cfg.getUInt("GENERAL", "deflicker_wii", 0), ARRAY_SIZE(CMenu::_GlobalDeflickerOptions) - 1u) : deflicker - 1;
	
	u8 videoWidth = min(m_gcfg2.getUInt(id, "video_width", 0), ARRAY_SIZE(CMenu::_VideoWidths) - 1u);
	videoWidth = (videoWidth == 0) ? min(m_cfg.getUInt("GENERAL", "video_width", 0), ARRAY_SIZE(CMenu::_GlobalVideoWidths) - 1u) : videoWidth-1;

	u8 private_server = m_gcfg2.getUInt(id, "private_server", 0);
	string server_addr = "";
	if(private_server > 2)
	{
		vector<string> custom_servers = stringToVector(m_cfg.getString("custom_servers", "servers"), '|');
		server_addr = m_cfg.getString("custom_servers", fmt("%s_url", custom_servers[private_server - 3]), "");
	}

	u32 returnTo = 0;
	const char *rtrn = m_cfg.getString("GENERAL", "returnto").c_str();
	if(strlen(rtrn) == 4)
		returnTo = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];

	/* get debugger selected - 0(off), 1(gecko), 2(OSReport) */
	/* gecko requires a hooktype */
	/* Operating System (OS) Report patches every fwrite() in main dol to send debug info. no hooktype is needed. */
	/* 2(OSReport) fwrite() patch is not patched for channels. not sure if it can be */
	debuggerselect = m_gcfg2.getInt(id, "debugger", 0);

	bool cheat = m_gcfg2.getBool(id, "cheat", false);
	hooktype = (u32) m_gcfg2.getInt(id, "hooktype", 0);
	if((cheat || debuggerselect == 1) && hooktype == 0)// cheats or gecko debugger enabled, set hooktype (0)auto to (1)vbi
		hooktype = 1;
	else if(!cheat && debuggerselect != 1)
		hooktype = 0;
	
	u8 *cheatFile = NULL;
	u32 cheatSize = 0;
	if(cheat)
		_loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", id.c_str()));
		
	/* note: no .wip or gameconfig.txt support for channels. not sure why */

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	int userIOS = m_gcfg2.getInt(id, "ios", 0);
	u32 gameIOS = ChannelHandle.GetRequestedIOS(gameTitle);
	
	//interesting - there is only a global option for nand emulation - no per game choice
	int emulate_mode = min(max(0, m_cfg.getInt(CHANNEL_DOMAIN, "emulation", 1)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
	
	bool useNK2o = m_gcfg2.getBool(id, "useneek", false);//if not in neek2o and use neek is set

	if(NANDemuView)
	{
		/* copy real NAND sysconf, settings.txt, & RFL_DB.dat if you want to, they are replaced if they already exist */
		NandHandle.PreNandCfg(m_cfg.getBool(CHANNEL_DOMAIN, "real_nand_miis", false), 
								m_cfg.getBool(CHANNEL_DOMAIN, "real_nand_config", false));
		//m_cfg.setBool(CHANNEL_DOMAIN, "real_nand_miis", false); 
		//m_cfg.setBool(CHANNEL_DOMAIN, "real_nand_config", false);
	}

	/* configs no longer needed */
	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	/* launch via neek2o */
	if(NANDemuView)
	{
		if(useNK2o)
		{
			if(!Load_Neek2o_Kernel())
			{
				_error(_t("errneek1", L"Cannot launch neek2o. Verify your neek2o setup"));//kernal.bin not found
				return;
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
				ShutdownBeforeExit();// before using neek2o to launch a channel
				if(IsOnWiiU())
					Launch_nk(gameTitle, NandHandle.Get_NandPath(), ((u64)(0x00010002) << 32) | (nkreturnTo & 0xFFFFFFFF));
				else
					Launch_nk(gameTitle, NandHandle.Get_NandPath(), ((u64)(0x00010001) << 32) | (nkreturnTo & 0xFFFFFFFF));
				while(1) usleep(500);
			}
		}
	}

	/* load external booter bins */
	if(ExternalBooter_LoadBins(m_binsDir.c_str()) == false)
	{
		_error(_t("errgame16", L"Missing ext_loader.bin or ext_booter.bin!"));
		return;
	}

	/* load selected cIOS if necessary */
	if(_loadGameIOS(gameIOS, userIOS, id.c_str(), !NANDemuView) == LOAD_IOS_FAILED)
	{
		/* error message already shown */
		return;
	}

	/* if d2x cios patch returnto */
	if(CurrentIOS.Type == IOS_TYPE_D2X && returnTo != 0)
	{
		if(D2X_PatchReturnTo(returnTo) >= 0)
			memset(&returnTo, 0, sizeof(u32));//Already patched - no need for giantpune patch in external booter
	}


	// nand emulation not available with hermes cios
	// waninkoko cios rev14 started nand emulation (must be on root of device)
	// waninkoko cios rev18 added full nand emulation
	// rev21 and d2x cios added path support
	if(NANDemuView)
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
			_error(_t("errgame5", L"Enabling emu failed!"));
			return;
		}
		DeviceHandle.MountAll();
	}
	
	cleanup();//no more error messages we can now cleanup
	
	if(wiiuWidescreen > 0 && IsOnWiiU())
	{
		write32(0xd8006a0, wiiuWidescreen == 2 ? 0x30000004 : 0x30000002);
		mask32(0xd8006a8, 0, 2);
	}

	setLanguage(language);// set configbyte[0] for external booter
	ocarina_load_code(cheatFile, cheatSize);// copy to address used by external booter
	if(cheatFile != NULL)
		MEM2_free(cheatFile);
	NandHandle.Patch_AHB(); /* Identify maybe uses it so keep AHBPROT disabled */
	PatchIOS(true,  isWiiVC); /* Patch cIOS for everything */
	Identify(gameTitle);// identify title with E-Ticket Service (ES) module

	ExternalBooter_ChannelSetup(gameTitle, use_dol);
	WiiFlow_ExternalBooter(videoMode, vipatch, countryPatch, patchVidMode, aspectRatio, private_server, server_addr.c_str(), 
							videoWidth, fix480p, deflicker, 0, TYPE_CHANNEL, use_led);

	Sys_Exit();
}

void CMenu::_launchWii(dir_discHdr *hdr, bool dvd, bool disc_cfg)
{
	if(dvd)
	{
		TempLoadIOS();// switch to cIOS if using IOS58 and not in neek2o
		/* Open Disc */
		if(Disc_Open(true) < 0)
		{
			_error(_t("wbfsoperr2", L"Disc_Open failed"));
			Sys_Exit();
		}
		/* Check disc */
		if(Disc_IsWii() < 0)
		{
			if(Disc_IsGC() < 0) 
			{
				_error(_t("errgame9", L"This is not a Wii or GC disc"));
				Sys_Exit();
			}
			else // GC disc
			{
				if(!m_nintendont_installed)
				{
					_error(_t("errgame12", L"Nintendont not found! Can't launch GC Disc."));
					return;
				}
				/* Read GC disc header to get id*/
				Disc_ReadGCHeader(&gc_hdr);
				memcpy(hdr->id, gc_hdr.id, 6);
				
				/* set game type */
				hdr->type = TYPE_GC_GAME;
				
				/* load game configs - gameconfig2.ini */
				m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
				
				/* go to game settings menu if wanted to make changes */
				if(disc_cfg)
					_configGCGame(hdr, dvd);
				
				/* prepare to launch GC disc */
				MusicPlayer.Stop();
				m_cfg.setInt("GENERAL", "cat_startpage", m_catStartPage);
				currentPartition = m_cfg.getInt(GC_DOMAIN, "partition", 1);
				_launchGC(hdr, dvd);
				return;
			}
		}
		else // Wii disc
		{
			/* Read Wii disc header to get id */
			Disc_ReadHeader(&wii_hdr);
			memcpy(hdr->id, wii_hdr.id, 6);
			
			/* set game type */
			hdr->type = TYPE_WII_GAME;
			
			/* load game configs - gameconfig2.ini */
			m_gcfg2.load(fmt("%s/" GAME_SETTINGS2_FILENAME, m_settingsDir.c_str()));
			
			/* go to game settings menu if wanted to make changes */
			if(disc_cfg)
				_configGame(hdr, dvd);
			
			/* prepare to launch Wii disc */
			MusicPlayer.Stop();
			m_cfg.setInt("GENERAL", "cat_startpage", m_catStartPage);
			currentPartition = m_cfg.getInt(WII_DOMAIN, "partition", 1);
		}
	}
	
	/* clear coverflow, start wiiflow wait animation, set exit handler */	
	_launchShutdown();
	string id(hdr->id);
	string path(hdr->path);// empty if a dvd
	m_gcfg1.setInt("PLAYCOUNT", id, m_gcfg1.getInt("PLAYCOUNT", id, 0) + 1);
	m_gcfg1.setUInt("LASTPLAYED", id, time(NULL));
	
	bool use_led = m_gcfg2.getBool(id, "led", false);
	
	int language = min(m_gcfg2.getUInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	language = (language == 0) ? min(m_cfg.getUInt("GENERAL", "game_language", 0), ARRAY_SIZE(CMenu::_languages) - 1u) : language;

	bool vipatch = m_gcfg2.getBool(id, "vipatch", false);
	bool countryPatch = m_gcfg2.getBool(id, "country_patch", false);

	u8 videoMode = min(m_gcfg2.getUInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_VideoModes) - 1u);
	videoMode = (videoMode == 0) ? min(m_cfg.getUInt("GENERAL", "video_mode", 0), ARRAY_SIZE(CMenu::_GlobalVideoModes) - 1u) : videoMode-1;

	u8 patchVidMode = min(m_gcfg2.getUInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);

	s8 aspectRatio = min(m_gcfg2.getUInt(id, "aspect_ratio", 0), ARRAY_SIZE(CMenu::_AspectRatio) - 1u) - 1;

	int fix480pVal = m_gcfg2.getOptBool(id, "fix480p", 2);
	bool fix480p = fix480pVal == 0 ? false : (fix480pVal == 1 ? true : m_cfg.getBool(WII_DOMAIN, "fix480p", false));

	u8 wiiuWidescreen = min(m_gcfg2.getUInt(id, "widescreen_wiiu", 0), ARRAY_SIZE(CMenu::_WidescreenWiiu) - 1u);

	u8 deflicker = min(m_gcfg2.getUInt(id, "deflicker_wii", 0), ARRAY_SIZE(CMenu::_DeflickerOptions) - 1u);
	deflicker = (deflicker == 0) ? min(m_cfg.getUInt("GENERAL", "deflicker_wii", 0), ARRAY_SIZE(CMenu::_GlobalDeflickerOptions) - 1u) : deflicker-1;

	u8 videoWidth = min(m_gcfg2.getUInt(id, "video_width", 0), ARRAY_SIZE(CMenu::_VideoWidths) - 1u);
	videoWidth = (videoWidth == 0) ? min(m_cfg.getUInt("GENERAL", "video_width", 0), ARRAY_SIZE(CMenu::_GlobalVideoWidths) - 1u) : videoWidth-1;
	
	u8 private_server = m_gcfg2.getUInt(id, "private_server", 0);
	string server_addr = "";
	if(private_server > 2)
	{
		vector<string> custom_servers = stringToVector(m_cfg.getString("custom_servers", "servers"), '|');
		server_addr = m_cfg.getString("custom_servers", fmt("%s_url", custom_servers[private_server - 3]), "");
	}

	u32 returnTo = 0;
	const char *rtrn = m_cfg.getString("GENERAL", "returnto").c_str();
	if(strlen(rtrn) == 4)
		returnTo = rtrn[0] << 24 | rtrn[1] << 16 | rtrn[2] << 8 | rtrn[3];
	
	/* get debugger selected - 0(off), 1(gecko), 2(OSReport) */
	/* gecko requires a hooktype */
	/* Operating System (OS) Report patches every fwrite() in main dol to send debug info. no hooktype is needed. */
	debuggerselect = m_gcfg2.getInt(id, "debugger", 0); // debuggerselect is defined in fst.h
	
	/* not sure why we disable debugger if it's Prince of Persia: The Forgotten Sands */
	/* there's wip patches for this game in apploader.c */
	if((id == "RPWE41" || id == "RPWZ41" || id == "SPXP41") && debuggerselect == 1) // Prince of Persia: The Forgotten Sands
		debuggerselect = 0;
		
	bool cheat = m_gcfg2.getBool(id, "cheat", false);	
	hooktype = (u32)m_gcfg2.getInt(id, "hooktype", 0); // hooktype is defined in patchcode.h
	if((cheat || debuggerselect == 1) && hooktype == 0)// cheats or gecko debugger enabled, set hooktype (0)auto to (1)vbi
		hooktype = 1;
	else if(!cheat && debuggerselect != 1)
		hooktype = 0;

	load_wip_patches((u8 *)m_wipDir.c_str(), (u8 *) &id);

	u8 *cheatFile = NULL;
	u8 *gameconfig = NULL;
	u32 cheatSize = 0, gameconfigSize = 0;
	if(cheat)
		_loadFile(cheatFile, cheatSize, m_cheatDir.c_str(), fmt("%s.gct", id.c_str()));
	_loadFile(gameconfig, gameconfigSize, m_txtCheatDir.c_str(), "gameconfig.txt");

	if(has_enabled_providers() && _initNetwork() == 0)
		add_game_to_card(id.c_str());

	int userIOS = m_gcfg2.getInt(id, "ios", 0);
	int gameIOS = dvd ? userIOS : GetRequestedGameIOS(hdr);
	
	setLanguage(language);

	/* emunand gamesave setup if necessary */
	u8 emulate_mode = min(m_gcfg2.getUInt(id, "emulate_save", 0), ARRAY_SIZE(CMenu::_SaveEmu) - 1u);
	u8 gameEmuMode = emulate_mode;
	if(emulate_mode == 0)// default then use global
		emulate_mode = min(max(0, m_cfg.getInt(WII_DOMAIN, "save_emulation", 0)), (int)ARRAY_SIZE(CMenu::_GlobalSaveEmu) - 1);
	else
		emulate_mode--;
		
	if(emulate_mode && !dvd)
	{
		int emuPart = _FindEmuPart(SAVES_NAND, true);
		if(emuPart < 0)//if savepartition is unusable
		{
			_hideWaitMessage();
			_error(_t("errgame13", L"EmuNAND for gamesave not found! Using real NAND."));
			emulate_mode = 0;
			_showWaitMessage();
		}
		else // partition is good so now check if save exists on savesnand
		{
			bool need_config = false;
			bool need_miis = false;
			const char *emuPath = NandHandle.Get_NandPath();
			
			char basepath[MAX_FAT_PATH];
			snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPart], emuPath);
			
			char testpath[strlen(basepath) + 42];
			
			// does not check to see if actual tmd exist just if the folder exist
			if(!_checkSave(id, SAVES_NAND))//if save is not on saves emunand
			{
				/* still use savesnand to create a new save on the savesnand */
				/* make savesnand folders in case they don't already exist */
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
			/* check if saves emulation is set to full per this game in case the global default is not full */
			if(gameEmuMode == 3)// if is then we need to make sure the savesnand contains mii's and config files
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
		emulate_mode = 0;//sets to off if we are launching a DVD game

	// this is a temporary region change of real nand(rn) if 'tempregionrn' is set true.
	// added by overjoy but he never added a gamesettings option. must be set by editing wiiflow_lite.ini.
	// when gamesave emulation set to full it doesn't use real nand, only full emunand emulation.
	// if I understand this, the real nand configs are changed in memory here.
	// and the games dol is patched in the external booter to stop it from changing the config files.
	// honestly though i don't understand the need for this.
	bool patchregion = false;
	if(emulate_mode <= 1 && m_cfg.getBool("GENERAL", "tempregionrn", false))
	{
		gprintf("Temp region change applied\n");
		// change real nand region to game ID[3] region. is reset when you turn wii off.
		patchregion = NandHandle.Do_Region_Change(id, true);
	}
	
	/* save and close config files */
	m_gcfg1.save(true);
	m_gcfg2.save(true);
	m_cat.save(true);
	m_cfg.save(true);

	/* load external booter bin files */
	if(ExternalBooter_LoadBins(m_binsDir.c_str()) == false)
	{
		_error(_t("errgame16", L"Missing ext_loader.bin or ext_booter.bin!"));
		return;
	}

	/* load selected cIOS if necessary */
	if(!dvd)
	{
		if(_loadGameIOS(gameIOS, userIOS, id.c_str()) == LOAD_IOS_FAILED)
		{
			/* error message already shown */
			return;
		}
	}

	/* if d2x cios patch returnto and enable emu nand */
	/* hermes cios can be used for wii games but does not have nand emulation support */
	/* waninkoko cios have some nand emulation support */
	if(CurrentIOS.Type == IOS_TYPE_D2X)
	{
		if(returnTo != 0 && !m_directLaunch && D2X_PatchReturnTo(returnTo) >= 0)
			memset(&returnTo, 0, sizeof(u32));//Already patched - no need for giantpune patch in external booter
		
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
				_error(_t("errgame6", L"Enabling emu after reload failed!"));
				Sys_Exit();
			}
			DeviceHandle.MountAll();
		}
	}
	
	/* no more error msgs - clear btns and snds  and stop wait animation */
	cleanup();

	/* handle frag_list for .wbfs files only */
	bool wbfs_partition = false;
	if(!dvd)
	{
		DeviceHandle.OpenWBFS(currentPartition);
		wbfs_partition = (DeviceHandle.GetFSType(currentPartition) == PART_FS_WBFS);// if USB device formatted to WBFS
		/* if not WBFS formatted get fragmented list. */
		/* if SD card (currentPartition == 0) set sector size to 512 (0x200) */
		if(!wbfs_partition && get_frag_list((u8 *)id.c_str(), (char*)path.c_str(), currentPartition == 0 ? 0x200 : USBStorage2_GetSectorSize()) < 0)
			Sys_Exit();// failed to get frag list
		WBFS_Close();
	}
	
	/* move cheats for external booter */
	if(cheatFile != NULL)
	{
		ocarina_load_code(cheatFile, cheatSize);
		MEM2_free(cheatFile);
	}
	
	/* move gameconfig for external booter */
	if(gameconfig != NULL)
	{
		app_gameconfig_load(id.c_str(), gameconfig, gameconfigSize);
		MEM2_free(gameconfig);
	}
	
	/* if on a wiiu set its widescreen to users choice */
	if(wiiuWidescreen > 0 && IsOnWiiU())
	{
		write32(0xd8006a0, wiiuWidescreen == 2 ? 0x30000004 : 0x30000002);
		mask32(0xd8006a8, 0, 2);
	}
	
	ExternalBooter_WiiGameSetup(wbfs_partition, dvd, patchregion, id.c_str());
	WiiFlow_ExternalBooter(videoMode, vipatch, countryPatch, patchVidMode, aspectRatio, private_server, server_addr.c_str(), 
							videoWidth, fix480p, deflicker, returnTo, TYPE_WII_GAME, use_led);

	Sys_Exit();
}
