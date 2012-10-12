
#ifndef _DISC_H_
#define _DISC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*s32 Disc_Open();
s32 Disc_FindPartition(u64 *outbuf);*/
s32	Disc_SetUSB(const u8 *id, u8 frag);
void Disc_SetLowMem();
/*
GXRModeObj *Disc_SelectVMode(u8 videoselected, u32 *rmode_reg);
void Disc_SetVMode(GXRModeObj *rmode, u32 rmode_reg);
*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
