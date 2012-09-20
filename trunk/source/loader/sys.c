
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
#include "loader/nk.h"
#include "gecko/gecko.h"
#include "memory/mem2.hpp"
#include "memory/memory.h"
#include "wiiuse/wpad.h"

/* Variables */
bool reset = false;
bool shutdown = false;
bool exiting = false;
u8 ExitOption = 0;
const char *NeekPath = NULL;

extern void __exception_closeall();

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
	
	WPAD_SetIdleTimeout(60 * 2); // idle after 2 minutes
}

void Close_Inputs(void)
{
	u32 cnt;

	/* Disconnect Wiimotes */
	for(cnt = 0; cnt < 4; cnt++)
		WPAD_Disconnect(cnt);

	/* Shutdown Wiimote subsystem */
	WPAD_Shutdown();
}

bool Sys_Exiting(void)
{
	return reset || shutdown || exiting;
}

void Sys_Shutdown(void)
{
	/* via hollywood registers first */
	*HW_GPIO_OUT |= (1<<1);
	usleep(50000);
	/* If it failed just do the libogc way */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
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
		*Priiloader_CFG1 = 0x50756E65;
		*Priiloader_CFG2 = 0x50756E65;
	}
	else if(ExitOption == EXIT_TO_PRIILOADER)
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
	if(ExitOption == EXIT_TO_DISABLE)
		return;

	/* Shutdown Inputs */
	Close_Inputs();
	WII_Initialize();
	if(ExitOption == EXIT_TO_WFNK2O)
		Launch_nk(0x1000144574641LL, NeekPath, 0);
	else if(ExitOption == EXIT_TO_SMNK2O)
		Launch_nk(0, NeekPath, 0);
	else if(ExitOption == EXIT_TO_BOOTMII)
		IOS_ReloadIOS(0xfe);
	else if(ExitOption == EXIT_TO_HBC)
	{
		WII_LaunchTitle(HBC_108);
		WII_LaunchTitle(HBC_JODI);
		WII_LaunchTitle(HBC_HAXX);
	}
	else if(ExitOption == BUTTON_CALLBACK)
		Sys_Shutdown();
	/* else Return to Menu */
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

bool AHBRPOT_Patched(void)
{
	return (*HW_AHBPROT == 0xFFFFFFFF);
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
	{
		gprintf("Dolphin-Emu\n");
		DolphinMode = true;
	}
	else
	{
		gprintf("Real Wii\n");
		DolphinMode = false;
	}
	ModeChecked = true;
	return DolphinMode;
}
