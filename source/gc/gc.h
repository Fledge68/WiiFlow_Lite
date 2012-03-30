#ifdef __cplusplus
extern "C"
{
#endif

#ifndef GC_H_
#define GC_H_

typedef struct DML_CFG
{
	u32 Magicbytes;			//0xD1050CF6
	u32 CfgVersion;			//0x00000001
	u32 VideoMode;
	u32 Config;
	char GamePath[255];
	char CheatPath[255];
} DML_CFG;

enum dmlconfig
{
	DML_CFG_CHEATS		= (1<<0),
	DML_CFG_DEBUGGER	= (1<<1),
	DML_CFG_DEBUGWAIT	= (1<<2),
	DML_CFG_NMM			= (1<<3),
	DML_CFG_NMM_DEBUG	= (1<<4),
	DML_CFG_GAME_PATH	= (1<<5),
	DML_CFG_CHEAT_PATH	= (1<<6),
	DML_CFG_ACTIVITY_LED= (1<<7),
	DML_CFG_PADHOOK		= (1<<8),
	DML_CFG_NODISC		= (1<<9),
};

enum dmlvideomode
{
	DML_VID_DML_AUTO	= (0<<16),
	DML_VID_FORCE		= (1<<16),
	DML_VID_NONE		= (2<<16),

	DML_VID_FORCE_PAL50	= (1<<0),
	DML_VID_FORCE_PAL60	= (1<<1),
	DML_VID_FORCE_NTSC	= (1<<2),
	DML_VID_FORCE_PROG	= (1<<3),
};

void GC_SetVideoMode(u8 videomode);
void GC_SetLanguage(u8 lang);
bool GC_GameIsInstalled(char *discid, const char* partition, const char* dmlgamedir);
void DML_New_SetOptions(char *GamePath, char *CheatPath, char *NewCheatPath, bool cheats, bool debugger, u8 NMM, u8 nodisc); //, u8 DMLvideoMode);
void DML_Old_SetOptions(char *GamePath, char *CheatPath, char *NewCheatPath, bool cheats);
#endif //GC_H_

#ifdef __cplusplus
}
#endif