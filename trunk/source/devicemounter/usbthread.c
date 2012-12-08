/****************************************************************************
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <ogc/cache.h>
#include <ogc/lwp.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "usbthread.h"
#include "usbstorage.h"
#include "gecko/gecko.hpp"

lwp_t USB_Thread = LWP_THREAD_NULL;
volatile bool CheckUSB = false;
volatile bool idle = false;
volatile time_t start = 0;
u8 sector[4096];

void *KeepUSBAlive(void *nothing)
{
	int NumberSectors = USBStorage2_GetCapacity(0, NULL);
	start = time(NULL);
	srand(start);
	while(CheckUSB)
	{
		if(idle || (time(NULL) - start) > 19)
		{
			USBStorage2_ReadSectors(0, rand() % NumberSectors, 1, sector);
			idle = true;
		}
		sleep(1);
	}
	return nothing;
}

void CreateUSBKeepAliveThread()
{
	CheckUSB = true;
	LWP_CreateThread(&USB_Thread, KeepUSBAlive, NULL, NULL, 0, 40);
}

void KillUSBKeepAliveThread()
{
	CheckUSB = false;
	USBKeepAliveThreadReset();
	if(USB_Thread != LWP_THREAD_NULL)
	{
		LWP_JoinThread(USB_Thread, NULL);
		USB_Thread = LWP_THREAD_NULL;
	}
}

void USBKeepAliveThreadReset()
{
	while(reading)
		usleep(100);
	start = time(NULL);
	idle = false;
}
