#ifndef __WIP_H__
#define __WIP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
	u32 offset;
	u32 srcaddress;
	u32 dstaddress;
} WIP_Code;

u32 get_wip_count();
WIP_Code *get_wip_list();
int load_wip_patches(u8 *dir, u8 *gameid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__WIP_H__
