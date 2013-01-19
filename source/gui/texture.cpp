#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ogcsys.h>
#include <malloc.h>
#include <cmath>

#include "texture.hpp"
#include "coverflow.hpp"
#include "memory/mem2.hpp"
#include "pngu.h"
#include "gcvid.h"

using namespace std;

STexture TexHandle;

static u32 upperPower(u32 width)
{
	u32 i = 8;
	u32 maxWidth = 1090;
	while (i < width && i < maxWidth)
		i <<= 1;
	return i;
}

u32 fixGX_GetTexBufferSize(u16 wd, u16 ht, u32 fmt, u8 mipmap, u8 maxlod)
{
	if (mipmap) return GX_GetTexBufferSize(wd, ht, fmt, mipmap, maxlod + 1);
	return GX_GetTexBufferSize(wd, ht, fmt, mipmap, maxlod);
}

static inline u32 coordsRGBA8(u32 x, u32 y, u32 w)
{
	return ((((y >> 2) * (w >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
}

static inline u32 coordsRGB565(u32 x, u32 y, u32 w)
{
	return (((y >> 2) * (w >> 2) + (x >> 2)) << 4) + ((y & 3) << 2) + (x & 3);
}

static inline void _convertToFlippedRGBA(u8 *dst, const u8 *src, u32 width, u32 height)
{
	for(u32 block = 0; block < height; block += 4)
	{
		for(u32 i = 0; i < width; i += 4)
		{
			for(u32 c = 0; c < 4; ++c)
			{
				for(u32 rgb = 0; rgb < 4; ++rgb)
				{
					u32 y = height - 1 - (c + block);
					u32 x = rgb + i;
					u32 dst_offset = (x + y * width) * 4;
					u32 src_offset = (x + ((block + c) * width)) * 3;
					/* RGB */
					memcpy(&dst[dst_offset], &src[src_offset], 3);
					/* Alpha */
					dst[dst_offset + 3] = 0xFF;
				}
			}
		}
	}
}

static inline void _convertToRGBA(u8 *dst, const u8 *src, u32 width, u32 height)
{
	for (u32 y = 0; y < height; ++y)
	{
		for (u32 x = 0; x < width; ++x)
		{
			u32 i = (x + y * width) * 4;
			dst[i] = src[coordsRGBA8(x, y, width) + 1];
			dst[i + 1] = src[coordsRGBA8(x, y, width) + 32];
			dst[i + 2] = src[coordsRGBA8(x, y, width) + 33];
			dst[i + 3] = src[coordsRGBA8(x, y, width)];
		}
	}
}

static inline void _convertToRGBA8(u8 *dst, const u8 *src, u32 width, u32 height)
{
	for (u32 y = 0; y < height; ++y)
		for (u32 x = 0; x < width; ++x)
		{
			u32 i = (x + y * width) * 4;
			dst[coordsRGBA8(x, y, width) + 1] = src[i];
			dst[coordsRGBA8(x, y, width) + 32] = src[i + 1];
			dst[coordsRGBA8(x, y, width) + 33] = src[i + 2];
			dst[coordsRGBA8(x, y, width)] = src[i + 3];
		}
}

static inline void _convertToRGB565(u8 *dst, const u8 *src, u32 width, u32 height)
{
	u16 *dst16 = (u16 *)dst;

	for (u32 y = 0; y < height; ++y)
		for (u32 x = 0; x < width; ++x)
		{
			u32 i = (x + y * width) * 4;
			dst16[coordsRGB565(x, y, width)] = ((src[i] & 0xF8) << 8) | ((src[i + 1] & 0xFC) << 3) | (src[i + 2] >> 3);
		}
}

static inline u16 rgb8ToRGB565(u8 *color)
{
	return ((color[0] >> 3) << 11) | ((color[1] >> 2) << 5) | (color[2] >> 3);
}

static int colorDistance(const u8 *c0, const u8 *c1)
{
	return (c1[0] - c0[0]) * (c1[0] - c0[0]) + (c1[1] - c0[1]) * (c1[1] - c0[1]) + (c1[2] - c0[2]) * (c1[2] - c0[2]);
}

static void getBaseColors(u8 *color0, u8 *color1, const u8 *srcBlock)
{
	int maxDistance = -1;
	for (int i = 0; i < 15; ++i)
		for (int j = i + 1; j < 16; ++j)
		{
			int distance = colorDistance(srcBlock + i * 4, srcBlock + j * 4);
			if (distance > maxDistance)
			{
				maxDistance = distance;
				*(u32 *)color0 = ((u32 *)srcBlock)[i];
				*(u32 *)color1 = ((u32 *)srcBlock)[j];
			}
		}
	if (rgb8ToRGB565(color0) < rgb8ToRGB565(color1))
	{
		u32 tmp;
		tmp = *(u32 *)color0;
		*(u32 *)color0 = *(u32 *)color1;
		*(u32 *)color1 = tmp;
	}
}

static u32 colorIndices(const u8 *color0, const u8 *color1, const u8 *srcBlock)
{
	u16 colors[4][4];
	u32 res = 0;

	// Make the 4 colors available in the block
	colors[0][0] = (color0[0] & 0xF8) | (color0[0] >> 5);
	colors[0][1] = (color0[1] & 0xFC) | (color0[1] >> 6);
	colors[0][2] = (color0[2] & 0xF8) | (color0[2] >> 5);
	colors[1][0] = (color1[0] & 0xF8) | (color1[0] >> 5);
	colors[1][1] = (color1[1] & 0xFC) | (color1[1] >> 6);
	colors[1][2] = (color1[2] & 0xF8) | (color1[2] >> 5);
	colors[2][0] = (2 * colors[0][0] + 1 * colors[1][0]) / 3;
	colors[2][1] = (2 * colors[0][1] + 1 * colors[1][1]) / 3;
	colors[2][2] = (2 * colors[0][2] + 1 * colors[1][2]) / 3;
	colors[3][0] = (1 * colors[0][0] + 2 * colors[1][0]) / 3;
	colors[3][1] = (1 * colors[0][1] + 2 * colors[1][1]) / 3;
	colors[3][2] = (1 * colors[0][2] + 2 * colors[1][2]) / 3;
	for (int i = 15; i >= 0; --i)
	{
		int c0 = srcBlock[i * 4 + 0];
		int c1 = srcBlock[i * 4 + 1];
		int c2 = srcBlock[i * 4 + 2];
		int d0 = abs(colors[0][0] - c0) + abs(colors[0][1] - c1) + abs(colors[0][2] - c2);
		int d1 = abs(colors[1][0] - c0) + abs(colors[1][1] - c1) + abs(colors[1][2] - c2);
		int d2 = abs(colors[2][0] - c0) + abs(colors[2][1] - c1) + abs(colors[2][2] - c2);
		int d3 = abs(colors[3][0] - c0) + abs(colors[3][1] - c1) + abs(colors[3][2] - c2);
		int b0 = d0 > d3;
		int b1 = d1 > d2;
		int b2 = d0 > d2;
		int b3 = d1 > d3;
		int b4 = d2 > d3;
		int x0 = b1 & b2;
		int x1 = b0 & b3;
		int x2 = b0 & b4;
		res |= (x2 | ((x0 | x1) << 1)) << ((15 - i) << 1);
	}
	return res;
}

static inline void _convertToCMPR(u8 *dst, const u8 *src, u32 width, u32 height)
{
	u8 srcBlock[16 * 4];
	u8 color0[4];
	u8 color1[4];

	for (u32 jj = 0; jj < height; jj += 8)
		for (u32 ii = 0; ii < width; ii += 8)
			for (u32 k = 0; k < 4; ++k)
			{
				int i = ii + ((k & 1) << 2);
				int j = jj + ((k >> 1) << 2);
				memcpy(srcBlock, src + (j * width + i) * 4, 16);
				memcpy(srcBlock + 4 * 4, src + ((j + 1) * width + i) * 4, 16);
				memcpy(srcBlock + 8 * 4, src + ((j + 2) * width + i) * 4, 16);
				memcpy(srcBlock + 12 * 4, src + ((j + 3) * width + i) * 4, 16);
				getBaseColors(color0, color1, srcBlock);
				*(u16 *)dst = rgb8ToRGB565(color0);
				dst += 2;
				*(u16 *)dst = rgb8ToRGB565(color1);
				dst += 2;
				*(u32 *)dst = colorIndices(color0, color1, srcBlock);
				dst += 4;
			}
}

void STexture::Cleanup(TexData &tex)
{
	if(tex.data != NULL)
		free(tex.data);
	tex.data = NULL;
	tex.dataSize = 0;
	tex.width = 0;
	tex.height = 0;
	tex.format = -1;
	tex.maxLOD = 0;
}

bool STexture::CopyTexture(const TexData &src, TexData &dest)
{
	if(src.data == NULL || src.dataSize == 0 || src.data == dest.data)
		return false;
	if(src.dataSize != dest.dataSize)
		Cleanup(dest);
	if(dest.data == NULL)
		dest.data = (u8*)MEM2_alloc(src.dataSize);
	if(dest.data == NULL)
		return false;
	dest.dataSize = src.dataSize;
	memcpy(dest.data, src.data, dest.dataSize);
	DCFlushRange(dest.data, dest.dataSize);
	dest.width = src.width;
	dest.height = src.height;
	dest.format = src.format;
	dest.maxLOD = src.maxLOD;
	return true;
}

TexErr STexture::fromImageFile(TexData &dest, const char *filename, u8 f, u32 minMipSize, u32 maxMipSize)
{
	Cleanup(dest);
	FILE *file = fopen(filename, "rb");
	if(file == NULL)
	{
		strncpy((char*)filename+(strlen(filename)-3), "jp", 2);
		file = fopen(filename, "rb");
	}
	if(file == NULL)
		return TE_ERROR;

	fseek(file, 0, SEEK_END);
	u32 fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	u8 *Image = NULL;
	if(fileSize)
	{
		Image = (u8*)MEM2_alloc(fileSize);
		if(Image != NULL)
			fread(Image, 1, fileSize, file);
	}
	fclose(file);

	TexErr result = TE_NOMEM;
	if(Image != NULL)
	{
		if(*(vu32*)Image == 0x89504E47) /* PNG Magic */
			result = fromPNG(dest, Image, f, minMipSize, maxMipSize);
		else
			result = fromJPG(dest, Image, fileSize, f, minMipSize, maxMipSize);
		free(Image);
	}
	return result;
}

TexErr STexture::fromTHP(TexData &dest, const u8 *src, u32 w, u32 h)
{
	dest.width = w;
	dest.height = h;
	dest.format = GX_TF_RGBA8;
	dest.dataSize = GX_GetTexBufferSize(dest.width, dest.height, dest.format, GX_FALSE, 0);
	dest.data = (u8*)MEM2_alloc(dest.dataSize);
	if(dest.data == NULL)
	{
		Cleanup(dest);
		return TE_NOMEM;
	}
	for(u32 block = 0; block < dest.height; block += 4)
	{
		for(u32 i = 0; i < dest.width; i += 4)
		{
			for(u32 c = 0; c < 4; ++c)
			{
				for(u32 argb = 0; argb < 4; ++argb)
				{
					u32 y = h - 1 - (c + block);
					u32 x = argb + i;
					u32 src_offset = ((i + argb) + ((block + c) * dest.width)) * 3;
					u32 dst_offset = coordsRGBA8(x, y, dest.width);
					/* Alpha */
					dest.data[dst_offset] = 0xFF;
					/* RGB */
					dest.data[dst_offset + 1] = src[src_offset];
					dest.data[dst_offset + 32] = src[src_offset + 1];
					dest.data[dst_offset + 33] = src[src_offset + 2];
				}
			}
		}
	}
	DCFlushRange(dest.data, dest.dataSize);
	return TE_OK;
}

TexErr STexture::fromJPG(TexData &dest, const u8 *buffer, const u32 buffer_size, u8 f, u32 minMipSize, u32 maxMipSize)
{
	Cleanup(dest);

	// Decode our JPG to raw
	VideoFrame VideoF;
	decodeRealJpeg(buffer, buffer_size, VideoF, true);
	if(!VideoF.getData())
		return TE_ERROR;
	dest.data = VideoF.getData();
	dest.width = VideoF.getWidth();
	dest.height = VideoF.getHeight();

	// Convert our raw stuff to a usable format
	u8 *rawData = (u8*)MEM2_alloc(dest.width * dest.height * 4);
	if(rawData == NULL)
	{
		Cleanup(dest);
		return TE_NOMEM;
	}
	_convertToFlippedRGBA(rawData, dest.data, dest.width, dest.height);

	//Free our raw stuff
	VideoF.dealloc();
	dest.data = NULL;

	//Let the real work begin
	u8 maxLODTmp = 0;
	u8 minLODTmp = 0;
	u32 baseWidth;
	u32 baseHeight;

	switch(f)
	{
		case GX_TF_RGBA8:
		case GX_TF_RGB565:
		case GX_TF_CMPR:
			break;
		default:
			f = GX_TF_RGBA8;
	}
	dest.format = f;

	if (minMipSize > 0 || maxMipSize > 0)
		_calcMipMaps(maxLODTmp, minLODTmp, baseWidth, baseHeight, dest.width, dest.height, minMipSize, maxMipSize);
	if (maxLODTmp > 0)
	{
		rawData = _genMipMaps(rawData, dest.width, dest.height, maxLODTmp, baseWidth, baseHeight);
		if(rawData == NULL)
		{
			Cleanup(dest);
			return TE_NOMEM;
		}
		u32 newWidth = baseWidth;
		u32 newHeight = baseHeight;
		for(int i = 0; i < minLODTmp; ++i)
		{
			newWidth >>= 1;
			newHeight >>= 1;
		}
		dest.dataSize = fixGX_GetTexBufferSize(newWidth, newHeight, dest.format, GX_TRUE, maxLODTmp - minLODTmp);
		dest.data = (u8*)MEM2_alloc(dest.dataSize);
		if(dest.data == NULL)
		{
			Cleanup(dest);
			free(rawData);
			return TE_NOMEM;
		}
		u32 nWidth = newWidth;
		u32 nHeight = newHeight;
		u8 *pSrc = rawData;
		if (minLODTmp > 0)
			pSrc += fixGX_GetTexBufferSize(baseWidth, baseHeight, dest.format, minLODTmp > 1 ? GX_TRUE : GX_FALSE, minLODTmp - 1);
		u8 *pDst = dest.data;
		for (u8 i = minLODTmp; i <= maxLODTmp; ++i)
		{
			switch(f)
			{
				case GX_TF_RGBA8:
					_convertToRGBA8(pDst, pSrc, nWidth, nHeight);
					break;
				case GX_TF_RGB565:
					_convertToRGB565(pDst, pSrc, nWidth, nHeight);
					break;
				case GX_TF_CMPR:
					_convertToCMPR(pDst, pSrc, nWidth, nHeight);
					break;
			}
			pSrc += nWidth * nHeight * 4;
			pDst += GX_GetTexBufferSize(nWidth, nHeight, dest.format, GX_FALSE, 0);
			nWidth >>= 1;
			nHeight >>= 1;
		}
		dest.maxLOD = maxLODTmp - minLODTmp;
		dest.width = newWidth;
		dest.height = newHeight;
	}
	else
	{
		dest.dataSize = GX_GetTexBufferSize(dest.width, dest.height, dest.format, GX_FALSE, 0);
		dest.data = (u8*)MEM2_alloc(dest.dataSize);
		if(dest.data == NULL)
		{
			Cleanup(dest);
			free(rawData);
			return TE_NOMEM;
		}
		switch(f)
		{
			case GX_TF_RGBA8:
				_convertToRGBA8(dest.data, rawData, dest.width, dest.height);
				break;
			case GX_TF_RGB565:
				_convertToRGB565(dest.data, rawData, dest.width, dest.height);
				break;
			case GX_TF_CMPR:
				_convertToCMPR(dest.data, rawData, dest.width, dest.height);
				break;
		}
	}
	DCFlushRange(dest.data, dest.dataSize);
	free(rawData);
	return TE_OK;
}

TexErr STexture::fromPNG(TexData &dest, const u8 *buffer, u8 f, u32 minMipSize, u32 maxMipSize, bool reduce_alpha)
{
	Cleanup(dest);
	u8 maxLODTmp = 0;
	u8 minLODTmp = 0;
	u32 baseWidth;
	u32 baseHeight;

	IMGCTX ctx = PNGU_SelectImageFromBuffer(buffer);
	if(ctx == 0)
		return TE_ERROR;
	PNGUPROP imgProp;
	if(PNGU_GetImageProperties(ctx, &imgProp) != PNGU_OK)
	{
		PNGU_ReleaseImageContext(ctx);
		return TE_ERROR;
	}
	if(imgProp.imgWidth > 1090 || imgProp.imgHeight > 1090)
	{
		PNGU_ReleaseImageContext(ctx);
		return TE_ERROR;
	}
	switch (f)
	{
		case GX_TF_RGBA8:
		case GX_TF_RGB565:
		case GX_TF_CMPR:
			break;
		default:
			f = (imgProp.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA || imgProp.imgColorType == PNGU_COLOR_TYPE_RGB_ALPHA) ? GX_TF_RGBA8 : GX_TF_RGB565;
	}
	u32 pngWidth = imgProp.imgWidth & (f == GX_TF_CMPR ? ~7u : ~3u);
	u32 pngHeight = imgProp.imgHeight & (f == GX_TF_CMPR ? ~7u : ~3u);
	if(minMipSize > 0 || maxMipSize > 0)
		_calcMipMaps(maxLODTmp, minLODTmp, baseWidth, baseHeight, imgProp.imgWidth, imgProp.imgHeight, minMipSize, maxMipSize);
	if (maxLODTmp > 0)
	{
		u32 newWidth = baseWidth;
		u32 newHeight = baseHeight;
		for (int i = 0; i < minLODTmp; ++i)
		{
			newWidth >>= 1;
			newHeight >>= 1;
		}
		u32 Size2 = imgProp.imgWidth * imgProp.imgHeight * 4;
		u8 *tmpData2 = (u8*)MEM2_alloc(Size2);
		if(tmpData2 == NULL)
		{
			PNGU_ReleaseImageContext(ctx);
			return TE_NOMEM;
		}
		memset(tmpData2, 0, Size2);
		PNGU_DecodeToRGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, tmpData2, 0, 0xFF);
		PNGU_ReleaseImageContext(ctx);
		if(dest.thread && (imgProp.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA 
			|| imgProp.imgColorType == PNGU_COLOR_TYPE_RGB_ALPHA)
			&& imgProp.imgWidth <= 640 && imgProp.imgHeight <= 480)
		{
			dest.format = GX_TF_RGBA8;
			dest.width = imgProp.imgWidth;
			dest.height = imgProp.imgHeight;
			dest.dataSize = GX_GetTexBufferSize(dest.width, dest.height, dest.format, GX_FALSE, 0);
			dest.data = (u8*)MEM2_alloc(dest.dataSize);
			_convertToRGBA8(dest.data, tmpData2, dest.width, dest.height);
			DCFlushRange(dest.data, dest.dataSize);
			CoverFlow.setRenderTex(true);
			while(CoverFlow.getRenderTex())
				usleep(50);
			_convertToRGBA(tmpData2, dest.data, dest.width, dest.height);
			DCFlushRange(tmpData2, Size2);
			Cleanup(dest);
		}
		tmpData2 = _genMipMaps(tmpData2, imgProp.imgWidth, imgProp.imgHeight, maxLODTmp, baseWidth, baseHeight);
		if(tmpData2 == NULL)
		{
			Cleanup(dest);
			return TE_NOMEM;
		}
		u32 nWidth = newWidth;
		u32 nHeight = newHeight;
		u8 *pSrc = tmpData2;
		if(minLODTmp > 0)
			pSrc += fixGX_GetTexBufferSize(baseWidth, baseHeight, f, minLODTmp > 1 ? GX_TRUE : GX_FALSE, minLODTmp - 1);
		dest.dataSize = fixGX_GetTexBufferSize(newWidth, newHeight, f, GX_TRUE, maxLODTmp - minLODTmp);
		dest.data = (u8*)MEM2_alloc(dest.dataSize);
		if(dest.data == NULL)
		{
			Cleanup(dest);
			free(tmpData2);
			return TE_NOMEM;
		}
		memset(dest.data, 0, dest.dataSize);
		u8 *pDst = dest.data;
		for(u8 i = minLODTmp; i <= maxLODTmp; ++i)
		{
			switch(f)
			{
				case GX_TF_RGBA8:
					_convertToRGBA8(pDst, pSrc, nWidth, nHeight);
					break;
				case GX_TF_RGB565:
					_convertToRGB565(pDst, pSrc, nWidth, nHeight);
					break;
				case GX_TF_CMPR:
					_convertToCMPR(pDst, pSrc, nWidth, nHeight);
					break;
			}
			pSrc += nWidth * nHeight * 4;
			pDst += GX_GetTexBufferSize(nWidth, nHeight, f, GX_FALSE, 0);
			nWidth >>= 1;
			nHeight >>= 1;
		}
		free(tmpData2);
		dest.maxLOD = maxLODTmp - minLODTmp;
		dest.format = f;
		dest.width = newWidth;
		dest.height = newHeight;
	}
	else
	{
		dest.dataSize = GX_GetTexBufferSize(pngWidth, pngHeight, f, GX_FALSE, 0);
		dest.data = (u8*)MEM2_alloc(dest.dataSize);
		if(dest.data == NULL)
		{
			Cleanup(dest);
			PNGU_ReleaseImageContext(ctx);
			return TE_NOMEM;
		}
		memset(dest.data, 0, dest.dataSize);
		dest.format = f;
		dest.width = pngWidth;
		dest.height = pngHeight;
		dest.maxLOD = 0;
		switch (f)
		{
			case GX_TF_RGBA8:
				PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, dest.data, 0xFF);
				break;
			case GX_TF_RGB565:
				PNGU_DecodeTo4x4RGB565(ctx, imgProp.imgWidth, imgProp.imgHeight, dest.data);
				break;
			case GX_TF_CMPR:
				PNGU_DecodeToCMPR(ctx, imgProp.imgWidth, imgProp.imgHeight, dest.data);
				break;
		}
		PNGU_ReleaseImageContext(ctx);
	}
	DCFlushRange(dest.data, dest.dataSize);
	_reduceAlpha(dest, reduce_alpha);
	return TE_OK;
}

void STexture::_reduceAlpha(TexData &dest, bool reduce_alpha)
{
	if(dest.format != GX_TF_RGBA8 || reduce_alpha == false)
		return;
	for(u32 y = 0; y < dest.height; ++y)
	{
		for(u32 x = 0; x < dest.width; ++x)
		{
			dest.data[coordsRGBA8(x, y, dest.width)] /= 2;
		}
	}
	DCFlushRange(dest.data, dest.dataSize);
}

void STexture::_resize(u8 *dst, u32 dstWidth, u32 dstHeight, const u8 *src, u32 srcWidth, u32 srcHeight)
{
	float wc = (float)srcWidth / (float)dstWidth;
	float hc = (float)srcHeight / (float)dstHeight;
	float ax1;
	float ay1;
	
	for (u32 y = 0; y < dstHeight; ++y)
	{
		for (u32 x = 0; x < dstWidth; ++x)
		{
			float xf = ((float)x + 0.5f) * wc - 0.5f;
			float yf = ((float)y + 0.5f) * hc - 0.5f;
			u32 x0 = (int)xf;
			u32 y0 = (int)yf;
			if (x0 >= srcWidth - 1)
			{
				x0 = srcWidth - 2;
				ax1 = 1.f;
			}
			else
				ax1 = xf - (float)x0;
			float ax0 = 1.f - ax1;
			if (y0 >= srcHeight - 1)
			{
				y0 = srcHeight - 2;
				ay1 = 1.f;
			}
			else
				ay1 = yf - (float)y0;
			float ay0 = 1.f - ay1;
			u8 *pdst = dst + (x + y * dstWidth) * 4;
			const u8 *psrc0 = src + (x0 + y0 * srcWidth) * 4;
			const u8 *psrc1 = psrc0 + srcWidth * 4;
			for (int c = 0; c < 3; ++c)
				pdst[c] = (u8)((((float)psrc0[c] * ax0) + ((float)psrc0[4 + c] * ax1)) * ay0 + (((float)psrc1[c] * ax0) + ((float)psrc1[4 + c] * ax1)) * ay1 + 0.5f);
			pdst[3] = 0xFF;	// Alpha not handled, it would require using it in the weights for color channels, easy but slower and useless so far.
		}
	}
}

// For powers of two
void STexture::_resizeD2x2(u8 *dst, const u8 *src, u32 srcWidth, u32 srcHeight)
{
#if 0
	u32 *dst32 = (u32 *)dst;
	const u32 *src32 = (const u32 *)src;
	u32 i = 0, i0 = 0, i1 = 1, i2 = srcWidth, i3 = srcWidth + 1, dstWidth = srcWidth >> 1, dstHeight = srcHeight >> 1;

	for (u32 y = 0; y < dstHeight; ++y)
	{
		for (u32 x = 0; x < dstWidth; ++x)
		{
			dst32[i] = (((src32[i0] & 0xFCFCFCFC) >> 2)
				+ ((src32[i1] & 0xFCFCFCFC) >> 2)
				+ ((src32[i2] & 0xFCFCFCFC) >> 2)
				+ ((src32[i3] & 0xFCFCFCFC) >> 2)) | 0x000000FF;
			++i;
			i0 += 2;
			i1 += 2;
			i2 += 2;
			i3 += 2;
		}
		i0 += srcWidth;
		i1 += srcWidth;
		i2 += srcWidth;
		i3 += srcWidth;
	}
#else
	u32 i = 0, i0 = 0, i1 = 4, i2 = srcWidth * 4, i3 = (srcWidth + 1) * 4, dstWidth = srcWidth >> 1, dstHeight = srcHeight >> 1, w4 = srcWidth * 4;

	for (u32 y = 0; y < dstHeight; ++y)
	{
		for (u32 x = 0; x < dstWidth; ++x)
		{
			dst[i] = ((u32)src[i0] + src[i1] + src[i2] + src[i3]) >> 2;
			dst[i + 1] = ((u32)src[i0 + 1] + src[i1 + 1] + src[i2 + 1] + src[i3 + 1]) >> 2;
			dst[i + 2] = ((u32)src[i0 + 2] + src[i1 + 2] + src[i2 + 2] + src[i3 + 2]) >> 2;
			dst[i + 3] = ((u32)src[i0 + 3] + src[i1 + 3] + src[i2 + 3] + src[i3 + 3]) >> 2;
			i += 4;
			i0 += 8;
			i1 += 8;
			i2 += 8;
			i3 += 8;
		}
		i0 += w4;
		i1 += w4;
		i2 += w4;
		i3 += w4;
	}
#endif
}

void STexture::_calcMipMaps(u8 &maxLOD, u8 &minLOD, u32 &lod0Width, u32 &lod0Height, u32 width, u32 height, u32 minSize, u32 maxSize)
{
	if (minSize < 8) minSize = 8;
	lod0Width = upperPower(width);
	lod0Height = upperPower(height);
	if (width - (lod0Width >> 1) < lod0Width >> 3 && minSize <= lod0Width >> 1)
		lod0Width >>= 1;
	if (height - (lod0Height >> 1) < lod0Height >> 3 && minSize <= lod0Height >> 1)
		lod0Height >>= 1;
	maxLOD = 0;
	for (u32 i = min(lod0Width, lod0Height); i > minSize; i >>= 1)
		++maxLOD;
	minLOD = 0;
	if (maxSize > 8)
		for (u32 i = max(lod0Width, lod0Height); i > maxSize; i >>= 1)
			++minLOD;
	if (minLOD > maxLOD)
		maxLOD = minLOD;
}

u8 *STexture::_genMipMaps(u8 *src, u32 width, u32 height, u8 maxLOD, u32 lod0Width, u32 lod0Height)
{
	u32 bufSize = fixGX_GetTexBufferSize(lod0Width, lod0Height, GX_TF_RGBA8, GX_TRUE, maxLOD);
	u8 *dstData = (u8*)MEM2_alloc(bufSize);
	if(dstData == NULL)
		return NULL;

	memset(dstData, 0, bufSize);
	_resize(dstData, lod0Width, lod0Height, src, width, height);
	DCFlushRange(dstData, lod0Width * lod0Height * 4);
	free(src);
	src = NULL;

	u32 nWidth = lod0Width;
	u32 nHeight = lod0Height;
	u8 *pDst = dstData;
	for(u8 i = 0; i < maxLOD; ++i)
	{
		u8 *pSrc = pDst;
		pDst += nWidth * nHeight * 4;
		_resizeD2x2(pDst, pSrc, nWidth, nHeight);
		DCFlushRange(pDst, nWidth * nWidth);
		nWidth >>= 1;
		nHeight >>= 1;
	}
	return dstData;
}
