// A simple wrapper for libsicksaxis, to make it resemble WPAD/PAD more closely.
// Written by daxtsu/thedax. I'm releasing this code into the public domain, so do whatever you want with it.

#ifndef _DS3WRAPPER_H_
#define _DS3WRAPPER_H_

//#include <gctypes.h>

struct ss_device;

enum
{
	DS3_BUTTON_PS = 1,
	DS3_BUTTON_START = 2,
	DS3_BUTTON_SELECT = 4,
	DS3_BUTTON_TRIANGLE = 8,
	DS3_BUTTON_CIRCLE = 16,
	DS3_BUTTON_CROSS = 32,
	DS3_BUTTON_SQUARE = 64,
	DS3_BUTTON_UP = 128,
	DS3_BUTTON_RIGHT = 256,
	DS3_BUTTON_DOWN = 512,
	DS3_BUTTON_LEFT = 1024,
	DS3_BUTTON_L1 = 2048,
	DS3_BUTTON_L2 = 4096,
	DS3_BUTTON_L3 = 8192,
	DS3_BUTTON_R1 = 16384,
	DS3_BUTTON_R2 = 32768,
	DS3_BUTTON_R3 = 65536,
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct ss_device DS3;
bool DS3_Init();
void DS3_Rumble();
void DS3_Cleanup();
unsigned int DS3_ButtonsDown();
void DS3_ScanPads();
int DS3_LStickX();
int DS3_RStickX();
int DS3_LStickY();
int DS3_RStickY();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
