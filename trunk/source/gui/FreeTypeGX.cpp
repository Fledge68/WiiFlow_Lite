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
#include <stdio.h>
#include "FreeTypeGX.h"
#include "memory/mem2.hpp"

FreeTypeGX::FreeTypeGX()
{
	FT_Init_FreeType(&this->ftLibrary);
    reset();
	this->ftFace = 0;
}

FreeTypeGX::~FreeTypeGX()
{
	this->unloadFont();
	FT_Done_Face(this->ftFace);
	FT_Done_FreeType(this->ftLibrary);
}

wchar_t* FreeTypeGX::charToWideChar(char* strChar)
{
      wchar_t *strWChar;
      strWChar = new wchar_t[strlen(strChar) + 1];

      char *tempSrc = strChar;
      wchar_t *tempDest = strWChar;
      while((*tempDest++ = *tempSrc++));

      return strWChar;
}

uint16_t FreeTypeGX::loadFont(FT_Byte *fontBuffer, FT_Long bufferSize, FT_UInt pointSize, FT_Pos weight, uint32_t index, bool cacheAll)
{
	this->unloadFont();
	this->ftPointSize = pointSize != 0 ? pointSize : this->ftPointSize;
	this->ftWeight = weight;

	// Check if the index is valid
	if (index != 0)
	{
		FT_New_Memory_Face(this->ftLibrary, fontBuffer, bufferSize, -1, &this->ftFace);
		if (index >= (uint32_t) this->ftFace->num_faces)
			index = this->ftFace->num_faces - 1; // Use the last face
		FT_Done_Face(this->ftFace);
		this->ftFace = NULL;
	}
	
	FT_New_Memory_Face(this->ftLibrary, fontBuffer, bufferSize, index, &this->ftFace);
	FT_Set_Pixel_Sizes(this->ftFace, 0, this->ftPointSize);

	this->ftSlot = this->ftFace->glyph;
	this->ftKerningEnabled = FT_HAS_KERNING(this->ftFace);
	
	if (cacheAll)
		return this->cacheGlyphDataComplete();
	
	return 0;
}

void FreeTypeGX::unloadFont()
{
	if(this->fontData.size() == 0)
		return;

	std::map<wchar_t, ftgxCharData>::iterator itr;
	for(itr = this->fontData.begin(); itr != this->fontData.end(); itr++)
		MEM2_free(itr->second.glyphDataTexture);
	this->fontData.clear();
}

ftgxCharData *FreeTypeGX::cacheGlyphData(wchar_t charCode)
{
	FT_UInt gIndex;
	uint16_t textureWidth = 0, textureHeight = 0;

	gIndex = FT_Get_Char_Index( this->ftFace, charCode );
	if(!FT_Load_Glyph(this->ftFace, gIndex, FT_LOAD_DEFAULT | FT_LOAD_RENDER))
	{
		if(this->ftSlot->format == FT_GLYPH_FORMAT_BITMAP)
		{
			FT_Bitmap *glyphBitmap = &this->ftSlot->bitmap;
			FT_Bitmap_Embolden(this->ftLibrary, glyphBitmap, this->ftWeight, this->ftWeight);

			textureWidth = glyphBitmap->width % 8 == 0 ? glyphBitmap->width : 8 + glyphBitmap->width - (glyphBitmap->width % 8);
			textureHeight = glyphBitmap->rows % 8 == 0 ? glyphBitmap->rows : 8 + glyphBitmap->rows - (glyphBitmap->rows % 8);

			this->fontData[charCode] = (ftgxCharData)
			{
				this->ftSlot->advance.x >> 6,
				gIndex,
				textureWidth,
				textureHeight,
				this->ftSlot->bitmap_top,
				this->ftSlot->bitmap_top,
				textureHeight - this->ftSlot->bitmap_top,
				NULL
			};
			this->loadGlyphData(glyphBitmap, &this->fontData[charCode]);

			return &this->fontData[charCode];
		}
	}

	return NULL;
}

uint16_t FreeTypeGX::cacheGlyphDataComplete()
{
	uint16_t i = 0;
	FT_UInt gIndex;
	FT_ULong charCode = FT_Get_First_Char( this->ftFace, &gIndex );
	while( gIndex != 0 )
	{
		if(this->cacheGlyphData(charCode) != NULL)
			i++;

		charCode = FT_Get_Next_Char( this->ftFace, charCode, &gIndex );
	}
	
	return i;
}

void FreeTypeGX::loadGlyphData(FT_Bitmap *bmp, ftgxCharData *charData)
{
    int glyphSize = (charData->textureWidth * charData->textureHeight) >> 1;

    uint8_t *glyphData = (uint8_t *) MEM2_alloc(glyphSize);
	if(glyphData < 0)
		return;
    memset(glyphData, 0x00, glyphSize);

    uint8_t *src = (uint8_t *)bmp->buffer;
    uint8_t *dst = glyphData;
    int32_t pos, x1, y1, x, y;

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

uint16_t FreeTypeGX::getStyleOffsetWidth(uint16_t width, uint16_t format)
{
	if (format & FTGX_JUSTIFY_LEFT )
		return 0;
	else if (format & FTGX_JUSTIFY_CENTER )
		return width >> 1;
	else if (format & FTGX_JUSTIFY_RIGHT )
		return width;
	return 0;
}

uint16_t FreeTypeGX::getStyleOffsetHeight(ftgxDataOffset offset, uint16_t format)
{
	if (format & FTGX_ALIGN_TOP )
		return -offset.max;
	else if (format & FTGX_ALIGN_MIDDLE )
		return -(offset.max - offset.min) >> 1;
	else if (format & FTGX_ALIGN_BOTTOM )
		return offset.min;
	return 0;
}

uint16_t FreeTypeGX::drawText(uint16_t x, uint16_t y, const wchar_t *text, GXColor color, uint16_t textStyle)
{
	uint16_t strLength = wcslen(text);
	uint16_t x_pos = x, printed = 0;
	uint16_t x_offset = 0, y_offset = 0;
	GXTexObj glyphTexture;
	FT_Vector pairDelta;
	
	if(textStyle & FTGX_JUSTIFY_MASK)
		x_offset = this->getStyleOffsetWidth(this->getWidth(text), textStyle);
	if(textStyle & FTGX_ALIGN_MASK) 
		y_offset = this->getStyleOffsetHeight(this->getOffset(text), textStyle);

	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	for (uint16_t i = 0; i < strLength; i++) 
	{
		ftgxCharData* glyphData = NULL;
		if( this->fontData.find(text[i]) != this->fontData.end())
			glyphData = &this->fontData[text[i]];
		else
			glyphData = this->cacheGlyphData(text[i]);

		if(glyphData != NULL)
		{
			
			if(this->ftKerningEnabled && i)
			{
				FT_Get_Kerning( this->ftFace, this->fontData[text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				x_pos += pairDelta.x >> 6;
			}

			GX_InitTexObj(&glyphTexture, glyphData->glyphDataTexture, glyphData->textureWidth, glyphData->textureHeight, GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE);
			this->copyTextureToFramebuffer(&glyphTexture, glyphData->textureWidth, glyphData->textureHeight, x_pos - x_offset, y - glyphData->renderOffsetY - y_offset, color);

			x_pos += glyphData->glyphAdvanceX;
			printed++;
		}
	}
	
	if(textStyle & FTGX_STYLE_MASK)
		this->drawTextFeature(x - x_offset, y, this->getWidth(text), this->getOffset(text), textStyle, color);
	
	return printed;
}

void FreeTypeGX::drawTextFeature(uint16_t x, uint16_t y, uint16_t width,  ftgxDataOffset offsetData, uint16_t format, GXColor color)
{
	uint16_t featureHeight = this->ftPointSize >> 4 > 0 ? this->ftPointSize >> 4 : 1;
	
	if (format & FTGX_STYLE_UNDERLINE)
		this->copyFeatureToFramebuffer(width, featureHeight, x, y + 1, color);
	
	if (format & FTGX_STYLE_STRIKE)
		this->copyFeatureToFramebuffer(width, featureHeight, x, y - ((offsetData.max) >> 1), color);
}

uint16_t FreeTypeGX::getWidth(const wchar_t *text)
{
	uint16_t strLength = wcslen(text);
	uint16_t strWidth = 0;
	FT_Vector pairDelta;
	
	for (uint16_t i = 0; i < strLength; i++)
	{
		ftgxCharData* glyphData = NULL;
		if( this->fontData.find(text[i]) != this->fontData.end())
			glyphData = &this->fontData[text[i]];
		else
			glyphData = this->cacheGlyphData(text[i]);
		if(glyphData != NULL)
		{
			if(this->ftKerningEnabled && (i > 0))
			{
				FT_Get_Kerning( this->ftFace, this->fontData[text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				strWidth += pairDelta.x >> 6;
			}

			strWidth += glyphData->glyphAdvanceX;
		}
	}

	return strWidth;
}

uint16_t FreeTypeGX::getHeight(const wchar_t *text)
{
	ftgxDataOffset offset = this->getOffset(text);
	return offset.max + offset.min;
}

ftgxDataOffset FreeTypeGX::getOffset(const wchar_t *text)
{
	uint16_t strLength = wcslen(text);
	uint16_t strMax = 0, strMin = 0;
	
	for (uint16_t i = 0; i < strLength; i++)
	{
		ftgxCharData* glyphData = NULL;
		if( this->fontData.find(text[i]) != this->fontData.end())
			glyphData = &this->fontData[text[i]];
		else
			glyphData = this->cacheGlyphData(text[i]);

		if(glyphData != NULL)
		{
			strMax = glyphData->renderOffsetMax > strMax ? glyphData->renderOffsetMax : strMax;
			strMin = glyphData->renderOffsetMin > strMin ? glyphData->renderOffsetMin : strMin;
		}
	}
	return (ftgxDataOffset){strMax, strMin};
}

void FreeTypeGX::copyTextureToFramebuffer(GXTexObj *texObj, uint16_t texWidth, uint16_t texHeight, int16_t screenX, int16_t screenY, GXColor color)
{
	f32	f32TexWidth = texWidth,	f32TexHeight = texHeight;
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
	f32	f32FeatureWidth = featureWidth,	f32FeatureHeight = featureHeight;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

	GX_Position3f32(screenX, screenY, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	
	GX_Position3f32(f32FeatureWidth + screenX, screenY, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	
	GX_Position3f32(f32FeatureWidth + screenX, f32FeatureHeight + screenY, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	
	GX_Position3f32(screenX, f32FeatureHeight + screenY, 0);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	
	GX_End();
}