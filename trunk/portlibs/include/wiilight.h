#ifndef _WIILIGHT_H_
#define _WIILIGHT_H_


#ifdef __cplusplus
extern "C" {
#endif

void WIILIGHT_Init();
void WIILIGHT_TurnOn();
int WIILIGHT_GetLevel();
int WIILIGHT_SetLevel(int level);

void WIILIGHT_Toggle();
void WIILIGHT_TurnOff();

#ifdef __cplusplus
}
#endif

#endif
