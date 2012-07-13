#ifndef _WDVD_H_
#define _WDVD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
s32 WDVD_Init(void);
s32 WDVD_Close(void);
s32 WDVD_GetHandle(void);
s32 WDVD_Reset(void);
s32 WDVD_ReadDiskId(void *);
s32 WDVD_Seek(u64);
s32 WDVD_Offset(u64);
s32 WDVD_StopLaser(void);
s32 WDVD_StopMotor(void);
s32 WDVD_OpenPartition(u64 offset);
s32 WDVD_ClosePartition(void);
s32 WDVD_UnencryptedRead(void *, u32, u64);
s32 WDVD_Read(void *, u32, u64);
s32 WDVD_LowRequestError(u32 *error);
s32 WDVD_WaitForDisc(void);
s32 WDVD_GetCoverStatus(u32 *);
s32 WDVD_SetUSBMode(u32, const u8 *, s32);
s32 WDVD_Eject(void);
s32 WDVD_Read_Disc_BCA(void *);
s32 WDVD_SetFragList(int device, void *fraglist, int size);
s32 WDVD_SetStreaming(void);
s32 WDVD_NEEK_LoadDisc(u32 id, u32 magic);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

