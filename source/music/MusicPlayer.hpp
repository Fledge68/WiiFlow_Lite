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
#ifndef _MUSICPLAYER_HPP_
#define _MUSICPLAYER_HPP_

#include <string>
#include "gui_sound.h"
#include "config/config.hpp"
#include "wstringEx/wstringEx.hpp"

class Musicplayer
{
public:
	void Cleanup();
	void Init(Config &cfg, const string& musicDir, const string& themeMusicDir);
	int InitPlaylist(Config &cfg, const char *playlist, u8 device);
	void Tick(bool attenuate);
	
	void SetFadeRate(u8 faderate);
	void SetVolume(u8 volume);
	void SetMaxVolume(u8 volume);
	u8 GetVolume() { return CurrentVolume; };
	u8 GetMaxVolume() { return Volume; };
	bool ResampleSetting;
	void SetResampleSetting(bool resample);

	void Previous();
	void Next();
	void Stop();
	void Pause();
	void Resume();

	bool IsStopped() { return MusicStopped; };

	/* For our GUI */
	wstringEx GetFileName();
	bool SongChanged();
	time_t DisplayTime;
	bool OneSong;

	void LoadFile(const char *name, bool display_change = true);
	void LoadCurrentFile();
	void ReLoadCurrentFile();
protected:
	bool PosFromPrevFile();

	u8 Volume;
	u8 CurrentVolume;
	u8 FadeRate;
	int CurrentPosition;
	bool MusicStopped;
	bool MusicChanged;
	const char *curPlaylist;
	bool usingPlaylist = false;
	u8 pl_device;

	GuiSound MusicFile;
};

extern Musicplayer MusicPlayer;

#endif /* _MUSICPLAYER_HPP_ */
