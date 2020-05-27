
// A simple wrapper for libsicksaxis, to make it resemble WPAD/PAD more closely.
// Written by daxtsu/thedax. I'm releasing this code into the public domain, so do whatever you want with it.

#include <sicksaxis.h>
#include "sicksaxis-wrapper.h"

static DS3 Controller1;
static bool psPressed = false;

bool DS3_Init()// does not check for and connect controller
{
	USB_Initialize();
	ss_init();// add controller to ios heap
	ss_initialize(&Controller1);// set all values to initial states
	return true;
}

void DS3_Rumble()
{
	//if (DS3_Connected())
	if (Controller1.connected && psPressed)
	{
		ss_set_rumble(&Controller1, 2, 255, 2, 255);
	}
}

void DS3_Cleanup()
{
	psPressed = false;
	ss_close(&Controller1);
	USB_Deinitialize();
}

unsigned int DS3_ButtonsDown()
{
	//if (!DS3_Connected())
	if (!ss_is_connected(&Controller1) || !psPressed)
		return 0;

	DS3 *controller;
	controller = &Controller1;

	unsigned int pressed = 0;

	pressed |= controller->pad.buttons.PS ? DS3_BUTTON_PS : 0;
	pressed |= controller->pad.buttons.start ? DS3_BUTTON_START : 0;
	pressed |= controller->pad.buttons.select ? DS3_BUTTON_SELECT : 0;
	pressed |= controller->pad.buttons.triangle ? DS3_BUTTON_TRIANGLE : 0;
	pressed |= controller->pad.buttons.circle ? DS3_BUTTON_CIRCLE : 0;
	pressed |= controller->pad.buttons.cross ? DS3_BUTTON_CROSS : 0;
	pressed |= controller->pad.buttons.square ? DS3_BUTTON_SQUARE : 0;
	pressed |= controller->pad.buttons.up ? DS3_BUTTON_UP : 0;
	pressed |= controller->pad.buttons.right ? DS3_BUTTON_RIGHT : 0;
	pressed |= controller->pad.buttons.down ? DS3_BUTTON_DOWN : 0;
	pressed |= controller->pad.buttons.left ? DS3_BUTTON_LEFT : 0;
	pressed |= controller->pad.buttons.L1 ? DS3_BUTTON_L1 : 0;
	pressed |= controller->pad.buttons.L2 ? DS3_BUTTON_L2 : 0;
	pressed |= controller->pad.buttons.L3 ? DS3_BUTTON_L3 : 0;
	pressed |= controller->pad.buttons.R1 ? DS3_BUTTON_R1 : 0;
	pressed |= controller->pad.buttons.R2 ? DS3_BUTTON_R2 : 0;
	pressed |= controller->pad.buttons.R3 ? DS3_BUTTON_R3 : 0;

	return pressed;
}

bool DS3_Connected()// not used but could be used in the functions ButtonsDown() and Rumble() above
{
	return Controller1.connected > 0 && psPressed;
}

/* does not scan pads. simply connects with the controller and starts a thread to read the controller. */
/* on later calls it simply checks if the PS button on the controller has been pressed and lights up the LED */
void DS3_ScanPads()
{
	if (!ss_is_connected(&Controller1))
	{
		psPressed = false;
		ss_initialize(&Controller1);
		if (ss_open(&Controller1) > 0)
		{
			ss_start_reading(&Controller1);
			ss_set_led(&Controller1, 0);
		}
	}
	else if (Controller1.pad.buttons.PS && !psPressed)
	{
		psPressed = true;
		ss_set_led(&Controller1, 1);
	}
}

int DS3_LStickX()
{
	return psPressed? Controller1.pad.left_analog.x - 128 : 0;
}

int DS3_RStickX()
{
	return psPressed? Controller1.pad.right_analog.x - 128 : 0;
}

int DS3_LStickY()
{
	return psPressed? Controller1.pad.left_analog.y - 128 : 0;
}

int DS3_RStickY()
{
	return psPressed? Controller1.pad.right_analog.y - 128 : 0;
}
