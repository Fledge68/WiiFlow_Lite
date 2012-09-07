#ifndef _CHAN_LAUNCHER
#define _CHAN_LAUNCHER

#include <gctypes.h>
#include <gccore.h>

s32 BootChannel(u64 chantitle, u32 ios, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, int aspectRatio);

u32 LoadChannel(u8 *buffer);
void PatchChannel(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio);

u8 *GetDol(u64 title, u32 bootcontent);

bool Identify(u64 titleid);
bool Identify_GenerateTik(signed_blob **outbuf, u32 *outlen);

#endif	/* _CHAN_LAUNCHER */
