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
#ifndef EXTERNAL_BOOTER_HPP
#define EXTERNAL_BOOTER_HPP

#ifdef __cplusplus
extern "C" {
#endif

extern u8 configbytes[2];
extern u32 hooktype;

#ifdef __cplusplus
}
#endif

void WiiFlow_ExternalBooter(u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, 
							int aspectRatio, u32 returnTo, u8 BootType);
void ExternalBooter_ChannelSetup(u64 title);
void ExternalBooter_WiiGameSetup(bool wbfs, bool dvd, const char *ID);
void ShutdownBeforeExit(bool KeepPatches = false);

#endif
