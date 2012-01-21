#ifndef _GCARD_H_
#define _GCARD_H_

#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

u8 register_card_provider(const char *url, const char *key);
u8 has_enabled_providers();
void add_game_to_card(const char *gameid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_GCARD_H_