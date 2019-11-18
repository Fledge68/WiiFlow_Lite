
#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__

#define NIN_CFG_VERSION		0x00000009
#define NIN_CFG_MAXPAD 4
#define MEM_CARD_MAX (5)
#define MEM_CARD_CODE(x) (1<<(x+2))
#define MEM_CARD_SIZE(x) (1<<(x+19))
#define MEM_CARD_BLOCKS(x) ((1<<(x+6))-5)

// nin_cfg version does not match nintendont version
typedef struct NIN_CFG
{
	unsigned int		Magicbytes;		// 0x01070CF6
	unsigned int		Version;		// v3 since rev135, v4 since v3.354, v5 since v3.358, v6 since v3.368, v7 since v4.424, v8 since v4.431, v9 since v6.487
	unsigned int		Config;
	unsigned int		VideoMode;
	unsigned int		Language;
	char	GamePath[255];
	char	CheatPath[255];
	unsigned int		MaxPads;
	unsigned int		GameID;
	unsigned char		MemCardBlocks;
	signed char			VideoScale; // 40 to 120 or 0 for auto, is added to 600 for 640 to 720
	signed char			VideoOffset;// -20 to 20 or 0 for center
	unsigned char		NetworkProfile;
} NIN_CFG;

// NIN_CFG_REMLIMIT is disc read speed limt enabled by default. It keeps loading speed at GC speed.
// disabling it may speed up loading times but cause other game issues.
enum ninconfig
{
	NIN_CFG_CHEATS			= (1<<0),
	NIN_CFG_DEBUGGER		= (1<<1),	// Only for Wii Version
	NIN_CFG_DEBUGWAIT		= (1<<2),	// Only for Wii Version
	NIN_CFG_MEMCARDEMU		= (1<<3),
	NIN_CFG_CHEAT_PATH		= (1<<4),
	NIN_CFG_FORCE_WIDE		= (1<<5),
	NIN_CFG_FORCE_PROG		= (1<<6),
	NIN_CFG_AUTO_BOOT		= (1<<7),
	NIN_CFG_HID				= (1<<8),	// Unused since v3.304
	NIN_CFG_REMLIMIT		= (1<<8),	// v3.358 cfg version 5
	NIN_CFG_OSREPORT		= (1<<9),
	NIN_CFG_USB				= (1<<10),	// r40
	NIN_CFG_LED				= (1<<11),	// v1.45
	NIN_CFG_LOG				= (1<<12),	// v1.109
	NIN_CFG_MC_MULTI		= (1<<13),	// v1.135
	NIN_CFG_NATIVE_SI		= (1<<14),	// v2.189 - only for Wii
	NIN_CFG_WIIU_WIDE		= (1<<15),	// v2.258 - only for Wii U
	NIN_CFG_ARCADE_MODE 	= (1<<16),	// v4.424 cfg version 7
	NIN_CFG_BIT_CC_RUMBLE 	= (1<<17),	// v4.431 cfg version 8
	NIN_CFG_SKIP_IPL 		= (1<<18),	// v4.435 cfg version 8
	NIN_CFG_BBA_EMU			= (1<<19), // v6.487 cfg version 9
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

	NIN_VID_PROG		= (1<<4),	
	NIN_VID_PATCH_PAL50 = (1<<5),	// v3.368 cfg version 6	
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


#endif
