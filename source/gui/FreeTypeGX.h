/* 
 * FreeTypeGX is a wrapper class for libFreeType which renders a compiled
 * FreeType parsable font into a GX texture for Wii homebrew development.
 * Copyright (C) 2008 Armin Tamzarian
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
#include "loader/utils.h"

#include <malloc.h>
#include <string.h>
#include <wchar.h>
#include <map>

typedef struct ftgxCharData_
{
	int16_t renderOffsetX; /**< Texture X axis bearing offset. */
	uint16_t glyphAdvanceX; /**< Character glyph X coordinate advance in pixels. */
	uint32_t glyphIndex; /**< Charachter glyph index in the font face. */

	uint16_t textureWidth; /**< Texture width in pixels/bytes. */
	uint16_t textureHeight; /**< Texture glyph height in pixels/bytes. */

	int16_t renderOffsetY; /**< Texture Y axis bearing offset. */
	int16_t renderOffsetMax; /**< Texture Y axis bearing maximum value. */
	int16_t renderOffsetMin; /**< Texture Y axis bearing minimum value. */

	uint8_t* glyphDataTexture; /**< Glyph texture bitmap data buffer. */
} ftgxCharData;

typedef struct ftgxDataOffset_
{
	int16_t ascender; /**< Maximum data offset. */
	int16_t descender; /**< Minimum data offset. */
	int16_t max; /**< Maximum data offset. */
	int16_t min; /**< Minimum data offset. */
} ftgxDataOffset;

#define FTGX_NULL				0x0000
#define FTGX_JUSTIFY_LEFT		0x0001 //0001
#define FTGX_JUSTIFY_CENTER		0x0002 //0010
#define FTGX_JUSTIFY_RIGHT		0x0004 //0100
#define FTGX_JUSTIFY_MASK		0x000f //1111

#define FTGX_ALIGN_TOP			0x0010
#define FTGX_ALIGN_MIDDLE		0x0020
#define FTGX_ALIGN_BOTTOM		0x0040
#define FTGX_ALIGN_BASELINE	    0x0080
#define FTGX_ALIGN_GLYPH_TOP	0x0100
#define FTGX_ALIGN_GLYPH_MIDDLE 0x0200
#define FTGX_ALIGN_GLYPH_BOTTOM 0x0400
#define FTGX_ALIGN_MASK			0x0ff0

#define FTGX_STYLE_UNDERLINE	0x1000
#define FTGX_STYLE_STRIKE		0x2000
#define FTGX_STYLE_MASK			0xf000

const GXColor ftgxWhite = (GXColor){0xff, 0xff, 0xff, 0xff}; /**< Constant color value used only to sanitize Doxygen documentation. */

class FreeTypeGX {

	private:
		FT_Library ftLibrary;	/**< FreeType FT_Library instance. */
		FT_Face ftFace;			/**< FreeType reusable FT_Face typographic object. */
		//FT_GlyphSlot ftSlot;	/**< FreeType reusable FT_GlyphSlot glyph container object. */
		int16_t ftPointSize;	/**< Requested size of the rendered font. */
		FT_Pos ftWeight;		/**< Requested weight of the rendered font. */
		bool ftKerningEnabled;	/**< Flag indicating the availability of font kerning data. */

		float xScale;
		float yScale;
		float xPos;
		float yPos;

		std::map<int16_t, std::map<wchar_t, ftgxCharData> > fontData; /**< Map which holds the glyph data structures for the corresponding characters in one size. */
		std::map<int16_t, ftgxDataOffset> ftgxAlign; /**< Map which holds the ascender and decender for different sizes. */
		
		int16_t getStyleOffsetWidth(uint16_t width, uint16_t format);
		int16_t getStyleOffsetHeight(int16_t format, uint16_t pixelSize);

		void unloadFont();
		ftgxCharData *cacheGlyphData(wchar_t charCode, int16_t pixelSize);
		uint16_t cacheGlyphDataComplete(int16_t pixelSize);
		void loadGlyphData(FT_Bitmap *bmp, ftgxCharData *charData);
		
		void drawTextFeature(int16_t x, int16_t y, int16_t pixelSize, uint16_t width,
				ftgxDataOffset *offsetData, uint16_t format, GXColor color);
		void copyTextureToFramebuffer(GXTexObj *texObj, uint16_t texWidth, uint16_t texHeight, int16_t screenX, int16_t screenY, GXColor color);
		void copyFeatureToFramebuffer(uint16_t featureWidth, uint16_t featureHeight, int16_t screenX, int16_t screenY, GXColor color);

	public:
		FreeTypeGX();
		~FreeTypeGX();

		static wchar_t* charToWideChar(char* p);

		void loadFont(const uint8_t* fontBuffer, FT_Long bufferSize, FT_Pos weight = 0, bool lastFace = false);

		uint16_t drawText(int16_t x, int16_t y, const wchar_t *text, int16_t pixelSize, GXColor color = ftgxWhite, 
						uint16_t textStyling = FTGX_NULL, uint16_t textWidth = 0, uint16_t widthLimit = 0);

		uint16_t getWidth(const wchar_t *text, int16_t pixelSize);
		uint16_t getCharWidth(const wchar_t wChar, int16_t pixelSize, const wchar_t prevChar = 0x0000);
		uint16_t getHeight(const wchar_t *text, int16_t pixelSize);
		void getOffset(const wchar_t *text, int16_t pixelSize, uint16_t widthLimit = 0);

		float getXScale(void) const { return xScale; }
		float getYScale(void) const { return yScale; }
		void setXScale(float f) { xScale = f; }
		void setYScale(float f) { yScale = f; }
		float getX(void) const { return xPos; }
		float getY(void) const { return yPos; }
		void setX(float f) { xPos = f; }
		void setY(float f) { yPos = f; }
		void reset(void) { xScale = 1.f; yScale = 1.f; xPos = 0.f; yPos = 0.f; };
};

#endif /* FREETYPEGX_H_ */
