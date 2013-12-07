/********************************************************************************************

PNGU Version : 0.2a

Coder : frontier

More info : http://frontier-dev.net

********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <png.h>

#include "pngu.h"
#include "gecko/gecko.hpp"
#include "loader/utils.h"
#include "memory/mem2.hpp"

// Constants
#define PNGU_SOURCE_BUFFER			1
#define PNGU_SOURCE_DEVICE			2

// Prototypes of helper functions
int pngu_info (IMGCTX ctx);
int pngu_decode (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, PNGU_u32 stripAlpha, int force32bits);
void pngu_free_info (IMGCTX ctx);
void pngu_read_data_from_buffer (png_structp png_ptr, png_bytep data, png_size_t length);
void pngu_write_data_to_buffer (png_structp png_ptr, png_bytep data, png_size_t length);
int pngu_clamp (int value, int min, int max);

// PNGU Image context struct
struct _IMGCTX
{
	int source;
	void *buffer;
	char *filename;
	PNGU_u32 cursor;
	PNGU_u32 buf_size; // buffer size

	PNGU_u32 propRead;
	PNGUPROP prop;

	PNGU_u32 infoRead;
	png_structp png_ptr;
	png_infop info_ptr;
	FILE *fd;
	
	png_bytep *row_pointers;
	png_bytep img_data;
};


// PNGU Implementation //

IMGCTX PNGU_SelectImageFromBuffer (const void *buffer)
{
	if (!buffer) return NULL;

	IMGCTX ctx = malloc(sizeof (struct _IMGCTX));
	if (!ctx) return NULL;

	ctx->buffer = (void *) buffer;
	ctx->source = PNGU_SOURCE_BUFFER;
	ctx->cursor = 0;
	ctx->filename = NULL;
	ctx->propRead = 0;
	ctx->infoRead = 0;
	ctx->buf_size = 0;

	return ctx;
}


IMGCTX PNGU_SelectImageFromDevice (const char *filename)
{
	if (!filename)
		return NULL;

	IMGCTX ctx = malloc(sizeof (struct _IMGCTX));
	if (ctx == NULL)
		return NULL;

	ctx->buffer = NULL;
	ctx->source = PNGU_SOURCE_DEVICE;
	ctx->cursor = 0;

	ctx->filename = malloc(strlen (filename) + 1);
	if (ctx->filename == NULL)
	{
		if(ctx != NULL)
			free(ctx);
		return NULL;
	}
	strcpy(ctx->filename, filename);

	ctx->propRead = 0;
	ctx->infoRead = 0;

	return ctx;
}


void PNGU_ReleaseImageContext (IMGCTX ctx)
{
	if(!ctx)
		return;

	if(ctx->filename)
		free(ctx->filename);

	if((ctx->propRead) && (ctx->prop.trans))
		free(ctx->prop.trans);

	pngu_free_info(ctx);

	free(ctx);
}


int PNGU_GetImageProperties (IMGCTX ctx, PNGUPROP *imgprop)
{
	if (!ctx->propRead)
	{
		int res = pngu_info (ctx);
		if (res != PNGU_OK)
			return res;
	}

	*imgprop = ctx->prop;

	return PNGU_OK;
}


int PNGU_DecodeToYCbYCr (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride)
{

	// width needs to be divisible by two
	if (width % 2) return PNGU_ODD_WIDTH;

	// stride needs to be divisible by two
	if (stride % 2) return PNGU_ODD_STRIDE;

	int result = pngu_decode (ctx, width, height, 1, 0);
	if (result != PNGU_OK)
		return result;

	PNGU_u32 x, y, buffWidth = (width + stride) / 2;
	// Copy image to the output buffer
	for (y = 0; y < height; y++)
		for (x = 0; x < (width / 2); x++)
			((PNGU_u32 *)buffer)[y*buffWidth+x] = PNGU_RGB8_TO_YCbYCr (*(ctx->row_pointers[y]+x*6), *(ctx->row_pointers[y]+x*6+1), *(ctx->row_pointers[y]+x*6+2),
															*(ctx->row_pointers[y]+x*6+3), *(ctx->row_pointers[y]+x*6+4), *(ctx->row_pointers[y]+x*6+5));
	
	// Free resources
	free(ctx->img_data);
	free(ctx->row_pointers);

	// Success
	return PNGU_OK;
}


int PNGU_DecodeToRGB565 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride)
{
	
	int result = pngu_decode (ctx, width, height, 1, 0);
	if (result != PNGU_OK) return result;

	PNGU_u32 x, y, buffWidth = width + stride;
	// Copy image to the output buffer
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			((PNGU_u16 *)buffer)[y*buffWidth+x] = 
				(((PNGU_u16) (ctx->row_pointers[y][x*3] & 0xF8)) << 8) | 
				(((PNGU_u16) (ctx->row_pointers[y][x*3+1] & 0xFC)) << 3) | 
				(((PNGU_u16) (ctx->row_pointers[y][x*3+2] & 0xF8)) >> 3);
	
	// Free resources
	free(ctx->img_data);
	free(ctx->row_pointers);

	// Success
	return PNGU_OK;
}


int PNGU_DecodeToRGBA8 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride, PNGU_u8 default_alpha)
{
	
	int result = pngu_decode (ctx, width, height, 0, 0);
	if (result != PNGU_OK)
		return result;

	PNGU_u32 x, y, buffWidth = width + stride;
	// Check is source image has an alpha channel
	if ( (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB_ALPHA) )
	{
		// Alpha channel present, copy image to the output buffer
		for (y = 0; y < height; y++)
			memcpy (buffer + (y * buffWidth * 4), ctx->row_pointers[y], width * 4);
	}
	else
	{
		// No alpha channel present, copy image to the output buffer
		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				((PNGU_u32 *)buffer)[y*buffWidth+x] = 
					(((PNGU_u32) ctx->row_pointers[y][x*3]) << 24) | 
					(((PNGU_u32) ctx->row_pointers[y][x*3+1]) << 16) | 
					(((PNGU_u32) ctx->row_pointers[y][x*3+2]) << 8) | 
					((PNGU_u32) default_alpha);
	}
	
	// Free resources
	free(ctx->img_data);
	free(ctx->row_pointers);

	// Success
	return PNGU_OK;
}


int PNGU_DecodeTo4x4RGB565 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer)
{

	// width and height need to be divisible by four
//	if ((width % 4) || (height % 4))
//		return PNGU_INVALID_WIDTH_OR_HEIGHT;

	int result = pngu_decode (ctx, width, height, 1, 0);
	if (result != PNGU_OK) return result;

	PNGU_u32 x, y, qwidth = width / 4, qheight = height / 4;
	// Copy image to the output buffer
	for (y = 0; y < qheight; y++)
	{
		if (((y + 0xFF) & 0xFF) == 0)
			usleep(100);
		for (x = 0; x < qwidth; x++)
		{
			int blockbase = (y * qwidth + x) * 4;

			PNGU_u64 field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4]+x*12));
			PNGU_u64 field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4]+x*12+8));
			((PNGU_u64 *) buffer)[blockbase] = 
				(((field64 & 0xF800000000000000ULL) | ((field64 & 0xFC000000000000ULL) << 3) | ((field64 & 0xF80000000000ULL) << 5)) | 
				(((field64 & 0xF800000000ULL) << 8) | ((field64 & 0xFC000000ULL) << 11) | ((field64 & 0xF80000ULL) << 13)) | 
				(((field64 & 0xF800ULL) << 16) | ((field64 & 0xFCULL) << 19) | ((field32 & 0xF8000000ULL) >> 11)) |
				(((field32 & 0xF80000ULL) >> 8) | ((field32 & 0xFC00ULL) >> 5) | ((field32 & 0xF8ULL) >> 3)));

			field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+1]+x*12));
			field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+1]+x*12+8));
			((PNGU_u64 *) buffer)[blockbase+1] = 
				(((field64 & 0xF800000000000000ULL) | ((field64 & 0xFC000000000000ULL) << 3) | ((field64 & 0xF80000000000ULL) << 5)) | 
				(((field64 & 0xF800000000ULL) << 8) | ((field64 & 0xFC000000ULL) << 11) | ((field64 & 0xF80000ULL) << 13)) | 
				(((field64 & 0xF800ULL) << 16) | ((field64 & 0xFCULL) << 19) | ((field32 & 0xF8000000ULL) >> 11)) |
				(((field32 & 0xF80000ULL) >> 8) | ((field32 & 0xFC00ULL) >> 5) | ((field32 & 0xF8ULL) >> 3)));

			field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+2]+x*12));
			field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+2]+x*12+8));
			((PNGU_u64 *) buffer)[blockbase+2] = 
				(((field64 & 0xF800000000000000ULL) | ((field64 & 0xFC000000000000ULL) << 3) | ((field64 & 0xF80000000000ULL) << 5)) | 
				(((field64 & 0xF800000000ULL) << 8) | ((field64 & 0xFC000000ULL) << 11) | ((field64 & 0xF80000ULL) << 13)) | 
				(((field64 & 0xF800ULL) << 16) | ((field64 & 0xFCULL) << 19) | ((field32 & 0xF8000000ULL) >> 11)) |
				(((field32 & 0xF80000ULL) >> 8) | ((field32 & 0xFC00ULL) >> 5) | ((field32 & 0xF8ULL) >> 3)));

			field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+3]+x*12));
			field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+3]+x*12+8));
			((PNGU_u64 *) buffer)[blockbase+3] = 
				(((field64 & 0xF800000000000000ULL) | ((field64 & 0xFC000000000000ULL) << 3) | ((field64 & 0xF80000000000ULL) << 5)) | 
				(((field64 & 0xF800000000ULL) << 8) | ((field64 & 0xFC000000ULL) << 11) | ((field64 & 0xF80000ULL) << 13)) | 
				(((field64 & 0xF800ULL) << 16) | ((field64 & 0xFCULL) << 19) | ((field32 & 0xF8000000ULL) >> 11)) |
				(((field32 & 0xF80000ULL) >> 8) | ((field32 & 0xFC00ULL) >> 5) | ((field32 & 0xF8ULL) >> 3)));
		}
	}

	// Free resources
	free(ctx->img_data);
	free(ctx->row_pointers);

	// Success
	return PNGU_OK;
}


int PNGU_DecodeTo4x4RGB5A3 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u8 default_alpha)
{

	// width and height need to be divisible by four
	if ((width % 4) || (height % 4)) return PNGU_INVALID_WIDTH_OR_HEIGHT;

	int result = pngu_decode (ctx, width, height, 0, 0);
	if (result != PNGU_OK) return result;

	// Init some vars
	PNGU_u32 x, y, qwidth = width / 4, qheight = height / 4;

	// Check if source image has an alpha channel
	if ( (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB_ALPHA) )
	{
		// Alpha channel present, copy image to the output buffer
		for (y = 0; y < qheight; y++)
			for (x = 0; x < qwidth; x++)
			{
				int blockbase = (y * qwidth + x) * 4;
				PNGU_u64 tmp;

				PNGU_u64 fieldA = *((PNGU_u64 *)(ctx->row_pointers[y*4]+x*16));
				PNGU_u64 fieldB = *((PNGU_u64 *)(ctx->row_pointers[y*4]+x*16+8));
				// If first pixel is opaque set MSB to 1 and encode colors in RGB555, else set MSB to 0 and encode colors in ARGB3444
				if ((fieldA & 0xE000000000ULL) == 0xE000000000ULL)			
					tmp = 0x8000000000000000ULL | ((fieldA & 0xF800000000000000ULL) >> 1) | ((fieldA & 0xF8000000000000ULL) << 2) | ((fieldA & 0xF80000000000ULL) << 5);
				else
					tmp = ((fieldA & 0xE000000000ULL) << 23) | ((fieldA & 0xF000000000000000ULL) >> 4) | (fieldA & 0xF0000000000000ULL) | ((fieldA & 0xF00000000000ULL) << 4);

				// If second pixel is opaque set MSB to 1 and encode colors in RGB555, else set MSB to 0 and encode colors in ARGB3444
				if ((fieldA & 0xE0ULL) == 0xE0ULL)
					tmp = tmp | 0x800000000000ULL | ((fieldA & 0xF8000000ULL) << 15) | ((fieldA & 0xF80000ULL) << 18) | ((fieldA & 0xF800ULL) << 21);
				else
					tmp = tmp | ((fieldA & 0xE0ULL) << 39) | ((fieldA & 0xF0000000ULL) << 12) | ((fieldA & 0xF00000ULL) << 16) | ((fieldA & 0xF000ULL) << 20);

				// If third pixel is opaque set MSB to 1 and encode colors in RGB555, else set MSB to 0 and encode colors in ARGB3444
				if ((fieldB & 0xE000000000ULL) == 0xE000000000ULL)
					tmp = tmp | 0x80000000ULL | ((fieldB & 0xF800000000000000ULL) >> 33) | ((fieldB & 0xF8000000000000ULL) >> 30) | ((fieldB & 0xF80000000000ULL) >> 27);
				else
					tmp = tmp | ((fieldB & 0xE000000000ULL) >> 9) | ((fieldB & 0xF000000000000000ULL) >> 36) | ((fieldB & 0xF0000000000000ULL) >> 32) | ((fieldB & 0xF00000000000ULL) >> 28);

				// If fourth pixel is opaque set MSB to 1 and encode colors in RGB555, else set MSB to 0 and encode colors in ARGB3444
				if ((fieldB & 0xE0ULL) == 0xE0ULL)
					tmp = tmp | 0x8000ULL | ((fieldB & 0xF8000000ULL) >> 17) | ((fieldB & 0xF80000ULL) >> 14) | ((fieldB & 0xF800ULL) >> 11);
				else
					tmp = tmp | ((fieldB & 0xE0ULL) << 7) | ((fieldB & 0xF0000000ULL) >> 20) | ((fieldB & 0xF00000ULL) >> 16) | ((fieldB & 0xF000ULL) >> 12);
				((PNGU_u64 *) buffer)[blockbase] = tmp;

				fieldA = *((PNGU_u64 *)(ctx->row_pointers[y*4+1]+x*16));
				fieldB = *((PNGU_u64 *)(ctx->row_pointers[y*4+1]+x*16+8));
				if ((fieldA & 0xE000000000ULL) == 0xE000000000ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = 0x8000000000000000ULL | ((fieldA & 0xF800000000000000ULL) >> 1) | ((fieldA & 0xF8000000000000ULL) << 2) | ((fieldA & 0xF80000000000ULL) << 5);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = ((fieldA & 0xE000000000ULL) << 23) | ((fieldA & 0xF000000000000000ULL) >> 4) | (fieldA & 0xF0000000000000ULL) | ((fieldA & 0xF00000000000ULL) << 4);

				if ((fieldA & 0xE0ULL) == 0xE0ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x800000000000ULL | ((fieldA & 0xF8000000ULL) << 15) | ((fieldA & 0xF80000ULL) << 18) | ((fieldA & 0xF800ULL) << 21);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldA & 0xE0ULL) << 39) | ((fieldA & 0xF0000000ULL) << 12) | ((fieldA & 0xF00000ULL) << 16) | ((fieldA & 0xF000ULL) << 20);

				if ((fieldB & 0xE000000000ULL) == 0xE000000000ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x80000000ULL | ((fieldB & 0xF800000000000000ULL) >> 33) | ((fieldB & 0xF8000000000000ULL) >> 30) | ((fieldB & 0xF80000000000ULL) >> 27);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldB & 0xE000000000ULL) >> 9) | ((fieldB & 0xF000000000000000ULL) >> 36) | ((fieldB & 0xF0000000000000ULL) >> 32) | ((fieldB & 0xF00000000000ULL) >> 28);

				if ((fieldB & 0xE0ULL) == 0xE0ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x8000ULL | ((fieldB & 0xF8000000ULL) >> 17) | ((fieldB & 0xF80000ULL) >> 14) | ((fieldB & 0xF800ULL) >> 11);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldB & 0xE0ULL) << 7) | ((fieldB & 0xF0000000ULL) >> 20) | ((fieldB & 0xF00000ULL) >> 16) | ((fieldB & 0xF000ULL) >> 12);
				((PNGU_u64 *) buffer)[blockbase+1] = tmp;

				fieldA = *((PNGU_u64 *)(ctx->row_pointers[y*4+2]+x*16));
				fieldB = *((PNGU_u64 *)(ctx->row_pointers[y*4+2]+x*16+8));
				if ((fieldA & 0xE000000000ULL) == 0xE000000000ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = 0x8000000000000000ULL | ((fieldA & 0xF800000000000000ULL) >> 1) | ((fieldA & 0xF8000000000000ULL) << 2) | ((fieldA & 0xF80000000000ULL) << 5);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = ((fieldA & 0xE000000000ULL) << 23) | ((fieldA & 0xF000000000000000ULL) >> 4) | (fieldA & 0xF0000000000000ULL) | ((fieldA & 0xF00000000000ULL) << 4);

				if ((fieldA & 0xE0ULL) == 0xE0ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x800000000000ULL | ((fieldA & 0xF8000000ULL) << 15) | ((fieldA & 0xF80000ULL) << 18) | ((fieldA & 0xF800ULL) << 21);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldA & 0xE0ULL) << 39) | ((fieldA & 0xF0000000ULL) << 12) | ((fieldA & 0xF00000ULL) << 16) | ((fieldA & 0xF000ULL) << 20);

				if ((fieldB & 0xE000000000ULL) == 0xE000000000ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x80000000ULL | ((fieldB & 0xF800000000000000ULL) >> 33) | ((fieldB & 0xF8000000000000ULL) >> 30) | ((fieldB & 0xF80000000000ULL) >> 27);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldB & 0xE000000000ULL) >> 9) | ((fieldB & 0xF000000000000000ULL) >> 36) | ((fieldB & 0xF0000000000000ULL) >> 32) | ((fieldB & 0xF00000000000ULL) >> 28);

				if ((fieldB & 0xE0ULL) == 0xE0ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x8000ULL | ((fieldB & 0xF8000000ULL) >> 17) | ((fieldB & 0xF80000ULL) >> 14) | ((fieldB & 0xF800ULL) >> 11);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldB & 0xE0ULL) << 7) | ((fieldB & 0xF0000000ULL) >> 20) | ((fieldB & 0xF00000ULL) >> 16) | ((fieldB & 0xF000ULL) >> 12);
				((PNGU_u64 *) buffer)[blockbase+2] = tmp;

				fieldA = *((PNGU_u64 *)(ctx->row_pointers[y*4+3]+x*16));
				fieldB = *((PNGU_u64 *)(ctx->row_pointers[y*4+3]+x*16+8));
				if ((fieldA & 0xE000000000ULL) == 0xE000000000ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = 0x8000000000000000ULL | ((fieldA & 0xF800000000000000ULL) >> 1) | ((fieldA & 0xF8000000000000ULL) << 2) | ((fieldA & 0xF80000000000ULL) << 5);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = ((fieldA & 0xE000000000ULL) << 23) | ((fieldA & 0xF000000000000000ULL) >> 4) | (fieldA & 0xF0000000000000ULL) | ((fieldA & 0xF00000000000ULL) << 4);

				if ((fieldA & 0xE0ULL) == 0xE0ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x800000000000ULL | ((fieldA & 0xF8000000ULL) << 15) | ((fieldA & 0xF80000ULL) << 18) | ((fieldA & 0xF800ULL) << 21);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldA & 0xE0ULL) << 39) | ((fieldA & 0xF0000000ULL) << 12) | ((fieldA & 0xF00000ULL) << 16) | ((fieldA & 0xF000ULL) << 20);

				if ((fieldB & 0xE000000000ULL) == 0xE000000000ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x80000000ULL | ((fieldB & 0xF800000000000000ULL) >> 33) | ((fieldB & 0xF8000000000000ULL) >> 30) | ((fieldB & 0xF80000000000ULL) >> 27);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldB & 0xE000000000ULL) >> 9) | ((fieldB & 0xF000000000000000ULL) >> 36) | ((fieldB & 0xF0000000000000ULL) >> 32) | ((fieldB & 0xF00000000000ULL) >> 28);

				if ((fieldB & 0xE0ULL) == 0xE0ULL)
					// Opaque pixel, so set MSB to 1 and encode colors in RGB555
					tmp = tmp | 0x8000ULL | ((fieldB & 0xF8000000ULL) >> 17) | ((fieldB & 0xF80000ULL) >> 14) | ((fieldB & 0xF800ULL) >> 11);
				else
					// Tranlucid pixel, so set MSB to 0 and encode colors in ARGB3444
					tmp = tmp | ((fieldB & 0xE0ULL) << 7) | ((fieldB & 0xF0000000ULL) >> 20) | ((fieldB & 0xF00000ULL) >> 16) | ((fieldB & 0xF000ULL) >> 12);
				((PNGU_u64 *) buffer)[blockbase+3] = tmp;
			}
	}
	else
	{
		// No alpha channel present, copy image to the output buffer
		PNGU_u64 alphaMask;
		default_alpha = (default_alpha >> 5);
		if (default_alpha == 7)
		{
			// The user wants an opaque texture, so set MSB to 1 and encode colors in RGB555
			alphaMask = 0x8000800080008000ULL;

			for (y = 0; y < qheight; y++)
				for (x = 0; x < qwidth; x++)
				{
					int blockbase = (y * qwidth + x) * 4;

					PNGU_u64 field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4]+x*12));
					PNGU_u64 field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4]+x*12+8));
					((PNGU_u64 *) buffer)[blockbase] = 
						 alphaMask | ((field64 & 0xF800000000000000ULL) >> 1) | ((field64 & 0xF8000000000000ULL) << 2) |
						((field64 & 0xF80000000000ULL) << 5) | ((field64 & 0xF800000000ULL) << 7) | ((field64 & 0xF8000000ULL) << 10) |
						((field64 & 0xF80000ULL) << 13) | ((field64 & 0xF800ULL) << 15) | ((field64 & 0xF8ULL) << 18) | 
						((field32 & 0xF8000000ULL) >> 11) |	((field32 & 0xF80000ULL) >> 9) | ((field32 & 0xF800ULL) >> 6) | ((field32 & 0xF8ULL) >> 3);

					field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+1]+x*12));
					field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+1]+x*12+8));
					((PNGU_u64 *) buffer)[blockbase+1] = 
						alphaMask | ((field64 & 0xF800000000000000ULL) >> 1) | ((field64 & 0xF8000000000000ULL) << 2) |
						((field64 & 0xF80000000000ULL) << 5) | ((field64 & 0xF800000000ULL) << 7) | ((field64 & 0xF8000000ULL) << 10) |
						((field64 & 0xF80000ULL) << 13) | ((field64 & 0xF800ULL) << 15) | ((field64 & 0xF8ULL) << 18) | 
						((field32 & 0xF8000000ULL) >> 11) |	((field32 & 0xF80000ULL) >> 9) | ((field32 & 0xF800ULL) >> 6) | ((field32 & 0xF8ULL) >> 3);

					field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+2]+x*12));
					field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+2]+x*12+8));
					((PNGU_u64 *) buffer)[blockbase+2] = 
						alphaMask | ((field64 & 0xF800000000000000ULL) >> 1) | ((field64 & 0xF8000000000000ULL) << 2) |
						((field64 & 0xF80000000000ULL) << 5) | ((field64 & 0xF800000000ULL) << 7) | ((field64 & 0xF8000000ULL) << 10) |
						((field64 & 0xF80000ULL) << 13) | ((field64 & 0xF800ULL) << 15) | ((field64 & 0xF8ULL) << 18) | 
						((field32 & 0xF8000000ULL) >> 11) |	((field32 & 0xF80000ULL) >> 9) | ((field32 & 0xF800ULL) >> 6) | ((field32 & 0xF8ULL) >> 3);

					field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+3]+x*12));
					field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+3]+x*12+8));
					((PNGU_u64 *) buffer)[blockbase+3] = 
						alphaMask | ((field64 & 0xF800000000000000ULL) >> 1) | ((field64 & 0xF8000000000000ULL) << 2) |
						((field64 & 0xF80000000000ULL) << 5) | ((field64 & 0xF800000000ULL) << 7) | ((field64 & 0xF8000000ULL) << 10) |
						((field64 & 0xF80000ULL) << 13) | ((field64 & 0xF800ULL) << 15) | ((field64 & 0xF8ULL) << 18) | 
						((field32 & 0xF8000000ULL) >> 11) |	((field32 & 0xF80000ULL) >> 9) | ((field32 & 0xF800ULL) >> 6) | ((field32 & 0xF8ULL) >> 3);
				}
		}
		else
		{
			// The user wants a translucid texture, so set MSB to 0 and encode colors in ARGB3444
			default_alpha = (default_alpha << 4);
			alphaMask = (((PNGU_u64) default_alpha) << 56) | (((PNGU_u64) default_alpha) << 40) |
						(((PNGU_u64) default_alpha) << 24) | (((PNGU_u64) default_alpha) << 8);

			for (y = 0; y < qheight; y++)
				for (x = 0; x < qwidth; x++)
				{
					int blockbase = (y * qwidth + x) * 4;

					PNGU_u64 field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4]+x*12));
					PNGU_u64 field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4]+x*12+8));
					((PNGU_u64 *) buffer)[blockbase] = 
						alphaMask | ((field64 & 0xF000000000000000ULL) >> 4) | (field64 & 0xF0000000000000ULL) | ((field64 & 0xF00000000000ULL) << 4) | 
						((field64 & 0xF000000000ULL) << 4) | ((field64 & 0xF0000000ULL) << 8) | ((field64 & 0xF00000ULL) << 12) | 
						((field64 & 0xF000ULL) << 12) | ((field64 & 0xF0ULL) << 16) | ((field32 & 0xF0000000ULL) >> 12) |	
						((field32 & 0xF00000ULL) >> 12) | ((field32 & 0xF000ULL) >> 8) | ((field32 & 0xF0ULL) >> 4);

					field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+1]+x*12));
					field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+1]+x*12+8));
					((PNGU_u64 *) buffer)[blockbase+1] = 
						alphaMask | ((field64 & 0xF000000000000000ULL) >> 4) | (field64 & 0xF0000000000000ULL) | ((field64 & 0xF00000000000ULL) << 4) | 
						((field64 & 0xF000000000ULL) << 4) | ((field64 & 0xF0000000ULL) << 8) | ((field64 & 0xF00000ULL) << 12) | 
						((field64 & 0xF000ULL) << 12) | ((field64 & 0xF0ULL) << 16) | ((field32 & 0xF0000000ULL) >> 12) |	
						((field32 & 0xF00000ULL) >> 12) | ((field32 & 0xF000ULL) >> 8) | ((field32 & 0xF0ULL) >> 4);

					field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+2]+x*12));
					field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+2]+x*12+8));
					((PNGU_u64 *) buffer)[blockbase+2] = 
						alphaMask | ((field64 & 0xF000000000000000ULL) >> 4) | (field64 & 0xF0000000000000ULL) | ((field64 & 0xF00000000000ULL) << 4) | 
						((field64 & 0xF000000000ULL) << 4) | ((field64 & 0xF0000000ULL) << 8) | ((field64 & 0xF00000ULL) << 12) | 
						((field64 & 0xF000ULL) << 12) | ((field64 & 0xF0ULL) << 16) | ((field32 & 0xF0000000ULL) >> 12) |	
						((field32 & 0xF00000ULL) >> 12) | ((field32 & 0xF000ULL) >> 8) | ((field32 & 0xF0ULL) >> 4);

					field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+3]+x*12));
					field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+3]+x*12+8));
					((PNGU_u64 *) buffer)[blockbase+3] = 
						alphaMask | ((field64 & 0xF000000000000000ULL) >> 4) | (field64 & 0xF0000000000000ULL) | ((field64 & 0xF00000000000ULL) << 4) | 
						((field64 & 0xF000000000ULL) << 4) | ((field64 & 0xF0000000ULL) << 8) | ((field64 & 0xF00000ULL) << 12) | 
						((field64 & 0xF000ULL) << 12) | ((field64 & 0xF0ULL) << 16) | ((field32 & 0xF0000000ULL) >> 12) |	
						((field32 & 0xF00000ULL) >> 12) | ((field32 & 0xF000ULL) >> 8) | ((field32 & 0xF0ULL) >> 4);
				}
		}
	}
	
	// Free resources
	free(ctx->img_data);
	free(ctx->row_pointers);

	// Success
	return PNGU_OK;
}


int PNGU_DecodeTo4x4RGBA8 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u8 default_alpha)
{
	// width and height need to be divisible by four
//	if ((width % 4) || (height % 4))
//		return PNGU_INVALID_WIDTH_OR_HEIGHT;

	int result = pngu_decode (ctx, width, height, 0, 0);
	if (result != PNGU_OK) return result;

	// Init some variables
	PNGU_u32 x, y, qwidth = width / 4, qheight = height / 4;

	// Check is source image has an alpha channel
	if ( (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB_ALPHA) )
	{
		// Alpha channel present, copy image to the output buffer
		for (y = 0; y < qheight; y++)
			for (x = 0; x < qwidth; x++)
			{
				int blockbase = (y * qwidth + x) * 8;

				PNGU_u64 fieldA = *((PNGU_u64 *)(ctx->row_pointers[y*4]+x*16));
				PNGU_u64 fieldB = *((PNGU_u64 *)(ctx->row_pointers[y*4]+x*16+8));
				((PNGU_u64 *) buffer)[blockbase] = 
					((fieldA & 0xFF00000000ULL) << 24) | ((fieldA & 0xFF00000000000000ULL) >> 8) | 
					((fieldA & 0xFFULL) << 40) | ((fieldA & 0xFF000000ULL) << 8) | 
					((fieldB & 0xFF00000000ULL) >> 8) | ((fieldB & 0xFF00000000000000ULL) >> 40) | 
					((fieldB & 0xFFULL) << 8) | ((fieldB & 0xFF000000ULL) >> 24);
				((PNGU_u64 *) buffer)[blockbase+4] =
					((fieldA & 0xFFFF0000000000ULL) << 8) | ((fieldA & 0xFFFF00ULL) << 24) |
					((fieldB & 0xFFFF0000000000ULL) >> 24) | ((fieldB & 0xFFFF00ULL) >> 8);

				fieldA = *((PNGU_u64 *)(ctx->row_pointers[y*4+1]+x*16));
				fieldB = *((PNGU_u64 *)(ctx->row_pointers[y*4+1]+x*16+8));
				((PNGU_u64 *) buffer)[blockbase+1] = 
					((fieldA & 0xFF00000000ULL) << 24) | ((fieldA & 0xFF00000000000000ULL) >> 8) | 
					((fieldA & 0xFFULL) << 40) | ((fieldA & 0xFF000000ULL) << 8) | 
					((fieldB & 0xFF00000000ULL) >> 8) | ((fieldB & 0xFF00000000000000ULL) >> 40) | 
					((fieldB & 0xFFULL) << 8) | ((fieldB & 0xFF000000ULL) >> 24);
				((PNGU_u64 *) buffer)[blockbase+5] =
					((fieldA & 0xFFFF0000000000ULL) << 8) | ((fieldA & 0xFFFF00ULL) << 24) |
					((fieldB & 0xFFFF0000000000ULL) >> 24) | ((fieldB & 0xFFFF00ULL) >> 8);

				fieldA = *((PNGU_u64 *)(ctx->row_pointers[y*4+2]+x*16));
				fieldB = *((PNGU_u64 *)(ctx->row_pointers[y*4+2]+x*16+8));
				((PNGU_u64 *) buffer)[blockbase+2] = 
					((fieldA & 0xFF00000000ULL) << 24) | ((fieldA & 0xFF00000000000000ULL) >> 8) | 
					((fieldA & 0xFFULL) << 40) | ((fieldA & 0xFF000000ULL) << 8) | 
					((fieldB & 0xFF00000000ULL) >> 8) | ((fieldB & 0xFF00000000000000ULL) >> 40) | 
					((fieldB & 0xFFULL) << 8) | ((fieldB & 0xFF000000ULL) >> 24);
				((PNGU_u64 *) buffer)[blockbase+6] =
					((fieldA & 0xFFFF0000000000ULL) << 8) | ((fieldA & 0xFFFF00ULL) << 24) |
					((fieldB & 0xFFFF0000000000ULL) >> 24) | ((fieldB & 0xFFFF00ULL) >> 8);

				fieldA = *((PNGU_u64 *)(ctx->row_pointers[y*4+3]+x*16));
				fieldB = *((PNGU_u64 *)(ctx->row_pointers[y*4+3]+x*16+8));
				((PNGU_u64 *) buffer)[blockbase+3] = 
					((fieldA & 0xFF00000000ULL) << 24) | ((fieldA & 0xFF00000000000000ULL) >> 8) | 
					((fieldA & 0xFFULL) << 40) | ((fieldA & 0xFF000000ULL) << 8) | 
					((fieldB & 0xFF00000000ULL) >> 8) | ((fieldB & 0xFF00000000000000ULL) >> 40) | 
					((fieldB & 0xFFULL) << 8) | ((fieldB & 0xFF000000ULL) >> 24);
				((PNGU_u64 *) buffer)[blockbase+7] =
					((fieldA & 0xFFFF0000000000ULL) << 8) | ((fieldA & 0xFFFF00ULL) << 24) |
					((fieldB & 0xFFFF0000000000ULL) >> 24) | ((fieldB & 0xFFFF00ULL) >> 8);
			}
	}
	else
	{
		// No alpha channel present, copy image to the output buffer
		PNGU_u64 alphaMask = (((PNGU_u64)default_alpha) << 56) | (((PNGU_u64)default_alpha) << 40) |
					(((PNGU_u64)default_alpha) << 24) | (((PNGU_u64)default_alpha) << 8);

		for (y = 0; y < qheight; y++)
			for (x = 0; x < qwidth; x++)
			{
				int blockbase = (y * qwidth + x) * 8;

				PNGU_u64 field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4]+x*12));
				PNGU_u64 field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4]+x*12+8));
				((PNGU_u64 *) buffer)[blockbase] = 
					(((field64 & 0xFF00000000000000ULL) >> 8) | (field64 & 0xFF00000000ULL) | 
					((field64 & 0xFF00ULL) << 8) | ((field32 & 0xFF0000ULL) >> 16) | alphaMask);
				((PNGU_u64 *) buffer)[blockbase+4] =
					(((field64 & 0xFFFF0000000000ULL) << 8) | ((field64 & 0xFFFF0000ULL) << 16) |
					((field64 & 0xFFULL) << 24) | ((field32 & 0xFF000000ULL) >> 8) | (field32 & 0xFFFFULL));

				field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+1]+x*12));
				field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+1]+x*12+8));
				((PNGU_u64 *) buffer)[blockbase+1] = 
					(((field64 & 0xFF00000000000000ULL) >> 8) | (field64 & 0xFF00000000ULL) | 
					((field64 & 0xFF00ULL) << 8) | ((field32 & 0xFF0000ULL) >> 16) | alphaMask);
				((PNGU_u64 *) buffer)[blockbase+5] =
					(((field64 & 0xFFFF0000000000ULL) << 8) | ((field64 & 0xFFFF0000ULL) << 16) |
					((field64 & 0xFFULL) << 24) | ((field32 & 0xFF000000ULL) >> 8) | (field32 & 0xFFFFULL));

				field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+2]+x*12));
				field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+2]+x*12+8));
				((PNGU_u64 *) buffer)[blockbase+2] = 
					(((field64 & 0xFF00000000000000ULL) >> 8) | (field64 & 0xFF00000000ULL) | 
					((field64 & 0xFF00ULL) << 8) | ((field32 & 0xFF0000ULL) >> 16) | alphaMask);
				((PNGU_u64 *) buffer)[blockbase+6] =
					(((field64 & 0xFFFF0000000000ULL) << 8) | ((field64 & 0xFFFF0000ULL) << 16) |
					((field64 & 0xFFULL) << 24) | ((field32 & 0xFF000000ULL) >> 8) | (field32 & 0xFFFFULL));

				field64 = *((PNGU_u64 *)(ctx->row_pointers[y*4+3]+x*12));
				field32 = (PNGU_u64) *((PNGU_u32 *)(ctx->row_pointers[y*4+3]+x*12+8));
				((PNGU_u64 *) buffer)[blockbase+3] = 
					(((field64 & 0xFF00000000000000ULL) >> 8) | (field64 & 0xFF00000000ULL) | 
					((field64 & 0xFF00ULL) << 8) | ((field32 & 0xFF0000ULL) >> 16) | alphaMask);
				((PNGU_u64 *) buffer)[blockbase+7] =
					(((field64 & 0xFFFF0000000000ULL) << 8) | ((field64 & 0xFFFF0000ULL) << 16) |
					((field64 & 0xFFULL) << 24) | ((field32 & 0xFF000000ULL) >> 8) | (field32 & 0xFFFFULL));
			}
	}
	
	// Free resources
	free(ctx->img_data);
	free(ctx->row_pointers);

	// Success
	return PNGU_OK;
}


static inline PNGU_u16 rgb8ToRGB565(PNGU_u8 *color)
{
	return ((color[0] >> 3) << 11) | ((color[1] >> 2) << 5) | (color[2] >> 3);
}


static int colorDistance(const PNGU_u8 *c0, const PNGU_u8 *c1)
{
	return (c1[0] - c0[0]) * (c1[0] - c0[0]) + (c1[1] - c0[1]) * (c1[1] - c0[1]) + (c1[2] - c0[2]) * (c1[2] - c0[2]);
}


static void getBaseColors(PNGU_u8 *color0, PNGU_u8 *color1, const PNGU_u8 *srcBlock)
{
	int i, j, maxDistance = -1;

	for (i = 0; i < 15; ++i)
		for (j = i + 1; j < 16; ++j)
		{
			int distance = colorDistance(srcBlock + i * 4, srcBlock + j * 4);
			if (distance > maxDistance)
			{
				maxDistance = distance;
				*(PNGU_u32 *)color0 = ((PNGU_u32 *)srcBlock)[i];
				*(PNGU_u32 *)color1 = ((PNGU_u32 *)srcBlock)[j];
			}
		}
	if (rgb8ToRGB565(color0) < rgb8ToRGB565(color1))
	{
		PNGU_u32 tmp = *(PNGU_u32 *)color0;
		*(PNGU_u32 *)color0 = *(PNGU_u32 *)color1;
		*(PNGU_u32 *)color1 = tmp;
	}
}


static PNGU_u32 colorIndices(const PNGU_u8 *color0, const PNGU_u8 *color1, const PNGU_u8 *srcBlock)
{
	PNGU_u16 colors[4][4];
	PNGU_u32 res = 0;
	int i;

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
	for (i = 15; i >= 0; --i)
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


int PNGU_DecodeToCMPR(IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer)
{
	PNGU_u8 srcBlock[16 * 4];
	PNGU_u8 color0[4];
	PNGU_u8 color1[4];
	PNGU_u8 *outBuf = (PNGU_u8 *)buffer;
	int ii, jj, k;

	int result = pngu_decode (ctx, width, height, 0, 1);
	if (result != PNGU_OK) return result;
	width = width & ~7u;
	height = height & ~7u;
	// Alpha channel present, copy image to the output buffer
	for (jj = 0; jj < (int)height; jj += 8)
		for (ii = 0; ii < (int)width; ii += 8)
			for (k = 0; k < 4; ++k)
			{
				int j = jj + ((k >> 1) << 2);
				int i = ii + ((k & 1) << 2);
				memcpy(srcBlock, ctx->row_pointers[j] + i * 4, 16);
				memcpy(srcBlock + 4 * 4, ctx->row_pointers[j + 1] + i * 4, 16);
				memcpy(srcBlock + 8 * 4, ctx->row_pointers[j + 2] + i * 4, 16);
				memcpy(srcBlock + 12 * 4, ctx->row_pointers[j + 3] + i * 4, 16);
				getBaseColors(color0, color1, srcBlock);
				*(PNGU_u16 *)outBuf = rgb8ToRGB565(color0);
				outBuf += 2;
				*(PNGU_u16 *)outBuf = rgb8ToRGB565(color1);
				outBuf += 2;
				*(PNGU_u32 *)outBuf = colorIndices(color0, color1, srcBlock);
				outBuf += 4;
			}
	// Free resources
	free(ctx->img_data);
	free(ctx->row_pointers);

	// Success
	return PNGU_OK;
}

void user_error(png_structp png_ptr, png_const_charp c)
{
	longjmp(png_ptr->jmpbuf, 1);
	gprintf("%s\n", c);
}

int PNGU_EncodeFromYCbYCr(IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride)
{
	// Erase from the context any readed info
	pngu_free_info (ctx);
	ctx->propRead = 0;

	// Check if the user has selected a file to write the image
	if (ctx->source == PNGU_SOURCE_BUFFER);	

	else if (ctx->source == PNGU_SOURCE_DEVICE)
	{
		// Open file
		if (!(ctx->fd = fopen (ctx->filename, "wb")))
			return PNGU_CANT_OPEN_FILE;
	}

	else
		return PNGU_NO_FILE_SELECTED;

	// Allocation of libpng structs
	ctx->png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!(ctx->png_ptr))
	{
		if (ctx->source == PNGU_SOURCE_DEVICE)
			fclose (ctx->fd);
		return PNGU_LIB_ERROR;
	}

	ctx->info_ptr = png_create_info_struct (ctx->png_ptr);
	if (!(ctx->info_ptr))
	{
		png_destroy_write_struct (&(ctx->png_ptr), (png_infopp)NULL);
		if (ctx->source == PNGU_SOURCE_DEVICE)
			fclose (ctx->fd);
		return PNGU_LIB_ERROR;
	}

	if (ctx->source == PNGU_SOURCE_BUFFER)
	{
		// Installation of our custom data writer function
		ctx->cursor = 0;
		png_set_write_fn (ctx->png_ptr, ctx, pngu_write_data_to_buffer, NULL);
	}
	else if (ctx->source == PNGU_SOURCE_DEVICE)
	{
		// Default data writer uses function fwrite, so it needs to use our FILE*
		png_init_io (ctx->png_ptr, ctx->fd);
	}

	// Setup output file properties
	png_set_IHDR (ctx->png_ptr, ctx->info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, 
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Allocate memory to store the image in RGB format
	png_uint_32 rowbytes = width * 3;
	if (rowbytes % 4)
		rowbytes = ((rowbytes / 4) + 1) * 4; // Add extra padding so each row starts in a 4 byte boundary

	ctx->img_data = malloc(rowbytes * height);
	if (!ctx->img_data)
	{
		png_destroy_write_struct (&(ctx->png_ptr), (png_infopp)NULL);
		if (ctx->source == PNGU_SOURCE_DEVICE)
			fclose (ctx->fd);
		return PNGU_LIB_ERROR;
	}

	ctx->row_pointers = malloc(sizeof (png_bytep) * height);
	if (!ctx->row_pointers)
	{
		png_destroy_write_struct (&(ctx->png_ptr), (png_infopp)NULL);
		if (ctx->source == PNGU_SOURCE_DEVICE)
			fclose (ctx->fd);
		return PNGU_LIB_ERROR;
	}

	// Encode YCbYCr image into RGB8 format
	PNGU_u32 x, y, buffWidth = (width + stride) / 2;
	for (y = 0; y < height; y++)
	{
		ctx->row_pointers[y] = ctx->img_data + (y * rowbytes);

		for (x = 0; x < (width / 2); x++)
			PNGU_YCbYCr_TO_RGB8 ( ((PNGU_u32 *)buffer)[y*buffWidth+x], 
				((PNGU_u8 *) ctx->row_pointers[y]+x*6), ((PNGU_u8 *) ctx->row_pointers[y]+x*6+1),
				((PNGU_u8 *) ctx->row_pointers[y]+x*6+2), ((PNGU_u8 *) ctx->row_pointers[y]+x*6+3),
				((PNGU_u8 *) ctx->row_pointers[y]+x*6+4), ((PNGU_u8 *) ctx->row_pointers[y]+x*6+5) );
	}

	// Tell libpng where is our image data
	png_set_rows (ctx->png_ptr, ctx->info_ptr, ctx->row_pointers);

	// Write file header and image data
	png_write_png (ctx->png_ptr, ctx->info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	// Tell libpng we have no more data to write
	png_write_end (ctx->png_ptr, (png_infop) NULL);

	// Free resources
	free(ctx->img_data);
	free(ctx->row_pointers);
	png_destroy_write_struct (&(ctx->png_ptr), &(ctx->info_ptr));
	if (ctx->source == PNGU_SOURCE_DEVICE)
		fclose (ctx->fd);

	// Success
	return PNGU_OK;
}


// This function is taken from a libogc example
PNGU_u32 PNGU_RGB8_TO_YCbYCr (PNGU_u8 r1, PNGU_u8 g1, PNGU_u8 b1, PNGU_u8 r2, PNGU_u8 g2, PNGU_u8 b2)
{
	int y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
	int cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
	int cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;
 
	int y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
	int cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
	int cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;
 
	int cb = (cb1 + cb2) >> 1;
	int cr = (cr1 + cr2) >> 1;
 
	return (PNGU_u32) ((y1 << 24) | (cb << 16) | (y2 << 8) | cr);
}


void PNGU_YCbYCr_TO_RGB8 (PNGU_u32 ycbycr, PNGU_u8 *r1, PNGU_u8 *g1, PNGU_u8 *b1, PNGU_u8 *r2, PNGU_u8 *g2, PNGU_u8 *b2)
{
	PNGU_u8 *val = (PNGU_u8 *) &ycbycr;

	int r = 1.371f * (val[3] - 128);
	int g = - 0.698f * (val[3] - 128) - 0.336f * (val[1] - 128);
	int b = 1.732f * (val[1] - 128);

	*r1 = pngu_clamp (val[0] + r, 0, 255);
	*g1 = pngu_clamp (val[0] + g, 0, 255);
	*b1 = pngu_clamp (val[0] + b, 0, 255);

	*r2 = pngu_clamp (val[2] + r, 0, 255);
	*g2 = pngu_clamp (val[2] + g, 0, 255);
	*b2 = pngu_clamp (val[2] + b, 0, 255);
}


int pngu_info (IMGCTX ctx)
{
	png_byte magic[8];

	// Check if there is a file selected and if it is a valid .png
	if (ctx->source == PNGU_SOURCE_BUFFER)
		memcpy (magic, ctx->buffer, 8);

	else if (ctx->source == PNGU_SOURCE_DEVICE)
	{
		// Open file
		if (!(ctx->fd = fopen (ctx->filename, "rb")))
			return PNGU_CANT_OPEN_FILE;

		// Load first 8 bytes into magic buffer
		if (fread (magic, 1, 8, ctx->fd) != 8)
		{
			fclose (ctx->fd);
			return PNGU_CANT_READ_FILE;
		}
	}

	else
		return PNGU_NO_FILE_SELECTED;;

	if (png_sig_cmp(magic, 0, 8) != 0)
	{
		if (ctx->source == PNGU_SOURCE_DEVICE)
			fclose (ctx->fd);
		return PNGU_FILE_IS_NOT_PNG;
	}

	// Allocation of libpng structs
	ctx->png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!(ctx->png_ptr))
	{
		if (ctx->source == PNGU_SOURCE_DEVICE)
			fclose (ctx->fd);
		return PNGU_LIB_ERROR;
	}

	ctx->info_ptr = png_create_info_struct (ctx->png_ptr);
	if (!(ctx->info_ptr))
	{
		if (ctx->source == PNGU_SOURCE_DEVICE)
			fclose (ctx->fd);
		png_destroy_read_struct (&(ctx->png_ptr), (png_infopp)NULL, (png_infopp)NULL);
		return PNGU_LIB_ERROR;
	}

	if (ctx->source == PNGU_SOURCE_BUFFER)
	{
		// Installation of our custom data provider function
		ctx->cursor = 0;
		png_set_read_fn (ctx->png_ptr, ctx, pngu_read_data_from_buffer);
	}
	else if (ctx->source == PNGU_SOURCE_DEVICE)
	{
		// Default data provider uses function fread, so it needs to use our FILE*
		png_init_io (ctx->png_ptr, ctx->fd);
		png_set_sig_bytes (ctx->png_ptr, 8); // We have read 8 bytes already to check PNG authenticity
	}

	// Read png header
	png_read_info (ctx->png_ptr, ctx->info_ptr);

	// Query image properties if they have not been queried before
	if (!ctx->propRead)
	{
		png_uint_32 width, height;
		png_get_IHDR(ctx->png_ptr, ctx->info_ptr, &width, &height,
					(int *) &(ctx->prop.imgBitDepth), 
					(int *) &(ctx->prop.imgColorType),
					NULL, NULL, NULL);

		ctx->prop.imgWidth = width;
		ctx->prop.imgHeight = height;
		switch (ctx->prop.imgColorType)
		{
			case PNG_COLOR_TYPE_GRAY:
				ctx->prop.imgColorType = PNGU_COLOR_TYPE_GRAY;
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				ctx->prop.imgColorType = PNGU_COLOR_TYPE_GRAY_ALPHA;
				break;
			case PNG_COLOR_TYPE_PALETTE:
				ctx->prop.imgColorType = PNGU_COLOR_TYPE_PALETTE;
				break;
			case PNG_COLOR_TYPE_RGB:
				ctx->prop.imgColorType = PNGU_COLOR_TYPE_RGB;
				break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
				ctx->prop.imgColorType = PNGU_COLOR_TYPE_RGB_ALPHA;
				break;
			default:
				ctx->prop.imgColorType = PNGU_COLOR_TYPE_UNKNOWN;
				break;
		}

		// Constant used to scale 16 bit values to 8 bit values
		int scale = 1;
		if (ctx->prop.imgBitDepth == 16)
			scale = 256;

		// Query background color, if any.
		png_color_16p background;
		ctx->prop.validBckgrnd = 0;
		if (((ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB_ALPHA)) &&
			(png_get_bKGD (ctx->png_ptr, ctx->info_ptr, &background)))
		{
			ctx->prop.validBckgrnd = 1;
			ctx->prop.bckgrnd.r = background->red / scale;
			ctx->prop.bckgrnd.g = background->green / scale;
			ctx->prop.bckgrnd.b = background->blue / scale;
		}
		else if (((ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA)) &&
			(png_get_bKGD (ctx->png_ptr, ctx->info_ptr, &background)))
		{
			ctx->prop.validBckgrnd = 1;
			ctx->prop.bckgrnd.r = ctx->prop.bckgrnd.g = ctx->prop.bckgrnd.b = background->gray / scale;
		}

		// Query list of transparent colors, if any.
		u32 i;
		png_bytep trans;
		png_color_16p trans_values;
		ctx->prop.numTrans = 0;
		ctx->prop.trans = NULL;
		if (((ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB_ALPHA)) &&
			(png_get_tRNS (ctx->png_ptr, ctx->info_ptr, &trans, (int *) &(ctx->prop.numTrans), &trans_values)))
		{
			if (ctx->prop.numTrans)
			{
				ctx->prop.trans = malloc(sizeof (PNGUCOLOR) * ctx->prop.numTrans);
				if (ctx->prop.trans)
				{
					for (i = 0; i < ctx->prop.numTrans; i++)
					{
						ctx->prop.trans[i].r = trans_values[i].red / scale;
						ctx->prop.trans[i].g = trans_values[i].green / scale;
						ctx->prop.trans[i].b = trans_values[i].blue / scale;
					}
				}
				else
					ctx->prop.numTrans = 0;
			}
		}
		else if (((ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA)) &&
			(png_get_tRNS (ctx->png_ptr, ctx->info_ptr, &trans, (int *) &(ctx->prop.numTrans), &trans_values)))
		{
			if (ctx->prop.numTrans)
			{
				ctx->prop.trans = malloc(sizeof (PNGUCOLOR) * ctx->prop.numTrans);
				if (ctx->prop.trans)
					for (i = 0; i < ctx->prop.numTrans; i++)
						ctx->prop.trans[i].r = ctx->prop.trans[i].g = ctx->prop.trans[i].b = 
						trans_values[i].gray / scale;
				else
					ctx->prop.numTrans = 0;
			}
		}

		ctx->propRead = 1;
	}

	// Success
	ctx->infoRead = 1;

	return PNGU_OK;
}


int pngu_decode (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, PNGU_u32 stripAlpha, int force32bit)
{
	u32 i;
	int mem_err = 0;

	// Read info if it hasn't been read before
	if (!ctx->infoRead)
	{
		i = pngu_info (ctx);
		if (i != PNGU_OK) return i;
	}

	// Check if the user has specified the real width and height of the image
	if ( (ctx->prop.imgWidth != width) || (ctx->prop.imgHeight != height) )
		return PNGU_INVALID_WIDTH_OR_HEIGHT;

	// Check if color type is supported by PNGU
	if ( (ctx->prop.imgColorType == PNGU_COLOR_TYPE_PALETTE) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_UNKNOWN) )
		return PNGU_UNSUPPORTED_COLOR_TYPE;

	 // error handling
	jmp_buf save_jmp;
	memcpy(save_jmp, png_jmpbuf(ctx->png_ptr), sizeof(save_jmp));
	if (setjmp(png_jmpbuf(ctx->png_ptr)))
	{
		error:
		memcpy(png_jmpbuf(ctx->png_ptr), save_jmp, sizeof(save_jmp));
		free(ctx->row_pointers);
		free(ctx->img_data);
		pngu_free_info (ctx);
		//printf("*** This is a corrupted image!!\n"); sleep(5);
		return mem_err ? PNGU_LIB_ERROR : -666;
	}
	png_set_error_fn(ctx->png_ptr, NULL, user_error, user_error);
	// Scale 16 bit samples to 8 bit
	if (ctx->prop.imgBitDepth == 16)
		png_set_strip_16 (ctx->png_ptr);

	// Remove alpha channel if we don't need it
	if (stripAlpha && ((ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB_ALPHA) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA)))
		png_set_strip_alpha (ctx->png_ptr);

	// Expand 1, 2 and 4 bit samples to 8 bit
	if (ctx->prop.imgBitDepth < 8)
		png_set_packing (ctx->png_ptr);

	// Transform grayscale images to RGB
	if ( (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY) || (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY_ALPHA) )
		png_set_gray_to_rgb (ctx->png_ptr);

	// Transform RBG images to RGBA
	if (force32bit && (ctx->prop.imgColorType == PNGU_COLOR_TYPE_GRAY || ctx->prop.imgColorType == PNGU_COLOR_TYPE_RGB))
		png_set_filler(ctx->png_ptr, 0xFF, PNG_FILLER_AFTER);

	// Flush transformations
	png_read_update_info (ctx->png_ptr, ctx->info_ptr);

	// Allocate memory to store the image
	png_uint_32 rowbytes = png_get_rowbytes (ctx->png_ptr, ctx->info_ptr);
	if (rowbytes % 4)
		rowbytes = ((rowbytes / 4) + 1) * 4; // Add extra padding so each row starts in a 4 byte boundary

	ctx->img_data = malloc(rowbytes * ctx->prop.imgHeight);
	if (!ctx->img_data)
	{
		mem_err = 1;
		goto error;
	}

	ctx->row_pointers = malloc(sizeof (png_bytep) * ctx->prop.imgHeight);
	if (!ctx->row_pointers)
	{
		mem_err = 1;
		goto error;
	}

	for (i = 0; i < ctx->prop.imgHeight; i++)
		ctx->row_pointers[i] = ctx->img_data + (i * rowbytes);

	// Transform the image and copy it to our allocated memory
	if (png_get_interlace_type(ctx->png_ptr, ctx->info_ptr) != PNG_INTERLACE_NONE)
	png_read_image (ctx->png_ptr, ctx->row_pointers);
	else
	{
		int rowsLeft = ctx->prop.imgHeight;
		png_bytep *curRow = ctx->row_pointers;
		while (rowsLeft > 0)
		{
			int chunk = rowsLeft > 0x80 ? 0x80 : rowsLeft;
			png_read_rows(ctx->png_ptr, curRow, NULL, chunk);
			usleep(1000);
			curRow += chunk;
			rowsLeft -= chunk;
		}
	}

	// restore default error handling
	memcpy(png_jmpbuf(ctx->png_ptr), save_jmp, sizeof(save_jmp));

	// Free resources
	pngu_free_info(ctx);

	// Success
	return PNGU_OK;
}


void pngu_free_info (IMGCTX ctx)
{
	if (ctx->infoRead)
	{
		if (ctx->source == PNGU_SOURCE_DEVICE)
			fclose (ctx->fd);

		png_destroy_read_struct (&(ctx->png_ptr), &(ctx->info_ptr), (png_infopp)NULL);

		ctx->infoRead = 0;
	}
}


void pngu_read_data_from_buffer (png_structp png_ptr, png_bytep data, png_size_t length)
{
	IMGCTX ctx = (IMGCTX) png_get_io_ptr (png_ptr);
	if (ctx->buf_size && (ctx->cursor + length > ctx->buf_size))
	{
		static char err_str[40];
		snprintf(err_str, sizeof(err_str), "read error (%x/%x)", 
			ctx->cursor + length, ctx->buf_size);
		png_error(png_ptr, err_str);
	}
	memcpy (data, ctx->buffer + ctx->cursor, length);
	ctx->cursor += length;
}


void pngu_write_data_to_buffer (png_structp png_ptr, png_bytep data, png_size_t length)
{
	IMGCTX ctx = (IMGCTX) png_get_io_ptr (png_ptr);
	memcpy (ctx->buffer + ctx->cursor, data, length);
	ctx->cursor += length;
}

// Function used in YCbYCr to RGB decoding
int pngu_clamp (int value, int min, int max)
{
	if (value < min) value = min;
	else if (value > max) value = max;

	return value;
}

