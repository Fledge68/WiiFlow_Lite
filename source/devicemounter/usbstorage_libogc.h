#ifndef __USBSTORAGE_LIBOGC_H__
#define __USBSTORAGE_LIBOGC_H__

#if defined(HW_RVL)

#include <gctypes.h>
#include <ogc/mutex.h>
#include <ogc/disc_io.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

s32 USBStorage_OGC_Initialize();

s32 USBStorage_OGC_Open(usbstorage_handle *dev, s32 device_id, u16 vid, u16 pid);
s32 USBStorage_OGC_Close(usbstorage_handle *dev);
s32 USBStorage_OGC_Reset(usbstorage_handle *dev);

s32 USBStorage_OGC_GetMaxLUN(usbstorage_handle *dev);
s32 USBStorage_OGC_MountLUN(usbstorage_handle *dev, u8 lun);
s32 USBStorage_OGC_Suspend(usbstorage_handle *dev);

s32 USBStorage_OGC_ReadCapacity(usbstorage_handle *dev, u8 lun, u32 *sector_size, u32 *n_sectors);
s32 USBStorage_OGC_Read(usbstorage_handle *dev, u8 lun, u32 sector, u16 n_sectors, u8 *buffer);
s32 USBStorage_OGC_Write(usbstorage_handle *dev, u8 lun, u32 sector, u16 n_sectors, const u8 *buffer);
s32 USBStorage_OGC_StartStop(usbstorage_handle *dev, u8 lun, u8 lo_ej, u8 start, u8 imm);

extern DISC_INTERFACE __io_usbstorage_ogc;

u32 USB_OGC_GetCapacity(u32 *numSectors, u32 *sectorSize);

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif /* HW_RVL */

#endif /* __USBSTORAGE_H__ */
