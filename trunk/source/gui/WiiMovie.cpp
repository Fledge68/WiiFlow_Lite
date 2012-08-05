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
 * WiiMovie.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <unistd.h>
#include <asndlib.h>

#include "WiiMovie.hpp"
#include "gecko/gecko.h"
#include "memory/mem2.hpp"

#define SND_BUFFERS     8
#define FRAME_BUFFERS	8

static BufferCircle * soundBuffer = NULL;

WiiMovie::WiiMovie(const char * filepath)
{
    VideoFrameCount = 0;
	fps = 0.0f;
    ExitRequested = false;
	fullScreen = false;
    Playing = false;
    volume = 128;
	ThreadStack = NULL;
	PlayThread = LWP_THREAD_NULL;

	gprintf("Opening video '%s'\n", filepath);

    string file(filepath);
    Video = openVideo(file);
    if(!Video)
    {
		gprintf("Open video failed\n");
        ExitRequested = true;
		return;
    }

    SndChannels = (Video->getNumChannels() == 2) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
    SndFrequence = Video->getFrequency();
	fps = Video->getFps();
    maxSoundSize = Video->getMaxAudioSamples()*Video->getNumChannels()*2;
	gprintf("Open video succeeded: sound channels: %d, Frequency: %d, FPS: %4.3f\n", SndChannels, SndFrequence, fps);

	if (Video->hasSound())
	{
		gprintf("Video has sound\n");
		soundBuffer = &SoundBuffer;
		soundBuffer->Resize(SND_BUFFERS);
		soundBuffer->SetBufferBlockSize(maxSoundSize * FRAME_BUFFERS);
	}

	PlayThreadStack = NULL;
	ThreadStack = (u8 *)malloc(32768);
	if (!ThreadStack)
		return;

    LWP_MutexInit(&mutex, true);
	LWP_CreateThread (&ReadThread, UpdateThread, this, ThreadStack, 32768, LWP_PRIO_HIGHEST);
	gprintf("Reading frames thread started\n");
}

WiiMovie::~WiiMovie()
{
	gprintf("Destructing WiiMovie object\n");
    Playing = true;
    ExitRequested = true;

	Stop();

    LWP_ResumeThread(ReadThread);
    LWP_JoinThread(ReadThread, NULL);
    LWP_MutexDestroy(mutex);

    ASND_StopVoice(10);
	if (ReadThread != LWP_THREAD_NULL)
	{
		LWP_ResumeThread(ReadThread);
		LWP_JoinThread(ReadThread, NULL);
	}
	if (mutex != LWP_MUTEX_NULL)
	{
		LWP_MutexUnlock(mutex);
		LWP_MutexDestroy(mutex);
	}
	if (ThreadStack != NULL)
	{
		free(ThreadStack);
		ThreadStack = NULL;
	}

	soundBuffer = NULL;

    Frames.clear();

    if(Video)
       closeVideo(Video);
}

bool WiiMovie::Play(bool loop)
{
    if(!Video) return false;

	gprintf("Start playing video\n");

	PlayThreadStack = (u8 *)malloc(32768);
	if (PlayThreadStack == NULL)
		return false;

    Playing = true;
    PlayTime.reset();

	Video->loop = loop;

	gprintf("Start playing thread\n");

    LWP_ResumeThread(ReadThread);
	LWP_CreateThread(&PlayThread, PlayingThread, this, PlayThreadStack, 32768, 70);
	
	return true;
}

void WiiMovie::Stop()
{
	gprintf("Stopping WiiMovie video\n");
    ExitRequested = true;
	if (PlayThread != LWP_THREAD_NULL)
		LWP_JoinThread(PlayThread, NULL);

	PlayThread = LWP_THREAD_NULL;
	gprintf("Playing thread stopped\n");

	if(PlayThreadStack != NULL)
		free(PlayThreadStack);
}

void WiiMovie::SetVolume(int vol)
{
    volume = 255 * vol/100;
    ASND_ChangeVolumeVoice(10, volume, volume);
}

void WiiMovie::SetScreenSize(int width, int height, int top, int left)
{
	screenwidth = width;
	screenheight = height;
	screenleft = left;
	screentop = top;
}

void WiiMovie::SetFullscreen()
{
    if(!Video) return;

    float newscale = 1000.0f;

    float vidwidth = (float) width * 1.0f;
    float vidheight = (float) height * 1.0f;
    int retries = 100;
	fullScreen = true;

    while(vidheight * newscale > screenheight || vidwidth * newscale > screenwidth)
    {
        if(vidheight * newscale > screenheight)
            newscale = screenheight/vidheight;
        if(vidwidth * newscale > screenwidth)
            newscale = screenwidth/vidwidth;

        retries--;
        if(retries == 0)
        {
            newscale = 1.0f;
            break;
        }
    }

    scaleX = scaleY = newscale;
}

void WiiMovie::SetFrameSize(int w, int h)
{
    if(!Video) return;

    scaleX = (float) w /(float) width;
    scaleY = (float) h /(float) height;
}

void WiiMovie::SetAspectRatio(float Aspect)
{
    if(!Video) return;

    float vidwidth = (float) height*scaleY*Aspect;

    scaleX = (float) width/vidwidth;
}

extern "C" void THPSoundCallback(int voice)
{
	if (!soundBuffer || !soundBuffer->IsBufferReady()) return;
	
    if(ASND_AddVoice(voice, soundBuffer->GetBuffer(), soundBuffer->GetBufferSize()) != SND_OK)
        return;
	
	soundBuffer->LoadNext();
}

void WiiMovie::FrameLoadLoop()
{
	while (!ExitRequested)
	{
		LoadNextFrame();
		
		while (Frames.size() > FRAME_BUFFERS && !ExitRequested)
			usleep(100);
	}
}

void * WiiMovie::UpdateThread(void *arg)
{
	WiiMovie *movie = static_cast<WiiMovie *>(arg);
	while (!movie->ExitRequested)
	{
		movie->ReadNextFrame();
		usleep(100);
	}
	return NULL;
}

void * WiiMovie::PlayingThread(void *arg)
{
	WiiMovie *movie = static_cast<WiiMovie *>(arg);
	movie->FrameLoadLoop();
	
	return NULL;
}

void WiiMovie::ReadNextFrame()
{
    if(!Playing) LWP_SuspendThread(ReadThread);

    u32 FramesNeeded = (u32) (PlayTime.elapsed()*fps);

	gprintf("Reading needed frames: %d\n", FramesNeeded);

    while(VideoFrameCount < FramesNeeded)
    {
        LWP_MutexLock(mutex);
        Video->loadNextFrame();
        LWP_MutexUnlock(mutex);

        ++VideoFrameCount;

		gprintf("Loaded video frame: %d\n", VideoFrameCount);

        if(Video->hasSound())
        {
			u32 newWhich = SoundBuffer.Which();
			int i = 0;
			for (i = 0; i < SoundBuffer.Size()-2; ++i)
			{
				if (!SoundBuffer.IsBufferReady(newWhich)) break;
				
				newWhich = (newWhich + 1) % SoundBuffer.Size();
			}
			
			if (i == SoundBuffer.Size() - 2) return;
			
			int currentSize = SoundBuffer.GetBufferSize(newWhich);
			currentSize += Video->getCurrentBuffer((s16 *) (&SoundBuffer.GetBuffer(newWhich)[currentSize]))*SndChannels*2;
			SoundBuffer.SetBufferSize(newWhich, currentSize);
			
			if(currentSize >= (FRAME_BUFFERS-1)*maxSoundSize)
				SoundBuffer.SetBufferReady(newWhich, true);
			
            if(ASND_StatusVoice(10) == SND_UNUSED && SoundBuffer.IsBufferReady())
            {
                ASND_StopVoice(10);
                ASND_SetVoice(10, SndChannels == 2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, SndFrequence, 0, SoundBuffer.GetBuffer(), SoundBuffer.GetBufferSize(), volume, volume, THPSoundCallback);
                SoundBuffer.LoadNext();
            }
        }
    }

//    usleep(100);
}

void WiiMovie::LoadNextFrame()
{
    if(!Video || !Playing)
    {
        return;
    }

    VideoFrame VideoF;
    LWP_MutexLock(mutex);
    Video->getCurrentFrame(VideoF);
    LWP_MutexUnlock(mutex);

    if(!VideoF.getData()) return;

    if(width != VideoF.getWidth())
    {
        width = VideoF.getWidth();
        height = VideoF.getHeight();
		if (fullScreen)
			SetFullscreen();
		else
		{
			// Calculate new top and left
			screenleft = (screenwidth - width) / 2;
			screentop = (screenheight - height) / 2;
		}
    }

	STexture frame;
	if (frame.fromRAW(VideoF.getData(), VideoF.getWidth(), VideoF.getHeight()) == STexture::TE_OK)
		Frames.push_back(frame);
}

bool WiiMovie::GetNextFrame(STexture *tex)
{
	if (!Video || Frames.size() == 0) return false;
	
	*tex = Frames.at(0);
	Frames.erase(Frames.begin());
	return true;
}
