/* 
 * Metaphrasis is a static conversion class for transforming RGBA image
 * buffers into verious GX texture formats for Wii homebrew development.
 * Copyright (C) 2008 Armin Tamzarian
 * 
 * This file is part of Metaphrasis.
 * 
 * Metaphrasis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Metaphrasis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Metaphrasis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Metaphrasis.h"
#include "mem2.hpp"

/**
 * Default constructor for the Metaphrasis class.
 */

Metaphrasis::Metaphrasis() {
}

/**
 * Default destructor for the Metaphrasis class.
 */

Metaphrasis::~Metaphrasis() {
}

/**
 * Convert the specified RGBA data buffer into the I4 texture format
 * 
 * This routine converts the RGBA data buffer into the I4 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToI4(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = bufferWidth * bufferHeight >> 1;
	uint32_t* dataBufferI4 = (uint32_t *)MEM2_alloc(bufferSize);
	memset(dataBufferI4, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint8_t *dst = (uint8_t *)dataBufferI4;

	for(uint16_t y = 0; y < bufferHeight; y += 8) {
		for(uint16_t x = 0; x < bufferWidth; x += 8) {
			for(uint16_t rows = 0; rows < 8; rows++) {
				*dst++ = (src[((y + rows) * bufferWidth) + (x + 0)] & 0xf0) | ((src[((y + rows) * bufferWidth) + (x + 1)] & 0xf0) >> 4);
				*dst++ = (src[((y + rows) * bufferWidth) + (x + 2)] & 0xf0) | ((src[((y + rows) * bufferWidth) + (x + 3)] & 0xf0) >> 4);
				*dst++ = (src[((y + rows) * bufferWidth) + (x + 4)] & 0xf0) | ((src[((y + rows) * bufferWidth) + (x + 5)] & 0xf0) >> 4);
				*dst++ = (src[((y + rows) * bufferWidth) + (x + 6)] & 0xf0) | ((src[((y + rows) * bufferWidth) + (x + 7)] & 0xf0) >> 4);
			}
		}
	}
	DCFlushRange(dataBufferI4, bufferSize);

	return dataBufferI4;
}

/**
 * Convert the specified RGBA data buffer into the I8 texture format
 * 
 * This routine converts the RGBA data buffer into the I8 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToI8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = bufferWidth * bufferHeight;
	uint32_t* dataBufferI8 = (uint32_t *)MEM2_alloc(bufferSize);
	memset(dataBufferI8, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint8_t *dst = (uint8_t *)dataBufferI8;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 8) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = src[((y + rows) * bufferWidth) + (x + 0)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 1)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 2)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 3)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 4)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 5)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 6)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 7)] & 0xff;
			}
		}
	}
	DCFlushRange(dataBufferI8, bufferSize);

	return dataBufferI8;
}

/**
 * Downsample the specified RGBA value data buffer to an IA4 value.
 *
 * This routine downsamples the given RGBA data value into the IA4 texture data format.
 *
 * <strong>Format Explanation</strong>
 * \n
 * IA4 is a greyscale color format with a 4 bit Alpha component. In order to convert from the given RGBA color format simply concatenate
 * the first 4 MSB from any color component (in this case the Blue component to decrease the number of place shifts performed) to
 * the first 4 MSB from the Alpha component.
 * \n\n
 * <code>
 * RGBA (32bit):  r7r6r5r4r3r2r1r0|g7g6g5g4g3g2g1g0|b7b6b5b4b3b2b1b|0a7a6a5a4a3a2a1a0
 * \n
 * RGBIA4 (8bit): b7b6b5b4a7a6a5a4
 * </code>
 *
 * @param rgba	A 32-bit RGBA value to convert to the IA4 format.
 * @return The IA4 value of the given RGBA value.
 */

uint8_t Metaphrasis::convertRGBAToIA4(uint32_t rgba) {
	return RGBA_TO_IA4(rgba);
}

/**
 * Convert the specified RGBA data buffer into the IA4 texture format
 * 
 * This routine converts the RGBA data buffer into the IA4 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToIA4(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = bufferWidth * bufferHeight;
	uint32_t* dataBufferIA4 = (uint32_t *)MEM2_alloc(bufferSize);
	memset(dataBufferIA4, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint8_t *dst = (uint8_t *)dataBufferIA4;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 8) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = RGBA_TO_IA4(src[((y + rows) * bufferWidth) + (x + 0)]);
				*dst++ = RGBA_TO_IA4(src[((y + rows) * bufferWidth) + (x + 1)]);
				*dst++ = RGBA_TO_IA4(src[((y + rows) * bufferWidth) + (x + 2)]);
				*dst++ = RGBA_TO_IA4(src[((y + rows) * bufferWidth) + (x + 3)]);
				*dst++ = RGBA_TO_IA4(src[((y + rows) * bufferWidth) + (x + 4)]);
				*dst++ = RGBA_TO_IA4(src[((y + rows) * bufferWidth) + (x + 5)]);
				*dst++ = RGBA_TO_IA4(src[((y + rows) * bufferWidth) + (x + 6)]);
				*dst++ = RGBA_TO_IA4(src[((y + rows) * bufferWidth) + (x + 7)]);
			}
		}
	}
	DCFlushRange(dataBufferIA4, bufferSize);

	return dataBufferIA4;
}

/**
 * Downsample the specified RGBA value data buffer to an IA8 value.
 * 
 * This routine downsamples the given RGBA data value into the IA8 texture data format.
 * 
 * <strong>Format Explanation</strong>
 * \n
 * IA8 is a greyscale color format with a full 8 bit Alpha component. In order to convert from the given RGBA color format simply concatenate
 * the entire 8 bit information from any color component (in this case the Blue component to decrease the number of place shifts performed) to
 * the entire 8 bit Alpha component.
 * \n\n
 * <code>
 * RGBA (32bit):   r7r6r5r4r3r2r1r0|g7g6g5g4g3g2g1g0|b7b6b5b4b3b2b1b|0a7a6a5a4a3a2a1a0
 * \n
 * RGBIA8 (16bit): b7b6b5b4b3b2b1b0|a7a6a5a4a3a2a1a0
 * </code>
 *
 * @param rgba	A 32-bit RGBA value to convert to the IA8 format.
 * @return The IA8 value of the given RGBA value.
 */

uint16_t Metaphrasis::convertRGBAToIA8(uint32_t rgba) {

	return RGBA_TO_IA8(rgba);
}

/**
 * Convert the specified RGBA data buffer into the IA8 texture format
 * 
 * This routine converts the RGBA data buffer into the IA8 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToIA8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = (bufferWidth * bufferHeight) << 1;
	uint32_t* dataBufferIA8 = (uint32_t *)MEM2_alloc(bufferSize);
	memset(dataBufferIA8, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint16_t *dst = (uint16_t *)dataBufferIA8;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 4) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = RGBA_TO_IA8(src[((y + rows) * bufferWidth) + (x + 0)]);
				*dst++ = RGBA_TO_IA8(src[((y + rows) * bufferWidth) + (x + 1)]);
				*dst++ = RGBA_TO_IA8(src[((y + rows) * bufferWidth) + (x + 2)]);
				*dst++ = RGBA_TO_IA8(src[((y + rows) * bufferWidth) + (x + 3)]);
			}
		}
	}
	DCFlushRange(dataBufferIA8, bufferSize);

	return dataBufferIA8;
}

/**
 * Convert the specified RGBA data buffer into the RGBA8 texture format
 * 
 * This routine converts the RGBA data buffer into the RGBA8 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToRGBA8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = (bufferWidth * bufferHeight) << 2;
	uint32_t* dataBufferRGBA8 = (uint32_t *)MEM2_alloc(bufferSize);
	memset(dataBufferRGBA8, 0x00, bufferSize);

	uint8_t *src = (uint8_t *)rgbaBuffer;
	uint8_t *dst = (uint8_t *)dataBufferRGBA8;

	for(uint16_t block = 0; block < bufferHeight; block += 4) {
		for(uint16_t i = 0; i < bufferWidth; i += 4) {
            for (uint32_t c = 0; c < 4; c++) {
				uint32_t blockWid = (((block + c) * bufferWidth) + i) << 2 ;

				*dst++ = src[blockWid + 3];  // ar = 0
				*dst++ = src[blockWid + 0];
				*dst++ = src[blockWid + 7];  // ar = 1
				*dst++ = src[blockWid + 4];
				*dst++ = src[blockWid + 11]; // ar = 2
				*dst++ = src[blockWid + 8];
				*dst++ = src[blockWid + 15]; // ar = 3
				*dst++ = src[blockWid + 12];
            }
            for (uint32_t c = 0; c < 4; c++) {
				uint32_t blockWid = (((block + c) * bufferWidth) + i ) << 2 ;

				*dst++ = src[blockWid + 1];  // gb = 0
				*dst++ = src[blockWid + 2];
				*dst++ = src[blockWid + 5];  // gb = 1
				*dst++ = src[blockWid + 6];
				*dst++ = src[blockWid + 9];  // gb = 2
				*dst++ = src[blockWid + 10];
				*dst++ = src[blockWid + 13]; // gb = 3
				*dst++ = src[blockWid + 14];
            }
		}
	}
	DCFlushRange(dataBufferRGBA8, bufferSize);

	return dataBufferRGBA8;
}

/**
 * Downsample the specified RGBA value data buffer to an RGB565 value.
 * 
 * This routine downsamples the given RGBA data value into the RGB565 texture data format.
 * Attribution for this routine is given fully to NoNameNo of GRRLIB Wii library.
 * 
 * <strong>Format Explanation</strong>
 * \n
 * RGB565 is a color format without an Alpha component. In order to convert from the given RGBA color format simply concatenate
 * the first 5 MSB from the Red component to the first 6 MSB from the Green component to the first 5 MSB from the Blue component.
 * \n\n
 * <code>
 * RGBA (32bit):   r7r6r5r4r3r2r1r0|g7g6g5g4g3g2g1g0|b7b6b5b4b3b2b1b|0a7a6a5a4a3a2a1a0
 * \n
 * RGB565 (16bit): r7r6r5r4r3g7g6g5|g4g3g2b7b6b5b4b3
 * </code>
 *
 * @param rgba	A 32-bit RGBA value to convert to the RGB565 format.
 * @return The RGB565 value of the given RGBA value.
 */

uint16_t Metaphrasis::convertRGBAToRGB565(uint32_t rgba) {
	return RGBA_TO_RGB565(rgba);
}

/**
 * Convert the specified RGBA data buffer into the RGB565 texture format
 * 
 * This routine converts the RGBA data buffer into the RGB565 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToRGB565(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = (bufferWidth * bufferHeight) << 1;
	uint32_t* dataBufferRGB565 = (uint32_t *)MEM2_alloc(bufferSize);
	memset(dataBufferRGB565, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint16_t *dst = (uint16_t *)dataBufferRGB565;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 4) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = RGBA_TO_RGB565(src[((y + rows) * bufferWidth) + (x + 0)]);
				*dst++ = RGBA_TO_RGB565(src[((y + rows) * bufferWidth) + (x + 1)]);
				*dst++ = RGBA_TO_RGB565(src[((y + rows) * bufferWidth) + (x + 2)]);
				*dst++ = RGBA_TO_RGB565(src[((y + rows) * bufferWidth) + (x + 3)]);
			}
		}
	}
	DCFlushRange(dataBufferRGB565, bufferSize);

	return dataBufferRGB565;
}

/**
 * Downsample the specified RGBA value data buffer to an RGB5A3 value.
 * 
 * This routine downsamples the given RGBA data value into the RGB5A3 texture data format.
 * 
 * <strong>Format Explanation</strong>
 * \n
 * RGB5A3 is really a conditional application of the RGB444 and RGB555 formats based off of the value of the Alpha channel.
 * If the 3 MSB of the Alpha channel is fully opaque (a7a6a5 == [1][1][1]) then the RGBA color is converted to RGB555 with the first bit set to 1.
 * Otherwise the RGBA color is converted to RGB444 with a 3 bit alpha channel and the first bit set to 0.
 *\n\n
 * <code>
 * RGBA (32bit):    r7r6r5r4r3r2r1r0|g7g6g5g4g3g2g1g0|b7b6b5b4b3b2b1b|0a7a6a5a4a3a2a1a0
 * \n
 * RGB555 (16bit): [1]r7r6r5r4r3g7g6|g5g4g3b7b6b5b4b3
 * \n
 * RGB444 (16bit): [0]r7r6r5r4g7g6g5|g4b7b6b5b4a7a6a5
 * </code>
 *
 * @param rgba	A 32-bit RGBA value to convert to the RGB5A3 format.
 * @return The RGB5A3 value of the given RGBA value.
 */

uint16_t Metaphrasis::convertRGBAToRGB5A3(uint32_t rgba) {
	return RGBA_TO_RGB5A3(rgba);
}
	
/**
 * Convert the specified RGBA data buffer into the RGB5A3 texture format
 * 
 * This routine converts the RGBA data buffer into the RGB5A3 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToRGB5A3(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = (bufferWidth * bufferHeight) << 1;
	uint32_t* dataBufferRGB5A3 = (uint32_t *)MEM2_alloc(bufferSize);
	memset(dataBufferRGB5A3, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint16_t *dst = (uint16_t *)dataBufferRGB5A3;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 4) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = RGBA_TO_RGB5A3(src[((y + rows) * bufferWidth) + (x + 0)]);
				*dst++ = RGBA_TO_RGB5A3(src[((y + rows) * bufferWidth) + (x + 1)]);
				*dst++ = RGBA_TO_RGB5A3(src[((y + rows) * bufferWidth) + (x + 2)]);
				*dst++ = RGBA_TO_RGB5A3(src[((y + rows) * bufferWidth) + (x + 3)]);
			}
		}
	}
	DCFlushRange(dataBufferRGB5A3, bufferSize);

	return dataBufferRGB5A3;
}
