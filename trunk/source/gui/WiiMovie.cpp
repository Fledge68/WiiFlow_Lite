/***************************************************************************
 * Copyright (C) 2010 by Dimok
 *           (C) 2014 by FIX94
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
#include "memory/mem2.hpp"
#include "WiiMovie.hpp"
#include "gecko/gecko.hpp"

WiiMovie movie;
void WiiMovie::Init(const char *filepath)
{
	Frame = NULL;
	BufferPos = 0;
	Buffer[0].thread = false;
	Buffer[1].thread = false;
	VideoFrameCount = 0;
	fps = 0.0f;
	ExitRequested = false;
	Playing = false;
	ThreadStack = NULL;
	//gprintf("Opening video '%s'\n", filepath);
	ReadThread = LWP_THREAD_NULL;
	vFile = fopen(filepath, "rb");
	if(!vFile)
	{
		//gprintf("Open video failed\n");
		ExitRequested = true;
		return;
	}
	if(Video.Init(vFile) == false)
	{
		//gprintf("Memory Allocation failed\n");
		ExitRequested = true;
		return;
	}
	fps = Video.getFps();
	inited = true;
}

void WiiMovie::DeInit()
{
	if(inited == false)
		return;
	inited = false;
	//gprintf("Destructing WiiMovie object\n");
	Playing = false;
	ExitRequested = true;

	Stop();

	if(ReadThread != LWP_THREAD_NULL)
	{
		LWP_ResumeThread(ReadThread);
		LWP_JoinThread(ReadThread, NULL);
	}
	if(ThreadStack != NULL)
	{
		MEM2_lo_free(ThreadStack);
		ThreadStack = NULL;
	}

	Video.DeInit();
	if(vFile != NULL)
		fclose(vFile);
	vFile = NULL;
	Frame = NULL;
	TexHandle.Cleanup(Buffer[0]);
	TexHandle.Cleanup(Buffer[1]);
	Buffer[0].thread = false;
	Buffer[1].thread = false;
	BufferPos = 0;
}

bool WiiMovie::Play(bool loop)
{
	if(!vFile)
		return false;

	ThreadStack = (u8*)MEM2_lo_alloc(32768);
	if(!ThreadStack)
		return false;

	//gprintf("Start playing video\n");
	Video.loop = loop;
	PlayTime.reset();
	Playing = true;
	Buffer[0].thread = false;
	Buffer[1].thread = false;
	LWP_CreateThread(&ReadThread, UpdateThread, this, ThreadStack, 32768, 60);
	//gprintf("Reading frames thread started\n");
	return true;
}

void WiiMovie::Stop()
{
	//gprintf("Stopping WiiMovie video\n");
	ExitRequested = true;
}

void * WiiMovie::UpdateThread(void *arg)
{
	WiiMovie *movie = static_cast<WiiMovie *>(arg);
	while(!movie->ExitRequested)
	{
		movie->ReadNextFrame();
		if(movie->Buffer[movie->BufferPos].thread == false)
			movie->LoadNextFrame();
		else if(movie->Frame->thread == false)
		{
			movie->Frame = &movie->Buffer[movie->BufferPos];
			movie->BufferPos ^= 1;
		}
	}
	return NULL;
}

void WiiMovie::ReadNextFrame()
{
	for(u32 FramesNeeded = (u32)(PlayTime.elapsed()*fps); VideoFrameCount <= FramesNeeded; ++VideoFrameCount)
	{
		if(VideoFrameCount == FramesNeeded)
			Playing = Video.loadNextFrame();
		else
			Video.loadNextFrame(true);
	}
}

void WiiMovie::LoadNextFrame()
{
	if(!vFile || !Playing)
		return;

	VideoFrame VideoF;
	Video.getCurrentFrame(VideoF);

	if(!VideoF.getData())
		return;
	TexData *CurFrame = &Buffer[BufferPos];
	if(TexHandle.fromTHP(CurFrame, VideoF.getData(), VideoF.getWidth(), VideoF.getHeight()) == TE_OK)
	{
		CurFrame->thread = true;
		Frame = CurFrame;
		BufferPos ^= 1;
	}
	VideoF.dealloc();
}

bool WiiMovie::Continue()
{
	if(!vFile || !Playing)
		return false;
	return true;
}
