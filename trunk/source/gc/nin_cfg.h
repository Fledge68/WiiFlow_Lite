
#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__

#define NIN_CFG_VERSION		0x00000003
#define OLD_NIN_CFG_VERSION	0x00000002
#define NIN_CFG_MAXPAD 4
#define MEM_CARD_MAX (5)
#define MEM_CARD_CODE(x) (1<<(x+2))
#define MEM_CARD_SIZE(x) (1<<(x+19))
#define MEM_CARD_BLOCKS(x) ((1<<(x+6))-5)

typedef struct NIN_CFG
{
	unsigned int		Magicbytes;		// 0x01070CF6
	unsigned int		Version;		// 0x00000001
	unsigned int		Config;
	unsigned int		VideoMode;
	unsigned int		Language;
	char	GamePath[255];
	char	CheatPath[255];
	unsigned int		MaxPads;
	unsigned int		GameID;
	unsigned int		MemCardBlocks;
} NIN_CFG;

enum ninconfig
{
	NIN_CFG_CHEATS		= (1<<0),
	NIN_CFG_DEBUGGER	= (1<<1),	// Only for Wii Version
	NIN_CFG_DEBUGWAIT	= (1<<2),	// Only for Wii Version
	NIN_CFG_MEMCARDEMU	= (1<<3),
	NIN_CFG_CHEAT_PATH	= (1<<4),
	NIN_CFG_FORCE_WIDE	= (1<<5),
	NIN_CFG_FORCE_PROG	= (1<<6),
	NIN_CFG_AUTO_BOOT	= (1<<7),
	NIN_CFG_HID			= (1<<8),
	NIN_CFG_OSREPORT	= (1<<9),
	NIN_CFG_USB			= (1<<10),
	NIN_CFG_LED			= (1<<11),
	NIN_CFG_LOG			= (1<<12),
	NIN_CFG_MC_MULTI 	= (1<<13),
	NIN_CFG_NATIVE_SI	= (1<<14),
	NIN_CFG_WIIU_WIDE	= (1<<15),
};

enum ninvideomode
{
	NIN_VID_AUTO		= (0<<16),
	NIN_VID_FORCE		= (1<<16),
	NIN_VID_NONE		= (2<<16),
	NIN_VID_FORCE_DF	= (4<<16),

	NIN_VID_MASK		= NIN_VID_AUTO|NIN_VID_FORCE|NIN_VID_NONE|NIN_VID_FORCE_DF,

	NIN_VID_FORCE_PAL50	= (1<<0),
	NIN_VID_FORCE_PAL60	= (1<<1),
	NIN_VID_FORCE_NTSC	= (1<<2),
	NIN_VID_FORCE_MPAL	= (1<<3),

	NIN_VID_FORCE_MASK	= NIN_VID_FORCE_PAL50|NIN_VID_FORCE_PAL60|NIN_VID_FORCE_NTSC|NIN_VID_FORCE_MPAL,

	NIN_VID_PROG		= (1<<4),	//important to prevent blackscreens
};

enum ninlanguage
{
	NIN_LAN_ENGLISH		= 0,
	NIN_LAN_GERMAN		= 1,
	NIN_LAN_FRENCH		= 2,
	NIN_LAN_SPANISH		= 3,
	NIN_LAN_ITALIAN		= 4,
	NIN_LAN_DUTCH		= 5,

/* Auto will use English for E/P region codes and 
   only other languages when these region codes are used: D/F/S/I/J  */

	NIN_LAN_AUTO		= -1, 
};

enum VideoModes
{
	GCVideoModeNone		= 0,
	GCVideoModePAL60	= 1,
	GCVideoModeNTSC		= 2,
	GCVideoModePROG		= 3,
};

#endif
