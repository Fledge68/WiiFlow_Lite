#ifndef _MUSICPLAYER_H
#define _MUSICPLAYER_H

#include <string>
#include "gui_sound.h"
#include "config/config.hpp"
#include "list/cachedlist.hpp"

enum MusicDirectory
{
	NORMAL_MUSIC = 1,
	THEME_MUSIC = 2
};

class MusicPlayer
{
public:
	void cleanup();
	void Init(Config &cfg, std::string musicDir, std::string themeMusicDir);
	void Tick(bool attenuate);

	void SetVolume(u8 volume);
	u8 GetVolume() { return m_music_current_volume; };
	u8 GetMaxVolume() { return m_music_volume; };

	void Previous();
	void Next();
	void Play();
	void Stop();

	bool IsStopped() { return m_stopped; };

protected:
	void LoadCurrentFile();

	u8 m_music_volume;
	u8 m_music_current_volume;
	u8 m_fade_rate;
	bool m_stopped;

	GuiSound MusicFile;
	CachedList<std::string> m_music_files;
	vector<std::string>::iterator m_current_music;
};

extern MusicPlayer m_music;

#endif