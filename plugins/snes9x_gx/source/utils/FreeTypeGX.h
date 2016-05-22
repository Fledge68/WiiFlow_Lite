/*
 * FreeTypeGX is a wrapper class for libFreeType which renders a compiled
 * FreeType parsable font into a GX texture for Wii homebrew development.
 * Copyright (C) 2008 Armin Tamzarian
 * Modified by Tantric, 2009-2010
 *
 * This file is part of FreeTypeGX.
 *
 * FreeTypeGX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeTypeGX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FreeTypeGX.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FREETYPEGX_H_
#define FREETYPEGX_H_

#include <gccore.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#include <malloc.h>
#include <string.h>
#include <wchar.h>
#include <map>

#define MAX_FONT_SIZE 100

/*! \struct ftgxCharData_
 *
 * Font face character glyph relevant data structure.
 */
typedef struct ftgxCharData_ {
	int16_t renderOffsetX;		/**< Texture X axis bearing offset. */
	uint16_t glyphAdvanceX;		/**< Character glyph X coordinate advance in pixels. */
	uint16_t glyphIndex;		/**< Charachter glyph index in the font face. */

	uint16_t textureWidth;		/**< Texture width in pixels/bytes. */
	uint16_t textureHeight;		/**< Texture glyph height in pixels/bytes. */

	int16_t renderOffsetY;		/**< Texture Y axis bearing offset. */
	int16_t renderOffsetMax;	/**< Texture Y axis bearing maximum value. */
	int16_t renderOffsetMin;	/**< Texture Y axis bearing minimum value. */

	uint32_t* glyphDataTexture;	/**< Glyph texture bitmap data buffer. */
} ftgxCharData;

/*! \struct ftgxDataOffset_
 *
 * Offset structure which hold both a maximum and minimum value.
 */
typedef struct ftgxDataOffset_ {
	int16_t ascender;	/**< Maximum data offset. */
	int16_t descender;	/**< Minimum data offset. */
	int16_t max;		/**< Maximum data offset. */
	int16_t min;		/**< Minimum data offset. */
} ftgxDataOffset;

typedef struct ftgxCharData_ ftgxCharData;
typedef struct ftgxDataOffset_ ftgxDataOffset;

#define _TEXT(t) L ## t /**< Unicode helper macro. */

#define FTGX_NULL				0x0000
#define FTGX_JUSTIFY_LEFT		0x0001
#define FTGX_JUSTIFY_CENTER		0x0002
#define FTGX_JUSTIFY_RIGHT		0x0004
#define FTGX_JUSTIFY_MASK		0x000f

#define FTGX_ALIGN_TOP			0x0010
#define FTGX_ALIGN_MIDDLE		0x0020
#define FTGX_ALIGN_BOTTOM		0x0040
#define FTGX_ALIGN_BASELINE		0x0080
#define FTGX_ALIGN_GLYPH_TOP	0x0100
#define FTGX_ALIGN_GLYPH_MIDDLE	0x0200
#define FTGX_ALIGN_GLYPH_BOTTOM	0x0400
#define FTGX_ALIGN_MASK			0x0ff0

#define FTGX_STYLE_UNDERLINE	0x1000
#define FTGX_STYLE_STRIKE		0x2000
#define FTGX_STYLE_MASK			0xf000

#define FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_MODULATE	0X0001
#define FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_DECAL		0X0002
#define FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_BLEND		0X0004
#define FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_REPLACE		0X0008
#define FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR		0X0010

#define FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE		0X0100
#define FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_DIRECT	0X0200
#define FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_INDEX8	0X0400
#define FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_INDEX16	0X0800

#define FTGX_COMPATIBILITY_NONE							0x0000
#define FTGX_COMPATIBILITY_GRRLIB						FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR | FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE
#define FTGX_COMPATIBILITY_LIBWIISPRITE					FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_MODULATE | FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_DIRECT

const GXColor ftgxWhite = (GXColor){0xff, 0xff, 0xff, 0xff}; /**< Constant color value used only to sanitize Doxygen documentation. */

void InitFreeType(uint8_t* fontBuffer, FT_Long bufferSize);
void DeinitFreeType();
void ChangeFontSize(FT_UInt pixelSize);
wchar_t* charToWideChar(const char* p);
void ClearFontData();

/*! \class FreeTypeGX
 * \brief Wrapper class for the libFreeType library with GX rendering.
 * \author Armin Tamzarian
 * \version 0.2.4
 *
 * FreeTypeGX acts as a wrapper class for the libFreeType library. It supports precaching of transformed glyph data into
 * a specified texture format. Rendering of the data to the EFB is accomplished through the application of high performance
 * GX texture functions resulting in high throughput of string rendering.
 */
class FreeTypeGX {

	private:
		FT_UInt ftPointSize;	/**< Requested size of the rendered font. */
		bool ftKerningEnabled;	/**< Flag indicating the availability of font kerning data. */
		uint8_t vertexIndex;	/**< Vertex format descriptor index. */
		uint32_t compatibilityMode;	/**< Compatibility mode for default tev operations and vertex descriptors. */
		std::map<wchar_t, ftgxCharData> fontData; /**< Map which holds the glyph data structures for the corresponding characters. */

		static uint16_t adjustTextureWidth(uint16_t textureWidth);
		static uint16_t adjustTextureHeight(uint16_t textureHeight);

		static int16_t getStyleOffsetWidth(uint16_t width, uint16_t format);
		static int16_t getStyleOffsetHeight(ftgxDataOffset *offset, uint16_t format);

		void unloadFont();
		ftgxCharData *cacheGlyphData(wchar_t charCode);
		uint16_t cacheGlyphDataComplete();
		void loadGlyphData(FT_Bitmap *bmp, ftgxCharData *charData);

		void setDefaultMode();

		void drawTextFeature(int16_t x, int16_t y, uint16_t width, ftgxDataOffset *offsetData, uint16_t format, GXColor color);
		void copyTextureToFramebuffer(GXTexObj *texObj, f32 texWidth, f32 texHeight, int16_t screenX, int16_t screenY, GXColor color);
		void copyFeatureToFramebuffer(f32 featureWidth, f32 featureHeight, int16_t screenX, int16_t screenY,  GXColor color);

	public:
		FreeTypeGX(FT_UInt pixelSize, uint8_t vertexIndex = GX_VTXFMT1);
		~FreeTypeGX();

		void setVertexFormat(uint8_t vertexIndex);
		void setCompatibilityMode(uint32_t compatibilityMode);

		uint16_t drawText(int16_t x, int16_t y, wchar_t *text, GXColor color = ftgxWhite, uint16_t textStyling = FTGX_NULL);
		uint16_t drawText(int16_t x, int16_t y, wchar_t const *text, GXColor color = ftgxWhite, uint16_t textStyling = FTGX_NULL);

		uint16_t getWidth(wchar_t *text);
		uint16_t getWidth(wchar_t const *text);
		uint16_t getHeight(wchar_t *text);
		uint16_t getHeight(wchar_t const *text);
		void getOffset(wchar_t *text, ftgxDataOffset* offset);
		void getOffset(wchar_t const *text, ftgxDataOffset* offset);
};

#endif /* FREETYPEGX_H_ */
