
#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
	COVERFLOW_USB = 0,
	COVERFLOW_DML,
	COVERFLOW_CHANNEL,
	COVERFLOW_PLUGIN,
	COVERFLOW_HOMEBREW,
	COVERFLOW_MAX
};

enum
{
	TYPE_WII_DISC = 0,
	TYPE_WII_WBFS,
	TYPE_WII_WBFS_EXT,
};

enum
{
    TYPE_WII_GAME = 0,
    TYPE_GC_GAME,
    TYPE_CHANNEL,
    TYPE_PLUGIN,
    TYPE_HOMEBREW,
    TYPE_END
};
#define NoGameID(x)			(x == TYPE_PLUGIN || x == TYPE_HOMEBREW)

enum
{
	IOS_TYPE_D2X = 0,
	IOS_TYPE_WANIN,
	IOS_TYPE_HERMES,
	IOS_TYPE_KWIIRK,
	IOS_TYPE_NEEK2O,
	IOS_TYPE_NORMAL_IOS,
	IOS_TYPE_STUB,
};
#define CustomIOS(x)		(x != IOS_TYPE_NORMAL_IOS && x != IOS_TYPE_STUB)

#ifdef __cplusplus
}
#endif

#endif