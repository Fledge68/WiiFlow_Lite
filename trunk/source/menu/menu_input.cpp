#include <stdlib.h>
#include <ogc\lwp_watchdog.h>

#include "menu.hpp"
#include "sicksaxis-wrapper/sicksaxis-wrapper.h"

static const u32 g_repeatDelay = 25;
u64 button_time = 0;

#define CheckTime()		(ticks_to_millisecs(diff_ticks((button_time), gettick())) > 200)
#define UpdateTime()	button_time = gettick()

void CMenu::SetupInput(bool reset_pos)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(reset_pos)
		{
			m_show_pointer[chan] = false;
			stickPointer_x[chan] = (m_vid.width() + m_cursor[chan].width())/2;
			stickPointer_y[chan] = (m_vid.height() + m_cursor[chan].height())/2;
		}
		left_stick_angle[chan] = 0;
		left_stick_mag[chan] = 0;
		right_stick_angle[chan] = 0;
		right_stick_mag[chan] = 0;
		right_stick_skip[chan] = 0;
		wmote_roll[chan] = 0;
		wmote_roll_skip[chan] = 0;
	}
	enable_wmote_roll = m_cfg.getBool("GENERAL", "wiimote_gestures", false);
}

static int CalculateRepeatSpeed(float magnitude, int current_value)
{
	if(magnitude < 0)
		magnitude *= -1;

	// Calculate frameskips based on magnitude
	// Max frameskips is 50 (1 sec, or slightly less)
	if(magnitude < 0.15f)
	{
		return -1; // Force a direct start
	}
	else if(current_value > 0)
	{
		return current_value - 1; // Wait another frame
	}
	else if(current_value == -1)
	{
		return 0; // Process the input
	}
	else
	{
		s32 frames = 50 - ((u32) (50.f * magnitude)); // Calculate the amount of frames to wait
		return (frames < 0) ? 0 : frames;
	}
}

void CMenu::ScanInput()
{
	m_show_zone_main = false;
	m_show_zone_main2 = false;
	m_show_zone_main3 = false;
	m_show_zone_prev = false;
	m_show_zone_next = false;

	WUPC_UpdateButtonStats();
	WPAD_ScanPads();
	PAD_ScanPads();
	DS3_ScanPads();
	
	ButtonsPressed();
	ButtonsHeld();
	LeftStick();

	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		wd[chan] = WPAD_Data(chan);
		left_stick_angle[chan] = 0;
		left_stick_mag[chan] = 0;
		right_stick_angle[chan] = 0;
		right_stick_mag[chan] = 0;
		switch(wd[chan]->exp.type)
		{
			case WPAD_EXP_NUNCHUK:
				right_stick_mag[chan] = wd[chan]->exp.nunchuk.js.mag;
				right_stick_angle[chan] = wd[chan]->exp.nunchuk.js.ang;
				break;
			case WPAD_EXP_GUITARHERO3:
				left_stick_mag[chan] = wd[chan]->exp.nunchuk.js.mag;
				left_stick_angle[chan] = wd[chan]->exp.nunchuk.js.ang;
				break;
			case WPAD_EXP_CLASSIC:
				left_stick_mag[chan] = wd[chan]->exp.classic.ljs.mag;
				left_stick_angle[chan] = wd[chan]->exp.classic.ljs.ang;
				right_stick_mag[chan] = wd[chan]->exp.classic.rjs.mag;
				right_stick_angle[chan] = wd[chan]->exp.classic.rjs.ang;
				break;
			default:
				break;
		}
		if(enable_wmote_roll)
		{
			wmote_roll[chan] = wd[chan]->orient.roll; // Use wd[chan]->ir.angle if you only want this to work when pointing at the screen
			wmote_roll_skip[chan] = CalculateRepeatSpeed(wmote_roll[chan] / 45.f, wmote_roll_skip[chan]);
		}
		right_stick_skip[chan] = CalculateRepeatSpeed(right_stick_mag[chan], right_stick_skip[chan]);
	}
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
        m_btnMgr.setRumble(chan, WPadIR_Valid(chan), PAD_StickX(chan) < -20 || PAD_StickX(chan) > 20 || PAD_StickY(chan) < -20 || PAD_StickY(chan) > 20,
            WUPC_lStickX(chan) < -160 || WUPC_lStickX(chan) > 160 || WUPC_lStickY(chan) < -160 || WUPC_lStickY(chan) > 160);
        m_btnMgr.setMouse(WPadIR_Valid(chan) || m_show_pointer[chan]);
		if(WPadIR_Valid(chan))
		{
			m_cursor[chan].draw(wd[chan]->ir.x, wd[chan]->ir.y, wd[chan]->ir.angle);
			m_btnMgr.mouse(chan, wd[chan]->ir.x - m_cursor[chan].width() / 2, wd[chan]->ir.y - m_cursor[chan].height() / 2);
		}
		else if(m_show_pointer[chan])
		{
			m_cursor[chan].draw(stickPointer_x[chan], stickPointer_y[chan], 0);
			m_btnMgr.mouse(chan, stickPointer_x[chan] - m_cursor[chan].width() / 2, stickPointer_y[chan] - m_cursor[chan].height() / 2);
		}
	}
	ShowMainZone();
	ShowMainZone2();
	ShowMainZone3();
	ShowPrevZone();
	ShowNextZone();
	ShowGameZone();
}
extern "C" { extern bool shutdown; };
void CMenu::ButtonsPressed()
{
	
	gc_btnsPressed = 0;
	if (CheckTime())
	{
		ds3_btnsPressed = DS3_ButtonsDown();
		UpdateTime();
	}
	else
		ds3_btnsPressed = 0;
		
	if(ds3_btnsPressed & DBTN_SELECT) shutdown = 1;
	{
		for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		{
			wii_btnsPressed[chan] = WPAD_ButtonsDown(chan);
			gc_btnsPressed |= PAD_ButtonsDown(chan);
			wupc_btnsPressed[chan] = WUPC_ButtonsDown(chan);
		}
	}
}

void CMenu::ButtonsHeld()
{
	gc_btnsHeld = 0;
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		wii_btnsHeld[chan] = WPAD_ButtonsHeld(chan);
		gc_btnsHeld |= PAD_ButtonsHeld(chan);
		wupc_btnsHeld[chan] = WUPC_ButtonsHeld(chan);
	}
}

bool CMenu::wBtn_PressedChan(int btn, u8 ext, int &chan)
{
	return ((wii_btnsPressed[chan] & btn) && (ext == WPAD_EXP_NONE || ext == wd[chan]->exp.type)) || ((wupc_btnsPressed[chan] & btn) && (ext == WPAD_EXP_CLASSIC));
}

bool CMenu::wBtn_Pressed(int btn, u8 ext)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(wBtn_PressedChan(btn, ext, chan))
			return true;
	}
	return false;
}

bool CMenu::wBtn_HeldChan(int btn, u8 ext, int &chan)
{
	return ((wii_btnsHeld[chan] & btn) && (ext == WPAD_EXP_NONE || ext == wd[chan]->exp.type)) || ((wupc_btnsHeld[chan] & btn) && (ext == WPAD_EXP_CLASSIC));
}

bool CMenu::wBtn_Held(int btn, u8 ext)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(wBtn_HeldChan(btn, ext, chan))
			return true;
	}
	return false;
}

void CMenu::LeftStick()
{
	u8 speed = 0, pSpeed = 0;
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(left_stick_mag[chan] > 0.15 || abs(PAD_StickX(chan)) > 20 || abs(PAD_StickY(chan)) > 20 || (chan == 0 && (abs(DS3_StickX()) > 20 || abs(DS3_StickY()) > 20)) || abs(WUPC_lStickX(chan)) > 160 || abs(WUPC_lStickY(chan)) > 160)
		{
			m_show_pointer[chan] = true;
			if(LEFT_STICK_LEFT)
			{
				speed = (u8)(left_stick_mag[chan] * 10.00);
				pSpeed = (u8)((abs(PAD_StickX(chan))/10)|(abs(DS3_StickX()/10))|(abs(WUPC_lStickX(chan))/80));
				if(stickPointer_x[chan] > m_cursor[chan].width()/2)
					stickPointer_x[chan] = stickPointer_x[chan]-speed-pSpeed;
				pointerhidedelay[chan] = 150;
			}
			if(LEFT_STICK_DOWN)
			{
				speed = (u8)(left_stick_mag[chan] * 10.00);
				pSpeed = (u8)((abs(PAD_StickY(chan))/10)|(abs(DS3_StickY()/10))|(abs(WUPC_lStickY(chan))/80));
				if(stickPointer_y[chan] < (m_vid.height() + (m_cursor[chan].height()/2)))
					stickPointer_y[chan] = stickPointer_y[chan]+speed+pSpeed;
				pointerhidedelay[chan] = 150;
			}
			if(LEFT_STICK_RIGHT)
			{
				speed = (u8)(left_stick_mag[chan] * 10.00);
				pSpeed = (u8)((abs(PAD_StickX(chan))/10)|(abs(DS3_StickX()/10))|(abs(WUPC_lStickX(chan))/80));
				if(stickPointer_x[chan] < (m_vid.width() + (m_cursor[chan].width()/2)))
					stickPointer_x[chan] = stickPointer_x[chan]+speed+pSpeed;
				pointerhidedelay[chan] = 150;
			}
			if(LEFT_STICK_UP)
			{
				speed = (u8)(left_stick_mag[chan] * 10.00);
				pSpeed = (u8)((abs(PAD_StickY(chan))/10)|(abs(DS3_StickY()/10))|(abs(WUPC_lStickY(chan))/80));
				if(stickPointer_y[chan] > m_cursor[chan].height()/2)
					stickPointer_y[chan] = stickPointer_y[chan]-speed-pSpeed;
				pointerhidedelay[chan] = 150;
			}
		}
		else
		{
			if(pointerhidedelay[chan] > 0 && !wii_btnsHeld[chan] && !wii_btnsPressed[chan] 
				&& !gc_btnsHeld && !gc_btnsPressed) 
				pointerhidedelay[chan]--;
		}
		if (pointerhidedelay[chan] == 0)
			m_show_pointer[chan] = false;
	}
}

bool CMenu::WPadIR_Valid(int chan)
{
	wd[chan] = WPAD_Data(chan);
	if(wd[chan]->ir.valid)
		return true;
	return false;
}

bool CMenu::ShowPointer(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(WPadIR_Valid(chan) || m_show_pointer[chan])
			return true;
	}
	return false;
}

bool CMenu::WPadIR_ANY(void)
{
	return (wd[0]->ir.valid || wd[1]->ir.valid || wd[2]->ir.valid || wd[3]->ir.valid);
}

bool CMenu::wii_btnRepeat(u8 btn)
{
	bool b = false;

	if(btn == WBTN_UP)
	{
		if(WBTN_UP_HELD)
		{
			if(m_wpadUpDelay == 0 || m_wpadUpDelay >= g_repeatDelay)
				b = true;
			if(m_wpadUpDelay < g_repeatDelay) 
				++m_wpadUpDelay;
		}
		else
			m_wpadUpDelay = 0;
	}
	else if(btn == WBTN_RIGHT)
	{
		if(WBTN_RIGHT_HELD)
		{
			if(m_wpadRightDelay == 0 || m_wpadRightDelay >= g_repeatDelay)
				b = true;
			if(m_wpadRightDelay < g_repeatDelay) 
				++m_wpadRightDelay;
		}
		else
			m_wpadRightDelay = 0;
	}
	else if(btn == WBTN_DOWN)
	{
		if(WBTN_DOWN_HELD)
		{
			if(m_wpadDownDelay == 0 || m_wpadDownDelay >= g_repeatDelay)
				b = true;
			if(m_wpadDownDelay < g_repeatDelay) 
				++m_wpadDownDelay;
		}
		else
			m_wpadDownDelay = 0;
	}
	else if(btn == WBTN_LEFT || DBTN_LEFT)
	{
		if(WBTN_LEFT_HELD || DBTN_LEFT_PRESSED)
		{
			if(m_wpadLeftDelay == 0 || m_wpadLeftDelay >= g_repeatDelay)
				b = true;
			if(m_wpadLeftDelay < g_repeatDelay) 
				++m_wpadLeftDelay;
		}
		else
			m_wpadLeftDelay = 0;
	}
	else if(btn == WBTN_A)
	{
		if(WBTN_A_HELD)
		{
			m_btnMgr.noClick(true);
			if(m_wpadADelay == 0 || m_wpadADelay >= g_repeatDelay)
				b = true;
			if(m_wpadADelay < g_repeatDelay) 
				++m_wpadADelay;
		}
		else
		{
			m_wpadADelay = 0;
			m_btnMgr.noClick();
		}
	}
	return b;
}

bool CMenu::gc_btnRepeat(s64 btn)
{
	bool b = false;
	if(btn == GBTN_UP)
	{
		if(gc_btnsHeld & GBTN_UP)
		{
			if(m_padUpDelay == 0 || m_padUpDelay >= g_repeatDelay)
				b = true;
			if(m_padUpDelay < g_repeatDelay)
				++m_padUpDelay;
		}
		else
			m_padUpDelay = 0;
	}
	else if(btn == GBTN_RIGHT)
	{
		if(gc_btnsHeld & GBTN_RIGHT)
		{
			if(m_padRightDelay == 0 || m_padRightDelay >= g_repeatDelay)
				b = true;
			if(m_padRightDelay < g_repeatDelay)
				++m_padRightDelay;
		}
		else
			m_padRightDelay = 0;
	}
	else if(btn == GBTN_DOWN)
	{
		if(gc_btnsHeld & GBTN_DOWN)
		{
			if(m_padDownDelay == 0 || m_padDownDelay >= g_repeatDelay)
				b = true;
			if(m_padDownDelay < g_repeatDelay)
				++m_padDownDelay;
		}
		else
			m_padDownDelay = 0;
	}
	else if(btn == GBTN_LEFT)
	{
		if(gc_btnsHeld & GBTN_LEFT)
		{
			if(m_padLeftDelay == 0 || m_padLeftDelay >= g_repeatDelay)
				b = true;
			if(m_padLeftDelay < g_repeatDelay)
				++m_padLeftDelay;
		}
		else
			m_padLeftDelay = 0;
	}
	else if(btn == GBTN_A)
	{
		if(gc_btnsHeld & GBTN_A)
		{
			m_btnMgr.noClick(true);
			if(m_padADelay == 0 || m_padADelay >= g_repeatDelay)
				b = true;
			if(m_padADelay < g_repeatDelay)
				++m_padADelay;
		}
		else
		{
			m_padADelay = 0;
			m_btnMgr.noClick();
		}
	}
	return b;
}

bool CMenu::ds3_btnRepeat(s64 btn)
{
	bool b = false;
	if(btn == DBTN_UP)
	{
		if(ds3_btnsPressed & DBTN_UP)
		{
			if(m_dpadUpDelay == 0 || m_dpadUpDelay >= g_repeatDelay)
				b = true;
			if(m_dpadUpDelay < g_repeatDelay)
				++m_dpadUpDelay;
		}
		else
		{
			m_dpadUpDelay = 0;
			m_btnMgr.noClick();
		}
	}
	else if(btn == DBTN_RIGHT)
	{
		if(ds3_btnsPressed & DBTN_RIGHT)
		{
			if(m_dpadRightDelay == 0 || m_dpadRightDelay >= g_repeatDelay)
				b = true;
			if(m_dpadRightDelay < g_repeatDelay)
				++m_dpadRightDelay;
		}
		else
		{
			m_dpadRightDelay = 0;
			m_btnMgr.noClick();
		}
	}
	else if(btn == DBTN_DOWN)
	{
		if(ds3_btnsPressed & DBTN_DOWN)
		{
			if(m_dpadDownDelay == 0 || m_dpadDownDelay >= g_repeatDelay)
				b = true;
			if(m_dpadDownDelay < g_repeatDelay)
				++m_dpadDownDelay;
		}
		else
		{
			m_dpadDownDelay = 0;
			m_btnMgr.noClick();
		}
	}
	else if(btn == DBTN_LEFT)
	{
		if(ds3_btnsPressed & DBTN_LEFT)
		{
			if(m_dpadLeftDelay == 0 || m_dpadLeftDelay >= g_repeatDelay)
				b = true;
			if(m_dpadLeftDelay < g_repeatDelay)
				++m_dpadLeftDelay;
		}
		else
		{
			m_dpadLeftDelay = 0;
			m_btnMgr.noClick();
		}
	}
	else if(btn == DBTN_A)
	{
		if(ds3_btnsPressed & DBTN_A)
		{
			m_btnMgr.noClick(true);
			if(m_dpadADelay == 1 || m_dpadADelay >= g_repeatDelay)
				b = true;
			if(m_dpadADelay < g_repeatDelay)
				++m_dpadADelay;
		}
		else
		{
			m_dpadADelay = 1;
			m_btnMgr.noClick();
		}
	}
	else if(btn == DBTN_START)
	{
		if(ds3_btnsPressed & DBTN_START)
		{
			m_btnMgr.noClick(true);
			if(m_dpadHDelay == 0.5 || m_dpadHDelay >= g_repeatDelay)
				b = true;
			if(m_dpadHDelay < g_repeatDelay)
				++m_dpadHDelay;
		}
		else
		{
			m_dpadHDelay = 0.5;
			m_btnMgr.noClick();
		}
	}        
	return b;
}

bool CMenu::lStick_Up(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if((LEFT_STICK_ANG_UP && left_stick_mag[chan] > 0.15) || PAD_StickY(chan) > 20 || DS3_StickY() < -20 || WUPC_lStickY(chan) > 160)
			return true;
	}
	return false;
}

bool CMenu::lStick_Right(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if((LEFT_STICK_ANG_RIGHT && left_stick_mag[chan] > 0.15) || PAD_StickX(chan) > 20 || DS3_StickX() > 20 || WUPC_lStickX(chan) > 160)
			return true;
	}
	return false;
}

bool CMenu::lStick_Down(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if((LEFT_STICK_ANG_DOWN && left_stick_mag[chan] > 0.15) || PAD_StickY(chan) < -20 || DS3_StickY() > 20 || WUPC_lStickY(chan) < -160)
			return true;
	}
	return false;
}

bool CMenu::lStick_Left(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if((LEFT_STICK_ANG_LEFT && left_stick_mag[chan] > 0.15) || PAD_StickX(chan) < -20 || DS3_StickX() < -20 || WUPC_lStickX(chan) < -160)
			return true;
	}
	return false;
}

bool CMenu::rStick_Up(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if((RIGHT_STICK_ANG_UP && right_stick_mag[chan] > 0.15 && right_stick_skip[chan] == 0) || PAD_SubStickY(chan) > 20 || WUPC_rStickY(chan) > 160)
			return true;
	}
	return false;
}

bool CMenu::rStick_Right(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if((RIGHT_STICK_ANG_RIGHT && right_stick_mag[chan] > 0.15 && right_stick_skip[chan] == 0) || PAD_SubStickX(chan) > 20 || WUPC_rStickX(chan) > 160)
			return true;
	}
	return false;
}

bool CMenu::rStick_Down(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if((RIGHT_STICK_ANG_DOWN && right_stick_mag[chan] > 0.15 && right_stick_skip[chan] == 0) || PAD_SubStickY(chan) < -20 || WUPC_rStickY(chan) < -160)
			return true;
	}
	return false;
}

bool CMenu::rStick_Left(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if((RIGHT_STICK_ANG_LEFT && right_stick_mag[chan] > 0.15 && right_stick_skip[chan] == 0) || PAD_SubStickX(chan) < -20 || WUPC_rStickX(chan) < -160)
			return true;
	}
	return false;
}

bool CMenu::wRoll_Left(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(WBTN_B_HELD && (wmote_roll[chan] < -5) && wmote_roll_skip[chan] == 0)
			return true;
	}
	return false;
}

bool CMenu::wRoll_Right(void)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(WBTN_B_HELD && (wmote_roll[chan] > 5) && wmote_roll_skip[chan] == 0)
			return true;
	}
	return false;
}

void CMenu::ShowZone(SZone zone, bool &showZone)
{
	if(zone.hide)
	{
		showZone = false;
		for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		{
			if((WPadIR_Valid(chan) || m_show_pointer[chan]) && m_cursor[chan].x() >= zone.x && m_cursor[chan].y() >= zone.y
				&& m_cursor[chan].x() < zone.x + zone.w && m_cursor[chan].y() < zone.y + zone.h)
				showZone = true;
		}
	}
	else
		showZone = true;
}

void CMenu::ShowMainZone()
{
	ShowZone(m_mainButtonsZone, m_show_zone_main);
}

void CMenu::ShowMainZone2()
{
	ShowZone(m_mainButtonsZone2, m_show_zone_main2);
}

void CMenu::ShowMainZone3()
{
	ShowZone(m_mainButtonsZone3, m_show_zone_main3);
}

void CMenu::ShowPrevZone()
{
	ShowZone(m_mainPrevZone, m_show_zone_prev);
}

void CMenu::ShowNextZone()
{
	ShowZone(m_mainNextZone, m_show_zone_next);
}

void CMenu::ShowGameZone()
{
	ShowZone(m_gameButtonsZone, m_show_zone_game);
}

u32 CMenu::NoInputTime()
{
	bool input_found = false;
	if(ShowPointer() == true || RIGHT_STICK_MOVE == true || gc_btnsPressed != 0)
		input_found = true;
	else
	{
		for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		{
			if(wii_btnsPressed[chan] != 0 || wii_btnsHeld[chan] != 0)
			{
				input_found = true;
				break;
			}
		}
	}
	if(input_found == false)
	{
		if(no_input_time == 0)
			no_input_time = time(NULL);
		return time(NULL) - no_input_time;
	}
	no_input_time = 0;
	return 0;
}
