#ifndef _ALT_IOS_H_
#define _ALT_IOS_H_

#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif

bool loadIOS(int ios, bool launch_game);
u32 get_ios_base();

extern int mainIOS;

#ifdef __cplusplus
}
#endif

#endif
