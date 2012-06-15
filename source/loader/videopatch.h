#ifndef _VIDEOPATCH_H_
#define _VIDEOPATCH_H_

#include <gccore.h>

void patchVideoModes(void *dst, u32 len, int vidMode, GXRModeObj *vmode, int patchVidModes); 


#endif // !defined(_VIDEOPATCH_H_)
