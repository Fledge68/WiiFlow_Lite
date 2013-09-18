
#define WBTN_UP_PRESSED (wBtn_Pressed(WPAD_BUTTON_UP, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_UP, WPAD_EXP_CLASSIC))
#define WBTN_DOWN_PRESSED (wBtn_Pressed(WPAD_BUTTON_DOWN, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_DOWN, WPAD_EXP_CLASSIC))
#define WBTN_LEFT_PRESSED (wBtn_Pressed(WPAD_BUTTON_LEFT, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_LEFT, WPAD_EXP_CLASSIC))
#define WBTN_RIGHT_PRESSED (wBtn_Pressed(WPAD_BUTTON_RIGHT, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_RIGHT, WPAD_EXP_CLASSIC))
#define WBTN_HOME_PRESSED (wBtn_Pressed(WPAD_BUTTON_HOME, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_HOME, WPAD_EXP_CLASSIC))
#define WBTN_MINUS_PRESSED (wBtn_Pressed(WPAD_BUTTON_MINUS, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_FULL_L, WPAD_EXP_CLASSIC))
#define WBTN_PLUS_PRESSED (wBtn_Pressed(WPAD_BUTTON_PLUS, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_FULL_R, WPAD_EXP_CLASSIC))
#define WBTN_Z_PRESSED (wBtn_Pressed(WPAD_NUNCHUK_BUTTON_Z, WPAD_EXP_NUNCHUK) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_ZR, WPAD_EXP_CLASSIC))
#define WBTN_A_PRESSED (wBtn_Pressed(WPAD_BUTTON_A, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_A, WPAD_EXP_CLASSIC))
#define WBTN_B_PRESSED (wBtn_Pressed(WPAD_BUTTON_B, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_B, WPAD_EXP_CLASSIC))
#define WBTN_1_PRESSED (wBtn_Pressed(WPAD_BUTTON_1, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_Y, WPAD_EXP_CLASSIC))
#define WBTN_2_PRESSED (wBtn_Pressed(WPAD_BUTTON_2, WPAD_EXP_NONE) \
		|| wBtn_Pressed(WPAD_CLASSIC_BUTTON_X, WPAD_EXP_CLASSIC))

#define WBTN_UP_HELD (wBtn_Held(WPAD_BUTTON_UP, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_UP, WPAD_EXP_CLASSIC))
#define WBTN_DOWN_HELD (wBtn_Held(WPAD_BUTTON_DOWN, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_DOWN, WPAD_EXP_CLASSIC))
#define WBTN_LEFT_HELD (wBtn_Held(WPAD_BUTTON_LEFT, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_LEFT, WPAD_EXP_CLASSIC))
#define WBTN_RIGHT_HELD (wBtn_Held(WPAD_BUTTON_RIGHT, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_RIGHT, WPAD_EXP_CLASSIC))
#define WBTN_HOME_HELD (wBtn_Held(WPAD_BUTTON_HOME, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_HOME, WPAD_EXP_CLASSIC))
#define WBTN_MINUS_HELD (wBtn_Held(WPAD_BUTTON_MINUS, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_FULL_L, WPAD_EXP_CLASSIC))
#define WBTN_PLUS_HELD (wBtn_Held(WPAD_BUTTON_PLUS, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_FULL_R, WPAD_EXP_CLASSIC))
#define WBTN_A_HELD (wBtn_Held(WPAD_BUTTON_A, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_A, WPAD_EXP_CLASSIC))
#define WBTN_B_HELD (wBtn_Held(WPAD_BUTTON_B, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_B, WPAD_EXP_CLASSIC))
#define WBTN_1_HELD (wBtn_Held(WPAD_BUTTON_1, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_Y, WPAD_EXP_CLASSIC))
#define WBTN_2_HELD (wBtn_Held(WPAD_BUTTON_2, WPAD_EXP_NONE) \
		|| wBtn_Held(WPAD_CLASSIC_BUTTON_X, WPAD_EXP_CLASSIC))

#define GBTN_UP (PAD_BUTTON_UP)
#define GBTN_DOWN (PAD_BUTTON_DOWN)
#define GBTN_LEFT (PAD_BUTTON_LEFT)
#define GBTN_RIGHT (PAD_BUTTON_RIGHT)
#define GBTN_START (PAD_BUTTON_MENU)
#define GBTN_L (PAD_TRIGGER_L)
#define GBTN_R (PAD_TRIGGER_R)
#define GBTN_Z (PAD_TRIGGER_Z)
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
#define GBTN_Z_PRESSED (gc_btnsPressed & GBTN_Z)
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

enum
{
	WBTN_UP = 0,
	WBTN_DOWN,
	WBTN_LEFT,
	WBTN_RIGHT,
	WBTN_A,
};

#define BTN_UP_REPEAT (wii_btnRepeat(WBTN_UP) || gc_btnRepeat(GBTN_UP))
#define BTN_DOWN_REPEAT (wii_btnRepeat(WBTN_DOWN) || gc_btnRepeat(GBTN_DOWN))
#define BTN_LEFT_REPEAT (wii_btnRepeat(WBTN_LEFT) || gc_btnRepeat(GBTN_LEFT))
#define BTN_RIGHT_REPEAT (wii_btnRepeat(WBTN_RIGHT) || gc_btnRepeat(GBTN_RIGHT))
#define BTN_A_REPEAT (wii_btnRepeat(WBTN_A) || gc_btnRepeat(GBTN_A))

#define LEFT_STICK_UP lStick_Up()
#define LEFT_STICK_DOWN lStick_Down()
#define LEFT_STICK_LEFT lStick_Left()
#define LEFT_STICK_RIGHT lStick_Right()

#define RIGHT_STICK_UP rStick_Up()
#define RIGHT_STICK_DOWN rStick_Down()
#define RIGHT_STICK_LEFT rStick_Left()
#define RIGHT_STICK_RIGHT rStick_Right()
#define RIGHT_STICK_MOVE (RIGHT_STICK_UP || RIGHT_STICK_DOWN \
					|| RIGHT_STICK_LEFT || RIGHT_STICK_RIGHT)
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
