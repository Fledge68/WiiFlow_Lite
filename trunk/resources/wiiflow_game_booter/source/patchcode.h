/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *
 *  this file is part of GeckoOS for USB Gecko
 *  http://www.usbgecko.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __PATCHCODE_H__
#define __PATCHCODE_H__

#ifdef __cplusplus
extern "C" {
#endif
// Globals
extern u32 hooktype;
extern u8 configbytes[2];

// Function prototypes
bool dogamehooks(void *addr, u32 len, bool channel);
void langpatcher(void *addr, u32 len);
void vidolpatcher(void *addr, u32 len);
void PatchVideoSneek(void *addr, u32 len);
void PatchCountryStrings(void *Address, int Size);
void PatchAspectRatio(void *addr, u32 len, u8 aspect);
bool PatchReturnTo(void *Address, int Size, u32 id);
void Patch_fwrite(void *Address, int Size);
s32 BlockIOSReload(void);
void PatchRegion(void *Address, int Size);
void PrivateServerPatcher(void *addr, u32 len);
void domainpatcher(void *addr, u32 len, const char* domain);

#ifdef __cplusplus
}
#endif

#endif // __PATCHCODE_H__
