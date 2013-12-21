
#ifndef _SYS_H_
#define _SYS_H_

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define HBC_LULZ			0x000100014c554c5aULL
#define HBC_108				0x00010001af1bf516ULL
#define HBC_JODI			0x0001000148415858ULL
#define HBC_HAXX			0x000100014a4f4449ULL
#define RETURN_CHANNEL		0x0001000857494948ULL
#define SYSTEM_MENU			0x0000000100000002ULL

enum
{
	PRIILOADER_DEF = 0,
	EXIT_TO_MENU,
	EXIT_TO_HBC,
	EXIT_TO_PRIILOADER,
	EXIT_TO_DISABLE,
	EXIT_TO_BOOTMII,
	EXIT_TO_WFNK2O,
	EXIT_TO_SMNK2O,
	BUTTON_CALLBACK,
	WIIFLOW_DEF,
};

/* Prototypes */
void Sys_Init(void);
bool Sys_DolphinMode(void);
bool Sys_HW_Access(void);
bool Sys_Exiting(void);
void Sys_Exit(void);
void Sys_ExitTo(int);
int Sys_GetExitTo(void);
void Sys_SetNeekPath(const char*);

void Open_Inputs(void);
void Close_Inputs(void);

bool AHBRPOT_Patched(void);
bool IsOnWiiU(void);

/* All our extern C stuff */
extern void __exception_setreload(int t);
extern int mainIOS;
extern bool useMainIOS;
extern volatile bool NANDemuView;
extern volatile bool networkInit;
extern u8 currentPartition;
extern u8 currentPort;
extern char wii_games_dir[];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
