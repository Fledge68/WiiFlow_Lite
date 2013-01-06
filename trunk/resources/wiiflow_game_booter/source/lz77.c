/*******************************************************************************
 * lz77.c
 *
 * Copyright (c) 2009 The Lemon Man
 * Copyright (c) 2009 Nicksasa
 * Copyright (c) 2009 WiiPower
 *
 * Distributed under the terms of the GNU General Public License (v2)
 * See http://www.gnu.org/licenses/gpl-2.0.txt for more info.
 *
 * Description:
 * -----------
 *
 ******************************************************************************/
#include <gccore.h>
#include <stdlib.h>
#include <malloc.h>
#include "lz77.h"

static inline u32 packBytes(int a, int b, int c, int d)
{
	return (d << 24) | (c << 16) | (b << 8) | (a);
}

static int __decompressLZ77_11(const u8 *in, u32 inputLen, u8 **output, u32 *outputLen)
{
	int x = 0;
	u32 y = 0;
	u32 compressedPos = 0x4;
	u32 decompressedPos = 0;
	u32 decompressedSize = packBytes(in[0], in[1], in[2], in[3]) >> 8;
	if(decompressedSize == 0)
	{
		decompressedSize = packBytes(in[4], in[5], in[6], in[7]);
		compressedPos += 0x4;
	}
	//printf("Decompressed size : %i\n", decompressedSize);

	u8 *out = (u8*)malloc(decompressedSize);
	if(out == NULL)
	{
		//printf("Out of memory\n");
		return -1;
	}

	while(compressedPos < inputLen && decompressedPos < decompressedSize)
	{
		u8 byteFlag = in[compressedPos];
		compressedPos++;

		for(x = 7; x >= 0; x--)
		{
			if((byteFlag & (1 << x)) > 0)
			{
				u8 first = in[compressedPos];
				u8 second = in[compressedPos + 1];

				u32 pos, copyLen;
				if(first < 0x20)
				{
					u8 third = in[compressedPos + 2];
					if(first >= 0x10)
					{
						u32 fourth = in[compressedPos + 3];
						pos = (u32)(((third & 0xF) << 8) | fourth) + 1;
						copyLen = (u32)((second << 4) | ((first & 0xF) << 12) | (third >> 4)) + 273;
						compressedPos += 4;
					}
					else
					{
						pos = (u32)(((second & 0xF) << 8) | third) + 1;
						copyLen = (u32)(((first & 0xF) << 4) | (second >> 4)) + 17;
						compressedPos += 3;
					}
				}
				else
				{
					pos = (u32)(((first & 0xF) << 8) | second) + 1;
					copyLen = (u32)(first >> 4) + 1;
					compressedPos += 2;
				}
				for(y = 0; y < copyLen; y++)
					out[decompressedPos + y] = out[decompressedPos - pos + y];
				decompressedPos += copyLen;
			}
			else
			{
				out[decompressedPos] = in[compressedPos];
				decompressedPos++;
				compressedPos++;
			}
			if(compressedPos >= inputLen || decompressedPos >= decompressedSize)
				break;
		}
	}
	*output = out;
	*outputLen = decompressedSize;
	return 0;
}

static int __decompressLZ77_10(const u8 *in, u8 **output, u32 *outputLen)
{
	int x = 0;
	u32 y = 0;
	u32 compressedPos = 0x4;
	u32 decompressedPos = 0;
	u32 decompressedSize = packBytes(in[0], in[1], in[2], in[3]) >> 8;
	//printf("Decompressed size : %i\n", decompressedSize);

	u8 *out = (u8*)malloc(decompressedSize);
	if(out == NULL)
	{
		//printf("Out of memory\n");
		return -1;
	}

	while(decompressedPos < decompressedSize)
	{
		u8 byteFlag = in[compressedPos];
		compressedPos ++;

		for(x = 0; x < 8; ++x)
		{
			if(byteFlag & 0x80)
			{
				u8 first = in[compressedPos];
				u8 second = in[compressedPos + 1];
				u16 pos = (u16)((((first << 8) + second) & 0xFFF) + 1);
				u8 copyLen = (u8)(3 + ((first >> 4) & 0xF));
				for(y = 0; y < copyLen; y++)
					out[decompressedPos + y] = out[decompressedPos - pos + (y % pos)];
				compressedPos += 2;
				decompressedPos += copyLen;
			}
			else
			{
				out[decompressedPos] = in[compressedPos];
				compressedPos += 1;
				decompressedPos += 1;
			}
			byteFlag <<= 1;
			if(decompressedPos >= decompressedSize)
				break;
		}
	}
	*output = out;
	*outputLen = decompressedSize;
	return 0;
}

int isLZ77compressed(const u8 *buffer)
{
	if((buffer[0] == LZ77_0x10_FLAG) || (buffer[0] == LZ77_0x11_FLAG))
		return 1;
	return 0;
}

int decompressLZ77content(const u8 *buffer, u32 length, u8 **output, u32 *outputLen)
{
	int ret = 0;
	switch(buffer[0])
	{
		case LZ77_0x10_FLAG:
			//printf("LZ77 variant 0x10 compressed content...unpacking may take a while...\n");
			ret = __decompressLZ77_10(buffer, output, outputLen);
			break;
		case LZ77_0x11_FLAG:
			//printf("LZ77 variant 0x11 compressed content...unpacking may take a while...\n");
			ret = __decompressLZ77_11(buffer, length, output, outputLen);
			break;
		default:
			//printf("Not compressed ...\n");
			ret = -1;
			break;
	}
	return ret;
}
