#ifndef WII_MOVIE_H_
#define WII_MOVIE_H_

#include "gcvid.h"
#include "Timer.h"
#include "texture.hpp"
#include "BufferCircle.hpp"

using namespace std;

class WiiMovie
{
    public:
        WiiMovie(const char * filepath);
        ~WiiMovie();
        bool Play(bool loop = false);
        void Stop();
        void SetVolume(int vol);
		void SetScreenSize(int width, int height, int top, int left);
        void SetFullscreen();
        void SetFrameSize(int w, int h);
        void SetAspectRatio(float Aspect);
		bool GetNextFrame(STexture *tex);
    protected:
		static void * UpdateThread(void *arg);
		static void * PlayingThread(void *arg);
        void FrameLoadLoop();
        void ReadNextFrame();
        void LoadNextFrame();

		u8 * ThreadStack;
		u8 * PlayThreadStack;
		lwp_t ReadThread;
		lwp_t PlayThread;
		mutex_t mutex;

        VideoFile * Video;
		BufferCircle SoundBuffer;
		float fps;
        Timer PlayTime;
        u32 VideoFrameCount;
        safe_vector<STexture> Frames;
		bool Playing;
		bool ExitRequested;
		bool fullScreen;
		int maxSoundSize;
		int SndChannels;
		int SndFrequence;
		int volume;
		
		int screentop;
		int screenleft;
		int screenwidth;
		int screenheight;
		float scaleX;
		float scaleY;
		int width;
		int height;
};

#endif
