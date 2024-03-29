
#ifndef _DISC_H_
#define _DISC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

s32 Disc_Open(u8 type);
s32 Disc_FindPartition(u32 *outbuf);
s32	Disc_SetUSB(const u8 *id, bool frag);
void Disc_SetLowMem();

GXRModeObj *Disc_SelectVMode(u8 videoselected, u32 *rmode_reg);
void Disc_SetVMode(GXRModeObj *rmode, u32 rmode_reg);

//just for frag copy
void copy_frag_list(FragList *src);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
