
#include <ogc/system.h>
#include <unistd.h>

#include "const_str.hpp"
#include "booter/external_booter.hpp"
#include "channel/nand.hpp"
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

CMenu mainMenu;
bool useMainIOS = false;

int main(int argc, char **argv)
{
	mainIOS = DOL_MAIN_IOS;
	__exception_setreload(5);
	Gecko_Init(); //USB Gecko and SD/WiFi buffer
	gprintf(" \nWelcome to %s!\nThis is the debug output.\n", VERSION_STRING.c_str());

	m_vid.init(); // Init video
	MEM_init(); //Inits both mem1lo and mem2
	DeviceHandle.Init();
	NandHandle.Init();

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
	/* Init ISFS */
	NandHandle.Init_ISFS();
	/* Handle (c)IOS Loading */
	if(neek2o() || Sys_DolphinMode())
		iosOK = loadIOS(IOS_GetVersion(), false);
	else if((AHBRPOT_Patched() && IOS_GetVersion() == 58) || /* Normal HBC or FW Boot */
	(!AHBRPOT_Patched() && IOS_GetType(mainIOS) == IOS_TYPE_STUB)) /* Maybe old HBC or WiiU */
		iosOK = loadIOS(58, false);
	else /* cIOS wanted */
		iosOK = loadIOS(mainIOS, false) && CustomIOS(CurrentIOS.Type);

	// Init
	Sys_Init();
	Sys_ExitTo(EXIT_TO_HBC);

	DeviceHandle.MountAll();
	m_vid.waitMessage(0.15f);

	Open_Inputs();
	mainMenu.init();
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
	else 
	{
		writeStub();
		if(Emulator_boot)
			mainMenu.m_Emulator_boot = true;
		if(gameid != NULL && strlen(gameid) == 6)
			mainMenu.directlaunch(gameid);
		else
			mainMenu.main();
	}
	//Exit WiiFlow, no game booted...
	mainMenu.cleanup();
	ShutdownBeforeExit();
	Sys_Exit();
	return 0;
}
