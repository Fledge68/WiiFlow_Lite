/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * michniewski August 2008
 * Tantric 2008-2010
 *
 * button_mapping.c
 *
 * Controller button mapping
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include "button_mapping.h"

/****************************************************************************
 * Controller Button Descriptions:
 * used for identifying which buttons have been pressed when configuring
 * and for displaying the name of said button
 ***************************************************************************/

CtrlrMap ctrlr_def[4] = {
// Gamecube controller btn def
{
	CTRLR_GCPAD,
	13,
	{
		{PAD_BUTTON_DOWN, "DOWN"},
		{PAD_BUTTON_UP, "UP"},
		{PAD_BUTTON_LEFT, "LEFT"},
		{PAD_BUTTON_RIGHT, "RIGHT"},
		{PAD_BUTTON_A, "A"},
		{PAD_BUTTON_B, "B"},
		{PAD_BUTTON_X, "X"},
		{PAD_BUTTON_Y, "Y"},
		{PAD_BUTTON_MENU, "START"},
		{PAD_BUTTON_START, "START"},
		{PAD_TRIGGER_L, "L"},
		{PAD_TRIGGER_R, "R"},
		{PAD_TRIGGER_Z, "Z"},
		{0, ""},
		{0, ""}
	}
},
// Wiimote btn def
{
	CTRLR_WIIMOTE,
	11,
	{
		{WPAD_BUTTON_DOWN, "DOWN"},
		{WPAD_BUTTON_UP, "UP"},
		{WPAD_BUTTON_LEFT, "LEFT"},
		{WPAD_BUTTON_RIGHT, "RIGHT"},
		{WPAD_BUTTON_A, "A"},
		{WPAD_BUTTON_B, "B"},
		{WPAD_BUTTON_1, "1"},
		{WPAD_BUTTON_2, "2"},
		{WPAD_BUTTON_PLUS, "PLUS"},
		{WPAD_BUTTON_MINUS, "MINUS"},
		{WPAD_BUTTON_HOME, "HOME"},
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""}
	}
},
// Nunchuk btn def
{
	CTRLR_NUNCHUK,
	13,
	{
		{WPAD_BUTTON_DOWN, "DOWN"},
		{WPAD_BUTTON_UP, "UP"},
		{WPAD_BUTTON_LEFT, "LEFT"},
		{WPAD_BUTTON_RIGHT, "RIGHT"},
		{WPAD_BUTTON_A, "A"},
		{WPAD_BUTTON_B, "B"},
		{WPAD_BUTTON_1, "1"},
		{WPAD_BUTTON_2, "2"},
		{WPAD_BUTTON_PLUS, "PLUS"},
		{WPAD_BUTTON_MINUS, "MINUS"},
		{WPAD_BUTTON_HOME, "HOME"},
		{WPAD_NUNCHUK_BUTTON_Z, "Z"},
		{WPAD_NUNCHUK_BUTTON_C, "C"},
		{0, ""},
		{0, ""}
	}
},
// Classic btn def
{
	CTRLR_CLASSIC,
	15,
	{
		{WPAD_CLASSIC_BUTTON_DOWN, "DOWN"},
		{WPAD_CLASSIC_BUTTON_UP, "UP"},
		{WPAD_CLASSIC_BUTTON_LEFT, "LEFT"},
		{WPAD_CLASSIC_BUTTON_RIGHT, "RIGHT"},
		{WPAD_CLASSIC_BUTTON_A, "A"},
		{WPAD_CLASSIC_BUTTON_B, "B"},
		{WPAD_CLASSIC_BUTTON_X, "X"},
		{WPAD_CLASSIC_BUTTON_Y, "Y"},
		{WPAD_CLASSIC_BUTTON_PLUS, "PLUS"},
		{WPAD_CLASSIC_BUTTON_MINUS, "MINUS"},
		{WPAD_CLASSIC_BUTTON_HOME, "HOME"},
		{WPAD_CLASSIC_BUTTON_FULL_L, "L"},
		{WPAD_CLASSIC_BUTTON_FULL_R, "R"},
		{WPAD_CLASSIC_BUTTON_ZL, "ZL"},
		{WPAD_CLASSIC_BUTTON_ZR, "ZR"}
	}
}
};
