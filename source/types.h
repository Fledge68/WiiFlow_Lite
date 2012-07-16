
#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    TYPE_WII_GAME = 0,
    TYPE_GC_GAME,
    TYPE_CHANNEL,
    TYPE_PLUGIN,
    TYPE_HOMEBREW,
    TYPE_END
};

enum
{
	IOS_TYPE_D2X = 0,
	IOS_TYPE_WANIN,
	IOS_TYPE_HERMES,
	IOS_TYPE_KWIIRK,
	IOS_TYPE_NO_CIOS,
};

#define NoGameID(x)			(x == TYPE_PLUGIN || x == TYPE_HOMEBREW)

#ifdef __cplusplus
}
#endif

#endif