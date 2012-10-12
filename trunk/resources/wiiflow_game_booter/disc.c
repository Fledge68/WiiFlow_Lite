#include "types.h"
#include "memory.h"
#include "utils.h"
#include "frag.h"
#include "cache.h"
#include "di.h"
#include "cios.h"
void Disc_SetLowMem()
{
	/* Setup low memory */
	*Sys_Magic			= 0x0D15EA5E; // Standard Boot Code
	*Sys_Version		= 0x00000001; // Version
	*Arena_L			= 0x00000000; // Arena Low
	*BI2				= 0x817E5480; // BI2
	*Bus_Speed			= 0x0E7BE2C0; // Console Bus Speed
	*CPU_Speed			= 0x2B73A840; // Console CPU Speed
	*Assembler			= 0x38A00040; // Assembler
	*(vu32*)0x800000E4	= 0x80431A80;
	*Dev_Debugger		= 0x81800000; // Dev Debugger Monitor Address
	*Simulated_Mem		= 0x01800000; // Simulated Memory Size
	*(vu32*)0xCD00643C	= 0x00000000; // 32Mhz on Bus

	/* Fix for Sam & Max (WiiPower) */
	if(CurrentIOS.Type != IOS_TYPE_HERMES)
		*GameID_Address	= 0x80000000;

	/* Copy disc ID */
	memcpy((void *)Online_Check, (void *)Disc_ID, 4);

	/* ERROR 002 fix (WiiPower) */
	*(u32 *)0x80003140 = *(u32 *)0x80003188;

}

s32 wbfsDev = 0;
u32 wbfs_part_idx = 0;
FragList *frag_list = NULL;
static int set_frag_list()
{
	// (+1 for header which is same size as fragment)
	int size = sizeof(Fragment) * (frag_list->num + 1);
	return WDVD_SetFragList(wbfsDev, frag_list, size);
}

s32 Disc_SetUSB(const u8 *id, u8 frag)
{
	//ENABLE USB in cIOS
	if(id)
	{
		if(frag)
			return set_frag_list();
		s32 part = -1;
		if(CurrentIOS.Type == IOS_TYPE_HERMES)
			part = wbfs_part_idx ? wbfs_part_idx - 1 : 0;
		return WDVD_SetUSBMode(wbfsDev, (u8*)id, part);
	}
	//DISABLE USB in cIOS
	return WDVD_SetUSBMode(0, NULL, -1);
}
