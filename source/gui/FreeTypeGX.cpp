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
#include "FreeTypeGX.h"
#include "memory/mem2.hpp"

using namespace std;

#define ALIGN8(x) (((x) + 7) & ~7)

FreeTypeGX::FreeTypeGX()
{
	FT_Init_FreeType(&ftLibrary);
    reset();
	ftFace = 0;
}

FreeTypeGX::~FreeTypeGX()
{
	unloadFont();
	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLibrary);
}

wchar_t* FreeTypeGX::charToWideChar(char* strChar)
{
	if (!strChar) return NULL;

	wchar_t *strWChar = new (std::nothrow) wchar_t[strlen(strChar) + 1];
	if (!strWChar) return NULL;

	int bt = mbstowcs(strWChar, strChar, strlen(strChar));
	if (bt > 0)
	{
		strWChar[bt] = 0;
		return strWChar;
	}

	wchar_t *tempDest = strWChar;
	while ((*tempDest++ = *strChar++));

	return strWChar;
}

void FreeTypeGX::loadFont(const uint8_t* fontBuffer, FT_Long bufferSize, FT_Pos weight, bool lastFace)
{
	// lastFace is true for wii system font from buffer
	unloadFont();
	//ftPointSize = pointSize != 0 ? pointSize : ftPointSize;
	ftWeight = weight;

	int faceIndex = 0;
	ftPointSize = 0;
	
	// if we want the last face of the font use this to set faceIndex to the last face. then close ftface to reload it after the if with that faceIndex
	if(lastFace)
	{
		FT_New_Memory_Face(this->ftLibrary, (FT_Byte *)fontBuffer, bufferSize, -1, &this->ftFace);
		faceIndex = this->ftFace->num_faces - 1; // Use the last face
		FT_Done_Face(this->ftFace);
		this->ftFace = NULL;
	}
	
	// load font face 0 or the last face available
	FT_New_Memory_Face(this->ftLibrary, (FT_Byte *) fontBuffer, bufferSize, faceIndex, &this->ftFace);
	
	ftKerningEnabled = false;//FT_HAS_KERNING(ftFace);
}

void FreeTypeGX::unloadFont()
{
	if (this->fontData.size() == 0) return;

	map<int16_t, map<wchar_t, ftgxCharData> >::iterator itr;
	map<wchar_t, ftgxCharData>::iterator itr2;

	for (itr = fontData.begin(); itr != fontData.end(); itr++)
	{
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
			MEM2_free(itr2->second.glyphDataTexture);

		itr->second.clear();
	}

	fontData.clear();
	ftgxAlign.clear();
}

ftgxCharData * FreeTypeGX::cacheGlyphData(wchar_t charCode, int16_t pixelSize)
{
	map<int16_t, map<wchar_t, ftgxCharData> >::iterator itr = fontData.find(pixelSize);
	if (itr != fontData.end())
	{
		map<wchar_t, ftgxCharData>::iterator itr2 = itr->second.find(charCode);
		if (itr2 != itr->second.end())
		{
			return &itr2->second;
		}
	}

	FT_UInt gIndex;
	uint16_t textureWidth = 0, textureHeight = 0;

	if (ftPointSize != pixelSize)
	{
		ftPointSize = pixelSize;
		FT_Set_Pixel_Sizes(ftFace, 0, ftPointSize);

		//!Cache ascender and decender as well
		map<int16_t, ftgxDataOffset>::iterator itrAlign = ftgxAlign.find(ftPointSize);
		if (itrAlign == ftgxAlign.end())
		{
			ftgxAlign[ftPointSize].ascender = (int16_t) ftFace->size->metrics.ascender >> 6;
			ftgxAlign[ftPointSize].descender = (int16_t) ftFace->size->metrics.descender >> 6;
			ftgxAlign[ftPointSize].max = 0;
			ftgxAlign[ftPointSize].min = 0;
		}
	}

	gIndex = FT_Get_Char_Index(ftFace, (FT_ULong) charCode);
	if (gIndex != 0 && FT_Load_Glyph(ftFace, gIndex, FT_LOAD_DEFAULT | FT_LOAD_RENDER) == 0)
	{
		if (ftFace->glyph->format == FT_GLYPH_FORMAT_BITMAP)
		{
			FT_Bitmap *glyphBitmap = &ftFace->glyph->bitmap;
			FT_Bitmap_Embolden(ftLibrary, glyphBitmap, ftWeight, ftWeight);
			
			textureWidth = ALIGN8(glyphBitmap->width);
			textureHeight = ALIGN8(glyphBitmap->rows);
			if(textureWidth == 0)
				textureWidth = 8;
			if(textureHeight == 0)
				textureHeight = 8;

			fontData[pixelSize][charCode].renderOffsetX = (int16_t) ftFace->glyph->bitmap_left;
			fontData[pixelSize][charCode].glyphAdvanceX = (uint16_t) (ftFace->glyph->advance.x >> 6);
			fontData[pixelSize][charCode].glyphIndex = (uint32_t) gIndex;
			fontData[pixelSize][charCode].textureWidth = (uint16_t) textureWidth;
			fontData[pixelSize][charCode].textureHeight = (uint16_t) textureHeight;
			fontData[pixelSize][charCode].renderOffsetY = (int16_t) ftFace->glyph->bitmap_top;
			fontData[pixelSize][charCode].renderOffsetMax = (int16_t) ftFace->glyph->bitmap_top;
			fontData[pixelSize][charCode].renderOffsetMin = (int16_t) glyphBitmap->rows - ftFace->glyph->bitmap_top;
			fontData[pixelSize][charCode].glyphDataTexture = NULL;

			loadGlyphData(glyphBitmap, &fontData[pixelSize][charCode]);

			return &fontData[pixelSize][charCode];
		}
	}
	return NULL;
}

uint16_t FreeTypeGX::cacheGlyphDataComplete(int16_t pixelSize)
{
	uint16_t i = 0;
	FT_UInt gIndex;
	FT_ULong charCode = FT_Get_First_Char(ftFace, &gIndex);
	while(gIndex != 0)
	{
		if(cacheGlyphData(charCode, pixelSize) != NULL)
			++i;
		charCode = FT_Get_Next_Char(ftFace, charCode, &gIndex);
	}
	return i;
}

void FreeTypeGX::loadGlyphData(FT_Bitmap *bmp, ftgxCharData *charData)
{
    int glyphSize = (charData->textureWidth * charData->textureHeight) >> 1;

    uint8_t *glyphData = (uint8_t *) MEM2_alloc(glyphSize);
	if(glyphData == NULL)
		return;
    memset(glyphData, 0x00, glyphSize);

    uint8_t *src = (uint8_t *)bmp->buffer;
    uint8_t *dst = glyphData;
    uint32_t pos, x1, y1, x, y;

    for(y1 = 0; y1 < bmp->rows; y1 += 8)
    {
            for(x1 = 0; x1 < bmp->width; x1 += 8)
            {
                    for(y = y1; y < (y1 + 8); y++)
                    {
                            for(x = x1; x < (x1 + 8); x += 2, dst++)
                            {
                                    if(x >= bmp->width || y >= bmp->rows)
                                            continue;

                                    pos = y * bmp->width + x;
                                    *dst = (src[pos] & 0xF0);

                                        if(x+1 < bmp->width)
                                            *dst |= (src[pos + 1] >> 4);
                            }
                    }
            }
    }
    DCFlushRange(glyphData, glyphSize);
    charData->glyphDataTexture = glyphData;
}

int16_t FreeTypeGX::getStyleOffsetWidth(uint16_t width, uint16_t format)
{
	if (format & FTGX_JUSTIFY_LEFT)
		return 0;
	else if (format & FTGX_JUSTIFY_CENTER)
		return width >> 1;
	else if (format & FTGX_JUSTIFY_RIGHT)
		return width;
	return 0;
}

int16_t FreeTypeGX::getStyleOffsetHeight(int16_t format, uint16_t pixelSize)
{
	map<int16_t, ftgxDataOffset>::iterator itrAlign = ftgxAlign.find(pixelSize);
	if (itrAlign == ftgxAlign.end()) return 0;

	switch (format & FTGX_ALIGN_MASK)
	{
		case FTGX_ALIGN_TOP:
			return itrAlign->second.ascender;

		case FTGX_ALIGN_MIDDLE:
		default:
			return (itrAlign->second.ascender + itrAlign->second.descender + 1) >> 1;

		case FTGX_ALIGN_BOTTOM:
			return itrAlign->second.descender;

		case FTGX_ALIGN_BASELINE:
			return 0;

		case FTGX_ALIGN_GLYPH_TOP:
			return itrAlign->second.max;

		case FTGX_ALIGN_GLYPH_MIDDLE:
			return (itrAlign->second.max + itrAlign->second.min + 1) >> 1;

		case FTGX_ALIGN_GLYPH_BOTTOM:
			return itrAlign->second.min;
	}
	return 0;
}

uint16_t FreeTypeGX::drawText(int16_t x, int16_t y, const wchar_t *text, int16_t pixelSize, GXColor color,
		uint16_t textStyle, uint16_t textWidth, uint16_t widthLimit)
{
	if (!text) return 0;

	uint16_t fullTextWidth = textWidth > 0 ? textWidth : getWidth(text, pixelSize);
	uint16_t x_pos = x, printed = 0;
	uint16_t x_offset = 0, y_offset = 0;
	GXTexObj glyphTexture;
	FT_Vector pairDelta;

	if (textStyle & FTGX_JUSTIFY_MASK)
	{
		x_offset = getStyleOffsetWidth(fullTextWidth, textStyle);
	}
	if (textStyle & FTGX_ALIGN_MASK)
	{
		y_offset = getStyleOffsetHeight(textStyle, pixelSize);
	}

	int i = 0;
	while (text[i])
	{
		if (widthLimit > 0 && (x_pos - x) > widthLimit) break;

		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			if (ftKerningEnabled && i > 0)
			{
				FT_Get_Kerning(ftFace, fontData[pixelSize][text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				x_pos += pairDelta.x >> 6;
			}

			// renderoffsety is to make sure short letters are not aligned with the top of tall letters.
			// renderoffsetx is to properly space wide letters like w or short letters like j.
			// x_offset and y_offset are used for justify x and align y the text.
			// in wiiflow they are 0. getStyleOffsetWidth and getStyleOffsetHeight above are not called. wiiflow does style prior to calling drawText.
			GX_InitTexObj(&glyphTexture, glyphData->glyphDataTexture, glyphData->textureWidth, glyphData->textureHeight, GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE);
			copyTextureToFramebuffer(&glyphTexture, glyphData->textureWidth, glyphData->textureHeight, x_pos + glyphData->renderOffsetX - x_offset, y - glyphData->renderOffsetY - y_offset, color);

			x_pos += glyphData->glyphAdvanceX;
			++printed;
		}
		++i;
	}

	if (textStyle & FTGX_STYLE_MASK)
	{
		getOffset(text, pixelSize, widthLimit);
		drawTextFeature(x + x_offset, y + y_offset, pixelSize, fullTextWidth, &ftgxAlign[pixelSize], textStyle, color);
	}

	return printed;
}

void FreeTypeGX::drawTextFeature(int16_t x, int16_t y, int16_t pixelSize, uint16_t width,
		ftgxDataOffset *offsetData, uint16_t format, GXColor color)
{
	uint16_t featureHeight = pixelSize >> 4 > 0 ? pixelSize >> 4 : 1;

	if (format & FTGX_STYLE_UNDERLINE) this->copyFeatureToFramebuffer(width, featureHeight, x, y + 1, color);

	if (format & FTGX_STYLE_STRIKE) this->copyFeatureToFramebuffer(width, featureHeight, x, y - ((offsetData->max) >> 1), color);
}

uint16_t FreeTypeGX::getWidth(const wchar_t *text, int16_t pixelSize)
{
	if (!text) return 0;

	uint16_t strWidth = 0;
	FT_Vector pairDelta;

	int i = 0;
	while (text[i])
	{
		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			if (ftKerningEnabled && (i > 0))
			{
				FT_Get_Kerning(ftFace, fontData[pixelSize][text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				strWidth += pairDelta.x >> 6;
			}

			strWidth += glyphData->glyphAdvanceX;
		}
		++i;
	}
	return strWidth;
}

uint16_t FreeTypeGX::getCharWidth(const wchar_t wChar, int16_t pixelSize, const wchar_t prevChar)
{
	uint16_t strWidth = 0;
	ftgxCharData * glyphData = cacheGlyphData(wChar, pixelSize);

	if (glyphData != NULL)
	{
		if (ftKerningEnabled && prevChar != 0x0000)
		{
			FT_Vector pairDelta;
			FT_Get_Kerning(ftFace, fontData[pixelSize][prevChar].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT,
					&pairDelta);
			strWidth += pairDelta.x >> 6;
		}
		strWidth += glyphData->glyphAdvanceX;
	}

	return strWidth;
}

uint16_t FreeTypeGX::getHeight(const wchar_t *text, int16_t pixelSize)
{
	getOffset(text, pixelSize);

	return ftgxAlign[pixelSize].max - ftgxAlign[pixelSize].min;
}

void FreeTypeGX::getOffset(const wchar_t *text, int16_t pixelSize, uint16_t widthLimit)
{
	if (ftgxAlign.find(pixelSize) != ftgxAlign.end()) return;

	int16_t strMax = 0, strMin = 9999;
	uint16_t currWidth = 0;

	int i = 0;

	while (text[i])
	{
		if (widthLimit > 0 && currWidth >= widthLimit) break;

		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			strMax = glyphData->renderOffsetMax > strMax ? glyphData->renderOffsetMax : strMax;
			strMin = glyphData->renderOffsetMin < strMin ? glyphData->renderOffsetMin : strMin;
			currWidth += glyphData->glyphAdvanceX;
		}

		++i;
	}

	if (ftPointSize != pixelSize)
	{
		ftPointSize = pixelSize;
		FT_Set_Pixel_Sizes(ftFace, 0, ftPointSize);
	}

	ftgxAlign[pixelSize].ascender = ftFace->size->metrics.ascender >> 6;
	ftgxAlign[pixelSize].descender = ftFace->size->metrics.descender >> 6;
	ftgxAlign[pixelSize].max = strMax;
	ftgxAlign[pixelSize].min = strMin;
}


void FreeTypeGX::copyTextureToFramebuffer(GXTexObj *texObj, uint16_t texWidth, uint16_t texHeight, int16_t screenX, int16_t screenY, GXColor color)
{
	f32	f32TexWidth = (float)texWidth;
	f32 f32TexHeight = (float)texHeight;
	float x = (float)screenX + xPos;
	float y = (float)screenY + yPos;

	GX_LoadTexObj(texObj, GX_TEXMAP0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

	GX_Position3f32(x * xScale, y * yScale, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(0.0f, 0.0f);
	
	GX_Position3f32((f32TexWidth + x) * xScale, y * yScale, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(1.0f, 0.0f);
	
	GX_Position3f32((f32TexWidth + x) * xScale, (f32TexHeight + y) * yScale, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(1.0f, 1.0f);
	
	GX_Position3f32(x * xScale, (f32TexHeight + y) * yScale, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(0.0f, 1.0f);

	GX_End();
}

void FreeTypeGX::copyFeatureToFramebuffer(uint16_t featureWidth, uint16_t featureHeight, int16_t screenX, int16_t screenY, GXColor color)
{
	f32	f32FeatureWidth = (float)featureWidth;
	f32 f32FeatureHeight = (float)featureHeight;
	float x = (float)screenX;// + xPos;
	float y = (float)screenY;// + yPos;
	
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x, y, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	
	GX_Position3f32(f32FeatureWidth + x, y, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	
	GX_Position3f32(f32FeatureWidth + x, f32FeatureHeight + y, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	
	GX_Position3f32(x, f32FeatureHeight + y, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_End();
}