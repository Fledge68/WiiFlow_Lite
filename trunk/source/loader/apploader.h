#ifndef _APPLOADER_H_
#define _APPLOADER_H_

/* Entry point */
typedef void (*entry_point)(void);

/* Prototypes */
s32 Apploader_Run(entry_point *entry,u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, u32 returnTo);

#endif
