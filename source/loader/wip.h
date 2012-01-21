#ifndef __WIP_H__
#define __WIP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void wip_reset_counter();
void free_wip();
void do_wip_code(u8 * dst, u32 len);
void load_wip_patches(u8 *wippath, u8 *discid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__WIP_H__
