
#include <dirent.h>
#include <cstdio>

#include "musicplayer.h"
#include "fileOps/fileOps.h"
#include "gui/text.hpp"

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
	m_music_files.clear();
	ScanDirectories(themeMusicDir.c_str());
	ScanDirectories(musicDir.c_str());
	
	if(cfg.getBool("GENERAL", "randomize_music", true) && m_music_files.size() > 0)
	{
		srand(unsigned(time(NULL)));
		random_shuffle(m_music_files.begin(), m_music_files.end());
	}
	m_current_music = m_music_files.begin();
}

void MusicPlayer::ScanDirectories(const char *directory)
{
	struct dirent *pent = NULL;
	DIR *pdir = opendir(directory);
	while((pent = readdir(pdir)) != NULL) 
	{
		if(strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;
		string CurrentItem = sfmt("%s/%s", directory, pent->d_name);
		if(fsop_DirExist(CurrentItem.c_str()))
			ScanDirectories(CurrentItem.c_str());
		else if(strcasestr(pent->d_name, ".mp3") != NULL || strcasestr(pent->d_name, ".ogg")  != NULL)
			m_music_files.push_back(CurrentItem);
	}
	closedir(pdir);
}

void MusicPlayer::SetMaxVolume(u8 volume)
{
	m_music_volume = volume;
	SetVolume(volume);
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
	MusicFile.Load((*m_current_music).c_str());
	Play();
}
