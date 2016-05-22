/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May-June 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * input.cpp
 *
 * Wii/Gamecube controller management
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <wupc/wupc.h>
#include <ogc/lwp_watchdog.h>

#include "snes9xgx.h"
#include "button_mapping.h"
#include "menu.h"
#include "video.h"
#include "input.h"
#include "gui/gui.h"

#include "snes9x/snes9x.h"
#include "snes9x/memmap.h"
#include "snes9x/controls.h"

int rumbleRequest[4] = {0,0,0,0};
GuiTrigger userInput[4];

#ifdef HW_RVL
static int rumbleCount[4] = {0,0,0,0};
#endif

// hold superscope/mouse/justifier cursor positions
static int cursor_x[5] = {0,0,0,0,0};
static int cursor_y[5] = {0,0,0,0,0};

/****************************************************************************
 * Controller Functions
 *
 * The following map the Wii controls to the Snes9x controller system
 ***************************************************************************/
#define ASSIGN_BUTTON_TRUE( keycode, snescmd ) \
	  S9xMapButton( keycode, cmd = S9xGetCommandT(snescmd), true)

#define ASSIGN_BUTTON_FALSE( keycode, snescmd ) \
	  S9xMapButton( keycode, cmd = S9xGetCommandT(snescmd), false)

static int scopeTurbo = 0; // tracks whether superscope turbo is on or off
u32 btnmap[4][4][12]; // button mapping

void ResetControls(int consoleCtrl, int wiiCtrl)
{
	int i;
	/*** Gamecube controller Padmap ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_GCPAD))
	{
		i=0;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_X;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_Y;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_TRIGGER_L;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_TRIGGER_R;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_START;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_TRIGGER_Z;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_UP;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_DOWN;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_LEFT;
		btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_RIGHT;
	}

	/*** Wiimote Padmap ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_WIIMOTE))
	{
		i=0;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_B;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_2;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_1;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_A;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = 0x0000;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = 0x0000;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_PLUS;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_MINUS;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_RIGHT;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_LEFT;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_UP;
		btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_DOWN;
	}

	/*** Classic Controller Padmap ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_CLASSIC))
	{
		i=0;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_A;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_B;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_X;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_Y;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_FULL_L;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_FULL_R;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_PLUS;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_MINUS;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_UP;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_DOWN;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_LEFT;
		btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_RIGHT;
	}
		
	/*** Nunchuk + wiimote Padmap ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_NUNCHUK))
	{
		i=0;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_A;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_B;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_NUNCHUK_BUTTON_C;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_NUNCHUK_BUTTON_Z;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_2;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_1;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_PLUS;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_MINUS;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_UP;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_DOWN;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_LEFT;
		btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_RIGHT;
	}

	/*** Superscope : GC controller button mapping ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_SCOPE && wiiCtrl == CTRLR_GCPAD))
	{
		i=0;
		btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
		btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
		btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_TRIGGER_Z;
		btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_Y;
		btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_X;
		btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_START;
	}

	/*** Superscope : wiimote button mapping ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_SCOPE && wiiCtrl == CTRLR_WIIMOTE))
	{
		i=0;
		btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_B;
		btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_A;
		btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_MINUS;
		btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_UP;
		btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_DOWN;
		btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_PLUS;
	}

	/*** Mouse : GC controller button mapping ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_MOUSE && wiiCtrl == CTRLR_GCPAD))
	{
		i=0;
		btnmap[CTRL_MOUSE][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
		btnmap[CTRL_MOUSE][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
	}

	/*** Mouse : wiimote button mapping ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_MOUSE && wiiCtrl == CTRLR_WIIMOTE))
	{
		i=0;
		btnmap[CTRL_MOUSE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_A;
		btnmap[CTRL_MOUSE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_B;
	}

	/*** Justifier : GC controller button mapping ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_JUST && wiiCtrl == CTRLR_GCPAD))
	{
		i=0;
		btnmap[CTRL_JUST][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
		btnmap[CTRL_JUST][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
		btnmap[CTRL_JUST][CTRLR_GCPAD][i++] = PAD_BUTTON_START;
	}

	/*** Justifier : wiimote button mapping ***/
	if(consoleCtrl == -1 || (consoleCtrl == CTRL_JUST && wiiCtrl == CTRLR_WIIMOTE))
	{
		i=0;
		btnmap[CTRL_JUST][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_B;
		btnmap[CTRL_JUST][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_A;
		btnmap[CTRL_JUST][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_PLUS;
	}
}

/****************************************************************************
 * UpdatePads
 *
 * Scans pad and wpad
 ***************************************************************************/

void
UpdatePads()
{
	#ifdef HW_RVL
	WUPC_UpdateButtonStats();
	WPAD_ScanPads();
	#endif

	PAD_ScanPads();

	for(int i=3; i >= 0; i--)
	{
		userInput[i].pad.btns_d = PAD_ButtonsDown(i);
		userInput[i].pad.btns_u = PAD_ButtonsUp(i);
		userInput[i].pad.btns_h = PAD_ButtonsHeld(i);
		userInput[i].pad.stickX = PAD_StickX(i);
		userInput[i].pad.stickY = PAD_StickY(i);
		userInput[i].pad.substickX = PAD_SubStickX(i);
		userInput[i].pad.substickY = PAD_SubStickY(i);
		userInput[i].pad.triggerL = PAD_TriggerL(i);
		userInput[i].pad.triggerR = PAD_TriggerR(i);
		#ifdef HW_RVL
		userInput[i].wupcdata.btns_d = WUPC_ButtonsDown(i);
		userInput[i].wupcdata.btns_u = WUPC_ButtonsUp(i);
		userInput[i].wupcdata.btns_h = WUPC_ButtonsHeld(i);
		userInput[i].wupcdata.stickX = WUPC_lStickX(i);
		userInput[i].wupcdata.stickY = WUPC_lStickY(i);
		userInput[i].wupcdata.substickX = WUPC_rStickX(i);
		userInput[i].wupcdata.substickY = WUPC_rStickY(i);
		#endif
	}
}

/****************************************************************************
 * SetupPads
 *
 * Sets up userInput triggers for use
 ***************************************************************************/
void
SetupPads()
{
	PAD_Init();

	#ifdef HW_RVL
	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
	#endif

	for(int i=0; i < 4; i++)
	{
		userInput[i].chan = i;
		#ifdef HW_RVL
		userInput[i].wpad = WPAD_Data(i);
		#endif
	}
}

#ifdef HW_RVL
/****************************************************************************
 * ShutoffRumble
 ***************************************************************************/
void ShutoffRumble()
{
	if(CONF_GetPadMotorMode() == 0)
		return;

	for(int i=0;i<4;i++)
	{
		WPAD_Rumble(i, 0);
		rumbleCount[i] = 0;
		rumbleRequest[i] = 0;
	}
}

/****************************************************************************
 * DoRumble
 ***************************************************************************/
void DoRumble(int i)
{
	if(CONF_GetPadMotorMode() == 0 || !GCSettings.Rumble) return;

	if(rumbleRequest[i] && rumbleCount[i] < 3)
	{
		WPAD_Rumble(i, 1); // rumble on
		rumbleCount[i]++;
	}
	else if(rumbleRequest[i])
	{
		rumbleCount[i] = 12;
		rumbleRequest[i] = 0;
	}
	else
	{
		if(rumbleCount[i])
			rumbleCount[i]--;
		WPAD_Rumble(i, 0); // rumble off
	}
}
#endif

/****************************************************************************
 * UpdateCursorPosition
 *
 * Updates X/Y coordinates for Superscope/mouse/justifier position
 ***************************************************************************/
static void UpdateCursorPosition (int chan, int &pos_x, int &pos_y)
{
	#define SCOPEPADCAL 20

	#define WUPCSCOPEPADCAL 160
	// gc left joystick

	if (userInput[chan].pad.stickX > SCOPEPADCAL)
	{
		pos_x += (userInput[chan].pad.stickX*1.0)/SCOPEPADCAL;
		if (pos_x > 256) pos_x = 256;
	}
	if (userInput[chan].pad.stickX < -SCOPEPADCAL)
	{
		pos_x -= (userInput[chan].pad.stickX*-1.0)/SCOPEPADCAL;
		if (pos_x < 0) pos_x = 0;
	}

	if (userInput[chan].pad.stickY < -SCOPEPADCAL)
	{
		pos_y += (userInput[chan].pad.stickY*-1.0)/SCOPEPADCAL;
		if (pos_y > 224) pos_y = 224;
	}
	if (userInput[chan].pad.stickY > SCOPEPADCAL)
	{
		pos_y -= (userInput[chan].pad.stickY*1.0)/SCOPEPADCAL;
		if (pos_y < 0) pos_y = 0;
	}

#ifdef HW_RVL
	if (userInput[chan].wpad->ir.valid)
	{
		pos_x = (userInput[chan].wpad->ir.x * 256) / 640;
		pos_y = (userInput[chan].wpad->ir.y * 224) / 480;
	}
	else
	{
		s8 wm_ax = userInput[chan].WPAD_StickX(0);
		s8 wm_ay = userInput[chan].WPAD_StickY(0);

		if (wm_ax > SCOPEPADCAL)
		{
			pos_x += (wm_ax*1.0)/SCOPEPADCAL;
			if (pos_x > 256) pos_x = 256;
		}
		if (wm_ax < -SCOPEPADCAL)
		{
			pos_x -= (wm_ax*-1.0)/SCOPEPADCAL;
			if (pos_x < 0) pos_x = 0;
		}

		if (wm_ay < -SCOPEPADCAL)
		{
			pos_y += (wm_ay*-1.0)/SCOPEPADCAL;
			if (pos_y > 224) pos_y = 224;
		}
		if (wm_ay > SCOPEPADCAL)
		{
			pos_y -= (wm_ay*1.0)/SCOPEPADCAL;
			if (pos_y < 0) pos_y = 0;
		}
	
		/* WiiU Pro Controller */
		s8 wupc_ax = userInput[chan].wupcdata.stickX;
		s8 wupc_ay = userInput[chan].wupcdata.stickX;

		if (wupc_ax > WUPCSCOPEPADCAL)
		{
			pos_x += (wupc_ax*1.0)/WUPCSCOPEPADCAL;
			if (pos_x > 256) pos_x = 256;
		}
		if (wupc_ax < -WUPCSCOPEPADCAL)
		{
			pos_x -= (wupc_ax*-1.0)/WUPCSCOPEPADCAL;
			if (pos_x < 0) pos_x = 0;
		}

		if (wupc_ay < -WUPCSCOPEPADCAL)
		{
			pos_y += (wupc_ay*-1.0)/WUPCSCOPEPADCAL;
			if (pos_y > 224) pos_y = 224;
		}
		if (wupc_ay > WUPCSCOPEPADCAL)
		{
			pos_y -= (wupc_ay*1.0)/WUPCSCOPEPADCAL;
			if (pos_y < 0) pos_y = 0;
		}	
	}
#endif

}

/****************************************************************************
 * decodepad
 *
 * Reads the changes (buttons pressed, etc) from a controller and reports
 * these changes to Snes9x
 ***************************************************************************/
static void decodepad (int chan)
{
	int i, offset;
	double angle;
	static const double THRES = 0.38268343236508984; // cos(67.5)

	s8 pad_x = userInput[chan].pad.stickX;
	s8 pad_y = userInput[chan].pad.stickY;
	u32 jp = userInput[chan].pad.btns_h;

#ifdef HW_RVL
	s8 wm_ax = userInput[chan].WPAD_StickX(0);
	s8 wm_ay = userInput[chan].WPAD_StickY(0);
	u32 wp = userInput[chan].wpad->btns_h;

	u32 exp_type;
	if ( WPAD_Probe(chan, &exp_type) != 0 )
		exp_type = WPAD_EXP_NONE;

	s16 wupc_ax = userInput[chan].wupcdata.stickX;
	s16 wupc_ay = userInput[chan].wupcdata.stickY;
	u32 wupcp = userInput[chan].wupcdata.btns_h;
#endif

	/***
	Gamecube Joystick input
	***/
	// Is XY inside the "zone"?
	if (pad_x * pad_x + pad_y * pad_y > PADCAL * PADCAL)
	{
		angle = atan2(pad_y, pad_x);
 
		if(cos(angle) > THRES)
			jp |= PAD_BUTTON_RIGHT;
		else if(cos(angle) < -THRES)
			jp |= PAD_BUTTON_LEFT;
		if(sin(angle) > THRES)
			jp |= PAD_BUTTON_UP;
		else if(sin(angle) < -THRES)
			jp |= PAD_BUTTON_DOWN;
	}

	// Count as pressed if down far enough (~50% down)
	if (userInput[chan].pad.triggerL > 0x80)
		jp |= PAD_TRIGGER_L;
	if (userInput[chan].pad.triggerR > 0x80)
		jp |= PAD_TRIGGER_R;

#ifdef HW_RVL
	/***
	Wii Joystick (classic, nunchuk) input
	***/
	// Is XY inside the "zone"?
	if (wm_ax * wm_ax + wm_ay * wm_ay > PADCAL * PADCAL)
	{
		angle = atan2(wm_ay, wm_ax);
 
		if(cos(angle) > THRES)
			wp |= (exp_type == WPAD_EXP_CLASSIC) ? WPAD_CLASSIC_BUTTON_RIGHT : WPAD_BUTTON_RIGHT;
		else if(cos(angle) < -THRES)
			wp |= (exp_type == WPAD_EXP_CLASSIC) ? WPAD_CLASSIC_BUTTON_LEFT : WPAD_BUTTON_LEFT;
		if(sin(angle) > THRES)
			wp |= (exp_type == WPAD_EXP_CLASSIC) ? WPAD_CLASSIC_BUTTON_UP : WPAD_BUTTON_UP;
		else if(sin(angle) < -THRES)
			wp |= (exp_type == WPAD_EXP_CLASSIC) ? WPAD_CLASSIC_BUTTON_DOWN : WPAD_BUTTON_DOWN;
	}

	/* Pro Controller */
	if (wupc_ax * wupc_ax + wupc_ay * wupc_ay > WUPCCAL * WUPCCAL)
	{
		angle = atan2(wupc_ay, wupc_ax);
		if(cos(angle) > THRES)
			wupcp |= WPAD_CLASSIC_BUTTON_RIGHT;
		else if(cos(angle) < -THRES)
			wupcp |= WPAD_CLASSIC_BUTTON_LEFT;
		if(sin(angle) > THRES)
			wupcp |= WPAD_CLASSIC_BUTTON_UP;
		else if(sin(angle) < -THRES)
			wupcp |= WPAD_CLASSIC_BUTTON_DOWN;
	}
#endif

	/*** Fix offset to pad ***/
	offset = ((chan + 1) << 4);

	/*** Report pressed buttons (gamepads) ***/
	for (i = 0; i < MAXJP; i++)
    {
		if ( (jp & btnmap[CTRL_PAD][CTRLR_GCPAD][i])											// gamecube controller
#ifdef HW_RVL
		|| ( (exp_type == WPAD_EXP_NONE) && (wp & btnmap[CTRL_PAD][CTRLR_WIIMOTE][i]) )	// wiimote
		|| ( (exp_type == WPAD_EXP_CLASSIC) && (wp & btnmap[CTRL_PAD][CTRLR_CLASSIC][i]) )	// classic controller
		|| ( (exp_type == WPAD_EXP_NUNCHUK) && (wp & btnmap[CTRL_PAD][CTRLR_NUNCHUK][i]) )	// nunchuk + wiimote
		|| ( (wupcp & btnmap[CTRL_PAD][CTRLR_CLASSIC][i]) ) // WiiU Pro Controller
#endif
		)
			S9xReportButton (offset + i, true);
		else
			S9xReportButton (offset + i, false);
    }

	/*** Superscope ***/
	if (Settings.SuperScopeMaster && chan == 0) // report only once
	{
		// buttons
		offset = 0x50;
		for (i = 0; i < 6; i++)
		{
			if (jp & btnmap[CTRL_SCOPE][CTRLR_GCPAD][i]
#ifdef HW_RVL
			|| wp & btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i]
			|| wupcp & btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i]
#endif
			)
			{
				if(i == 3 || i == 4) // turbo
				{
					if((i == 3 && scopeTurbo == 1) || // turbo ON already, don't change
						(i == 4 && scopeTurbo == 0)) // turbo OFF already, don't change
					{
						S9xReportButton(offset + i, false);
					}
					else // turbo changed to ON or OFF
					{
						scopeTurbo = 4-i;
						S9xReportButton(offset + i, true);
					}
				}
				else
					S9xReportButton(offset + i, true);
			}
			else
				S9xReportButton(offset + i, false);
		}
		// pointer
		offset = 0x80;
		UpdateCursorPosition(chan, cursor_x[0], cursor_y[0]);
		S9xReportPointer(offset, (u16) cursor_x[0], (u16) cursor_y[0]);
	}
	/*** Mouse ***/
	else if (Settings.MouseMaster && chan == 0)
	{
		// buttons
		offset = 0x60 + (2 * chan);
		for (i = 0; i < 2; i++)
		{
			if (jp & btnmap[CTRL_MOUSE][CTRLR_GCPAD][i]
#ifdef HW_RVL
			|| wp & btnmap[CTRL_MOUSE][CTRLR_WIIMOTE][i]
			|| wupcp & btnmap[CTRL_MOUSE][CTRLR_WIIMOTE][i]
#endif
			)
				S9xReportButton(offset + i, true);
			else
				S9xReportButton(offset + i, false);
		}
		// pointer
		offset = 0x81;
		UpdateCursorPosition(chan, cursor_x[1 + chan], cursor_y[1 + chan]);
		S9xReportPointer(offset + chan, (u16) cursor_x[1 + chan],
				(u16) cursor_y[1 + chan]);
	}
	/*** Justifier ***/
	else if (Settings.JustifierMaster && chan < 2)
	{
		// buttons
		offset = 0x70 + (3 * chan);
		for (i = 0; i < 3; i++)
		{
			if (jp & btnmap[CTRL_JUST][CTRLR_GCPAD][i]
#ifdef HW_RVL
			|| wp & btnmap[CTRL_JUST][CTRLR_WIIMOTE][i]
			|| wupcp & btnmap[CTRL_JUST][CTRLR_WIIMOTE][i]
#endif
			)
				S9xReportButton(offset + i, true);
			else
				S9xReportButton(offset + i, false);
		}
		// pointer
		offset = 0x83;
		UpdateCursorPosition(chan, cursor_x[3 + chan], cursor_y[3 + chan]);
		S9xReportPointer(offset + chan, (u16) cursor_x[3 + chan],
				(u16) cursor_y[3 + chan]);
	}

#ifdef HW_RVL
	// screenshot (temp)
	if (wp & CLASSIC_CTRL_BUTTON_ZR)
		S9xReportButton(0x90, true);
	else
		S9xReportButton(0x90, false);
#endif
}

bool MenuRequested()
{
	for(int i=0; i<4; i++)
	{
		if (
			(userInput[i].pad.substickX < -70) ||
			(userInput[i].pad.btns_h & PAD_TRIGGER_L &&
			userInput[i].pad.btns_h & PAD_TRIGGER_R &&
			userInput[i].pad.btns_h & PAD_BUTTON_X &&
			userInput[i].pad.btns_h & PAD_BUTTON_Y
			)
			#ifdef HW_RVL
			|| (userInput[i].wpad->btns_h & WPAD_BUTTON_HOME) ||
			(userInput[i].wpad->btns_h & WPAD_CLASSIC_BUTTON_HOME) ||
			(userInput[i].wupcdata.btns_h & WPAD_CLASSIC_BUTTON_HOME)
			#endif
		)
		{
			return true;
		}
	}
	return false;
}

/****************************************************************************
 * ReportButtons
 *
 * Called on each rendered frame
 * Our way of putting controller input into Snes9x
 ***************************************************************************/
void ReportButtons ()
{
	int i, j;

	UpdatePads();

	Settings.TurboMode = (
		userInput[0].pad.substickX > 70 ||
		userInput[0].WPAD_StickX(1) > 70 ||
		userInput[0].wupcdata.substickX > 560
	);	// RIGHT on c-stick and on classic controller right joystick

	/* Check for menu:
	 * CStick left
	 * OR "L+R+X+Y" (eg. Homebrew/Adapted SNES controllers)
	 * OR "Home" on the wiimote or classic controller
	 * OR Left on classic right analog stick
	 */
	if(MenuRequested())
		ScreenshotRequested = 1; // go to the menu

	j = (Settings.MultiPlayer5Master == true ? 4 : 2);

	for (i = 0; i < j; i++)
		decodepad (i);
}

void SetControllers()
{
	if (Settings.MultiPlayer5Master == true)
	{
		S9xSetController (0, CTL_JOYPAD, 0, 0, 0, 0);
		S9xSetController (1, CTL_MP5, 1, 2, 3, -1);
	}
	else if (Settings.SuperScopeMaster == true)
	{
		S9xSetController (0, CTL_JOYPAD, 0, 0, 0, 0);
		S9xSetController (1, CTL_SUPERSCOPE, 0, 0, 0, 0);
	}
	else if (Settings.MouseMaster == true)
	{
		S9xSetController (0, CTL_MOUSE, 0, 0, 0, 0);
		S9xSetController (1, CTL_JOYPAD, 1, 0, 0, 0);
	}
	else if (Settings.JustifierMaster == true)
	{
		S9xSetController (0, CTL_JOYPAD, 0, 0, 0, 0);
		S9xSetController(1, CTL_JUSTIFIER, 1, 0, 0, 0);
	}
	else
	{
		// Plugin 2 Joypads by default
		S9xSetController (0, CTL_JOYPAD, 0, 0, 0, 0);
		S9xSetController (1, CTL_JOYPAD, 1, 0, 0, 0);
	}
}

/****************************************************************************
 * Set the default mapping
 ***************************************************************************/
void SetDefaultButtonMap ()
{
	int maxcode = 0x10;
	s9xcommand_t cmd;

	/*** Joypad 1 ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 A");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 B");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 X");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Y");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Select");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Up");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Down");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Left");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Right");

	maxcode = 0x20;
	/*** Joypad 2 ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 A");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 B");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 X");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Y");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Select");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Up");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Down");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Left");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Right");

	maxcode = 0x30;
	/*** Joypad 3 ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 A");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 B");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 X");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Y");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Select");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Up");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Down");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Left");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Right");

	maxcode = 0x40;
	/*** Joypad 4 ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 A");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 B");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 X");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Y");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Select");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Up");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Down");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Left");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Right");

	maxcode = 0x50;
	/*** Superscope ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope Fire");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope AimOffscreen");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope Cursor");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope ToggleTurbo");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope ToggleTurbo");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope Pause");

	maxcode = 0x60;
	/*** Mouse ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Mouse1 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Mouse1 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Mouse2 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Mouse2 R");

	maxcode = 0x70;
	/*** Justifier ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier1 Trigger");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier1 AimOffscreen");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier1 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier2 Trigger");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier2 AimOffscreen");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier2 Start");

	maxcode = 0x80;
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Superscope"), false);
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Mouse1"), false);
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Mouse2"), false);
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Justifier1"), false);
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Justifier2"), false);

	maxcode = 0x90;
	//ASSIGN_BUTTON_FALSE (maxcode++, "Screenshot");

	SetControllers();
}
