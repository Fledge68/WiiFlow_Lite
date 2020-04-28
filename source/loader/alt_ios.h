#ifndef _ALT_IOS_H_
#define _ALT_IOS_H_

#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif

bool loadIOS(int ios, bool MountDevices);
void load_ehc_module_ex(void);
void load_dip_249();

#ifdef __cplusplus
}
#endif

#endif
