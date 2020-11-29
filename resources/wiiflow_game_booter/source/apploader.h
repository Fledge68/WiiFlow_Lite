#ifndef _APPLOADER_H_
#define _APPLOADER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
u32 Apploader_Run(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio, u32 returnTo, 
					bool patchregion, u8 private_server, bool patchFix480p, u8 bootType);

enum
{
	PRIVSERV_OFF,
	PRIVSERV_NOSSL,
	PRIVSERV_WIIMMFI,
	PRIVSERV_MAX_CHOICE
	
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
