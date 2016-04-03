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
#ifndef _GC_HPP_
#define _GC_HPP_

#include <gccore.h>

// Nintendont
#include "nin_cfg.h"
#define NIN_CFG_PATH "nincfg.bin"
#define NIN_LOADER_PATH "%s:/apps/nintendont/boot.dol"
//const char *NINversionDate(NIN_LOADER_PATH);

bool Nintendont_Installed();
bool Nintendont_GetLoader();
void Nintendont_BootDisc(u8 emuMC, bool widescreen, bool usb_hid, bool native_ctl, bool deflicker);
void Nintendont_SetOptions(const char *game, const char *gameID, char *CheatPath, char *NewCheatPath, const char *partition,
	bool cheats, u8 emuMC, u8 videomode, bool widescreen, bool usb_hid, bool native_ctl, bool deflicker, bool wiiu_widescreen, bool NIN_Debugger);
void Nintendont_WriteOptions();

// Devolution
#define DEVO_LOADER_PATH "%s/loader.bin"

typedef struct global_config
{
	u32 signature;
	u16 version;
	u16 device_signature;
	u32 memcard_cluster;
	u32 disc1_cluster;
	u32 disc2_cluster;
	u32 options;
} gconfig;

// constant value for identification purposes
#define DEVO_CONFIG_SIG			0x3EF9DB23
// version may change when future options are added
#define DEVO_CONFIG_VERSION		0x0110
// option flags
#define DEVO_CONFIG_WIFILOG		(1<<0)
#define DEVO_CONFIG_WIDE		(1<<1)
#define DEVO_CONFIG_NOLED		(1<<2)

bool DEVO_Installed(const char *path);
void DEVO_GetLoader(const char *path);
void DEVO_SetOptions(const char *isopath, const char *gameID, 
		bool memcard_emum, bool widescreen, bool activity_led, bool wifi);
void DEVO_Boot();


// General
void GC_SetVideoMode(u8 videomode, u8 loader);
void GC_SetLanguage(u8 lang, u8 loader);

#endif //_GC_HPP_
