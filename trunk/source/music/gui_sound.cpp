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
#include "SoundHandler.hpp"
#include "gui_sound.h"
#include "musicplayer.h"
#include "WavDecoder.hpp"
#include "loader/sys.h"

#define MAX_SND_VOICES      16

using namespace std;

static bool VoiceUsed[MAX_SND_VOICES] =
{
    true, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false, false, false
};

static inline int GetFirstUnusedVoice()
{
    for(int i = 1; i < MAX_SND_VOICES; i++)
    {
        if(VoiceUsed[i] == false)
            return i;
    }
	gprintf("ALL VOICES USED UP!!\n");
    return -1;
}

extern "C" void SoundCallback(s32 voice)
{
    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder) return;

    if(decoder->IsBufferReady())
    {
        if(ASND_AddVoice(voice, decoder->GetBuffer(), decoder->GetBufferSize()) == SND_OK)
        {
            decoder->LoadNext();
            SoundHandler::Instance()->ThreadSignal();
        }
    }
    else if(decoder->IsEOF())
        ASND_StopVoice(voice);
    else
        SoundHandler::Instance()->ThreadSignal();
}

GuiSound::GuiSound()
{
	voice = -1;
	Init();
}

GuiSound::GuiSound(string filepath, int v)
{
	voice = v;
	Init();
	Load(filepath.c_str());
}

GuiSound::GuiSound(const u8 * snd, u32 len, string name, bool isallocated, int v)
{
	voice = v;
	Init();
	Load(snd, len, isallocated);
	this->filepath = name;
}

GuiSound::GuiSound(GuiSound *g)
{	
	voice = -1;

	Init();
	if (g == NULL) return;
	
	if (g->sound != NULL)
	{
		u8 * snd = (u8 *) malloc(g->length);
		memcpy(snd, g->sound, length);
		Load(snd, g->length, true);
	}
	else
		Load(g->filepath.c_str());
}

GuiSound::~GuiSound()
{
	FreeMemory();
	VoiceUsed[voice] = false;
}

void GuiSound::Init()
{
	sound = NULL;
	length = 0;

	if (voice == -1)
		voice = GetFirstUnusedVoice();
    if(voice > 0)
        VoiceUsed[voice] = true;
	
	volume = 255;
	SoundEffectLength = 0;
	loop = false;
	allocated = false;
}

void GuiSound::FreeMemory()
{
	Stop();

	// Prevent reinitialization of SoundHandler since we're exiting
	if (!Sys_Exiting())
		SoundHandler::Instance()->RemoveDecoder(voice);

    if(allocated)
    {
        SAFE_FREE(sound);
        allocated = false;
    }
	filepath = "";

    SoundEffectLength = 0;
}

bool GuiSound::Load(const char * filepath)
{
    FreeMemory();

	if(!filepath || filepath[strlen(filepath)-1] == '/' || strlen(filepath) < 4)
		return false;

    FILE * f = fopen(filepath, "rb");
    if(!f)
	{	gprintf("Failed to load file %s!!\n", filepath);
        return false;
	}

    u32 magic;
    fread(&magic, 1, 4, f);
    fclose(f);

    SoundHandler::Instance()->AddDecoder(voice, filepath);
	gprintf("Loading %s using voice %d\n", filepath, voice);
    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
	{	gprintf("No Decoder!!!\n");
		return false;
	}
	
    if(!decoder->IsBufferReady())
    {	gprintf("Buffer not ready!!n");
        SoundHandler::Instance()->RemoveDecoder(voice);
        return false;
    }

	this->filepath = filepath;
    SetLoop(loop);

	return true;
}

bool GuiSound::Load(const u8 * snd, u32 len, bool isallocated)
{
    FreeMemory();
	this->voice = voice;

    if(!snd)
	{
        return false;
	}

    if(!isallocated && *((u32 *) snd) == 'RIFF')
    {
        return LoadSoundEffect(snd, len);
    }

    if(*((u32 *) snd) == 'IMD5')
    {
        UncompressSoundbin(snd, len, isallocated);
    }
    else
    {
        sound = (u8 *) snd;
        length = len;
        allocated = isallocated;
    }
	
    SoundHandler::Instance()->AddDecoder(this->voice, sound, length);

    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
	{
        return false;
	}

    if(!decoder->IsBufferReady())
    {
        SoundHandler::Instance()->RemoveDecoder(voice);
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
    sound = (u8 *) malloc(4096);
    memset(sound, 0, 4096);

    while(1)
    {
        u8 * tmpsnd = (u8 *) realloc(sound, done+4096);
        if(!tmpsnd)
        {
            SAFE_FREE(sound);
            return false;
        }

        sound = tmpsnd;

        int read = decoder.Read(sound+done, 4096, done);
        if(read <= 0)
            break;

        done += read;
    }

    sound = (u8 *) realloc(sound, done);
    SoundEffectLength = done;
    allocated = true;

    return true;
}

void GuiSound::Play(int vol, bool restart)
{
    if(SoundEffectLength > 0)
    {
        ASND_StopVoice(voice);
        ASND_SetVoice(voice, VOICE_STEREO_16BIT, 32000, 0, sound, SoundEffectLength, vol, vol, NULL);
        return;
    }

    if(IsPlaying() && !restart)
	{
		return;
	}

	if(voice < 0 || voice >= 16)
	{
		return;
	}
	
    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
	{
        return;
	}

	ASND_StopVoice(voice);
    if(decoder->IsEOF())
    {
        decoder->ClearBuffer();
        decoder->Rewind();
        decoder->Decode();
    }

    u8 * curbuffer = decoder->GetBuffer();
    int bufsize = decoder->GetBufferSize();
    decoder->LoadNext();
    SoundHandler::Instance()->ThreadSignal();

    ASND_SetVoice(voice, decoder->GetFormat(), decoder->GetSampleRate(), 0, curbuffer, bufsize, vol, vol, SoundCallback);
}

void GuiSound::Play()
{
	Play(volume);
}

void GuiSound::Stop()
{
	if (!IsPlaying()) return;

	if(voice < 0 || voice >= 16)
		return;

	ASND_StopVoice(voice);

    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
	{
        return;
	}

    decoder->ClearBuffer();
	Rewind();
	
    SoundHandler::Instance()->ThreadSignal();
}

void GuiSound::Pause()
{
	if(voice < 0 || voice >= 16)
		return;

    ASND_StopVoice(voice);
}

void GuiSound::Resume()
{
    Play();
}

bool GuiSound::IsPlaying()
{
	if(voice < 0 || voice >= 16)
		return false;

    int result = ASND_StatusVoice(voice);

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
	if(voice < 0 || voice >= 16)
		return;

	if(vol < 0)
		return;

	volume = vol;
    ASND_ChangeVolumeVoice(voice, volume, volume);
}

void GuiSound::SetLoop(u8 l)
{
	loop = l;

    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
        return;

    decoder->SetLoop(l == 1);
}

void GuiSound::Rewind()
{
    SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
    if(!decoder)
        return;

    decoder->Rewind();
}

struct _LZ77Info
{
        u16 length : 4;
        u16 offset : 12;
} __attribute__((packed));

typedef struct _LZ77Info LZ77Info;

u8 * uncompressLZ77(const u8 *inBuf, u32 inLength, u32 * size)
{
	u8 *buffer = NULL;
	if (inLength <= 0x8 || *((const u32 *)inBuf) != 0x4C5A3737 /*"LZ77"*/ || inBuf[4] != 0x10)
		return NULL;

	u32 uncSize = le32(((const u32 *)inBuf)[1] << 8);
	if(uncSize <= 0) return 0;

	const u8 *inBufEnd = inBuf + inLength;
	inBuf += 8;

	buffer = (u8 *) malloc(uncSize);

	if (!buffer)
		return buffer;

	u8 *bufCur = buffer;
	u8 *bufEnd = buffer + uncSize;

	while (bufCur < bufEnd && inBuf < inBufEnd)
	{
		u8 flags = *inBuf;
		++inBuf;
		int i = 0;
		for (i = 0; i < 8 && bufCur < bufEnd && inBuf < inBufEnd; ++i)
		{
			if ((flags & 0x80) != 0)
			{
				const LZ77Info  * info = (const LZ77Info *)inBuf;
				inBuf += sizeof (LZ77Info);
				int length = info->length + 3;
				if (bufCur - info->offset - 1 < buffer || bufCur + length > bufEnd)
					return buffer;
				memcpy(bufCur, bufCur - info->offset - 1, length);
				bufCur += length;
			}
			else
			{
				*bufCur = *inBuf;
				++inBuf;
				++bufCur;
			}
			flags <<= 1;
		}
	}

	*size = uncSize;

	return buffer;
}

void GuiSound::UncompressSoundbin(const u8 * snd, u32 len, bool isallocated)
{
    const u8 * file = snd+32;

	length = len-32;
	if (length <= 0) return;

    if(*((u32 *) file) == 'LZ77')
    {
        u32 size = 0;
        sound = uncompressLZ77(file, length, &size);
		if (!sound)
		{
			length = 0;
			return;
		}
        length = size;
    }
    else
    {
        sound = (u8 *) malloc(length);
		if (!sound)
		{
			length = 0;
			return;
		}
        memcpy(sound, file, length);
    }

    if(isallocated)
	{
		void *p = (void *) snd;
        SAFE_FREE(p);
	}

    allocated = true;
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
