#include <string.h>
#include <stdio.h>
#include <ogcsys.h>

#include "texture.hpp"
#include "pngu.h"

using namespace std;

static u32 upperPower(u32 width)
{
	u32 i = 8;
	u32 maxWidth = 1024;
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

void STexture::_convertToFlippedRGBA8(u8 *dst, const u8 * src, u32 width, u32 height)
{
    u32 block, i, c, ar, gb;

    for (block = 0; block < height; block += 4)
    {
        for (i = 0; i < width; i += 4)
        {
            /* Alpha and Red */
            for (c = 0; c < 4; ++c)
            {
                for (ar = 0; ar < 4; ++ar)
                {
                    u32 y = height - 1 - (c + block);
                    u32 x = ar + i;
                    u32 offset = ((((y >> 2) * (width >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
                    /* Alpha pixels */
                    dst[offset] = 255;
                    /* Red pixels */
                    dst[offset+1] = src[((i + ar) + ((block + c) * width)) * 3];
                }
            }

            /* Green and Blue */
            for (c = 0; c < 4; ++c)
            {
                for (gb = 0; gb < 4; ++gb)
                {
                    u32 y = height - 1 - (c + block);
                    u32 x = gb + i;
                    u32 offset = ((((y >> 2) * (width >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
                    /* Green pixels */
                    dst[offset+32] = src[(((i + gb) + ((block + c) * width)) * 3) + 1];
                    /* Blue pixels */
                    dst[offset+33] = src[(((i + gb) + ((block + c) * width)) * 3) + 2];
                }
            }
        }
    }
}

void STexture::_convertToRGBA8(u8 *dst, const u8 *src, u32 width, u32 height)
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

void STexture::_convertToRGB565(u8 *dst, const u8 *src, u32 width, u32 height)
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

void STexture::_convertToCMPR(u8 *dst, const u8 *src, u32 width, u32 height)
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

STexture::TexErr STexture::fromPNGFile(const char *filename, u8 f, Alloc alloc, u32 minMipSize, u32 maxMipSize)
{
	SmartBuf ptrPng;

	FILE *file = fopen(filename, "rb");
	if (file == 0) return STexture::TE_ERROR;
	fseek(file, 0, SEEK_END);
	u32 fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (fileSize > 0)
	{
		ptrPng = smartMem2Alloc(fileSize);
		if (!!ptrPng)
			if (fread(ptrPng.get(), 1, fileSize, file) != fileSize)
				SMART_FREE(ptrPng);
	}
	SAFE_CLOSE(file);
	return !!ptrPng ? fromPNG(ptrPng.get(), f, alloc, minMipSize, maxMipSize) : STexture::TE_NOMEM;
}

STexture::TexErr STexture::fromRAW(const u8 *buffer, u32 w, u32 h, u8 f, Alloc alloc)
{
	SmartBuf tmpData;

	switch (f)
	{
		case GX_TF_RGBA8:
		case GX_TF_RGB565:
		case GX_TF_CMPR:
			break;
		default:
			f = GX_TF_RGBA8;
	}

	switch (alloc)
	{
		case ALLOC_MEM2:
			tmpData = smartMem2Alloc(GX_GetTexBufferSize(w, h, f, GX_FALSE, 0));
			break;
		case ALLOC_MALLOC:
			tmpData = smartMemAlign32(GX_GetTexBufferSize(w, h, f, GX_FALSE, 0));
			break;
	}
	if (!tmpData) return STexture::TE_NOMEM;

	format = f;
	width = w;
	height = h;
	maxLOD = 0;
	data = tmpData;
	switch (f)
	{
		case GX_TF_RGBA8:
			STexture::_convertToFlippedRGBA8(tmpData.get(), buffer, width, height);
			break;
		case GX_TF_RGB565:
			STexture::_convertToRGB565(tmpData.get(), buffer, width, height);
			break;
		case GX_TF_CMPR:
			STexture::_convertToCMPR(tmpData.get(), buffer, width, height);
			break;
	}
	DCFlushRange(data.get(), GX_GetTexBufferSize(width, height, format, GX_FALSE, 0));

	return STexture::TE_OK;
}

STexture::TexErr STexture::fromPNG(const u8 *buffer, u8 f, Alloc alloc, u32 minMipSize, u32 maxMipSize)
{
	PNGUPROP imgProp;
	SmartBuf tmpData;
	u8 maxLODTmp = 0;
	u8 minLODTmp = 0;
	u32 baseWidth;
	u32 baseHeight;

	IMGCTX ctx = PNGU_SelectImageFromBuffer(buffer);
	if (ctx == 0) return STexture::TE_ERROR;
	if (PNGU_GetImageProperties(ctx, &imgProp) != PNGU_OK)
	{
		PNGU_ReleaseImageContext(ctx);
		return STexture::TE_ERROR;
	}
	if (imgProp.imgWidth > 1024 || imgProp.imgHeight > 1024)
	{
		PNGU_ReleaseImageContext(ctx);
		return STexture::TE_ERROR;
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
	if (minMipSize > 0 || maxMipSize > 0)
		STexture::_calcMipMaps(maxLODTmp, minLODTmp, baseWidth, baseHeight, imgProp.imgWidth, imgProp.imgHeight, minMipSize, maxMipSize);
	if (maxLODTmp > 0)
	{
		u32 newWidth = baseWidth;
		u32 newHeight = baseHeight;
		for (int i = 0; i < minLODTmp; ++i)
		{
			newWidth >>= 1;
			newHeight >>= 1;
		}
		SmartBuf tmpData2 = smartMem2Alloc(imgProp.imgWidth * imgProp.imgHeight * 4);
		switch (alloc)
		{
			case ALLOC_MEM2:
				tmpData = smartMem2Alloc(fixGX_GetTexBufferSize(newWidth, newHeight, f, GX_TRUE, maxLODTmp - minLODTmp));
				break;
			case ALLOC_MALLOC:
				tmpData = smartMemAlign32(fixGX_GetTexBufferSize(newWidth, newHeight, f, GX_TRUE, maxLODTmp - minLODTmp));
				break;
		}
		if (!tmpData || !tmpData2)
		{
			SMART_FREE(tmpData);
			SMART_FREE(tmpData2);
			PNGU_ReleaseImageContext(ctx);
			return STexture::TE_NOMEM;
		}
		PNGU_DecodeToRGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, tmpData2.get(), 0, 0xFF);
		PNGU_ReleaseImageContext(ctx);
		DCFlushRange(tmpData2.get(), imgProp.imgWidth * imgProp.imgHeight * 4);

		tmpData2 = STexture::_genMipMaps(tmpData2.get(), imgProp.imgWidth, imgProp.imgHeight, maxLODTmp, baseWidth, baseHeight);
		if (!tmpData2) return STexture::TE_NOMEM;

		u32 nWidth = newWidth;
		u32 nHeight = newHeight;
		u8 *pSrc = tmpData2.get();
		if (minLODTmp > 0)
			pSrc += fixGX_GetTexBufferSize(baseWidth, baseHeight, GX_TF_RGBA8, minLODTmp > 1 ? GX_TRUE : GX_FALSE, minLODTmp - 1);
		u8 *pDst = tmpData.get();
		for (u8 i = minLODTmp; i <= maxLODTmp; ++i)
		{
			switch (f)
			{
				case GX_TF_RGBA8:
					STexture::_convertToRGBA8(pDst, pSrc, nWidth, nHeight);
					break;
				case GX_TF_RGB565:
					STexture::_convertToRGB565(pDst, pSrc, nWidth, nHeight);
					break;
				case GX_TF_CMPR:
					STexture::_convertToCMPR(pDst, pSrc, nWidth, nHeight);
					break;
			}
			pSrc += nWidth * nHeight * 4;
			pDst += GX_GetTexBufferSize(nWidth, nHeight, f, GX_FALSE, 0);
			nWidth >>= 1;
			nHeight >>= 1;
		}
		maxLOD = maxLODTmp - minLODTmp;
		data = tmpData;
		format = f;
		width = newWidth;
		height = newHeight;
		DCFlushRange(data.get(), fixGX_GetTexBufferSize(width, height, format, maxLOD > 0 ? GX_TRUE : GX_FALSE, maxLOD));
	}
	else
	{
		switch (alloc)
		{
			case ALLOC_MEM2:
				tmpData = smartMem2Alloc(GX_GetTexBufferSize(pngWidth, pngHeight, f, GX_FALSE, maxLOD));
				break;
			case ALLOC_MALLOC:
				tmpData = smartMemAlign32(GX_GetTexBufferSize(pngWidth, pngHeight, f, GX_FALSE, maxLOD));
				break;
		}
		if (!tmpData)
		{
			PNGU_ReleaseImageContext(ctx);
			return STexture::TE_NOMEM;
		}
		format = f;
		width = pngWidth;
		height = pngHeight;
		maxLOD = 0;
		data = tmpData;
		switch (f)
		{
			case GX_TF_RGBA8:
				PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, data.get(), 0xFF);
				break;
			case GX_TF_RGB565:
				PNGU_DecodeTo4x4RGB565(ctx, imgProp.imgWidth, imgProp.imgHeight, data.get());
				break;
			case GX_TF_CMPR:
				PNGU_DecodeToCMPR(ctx, imgProp.imgWidth, imgProp.imgHeight, data.get());
				break;
		}
		PNGU_ReleaseImageContext(ctx);
		DCFlushRange(data.get(), GX_GetTexBufferSize(width, height, format, GX_FALSE, 0));
	}
	return STexture::TE_OK;
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

SmartBuf STexture::_genMipMaps(const u8 *src, u32 width, u32 height, u8 maxLOD, u32 lod0Width, u32 lod0Height)
{
	u32 bufSize = fixGX_GetTexBufferSize(lod0Width, lod0Height, GX_TF_RGBA8, GX_TRUE, maxLOD);
	SmartBuf dst = smartMem2Alloc(bufSize);
	if (!dst) return dst;
	STexture::_resize(dst.get(), lod0Width, lod0Height, src, width, height);
	DCFlushRange(dst.get(), lod0Width * lod0Height * 4);
	u32 nWidth = lod0Width;
	u32 nHeight = lod0Height;
	u8 *pDst = dst.get();
	for (u8 i = 0; i < maxLOD; ++i)
	{
		u8 *pSrc = pDst;
		pDst += nWidth * nHeight * 4;
		STexture::_resizeD2x2(pDst, pSrc, nWidth, nHeight);
		DCFlushRange(pDst, nWidth * nWidth);
		nWidth >>= 1;
		nHeight >>= 1;
	}
	return dst;
}
