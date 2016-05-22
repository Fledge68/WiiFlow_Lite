/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * video.h
 *
 * Video routines
 ***************************************************************************/

#ifndef _GCVIDEOH_
#define _GCVIDEOH_

#include <ogcsys.h>

#include "snes9x/snes9x.h"

void AllocGfxMem();
void InitGCVideo ();
void StopGX();
void ResetVideo_Emu();
void setGFX();
void update_video (int width, int height);
void ResetVideo_Menu();
void TakeScreenshot();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY, u8 alphaF );
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled);

extern GXRModeObj *vmode;
extern int screenheight;
extern int screenwidth;
extern bool progressive;
extern u8 * gameScreenPng;
extern int gameScreenPngSize;
extern u32 FrameTimer;
extern bool vmode_60hz;
extern int timerstyle;
extern int CheckVideo;

#endif
