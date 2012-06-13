// Inspired by WiiPower's "video toy", but simpler

#include "videopatch.h"
#include "video_types.h"

#include <string.h>

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

static const GXRModeObj *g_vidmodes[] = {
	&CUSTOM_TVNtsc480Int,
	&CUSTOM_TVNtsc480IntDf,
	&CUSTOM_TVNtsc480Prog,

	&CUSTOM_TVPal528Int, 
	&CUSTOM_TVPal528IntDf,
	&CUSTOM_TVPal528Prog,
	&CUSTOM_TVPal528ProgSoft,
	&CUSTOM_TVPal528ProgUnknown,

	&CUSTOM_TVMpal480IntDf,
	&CUSTOM_TVMpal480Prog,

	&CUSTOM_TVEurgb60Hz480Int,
	&CUSTOM_TVEurgb60Hz480IntDf,
	&CUSTOM_TVEurgb60Hz480Prog
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
	&CUSTOM_TVMpal480IntDf,		&CUSTOM_TVNtsc480IntDf,
	&CUSTOM_TVPal264Ds,			&CUSTOM_TVNtsc240Ds,
	&CUSTOM_TVPal264DsAa,			&CUSTOM_TVNtsc240DsAa,
	&CUSTOM_TVPal264Int,			&CUSTOM_TVNtsc240Int,
	&CUSTOM_TVPal264IntAa,			&CUSTOM_TVNtsc240IntAa,
	&CUSTOM_TVPal524IntAa,			&CUSTOM_TVNtsc480IntAa,
	&CUSTOM_TVPal528Int,			&CUSTOM_TVNtsc480IntAa,
	&CUSTOM_TVPal528IntDf,			&CUSTOM_TVNtsc480IntDf,
	&CUSTOM_TVPal574IntDfScale,	&CUSTOM_TVNtsc480IntDf,
	&CUSTOM_TVEurgb60Hz240Ds,		&CUSTOM_TVNtsc240Ds,
	&CUSTOM_TVEurgb60Hz240DsAa,	&CUSTOM_TVNtsc240DsAa,
	&CUSTOM_TVEurgb60Hz240Int,		&CUSTOM_TVNtsc240Int,
	&CUSTOM_TVEurgb60Hz240IntAa,	&CUSTOM_TVNtsc240IntAa,
	&CUSTOM_TVEurgb60Hz480Int,		&CUSTOM_TVNtsc480IntAa,
	&CUSTOM_TVEurgb60Hz480IntDf,	&CUSTOM_TVNtsc480IntDf,
	&CUSTOM_TVEurgb60Hz480IntAa,	&CUSTOM_TVNtsc480IntAa,
	&CUSTOM_TVEurgb60Hz480Prog,	&CUSTOM_TVNtsc480Prog,
	&CUSTOM_TVEurgb60Hz480ProgSoft,&CUSTOM_TVNtsc480Prog,
	&CUSTOM_TVEurgb60Hz480ProgAa,  &CUSTOM_TVNtsc480Prog,
	0,0
};

static GXRModeObj* NTSC2PAL[]={
	&CUSTOM_TVNtsc240Ds,			&CUSTOM_TVPal264Ds,
	&CUSTOM_TVNtsc240DsAa,			&CUSTOM_TVPal264DsAa,
	&CUSTOM_TVNtsc240Int,			&CUSTOM_TVPal264Int,
	&CUSTOM_TVNtsc240IntAa,		&CUSTOM_TVPal264IntAa,
	&CUSTOM_TVNtsc480IntDf,		&CUSTOM_TVPal528IntDf,
	&CUSTOM_TVNtsc480IntAa,		&CUSTOM_TVPal524IntAa,
	&CUSTOM_TVNtsc480Prog,			&CUSTOM_TVPal528IntDf,
	0,0
};

static GXRModeObj* NTSC2PAL60[]={
	&CUSTOM_TVNtsc240Ds,			&CUSTOM_TVEurgb60Hz240Ds,
	&CUSTOM_TVNtsc240DsAa,			&CUSTOM_TVEurgb60Hz240DsAa,
	&CUSTOM_TVNtsc240Int,			&CUSTOM_TVEurgb60Hz240Int,
	&CUSTOM_TVNtsc240IntAa,		&CUSTOM_TVEurgb60Hz240IntAa,
	&CUSTOM_TVNtsc480IntDf,		&CUSTOM_TVEurgb60Hz480IntDf,
	&CUSTOM_TVNtsc480IntAa,		&CUSTOM_TVEurgb60Hz480IntAa,
	&CUSTOM_TVNtsc480Prog,			&CUSTOM_TVEurgb60Hz480Prog,
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
