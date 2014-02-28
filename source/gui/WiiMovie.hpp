#ifndef WII_MOVIE_H_
#define WII_MOVIE_H_

#include "gcvid.h"
#include "Timer.h"
#include "texture.hpp"

using namespace std;

class WiiMovie
{
public:
	void Init(const char * filepath);
	void DeInit();
	bool Play(TexData *Background, bool loop = false);
	void Stop();
	bool Continue();
	volatile bool loaded;
	volatile bool rendered;
protected:
	static void * UpdateThread(void *arg);
	void FrameLoadLoop();
	void ReadNextFrame();
	void LoadNextFrame();

	u8 * ThreadStack;
	lwp_t ReadThread;

	ThpVideoFile Video;
	FILE *vFile;
	float fps;
	Timer PlayTime;
	u32 VideoFrameCount;
	TexData *Frame;
	bool Playing;
	bool ExitRequested;
};

extern WiiMovie movie;

#endif
