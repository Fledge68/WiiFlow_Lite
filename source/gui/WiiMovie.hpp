#ifndef WII_MOVIE_H_
#define WII_MOVIE_H_

#include "gcvid.h"
#include "Timer.h"
#include "texture.hpp"

class WiiMovie
{
public:
	WiiMovie() { inited = false; };
	void Init(const char * filepath);
	void DeInit();
	bool Play(bool loop = false);
	void Stop();
	bool Continue();
	volatile bool loaded;
	TexData *Frame; //our current texture
	TexData Buffer[2];
	u8 BufferPos;
protected:
	static void * UpdateThread(void *arg);
	void FrameLoadLoop();
	void ReadNextFrame();
	void LoadNextFrame();

	u8 * ThreadStack;
	lwp_t ReadThread;

	ThpVideoFile Video;
	VideoFrame VideoF;
	FILE *vFile;
	float fps;
	Timer PlayTime;
	u32 VideoFrameCount;
	bool Playing;
	bool ExitRequested;
	bool inited;
};

struct movieP
{
	float x1;
	float y1;
	float x2;
	float y2;
};
extern struct movieP normalMoviePos;
extern struct movieP zoomedMoviePos;
extern struct movieP currentMoviePos;
extern WiiMovie movie;

#endif
