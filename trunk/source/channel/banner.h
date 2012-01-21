/***************************************************************************
 * Copyright (C) 2010
 * by r-win
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
 *
 * Channel Launcher Class
 *
 * for WiiXplorer 2010
 ***************************************************************************/

#ifndef _BANNER_H_
#define _BANNER_H_

#include "safe_vector.hpp"
#include <string>

#define IMET_MAX_NAME_LEN 0x2a

typedef struct
{
	u8  zeroes1[0x40];
	u32 sig;	// "IMET"
	u32 unk1;
	u32 unk2;
	u32 filesizes[3];
	u32 unk3;
	u16 name_japanese[IMET_MAX_NAME_LEN];
	u16 name_english[IMET_MAX_NAME_LEN];
	u16 name_german[IMET_MAX_NAME_LEN];
	u16 name_french[IMET_MAX_NAME_LEN];
	u16 name_spanish[IMET_MAX_NAME_LEN];
	u16 name_italian[IMET_MAX_NAME_LEN];
	u16 name_dutch[IMET_MAX_NAME_LEN];
	u16 name_simp_chinese[IMET_MAX_NAME_LEN];
	u16 name_trad_chinese[IMET_MAX_NAME_LEN];
	u16 name_korean[IMET_MAX_NAME_LEN];
	u8  zeroes2[0x24c];
	u8  md5[0x10];
} IMET;

using namespace std;

class Banner
{
	public:
		Banner(u8 *bnr, u64 title = 0);
		~Banner();
		
		bool IsValid();

		bool GetName(u8 *name, int language);
		bool GetName(wchar_t *name, int language);
		const u8 *GetFile(char *name, u32 *size);
		
		static Banner *GetBanner(u64 title, char *appname, bool isfs, bool imetOnly = false);
		
	private:
		u8 *opening;
		u64 title;
		IMET *imet;
		
		u16 *GetName(int language);

		static bool GetChannelNameFromApp(u64 title, wchar_t* name, int language);
};

#endif //_BANNER_H_
