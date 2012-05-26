
#define WBTN_UP (WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP)
#define WBTN_DOWN (WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN)
#define WBTN_LEFT (WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT)
#define WBTN_RIGHT (WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT)
#define WBTN_HOME (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)
#define WBTN_MINUS (WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_FULL_L)
#define WBTN_PLUS (WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_FULL_R)
#define WBTN_A (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)
#define WBTN_B (WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B)
#define WBTN_1 (WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_Y)
#define WBTN_2 (WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_X)

#define WBTN_UP_PRESSED (wii_btnsPressed & WBTN_UP)
#define WBTN_DOWN_PRESSED (wii_btnsPressed & WBTN_DOWN)
#define WBTN_LEFT_PRESSED (wii_btnsPressed & WBTN_LEFT)
#define WBTN_RIGHT_PRESSED (wii_btnsPressed & WBTN_RIGHT)
#define WBTN_HOME_PRESSED (wii_btnsPressed & WBTN_HOME)
#define WBTN_MINUS_PRESSED (wii_btnsPressed & WBTN_MINUS)
#define WBTN_PLUS_PRESSED (wii_btnsPressed & WBTN_PLUS)
#define WBTN_A_PRESSED (wii_btnsPressed & WBTN_A)
#define WBTN_B_PRESSED (wii_btnsPressed & WBTN_B)
#define WBTN_1_PRESSED (wii_btnsPressed & WBTN_1)
#define WBTN_2_PRESSED (wii_btnsPressed & WBTN_2)

#define WBTN_UP_HELD (wii_btnsHeld & WBTN_UP)
#define WBTN_DOWN_HELD (wii_btnsHeld & WBTN_DOWN)
#define WBTN_LEFT_HELD (wii_btnsHeld & WBTN_LEFT)
#define WBTN_RIGHT_HELD (wii_btnsHeld & WBTN_RIGHT)
#define WBTN_HOME_HELD (wii_btnsHeld & WBTN_HOME)
#define WBTN_MINUS_HELD (wii_btnsHeld & WBTN_MINUS)
#define WBTN_PLUS_HELD (wii_btnsHeld & WBTN_PLUS)
#define WBTN_A_HELD (wii_btnsHeld & WBTN_A)
#define WBTN_B_HELD (wii_btnsHeld & WBTN_B)
#define WBTN_1_HELD (wii_btnsHeld & WBTN_1)
#define WBTN_2_HELD (wii_btnsHeld & WBTN_2)

#define GBTN_UP (PAD_BUTTON_UP)
#define GBTN_DOWN (PAD_BUTTON_DOWN)
#define GBTN_LEFT (PAD_BUTTON_LEFT)
#define GBTN_RIGHT (PAD_BUTTON_RIGHT)
#define GBTN_START (PAD_BUTTON_MENU)
#define GBTN_L (PAD_TRIGGER_L)
#define GBTN_R (PAD_TRIGGER_R)
#define GBTN_A (PAD_BUTTON_A)
#define GBTN_B (PAD_BUTTON_B)
#define GBTN_1 (PAD_BUTTON_Y)
#define GBTN_2 (PAD_BUTTON_X)

#define GBTN_UP_PRESSED (gc_btnsPressed & GBTN_UP)
#define GBTN_DOWN_PRESSED (gc_btnsPressed & GBTN_DOWN)
#define GBTN_LEFT_PRESSED (gc_btnsPressed & GBTN_LEFT)
#define GBTN_RIGHT_PRESSED (gc_btnsPressed & GBTN_RIGHT)
#define GBTN_START_PRESSED (gc_btnsPressed & GBTN_START)
#define GBTN_L_PRESSED (gc_btnsPressed & GBTN_L)
#define GBTN_R_PRESSED (gc_btnsPressed & GBTN_R)
#define GBTN_A_PRESSED (gc_btnsPressed & GBTN_A)
#define GBTN_B_PRESSED (gc_btnsPressed & GBTN_B)
#define GBTN_1_PRESSED (gc_btnsPressed & GBTN_1)
#define GBTN_2_PRESSED (gc_btnsPressed & GBTN_2)

#define GBTN_UP_HELD (gc_btnsHeld & GBTN_UP)
#define GBTN_DOWN_HELD (gc_btnsHeld & GBTN_DOWN)
#define GBTN_LEFT_HELD (gc_btnsHeld & GBTN_LEFT)
#define GBTN_RIGHT_HELD (gc_btnsHeld & GBTN_RIGHT)
#define GBTN_START_HELD (gc_btnsHeld & GBTN_START)
#define GBTN_L_HELD (gc_btnsHeld & GBTN_L)
#define GBTN_R_HELD (gc_btnsHeld & GBTN_R)
#define GBTN_A_HELD (gc_btnsHeld & GBTN_A)
#define GBTN_B_HELD (gc_btnsHeld & GBTN_B)
#define GBTN_1_HELD (gc_btnsHeld & GBTN_1)
#define GBTN_2_HELD (gc_btnsHeld & GBTN_2)

#define BTN_UP_PRESSED (WBTN_UP_PRESSED || GBTN_UP_PRESSED)
#define BTN_DOWN_PRESSED (WBTN_DOWN_PRESSED || GBTN_DOWN_PRESSED)
#define BTN_LEFT_PRESSED (WBTN_LEFT_PRESSED || GBTN_LEFT_PRESSED)
#define BTN_RIGHT_PRESSED (WBTN_RIGHT_PRESSED || GBTN_RIGHT_PRESSED)
#define BTN_HOME_PRESSED (WBTN_HOME_PRESSED || GBTN_START_PRESSED)
#define BTN_MINUS_PRESSED (WBTN_MINUS_PRESSED || GBTN_L_PRESSED)
#define BTN_PLUS_PRESSED (WBTN_PLUS_PRESSED || GBTN_R_PRESSED)
#define BTN_A_PRESSED (WBTN_A_PRESSED || GBTN_A_PRESSED)
#define BTN_B_PRESSED (WBTN_B_PRESSED || GBTN_B_PRESSED)
#define BTN_1_PRESSED (WBTN_1_PRESSED || GBTN_1_PRESSED)
#define BTN_2_PRESSED (WBTN_2_PRESSED || GBTN_2_PRESSED)

#define BTN_UP_HELD (WBTN_UP_HELD || GBTN_UP_HELD)
#define BTN_DOWN_HELD (WBTN_DOWN_HELD || GBTN_DOWN_HELD)
#define BTN_LEFT_HELD (WBTN_LEFT_HELD || GBTN_LEFT_HELD)
#define BTN_RIGHT_HELD (WBTN_RIGHT_HELD || GBTN_RIGHT_HELD)
#define BTN_HOME_HELD (WBTN_HOME_HELD || GBTN_START_HELD)
#define BTN_MINUS_HELD (WBTN_MINUS_HELD || GBTN_L_HELD)
#define BTN_PLUS_HELD (WBTN_PLUS_HELD || GBTN_R_HELD)
#define BTN_A_HELD (WBTN_A_HELD || GBTN_A_HELD)
#define BTN_B_HELD (WBTN_B_HELD || GBTN_B_HELD)
#define BTN_1_HELD (WBTN_1_HELD  || GBTN_1_HELD)
#define BTN_2_HELD (WBTN_2_HELD || GBTN_2_HELD)

#define BTN_UP_REPEAT (wii_btnRepeat(WBTN_UP) || gc_btnRepeat(GBTN_UP))
#define BTN_DOWN_REPEAT (wii_btnRepeat(WBTN_DOWN) || gc_btnRepeat(GBTN_DOWN))
#define BTN_LEFT_REPEAT (wii_btnRepeat(WBTN_LEFT) || gc_btnRepeat(GBTN_LEFT))
#define BTN_RIGHT_REPEAT (wii_btnRepeat(WBTN_RIGHT) || gc_btnRepeat(GBTN_RIGHT))
/* #define BTN_MINUS_REPEAT (wii_btnRepeat(WBTN_MINUS) || gc_btnRepeat(GBTN_MINUS))
#define BTN_PLUS_REPEAT (wii_btnRepeat(WBTN_PLUS) || gc_btnRepeat(GBTN_PLUS))
#define BTN_HOME_REPEAT (wii_btnRepeat(WBTN_HOME) || gc_btnRepeat(GBTN_HOME)) */
#define BTN_A_REPEAT (wii_btnRepeat(WBTN_A) || gc_btnRepeat(GBTN_A))
/* #define BTN_B_REPEAT (wii_btnRepeat(WBTN_B) || gc_btnRepeat(GBTN_B))
#define BTN_1_REPEAT (wii_btnRepeat(WBTN_1) || gc_btnRepeat(GBTN_1))
#define BTN_2_REPEAT (wii_btnRepeat(WBTN_2) || gc_btnRepeat(GBTN_2)) */

#define LEFT_STICK_UP lStick_Up()
#define LEFT_STICK_DOWN lStick_Down()
#define LEFT_STICK_LEFT lStick_Left()
#define LEFT_STICK_RIGHT lStick_Right()

#define RIGHT_STICK_UP rStick_Up()
#define RIGHT_STICK_DOWN rStick_Down()
#define RIGHT_STICK_LEFT rStick_Left()
#define RIGHT_STICK_RIGHT rStick_Right()

#define WROLL_LEFT  wRoll_Left()
#define WROLL_RIGHT wRoll_Right()

/* Internal */
#define LEFT_STICK_ANG_UP ((left_stick_angle[chan] >= 300 && left_stick_angle[chan] <= 360) \
		|| (left_stick_angle[chan] >= 0 && left_stick_angle[chan] <= 60))
#define LEFT_STICK_ANG_RIGHT (left_stick_angle[chan] >= 30 && left_stick_angle[chan] <= 150)
#define LEFT_STICK_ANG_DOWN (left_stick_angle[chan] >= 120 && left_stick_angle[chan] <= 240)
#define LEFT_STICK_ANG_LEFT (left_stick_angle[chan] >= 210 && left_stick_angle[chan] <= 330)

#define RIGHT_STICK_ANG_UP ((right_stick_angle[chan] >= 300 && right_stick_angle[chan] <= 360) \
		|| (right_stick_angle[chan] >= 0 && right_stick_angle[chan] <= 60))
#define RIGHT_STICK_ANG_RIGHT (right_stick_angle[chan] >= 30 && right_stick_angle[chan] <= 150)
#define RIGHT_STICK_ANG_DOWN (right_stick_angle[chan] >= 120 && right_stick_angle[chan] <= 240)
#define RIGHT_STICK_ANG_LEFT (right_stick_angle[chan] >= 210 && right_stick_angle[chan] <= 330)

/*
//Button values reference//
WPAD_BUTTON_2							0x0001
PAD_BUTTON_LEFT							0x0001

WPAD_BUTTON_1							0x0002
PAD_BUTTON_RIGHT						0x0002

WPAD_BUTTON_B							0x0004
PAD_BUTTON_DOWN							0x0004

WPAD_BUTTON_A							0x0008
PAD_BUTTON_UP							0x0008

WPAD_BUTTON_MINUS						0x0010
PAD_TRIGGER_Z							0x0010

PAD_TRIGGER_R							0x0020

PAD_TRIGGER_L							0x0040

WPAD_BUTTON_HOME						0x0080

WPAD_BUTTON_LEFT						0x0100
PAD_BUTTON_A							0x0100

WPAD_BUTTON_RIGHT						0x0200
PAD_BUTTON_B							0x0200

WPAD_BUTTON_DOWN						0x0400
PAD_BUTTON_X							0x0400

WPAD_BUTTON_UP							0x0800
PAD_BUTTON_Y							0x0800

WPAD_BUTTON_PLUS						0x1000
PAD_BUTTON_MENU							0x1000
PAD_BUTTON_START						0x1000

WPAD_NUNCHUK_BUTTON_Z					(0x0001<<16)
WPAD_CLASSIC_BUTTON_UP					(0x0001<<16)
WPAD_GUITAR_HERO_3_BUTTON_STRUM_UP		(0x0001<<16)

WPAD_NUNCHUK_BUTTON_C					(0x0002<<16)
WPAD_CLASSIC_BUTTON_LEFT				(0x0002<<16)

WPAD_CLASSIC_BUTTON_ZR					(0x0004<<16)

WPAD_CLASSIC_BUTTON_X					(0x0008<<16)
WPAD_GUITAR_HERO_3_BUTTON_YELLOW		(0x0008<<16)

WPAD_CLASSIC_BUTTON_A					(0x0010<<16)
WPAD_GUITAR_HERO_3_BUTTON_GREEN			(0x0010<<16)

WPAD_CLASSIC_BUTTON_Y					(0x0020<<16)
WPAD_GUITAR_HERO_3_BUTTON_BLUE			(0x0020<<16)

WPAD_CLASSIC_BUTTON_B					(0x0040<<16)
WPAD_GUITAR_HERO_3_BUTTON_RED			(0x0040<<16)

WPAD_CLASSIC_BUTTON_ZL					(0x0080<<16)
WPAD_GUITAR_HERO_3_BUTTON_ORANGE		(0x0080<<16)

WPAD_CLASSIC_BUTTON_FULL_R				(0x0200<<16)

WPAD_CLASSIC_BUTTON_PLUS				(0x0400<<16)
WPAD_GUITAR_HERO_3_BUTTON_PLUS			(0x0400<<16)

WPAD_CLASSIC_BUTTON_HOME				(0x0800<<16)

WPAD_CLASSIC_BUTTON_MINUS				(0x1000<<16)
WPAD_GUITAR_HERO_3_BUTTON_MINUS			(0x1000<<16)

WPAD_CLASSIC_BUTTON_FULL_L				(0x2000<<16)

WPAD_CLASSIC_BUTTON_DOWN				(0x4000<<16)
WPAD_GUITAR_HERO_3_BUTTON_STRUM_DOWN	(0x4000<<16)

WPAD_CLASSIC_BUTTON_RIGHT				(0x8000<<16)
*/
