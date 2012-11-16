
#ifndef __TEXTURE_HPP
#define __TEXTURE_HPP

#include <gccore.h>

enum TexErr
{
	TE_OK = 0,
	TE_ERROR,
	TE_NOMEM
};

class STexture
{
public:
	STexture(void) : data(NULL), dataSize(0), width(0), height(0), format(-1), maxLOD(0), thread(false) { }
	void Cleanup();
	bool CopyTexture(const STexture &tex);
	u8 *data;
	u32 dataSize;
	u32 width;
	u32 height;
	u8 format;
	u8 maxLOD;
	bool thread;
	// Get from PNG, if not found from JPG
	TexErr fromImageFile(const char *filename, u8 f = -1, u32 minMipSize = 0, u32 maxMipSize = 0);
	// This function doesn't use MEM2 if the PNG is loaded from memory and there's no mip mapping
	TexErr fromPNG(const u8 *buffer, u8 f = -1, u32 minMipSize = 0, u32 maxMipSize = 0);
	TexErr fromJPG(const u8 *buffer, const u32 buffer_size, u8 f = -1, u32 minMipSize = 0, u32 maxMipSize = 0);
	/* Just for THP */
	TexErr fromTHP(const u8 *buffer, u32 w, u32 h);
private:
	void _resize(u8 *dst, u32 dstWidth, u32 dstHeight, const u8 *src, u32 srcWidth, u32 srcHeight);
	void _resizeD2x2(u8 *dst, const u8 *src, u32 srcWidth, u32 srcHeight);
	u8 *_genMipMaps(u8 *src, u32 width, u32 height, u8 maxLOD, u32 lod0Width, u32 lod0Height);
	void _calcMipMaps(u8 &maxLOD, u8 &minLOD, u32 &lod0Width, u32 &lod0Height, u32 width, u32 height, u32 minSize, u32 maxSize);
};

u32 fixGX_GetTexBufferSize(u16 wd, u16 ht, u32 fmt, u8 mipmap, u8 maxlod);

#endif //!defined(__TEXTURE_HPP)
