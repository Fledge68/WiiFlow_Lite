
#ifndef __TEXTURE_HPP
#define __TEXTURE_HPP

#include <gccore.h>

#include "smartptr.hpp"

struct STexture
{
	SmartBuf data;
	u32 width;
	u32 height;
	u8 format;
	u8 maxLOD;
	STexture(void) : data(), width(0), height(0), format(-1), maxLOD(0) { }
	// Utility funcs
	enum TexErr { TE_OK, TE_ERROR, TE_NOMEM };
	// This function doesn't use MEM2 if the PNG is loaded from memory and there's no mip mapping
	TexErr fromPNG(const u8 *buffer, u8 f = -1, Alloc alloc = ALLOC_MEM2, u32 minMipSize = 0, u32 maxMipSize = 0);
	TexErr fromPNGFile(const char *filename, u8 f = -1, Alloc alloc = ALLOC_MEM2, u32 minMipSize = 0, u32 maxMipSize = 0);
	TexErr fromRAW(const u8 *buffer, u32 w, u32 h, u8 f = -1, Alloc alloc = ALLOC_MEM2);
private:
	static void _resize(u8 *dst, u32 dstWidth, u32 dstHeight, const u8 *src, u32 srcWidth, u32 srcHeight);
	static void _resizeD2x2(u8 *dst, const u8 *src, u32 srcWidth, u32 srcHeight);
	static SmartBuf _genMipMaps(const u8 *src, u32 width, u32 height, u8 maxLOD, u32 lod0Width, u32 lod0Height);
	static void _calcMipMaps(u8 &maxLOD, u8 &minLOD, u32 &lod0Width, u32 &lod0Height, u32 width, u32 height, u32 minSize, u32 maxSize);
	static void _convertToRGBA8(u8 *dst, const u8 *src, u32 width, u32 height);
	static void _convertToFlippedRGBA8(u8 *dst, const u8 *src, u32 width, u32 height);
	static void _convertToRGB565(u8 *dst, const u8 *src, u32 width, u32 height);
	static void _convertToCMPR(u8 *dst, const u8 *src, u32 width, u32 height);
};

u32 fixGX_GetTexBufferSize(u16 wd, u16 ht, u32 fmt, u8 mipmap, u8 maxlod);

#endif //!defined(__TEXTURE_HPP)
