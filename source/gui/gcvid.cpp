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
 * gcvid.cpp
 ***************************************************************************/

#include <cstdlib> //NULL
#include <cstring> //memcmp
#include <string>
#include <cassert>

#include "gcvid.h"
#include "loader/utils.h"
#include "memory/mem2.hpp"

using namespace std;

void readThpHeader(FILE* f, ThpHeader& h)
{
	fread(&h, sizeof(h), 1, f);
}

void readThpComponents(FILE* f, ThpComponents& c)
{
	fread(&c, sizeof(c), 1, f);
}

void readThpVideoInfo(FILE* f, ThpVideoInfo& i, bool isVersion11)
{
	fread(&i, sizeof(i), 1, f);
	if(!isVersion11)
	{
		i.unknown = 0;
		fseek(f, -4, SEEK_CUR);
	}
}

void readThpAudioInfo(FILE* f, ThpAudioInfo& i, bool isVersion11)
{
	fread(&i, sizeof(i), 1, f);
	if(!isVersion11)
	{
		i.numData = 1;
		fseek(f, -4, SEEK_CUR);
	}
}

void readMthHeader(FILE* f, MthHeader& h)
{
	fread(&h, sizeof(h), 1, f);
}


struct DecStruct
{
	const u8* currSrcByte;
	u32 blockCount;
	u8 index;
	u8 shift;
};

void thpAudioInitialize(DecStruct& s, const u8* srcStart)
{
	s.currSrcByte = srcStart;
	s.blockCount = 2;
	s.index = (*s.currSrcByte >> 4) & 0x7;
	s.shift = *s.currSrcByte & 0xf;
	++s.currSrcByte;
}

s32 thpAudioGetNewSample(DecStruct& s)
{
	//the following if is executed all 14 calls
	//to thpAudioGetNewSample() (once for each
	//microblock) because mask & 0xf can contain
	//16 different values and starts with 2
	if((s.blockCount & 0xf) == 0)
	{
		s.index = (*s.currSrcByte >> 4) & 0x7;
		s.shift = *s.currSrcByte & 0xf;
		++s.currSrcByte;
		s.blockCount += 2;
	}

	s32 ret;
	if((s.blockCount & 1) != 0)
	{
		s32 t = (*s.currSrcByte	<< 28) & 0xf0000000;
		ret = t >> 28; //this has to be an arithmetic shift
		++s.currSrcByte;
	}
	else
	{
		s32 t = (*s.currSrcByte << 24) & 0xf0000000;
		ret = t >> 28; //this has to be an arithmetic shift
	}

	++s.blockCount;
	return ret;
}

int thpAudioDecode(s16 * destBuffer, const u8* srcBuffer, bool separateChannelsInOutput, bool isInputStereo)
{
	if(destBuffer == NULL || srcBuffer == NULL)
		return 0;

	ThpAudioBlockHeader* head = (ThpAudioBlockHeader*)srcBuffer;

	u32 channelInSize = head->channelSize;
	u32 numSamples = head->numSamples;

	const u8* srcChannel1 = srcBuffer + sizeof(ThpAudioBlockHeader);
	const u8* srcChannel2 = srcChannel1 + channelInSize;

	s16* table1 = head->table1;
	s16* table2 = head->table2;

	s16* destChannel1, * destChannel2;
	u32 delta;

	if(separateChannelsInOutput)
	{
		//separated channels in output
		destChannel1 = destBuffer;
		destChannel2 = destBuffer + numSamples;
		delta = 1;
	}
	else
	{
		//interleaved channels in output
		destChannel1 = destBuffer;
		destChannel2 = destBuffer + 1;
		delta = 2;
	}

	DecStruct s;
	if(!isInputStereo)
	{
		//mono channel in input

		thpAudioInitialize(s, srcChannel1);

		s16 prev1 = *(s16*)(srcBuffer + 72);
		s16 prev2 = *(s16*)(srcBuffer + 74);

		for(u32 i = 0; i < numSamples; ++i)
		{
			s64 res = (s64)thpAudioGetNewSample(s);
			res = ((res << s.shift) << 11); //convert to 53.11 fixedpoint

			//these values are 53.11 fixed point numbers
			s64 val1 = table1[2*s.index];
			s64 val2 = table1[2*s.index + 1];

			//convert to 48.16 fixed point
			res = (val1*prev1 + val2*prev2 + res) << 5;

			//rounding:
			u16 decimalPlaces = res & 0xffff;
			if(decimalPlaces > 0x8000) //i.e. > 0.5
				//round up
				++res;
			else if(decimalPlaces == 0x8000) //i.e. == 0.5
				if((res & 0x10000) != 0)
					//round up every other number
					++res;

			//get nonfractional parts of number, clamp to [-32768, 32767]
			s32 final = (res >> 16);
			if(final > 32767) final = 32767;
			else if(final < -32768) final = -32768;

			prev2 = prev1;
			prev1 = final;
			*destChannel1 = (s16)final;
			*destChannel2 = (s16)final;
			destChannel1 += delta;
			destChannel2 += delta;
		}
	}
	else
	{
		//two channels in input - nearly the same as for one channel,
		//so no comments here (different lines are marked with XXX)

		thpAudioInitialize(s, srcChannel1);
		s16 prev1 = *(s16*)(srcBuffer + 72);
		s16 prev2 = *(s16*)(srcBuffer + 74);
		for(u32 i = 0; i < numSamples; ++i)
		{
			s64 res = (s64)thpAudioGetNewSample(s);
			res = ((res << s.shift) << 11);
			s64 val1 = table1[2*s.index];
			s64 val2 = table1[2*s.index + 1];
			res = (val1*prev1 + val2*prev2 + res) << 5;
			u16 decimalPlaces = res & 0xffff;
			if(decimalPlaces > 0x8000)
				++res;
			else if(decimalPlaces == 0x8000)
				if((res & 0x10000) != 0)
					++res;
			s32 final = (res >> 16);
			if(final > 32767) final = 32767;
			else if(final < -32768) final = -32768;
			prev2 = prev1;
			prev1 = final;
			*destChannel1 = (s16)final;
			destChannel1 += delta;
		}

		thpAudioInitialize(s, srcChannel2);//XXX
		prev1 = *(s16*)(srcBuffer + 76);//XXX
		prev2 = *(s16*)(srcBuffer + 78);//XXX
		for(u32 j = 0; j < numSamples; ++j)
		{
			s64 res = (s64)thpAudioGetNewSample(s);
			res = ((res << s.shift) << 11);
			s64 val1 = table2[2*s.index];//XXX
			s64 val2 = table2[2*s.index + 1];//XXX
			res = (val1*prev1 + val2*prev2 + res) << 5;
			u16 decimalPlaces = res & 0xffff;
			if(decimalPlaces > 0x8000)
				++res;
			else if(decimalPlaces == 0x8000)
				if((res & 0x10000) != 0)
					++res;
			s32 final = (res >> 16);
			if(final > 32767) final = 32767;
			else if(final < -32768) final = -32768;
			prev2 = prev1;
			prev1 = final;
			*destChannel2 = (s16)final;
			destChannel2 += delta;
		}
	}

	return numSamples;
}

void VideoFrame::resize(int width, int height)
{
	if(width == _w && height == _h)
		return;

	dealloc();
	_w = width;
	_h = height;

	//24 bpp, 4 byte padding
	_p = 3*width;
	_p += (4 - _p%4)%4;

	_data = (u8 *)malloc(_p * _h);
}

int VideoFrame::getWidth() const
{ return _w; }

int VideoFrame::getHeight() const
{ return _h; }

int VideoFrame::getPitch() const
{ return _p; }

u8* VideoFrame::getData()
{ return _data; }

const u8* VideoFrame::getData() const
{ return _data; }

void VideoFrame::dealloc()
{
	if(_data != NULL)
		free(_data);
	_data = NULL;
	_w = _h = _p = 0;
}

//swaps red and blue channel of a video frame
void swapRB(VideoFrame& f)
{
	u8* currLine = f.getData();

	int hyt = f.getHeight();
	int pitch = f.getPitch();

	for(int y = 0; y < hyt; ++y)
	{
		for(int x = 0, x2 = 2; x < pitch; x += 3, x2 += 3)
		{
			u8 t = currLine[x];
			currLine[x] = currLine[x2];
			currLine[x2] = t;
		}
		currLine += pitch;
	}
}

enum FILETYPE
{
	THP, MTH, JPG,
	UNKNOWN = -1
};

FILETYPE getFiletype(FILE* f)
{
	long t = ftell(f);
	fseek(f, 0, SEEK_SET);

	u8 buff[4];
	fread(buff, 1, 4, f);

	FILETYPE ret = UNKNOWN;
	if(memcmp("THP\0", buff, 4) == 0)
		ret = THP;
	else if(memcmp("MTHP", buff, 4) == 0)
		ret = MTH;
	else if(buff[0] == 0xff && buff[1] == 0xd8)
		ret = JPG;

	fseek(f, t, SEEK_SET);
	return ret;
}

long getFilesize(FILE* f)
{
	long t = ftell(f);
	fseek(f, 0, SEEK_END);
	long ret = ftell(f);
	fseek(f, t, SEEK_SET);
	return ret;
}

void decodeJpeg(const u8* data, int size, VideoFrame& dest);


VideoFile::VideoFile(FILE* f)
: loop(true), _f(f)
{}

VideoFile::~VideoFile()
{
	fclose(_f);
}

int VideoFile::getWidth() const
{ return 0; }

int VideoFile::getHeight() const
{ return 0; }

float VideoFile::getFps() const
{ return 0.f; }

int VideoFile::getFrameCount() const
{ return 0; }

int VideoFile::getCurrentFrameNr() const
{ return 0; }

bool VideoFile::hasSound() const
{ return false; }

int VideoFile::getNumChannels() const
{ return 0; }

int VideoFile::getFrequency() const
{ return 0; }

int VideoFile::getMaxAudioSamples() const
{ return 0; }

int VideoFile::getCurrentBuffer(s16*) const
{ return 0; }

void VideoFile::loadFrame(VideoFrame& frame, const u8* data, int size) const
{
	decodeJpeg(data, size, frame);
}


ThpVideoFile::ThpVideoFile(FILE* f)
: VideoFile(f)
{
	readThpHeader(f, _head);

	//this is just to find files that have this field != 0, i
	//have no such a file
	assert(_head.offsetsDataOffset == 0);

	readThpComponents(f, _components);
	for(u32 i = 0; i < _components.numComponents; ++i)
	{
		if(_components.componentTypes[i] == 0) //video
			readThpVideoInfo(_f, _videoInfo, _head.version == 0x00011000);
		else if(_components.componentTypes[i] == 1) //audio
		{
			readThpAudioInfo(_f, _audioInfo, _head.version == 0x00011000);
			assert(_head.maxAudioSamples != 0);
		}
	}

	_numInts = 3;
	if(_head.maxAudioSamples != 0)
		_numInts = 4;

	_currFrameNr = -1;
	_nextFrameOffset = _head.firstFrameOffset;
	_nextFrameSize = _head.firstFrameSize;
	_currFrameData.resize(_head.maxBufferSize); //include some padding
	loadNextFrame();
}

int ThpVideoFile::getWidth() const
{ return _videoInfo.width; }

int ThpVideoFile::getHeight() const
{ return _videoInfo.height; }

float ThpVideoFile::getFps() const
{ return _head.fps; }

int ThpVideoFile::getFrameCount() const
{ return _head.numFrames; }

int ThpVideoFile::getCurrentFrameNr() const
{ return _currFrameNr; }

bool ThpVideoFile::loadNextFrame()
{
	++_currFrameNr;
	if(_currFrameNr >= (int) _head.numFrames)
	{
	if (!loop)
		return false;
		
		_currFrameNr = 0;
		_nextFrameOffset = _head.firstFrameOffset;
		_nextFrameSize = _head.firstFrameSize;
	}

	fseek(_f, _nextFrameOffset, SEEK_SET);
	fread(&_currFrameData[0], 1, _nextFrameSize, _f);

	_nextFrameOffset += _nextFrameSize;
	_nextFrameSize = *(u32*)&_currFrameData[0];
	return true;
}

void ThpVideoFile::getCurrentFrame(VideoFrame& f) const
{
	int size = *(u32*)(&_currFrameData[0] + 8);
	loadFrame(f, &_currFrameData[0] + 4*_numInts, size);
}

bool ThpVideoFile::hasSound() const
{ return _head.maxAudioSamples != 0; }

int ThpVideoFile::getNumChannels() const
{
	if(hasSound())
		return _audioInfo.numChannels;
	else
		return 0;
}

int ThpVideoFile::getFrequency() const
{
	if(hasSound())
		return _audioInfo.frequency;
	else
		return 0;
}

int ThpVideoFile::getMaxAudioSamples() const
{ return _head.maxAudioSamples; }

int ThpVideoFile::getCurrentBuffer(s16* data) const
{
	if(!hasSound())
		return 0;

	int jpegSize = *(u32*)(&_currFrameData[0] + 8);
	const u8* src = &_currFrameData[0] + _numInts*4 + jpegSize;

	return thpAudioDecode(data, src, false, _audioInfo.numChannels == 2);
}

MthVideoFile::MthVideoFile(FILE* f)
: VideoFile(f)
{
	readMthHeader(f, _head);

	_currFrameNr = -1;
	_nextFrameOffset = _head.offset;
	_nextFrameSize = _head.firstFrameSize;
	_thisFrameSize = 0;

	_currFrameData.resize(_head.maxFrameSize);
	loadNextFrame();
}


int MthVideoFile::getWidth() const
{ return _head.width; }

int MthVideoFile::getHeight() const
{ return _head.height; }

float MthVideoFile::getFps() const
{
	return (float) 1.0f*_head.fps; //TODO: This has to be in there somewhere
}

int MthVideoFile::getFrameCount() const
{
	return _head.numFrames;
}

int MthVideoFile::getCurrentFrameNr() const
{ return _currFrameNr; }

bool MthVideoFile::loadNextFrame()
{
	++_currFrameNr;
	if(_currFrameNr >= (int) _head.numFrames)
	{
	if (!loop)
		return false;
		_currFrameNr = 0;
		_nextFrameOffset = _head.offset;
		_nextFrameSize = _head.firstFrameSize;
	}

	fseek(_f, _nextFrameOffset, SEEK_SET);
	_currFrameData.resize(_nextFrameSize);
	fread(&_currFrameData[0], 1, _nextFrameSize, _f);
	_thisFrameSize = _nextFrameSize;

	u32 nextSize;
	nextSize = *(u32*)(&_currFrameData[0]);
	_nextFrameOffset += _nextFrameSize;
	_nextFrameSize = nextSize;
	return true;
}

void MthVideoFile::getCurrentFrame(VideoFrame& f) const
{
	int size = _thisFrameSize;
	loadFrame(f, &_currFrameData[0] + 4, size - 4);
}


JpgVideoFile::JpgVideoFile(FILE* f)
: VideoFile(f)
{
	vector<u8> data(getFilesize(f));
	fread(&data[0], 1, getFilesize(f), f);

	loadFrame(_currFrame, &data[0], getFilesize(f));
}

int JpgVideoFile::getWidth() const
{ return _currFrame.getWidth(); }

int JpgVideoFile::getHeight() const
{ return _currFrame.getHeight(); }

int JpgVideoFile::getFrameCount() const
{ return 1; }

void JpgVideoFile::getCurrentFrame(VideoFrame& f) const
{
	f.resize(_currFrame.getWidth(), _currFrame.getHeight());
	memcpy(f.getData(), _currFrame.getData(),f.getPitch()*f.getHeight());
}

VideoFile* openVideo(const string& fileName)
{
	FILE* f = fopen(fileName.c_str(), "rb");
	if(f == NULL)
		return NULL;

	FILETYPE type = getFiletype(f);
	switch(type)
	{
		case THP:
			return new ThpVideoFile(f);
		case MTH:
			return new MthVideoFile(f);
		case JPG:
			return new JpgVideoFile(f);

		default:
			fclose(f);
			return NULL;
	}
}

void closeVideo(VideoFile*& vf)
{
	if(vf != NULL)
		delete vf;
	vf = NULL;
}

//as mentioned above, we have to convert 0xff to 0xff 0x00
//after the image date has begun (ie, after the 0xff 0xda marker)
//but we must not convert the end-of-image-marker (0xff 0xd9)
//this way. There may be 0xff 0xd9 bytes embedded in the image
//data though, so I add 4 bytes to the input buffer
//and fill them with zeroes and check for 0xff 0xd9 0 0
//as end-of-image marker. this is not correct, but works
//and is easier to code... ;-)
//a better solution would be to patch jpeglib so that this conversion
//is not neccessary

u8 endBytesThp[] = { 0xff, 0xd9, 0, 0 }; //used in thp files
u8 endBytesMth[] = { 0xff, 0xd9, 0xff, 0 }; //used in mth files

int countRequiredSize(const u8* data, int size, int& start, int& end)
{
	start = 2*size;
	end = size;
	int count = 0;

	int j;
	for(j = size - 1; data[j] == 0; --j)
		; //search end of data

	if(data[j] == 0xd9) //thp file
		end = j - 1;
	else if(data[j] == 0xff) //mth file
		end = j - 2;

	for(int i = 0; i < end; ++i)
	{
		if(data[i] == 0xff)
		{
			//if i == srcSize - 1, then this would normally overrun src - that's why 4 padding
			//bytes are included at the end of src
			if(data[i + 1] == 0xda && start == 2*size)
				start = i;
			if(i > start)
				++count;
		}
	}
	return size + count;
}

void convertToRealJpeg(u8* dest, const u8* src, int srcSize, int start, int end)
{
	int di = 0;
	for(int i = 0; i < srcSize; ++i, ++di)
	{
		dest[di] = src[i];
		//if i == srcSize - 1, then this would normally overrun src - that's why 4 padding
		//bytes are included at the end of src
		if(src[i] == 0xff && i > start && i < end)
		{
			++di;
			dest[di] = 0;
		}
	}
}

void decodeJpeg(const u8* data, int size, VideoFrame& dest)
{
	//convert format so jpeglib understands it...
	int start, end;
	int newSize = countRequiredSize(data, size, start, end);
	u8 *buff = (u8*)MEM2_alloc(newSize);
	convertToRealJpeg(buff, data, size, start, end);

	//...and feed it to jpeglib
	decodeRealJpeg(buff, newSize, dest);
	MEM2_free(buff);
}

extern "C"
{
#include "jpeglib.h"
#include <setjmp.h>
}

//the following functions are needed to let
//libjpeg read from memory instead of from a file...
//it's a little clumsy to do :-|
const u8* g_jpegBuffer;
int g_jpegSize;
bool g_isLoading = false;

void jpegInitSource(j_decompress_ptr)
{}

boolean jpegFillInputBuffer(j_decompress_ptr cinfo)
{
	cinfo->src->next_input_byte = g_jpegBuffer;
	cinfo->src->bytes_in_buffer = g_jpegSize;
	return TRUE;
}

void jpegSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
	cinfo->src->next_input_byte += num_bytes;
	cinfo->src->bytes_in_buffer -= num_bytes;
}

boolean jpegResyncToRestart(j_decompress_ptr cinfo, int desired)
{
	jpeg_resync_to_restart(cinfo, desired);
	return TRUE;
}

void jpegTermSource(j_decompress_ptr)
{}

void jpegErrorHandler(j_common_ptr cinfo)
{
	char buff[1024];
	(*cinfo->err->format_message)(cinfo, buff);
	//MessageBox(g_hWnd, buff, "JpegLib error:", MB_OK);
}

void decodeRealJpeg(const u8* data, int size, VideoFrame& dest, bool fancy)
{
	if(g_isLoading)
		return;
	g_isLoading = true;

	//decompressor state
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr errorMgr;

	//read from memory manager
	jpeg_source_mgr sourceMgr;

	cinfo.err = jpeg_std_error(&errorMgr);
	errorMgr.error_exit = jpegErrorHandler;

	jpeg_create_decompress(&cinfo);

	//setup read-from-memory
	g_jpegBuffer = data;
	g_jpegSize = size;
	sourceMgr.bytes_in_buffer = size;
	sourceMgr.next_input_byte = data;
	sourceMgr.init_source = jpegInitSource;
	sourceMgr.fill_input_buffer = jpegFillInputBuffer;
	sourceMgr.skip_input_data = jpegSkipInputData;
	sourceMgr.resync_to_restart = jpegResyncToRestart;
	sourceMgr.term_source = jpegTermSource;
	cinfo.src = &sourceMgr;

	jpeg_read_header(&cinfo, TRUE);
	if(fancy)
	{
		cinfo.do_fancy_upsampling = TRUE;
		cinfo.do_block_smoothing = TRUE;
		cinfo.dct_method = JDCT_ISLOW;
		jpeg_start_decompress(&cinfo);
		dest.resize(ALIGN(4, cinfo.output_width), ALIGN(4, cinfo.output_height));
	}
	else
	{
		cinfo.do_fancy_upsampling = FALSE;
		cinfo.do_block_smoothing = FALSE;
		jpeg_start_decompress(&cinfo);
		dest.resize(cinfo.output_width, cinfo.output_height);
	}
	if(cinfo.num_components == 3)
	{
		int y = 0;
		while(cinfo.output_scanline < cinfo.output_height)
		{
			//invert image because windows wants it downside up
			u8* destBuffer =	&dest.getData()[(dest.getHeight() - y - 1)*dest.getPitch()];

			//NO idea why jpeglib wants a pointer to a pointer
			jpeg_read_scanlines(&cinfo, &destBuffer, 1);
			++y;
		}

		//jpeglib gives an error in jpeg_finish_decompress() if no all
		//scanlines are read by the application... :-|
		//(but because we read all scanlines, it's not really needed)
		cinfo.output_scanline = cinfo.output_height;

	}
	else
	{
		//MessageBox(g_hWnd, "Only RGB videos are currently supported.", "oops?", MB_OK);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	g_isLoading = false;
}
