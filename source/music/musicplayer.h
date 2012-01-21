#ifndef _MUSICPLAYER_H
#define _MUSICPLAYER_H

#include <string>
#include "config/config.hpp"

#include "cachedlist.hpp"
#include "gui_sound.h"

enum MusicDirectory
{
	NORMAL_MUSIC = 1,
	THEME_MUSIC = 2
};

#define ASND_MUSIC_VOICE 0

class MusicPlayer
{
public:
	static MusicPlayer *Instance();
	static void DestroyInstance();

	void Init(Config &cfg, std::string musicDir, std::string themeMusicDir);
	void Tick(bool attenuate);
	
	void SetVolume(int volume);
	void SetVolume(int volume, int max_volume);
	int GetVolume() { return m_music != NULL ? m_music_current_volume : 0; };
	int GetMaxVolume() { return m_music_volume; };
	
	void Previous();
	void Next();
	void Pause();
	void Play();
	void Stop();
	
	bool IsStopped() { return m_stopped; };
private:
	~MusicPlayer();

	static MusicPlayer *instance;

	void LoadCurrentFile();

	CachedList<std::string> m_music_files;
	safe_vector<std::string>::iterator m_current_music;
	
	int m_fade_rate;
	
	int m_music_volume;
	int m_music_current_volume;
	bool m_manual_stop;
	
	bool m_paused;
	bool m_stopped;
	bool m_playbackFinished;
	
	GuiSound *m_music;
	u32 *m_songCount;
};

#endif