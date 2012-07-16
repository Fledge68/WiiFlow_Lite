#ifndef _MLOAD_MODULES_H_
#define _MLOAD_MODULES_H_

#include "mload.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void *ehcmodule;
extern int size_ehcmodule;

extern void *dip_plugin;
extern int size_dip_plugin;

int load_ehc_module();
//int load_fatffs_module(u8 *discid);

void enable_ES_ioctlv_vector(void);
void Set_DIP_BCA_Datas(u8 *bca_data);

u8 *search_for_ehcmodule_cfg(u8 *p, int size);
//void disable_ffs_patch(void);
//int enable_ffs(int mode);

#ifdef __cplusplus
}
#endif

#endif


