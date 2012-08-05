
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <ogc/lwp_watchdog.h>
#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <string.h>

#include "sha1.h"
#include "fs.h"
#include "mload.h"
#include "sys.h"
#include "channel/channel_launcher.h"
#include "gecko/gecko.h"
#include "memory/mem2.hpp"
#include "memory/memory.h"
#include "wiiuse/wpad.h"

/* Variables */
static bool reset = false;
static bool shutdown = false;
bool exiting = false;

static bool priiloader_def = false;
static bool return_to_hbc = false;
static bool return_to_menu = false;
static bool return_to_priiloader = false;
static bool return_to_disable = false;
static bool return_to_bootmii = false;

extern void __exception_closeall();
extern u32 __PADDisableRecalibration(s32 disable);

void __Wpad_PowerCallback()
{
	/* Poweroff console */
	shutdown = 1;
}

void Open_Inputs(void)
{
	/* Initialize Wiimote subsystem */
	PAD_Init();
	WPAD_Init();

	/* Set POWER button callback */
	WPAD_SetPowerButtonCallback(__Wpad_PowerCallback);
	
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	
	WPAD_SetIdleTimeout(60*5); // idle after 5 minutes
}

void Close_Inputs(void)
{
	u32 cnt;

	/* Disconnect Wiimotes */
	for (cnt = 0; cnt < 4; cnt++)
		WPAD_Disconnect(cnt);

	/* Shutdown Wiimote subsystem */
	WPAD_Shutdown();
}

bool Sys_Exiting(void)
{
	return reset || shutdown || exiting;
}

void Sys_Test(void)
{
	if(reset || shutdown) Close_Inputs();

	if (reset) SYS_ResetSystem(SYS_RESTART, 0, 0);
	else if (shutdown) SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}

void Sys_ExitTo(int option)
{
	priiloader_def = option == PRIILOADER_DEF;
	return_to_hbc = option == EXIT_TO_HBC;
	return_to_menu = option == EXIT_TO_MENU;
	return_to_priiloader = option == EXIT_TO_PRIILOADER;
	return_to_disable = option == EXIT_TO_DISABLE;
	return_to_bootmii = option == EXIT_TO_BOOTMII;

	//magic word to force wii menu in priiloader.
	if(return_to_menu)
	{
		*Priiloader_CFG1 = 0x50756E65;
		*Priiloader_CFG2 = 0x50756E65;
	}
	else if(return_to_priiloader)
	{
		*Priiloader_CFG1 = 0x4461636F;
		*Priiloader_CFG2 = 0x4461636F;
	}
	else
	{
		*Priiloader_CFG1 = 0xFFFFFFFF;
		*Priiloader_CFG2 = 0xFFFFFFFF;
	}
	DCFlushRange((void*)Priiloader_CFG1, 4);
	DCFlushRange((void*)Priiloader_CFG2, 4);
}

void Sys_Exit(void)
{
	if(return_to_disable)
		return;

	/* Shutdown Inputs */
	Close_Inputs();

	WII_Initialize();
	if(return_to_menu || return_to_priiloader || priiloader_def)
		Sys_LoadMenu();
	else if(return_to_bootmii)
		IOS_ReloadIOS(254);

	//else
	WII_LaunchTitle(HBC_108);
	WII_LaunchTitle(HBC_JODI);
	WII_LaunchTitle(HBC_HAXX);
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void __Sys_ResetCallback(void)
{
	reset = true;
}

void __Sys_PowerCallback(void)
{
	shutdown = true;
}

void Sys_Init(void)
{
	/* Set RESET/POWER button callback */
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

void Sys_LoadMenu(void)
{
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}
