#ifndef __NK_H__
#define __NK_H__

enum ExtNANDCfg
{
	NCON_EXT_DI_PATH		= (1<<0),
	NCON_EXT_NAND_PATH		= (1<<1),
};

typedef struct _memcfg
{
	u32 magic;
	u64 titleid;
	u32 config;
	u32 paddinga;
	u32 paddingb;
	u32 paddingc;
	u32 paddingd;
	char dipath[256];
	char nandpath[256];
} memcfg;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

s32 Launch_nk(u64 TitleID, const char *nandpath);
bool Load_Neek2o_Kernel();
bool neek2o(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__NK_H__
