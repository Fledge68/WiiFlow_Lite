
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
#include "menu/menu.hpp"

CMenu *mainMenu;

extern "C" 
{ 
	extern void __exception_setreload(int t);

	void ShowError(const wstringEx &error) { mainMenu->error(error); }
	void HideWaitMessage() { mainMenu->_hideWaitMessage(); }
}

int main(int argc, char **argv)
{
	__exception_setreload(5);
	InitGecko();

	// Init video
	CVideo vid;
	vid.init();

	MEM2_init(47); //Should be safe to use
	vid.waitMessage(0.1f);

	AllocSDGeckoBuffer();
	gprintf(" \nWelcome to %s (%s-r%s)!\nThis is the debug output.\n", APP_NAME, APP_VERSION, SVN_REV);

	char *gameid = NULL;
	bool Emulator_boot = false;

	for (int i = 0; i < argc; i++)
	{
		if (argv[i] != NULL && strcasestr(argv[i], "ios=") != NULL && strlen(argv[i]) > 4)
		{
			while(argv[i][0] && !isdigit(argv[i][0])) argv[i]++;
			if (atoi(argv[i]) < 254 && atoi(argv[i]) > 0)
				mainIOS = atoi(argv[i]);
		}
		else if (strlen(argv[i]) == 6)
		{
			gameid = argv[i];
			for (int i=0; i < 5; i++)
				if (!isalnum(gameid[i]))
					gameid = NULL;
		}
		else if (argv[i] != NULL && strcasestr(argv[i], "EMULATOR_MAGIC") != NULL)
			Emulator_boot = true;
	}
#ifndef DOLPHIN
	// Load Custom IOS
	gprintf("Loading cIOS: %d\n", mainIOS);	
	bool iosOK = loadIOS(mainIOS, false, false);

	u8 mainIOSBase = 0;
	D2X(mainIOS, &mainIOSBase);
	if(!mainIOSBase)
		mainIOSBase = get_ios_base();
	iosOK = iosOK && mainIOSBase > 0;
	gprintf("Loaded cIOS: %u has base %u\n", mainIOS, mainIOSBase);	
#else
	bool iosOK = true;
#endif

	// Init
	Sys_Init();
	Sys_ExitTo(EXIT_TO_HBC);

	int ret = 1;

	while(ret == 1) 
	{
		Open_Inputs(); //(re)init wiimote
#ifndef DOLPHIN
		const DISC_INTERFACE *handle = DeviceHandler::GetUSB0Interface();
		bool deviceAvailable = false;
		u8 timeout = 0;
		DeviceHandler::Instance()->MountSD();
		while(!deviceAvailable && timeout++ != 20)
		{
			deviceAvailable = (handle->startup() && handle->isInserted());
			if(deviceAvailable)
				break;
			usleep(50000);
		}
		DeviceHandler::Instance()->MountAllUSB();
		if(DeviceHandler::Instance()->IsInserted(SD))
			deviceAvailable = true;
#else
		bool deviceAvailable = true;
		DeviceHandler::Instance()->MountAll();
#endif
		bool dipOK = Disc_Init() >= 0;

		mainMenu = new CMenu(vid);
		mainMenu->init();

		if(!iosOK)
		{
			mainMenu->terror("errboot1", L"No cIOS found!\ncIOS d2x 249 base 56 and 250 base 57 are enough for all your games.");
			break;
		}
		else if(!deviceAvailable)
		{
			mainMenu->terror("errboot2", L"Could not find a device to save configuration files on!");
			break;
		}
		else if(!dipOK)
		{
			mainMenu->terror("errboot3", L"Could not initialize the DIP module!");
			break;
		}
		else if(gameid != NULL && strlen(gameid) == 6)
			mainMenu->directlaunch(gameid);
		else
		{
			if(Emulator_boot)
				mainMenu->m_Emulator_boot = true;
			ret = mainMenu->main();
		}
	}
	mainMenu->cleanup();
#ifndef DOLPHIN
	ISFS_Deinitialize();
#endif
	Sys_Exit();
	exit(1);
	return 0;
}
