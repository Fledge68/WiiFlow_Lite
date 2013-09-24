typedef struct NIN_CFG 
{
	u32		Magicbytes;		// 0x01070CF6
	u32		Version;		// 0x00000001
	u32		Config;
	u32		VideoMode;
	u32		Language;		// NYI
	char	GamePath[255];
	char	CheatPath[255];
} NIN_CFG;

enum ninconfig
{
	NIN_CFG_CHEATS		= (1<<0),	// NYI
	NIN_CFG_DEBUGGER	= (1<<1),	// NYI
	NIN_CFG_DEBUGWAIT	= (1<<2),	// NYI
	NIN_CFG_MEMCARDEMU	= (1<<3),
	NIN_CFG_GAME_PATH	= (1<<4),
	NIN_CFG_CHEAT_PATH	= (1<<5),
	NIN_CFG_FORCE_WIDE	= (1<<6),
	NIN_CFG_FORCE_PROG	= (1<<7),
	NIN_CFG_AUTO_BOOT	= (1<<8),
	NIN_CFG_HID			= (1<<9),
	NIN_CFG_OSREPORT	= (1<<10),	// Only for Wii
};

enum ninvideomode
{
	NIN_VID_AUTO		= (0<<16),
	NIN_VID_FORCE		= (1<<16),
	NIN_VID_NONE		= (2<<16),

	NIN_VID_MASK		= NIN_VID_AUTO|NIN_VID_FORCE|NIN_VID_NONE,

	NIN_VID_FORCE_PAL50	= (1<<0),
	NIN_VID_FORCE_PAL60	= (1<<1),
	NIN_VID_FORCE_NTSC	= (1<<2),
	NIN_VID_FORCE_MPAL	= (1<<3),

	NIN_VID_FORCE_MASK	= NIN_VID_FORCE_PAL50|NIN_VID_FORCE_PAL60|NIN_VID_FORCE_NTSC|NIN_VID_FORCE_MPAL,
};
