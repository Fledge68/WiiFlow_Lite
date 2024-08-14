
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
bool sdOnly = false;
volatile bool NANDemuView = false;
volatile bool networkInit = false;

int main(int argc, char **argv)
{
	MEM_init(); //Inits both mem1lo and mem2
	mainIOS = DOL_MAIN_IOS;// 249
	__exception_setreload(10);
	Gecko_Init(); //USB Gecko and SD/WiFi buffer
	#ifdef COMMITHASH
		gprintf(" \nWelcome to %s %s %s!\nThis is the debug output.\n", APP_NAME, APP_VERSION, COMMITHASH);
	#else
		gprintf(" \nWelcome to %s %s!\nThis is the debug output.\n", APP_NAME, APP_VERSION);
	#endif

	char gameid[6];
	memset(&gameid, 0, sizeof(gameid));
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
		else if(strlen(argv[i]) == 6 || strlen(argv[i]) == 4)
		{
			strcpy(gameid, argv[i]);
			for(u8 j = 0; j < strlen(gameid) - 1; j++)
			{
				if(!isalnum(gameid[j]))
				{
					gameid[0] = 0;
					break;
				}
			}
		}
			
	}
	/* Init video */
	m_vid.init();

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
			IOS_ReloadIOS(mainIOS);
			NandHandle.Init_ISFS();
			gprintf("AHBPROT disabled after IOS Reload: %s\n", AHBPROT_Patched() ? "yes" : "no");
			gprintf("Now using ");// gprintf finished in IOS_GetCurrentIOSInfo()
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
	DeviceHandle.SetMountUSB(!sdOnly && !isWiiVC);
	bool usb_mounted = DeviceHandle.MountAllUSB();// only mounts any USB if !sdOnly
	
	/* init wait images and show wait animation */
	if(strlen(gameid) == 0)// dont show if autobooting a wii game.
	{
		m_vid.setCustomWaitImgs(wait_dir, wait_loop);
		m_vid.waitMessage(0.15f);
	}

	/* init controllers for input */
	Open_Inputs();// WPAD_SetVRes() is called later in mainMenu.init() during cursor init which gets the theme pointer images

	/* sys inits */
	Sys_Init();// set reset and power button callbacks
	
	bool startup_successful = false;
	/* init configs, folders, coverflow, gui and more */
	if(mainMenu.init(usb_mounted))
	{
		startup_successful = true;
		if(!isWiiVC)
			writeStub();// copy return stub to memory
		if(!isWiiVC && strlen(gameid) != 0)// if argv game ID then launch it
			mainMenu.directlaunch(gameid);
		else
			mainMenu.main();// start wiiflow with main menu displayed
	}
	// at this point either wiiflow bootup failed or the user is exiting wiiflow
	ShutdownBeforeExit();// unmount devices and close inputs
	if(startup_successful)// use wiiflow's exit choice
		Sys_Exit();
	return 0;// otherwise just exit to loader (system menu or hbc).
}
