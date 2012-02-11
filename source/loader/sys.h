#ifndef _SYS_H_
#define _SYS_H_

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define HBC_108				0x00010001af1bf516ULL
#define HBC_JODI			0x0001000148415858ULL
#define HBC_HAXX			0x000100014a4f4449ULL

#define PRIILOADER_DEF		0
#define EXIT_TO_MENU 		1
#define EXIT_TO_HBC 		2
#define EXIT_TO_PRIILOADER 	3
#define EXIT_TO_DISABLE 	4
#define EXIT_TO_BOOTMII 	5

	/* Prototypes */
	void Sys_Init(void);
	void Sys_LoadMenu(void);
	bool Sys_Exiting(void);
	void Sys_Test(void);
	void Sys_Exit(void);
	void Sys_ExitTo(int);
	
	void Open_Inputs(void);
	void Close_Inputs(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
