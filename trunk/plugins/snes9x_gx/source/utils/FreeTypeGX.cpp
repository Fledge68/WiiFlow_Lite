/*
 * FreeTypeGX is a wrapper class for libFreeType which renders a compiled
 * FreeType parsable font into a GX texture for Wii homebrew development.
 * Copyright (C) 2008 Armin Tamzarian
 * Modified by Tantric, 2009
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

static FT_Library ftLibrary;	/**< FreeType FT_Library instance. */
static FT_Face ftFace;			/**< FreeType reusable FT_Face typographic object. */
static FT_GlyphSlot ftSlot;		/**< FreeType reusable FT_GlyphSlot glyph container object. */

FreeTypeGX *fontSystem[MAX_FONT_SIZE+1];

void InitFreeType(uint8_t* fontBuffer, FT_Long bufferSize)
{
	FT_Init_FreeType(&ftLibrary);
	FT_New_Memory_Face(ftLibrary, (FT_Byte *)fontBuffer, bufferSize, 0, &ftFace);
	ftSlot = ftFace->glyph;

	for(int i=0; i<50; i++)
		fontSystem[i] = NULL;
}

void DeinitFreeType()
{
	ClearFontData();
	FT_Done_FreeType(ftLibrary);
	ftLibrary = NULL;
}

void ChangeFontSize(FT_UInt pixelSize)
{
	FT_Set_Pixel_Sizes(ftFace, 0, pixelSize);
}

void ClearFontData()
{
	for(int i=0; i<50; i++)
	{
		if(fontSystem[i])
			delete fontSystem[i];
		fontSystem[i] = NULL;
	}
}

/**
 * Convert a short char string to a wide char string.
 *
 * This routine converts a supplied shot character string into a wide character string.
 * Note that it is the user's responsibility to clear the returned buffer once it is no longer needed.
 *
 * @param strChar	Character string to be converted.
 * @return Wide character representation of supplied character string.
 */

wchar_t* charToWideChar(const char* strChar)
{
	wchar_t *strWChar = new wchar_t[strlen(strChar) + 1];
	if(!strWChar)
		return NULL;

	int bt = mbstowcs(strWChar, strChar, strlen(strChar));
	if (bt > 0)
	{
		strWChar[bt] = (wchar_t)'\0';
		return strWChar;
	}

	wchar_t *tempDest = strWChar;
	while((*tempDest++ = *strChar++));

	return strWChar;
}

/**
 * Default constructor for the FreeTypeGX class.
 *
 * @param vertexIndex	Optional vertex format index (GX_VTXFMT*) of the glyph textures as defined by the libogc gx.h header file. If not specified default value is GX_VTXFMT1.
 */
FreeTypeGX::FreeTypeGX(FT_UInt pixelSize, uint8_t vertexIndex)
{
	this->setVertexFormat(vertexIndex);
	this->setCompatibilityMode(FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR | FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE);
	this->ftPointSize = pixelSize;
	this->ftKerningEnabled = FT_HAS_KERNING(ftFace);
}

/**
 * Default destructor for the FreeTypeGX class.
 */
FreeTypeGX::~FreeTypeGX()
{
	this->unloadFont();
}

/**
 * Setup the vertex attribute formats for the glyph textures.
 *
 * This function sets up the vertex format for the glyph texture on the specified vertex format index.
 * Note that this function should not need to be called except if the vertex formats are cleared or the specified
 * vertex format index is modified.
 *
 * @param vertexIndex	Vertex format index (GX_VTXFMT*) of the glyph textures as defined by the libogc gx.h header file.
*/
void FreeTypeGX::setVertexFormat(uint8_t vertexIndex)
{
	this->vertexIndex = vertexIndex;
	GX_SetVtxAttrFmt(this->vertexIndex, GX_VA_POS, GX_POS_XY, GX_S16, 0);
	GX_SetVtxAttrFmt(this->vertexIndex, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetVtxAttrFmt(this->vertexIndex, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
}

/**
 * Sets the TEV and VTX rendering compatibility requirements for the class.
 *
 * This sets up the default TEV opertion and VTX descriptions rendering values for the class. This ensures that FreeTypeGX
 * can remain compatible with external liraries or project code. Certain external libraries or code by design or lack of
 * foresight assume that the TEV opertion and VTX descriptions values will remain constant or are always returned to a
 * certain value. This will enable compatibility with those libraries and any other code which cannot or will not be changed.
 *
 * @param compatibilityMode	Compatibility descritor (FTGX_COMPATIBILITY_*) as defined in FreeTypeGX.h
*/
void FreeTypeGX::setCompatibilityMode(uint32_t compatibilityMode)
{
	this->compatibilityMode = compatibilityMode;
}

/**
 * Sets the TEV operation and VTX descriptor values after texture rendering it complete.
 *
 * This function calls the GX_SetTevOp and GX_SetVtxDesc functions with the compatibility parameters specified
 * in setCompatibilityMode.
 */
void FreeTypeGX::setDefaultMode()
{
	if(this->compatibilityMode)
	{
		switch(this->compatibilityMode & 0x00FF)
		{
			case FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_MODULATE:
				GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
				break;
			case FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_DECAL:
				GX_SetTevOp(GX_TEVSTAGE0, GX_DECAL);
				break;
			case FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_BLEND:
				GX_SetTevOp(GX_TEVSTAGE0, GX_BLEND);
				break;
			case FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_REPLACE:
				GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
				break;
			case FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR:
				GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
				break;
			default:
				break;
		}

		switch(this->compatibilityMode & 0xFF00)
		{
			case FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE:
				GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
				break;
			case FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_DIRECT:
				GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
				break;
			case FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_INDEX8:
				GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX8);
				break;
			case FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_INDEX16:
				GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);
				break;
			default:
				break;
		}
	}
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
	for(std::map<wchar_t, ftgxCharData>::iterator i = this->fontData.begin(), iEnd = this->fontData.end(); i != iEnd; ++i)
		free(i->second.glyphDataTexture);
	this->fontData.clear();
}

uint16_t FreeTypeGX::adjustTextureWidth(uint16_t textureWidth)
{
	uint16_t alignment = 4;
	return textureWidth % alignment == 0 ? textureWidth : alignment + textureWidth - (textureWidth % alignment);
}

uint16_t FreeTypeGX::adjustTextureHeight(uint16_t textureHeight)
{
	uint16_t alignment = 4;
	return textureHeight % alignment == 0 ? textureHeight : alignment + textureHeight - (textureHeight % alignment);
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

	gIndex = FT_Get_Char_Index( ftFace, charCode );
	if (!FT_Load_Glyph(ftFace, gIndex, FT_LOAD_DEFAULT | FT_LOAD_RENDER ))
	{
		if(ftSlot->format == FT_GLYPH_FORMAT_BITMAP)
		{
			FT_Bitmap *glyphBitmap = &ftSlot->bitmap;

			textureWidth = adjustTextureWidth(glyphBitmap->width);
			textureHeight = adjustTextureHeight(glyphBitmap->rows);

			this->fontData[charCode] = (ftgxCharData){
				ftSlot->bitmap_left,
				ftSlot->advance.x >> 6,
				gIndex,
				textureWidth,
				textureHeight,
				ftSlot->bitmap_top,
				ftSlot->bitmap_top,
				glyphBitmap->rows - ftSlot->bitmap_top,
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
	uint32_t i = 0;
	FT_UInt gIndex;
	FT_ULong charCode = FT_Get_First_Char( ftFace, &gIndex );
	while ( gIndex != 0 )
	{
		if(this->cacheGlyphData(charCode) != NULL)
			++i;
		charCode = FT_Get_Next_Char( ftFace, charCode, &gIndex );
	}
	return (uint16_t)(i);
}

/**
 * Loads the rendered bitmap into the relevant structure's data buffer.
 *
 * This routine does a simple byte-wise copy of the glyph's rendered 8-bit grayscale bitmap into the structure's buffer.
 * Each byte is converted from the bitmap's intensity value into the a uint32_t RGBA value.
 *
 * @param bmp	A pointer to the most recently rendered glyph's bitmap.
 * @param charData	A pointer to an allocated ftgxCharData structure whose data represent that of the last rendered glyph.
 *
 * Optimized for RGBA8 use by Dimok.
 */
void FreeTypeGX::loadGlyphData(FT_Bitmap *bmp, ftgxCharData *charData)
{
    int length = charData->textureWidth * charData->textureHeight * 4;

	uint8_t * glyphData = (uint8_t *) memalign(32, length);
	if(!glyphData)
        return;

	memset(glyphData, 0x00, length);

	uint8_t *src = (uint8_t *)bmp->buffer;
	uint32_t offset;

	for (int imagePosY = 0; imagePosY < bmp->rows; ++imagePosY)
	{
		for (int imagePosX = 0; imagePosX < bmp->width; ++imagePosX)
		{
		    offset = ((((imagePosY >> 2) * (charData->textureWidth >> 2) + (imagePosX >> 2)) << 5) + ((imagePosY & 3) << 2) + (imagePosX & 3)) << 1;
			glyphData[offset] = *src;
			glyphData[offset+1] = *src;
			glyphData[offset+32] = *src;
			glyphData[offset+33] = *src;
			++src;
		}
	}
	DCFlushRange(glyphData, length);
	charData->glyphDataTexture = (uint32_t *) glyphData;
}

/**
 * Determines the x offset of the rendered string.
 *
 * This routine calculates the x offset of the rendered string based off of a supplied positional format parameter.
 *
 * @param width	Current pixel width of the string.
 * @param format	Positional format of the string.
 */
int16_t FreeTypeGX::getStyleOffsetWidth(uint16_t width, uint16_t format)
{
	if (format & FTGX_JUSTIFY_LEFT)
		return 0;
	else if (format & FTGX_JUSTIFY_CENTER)
		return -(width >> 1);
	else if (format & FTGX_JUSTIFY_RIGHT)
		return -width;
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
int16_t FreeTypeGX::getStyleOffsetHeight(ftgxDataOffset *offset, uint16_t format)
{
	switch(format & FTGX_ALIGN_MASK)
	{
		case FTGX_ALIGN_TOP:
			return offset->ascender;

		default:
		case FTGX_ALIGN_MIDDLE:
			return  (offset->ascender + offset->descender + 1) >> 1;

		case FTGX_ALIGN_BOTTOM:
			return offset->descender;

		case FTGX_ALIGN_BASELINE:
			return 0;

		case FTGX_ALIGN_GLYPH_TOP:
			return offset->max;

		case FTGX_ALIGN_GLYPH_MIDDLE:
			return (offset->max + offset->min + 1) >> 1;

		case FTGX_ALIGN_GLYPH_BOTTOM:
			return offset->min;
	}
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
uint16_t FreeTypeGX::drawText(int16_t x, int16_t y, wchar_t *text, GXColor color, uint16_t textStyle)
{
	uint16_t x_pos = x, printed = 0;
	uint16_t x_offset = 0, y_offset = 0;
	GXTexObj glyphTexture;
	FT_Vector pairDelta;
	ftgxDataOffset offset;

	if(textStyle & FTGX_JUSTIFY_MASK)
	{
		x_offset = this->getStyleOffsetWidth(this->getWidth(text), textStyle);
	}
	if(textStyle & FTGX_ALIGN_MASK)
	{
		this->getOffset(text, &offset);
		y_offset = this->getStyleOffsetHeight(&offset, textStyle);
	}

	int i = 0;
	while (text[i])
	{
		ftgxCharData* glyphData = NULL;
		if (this->fontData.find(text[i]) != this->fontData.end())
		{
			glyphData = &this->fontData[text[i]];
		}
		else
		{
			glyphData = this->cacheGlyphData(text[i]);
		}

		if (glyphData != NULL)
		{
			if (this->ftKerningEnabled && i)
			{
				FT_Get_Kerning(ftFace, this->fontData[text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				x_pos += pairDelta.x >> 6;
			}

			GX_InitTexObj(&glyphTexture, glyphData->glyphDataTexture, glyphData->textureWidth, glyphData->textureHeight, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
			this->copyTextureToFramebuffer(&glyphTexture, glyphData->textureWidth, glyphData->textureHeight, x_pos + glyphData->renderOffsetX + x_offset, y - glyphData->renderOffsetY + y_offset, color);

			x_pos += glyphData->glyphAdvanceX;
			++printed;
		}
		++i;
	}

	if(textStyle & FTGX_STYLE_MASK)
	{
		this->getOffset(text, &offset);
		this->drawTextFeature(x + x_offset, y + y_offset, this->getWidth(text), &offset, textStyle, color);
	}

	return printed;
}

/**
 * \overload
 */
uint16_t FreeTypeGX::drawText(int16_t x, int16_t y, wchar_t const *text, GXColor color, uint16_t textStyle)
{
	return this->drawText(x, y, (wchar_t *)text, color, textStyle);
}

void FreeTypeGX::drawTextFeature(int16_t x, int16_t y, uint16_t width, ftgxDataOffset *offsetData, uint16_t format, GXColor color)
{
	uint16_t featureHeight = this->ftPointSize >> 4 > 0 ? this->ftPointSize >> 4 : 1;

	if (format & FTGX_STYLE_UNDERLINE)
		this->copyFeatureToFramebuffer(width, featureHeight, x, y + 1, color);

	if (format & FTGX_STYLE_STRIKE)
		this->copyFeatureToFramebuffer(width, featureHeight, x, y - ((offsetData->max) >> 1), color);
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
	uint16_t strWidth = 0;
	FT_Vector pairDelta;

	int i = 0;
	while (text[i])
	{
		ftgxCharData* glyphData = NULL;
		if (this->fontData.find(text[i]) != this->fontData.end())
		{
			glyphData = &this->fontData[text[i]];
		}
		else
		{
			glyphData = this->cacheGlyphData(text[i]);
		}

		if (glyphData != NULL)
		{
			if (this->ftKerningEnabled && (i > 0))
			{
				FT_Get_Kerning(ftFace, this->fontData[text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				strWidth += pairDelta.x >> 6;
			}

			strWidth += glyphData->glyphAdvanceX;
		}
		++i;
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
	ftgxDataOffset offset;
	this->getOffset(text, &offset);
	return offset.max - offset.min;
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
 * @param offset returns the max and min values above and below the font origin line
 *
 */
void FreeTypeGX::getOffset(wchar_t *text, ftgxDataOffset* offset)
{
	int16_t strMax = 0, strMin = 9999;

	int i = 0;
	while (text[i])
	{
		ftgxCharData* glyphData = NULL;
		if (this->fontData.find(text[i]) != this->fontData.end())
		{
			glyphData = &this->fontData[text[i]];
		}
		else
		{
			glyphData = this->cacheGlyphData(text[i]);
		}

		if(glyphData != NULL)
		{
			strMax = glyphData->renderOffsetMax > strMax ? glyphData->renderOffsetMax : strMax;
			strMin = glyphData->renderOffsetMin < strMin ? glyphData->renderOffsetMin : strMin;
		}
		++i;
	}
	offset->ascender = ftFace->size->metrics.ascender>>6;
	offset->descender = ftFace->size->metrics.descender>>6;
	offset->max = strMax;
	offset->min = strMin;
}

/**
 *
 * \overload
 */
void FreeTypeGX::getOffset(wchar_t const *text, ftgxDataOffset* offset)
{
	this->getOffset(text, offset);
}

/**
 * Copies the supplied texture quad to the EFB.
 *
 * This routine uses the in-built GX quad builder functions to define the texture bounds and location on the EFB target.
 *
 * @param texObj	A pointer to the glyph's initialized texture object.
 * @param texWidth	The pixel width of the texture object.
 * @param texHeight	The pixel height of the texture object.
 * @param screenX	The screen X coordinate at which to output the rendered texture.
 * @param screenY	The screen Y coordinate at which to output the rendered texture.
 * @param color	Color to apply to the texture.
 */
void FreeTypeGX::copyTextureToFramebuffer(GXTexObj *texObj, f32 texWidth, f32 texHeight, int16_t screenX, int16_t screenY, GXColor color)
{
	GX_LoadTexObj(texObj, GX_TEXMAP0);
	GX_InvalidateTexAll();

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	GX_Begin(GX_QUADS, this->vertexIndex, 4);
	GX_Position2s16(screenX, screenY);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(0.0f, 0.0f);

	GX_Position2s16(texWidth + screenX, screenY);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(1.0f, 0.0f);

	GX_Position2s16(texWidth + screenX, texHeight + screenY);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(1.0f, 1.0f);

	GX_Position2s16(screenX, texHeight + screenY);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(0.0f, 1.0f);
	GX_End();

	this->setDefaultMode();
}

/**
 * Creates a feature quad to the EFB.
 *
 * This function creates a simple quad for displaying underline or strikeout text styling.
 *
 * @param featureWidth	The pixel width of the quad.
 * @param featureHeight	The pixel height of the quad.
 * @param screenX	The screen X coordinate at which to output the quad.
 * @param screenY	The screen Y coordinate at which to output the quad.
 * @param color	Color to apply to the texture.
 */
void FreeTypeGX::copyFeatureToFramebuffer(f32 featureWidth, f32 featureHeight, int16_t screenX, int16_t screenY, GXColor color)
{
	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

	GX_Begin(GX_QUADS, this->vertexIndex, 4);
	GX_Position2s16(screenX, screenY);
	GX_Color4u8(color.r, color.g, color.b, color.a);

	GX_Position2s16(featureWidth + screenX, screenY);
	GX_Color4u8(color.r, color.g, color.b, color.a);

	GX_Position2s16(featureWidth + screenX, featureHeight + screenY);
	GX_Color4u8(color.r, color.g, color.b, color.a);

	GX_Position2s16(screenX, featureHeight + screenY);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_End();

	this->setDefaultMode();
}
