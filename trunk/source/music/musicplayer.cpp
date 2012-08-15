#include "musicplayer.h"

using namespace std;

MusicPlayer m_music;

void MusicPlayer::cleanup()
{
	MusicFile.FreeMemory();
}

void MusicPlayer::Init(Config &cfg, string musicDir, string themeMusicDir) 
{
	m_stopped = true;
	m_fade_rate = cfg.getInt("GENERAL", "music_fade_rate", 8);
	m_music_volume = cfg.getInt("GENERAL", "sound_volume_music", 255);

	SetVolume(0);
	MusicFile.SetVoice(0);

	MusicDirectory dir = (MusicDirectory)cfg.getInt("GENERAL", "music_directories", NORMAL_MUSIC | THEME_MUSIC);
	m_music_files.Init(cfg.getString("GENERAL", "dir_list_cache"), std::string(), std::string(), std::string(), false);

	if(dir & THEME_MUSIC)
		m_music_files.Load(themeMusicDir, ".ogg|.mp3", "EN", cfg); //|.mod|.xm|.s3m|.wav|.aiff");

	if(dir & NORMAL_MUSIC)
		m_music_files.Load(musicDir, ".ogg|.mp3", "EN", cfg); //|.mod|.xm|.s3m|.wav|.aiff");
	
	if(cfg.getBool("GENERAL", "randomize_music", true) && m_music_files.size() > 0)
	{
		srand(unsigned(time(NULL)));
		random_shuffle(m_music_files.begin(), m_music_files.end());
	}
	m_current_music = m_music_files.begin();
}

void MusicPlayer::SetVolume(u8 volume)
{
	m_music_current_volume = volume;
	MusicFile.SetVolume(m_music_current_volume);
}

void MusicPlayer::Previous()
{
	if(m_music_files.empty())
		return;
	if(m_current_music == m_music_files.begin())
		m_current_music = m_music_files.end();

	m_current_music--;

	LoadCurrentFile();
}

void MusicPlayer::Next()
{
	if(m_music_files.empty())
		return;

	m_current_music++;
	if (m_current_music == m_music_files.end())
		m_current_music = m_music_files.begin();
	
	LoadCurrentFile();
}

void MusicPlayer::Play()
{
	SetVolume(m_music_current_volume);
	MusicFile.Play();
	m_stopped = false;
}

void MusicPlayer::Stop()
{
	MusicFile.Pause();
	MusicFile.Stop();
	m_stopped = true;
}

void MusicPlayer::Tick(bool attenuate)
{
	if(m_music_files.empty())
		return;
	if(!attenuate && m_music_current_volume < m_music_volume)
	{
		SetVolume(m_music_current_volume + m_fade_rate > m_music_volume ? m_music_volume
				: m_music_current_volume + m_fade_rate);
		if(!MusicFile.IsPlaying())
			Next();
	}
	else if(attenuate && m_music_current_volume > 0)
	{
		SetVolume(m_music_current_volume - m_fade_rate < 0 ? 0
				: m_music_current_volume - m_fade_rate);
	}
}

void MusicPlayer::LoadCurrentFile()
{
	MusicFile.Load((*m_current_music).c_str());
	Play();
}
