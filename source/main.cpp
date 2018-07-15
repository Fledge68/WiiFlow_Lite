
#include <ogc/system.h>
#include <unistd.h>
#include <sys/stat.h>

#include "const_str.hpp"
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

bool useMainIOS = false;
volatile bool NANDemuView = false;
volatile bool networkInit = false;

/* quick check if we will be using a USB device. */
/* if not then we can skip the 20 second cycle trying to connect a USB device. */
/* this is nice for SD only users */
bool isUsingUSB() {
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
	
	/* If we have the WiiFlow data on USB, then we're using USB */
	if(m_cfg.getBool("general", "data_on_usb", false))
	{
		// data_on_usb is true, so assuming we're using USB.
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
	gprintf("using SD only, no need for USB mounting.\n");
	return false;
}

int main(int argc, char **argv)
{
	MEM_init(); //Inits both mem1lo and mem2
	mainIOS = DOL_MAIN_IOS;// 249
	__exception_setreload(10);
	Gecko_Init(); //USB Gecko and SD/WiFi buffer
	gprintf(" \nWelcome to %s!\nThis is the debug output.\n", VERSION_STRING.c_str());

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
			strncpy(wait_dir, ptr+strlen("waitdir="), sizeof(wait_dir));
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
	if(Sys_DolphinMode())
		gprintf("Dolphin-Emu\n");
	else if(IsOnWiiU())
		gprintf("vWii Mode\n");
	else 
		gprintf("Real Wii\n");

	/* Init video */
	m_vid.init();
	if(showFlashImg)
		m_vid.startImage();
		
	/* Init device partition handlers */
	DeviceHandle.Init();
	
	/* Init NAND handlers */
	NandHandle.Init();
	check_neek2o();
	if(neek2o() || Sys_DolphinMode())
		NandHandle.Init_ISFS();
	else
		NandHandle.LoadDefaultIOS(); /* safe reload to preferred IOS */
		
	/* load and check wiiflow save for possible new IOS and Port settings */
	if(InternalSave.CheckSave())
		InternalSave.LoadSettings();
		
	/* Handle (c)IOS Loading */
	if(neek2o() || Sys_DolphinMode()) /* wont reload anythin */
		iosOK = loadIOS(IOS_GetVersion(), false);
	else if(useMainIOS && CustomIOS(IOS_GetType(mainIOS))) /* Requested */
		iosOK = loadIOS(mainIOS, false) && CustomIOS(CurrentIOS.Type);
		
	/* sys inits */
	Sys_Init();// set reset and power button callbacks
	Sys_ExitTo(EXIT_TO_HBC);// set exit to in case of failed launch

	/* mount Devices */
	DeviceHandle.MountSD();// mount SD before calling isUsingUSB() duh!		
	DeviceHandle.SetMountUSB(isUsingUSB() && !Sys_DolphinMode());
	DeviceHandle.MountAllUSB();// only mounts any USB if isUsingUSB()
	
	/* init wait images and show wait animation */
	m_vid.setCustomWaitImgs(wait_dir, wait_loop);
	m_vid.waitMessage(0.15f);

	/* init controllers for input */
	Open_Inputs();
	
	/* init configs, folders, coverflow, gui and more */
	if(mainMenu.init())
	{
		if(CurrentIOS.Version != mainIOS && !neek2o() && !Sys_DolphinMode())
		{
			if(useMainIOS || !DeviceHandle.UsablePartitionMounted())
			{
				useMainIOS = false;
				mainMenu.TempLoadIOS();
				iosOK = CustomIOS(CurrentIOS.Type);
			}
		}
		if(CurrentIOS.Version == mainIOS)
			useMainIOS = true; //Needed for later checks
		if(!iosOK)
			mainMenu.terror("errboot1", L"No cIOS found!\ncIOS d2x 249 base 56 and 250 base 57 are enough for all your games.");
		else if(!DeviceHandle.UsablePartitionMounted())
			mainMenu.terror("errboot2", L"Could not find a device to save configuration files on!");
		else if(WDVD_Init() < 0)
			mainMenu.terror("errboot3", L"Could not initialize the DIP module!");
		else // alls good lets start wiiflow
		{
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
	Sys_Exit();
	return 0;
}
