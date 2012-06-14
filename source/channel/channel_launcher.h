#ifndef _CHAN_LAUNCHER
#define _CHAN_LAUNCHER

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gctypes.h>
#include <gccore.h>

s32 BootChannel(u32 entry, u64 chantitle, u32 ios, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, int aspectRatio);

u32 LoadChannel(u8 *buffer);
void PatchChannel(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio);

u8 *GetDol(u64 title, u32 bootcontent);

bool Identify(u64 titleid, u32 *ios);
bool Identify_GenerateTik(signed_blob **outbuf, u32 *outlen);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _CHAN_LAUNCHER */
