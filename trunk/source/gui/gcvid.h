/***************************************************************************
 * Copyright (C) 2010
 * by thakis
 *
 * Modification and adjustment for the Wii by Dimok
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
 * gcvid.h
 ***************************************************************************/

#ifndef THAKIS_GCVID_H
#define THAKIS_GCVID_H THAKIS_GCVID_H

#include <stdio.h> //FILE*
#include <string>
#include <vector>
using namespace std;

#include <gccore.h>

#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
//THP

struct ThpHeader
{
  char tag[4]; //'THP\0'

  //from monk's thp player:
  u32 version; //0x00011000 = 1.1, 0x00010000 = 1.0
  u32 maxBufferSize;
  u32 maxAudioSamples; //!= 0 if sound is stored in file


  float fps; //usually 29.something (=0x41efc28f) for ntsc
  u32 numFrames;
  u32 firstFrameSize; //size of first frame

  u32 dataSize; //size of file - ThpHeader.offset

  //from monk's thp player:
  u32 componentDataOffset; //ThpComponents stored here
  u32 offsetsDataOffset; //?? if != 0, offset to table with offsets of all frames?

  u32 firstFrameOffset;
  u32 lastFrameOffset;
};

//monk:

struct ThpComponents
{
  u32 numComponents; //usually 1 or 2 (video or video + audio)

  //component type 0 is video, type 1 is audio,
  //type 0xff is "no component" (numComponent many entries
  //are != 0xff)
  u8 componentTypes[16];
};

struct ThpVideoInfo
{
  u32 width;
  u32 height;
  u32 unknown; //only for version 1.1 thp files
};

struct ThpAudioInfo
{
  u32 numChannels;
  u32 frequency;
  u32 numSamples;
  u32 numData; //only for version 1.1 - that many
               //audio blocks are after each video block
               //(for surround sound?)
};

//endmonk


//a frame image is basically a normal jpeg image (without
//the jfif application marker), the only important difference
//is that after the image start marker (0xff 0xda) values
//of 0xff are simply written as 0xff whereas the jpeg
//standard requires them to be written as 0xff 0x00 because
//0xff is the start of a 2-byte control code in jpeg

//frame (offsets relative to frame start):
//u32 total (image, sound, etc) size of NEXT frame
//u32 size1 at 0x04 (total size of PREV frame according to monk)
//u32 image data size at 0x08
//size of one audio block ONLY IF THE FILE HAS SOUND. ThpAudioInfo.numData
//many audio blocks after jpeg data
//jpeg data
//audio block(s)

struct ThpAudioBlockHeader
{
  //size 80 byte
  u32 channelSize; //size of one channel in bytes
  u32 numSamples; //number of samples/channel
  s16 table1[16]; //table for first channel
  s16 table2[16]; //table for second channel
  s16 channel1Prev1;
  s16 channel1Prev2;
  s16 channel2Prev1;
  s16 channel2Prev2;
};

//audio block:
//u32 size of this audioblock
//
//u32 numBytes/channel of audioblock - that many bytes per channel after adpcm table)
//
//u32 number of samples per channel
//
//2*16 shorts adpcm table (one per channel - always stored both,
//even for mono files), 5.11 fixed point values
//
//4 s16: 2 shorts prev1 and prev2 for each channel (even for mono files)
//
//sound data

//sound data:
//8 byte are 14 samples:
//the first byte stores index (upper nibble) and shift (lower nibble),
//the following 7 bytes contain 14o samples a 4 bit each


/////////////////////////////////////////////////////////////////////
//MTH ("mute thp"?)

//similar to a thp file, but without sound

struct MthHeader
{
  //one of the unknown has to be fps in some form

  char tag[4]; //'MTHP'
  u32 unknown;
  u32 unknown2;
  u32 maxFrameSize;

  u32 width;
  u32 height;
  u32 fps;
  u32 numFrames;

  u32 offset;
  u32 unknown5;
  u32 firstFrameSize;

  //5 padding u32's follow
};

//frame:
//u32 size of NEXT frame
//jpeg data

//see thp (above) for jpeg format. there's a small difference, though:
//mth jpegs end with 0xff 0xd9 0xff instead of 0xff 0xd9

#pragma pack(pop)

//little helper class that represents one frame of video
//data is 24 bpp, scanlines aligned to 4 byte boundary
class VideoFrame
{
 public:
  VideoFrame() : _data(NULL), _w(0), _h(0), _p(0) { };
  ~VideoFrame() { };

  void resize(int width, int height);

  int getWidth() const;
  int getHeight() const;
  int getPitch() const;
  u8* getData();
  const u8* getData() const;

  void dealloc();
  
 private:
  u8* _data;
  int _w;
  int _h;
  int _p; //pitch in bytes

  //copy constructor and asignment operator are not allowed
  //VideoFrame(const VideoFrame& f);
  VideoFrame& operator=(const VideoFrame& f);
};

//swaps red and blue channel of a video frame
void swapRB(VideoFrame& f);


class VideoFile
{
 public:
  VideoFile(FILE* f);
  virtual ~VideoFile();


  virtual int getWidth() const;
  virtual int getHeight() const;
  virtual float getFps() const;
  virtual int getFrameCount() const;
  virtual int getCurrentFrameNr() const;

  virtual bool loadNextFrame() = 0;

  virtual void getCurrentFrame(VideoFrame& frame) const = 0;

  //sound support:
  virtual bool hasSound() const;
  virtual int getNumChannels() const;
  virtual int getFrequency() const;
  virtual int getMaxAudioSamples() const;
  virtual int getCurrentBuffer(s16* data) const;

  bool loop;
 protected:

  FILE* _f;

  //void loadFrame(long offset, int size);
  void loadFrame(VideoFrame& frame, const u8* data, int size) const;

};

VideoFile* openVideo(const std::string& fileName);
void closeVideo(VideoFile*& vf);


class ThpVideoFile : public VideoFile
{
 public:
  ThpVideoFile(FILE* f);

  virtual int getWidth() const;
  virtual int getHeight() const;
  virtual float getFps() const;
  virtual int getFrameCount() const;

  virtual int getCurrentFrameNr() const;

  virtual bool loadNextFrame();

  virtual void getCurrentFrame(VideoFrame& frame) const;

  virtual bool hasSound() const;
  virtual int getNumChannels() const;
  virtual int getFrequency() const;
  virtual int getMaxAudioSamples() const;
  virtual int getCurrentBuffer(s16* data) const;


 protected:
  ThpHeader _head;
  ThpComponents _components;
  ThpVideoInfo _videoInfo;
  ThpAudioInfo _audioInfo;
  int _numInts;

  int _currFrameNr;
  int _nextFrameOffset;
  int _nextFrameSize;
  vector<u8> _currFrameData;
};

class MthVideoFile : public VideoFile
{
 public:
  MthVideoFile(FILE* f);

  virtual int getWidth() const;
  virtual int getHeight() const;
  virtual float getFps() const;
  virtual int getFrameCount() const;

  virtual int getCurrentFrameNr() const;

  virtual bool loadNextFrame();

  virtual void getCurrentFrame(VideoFrame& frame) const;

 protected:
  MthHeader _head;

  int _currFrameNr;
  int _nextFrameOffset;
  int _nextFrameSize;
  int _thisFrameSize;
  vector<u8> _currFrameData;
};

class JpgVideoFile : public VideoFile
{
 public:
  JpgVideoFile(FILE* f);

  virtual int getWidth() const;
  virtual int getHeight() const;
  virtual int getFrameCount() const;

  virtual bool loadNextFrame() { return false; }
  virtual void getCurrentFrame(VideoFrame& frame) const;

 private:
   VideoFrame _currFrame;
};

void decodeRealJpeg(const u8* data, int size, VideoFrame& dest, bool fancy = false);

#endif //THAKIS_GCVID_H
