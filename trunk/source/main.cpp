
#include <ogc/system.h>
#include <unistd.h>

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
	bool sd_only = false;
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
		else if(strcasestr(argv[i], "sdonly") != NULL)
			sd_only = true;
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
		
	/* set reset and power button callbacks and exitTo option */
	Sys_Init();
	Sys_ExitTo(EXIT_TO_HBC);

	/* mount Devices */
	DeviceHandle.MountSD();
	if(!sd_only && !Sys_DolphinMode())
		DeviceHandle.MountAllUSB();
	
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
