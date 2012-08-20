
#include <dirent.h>
#include <cstdio>

#include "musicplayer.h"
#include "fileOps/fileOps.h"
#include "gui/text.hpp"

MusicPlayer m_music;
extern const u8 gc_ogg[];
extern const u32 gc_ogg_size;

void MusicPlayer::cleanup()
{
	MusicFile.FreeMemory();
}

void MusicPlayer::Init(Config &cfg, string, string) 
{
	m_stopped = true;
	m_fade_rate = cfg.getInt("GENERAL", "music_fade_rate", 8);
	m_music_volume = cfg.getInt("GENERAL", "sound_volume_music", 255);

	SetVolume(0);
	MusicFile.SetVoice(0);
}

void MusicPlayer::SetVolume(u8 volume)
{
	m_music_current_volume = volume;
	MusicFile.SetVolume(m_music_current_volume);
}

void MusicPlayer::Previous()
{
	LoadCurrentFile();
}

void MusicPlayer::Next()
{
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
	if(!attenuate && m_music_current_volume < m_music_volume)
	{
		SetVolume(m_music_current_volume + m_fade_rate > m_music_volume ? m_music_volume
				: m_music_current_volume + m_fade_rate);
	}
	else if(attenuate && m_music_current_volume > 0)
	{
		SetVolume(m_music_current_volume - m_fade_rate < 0 ? 0
				: m_music_current_volume - m_fade_rate);
	}
	if(!attenuate && !MusicFile.IsPlaying())
		Next();
}

void MusicPlayer::LoadCurrentFile()
{
	MusicFile.Load(gc_ogg, gc_ogg_size, false);
	Play();
}
