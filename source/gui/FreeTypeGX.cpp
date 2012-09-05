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

/**
 * Default constructor for the FreeTypeGX class.
 * @param positionFormat	Optional positional format (GX_POS_*) of the texture as defined by the libogc gx.h header file. If not specified default value is GX_POS_XYZ.
 */ 
FreeTypeGX::FreeTypeGX(uint8_t positionFormat)
{
	FT_Init_FreeType(&this->ftLibrary);

	this->positionFormat = positionFormat;
    reset();
	this->ftFace = 0;
}

/**
 * Default destructor for the FreeTypeGX class.
 */
FreeTypeGX::~FreeTypeGX()
{
	this->unloadFont();
	FT_Done_Face(this->ftFace);
	FT_Done_FreeType(this->ftLibrary);
}

/**
 * Convert a short char sctring to a wide char string.
 * 
 * This routine converts a supplied shot character string into a wide character string.
 * Note that it is the user's responsibility to clear the returned buffer once it is no longer needed.
 * 
 * @param strChar	Character string to be converted.
 * @return Wide character representation of supplied character string.
 */
wchar_t* FreeTypeGX::charToWideChar(char* strChar)
{
      wchar_t *strWChar;
      strWChar = new wchar_t[strlen(strChar) + 1];

      char *tempSrc = strChar;
      wchar_t *tempDest = strWChar;
      while((*tempDest++ = *tempSrc++));

      return strWChar;
}

/**
 * Loads and processes a specified true type font buffer to a specific point size.
 * 
 * This routine takes a precompiled true type font buffer and loads the necessary processed data into memory. This routine should be called before drawText will succeed. 
 * 
 * @param fontBuffer	A pointer in memory to a precompiled true type font buffer.
 * @param bufferSize	Size of the true type font buffer in bytes.
 * @param pointSize	The desired point size this wrapper's configured font face.
 * @param cacheAll	Optional flag to specify if all font characters should be cached when the class object is created. If specified as false the characters only become cached the first time they are used. If not specified default value is false.
 */
uint16_t FreeTypeGX::loadFont(uint8_t* fontBuffer, FT_Long bufferSize, FT_UInt pointSize, FT_Pos weight, uint32_t index, bool cacheAll)
{
	this->unloadFont();
	this->ftPointSize = pointSize != 0 ? pointSize : this->ftPointSize;
	this->ftWeight = weight;

	// check if the index is valid
	if (index != 0)
	{
		FT_New_Memory_Face(this->ftLibrary, (FT_Byte *)fontBuffer, bufferSize, -1, &this->ftFace);
		if (index >= (uint32_t) this->ftFace->num_faces)
			index = this->ftFace->num_faces - 1; // Use the last face
		FT_Done_Face(this->ftFace);
		this->ftFace = NULL;
	}
	
	FT_New_Memory_Face(this->ftLibrary, (FT_Byte *)fontBuffer, bufferSize, index, &this->ftFace);
	FT_Set_Pixel_Sizes(this->ftFace, 0, this->ftPointSize);

	this->ftSlot = this->ftFace->glyph;
	this->ftKerningEnabled = FT_HAS_KERNING(this->ftFace);
	
	if (cacheAll) {
		return this->cacheGlyphDataComplete();
	}
	
	return 0;
}

/**
 * 
 * \overload
 */
uint16_t FreeTypeGX::loadFont(const uint8_t* fontBuffer, FT_Long bufferSize, FT_UInt pointSize, FT_Pos weight, uint32_t index, bool cacheAll)
{
	return this->loadFont((uint8_t *)fontBuffer, bufferSize, pointSize, weight, index, cacheAll);
}

/**
 * Clears all loaded font glyph data.
 * 
 * This routine clears all members of the font map structure and frees all allocated memory back to the system.
 */
void FreeTypeGX::unloadFont()
{
	if(this->fontData.size() == 0)
		return;

	std::map<wchar_t, ftgxCharData>::iterator itr;
	for(itr = this->fontData.begin(); itr != this->fontData.end(); itr++)
		MEM2_free(itr->second.glyphDataTexture);
	this->fontData.clear();
}

/**
 * Caches the given font glyph in the instance font texture buffer.
 *
 * This routine renders and stores the requested glyph's bitmap and relevant information into its own quickly addressible
 * structure within an instance-specific map.
 * 
 * @param charCode	The requested glyph's character code.
 * @return A pointer to the allocated font structure.
 */
ftgxCharData *FreeTypeGX::cacheGlyphData(wchar_t charCode)
{
	FT_UInt gIndex;
	uint16_t textureWidth = 0, textureHeight = 0;

	gIndex = FT_Get_Char_Index( this->ftFace, charCode );
	if (!FT_Load_Glyph(this->ftFace, gIndex, FT_LOAD_DEFAULT ))
	{
		FT_Render_Glyph( this->ftSlot, FT_RENDER_MODE_NORMAL );

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

/**
 * Locates each character in this wrapper's configured font face and proccess them.
 *
 * This routine locates each character in the configured font face and renders the glyph's bitmap.
 * Each bitmap and relevant information is loaded into its own quickly addressible structure within an instance-specific map.
 */
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

/**
 * Loads the rendered bitmap into the relevant structure's data buffer.
 *
 * This routine does a simple byte-wise copy of the glyph's rendered 8-bit grayscale bitmap into the structure's buffer.
 * Each byte is converted from the bitmap's intensity value into the a uint32_t RGBA value.
 *
 * @param bmp   A pointer to the most recently rendered glyph's bitmap.
 * @param charData  A pointer to an allocated ftgxCharData structure whose data represent that of the last rendered glyph.
 */

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

/**
 * Determines the x offset of the rendered string.
 * 
 * This routine calculates the x offset of the rendered string based off of a supplied positional format parameter.
 * 
 * @param width	Current pixel width of the string.
 * @param format	Positional format of the string.
 */
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

/**
 * Determines the y offset of the rendered string.
 * 
 * This routine calculates the y offset of the rendered string based off of a supplied positional format parameter.
 * 
 * @param offset	Current pixel offset data of the string.
 * @param format	Positional format of the string.
 */
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

/**
 * Processes the supplied text string and prints the results at the specified coordinates.
 * 
 * This routine processes each character of the supplied text string, loads the relevant preprocessed bitmap buffer,
 * a texture from said buffer, and loads the resultant texture into the EFB.
 * 
 * @param x	Screen X coordinate at which to output the text.
 * @param y Screen Y coordinate at which to output the text. Note that this value corresponds to the text string origin and not the top or bottom of the glyphs.
 * @param text	NULL terminated string to output.
 * @param color	Optional color to apply to the text characters. If not specified default value is ftgxWhite: (GXColor){0xff, 0xff, 0xff, 0xff}
 * @param textStyle	Flags which specify any styling which should be applied to the rendered string.
 * @return The number of characters printed.
 */
uint16_t FreeTypeGX::drawText(uint16_t x, uint16_t y, wchar_t *text, GXColor color, uint16_t textStyle)
{
	uint16_t strLength = wcslen(text);
	uint16_t x_pos = x, printed = 0;
	uint16_t x_offset = 0, y_offset = 0;
	GXTexObj glyphTexture;
	FT_Vector pairDelta;
	
	if(textStyle & 0x000F)
		x_offset = this->getStyleOffsetWidth(this->getWidth(text), textStyle);
	if(textStyle & 0x00F0)
		y_offset = this->getStyleOffsetHeight(this->getOffset(text), textStyle);

	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	for (uint16_t i = 0; i < strLength; i++) 
	{
		ftgxCharData* glyphData = NULL;
		if( this->fontData.find(text[i]) != this->fontData.end() )
			glyphData = &this->fontData[text[i]];
		else
			glyphData = this->cacheGlyphData(text[i]);

		if(glyphData != NULL)
		{
			
			if(this->ftKerningEnabled && i)
			{
				FT_Get_Kerning( this->ftFace, this->fontData[text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta );
				x_pos += pairDelta.x >> 6;
			}

			GX_InitTexObj(&glyphTexture, glyphData->glyphDataTexture, glyphData->textureWidth, glyphData->textureHeight, GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE);
			this->copyTextureToFramebuffer(&glyphTexture, this->positionFormat, glyphData->textureWidth, glyphData->textureHeight, x_pos - x_offset, y - glyphData->renderOffsetY - y_offset, color);

			x_pos += glyphData->glyphAdvanceX;
			printed++;
		}
	}
	
	if(textStyle & 0x0F00)
		this->drawTextFeature(x - x_offset, y, this->getWidth(text), this->getOffset(text), textStyle, color);
	
	return printed;
}

/**
 * \overload
 */
uint16_t FreeTypeGX::drawText(uint16_t x, uint16_t y, wchar_t const *text, GXColor color, uint16_t textStyle)
{
	return this->drawText(x, y, (wchar_t *)text, color, textStyle);
}

void FreeTypeGX::drawTextFeature(uint16_t x, uint16_t y, uint16_t width,  ftgxDataOffset offsetData, uint16_t format, GXColor color)
{
	uint16_t featureHeight = this->ftPointSize >> 4 > 0 ? this->ftPointSize >> 4 : 1;
	
	if (format & FTGX_STYLE_UNDERLINE )
	{
		switch(format & 0x00F0)
		{
			case FTGX_ALIGN_TOP:
				this->copyFeatureToFramebuffer(this->positionFormat, width, featureHeight, x, y + offsetData.max + 1, color);
				break;
			case FTGX_ALIGN_MIDDLE:
				this->copyFeatureToFramebuffer(this->positionFormat, width, featureHeight, x, y + ((offsetData.max - offsetData.min) >> 1) + 1, color);
				break;
			case FTGX_ALIGN_BOTTOM:
				this->copyFeatureToFramebuffer(this->positionFormat, width, featureHeight, x, y - offsetData.min, color);
				break;
			default:
				this->copyFeatureToFramebuffer(this->positionFormat, width, featureHeight, x, y + 1, color);
				break;
		}
	}
	
	if (format & FTGX_STYLE_STRIKE )
	{
		switch(format & 0x00F0)
		{
			case FTGX_ALIGN_TOP:
				this->copyFeatureToFramebuffer(this->positionFormat, width, featureHeight, x, y + ((offsetData.max + offsetData.min) >> 1), color);
				break;
			case FTGX_ALIGN_MIDDLE:
				this->copyFeatureToFramebuffer(this->positionFormat, width, featureHeight, x, y, color);
				break;
			case FTGX_ALIGN_BOTTOM:
				this->copyFeatureToFramebuffer(this->positionFormat, width, featureHeight, x, y - ((offsetData.max + offsetData.min) >> 1), color);
				break;
			default:
				this->copyFeatureToFramebuffer(this->positionFormat, width, featureHeight, x, y - ((offsetData.max - offsetData.min) >> 1), color);
				break;
		}
	}
}

/**
 * Processes the supplied string and return the width of the string in pixels.
 * 
 * This routine processes each character of the supplied text string and calculates the width of the entire string.
 * Note that if precaching of the entire font set is not enabled any uncached glyph will be cached after the call to this function.
 * 
 * @param text	NULL terminated string to calculate.
 * @return The width of the text string in pixels.
 */
uint16_t FreeTypeGX::getWidth(wchar_t *text)
{
	uint16_t strLength = wcslen(text);
	uint16_t strWidth = 0;
	FT_Vector pairDelta;
	
	for (uint16_t i = 0; i < strLength; i++)
	{
		ftgxCharData* glyphData = NULL;
		if( this->fontData.find(text[i]) != this->fontData.end() )
			glyphData = &this->fontData[text[i]];
		else
			glyphData = this->cacheGlyphData(text[i]);
		if(glyphData != NULL)
		{
			if(this->ftKerningEnabled && (i > 0))
			{
				FT_Get_Kerning( this->ftFace, this->fontData[text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta );
				strWidth += pairDelta.x >> 6;
			}

			strWidth += glyphData->glyphAdvanceX;
		}
	}

	return strWidth;
}

/**
 * 
 * \overload
 */
uint16_t FreeTypeGX::getWidth(wchar_t const *text)
{
	return this->getWidth((wchar_t *)text);
}

/**
 * Processes the supplied string and return the height of the string in pixels.
 * 
 * This routine processes each character of the supplied text string and calculates the height of the entire string.
 * Note that if precaching of the entire font set is not enabled any uncached glyph will be cached after the call to this function.
 * 
 * @param text	NULL terminated string to calculate.
 * @return The height of the text string in pixels.
 */
uint16_t FreeTypeGX::getHeight(wchar_t *text)
{
	ftgxDataOffset offset = this->getOffset(text);
	return offset.max + offset.min;
}

/**
 * 
 * \overload
 */
uint16_t FreeTypeGX::getHeight(wchar_t const *text)
{
	return this->getHeight((wchar_t *)text);
}

/**
 * Get the maximum offset above and minimum offset below the font origin line.
 * 
 * This function calculates the maximum pixel height above the font origin line and the minimum
 * pixel height below the font origin line and returns the values in an addressible structure.
 * 
 * @param text	NULL terminated string to calculate.
 * @return The max and min values above and below the font origin line.
 */
ftgxDataOffset FreeTypeGX::getOffset(wchar_t *text)
{
	uint16_t strLength = wcslen(text);
	uint16_t strMax = 0, strMin = 0;
	
	for (uint16_t i = 0; i < strLength; i++)
	{
		ftgxCharData* glyphData = NULL;
		if( this->fontData.find(text[i]) != this->fontData.end() )
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

/**
 * 
 * \overload
 */
ftgxDataOffset FreeTypeGX::getOffset(wchar_t const *text)
{
	return this->getOffset(text);
}

/**
 * Copies the supplied texture quad to the EFB. 
 * 
 * This routine uses the in-built GX quad builder functions to define the texture bounds and location on the EFB target.
 * 
 * @param texObj	A pointer to the glyph's initialized texture object.
 * @param positionFormat	The positional format of the graphics subsystem.
 * @param texWidth	The pixel width of the texture object.
 * @param texHeight	The pixel height of the texture object.
 * @param screenX	The screen X coordinate at which to output the rendered texture.
 * @param screenY	The screen Y coordinate at which to output the rendered texture.
 * @param color	Color to apply to the texture.
 */
void FreeTypeGX::copyTextureToFramebuffer(GXTexObj *texObj, uint8_t positionFormat, uint16_t texWidth, uint16_t texHeight, int16_t screenX, int16_t screenY, GXColor color)
{
	f32	f32TexWidth = texWidth,	f32TexHeight = texHeight;
	float x = (float)screenX + xPos;
	float y = (float)screenY + yPos;

	GX_LoadTexObj(texObj, GX_TEXMAP0);

//	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
//	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
  
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  	switch(positionFormat)
	{
	  	case GX_POS_XY:
			GX_Position2f32(x * xScale, y * yScale);
			GX_Color4u8(color.r, color.g, color.b, color.a);
	 		GX_TexCoord2f32(0.0f, 0.0f);
	
	 		GX_Position2f32((f32TexWidth + x) * xScale, y * yScale);
			GX_Color4u8(color.r, color.g, color.b, color.a);
			GX_TexCoord2f32(1.0f, 0.0f);
	
			GX_Position2f32((f32TexWidth + x)  * xScale, (f32TexHeight + y) * yScale);
			GX_Color4u8(color.r, color.g, color.b, color.a);
			GX_TexCoord2f32(1.0f, 1.0f);
	
			GX_Position2f32(x * xScale, (f32TexHeight + y) * yScale);
			GX_Color4u8(color.r, color.g, color.b, color.a);
			GX_TexCoord2f32(0.0f, 1.0f);
			break;

	  	case GX_POS_XYZ:
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
			break;
	  	default:
	  		break;
  	}
	GX_End();
//	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
//	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
}

/**
 * Creates a feature quad to the EFB. 
 * 
 * 
 *
 * @param positionFormat	The positional format of the graphics subsystem.
 * @param featureWidth	The pixel width of the quad.
 * @param featureHeight	The pixel height of the quad.
 * @param screenX	The screen X coordinate at which to output the quad.
 * @param screenY	The screen Y coordinate at which to output the quad.
 * @param color	Color to apply to the texture.
 */
void FreeTypeGX::copyFeatureToFramebuffer(uint8_t positionFormat, uint16_t featureWidth, uint16_t featureHeight, int16_t screenX, int16_t screenY, GXColor color)
{
	f32	f32FeatureWidth = featureWidth,	f32FeatureHeight = featureHeight;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  	switch(positionFormat)
	{
	  	case GX_POS_XY:
			GX_Position2f32(screenX, screenY);
			GX_Color4u8(color.r, color.g, color.b, color.a);
	
	 		GX_Position2f32(f32FeatureWidth + screenX, screenY);
			GX_Color4u8(color.r, color.g, color.b, color.a);
	
			GX_Position2f32(f32FeatureWidth + screenX, f32FeatureHeight + screenY);
			GX_Color4u8(color.r, color.g, color.b, color.a);
	
			GX_Position2f32(screenX, f32FeatureHeight + screenY);
			GX_Color4u8(color.r, color.g, color.b, color.a);
			break;

	  	case GX_POS_XYZ:
			GX_Position3f32(screenX, screenY, 0);
			GX_Color4u8(color.r, color.g, color.b, color.a);
	
	 		GX_Position3f32(f32FeatureWidth + screenX, screenY, 0);
			GX_Color4u8(color.r, color.g, color.b, color.a);
	
			GX_Position3f32(f32FeatureWidth + screenX, f32FeatureHeight + screenY, 0);
			GX_Color4u8(color.r, color.g, color.b, color.a);
	
			GX_Position3f32(screenX, f32FeatureHeight + screenY, 0);
			GX_Color4u8(color.r, color.g, color.b, color.a);
			break;
	  	default:
	  		break;
  	}
	GX_End();
}
