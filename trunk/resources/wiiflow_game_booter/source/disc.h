
#ifndef _DISC_H_
#define _DISC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

s32 Disc_FindPartition(u64 *outbuf);
void Disc_SetLowMem();
void Disc_SetTime();

GXRModeObj *Disc_SelectVMode(u8 videoselected, u32 *rmode_reg);
void Disc_SetVMode(GXRModeObj *rmode, u32 rmode_reg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif