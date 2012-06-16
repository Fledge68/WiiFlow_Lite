
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

#define NoGameID(x)			(x == TYPE_PLUGIN || x == TYPE_HOMEBREW)

#ifdef __cplusplus
}
#endif

#endif