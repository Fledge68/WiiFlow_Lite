#ifdef __cplusplus
extern "C"
{
#endif

#ifndef GC_H_
#define GC_H_

// DIOS-MIOS
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
	DML_CFG_BOOT_DISC	= (1<<10),
	DML_CFG_BOOT_DOL	= (1<<11),
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
	DML_VID_PROG_PATCH	= (1<<4),
};

void DML_New_SetOptions(const char *GamePath, char *CheatPath, char *NewCheatPath, bool cheats, bool debugger, u8 NMM, u8 nodisc, u8 DMLvideoMode, u8 videoSetting);
void DML_Old_SetOptions(char *GamePath, char *CheatPath, char *NewCheatPath, bool cheats);
void DML_New_SetBootDiscOption();
void DML_New_WriteOptions();


// Devolution
typedef struct global_config
{
	u32 signature;			//0x3EF9DB23
	u16 version;			//0x00000100
	u16 device_signature;
	u32 memcard_cluster;
	u32 disc1_cluster;
	u32 disc2_cluster;
} gconfig;

bool DEVO_Installed(const char* path);
void DEVO_SetOptions(const char* path, const char *partition, const char *loader);
void DEVO_Boot();


// General
void GC_SetVideoMode(u8 videomode, u8 videoSetting);
void GC_SetLanguage(u8 lang);
int GC_GameIsInstalled(char *discid, const char* partition, const char* dmlgamedir);

#endif //GC_H_

#ifdef __cplusplus
}
#endif