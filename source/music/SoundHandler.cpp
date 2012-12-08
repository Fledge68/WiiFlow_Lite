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

#include "SoundHandler.hpp"
#include "Mp3Decoder.hpp"
#include "OggDecoder.hpp"
#include "WavDecoder.hpp"
#include "AifDecoder.hpp"
#include "BNSDecoder.hpp"
#include "gecko/gecko.hpp"
#include "memory/mem2.hpp"

SoundHandler SoundHandle;

void SoundHandler::Init()
{
	Decoding = false;
	ExitRequested = false;
	for(u32 i = 0; i < MAX_DECODERS; ++i)
		DecoderList[i] = NULL;

	ThreadStack = (u8 *)MEM2_memalign(32, 32768);
	if(!ThreadStack)
		return;

	LWP_CreateThread(&SoundThread, UpdateThread, this, ThreadStack, 32768, LWP_PRIO_HIGHEST);
	gprintf("SHND: Running sound thread\n");
}

void SoundHandler::Cleanup()
{
	gprintf("SHND: Stopping sound thread\n");

	ExitRequested = true;
	ThreadSignal();
	LWP_JoinThread(SoundThread, NULL);
	SoundThread = LWP_THREAD_NULL;
	if(ThreadStack != NULL)
	{
		MEM2_free(ThreadStack);
		ThreadStack = NULL;
	}

	ClearDecoderList();
	gprintf("SHND: Stopped sound thread\n");
}

void SoundHandler::AddDecoder(int voice, const char * filepath)
{
	if(voice < 0 || voice >= MAX_DECODERS)
		return;

	if(DecoderList[voice] != NULL)
		RemoveDecoder(voice);

	DecoderList[voice] = GetSoundDecoder(filepath);
}

void SoundHandler::AddDecoder(int voice, const u8 * snd, int len)
{
	if(voice < 0 || voice >= MAX_DECODERS)
	{
		gprintf("SoundHandler: Invalid voice!\n");
		return;
	}

	if(snd == NULL || len == 0)
	{
		gprintf("SoundHandler: Invalid sound!\n");
		return;
	}

	if(DecoderList[voice] != NULL)
		RemoveDecoder(voice);

	DecoderList[voice] = GetSoundDecoder(snd, len);
}

void SoundHandler::RemoveDecoder(int voice)
{
	if(voice < 0 || voice >= MAX_DECODERS)
		return;

	if(DecoderList[voice] != NULL)
	{
		(*DecoderList[voice]).ClearBuffer();
		if(DecoderList[voice]->GetSoundType() == SOUND_OGG)
			delete((OggDecoder *)DecoderList[voice]);
		else if(DecoderList[voice]->GetSoundType() == SOUND_MP3)
			delete((Mp3Decoder *)DecoderList[voice]);
		else if(DecoderList[voice]->GetSoundType() == SOUND_WAV)
			delete((WavDecoder *)DecoderList[voice]);
		else if(DecoderList[voice]->GetSoundType() == SOUND_AIF)
			delete((AifDecoder *)DecoderList[voice]);
		else if(DecoderList[voice]->GetSoundType() == SOUND_BNS)
			delete((BNSDecoder *)DecoderList[voice]);
		else
			delete DecoderList[voice];
		DecoderList[voice] = NULL;
	}
}

void SoundHandler::ClearDecoderList()
{
	for(u32 i = 0; i < MAX_DECODERS; ++i)
		RemoveDecoder(i);
}

static inline bool CheckMP3Signature(const u8 * buffer)
{
	const char MP3_Magic[][3] =
	{
		{'I', 'D', '3'},    //'ID3'
		{0xff, 0xfe},       //'MPEG ADTS, layer III, v1.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xff},       //'MPEG ADTS, layer III, v1.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xfa},       //'MPEG ADTS, layer III, v1.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xfb},       //'MPEG ADTS, layer III, v1.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xf2},       //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xf3},       //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xf4},       //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xf5},       //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xf6},       //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xf7},       //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xe2},       //'MPEG ADTS, layer III, v2.5 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xe3},       //'MPEG ADTS, layer III, v2.5', 'mp3', 'audio/mpeg'),
	};

	if(buffer[0] == MP3_Magic[0][0] && buffer[1] == MP3_Magic[0][1] &&
	buffer[2] == MP3_Magic[0][2])
	{
		return true;
	}

	for(int i = 1; i < 13; i++)
	{
		if(buffer[0] == MP3_Magic[i][0] && buffer[1] == MP3_Magic[i][1])
			return true;
	}

	return false;
}

SoundDecoder * SoundHandler::GetSoundDecoder(const char * filepath)
{
	u32 magic;
	CFile f(filepath, "rb");
	if(f.size() == 0)
		return NULL;

	do
	{
		f.read((u8 *) &magic, 1);
	}
	while(((u8 *) &magic)[0] == 0 && f.tell() < f.size());

	if(f.tell() == f.size())
		return NULL;

	f.seek(f.tell()-1, SEEK_SET);
	f.read((u8 *) &magic, 4);
	f.close();

/* 	gprintf("SHND: Searching decoder for magic\n");
	ghexdump((u8 *) &magic, 4); */

	if(memcmp(&magic, "OggS", 4) == 0)
		return new OggDecoder(filepath);
	else if(memcmp(&magic, "RIFF", 4) == 0)
		return new WavDecoder(filepath);
	else if(memcmp(&magic, "BNS ", 4) == 0)
		return new BNSDecoder(filepath);
	else if(memcmp(&magic, "FORM", 4) == 0)
		return new AifDecoder(filepath);
	else if(CheckMP3Signature((u8 *) &magic) == true)
		return new Mp3Decoder(filepath);

	return new SoundDecoder(filepath);
}

SoundDecoder * SoundHandler::GetSoundDecoder(const u8 * sound, int length)
{
	const u8 * check = sound;
	int counter = 0;

	while(check[0] == 0 && counter < length)
	{
		check++;
		counter++;
	}

	if(counter >= length)
		return NULL;

	u32 * magic = (u32 *)check;

	if(memcmp(&magic[0], "OggS", 4) == 0)
		return new OggDecoder(sound, length);
	else if(memcmp(&magic[0], "RIFF", 4) == 0)
		return new WavDecoder(sound, length);
	else if(memcmp(&magic[0], "BNS ", 4) == 0)
		return new BNSDecoder(sound, length);
	else if(memcmp(&magic[0], "FORM", 4) == 0)
		return new AifDecoder(sound, length);
	else if(CheckMP3Signature(check) == true)
		return new Mp3Decoder(sound, length);

	return new SoundDecoder(sound, length);
}

void * SoundHandler::UpdateThread(void *arg)
{
	((SoundHandler *) arg)->InternalSoundUpdates();
	return NULL;
}

void SoundHandler::InternalSoundUpdates()
{
	u16 i = 0;
	LWP_InitQueue(&ThreadQueue);
	while(!ExitRequested)
	{
		LWP_ThreadSleep(ThreadQueue);

		for(i = 0; i < MAX_DECODERS; ++i)
		{
			if(DecoderList[i] == NULL)
				continue;

			Decoding = true;
			DecoderList[i]->Decode();
		}
		Decoding = false;
	}
	LWP_CloseQueue(ThreadQueue);
	ThreadQueue = LWP_TQUEUE_NULL;
}
