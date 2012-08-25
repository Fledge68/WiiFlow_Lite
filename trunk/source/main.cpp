
#include <ogc/system.h>
#include <unistd.h>

#include "defines.h"
#include "svnrev.h"

#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gecko/gecko.h"
#include "gecko/wifi_gecko.h"
#include "gui/video.hpp"
#include "gui/text.hpp"
#include "homebrew/homebrew.h"
#include "loader/disc.h"
#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "loader/cios.h"
#include "loader/nk.h"
#include "menu/menu.hpp"
#include "memory/memory.h"

CMenu *mainMenu;
bool useMainIOS;

int main(int argc, char **argv)
{
	__exception_setreload(5);
	InitGecko();

	// Init video
	CVideo vid;
	vid.init();

	Nand::Instance()->Init_ISFS();
	MEM2_init(47); //Should be safe to use
	gprintf(" \nWelcome to %s (%s-r%s)!\nThis is the debug output.\n", APP_NAME, APP_VERSION, SVN_REV);

	char *gameid = NULL;
	bool Emulator_boot = false;
	bool iosOK = false;

	for(u8 i = 0; i < argc; i++)
	{
		if(argv[i] != NULL && strcasestr(argv[i], "ios=") != NULL && strlen(argv[i]) > 4)
		{
			while(argv[i][0] && !isdigit(argv[i][0]))
				argv[i]++;
			if (atoi(argv[i]) < 254 && atoi(argv[i]) > 0)
				mainIOS = atoi(argv[i]);
		}
		else if(strlen(argv[i]) == 6)
		{
			gameid = argv[i];
			for(u8 i = 0; i < 5; i++)
			{
				if(!isalnum(gameid[i]))
					gameid = NULL;
			}
		}
		else if(argv[i] != NULL && strcasestr(argv[i], "EMULATOR_MAGIC") != NULL)
			Emulator_boot = true;
	}
#ifndef DOLPHIN
	// Load Custom IOS
	if(neek2o())
	{
		iosOK = true;
		memset(&CurrentIOS, 0, sizeof(IOS_Info));
		CurrentIOS.Version = 254;
		CurrentIOS.Type = IOS_TYPE_NEEK2O;
		CurrentIOS.Base = 254;
		CurrentIOS.Revision = 999;
		DCFlushRange(&CurrentIOS, sizeof(IOS_Info));
		DeviceHandler::Instance()->SetModes();
	}
	else if(AHBRPOT_Patched())
	{
		gprintf("Loading cIOS: %d\n", mainIOS);	
		iosOK = loadIOS(mainIOS, false) && CustomIOS(CurrentIOS.Type);
	}
	else
	{
		gprintf("AHBPROT patched out, use IOS58\n");
		iosOK = loadIOS(58, false);
	}
#else
	iosOK = true;
#endif

	// Init
	Sys_Init();
	Sys_ExitTo(EXIT_TO_HBC);

	Open_Inputs(); //(re)init wiimote
#ifndef DOLPHIN
	const DISC_INTERFACE *handle = DeviceHandler::GetUSB0Interface();
	bool deviceAvailable = false;
	u8 timeout = time(NULL);
	while(time(NULL) - timeout < 20)
	{
		if(handle->startup() && handle->isInserted())
			break;
		usleep(50000);
	}
#endif
	DeviceHandler::Instance()->MountAll();
	vid.waitMessage(0.15f);
	bool dipOK = Disc_Init() >= 0;

	mainMenu = new CMenu(vid);
	mainMenu->init();
	if(CurrentIOS.Version != mainIOS && useMainIOS)
		iosOK = loadIOS(mainIOS, true) && CustomIOS(CurrentIOS.Type);
	if(DeviceHandler::Instance()->IsInserted(SD) || DeviceHandler::Instance()->IsInserted(USB1))
		deviceAvailable = true;

	if(!iosOK)
		mainMenu->terror("errboot1", L"No cIOS found!\ncIOS d2x 249 base 56 and 250 base 57 are enough for all your games.");
	else if(!deviceAvailable)
		mainMenu->terror("errboot2", L"Could not find a device to save configuration files on!");
	else if(!dipOK)
		mainMenu->terror("errboot3", L"Could not initialize the DIP module!");
	else if(gameid != NULL && strlen(gameid) == 6)
		mainMenu->directlaunch(gameid);
	else
	{
		if(Emulator_boot)
			mainMenu->m_Emulator_boot = true;
		mainMenu->main();
	}

	mainMenu->cleanup();
	DeviceHandler::Instance()->UnMountAll();
	Nand::Instance()->DeInit_ISFS();
	Sys_Exit();
	exit(1);
	return 0;
}
