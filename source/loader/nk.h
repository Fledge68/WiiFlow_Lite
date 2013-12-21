#ifndef __NK_H__
#define __NK_H__

enum ExtNANDCfg
{
	NCON_EXT_DI_PATH		= (1<<0),
	NCON_EXT_NAND_PATH		= (1<<1),
	NCON_HIDE_EXT_PATH		= (1<<2),
	NCON_EXT_RETURN_TO		= (1<<3),
};

typedef struct _memcfg
{
	u32 magic;
	u64 titleid;
	u32 config;
	u64 returnto;
	u32 paddinga;
	u32 paddingb;
	char dipath[256];
	char nandpath[256];
} memcfg;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

s32 Launch_nk(u64 TitleID, const char *nandpath, u64 ReturnTo);
bool Load_Neek2o_Kernel();
void check_neek2o(void);
bool neek2o(void);
/*void NKKeyCreate(u8 *TIK);
void NKAESDecryptBlock(u8 *in, u8 *out);*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__NK_H__
