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

u8 set_wip_list(WIP_Code *list, int size);
void free_wip();
void do_wip_code(u8 *dst, u32 len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__WIP_H__
