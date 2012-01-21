#include "musicplayer.h"

using namespace std;

MusicPlayer *MusicPlayer::instance = NULL;

MusicPlayer *MusicPlayer::Instance() 
{
	if (instance == NULL)
		instance = new MusicPlayer();

	return instance; 
}

void MusicPlayer::DestroyInstance()
{
	if (instance != NULL)
		delete instance;

	instance = NULL;
}

void MusicPlayer::Init(Config &cfg, string musicDir, string themeMusicDir) 
{
	m_music = NULL;
	m_manual_stop = true;
	m_stopped = true;
	m_fade_rate = cfg.getInt("GENERAL", "music_fade_rate", 8);
	m_music_volume = cfg.getInt("GENERAL", "sound_volume_music", 255);

	SetVolume(0); // Fades in with tick()
	
	MusicDirectory dir = (MusicDirectory) cfg.getInt("GENERAL", "music_directories", NORMAL_MUSIC | THEME_MUSIC);
	m_music_files.Init(cfg.getString("GENERAL", "dir_list_cache"), std::string(), std::string());

	if (dir & THEME_MUSIC)
		m_music_files.Load(themeMusicDir, ".ogg|.mp3"); //|.mod|.xm|.s3m|.wav|.aiff");

	if (dir & NORMAL_MUSIC)
		m_music_files.Load(musicDir, ".ogg|.mp3"); //|.mod|.xm|.s3m|.wav|.aiff");
	
	if (cfg.getBool("GENERAL", "randomize_music", false) && m_music_files.size() > 0)
	{
		srand(unsigned(time(NULL)));
		random_shuffle(m_music_files.begin(), m_music_files.end());
	}

	m_current_music = m_music_files.begin();
}

MusicPlayer::~MusicPlayer()
{
	if (m_music != NULL)
	{
		m_music->Stop();
		delete m_music;
	}
}

void MusicPlayer::SetVolume(int volume)
{
	m_music_current_volume = volume > m_music_volume ? m_music_volume : volume;
	if (m_music != NULL)
		m_music->SetVolume(m_music_current_volume);
}

void MusicPlayer::SetVolume(int volume, int max_volume)
{
	m_music_volume = max_volume;
	SetVolume(volume);
}

void MusicPlayer::Previous()
{
	if (m_music_files.empty()) return;

	if (m_current_music == m_music_files.begin())
		m_current_music = m_music_files.end();

	m_current_music--;

	LoadCurrentFile();
}

void MusicPlayer::Next()
{
	if (m_music_files.empty()) return;

	m_current_music++;
	if (m_current_music == m_music_files.end())
		m_current_music = m_music_files.begin();
	
	LoadCurrentFile();
}

void MusicPlayer::Pause()
{
	if (m_music != NULL)
		m_music->Pause();

	m_paused = true;
}

void MusicPlayer::Play()
{
	m_manual_stop = m_paused = false; // Next tick will start the music
	if (m_music != NULL)
		m_music->SetVolume(m_music_current_volume);
}
	
void MusicPlayer::Stop()
{
	m_manual_stop = true;
	if (m_music != NULL)
	{
		m_music->Pause();
		m_music->Stop();
		delete m_music;
		m_music = NULL;
	}
	m_stopped = true;
}

void MusicPlayer::Tick(bool attenuate)
{
	if (m_music_files.empty()) return;
	if (m_music_current_volume == 0 && attenuate) return;

	if (m_music != NULL)
	{
		if (!attenuate && m_music_current_volume < m_music_volume)
		{
			int volume = m_music_current_volume + m_fade_rate > m_music_volume ?
				m_music_volume : m_music_current_volume + m_fade_rate;
			SetVolume(volume);
		}
		else if (attenuate && m_music_current_volume > 0)
		{
			int volume = m_music_current_volume - m_fade_rate < 0 ?
				0 : m_music_current_volume - m_fade_rate;
			SetVolume(volume);
		}
	}
		
	if (!attenuate && !m_manual_stop && (m_music == NULL || m_stopped || !m_music->IsPlaying()))
		Next();
}

void MusicPlayer::LoadCurrentFile()
{
	if (m_music_files.empty()) return;

	if (m_music != NULL)
		m_music->Stop();
	
	if (m_music == NULL)
		m_music = new GuiSound((*m_current_music).c_str(), ASND_MUSIC_VOICE);
	else
		m_music->Load((*m_current_music).c_str());

	if (m_music != NULL && !m_manual_stop)
	{
		m_music->SetVolume(m_music_current_volume);
		m_music->Play();
		m_stopped = false;
	}
}
