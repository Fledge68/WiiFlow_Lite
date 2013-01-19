#ifndef __MEMORY_H_
#define __MEMORY_H_

#define	 Disc_ID		((vu32*)0x80000000)
#define	 Disc_Region	((vu32*)0x80000003)
#define	 Disc_Magic		((vu32*)0x80000018)
#define	 Sys_Magic		((vu32*)0x80000020)
#define	 Sys_Version	((vu32*)0x80000024)
#define	 Mem_Size		((vu32*)0x80000028)
#define	 Board_Model	((vu32*)0x8000002C)
#define	 Arena_L		((vu32*)0x80000030)
#define	 Arena_H		((vu32*)0x80000034)
#define	 FST			((vu32*)0x80000038)
#define	 Max_FST		((vu32*)0x8000003C)
#define	 Assembler		((vu32*)0x80000060)
#define	 Video_Mode		((vu32*)0x800000CC)
#define  OS_Thread		((vu32*)0x800000E4)
#define	 Dev_Debugger	((vu32*)0x800000EC)
#define	 Simulated_Mem	((vu32*)0x800000F0)
#define	 BI2			((vu32*)0x800000F4)
#define	 Bus_Speed		((vu32*)0x800000F8)
#define	 CPU_Speed		((vu32*)0x800000FC)
#define  Current_IOS	((vu32*)0x80003140)
#define	 Online_Check	((vu32*)0x80003180)
#define	 GameID_Address	((vu32*)0x80003184)
#define  Apploader_IOS	((vu32*)0x80003188)

#define Priiloader_CFG1	((vu32*)0x8132FFFB)
#define Priiloader_CFG2	((vu32*)0x817FEFF0)

#define HW_GPIO_OUT		((vu32*)0xCD8000E0)
#define HW_GPIOB_OUT	((vu32*)0xCD8000C0)
#define HW_AHBPROT		((vu32*)0xCD800064)
#define MEM_PROT		((vu32)0xCD8B420A)

#endif
