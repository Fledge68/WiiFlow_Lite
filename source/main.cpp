
#include <ogc/system.h>
#include <unistd.h>
#include <sys/stat.h>

#include "defines.h"
#include "booter/external_booter.hpp"
#include "channel/nand.hpp"
#include "channel/nand_save.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gecko/gecko.hpp"
#include "gui/video.hpp"
#include "gui/text.hpp"
#include "homebrew/homebrew.h"
#include "loader/alt_ios_gen.h"
#include "loader/wdvd.h"
#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "loader/cios.h"
#include "loader/nk.h"
#include "menu/menu.hpp"
#include "memory/memory.h"

bool isWiiVC = false;
bool useMainIOS = true;
volatile bool NANDemuView = false;
volatile bool networkInit = false;

/* quick check if we will be using a USB device. */
/* if not then we can skip the 20 second cycle trying to connect a USB device. */
/* this is nice for SD only users */
bool isUsingUSB() {
	if(isWiiVC)
		return false;
	/* First check if the app path exists on the SD card, if not then we're using USB */
	struct stat dummy;
	string appPath = fmt("%s:/%s", DeviceName[SD], APPS_DIR);
	if(DeviceHandle.IsInserted(SD) && DeviceHandle.GetFSType(SD) != PART_FS_WBFS && stat(appPath.c_str(), &dummy) != 0)
	{
		// No app path exists on SD card, so assuming we're using USB.
		return true;
	}
	
	/* Check that the config file exists, or we can't do the following checks */
	string configPath = fmt("%s/" CFG_FILENAME, appPath.c_str());
	if(stat(configPath.c_str(), &dummy) != 0)
	{
		// The app path is on SD but no config file exists, so assuming we might need USB.
		return true;
	}
	
	/* Load the config file */
	Config m_cfg;// = new Config();
	if(!m_cfg.load(configPath.c_str())) 
	{
		// The app path is on SD and a config file exists, but we can't load it, so assuming we might need USB.
		return true;
	}
	
	/* If any of the sections have partition set > 0, we're on USB */
	const char *domains[] = {WII_DOMAIN, GC_DOMAIN, CHANNEL_DOMAIN, PLUGIN_DOMAIN, HOMEBREW_DOMAIN};
	for(int i = 0; i < 5; i++)
	{
		if(!m_cfg.getBool(domains[i], "disable", false) && m_cfg.getInt(domains[i], "partition", SD) != SD)
		{
			// a domain is enabled and partition is not SD, so assuming we're using USB.
			return true;
		}
	}
	
	/* if sd_only is false, then we're using USB */
	if(!m_cfg.getBool("general", "sd_only", true))
	{
		// sd_only is false, so assuming we're using USB.
		return true;
	}
	
	gprintf("using SD only, no need for USB mounting.\n");
	return false;
}

int main(int argc, char **argv)
{
	MEM_init(); //Inits both mem1lo and mem2
	mainIOS = DOL_MAIN_IOS;// 249
	__exception_setreload(10);
	Gecko_Init(); //USB Gecko and SD/WiFi buffer
	gprintf(" \nWelcome to %s %s!\nThis is the debug output.\n", APP_NAME, APP_VERSION);

	bool iosOK = true;
	char *gameid = NULL;
	bool showFlashImg = true;
	bool wait_loop = false;
	char wait_dir[256];
	memset(&wait_dir, 0, sizeof(wait_dir));

	for(u8 i = 0; i < argc; i++)
	{
		if(argv[i] != NULL && strcasestr(argv[i], "ios=") != NULL && strlen(argv[i]) > 4)
		{
			while(argv[i][0] && !isdigit(argv[i][0]))
				argv[i]++;
			if(atoi(argv[i]) < 254 && atoi(argv[i]) > 0)
				mainIOS = atoi(argv[i]);
		}
		else if(strcasestr(argv[i], "waitdir=") != NULL)
		{
			char *ptr = strcasestr(argv[i], "waitdir=");
			strncpy(wait_dir, ptr+strlen("waitdir="), sizeof(wait_dir) - 1);
		}
		else if(strcasestr(argv[i], "Waitloop") != NULL)
			wait_loop = true;
		else if(strcasestr(argv[i], "noflash") != NULL)
			showFlashImg = false;
		else if(strlen(argv[i]) == 6)
		{
			gameid = argv[i];
			for(u8 i = 0; i < 5; i++)
			{
				if(!isalnum(gameid[i]))
					gameid = NULL;
			}
		}	
			
	}
	/* Init video */
	m_vid.init();
	if(showFlashImg)
		m_vid.startImage();
		
	/* check if WiiVC */
	WiiDRC_Init();
	isWiiVC = WiiDRC_Inited();
	
	if(IsOnWiiU())
	{
		gprintf("WiiU\n");
		if(isWiiVC)
			gprintf("WiiVC\n");
		else
			gprintf("vWii Mode\n");
	}
	else 
		gprintf("Real Wii\n");
		
	gprintf("AHBPROT disabled = %s\n", AHBPROT_Patched() ? "yes" : "no");
	IOS_GetCurrentIOSInfo();
	
	/* Init device partition handlers */
	DeviceHandle.Init();
	
	/* Init NAND handlers */
	NandHandle.Init();

	if(isWiiVC)
	{
		NandHandle.Init_ISFS();
		IOS_GetCurrentIOSInfo();
		DeviceHandle.SetModes();
	}
	else
	{
		NandHandle.Init_ISFS();
		
		/* load and check wiiflow save for possible new IOS and Port settings */
		if(InternalSave.CheckSave())
			InternalSave.LoadSettings();
			
		/* Handle (c)IOS Loading */
		if(useMainIOS && CustomIOS(IOS_GetType(mainIOS)))// load cios
		{
			NandHandle.DeInit_ISFS();
			NandHandle.Patch_AHB();
			iosOK = IOS_ReloadIOS(mainIOS) == 0;
			NandHandle.Init_ISFS();
			gprintf("AHBPROT disabled after IOS Reload: %s\n", AHBPROT_Patched() ? "yes" : "no");
			gprintf("Now using ");
		}
		else
			gprintf("Using IOS58\n");// stay on IOS58. no reload to cIOS
		
		IOS_GetCurrentIOSInfo();
		if(CurrentIOS.Type == IOS_TYPE_HERMES)
			load_ehc_module_ex();
		else if(CurrentIOS.Type == IOS_TYPE_WANIN && CurrentIOS.Revision >= 18)
			load_dip_249();
		DeviceHandle.SetModes();
		WDVD_Init();
	}
		
	/* mount SD */
	DeviceHandle.MountSD();// mount SD before calling isUsingUSB() duh!	

	/* mount USB if needed */
	DeviceHandle.SetMountUSB(isUsingUSB());
	bool usb_mounted = DeviceHandle.MountAllUSB();// only mounts any USB if isUsingUSB()
	
	/* init wait images and show wait animation */
	m_vid.setCustomWaitImgs(wait_dir, wait_loop);
	m_vid.waitMessage(0.15f);

	/* init controllers for input */
	Open_Inputs();// WPAD_SetVRes() is called later in mainMenu.init() during cursor init which gets the theme pointer images

	/* sys inits */
	Sys_Init();// set reset and power button callbacks
	
	bool startup_successful = false;
	/* init configs, folders, coverflow, gui and more */
	if(mainMenu.init(usb_mounted))
	{
		if(!iosOK)
			mainMenu.terror("errboot1", L"No cIOS found!\ncIOS d2x 249 base 56 and 250 base 57 are enough for all your games.");
		else if(!DeviceHandle.UsablePartitionMounted())
			mainMenu.terror("errboot2", L"Could not find a device to save configuration files on!");
		else if(WDVD_Init() < 0)
			mainMenu.terror("errboot3", L"Could not initialize the DIP module!");
		else // alls good lets start wiiflow
		{
			startup_successful = true;
			if(!isWiiVC)
				writeStub();// copy return stub to memory
			if(gameid != NULL && strlen(gameid) == 6)// if argv game ID then launch it
				mainMenu.directlaunch(gameid);
			else
				mainMenu.main();// start wiiflow with main menu displayed
		}
		//Exit WiiFlow, no game booted...
		mainMenu.cleanup();// removes all sounds, fonts, images, coverflow, plugin stuff, source menu and clear memory
	}
	ShutdownBeforeExit();// unmount devices and close inputs
	if(startup_successful)// use wiiflow's exit choice otherwise just exit to loader (system menu or hbc)
		Sys_Exit();
	return 0;
}
