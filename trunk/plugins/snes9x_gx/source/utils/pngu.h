/********************************************************************************************
*
* PNGU
* 
* Original author: frontier (http://frontier-dev.net)
* Modified by Tantric, 2009-2010
*
********************************************************************************************/

#ifndef __PNGU__
#define __PNGU__

#include <gccore.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct
{
	u8 r;
	u8 g;
	u8 b;
} PNGUCOLOR;

typedef struct
{
	u32 imgWidth; // In pixels
	u32 imgHeight; // In pixels
	u32 imgBitDepth; // In bitx
	u32 imgColorType; // PNGU_COLOR_TYPE_*
	u32 validBckgrnd; // Non zero if there is a background color
	PNGUCOLOR bckgrnd; // Background color
	u32 numTrans; // Number of transparent colors
	PNGUCOLOR *trans; // Transparent colors
} PNGUPROP;

// Image context, always initialize with SelectImageFrom* and free with ReleaseImageContext
struct _IMGCTX;
typedef struct _IMGCTX *IMGCTX; 

/****************************************************************************
* Image context handling							                        *
****************************************************************************/

// Selects a PNG file, previously loaded into a buffer, and creates an image context for subsequent processing.
IMGCTX PNGU_SelectImageFromBuffer (const void *buffer);

// Selects a PNG file, from any devoptab device, and creates an image context for subsequent processing.
IMGCTX PNGU_SelectImageFromDevice (const char *filename);

// Frees resources associated with an image context. Always call this function when you no longer need the IMGCTX.
void PNGU_ReleaseImageContext (IMGCTX ctx);

/****************************************************************************
* Miscellaneous								                             	*
****************************************************************************/

// Retrieves info from selected PNG file, including image dimensions, color format, background and transparency colors.
int PNGU_GetImageProperties (IMGCTX ctx, PNGUPROP *fileproperties);

/****************************************************************************
* Image conversion								                            *
****************************************************************************/

u8 * DecodePNG(const u8 *src, int *width, int *height, u8 *dst, int maxwidth, int maxheight);
int PNGU_EncodeFromRGB (IMGCTX ctx, u32 width, u32 height, void *buffer, u32 stride);
int PNGU_EncodeFromGXTexture (IMGCTX ctx, u32 width, u32 height, void *buffer, u32 stride);
int PNGU_EncodeFromEFB (IMGCTX ctx, u32 width, u32 height);

#ifdef __cplusplus
	}
#endif

#endif
