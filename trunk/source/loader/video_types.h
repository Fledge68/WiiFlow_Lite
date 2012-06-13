/*-------------------------------------------------------------

video_types.h -- support header

Copyright (C) 2004
Michael Wiedenbauer (shagkur)
Dave Murphy (WinterMute)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/


#ifndef __VIDEO_TYPES_SYS_H__
#define __VIDEO_TYPES_SYS_H__

#undef VI_MAX_HEIGHT_PAL

/*!
\file video_types.h
\brief support header
*/

#include <gctypes.h>

#define VI_DISPLAY_PIX_SZ           2		/*!< multiplier to get real pixel size in bytes */

#define VI_INTERLACE                0		/*!< Video mode INTERLACED. */
#define VI_NON_INTERLACE            1		/*!< Video mode NON INTERLACED */
#define VI_PROGRESSIVE              2		/*!< Video mode PROGRESSIVE. Special mode for higher quality */

#define VI_NTSC                     0		/*!< Video standard used in North America and Japan */
#define VI_PAL                      1		/*!< Video standard used in Europe */
#define VI_MPAL                     2		/*!< Video standard, similar to NTSC, used in Brazil */
#define VI_DEBUG                    3		/*!< Video standard, for debugging purpose, used in North America and Japan. Special decoder needed */
#define VI_DEBUG_PAL                4		/*!< Video standard, for debugging purpose, used in Europe. Special decoder needed */
#define VI_EURGB60                  5		/*!< RGB 60Hz, 480 lines mode (same timing and aspect ratio as NTSC) used in Europe */

#define VI_XFBMODE_SF				0
#define VI_XFBMODE_DF				1

#define VI_FIELD_ABOVE              1		/*!< Upper field in DS mode */
#define VI_FIELD_BELOW              0		/*!< Lower field in DS mode */

// Maximum screen space
#define VI_MAX_WIDTH_NTSC           720
#define VI_MAX_HEIGHT_NTSC          480

#define VI_MAX_WIDTH_PAL            720
#define VI_MAX_HEIGHT_PAL           574

#define VI_MAX_WIDTH_MPAL           720
#define VI_MAX_HEIGHT_MPAL          480

#define VI_MAX_WIDTH_EURGB60        VI_MAX_WIDTH_NTSC
#define VI_MAX_HEIGHT_EURGB60       VI_MAX_HEIGHT_NTSC

#define VI_TVMODE(fmt, mode)   ( ((fmt) << 2) + (mode) )

#define VI_TVMODE_NTSC_INT			VI_TVMODE(VI_NTSC,        VI_INTERLACE)
#define VI_TVMODE_NTSC_DS			VI_TVMODE(VI_NTSC,        VI_NON_INTERLACE)
#define VI_TVMODE_NTSC_PROG			VI_TVMODE(VI_NTSC,        VI_PROGRESSIVE)
#define VI_TVMODE_NTSC_PROG_DS		VI_TVMODE(VI_NTSC,        (VI_PROGRESSIVE|VI_NON_INTERLACE))

#define VI_TVMODE_PAL_INT			VI_TVMODE(VI_PAL,         VI_INTERLACE)
#define VI_TVMODE_PAL_DS			VI_TVMODE(VI_PAL,         VI_NON_INTERLACE)

#define VI_TVMODE_EURGB60_INT		VI_TVMODE(VI_EURGB60,     VI_INTERLACE)
#define VI_TVMODE_EURGB60_DS		VI_TVMODE(VI_EURGB60,     VI_NON_INTERLACE)
#define VI_TVMODE_EURGB60_PROG		VI_TVMODE(VI_EURGB60,     VI_PROGRESSIVE)
#define VI_TVMODE_EURGB60_PROG_DS	VI_TVMODE(VI_EURGB60,     (VI_PROGRESSIVE|VI_NON_INTERLACE))

#define VI_TVMODE_MPAL_INT			VI_TVMODE(VI_MPAL,        VI_INTERLACE)
#define VI_TVMODE_MPAL_DS			VI_TVMODE(VI_MPAL,        VI_NON_INTERLACE)

#define VI_TVMODE_DEBUG_INT			VI_TVMODE(VI_DEBUG,       VI_INTERLACE)

#define VI_TVMODE_DEBUG_PAL_INT		VI_TVMODE(VI_DEBUG_PAL,   VI_INTERLACE)
#define VI_TVMODE_DEBUG_PAL_DS		VI_TVMODE(VI_DEBUG_PAL,   VI_NON_INTERLACE)

extern GXRModeObj CUSTOM_TVNtsc240Ds;				/*!< Video and render mode configuration for 240 lines,singlefield NTSC mode */
extern GXRModeObj CUSTOM_TVNtsc240DsAa;				/*!< Video and render mode configuration for 240 lines,singlefield,antialiased NTSC mode */
extern GXRModeObj CUSTOM_TVNtsc240Int;				/*!< Video and render mode configuration for 240 lines,interlaced NTSC mode */
extern GXRModeObj CUSTOM_TVNtsc240IntAa;			/*!< Video and render mode configuration for 240 lines,interlaced,antialiased NTSC mode */
extern GXRModeObj CUSTOM_TVNtsc480Int;				/*!< Video and render mode configuration for 480 lines,interlaced NTSC mode */
extern GXRModeObj CUSTOM_TVNtsc480IntDf;			/*!< Video and render mode configuration for 480 lines,interlaced,doublefield NTSC mode */
extern GXRModeObj CUSTOM_TVNtsc480IntAa;			/*!< Video and render mode configuration for 480 lines,interlaced,doublefield,antialiased NTSC mode */
extern GXRModeObj CUSTOM_TVNtsc480Prog;				/*!< Video and render mode configuration for 480 lines,progressive,singlefield NTSC mode */
extern GXRModeObj CUSTOM_TVNtsc480ProgSoft;
extern GXRModeObj CUSTOM_TVNtsc480ProgAa;
extern GXRModeObj CUSTOM_TVMpal480IntDf;			/*!< Video and render mode configuration for 480 lines,interlaced,doublefield,antialiased MPAL mode */
extern GXRModeObj CUSTOM_TVMpal480IntAa;
extern GXRModeObj CUSTOM_TVMpal480Prog;
extern GXRModeObj CUSTOM_TVMpal240Ds;
extern GXRModeObj CUSTOM_TVMpal240DsAa;
extern GXRModeObj CUSTOM_TVPal264Ds;				/*!< Video and render mode configuration for 264 lines,singlefield PAL mode */
extern GXRModeObj CUSTOM_TVPal264DsAa;				/*!< Video and render mode configuration for 264 lines,singlefield,antialiased PAL mode */
extern GXRModeObj CUSTOM_TVPal264Int;				/*!< Video and render mode configuration for 264 lines,interlaced PAL mode */
extern GXRModeObj CUSTOM_TVPal264IntAa;				/*!< Video and render mode configuration for 264 lines,interlaced,antialiased PAL mode */
extern GXRModeObj CUSTOM_TVPal524IntAa;				/*!< Video and render mode configuration for 524 lines,interlaced,antialiased PAL mode */
extern GXRModeObj CUSTOM_TVPal528Int;				/*!< Video and render mode configuration for 528 lines,interlaced,antialiased PAL mode */
extern GXRModeObj CUSTOM_TVPal528IntDf;				/*!< Video and render mode configuration for 264 lines,interlaced,doublefield antialiased PAL mode */
extern GXRModeObj CUSTOM_TVPal574IntDfScale;
extern GXRModeObj CUSTOM_TVPal528Prog;
extern GXRModeObj CUSTOM_TVPal528ProgSoft;
extern GXRModeObj CUSTOM_TVPal528ProgUnknown;
extern GXRModeObj CUSTOM_TVEurgb60Hz240Ds;
extern GXRModeObj CUSTOM_TVEurgb60Hz240DsAa;
extern GXRModeObj CUSTOM_TVEurgb60Hz240Int;
extern GXRModeObj CUSTOM_TVEurgb60Hz240IntAa;
extern GXRModeObj CUSTOM_TVEurgb60Hz480Int;
extern GXRModeObj CUSTOM_TVEurgb60Hz480IntDf;
extern GXRModeObj CUSTOM_TVEurgb60Hz480IntAa;
extern GXRModeObj CUSTOM_TVEurgb60Hz480Prog;
extern GXRModeObj CUSTOM_TVEurgb60Hz480ProgSoft;
extern GXRModeObj CUSTOM_TVEurgb60Hz480ProgAa;

#endif
