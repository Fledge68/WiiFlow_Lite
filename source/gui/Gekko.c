/**
 * Gekko.c - Additional rendering routines for the Gekko platform.
 *
 * Copyright (c) 2009 Rhys "Shareese" Koedijk
 * 2012 Modified by FIX94 for WiiFlow
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE' included within this release
 */ 

#include <stdlib.h>
#include <ogcsys.h>
#include <gctypes.h>
#include <gccore.h>

#include "Gekko.h"
#include "memory/memory.h"

#define DISC_SLOT_LED		0x20

lwp_t light_thread = LWP_THREAD_NULL;

void *light_loop();
bool light_on = false;
u8 light_level = 0;

struct timespec light_timeon;
struct timespec light_timeoff;

void wiiLightOn()
{
	light_on = true;
	light_level = 0;
	if(light_thread == LWP_THREAD_NULL)
		LWP_CreateThread(&light_thread, light_loop, NULL, NULL, 0, LWP_PRIO_HIGHEST);
}

void wiiLightOff()
{
	light_on = false;
	light_level = 0;
	LWP_JoinThread(light_thread, NULL);
	light_thread = LWP_THREAD_NULL;
	*HW_GPIOB_OUT &= ~DISC_SLOT_LED;
}

void wiiLightSetLevel(int level)
{
	light_level = MIN(MAX(level, 0), 100);

	// Calculate the new on/off times for this light intensity
	u32 level_on;
	u32 level_off;
	level_on = (light_level * 2.55) * 40000;
	level_off = 10200000 - level_on;
	light_timeon.tv_nsec = level_on;
	light_timeoff.tv_nsec = level_off;
}

/**
 * Since you can only turn the disc slot light either completely on
 * or completely off, this thread simulates different light intensity
 * levels by turning the light on and off very quickly at a specific
 * interval defined by the current light intensity level.
 *
 * Its all an eye trick ;)
 *
 */
void *light_loop()
{
	struct timespec timeon;
	struct timespec timeoff;

	// Loop whilst the light is still 'on'
	while(light_on)
	{
		timeon = light_timeon;
		timeoff = light_timeoff;
		// Turn on the light and sleep for a bit
		*HW_GPIOB_OUT |= DISC_SLOT_LED;
		nanosleep(&timeon);
		// Turn off the light (if required) and sleep for a bit
		if(timeoff.tv_nsec > 0)
			*HW_GPIOB_OUT &= ~DISC_SLOT_LED;
		nanosleep(&timeoff);
	}
	return NULL;
}
