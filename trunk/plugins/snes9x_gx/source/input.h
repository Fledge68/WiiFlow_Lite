/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May-June 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * input.h
 *
 * Wii/Gamecube controller management
 ***************************************************************************/

#ifndef _INPUT_H_
#define _INPUT_H_

#include <gccore.h>
#include <wiiuse/wpad.h>

#define PI 				3.14159265f
#define PADCAL			50
#define WUPCCAL			400
#define MAXJP 			12 // # of mappable controller buttons

extern u32 btnmap[4][4][12];
extern int rumbleRequest[4];

void ResetControls(int cc = -1, int wc = -1);
void ShutoffRumble();
void DoRumble(int i);
void ReportButtons ();
void SetControllers ();
void SetDefaultButtonMap ();
bool MenuRequested();
void SetupPads();
void UpdatePads();

#endif
