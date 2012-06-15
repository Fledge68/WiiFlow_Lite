// Inspired by WiiPower's "video toy", but simpler

#include "videopatch.h"

#include <string.h>

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

extern GXRModeObj TVNtsc480Int;

GXRModeObj TVPal528Prog = 
{
    6,      		 // viDisplayMode
    640,             // fbWidth
    528,             // efbHeight
    528,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 528)/2,        // viYOrigin
    640,             // viWidth
    528,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},
	
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 0,         // line n-1
		 0,         // line n-1
		21,         // line n
		22,         // line n
		21,         // line n
		 0,         // line n+1
		 0          // line n+1
	}

};

GXRModeObj TVPal528ProgSoft = 
{
    6,      		 // viDisplayMode
    640,             // fbWidth
    528,             // efbHeight
    528,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 528)/2,        // viYOrigin
    640,             // viWidth
    528,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},
	
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 8,         // line n-1
		 8,         // line n-1
		10,         // line n
		12,         // line n
		10,         // line n
		 8,         // line n+1
		 8          // line n+1
	}

};

GXRModeObj TVPal528ProgUnknown = 
{
    6,      		 // viDisplayMode
    640,             // fbWidth
    264,             // efbHeight
    524,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 528)/2,        // viYOrigin
    640,             // viWidth
    524,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_TRUE,         // aa

    // sample points arranged in increasing Y order
	{
		{3,2},{9,6},{3,10},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{3,2},{9,6},{3,10},  // pix 1
		{9,2},{3,6},{9,10},  // pix 2
		{9,2},{3,6},{9,10}   // pix 3
	},
	
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 4,         // line n-1
		 8,         // line n-1
		12,         // line n
		16,         // line n
		12,         // line n
		 8,         // line n+1
		 4          // line n+1
	}

};

GXRModeObj TVMpal480Prog =
{
    10,     		 // viDisplayMode
    640,             // fbWidth
    480,             // efbHeight
    480,             // xfbHeight
    (VI_MAX_WIDTH_NTSC - 640)/2,        // viXOrigin
    (VI_MAX_HEIGHT_NTSC - 480)/2,       // viYOrigin
    640,             // viWidth
    480,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
    {
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
          0,         // line n-1
          0,         // line n-1
         21,         // line n
         22,         // line n
         21,         // line n
          0,         // line n+1
          0          // line n+1
    }
};

static const GXRModeObj *g_vidmodes[] = {
	&TVNtsc480Int,
	&TVNtsc480IntDf,
	&TVNtsc480Prog,

	&TVPal528Int, 
	&TVPal528IntDf,
	&TVPal528Prog,
	&TVPal528ProgSoft,
	&TVPal528ProgUnknown,

	&TVMpal480IntDf,
	&TVMpal480Prog,

	&TVEurgb60Hz480Int,
	&TVEurgb60Hz480IntDf,
	&TVEurgb60Hz480Prog
};

// Level :
// 0 : If same number of lines and same mode type (interlaced, progressive)
// 1 : If same mode type
// 2 : Always
static void applyVideoPatch(void *dst, u32 len, GXRModeObj *rmode, int level)
{
	u32 i;
	u32 *bufEnd = (u32 *)((u8 *)dst + (len - sizeof *rmode));
	u32 *p = (u32 *)dst;
	while (p <= bufEnd)
	{
		for (i = 0; i < ARRAY_SIZE(g_vidmodes); ++i)
			if (memcmp(p, g_vidmodes[i], sizeof *rmode) == 0)
			{
				// Video mode description found, replace it
				GXRModeObj *m = (GXRModeObj *)p;
				if (level == 2
					|| (((m->viTVMode & 3) == VI_PROGRESSIVE) == ((rmode->viTVMode & 3) == VI_PROGRESSIVE)
						&& (level == 1 || m->viHeight == rmode->viHeight)))
					memcpy(p, rmode, sizeof *rmode);
				p = (u32 *)(m + 1);
				break;
			}
		if (i == ARRAY_SIZE(g_vidmodes))
			p++;
	}
}

static bool compare_videomodes(GXRModeObj* mode1, GXRModeObj* mode2)
{
	return memcmp(mode1, mode2, sizeof *mode1) == 0;	// padding seems to always be 0
}

static void patch_videomode(GXRModeObj* mode1, GXRModeObj* mode2)
{
	memcpy(mode1, mode2, sizeof *mode1);
}

static GXRModeObj* PAL2NTSC[]={
	&TVMpal480IntDf,		&TVNtsc480IntDf,
	&TVPal264Ds,			&TVNtsc240Ds,
	&TVPal264DsAa,			&TVNtsc240DsAa,
	&TVPal264Int,			&TVNtsc240Int,
	&TVPal264IntAa,			&TVNtsc240IntAa,
	&TVPal524IntAa,			&TVNtsc480IntAa,
	&TVPal528Int,			&TVNtsc480IntAa,
	&TVPal528IntDf,			&TVNtsc480IntDf,
	&TVPal574IntDfScale,	&TVNtsc480IntDf,
	&TVEurgb60Hz240Ds,		&TVNtsc240Ds,
	&TVEurgb60Hz240DsAa,	&TVNtsc240DsAa,
	&TVEurgb60Hz240Int,		&TVNtsc240Int,
	&TVEurgb60Hz240IntAa,	&TVNtsc240IntAa,
	&TVEurgb60Hz480Int,		&TVNtsc480IntAa,
	&TVEurgb60Hz480IntDf,	&TVNtsc480IntDf,
	&TVEurgb60Hz480IntAa,	&TVNtsc480IntAa,
	&TVEurgb60Hz480Prog,	&TVNtsc480Prog,
	&TVEurgb60Hz480ProgSoft,&TVNtsc480Prog,
	&TVEurgb60Hz480ProgAa,  &TVNtsc480Prog,
	0,0
};

static GXRModeObj* NTSC2PAL[]={
	&TVNtsc240Ds,			&TVPal264Ds,
	&TVNtsc240DsAa,			&TVPal264DsAa,
	&TVNtsc240Int,			&TVPal264Int,
	&TVNtsc240IntAa,		&TVPal264IntAa,
	&TVNtsc480IntDf,		&TVPal528IntDf,
	&TVNtsc480IntAa,		&TVPal524IntAa,
	&TVNtsc480Prog,			&TVPal528IntDf,
	0,0
};

static GXRModeObj* NTSC2PAL60[]={
	&TVNtsc240Ds,			&TVEurgb60Hz240Ds,
	&TVNtsc240DsAa,			&TVEurgb60Hz240DsAa,
	&TVNtsc240Int,			&TVEurgb60Hz240Int,
	&TVNtsc240IntAa,		&TVEurgb60Hz240IntAa,
	&TVNtsc480IntDf,		&TVEurgb60Hz480IntDf,
	&TVNtsc480IntAa,		&TVEurgb60Hz480IntAa,
	&TVNtsc480Prog,			&TVEurgb60Hz480Prog,
	0,0
};

static bool Search_and_patch_Video_Modes(void *Address, u32 Size, GXRModeObj* Table[])
{
	u8 *Addr = (u8 *)Address;
	bool found = 0;
	u32 i;

	while(Size >= sizeof(GXRModeObj))
	{
		for(i = 0; Table[i]; i+=2)
		{
			if(compare_videomodes(Table[i], (GXRModeObj*)Addr))
			{
				found = 1;
				patch_videomode((GXRModeObj*)Addr, Table[i+1]);
				Addr += (sizeof(GXRModeObj)-4);
				Size -= (sizeof(GXRModeObj)-4);
				break;
			}
		}
		Addr += 4;
		Size -= 4;
	}
	return found;
}

void patchVideoModes(void *dst, u32 len, int vidMode, GXRModeObj *vmode, int patchVidModes)
{
	GXRModeObj **table = 0;

	if (vidMode == 5) // system
	{
		return;
	}
	if (vidMode == 6) // progressive 480P(NTSC + patch all)
	{
		applyVideoPatch(dst, len, vmode, 2);
	}
	else if (patchVidModes > 0 && vmode != 0)
	{
		applyVideoPatch(dst, len, vmode, patchVidModes - 1);
	}
	else
	{
		switch(vidMode)
		{
			case 0: // default / disc / game
				break;
			case 1: // PAL50
				Search_and_patch_Video_Modes(dst, len, NTSC2PAL);
				break;
			case 2: // PAL60
				Search_and_patch_Video_Modes(dst, len, NTSC2PAL60);
				break;
			case 3: // NTSC
				Search_and_patch_Video_Modes(dst, len, PAL2NTSC);
				break;
			case 4: // auto patch / system
				switch (CONF_GetVideo())
				{
					case CONF_VIDEO_PAL:
						table = CONF_GetEuRGB60() > 0 ? NTSC2PAL60 : NTSC2PAL;
						break;
					case CONF_VIDEO_MPAL:
						table = NTSC2PAL;
						break;
					default:
						table = PAL2NTSC;
						break;
				}
				Search_and_patch_Video_Modes(dst, len, table);
				break;
			default:
				break;
		}
	}
}
