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
#include <string.h>
#include "WavDecoder.hpp"

WavDecoder::WavDecoder(const char * filepath)
	: SoundDecoder(filepath)
{
	SoundType = SOUND_WAV;
	SampleRate = 48000;
	Format = VOICE_STEREO_16BIT;

	if(!file_fd)
		return;

	OpenFile();
}

WavDecoder::WavDecoder(const u8 * snd, int len)
	: SoundDecoder(snd, len)
{
	SoundType = SOUND_WAV;
	SampleRate = 48000;
	Format = VOICE_STEREO_16BIT;

	if(!file_fd)
		return;

	OpenFile();
}

WavDecoder::~WavDecoder()
{
	CloseFile();
}

void WavDecoder::OpenFile()
{
	DataOffset = 0;
	SWaveHdr Header;
	SWaveFmtChunk FmtChunk;
	memset(&Header, 0, sizeof(SWaveHdr));
	memset(&FmtChunk, 0, sizeof(SWaveFmtChunk));

	file_fd->read((u8 *) &Header, sizeof(SWaveHdr));
	file_fd->read((u8 *) &FmtChunk, sizeof(SWaveFmtChunk));

	if(Header.magicRIFF != 'RIFF')
	{
		CloseFile();
		return;
	}
	else if(Header.magicWAVE != 'WAVE')
	{
		CloseFile();
		return;
	}
	if(FmtChunk.magicFMT == 'bext') //Stupid metadata
	{
		DataOffset += le32(FmtChunk.size) + 8;
		file_fd->seek(sizeof(SWaveHdr) + le32(FmtChunk.size) + 8, SEEK_SET);
		file_fd->read((u8 *)&FmtChunk, sizeof(SWaveFmtChunk));
	}
	if(FmtChunk.magicFMT != 'fmt ')
	{
		CloseFile();
		return;
	}

	DataOffset += sizeof(SWaveHdr) + le32(FmtChunk.size) + 8;
	file_fd->seek(DataOffset, SEEK_SET);
	SWaveChunk DataChunk;
	file_fd->read((u8 *) &DataChunk, sizeof(SWaveChunk));

	while(DataChunk.magicDATA != 'data')
	{
		DataOffset += 8+le32(DataChunk.size);
		file_fd->seek(DataOffset, SEEK_SET);
		int ret = file_fd->read((u8 *) &DataChunk, sizeof(SWaveChunk));
		if(ret <= 0)
		{
			CloseFile();
			return;
		}
	}

	DataOffset += 8;
	DataSize = le32(DataChunk.size);
	Is16Bit = (le16(FmtChunk.bps) == 16);
	SampleRate = le32(FmtChunk.freq);

	if(le16(FmtChunk.channels) == 1 && le16(FmtChunk.bps) == 8 && le16(FmtChunk.alignment) <= 1)
		Format = VOICE_MONO_8BIT;
	else if(le16(FmtChunk.channels) == 1 && le16(FmtChunk.bps) == 16 && le16(FmtChunk.alignment) <= 2)
		Format = VOICE_MONO_16BIT;
	else if(le16(FmtChunk.channels) == 2 && le16(FmtChunk.bps) == 8 && le16(FmtChunk.alignment) <= 2)
		Format = VOICE_STEREO_8BIT;
	else if(le16(FmtChunk.channels) == 2 && le16(FmtChunk.bps) == 16 && le16(FmtChunk.alignment) <= 4)
		Format = VOICE_STEREO_16BIT;

	SWaveChunk LoopChunk;
	SWaveSmplChunk SmplChunk;
	SmplChunk.Start = 0;
	SmplChunk.End = 0;
	file_fd->seek(DataOffset + DataSize, SEEK_SET);
	while(file_fd->read((u8 *)&LoopChunk, sizeof(SWaveChunk)) == sizeof(SWaveChunk))
	{
		if(LoopChunk.magicDATA == 'smpl')
		{
			file_fd->seek(-8, SEEK_CUR);
			file_fd->read((u8*)&SmplChunk, sizeof(SWaveSmplChunk));
			SmplChunk.Start = ((le32(SmplChunk.Start) * le16(FmtChunk.channels) * le16(FmtChunk.bps) / 8) + 8191) & ~8191;
			SmplChunk.End = ((le32(SmplChunk.End) * le16(FmtChunk.channels) * le16(FmtChunk.bps) / 8) + 8191) & ~8191;
			break;
		}
		file_fd->seek(le32(LoopChunk.size), SEEK_CUR);
	}
	SetLoopStart(SmplChunk.Start);
	SetLoopEnd(SmplChunk.End);
	Decode();
}

void WavDecoder::CloseFile()
{
	if(file_fd)
		delete file_fd;

	file_fd = NULL;
}

int WavDecoder::Read(u8 * buffer, int buffer_size, int)
{
	if(!file_fd)
		return -1;

	if(CurPos >= (int) DataSize)
		return 0;

	file_fd->seek(DataOffset+CurPos, SEEK_SET);

	if(buffer_size > (int) DataSize-CurPos)
		buffer_size = DataSize-CurPos;

	int read = file_fd->read(buffer, buffer_size);
	if(read > 0)
	{
		if(Is16Bit)
		{
			read &= ~0x0001;

			for(u32 i = 0; i < (u32) (read / sizeof (u16)); ++i)
				((u16 *)buffer)[i] = le16(((u16 *)buffer)[i]);
		}
		CurPos += read;
	}

	return read;
}
