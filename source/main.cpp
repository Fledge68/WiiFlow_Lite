#include "video.hpp"
#include "menu/menu.hpp"
#include "loader/disc.h"

#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "text.hpp"
#include <ogc/system.h>
#include <unistd.h>
#include "DeviceHandler.hpp"
#include "homebrew.h"
#include "gecko.h"
#include "wifi_gecko.h"
#include "cios.hpp"
#include "nand.hpp"
#include "defines.h"
#include "svnrev.h"

CMenu *mainMenu;

extern "C" 
{ 
	extern void __exception_setreload(int t);

	void ShowError(const wstringEx &error)
	{
		mainMenu->error(error);
	}
	void HideWaitMessage() 
	{
		mainMenu->_hideWaitMessage();
	}
}

int main(int argc, char **argv)
{
	__exception_setreload(5);

	// Init video
	CVideo vid;
	vid.init();

	MEM2_init(52);
	geckoinit = InitGecko();
	gprintf(" \nWelcome to %s (%s-r%s)!\nThis is the debug output.\n", APP_NAME, APP_VERSION, SVN_REV);
	vid.waitMessage(0.2f);

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
	gprintf("Loading cIOS: %d\n", mainIOS);	

	// Load Custom IOS
	bool iosOK = loadIOS(mainIOS, false);
	
	ISFS_Initialize();

	u8 mainIOSBase = 0;
	iosOK = iosOK && cIOSInfo::D2X(mainIOS, &mainIOSBase);
	gprintf("Loaded cIOS: %u has base %u\n", mainIOS, mainIOSBase);	

	Open_Inputs(); //init wiimote early

	// Init
	Sys_Init();
	Sys_ExitTo(EXIT_TO_HBC);

	int ret = 0;

	do 
	{
		bool deviceAvailable = false;

		u8 timeout = 0;
		while(!deviceAvailable && timeout++ != 20)
		{
			DeviceHandler::Instance()->MountAll();
			sleep(1);

			for(u8 device = USB1; device <= USB8; device++)
			{
				if(DeviceHandler::Instance()->IsInserted(device))
					deviceAvailable = true;
			}
		}
		if(DeviceHandler::Instance()->IsInserted(SD))
			deviceAvailable = true;

		if(!deviceAvailable)
			Sys_Exit();

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
			{
				if(Emulator_boot)
					menu.m_Emulator_boot = true;
				ret = menu.main();
			}
		}
		if (bootHB)
		{
			//IOS_ReloadIOS(58);
			BootHomebrew();
		}
		Open_Inputs(); //reinit wiimote
	} while (ret == 1);
	
	WifiGecko_Close();

	Nand::Instance()->Disable_Emu();
	Nand::DestroyInstance();

	Sys_Exit();
	return 0;
};
