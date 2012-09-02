#ifndef _USBSTORAGE2_H_
#define _USBSTORAGE2_H_

#include "ogc/disc_io.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Disc interfaces */
extern const DISC_INTERFACE __io_usbstorage2_port0;
extern const DISC_INTERFACE __io_usbstorage2_port1;

/* Prototypes */
s32 USBStorage2_Init(u32 port);
void USBStorage2_Deinit();
s32 USBStorage2_GetCapacity(u32 port, u32 *size);

s32 USBStorage2_ReadSectors(u32 port, u32 sector, u32 numSectors, void *buffer);
s32 USBStorage2_WriteSectors(u32 port, u32 sector, u32 numSectors, const void *buffer);
s32 USBStorage2_GetSectorSize();

s32 USBStorage2_SetPort(s8 port);
s8 USBStorage2_GetPort();

extern int usb_libogc_mode;

#define DEVICE_TYPE_WII_UMS (('W'<<24)|('U'<<16)|('M'<<8)|'S')

#ifdef __cplusplus
}
#endif

#endif
