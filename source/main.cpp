#include "video.hpp"
#include "menu/menu.hpp"
#include "loader/disc.h"

#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "text.hpp"
#include <ogc/system.h>
#include <unistd.h>
#include <wiilight.h>
#include "DeviceHandler.hpp"
#include "homebrew.h"
#include "gecko.h"
#include "wifi_gecko.h"
#include "cios.hpp"
#include "nand.hpp"

extern "C" { extern void __exception_setreload(int t);}

CMenu *mainMenu;
extern "C" void ShowError(const wstringEx &error){mainMenu->error(error); }
extern "C" void HideWaitMessage() {mainMenu->_hideWaitMessage(true); }

int main(int argc, char **argv)
{
	geckoinit = InitGecko();
	__exception_setreload(5);

	SYS_SetArena1Hi(APPLOADER_START);
	CVideo vid;

	char *gameid = NULL;

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
	}
	gprintf("Loading cIOS: %d\n", mainIOS);


	ISFS_Initialize();

	// Load Custom IOS
	bool iosOK = loadIOS(mainIOS, false);
	MEM2_init(52);

	u8 mainIOSBase = 0;
	iosOK = iosOK && cIOSInfo::D2X(mainIOS, &mainIOSBase);
	gprintf("Loaded cIOS: %u has base %u\n", mainIOS, mainIOSBase);

	// Init video
	vid.init();
	WIILIGHT_Init();

	vid.waitMessage(0.2f);

	// Init
	Sys_Init();
	Sys_ExitTo(EXIT_TO_HBC);

	int ret = 0;

	do 
	{
		Open_Inputs();

		bool deviceAvailable = false;

		u8 timeout = 0;
		while(!deviceAvailable && timeout++ != 20)
		{
			DeviceHandler::Instance()->MountAll();
			sleep(1);

			for(u8 device = SD; device <= USB8; device++)
				if(DeviceHandler::Instance()->IsInserted(device))
					deviceAvailable = true;
		}
		if(!deviceAvailable) Sys_Exit();

		bool dipOK = Disc_Init() >= 0;

		CMenu menu(vid);
		menu.init();
		mainMenu = &menu;
		if (!iosOK)
		{
			menu.error(sfmt("d2x cIOS %i rev6 or later is required", mainIOS));
			break;
		}
		else if (!dipOK)
		{
			menu.error(L"Could not initialize the DIP module!");
			break;
		}
		else
		{
			if (gameid != NULL && strlen(gameid) == 6)
				menu._directlaunch(gameid);
			else
				ret = menu.main();
		}
		vid.cleanup();
		if (bootHB)
		{
			IOS_ReloadIOS(58);
			BootHomebrew();
		}

	} while (ret == 1);
	
	WifiGecko_Close();

	Nand::Instance()->Disable_Emu();
	Nand::DestroyInstance();

	Sys_Exit();
	return 0;
};