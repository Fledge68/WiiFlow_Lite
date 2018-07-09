
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <ogc/lwp_watchdog.h>
#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "mload.h"
#include "sys.h"
#include "channel/channel_launcher.h"
#include "loader/nk.h"
#include "gecko/gecko.hpp"
#include "memory/mem2.hpp"
#include "memory/memory.h"
#include "wiiuse/wpad.h"
#include "wupc/wupc.h"
#include "sicksaxis-wrapper/sicksaxis-wrapper.h"

/* Variables */
bool reset = false;
bool shutdown = false;
volatile u8 ExitOption = 0;
const char *NeekPath = NULL;
char wii_games_dir[64];
char gc_games_dir[64];

void __Wpad_PowerCallback()
{
	shutdown = 1;
}

void Open_Inputs(void)
{
	/* Initialize Wiimote subsystem */
	PAD_Init();
	WUPC_Init();
	WPAD_Init();
	DS3_Init();
	//drc
	
	/* Set POWER button callback */
	WPAD_SetPowerButtonCallback(__Wpad_PowerCallback);
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetIdleTimeout(60 * 2); // idle after 2 minutes
}

void Close_Inputs(void)
{
	WUPC_Shutdown();

	u32 cnt;

	/* Disconnect Wiimotes */
	for(cnt = 0; cnt < 4; cnt++)
		WPAD_Disconnect(cnt);

	/* Shutdown Wiimote subsystem */
	WPAD_Shutdown();
	DS3_Cleanup();
}

bool Sys_Exiting(void)
{
	DCFlushRange(&reset, 32);
	DCFlushRange(&shutdown, 32);
	return reset || shutdown;
}

int Sys_GetExitTo(void)
{
	return ExitOption;
}

void Sys_ExitTo(int option)
{
	ExitOption = option;
	//magic word to force wii menu in priiloader.
	if(ExitOption == EXIT_TO_MENU)
	{
		*Priiloader_CFG1 = 0x50756E65;// Pune
		*Priiloader_CFG2 = 0x50756E65;
	}
	else if(ExitOption == EXIT_TO_PRIILOADER)
	{
		*Priiloader_CFG1 = 0x4461636F;// Daco
		*Priiloader_CFG2 = 0x4461636F;
	}
	else // PRIILOADER_DEF
	{
		*Priiloader_CFG1 = 0xFFFFFFFF;
		*Priiloader_CFG2 = 0xFFFFFFFF;
	}
	DCFlushRange((void*)Priiloader_CFG1, 4);
	DCFlushRange((void*)Priiloader_CFG2, 4);
}

void Sys_Exit(void)
{
	/* Shutdown Inputs */
	Close_Inputs();
	/* Just shutdown  console*/
	if(ExitOption == BUTTON_CALLBACK)
		SYS_ResetSystem(SYS_POWEROFF_STANDBY, 0, 0);

	/* We wanna to boot something */
	WII_Initialize();
	/* if in neek2o mode Launch_nk will just return to neek2o system menu and not launch anything */
	if(ExitOption == EXIT_TO_WFNK2O)
		Launch_nk(0x1000157464C41LL, NeekPath, 0);// 57464C41 = WFLA : 44574641 = DWFA
	else if(ExitOption == EXIT_TO_SMNK2O)
		Launch_nk(0, NeekPath, 0);
	else if(ExitOption == EXIT_TO_BOOTMII)
		IOS_ReloadIOS(0xfe);// IOS254 Bootmii IOS
	else if(ExitOption == EXIT_TO_HBC)
	{
		WII_LaunchTitle(HBC_OHBC);
		WII_LaunchTitle(HBC_LULZ);
		WII_LaunchTitle(HBC_108);
		WII_LaunchTitle(HBC_JODI);
		WII_LaunchTitle(HBC_HAXX);
	}
	else if(ExitOption == EXIT_TO_WIIU)
		WII_LaunchTitle(WIIU_CHANNEL);
	/* else Return to System Menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	exit(1);
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

bool AHBPROT_Patched(void)
{
	return (*HW_AHBPROT == 0xFFFFFFFF);
}

/* WiiU Check by crediar, thanks */
bool WiiUChecked = false;
bool WiiUMode = false;
bool IsOnWiiU(void)
{
	if(WiiUChecked)
		return WiiUMode;

	if((*HW_PROCESSOR >> 16) == 0xCAFE)
		WiiUMode = true;

	WiiUChecked = true;
	return WiiUMode;
}

void Sys_SetNeekPath(const char *Path)
{
	NeekPath = Path;
}

bool ModeChecked = false;
bool DolphinMode = false;
bool Sys_DolphinMode(void)
{
	if(ModeChecked)
		return DolphinMode;

	/* Thanks to skidau for that code! */
	u32 ifpr11 = 0x12345678;
	u32 ifpr12 = 0x9abcdef0;
	u32 ofpr1 = 0x00000000;
	u32 ofpr2 = 0x00000000;
	asm volatile (
		"lwz 3,%[ifpr11]\n\t"
		"stw 3,8(1)\n\t"
		"lwz 3,%[ifpr12]\n\t"
		"stw 3,12(1)\n\t"

		"lfd 1,8(1)\n\t"
		"frsqrte	1, 1\n\t"
		"stfd 	1,8(1)\n\t"

		"lwz 	3,8(1)\n\t" 
		"stw	3, %[ofpr1]\n\t"
		"lwz 	3,12(1)\n\t" 
		"stw	3, %[ofpr2]\n\t"

		:
		 [ofpr1]"=m" (ofpr1)
		,[ofpr2]"=m" (ofpr2)
		:
		 [ifpr11]"m" (ifpr11)
		,[ifpr12]"m" (ifpr12)

	);
	if(ofpr1 != 0x56cc62b2)
		DolphinMode = true;
	else
		DolphinMode = false;
		
	ModeChecked = true;
	return DolphinMode;
}

bool hw_checked = false;
bool on_hw = false;
bool Sys_HW_Access(void)
{
	if(hw_checked == true)
		return on_hw;

	on_hw = AHBPROT_Patched() && (!Sys_DolphinMode() && !neek2o());
	hw_checked = true;
	return on_hw;
}

/* KILL IT */
s32 __IOS_LoadStartupIOS() { return 0; }
