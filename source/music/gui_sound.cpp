/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "gui_sound.h"
#include "SoundHandler.hpp"
#include "MusicPlayer.hpp"
#include "WavDecoder.hpp"
#include "fileOps/fileOps.h"
#include "loader/sys.h"
#include "banner/AnimatedBanner.h"
#include "memory/mem2.hpp"

#define MAX_SND_VOICES	16

static bool VoiceUsed[MAX_SND_VOICES] =
{
	false, false, false, false, false, false,
	false, false, false, false, false, false,
	false, false, false, false
};

static inline int GetFirstUnusedVoice()
{
	for(u8 i = 2; i < MAX_SND_VOICES; i++)// voice 0 is bg music, voice 1 is gamesound
	{
		if(VoiceUsed[i] == false)
			return i;
	}
	gprintf("gui_sound.cpp: ALL VOICES USED UP!!\n");
	return -1;
}

extern "C" void SoundCallback(s32 voice)
{
	SoundDecoder *decoder = SoundHandle.Decoder(voice);
	if(!decoder)
		return;

	if(decoder->IsBufferReady())
	{
		if(ASND_AddVoice(voice, decoder->GetBuffer(), decoder->GetBufferSize()) == SND_OK)
		{
			decoder->LoadNext();
			SoundHandle.ThreadSignal();
		}
	}
	else if(decoder->IsEOF())
		ASND_StopVoice(voice);
	else
		SoundHandle.ThreadSignal();
}

GuiSound::GuiSound()
{
	this->voice = -1;
	Init();
}

GuiSound::GuiSound(const char *path, int v)
{
	if(path == NULL)
		return;
	this->voice = v;
	Init();
	Load(path);
}

GuiSound::GuiSound(const u8 * snd, u32 len, const char *name, bool isallocated, int v)
{
	this->voice = v;
	Init();
	Load(snd, len, isallocated);
	if(name != NULL)
	{
		strncpy(this->filepath, name, 255);
		this->filepath[255] = '\0';
	}
}

GuiSound::GuiSound(GuiSound *g)
{	
	this->voice = -1;

	Init();
	if(g == NULL)
		return;

	if(g->sound != NULL)
	{
		u8 *snd = (u8 *)MEM2_memalign(32, g->length);
		memcpy(snd, g->sound, g->length);
		Load(snd, g->length, true);
	}
	else
		Load(g->filepath);
}

GuiSound::~GuiSound()
{
	FreeMemory();
	VoiceUsed[this->voice] = false;
}

void GuiSound::Init()
{
	memset(this->filepath, 0, 256);
	sound = NULL;
	length = 0;

	if(this->voice == -1)
		this->voice = GetFirstUnusedVoice();
	if(this->voice != -1)
		VoiceUsed[this->voice] = true;

	volume = 255;
	SoundEffectLength = 0;
	loop = false;
	allocated = false;
}

void GuiSound::FreeMemory()
{
	Stop();
	if(this->voice != -1)
		SoundHandle.RemoveDecoder(this->voice);
	if(allocated && sound != NULL)
		free(sound);
	allocated = false;
	memset(this->filepath, 0, 256);
	sound = NULL;
	length = 0;

	SoundEffectLength = 0;
}

bool GuiSound::Load(const char *path)
{
	FreeMemory();

	if(path == NULL || path[strlen(path)-1] == '/')
		return false;

	if(!fsop_FileExist(path))
	{
		gprintf("gui_sound.cpp: Failed to load file %s!!\n", path);
		return false;
	}

	SoundHandle.AddDecoder(this->voice, path);
	//gprintf("gui_sound.cpp: Loading %s using voice %d\n", path, this->voice);
	SoundDecoder *decoder = SoundHandle.Decoder(this->voice);
	if(!decoder)
	{
		gprintf("gui_sound.cpp: No Decoder!\n");
		return false;
	}

	if(!decoder->IsBufferReady())
	{
		gprintf("gui_sound.cpp: Buffer not ready!\n");
		SoundHandle.RemoveDecoder(this->voice);
		return false;
	}

	strncpy(this->filepath, path, 255);
	this->filepath[255] = '\0';
	SetLoop(loop);

	return true;
}

bool GuiSound::Load(const u8 *snd, u32 len, bool isallocated)
{
	FreeMemory();

	if(snd == NULL || len == 0)
		return false;

	if(!isallocated && memcmp(snd, "RIFF", 4) == 0)// "RIFF" is in WAV files
		return LoadSoundEffect(snd, len);

	sound = (u8*)snd;
	length = len;
	allocated = isallocated;

	SoundHandle.AddDecoder(this->voice, sound, length);
	SoundDecoder *decoder = SoundHandle.Decoder(this->voice);
	if(!decoder || !decoder->IsBufferReady())
	{
		SoundHandle.RemoveDecoder(this->voice);
		return false;
	}

	SetLoop(loop);

	return true;
}

bool GuiSound::LoadSoundEffect(const u8 * snd, u32 len)
{
	FreeMemory();

	WavDecoder decoder(snd, len);
	decoder.Rewind();

	u32 done = 0;
	sound = (u8 *)MEM2_memalign(32, 4096);
	memset(sound, 0, 4096);

	while(1)
	{
		u8 * tmpsnd = (u8 *)MEM2_realloc(sound, done+4096);
		if(!tmpsnd)
		{
			free(sound);
			return false;
		}

		sound = tmpsnd;

		int read = decoder.Read(sound+done, 4096);
		if(read <= 0)
			break;

		done += read;
	}

	sound = (u8 *)MEM2_realloc(sound, done);
	SoundEffectLength = done;
	allocated = true;

	return true;
}

void GuiSound::Play(int vol, bool restart)
{
	if(SoundEffectLength > 0)
	{
		ASND_StopVoice(this->voice);
		ASND_SetVoice(this->voice, VOICE_MONO_16BIT, 22050, 0, sound, SoundEffectLength, vol, vol, NULL);
		return;
	}

	if((IsPlaying() && !restart) || this->voice < 0 || this->voice >= 16)
		return;

	SoundDecoder *decoder = SoundHandle.Decoder(this->voice);
	if(!decoder)
		return;

	ASND_StopVoice(this->voice);
	if(decoder->IsEOF())
	{
		decoder->ClearBuffer();
		decoder->Rewind();
		decoder->Decode();
	}

	u8 * curbuffer = decoder->GetBuffer();
	int bufsize = decoder->GetBufferSize();
	decoder->LoadNext();
	SoundHandle.ThreadSignal();

	ASND_SetVoice(this->voice, decoder->GetFormat(), decoder->GetSampleRate(), 0, curbuffer, bufsize, vol, vol, SoundCallback);
}

void GuiSound::Play()
{
	Play(volume);
}

void GuiSound::Stop()
{
	volume = 0;
	if(!IsPlaying() || this->voice < 0 || this->voice >= 16)
		return;

	ASND_StopVoice(this->voice);

	SoundDecoder *decoder = SoundHandle.Decoder(this->voice);
	if(!decoder)
		return;

	decoder->ClearBuffer();
	Rewind();

	SoundHandle.ThreadSignal();
}

void GuiSound::Pause()
{
	if(this->voice < 0 || this->voice >= 16)
		return;

	ASND_StopVoice(this->voice);
}

void GuiSound::Resume()
{
	Play();
}

bool GuiSound::IsPlaying()
{
	if(this->voice < 0 || this->voice >= 16)
		return false;

	int result = ASND_StatusVoice(this->voice);

	if(result == SND_WORKING || result == SND_WAITING)
		return true;

	return false;
}

int GuiSound::GetVolume()
{
	return volume;
}

void GuiSound::SetVolume(int vol)
{
	if(this->voice < 0 || this->voice >= 16 || vol < 0)
		return;

	volume = vol;
	ASND_ChangeVolumeVoice(this->voice, volume, volume);
}

void GuiSound::SetLoop(u8 l)
{
	loop = l;

	SoundDecoder *decoder = SoundHandle.Decoder(this->voice);
	if(!decoder)
		return;

	decoder->SetLoop(l == 1);
}

void GuiSound::Rewind()
{
	SoundDecoder *decoder = SoundHandle.Decoder(this->voice);
	if(!decoder)
		return;

	decoder->Rewind();
}

void GuiSound::SetVoice(s8 v)
{
	this->voice = v;
}

void soundInit(void)
{
	ASND_Init();
	ASND_Pause(0);
}

void soundDeinit(void)
{
	ASND_Pause(1);
	ASND_End();
}
