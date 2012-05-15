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

// DSPCR bits
#define DSPCR_DSPRESET          0x0800  // Reset DSP
#define DSPCR_DSPINT            0x0080  // * interrupt active (RWC)
#define DSPCR_ARINT                     0x0020
#define DSPCR_AIINT                     0x0008
#define DSPCR_HALT                      0x0004  // halt DSP
#define DSPCR_RES                       0x0001  // reset DSP

#define _SHIFTL(v, s, w)        \
        ((u32) (((u32)(v) & ((0x01 << (w)) - 1)) << (s)))

	/* Prototypes */
	void Sys_Init(void);
	void Sys_LoadMenu(void);
	bool Sys_Exiting(void);
	void Sys_Test(void);
	void Sys_Exit(void);
	void Sys_ExitTo(int);
	void __dsp_shutdown(void);

	void Open_Inputs(void);
	void Close_Inputs(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
