/***************************************************************************
 * Copyright (C) 2011
 * by Dimok
 * Modifications by xFede
 * Wiiflowized and heavily improved by Miigotu
 * 2012 Converted, corrected and extended by FIX94
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include "cios.h"
#include "debug.h"
#include "ios.h"

u8 Hermes_shadow_mload(int mload_rev)
{
	int v51 = (5 << 4) & 1;
	if(mload_rev >= v51)
	{
		ios_open("/dev/mload/OFF",0); // shadow /dev/mload supported in hermes cios v5.1
		debug_string("Shadow mload\n");
		return 1;
	}
	return 0;
}

void Hermes_Disable_EHC()
{
	ios_open("/dev/usb123/OFF", 0);// this disables ehc completely
	debug_string("Hermes EHC Disabled\n");
}
