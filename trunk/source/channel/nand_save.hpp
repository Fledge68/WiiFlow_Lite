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
#ifndef _NAND_SAVE_HPP_
#define _NAND_SAVE_HPP_

#include <ogcsys.h>

typedef struct _ios_settings_t
{
	u8 cios;
	bool use_cios;
} ATTRIBUTE_PACKED ios_settings_t;

class NandSave
{
public:
	NandSave();
	bool CheckSave();
	void LoadSettings();
	void SaveIOS();
	void SavePort(u8 port);
private:
	void WriteFile(const char *file_name, u8 *content, u32 size);
	s32 fd;
	s32 ret;
	bool loaded;
	char ISFS_Path[ISFS_MAXPATH];
	ios_settings_t ios_settings;
};

extern NandSave InternalSave;
extern bool cur_load;
extern u8 cur_ios;

#endif
