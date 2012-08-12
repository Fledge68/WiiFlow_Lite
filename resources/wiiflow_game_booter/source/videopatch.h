#ifndef _VIDEOPATCH_H_
#define _VIDEOPATCH_H_

#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif

void patchVideoModes(void *dst, u32 len, int vidMode, GXRModeObj *vmode, int patchVidModes); 

#ifdef __cplusplus
}
#endif

#endif // !defined(_VIDEOPATCH_H_)
