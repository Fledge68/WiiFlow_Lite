/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "modplay.h"
#include "effects.h"
#include "xm.h"


#define DESCRIPTION "Triton FastTracker II Extended Module"
#define AUTHOR      "Christian Nowak <chnowak@web.de>, http://chn.roarvgm.com/"
#define VERSION     "v0.01b"
#define COPYRIGHT   "Copyright (c) 2003, 2007"



#define SAFE_MALLOC(dest, a) \
  dest = malloc(a); \
  if (dest == NULL) { \
    MODFILE_Free(xm); \
    return -2; \
  }

static int xmfinetab[] = {

  7893, 7897, 7900, 7904, 7907, 7911, 7915, 7918, 7922, 7925, 7929, 7932, 7936, 7940, 7943, 7947,
  7950, 7954, 7958, 7961, 7965, 7968, 7972, 7975, 7979, 7983, 7986, 7990, 7993, 7997, 8001, 8004,
  8008, 8012, 8015, 8019, 8022, 8026, 8030, 8033, 8037, 8041, 8044, 8048, 8051, 8055, 8059, 8062,
  8066, 8070, 8073, 8077, 8081, 8084, 8088, 8091, 8095, 8099, 8102, 8106, 8110, 8113, 8117, 8121,
  8124, 8128, 8132, 8135, 8139, 8143, 8146, 8150, 8154, 8157, 8161, 8165, 8169, 8172, 8176, 8180,
  8183, 8187, 8191, 8194, 8198, 8202, 8205, 8209, 8213, 8217, 8220, 8224, 8228, 8231, 8235, 8239,
  8243, 8246, 8250, 8254, 8257, 8261, 8265, 8269, 8272, 8276, 8280, 8284, 8287, 8291, 8295, 8299,
  8302, 8306, 8310, 8314, 8317, 8321, 8325, 8329, 8332, 8336, 8340, 8344, 8347, 8351, 8355, 8359,
  8363, 8366, 8370, 8374, 8378, 8381, 8385, 8389, 8393, 8397, 8400, 8404, 8408, 8412, 8416, 8419,
  8423, 8427, 8431, 8435, 8438, 8442, 8446, 8450, 8454, 8457, 8461, 8465, 8469, 8473, 8476, 8480,
  8484, 8488, 8492, 8496, 8499, 8503, 8507, 8511, 8515, 8519, 8523, 8526, 8530, 8534, 8538, 8542,
  8546, 8549, 8553, 8557, 8561, 8565, 8569, 8573, 8577, 8580, 8584, 8588, 8592, 8596, 8600, 8604,
  8608, 8611, 8615, 8619, 8623, 8627, 8631, 8635, 8639, 8643, 8646, 8650, 8654, 8658, 8662, 8666,
  8670, 8674, 8678, 8682, 8686, 8690, 8693, 8697, 8701, 8705, 8709, 8713, 8717, 8721, 8725, 8729,
  8733, 8737, 8741, 8745, 8749, 8752, 8756, 8760, 8764, 8768, 8772, 8776, 8780, 8784, 8788, 8792,
  8796, 8800, 8804, 8808, 8812, 8816, 8820, 8824, 8828, 8832, 8836, 8840, 8844, 8848, 8852, 8856
};




static u32 getu32(u8 *d) {

  return d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
}

static u16 getu16(u8 *d) {

  return d[0] | (d[1] << 8);
}

static int xmgetfinefreq(int fine) {

  /*  double fact = pow(2.0, 2.0 / 12.0);*/

  return xmfinetab[fine + 128];
  /*  return (int)(8363.0 * pow(fact, (double)fine / (double)steps));*/
  /*return 8363;*/
}

static void convertXMVolume(u8 v, u16 *e, u8 *o) {

  switch (v >> 4) {

    case 0x06: /* Volume slide down */
      *e = EFFECT_XMa0;
      *o = v & 0x0f;
      break;

    case 0x07: /* Volume slide up */
      *e = EFFECT_XMa0;
      *o = (v & 0x0f) << 4;
      break;

    case 0x08: /* Fine volume slide down */
      *e = EFFECT_STeb;
      *o = v & 0x0f;
      break;

    case 0x09: /* Fine volume slide up */
      *e = EFFECT_STea;
      *o = v & 0x0f;
      break;

    case 0x0c: /* Set panning */
      *e = EFFECT_ST80;
      *o = (v & 0x0f) << 4;
      break;

    case 0x0d: /* Panning slide left */
      *e = EFFECT_XM19;
      *o = v & 0x0f;
      break;

    case 0x0e: /* Panning slide right */
      *e = EFFECT_XM19;
      *o = (v & 0x0f) << 4;
      break;

    default:
      break;
  }
}

static void convertXMEffects(MODFILE *mod, int pattern, int pline, u8 effect, u8 operand, u16 *e, u8 *o) {

  if ((e == NULL) || (o == NULL))
    return;

  *o = operand;

  switch (effect) {

    case 0x00:  /* Arpeggio */
      if (operand != 0)
				*e = EFFECT_ST00;
      else
				*e = EFFECT_NONE;
      break;
    case 0x01:  /* Porta up */
      *e = EFFECT_XM01;
      break;
    case 0x02:  /* Porta down */
      *e = EFFECT_XM02;
      break;
    case 0x03:  /* Porta to note */
      *e = EFFECT_ST30;
      break;
    case 0x04:  /* Vibrato */
      *e = EFFECT_ST40;
      break;
    case 0x05:  /* Porta + volume slide */
      *e = EFFECT_ST50;
      break;
    case 0x06:  /* Vibrato + volume slide */
      *e = EFFECT_ST60;
      break;
    case 0x07:  /* Tremolo */
      *e = EFFECT_ST70;
      break;
    case 0x08:  /* Panning */
      *e = EFFECT_ST80;
      break;
    case 0x09:  /* Sample offset */
      *e = EFFECT_ST90;
      break;
    case 0x0a:  /* Volume slide */
      *e = EFFECT_XMa0;
      break;
    case 0x0b:  /* Pattern jump */
			mod->patterns[pattern][(pline * mod->nChannels) + mod->nChannels - 1].effect[0] = EFFECT_STb0;
			mod->patterns[pattern][(pline * mod->nChannels) + mod->nChannels - 1].operand[0] = operand;
	  	effect = EFFECT_NONE;
      break;
    case 0x0c:  /* Set volume */
      *e = EFFECT_STc0;
      if (operand > 64)
				operand = 64;
      *o = operand;
      break;
    case 0x0d:  /* Pattern break */
      operand = ((operand >> 4) * 10) + (operand & 0x0f);
			mod->patterns[pattern][(pline * mod->nChannels) + mod->nChannels - 1].effect[0] = EFFECT_STd0;
			mod->patterns[pattern][(pline * mod->nChannels) + mod->nChannels - 1].operand[0] = operand;
	  	*e = EFFECT_NONE;
      break;
    case 0x0e:
      switch (operand >> 4) {

				int temp;

        case 0x01:  /* Fine porta up */
	  			*e = EFFECT_STe1;
	  			*o = operand & 0x0f;
	  			break;
        case 0x02:  /* Fine porta down */
	  			*e = EFFECT_STe2;
	  			*o = operand & 0x0f;
	  			break;
        case 0x04:  /* Set vibrato waveform */
	  			*e = EFFECT_STe4;
	  			*o = operand & 0x0f;
	  			break;
        case 0x05:  /* Set finetune */
	  			*e = EFFECT_STe5;
	  			temp = operand & 0x0f;
	  			if (temp > 7) temp -= 16;
	  				temp += 8;
	  			*o = operand & 0x0f;
	  			break;
        case 0x06:  /* Pattern loop */
        	operand = operand & 0x0f;
					mod->patterns[pattern][(pline * mod->nChannels) + mod->nChannels - 1].effect[0] = EFFECT_STe6;
					mod->patterns[pattern][(pline * mod->nChannels) + mod->nChannels - 1].operand[0] = operand;
	  			*e = EFFECT_NONE;
	  			*o = 0;
	  			break;
        case 0x07:  /* Set tremolo waveform */
	  			*e = EFFECT_STe7;
	  			*o = operand & 0x0f;
	  			break;
        case 0x08:  /* Panning */
	  			*e = EFFECT_STe8;
	  			*o = operand & 0x0f;
	  			break;
        case 0x09:  /* Retrig */
	  			*e = EFFECT_STe9;
	  			*o = operand & 0x0f;
	  			break;
        case 0x0a:  /* Fine volume slide up */
	  			*e = EFFECT_STea;
	  			*o = operand & 0x0f;
	  			break;
        case 0x0b:  /* Fine volume slide down */
	  			*e = EFFECT_STeb;
	  			*o = operand & 0x0f;
	  			break;
        case 0x0c:  /* Note cut */
	  			*e = EFFECT_STec;
	  			*o = operand & 0x0f;
	  			break;
        case 0x0d:  /* Note delay */
	  			*e = EFFECT_STed;
	  			*o = operand & 0x0f;
	  			break;
        case 0x0e:  /* Pattern delay */
        	operand = operand & 0x0f;
					mod->patterns[pattern][(pline * mod->nChannels) + mod->nChannels - 1].effect[0] = EFFECT_STee;
					mod->patterns[pattern][(pline * mod->nChannels) + mod->nChannels - 1].operand[0] = operand;
	  			*e = EFFECT_NONE;
	  			*o = 0;
	  			break;
        default:
	  			*e = EFFECT_NONE;
	  			break;
      }
      break;
    case 0x0f:  /* Set speed/tempo */
			*e = EFFECT_STf0;
      break;
    case 0x10: /* Global volume */
      *e = EFFECT_XM10;
      break;
    case 0x11: /* Global volume slide */
      *e = EFFECT_XM11;
      break;
    case 0x19:	/* Panning slide */
      *e = EFFECT_XM19;
      break;
    default:
      *e = EFFECT_NONE;
      break;
  }
}

int MODFILE_SetXM(u8 *xmfile, int xmlength, MODFILE *xm) {

  int ofs = 0;
  int bak;
  int headersize;
  int nSamples = 0;
  int i;
  int sampleOfs;

  if (!MODFILE_IsXM(xmfile, xmlength))
    return -1;

  memcpy(xm->songname, &xmfile[17], 20);
  xm->songname[17] = '\0';

  ofs = 60;
  headersize = getu32(&xmfile[ofs]);
  xm->songlength = getu16(&xmfile[ofs + 4]);
  xm->restart_position = getu16(&xmfile[ofs + 6]);
  xm->nChannels = getu16(&xmfile[ofs + 8]);
  xm->nChannels++;
    xm->nPatterns = getu16(&xmfile[ofs + 10]);
  xm->nInstruments = getu16(&xmfile[ofs + 12]);
  if (getu16(&xmfile[ofs + 14]) & 1)
    xm->period_type = 1;
  else
    xm->period_type = 0;
  xm->start_tempo = getu16(&xmfile[ofs + 18]);
  xm->start_speed = getu16(&xmfile[ofs + 16]);
  memcpy(xm->playlist, &xmfile[ofs + 20], 256);

  ofs += headersize;

  /* Load patterns */
  SAFE_MALLOC(xm->patterns, sizeof(MOD_Note*) * xm->nPatterns);
  SAFE_MALLOC(xm->patternLengths, sizeof(int) * xm->nPatterns);
  /*  xm->patterns = malloc(sizeof(MOD_Note*) * xm->nPatterns);
      xm->patternLengths = malloc(sizeof(int) * xm->nPatterns);*/

  for (i = 0; i < xm->nPatterns; i++) {

    int packedSize;

    headersize = getu32(&xmfile[ofs]);
    xm->patternLengths[i] = getu16(&xmfile[ofs + 5]);
    packedSize = getu16(&xmfile[ofs + 7]);
    ofs += headersize;

    SAFE_MALLOC(xm->patterns[i], xm->patternLengths[i] * xm->nChannels * sizeof(MOD_Note));
    /*    xm->patterns[i] = malloc(xm->patternLengths[i] * xm->nChannels * sizeof(MOD_Note));*/
    MODFILE_ClearPattern(xm, i);

    if (packedSize != 0) { /* There is patterndata */

      int ppt = ofs;
      int pline, pchannel;

      for (pline = 0; pline < xm->patternLengths[i]; pline++) {

				for (pchannel = 0; pchannel < xm->nChannels - 1; pchannel++) {

	  			MOD_Note *destNote = &xm->patterns[i][(pline * xm->nChannels) + pchannel];

	  			if (xmfile[ppt] & 0x80) { /* A packed note */

	    			u8 t = xmfile[ppt++];
	    			u8 effect = 0x00;
	    			u8 operand = 0x00;

	    			if (t & 1) {

	      			u8 note = xmfile[ppt++];

	      			if (note == 97) { /* A key off */

								destNote->note = 0xfe;
	      			} else {

								note--;
								destNote->note = ((note / 12) << 4) | (note % 12);
	      			}
	    			}
	    			if (t & 2) {

	      			u8 instrument = xmfile[ppt++];
	      			destNote->instrument = instrument;
	    			}
	    			if (t & 4) {

	      			u8 volume = xmfile[ppt++];
	      			if (volume == 0)
	      				destNote->volume = 0xff;
	      			else
	      			if (volume < 0x10 || volume > 0x50) {

								destNote->volume = 0xff;
                convertXMVolume(volume, &destNote->effect[0], &destNote->operand[0]);
              } else {
	              
								destNote->volume = volume - 0x10;
							}
	    			}
	    			if (t & 8) {

	      			effect = xmfile[ppt++];
	    			}
	    			if (t & 16) {

	      			operand = xmfile[ppt++];
	    			}

	    			convertXMEffects(xm, i, pline, effect, operand, &destNote->effect[1], &destNote->operand[1]);

	  			} else { /* Not a packed note */

	    			u8 t;
	    			u8 t2;

	    			t = xmfile[ppt++];

	    			if (t == 97) { /* A key off */

	      			destNote->note = 0xfe;
	    			} else {

	      			t--;
	      			destNote->note = ((t / 12) << 4) | (t % 12);
	    			}

	    			destNote->instrument = xmfile[ppt++];

	    			t = xmfile[ppt++];
      			if (t == 0)
      				destNote->volume = 0xff;
      			else
	    			if (t < 0x10 || t > 0x50) {

	      			destNote->volume = 0xff;
              convertXMVolume(t, &destNote->effect[0], &destNote->operand[0]);
            } else {
	            
	      			destNote->volume = t - 0x10;
      			}

	    			t = xmfile[ppt++];
	    			t2 = xmfile[ppt++];
	    			convertXMEffects(xm, i, pline, t, t2, &destNote->effect[1], &destNote->operand[1]);
	  			}
				}
      }
    }
    ofs += packedSize;
  }

  /* Calculate number of samples */
  bak = ofs;
  nSamples = 0;
  for (i = 0; i < xm->nInstruments; i++) {

    int sheadersize = 0;
    int samplesininstr;
    int sampledatasize = 0;

    headersize = getu32(&xmfile[ofs]);
    samplesininstr = getu16(&xmfile[ofs + 27]);

    if (samplesininstr > 0)
      sheadersize = getu32(&xmfile[ofs + 29]);

    ofs += headersize;

    if (samplesininstr > 0) {

      int j;

      nSamples += samplesininstr;

      for (j = 0; j < samplesininstr; j++) {

	int ssize = getu32(&xmfile[ofs + 0]);
	sampledatasize += ssize;
	/*	if (ssize == 0)
		nSamples--;*/

	ofs += sheadersize;
      }
    }
    ofs += sampledatasize;
  }
  ofs = bak;

  xm->nSamples = nSamples;

  /* Load instruments */
  /*  xm->samples = malloc(xm->nSamples * sizeof(MOD_Sample));*/
  SAFE_MALLOC(xm->samples, xm->nSamples * sizeof(MOD_Sample));
  memset(xm->samples, 0, xm->nSamples * sizeof(MOD_Sample));
  /*  xm->instruments = malloc(xm->nInstruments * sizeof(MOD_Instrument));*/
  SAFE_MALLOC(xm->instruments, xm->nInstruments * sizeof(MOD_Instrument));
  memset(xm->instruments, 0, xm->nInstruments * sizeof(MOD_Instrument));

  sampleOfs = 0;

  for (i = 0; i < xm->nInstruments; i++) {

    int sheadersize = 0;
    int samplesininstr;
    int j;

    headersize = getu32(&xmfile[ofs]);
    samplesininstr = getu16(&xmfile[ofs + 27]);
    memcpy(xm->instruments[i].name, &xmfile[ofs + 4], 22);
    xm->instruments[i].name[22] = '\0';

    /* Instrument note -> Sample note mapping */
    for (j = 0; j < 256; j++) {

      xm->instruments[i].note[j] = j;
    }

    /* Sample number for all notes */
    for (j = 0; j < 256; j++) {

      xm->instruments[i].samples[j] = NULL;
    }

    if (samplesininstr > 0) {

      sheadersize = getu32(&xmfile[ofs + 29]);

      /* Volume Fade */
      xm->instruments[i].volumeFade = getu16(&xmfile[ofs + 239]);

      /* Sample number for all notes */
      for (j = 0; j < 96; j++) {

				u8 dnote = ((j / 12) << 4) | (j % 12);

				xm->instruments[i].samples[dnote] = &xm->samples[(int)(unsigned int)xmfile[ofs + 33 + j] + sampleOfs];
      }

      /* Volume envelope */
      if ((xmfile[ofs + 233] & 1) && (xmfile[ofs + 225] > 0)) {

				xm->instruments[i].envVolume.enabled = TRUE;
				xm->instruments[i].envVolume.numPoints = xmfile[ofs + 225];

				/* Looping */
				if (xmfile[ofs + 233] & 4) {

	  			xm->instruments[i].envVolume.loop_start = xmfile[ofs + 228];
	  			xm->instruments[i].envVolume.loop_end = xmfile[ofs + 229];
				} else {

	  			xm->instruments[i].envVolume.loop_start = 255;
	  			xm->instruments[i].envVolume.loop_end = 255;
				}

				/* Sustain */
				if (xmfile[ofs + 233] & 2) {

	  			xm->instruments[i].envVolume.sustain = xmfile[ofs + 227];
				} else {

	  			xm->instruments[i].envVolume.sustain = 255;
				}

				/* Envelope points */
				xm->instruments[i].envVolume.envPoints = malloc(sizeof(EnvPoint) * (int)xm->instruments[i].envVolume.numPoints);
				for (j = 0; j < xm->instruments[i].envVolume.numPoints; j++) {

	  			xm->instruments[i].envVolume.envPoints[j].x = getu16(&xmfile[ofs + 129 + (j * 4)]);
	  			xm->instruments[i].envVolume.envPoints[j].y = getu16(&xmfile[ofs + 129 + (j * 4) + 2]);
				}
      } else {

				xm->instruments[i].envVolume.enabled = FALSE;
      }

      /* Panning envelope */
      if ((xmfile[ofs + 234] & 1) && (xmfile[ofs + 226] > 0)) {

				xm->instruments[i].envPanning.enabled = TRUE;
				xm->instruments[i].envPanning.numPoints = xmfile[ofs + 226];

				/* Looping */
				if (xmfile[ofs + 234] & 4) {

	  			xm->instruments[i].envPanning.loop_start = xmfile[ofs + 231];
	  			xm->instruments[i].envPanning.loop_end = xmfile[ofs + 232];
				} else {

	  			xm->instruments[i].envPanning.loop_start = 255;
	  			xm->instruments[i].envPanning.loop_end = 255;
				}

				/* Sustain */
				if (xmfile[ofs + 234] & 2) {

	  			xm->instruments[i].envPanning.sustain = xmfile[ofs + 230];
				} else {

	  			xm->instruments[i].envPanning.sustain = 255;
				}

				/* Envelope points */
				xm->instruments[i].envPanning.envPoints = malloc(sizeof(EnvPoint) * (int)xm->instruments[i].envPanning.numPoints);
				for (j = 0; j < xm->instruments[i].envPanning.numPoints; j++) {

	  			xm->instruments[i].envPanning.envPoints[j].x = getu16(&xmfile[ofs + 177 + (j * 4)]);
	  			xm->instruments[i].envPanning.envPoints[j].y = getu16(&xmfile[ofs + 177 + (j * 4) + 2]);
				}
      } else {

				xm->instruments[i].envPanning.enabled = FALSE;
      }

      ofs += headersize; /* Go to the sample headers */

      /* Read sample headers */
      for (j = 0; j < samplesininstr; j++) {

				u8 flag = xmfile[ofs + 14];

				xm->samples[sampleOfs + j].default_volume = 64;
				xm->samples[sampleOfs + j].middle_c =
	  			xm->samples[sampleOfs + j].default_middle_c = xmgetfinefreq((int)(s8)xmfile[ofs + 13]);
        xm->samples[sampleOfs + j].finetune = (s8)xmfile[ofs + 13];

				xm->samples[sampleOfs + j].panning = xmfile[ofs + 15] >> 2;
				xm->samples[sampleOfs + j].default_volume =
	  			xm->samples[sampleOfs + j].volume = xmfile[ofs + 12];
				memcpy(xm->samples[sampleOfs + j].name, &xmfile[ofs + 18], 22);
				xm->samples[sampleOfs + j].name[22] = '\0';
				xm->samples[sampleOfs + j].relative_note = xmfile[ofs + 16];

				xm->samples[sampleOfs + j].sampleInfo.length = getu32(&xmfile[ofs + 0]);
				xm->samples[sampleOfs + j].sampleInfo.loop_start = getu32(&xmfile[ofs + 4]);
				xm->samples[sampleOfs + j].sampleInfo.loop_end = getu32(&xmfile[ofs + 8]);

				xm->samples[sampleOfs + j].sampleInfo.stereo = FALSE;
				xm->samples[sampleOfs + j].sampleInfo.bit_16 = (flag & 16) != 0;

				if (xm->samples[sampleOfs + j].sampleInfo.bit_16) {

	  			xm->samples[sampleOfs + j].sampleInfo.length /= 2;
	  			xm->samples[sampleOfs + j].sampleInfo.loop_start /= 2;
	  			xm->samples[sampleOfs + j].sampleInfo.loop_end /= 2;
				}

				flag &= 0x03;
				if ((flag == 0) || (flag == 3)) {

	  			xm->samples[sampleOfs + j].sampleInfo.looped = FALSE;
	  			xm->samples[sampleOfs + j].sampleInfo.pingpong = FALSE;
				} else if (flag == 1) {

	  			xm->samples[sampleOfs + j].sampleInfo.looped = TRUE;
	  			xm->samples[sampleOfs + j].sampleInfo.pingpong = FALSE;
				} else if (flag == 2) {

	  			xm->samples[sampleOfs + j].sampleInfo.looped = TRUE;
	  			xm->samples[sampleOfs + j].sampleInfo.pingpong = TRUE;
				}

				if (xm->samples[sampleOfs + j].sampleInfo.loop_end == 0) {

	  			xm->samples[sampleOfs + j].sampleInfo.looped = FALSE;
	  			xm->samples[sampleOfs + j].sampleInfo.pingpong = FALSE;
				}

				if (xm->samples[sampleOfs + j].sampleInfo.looped) {

	  			xm->samples[sampleOfs + j].sampleInfo.loop_end +=
	    			xm->samples[sampleOfs + j].sampleInfo.loop_start;
				} else {

	  			xm->samples[sampleOfs + j].sampleInfo.loop_start =
	    			xm->samples[sampleOfs + j].sampleInfo.loop_end =
	    				xm->samples[sampleOfs + j].sampleInfo.length - 1;
				}

				ofs += sheadersize;
			} /* for (j = 0; j < samplesininstr; j++) */

      /* Read sample data */
      for (j = 0; j < samplesininstr; j++) {

				if (xm->samples[sampleOfs + j].sampleInfo.length != 0) {

	  			u32 length = xm->samples[sampleOfs + j].sampleInfo.length;

	  			if (xm->samples[sampleOfs + j].sampleInfo.bit_16) {

	    			s16 *d;
	    			s16 old, new;
	    			u32 k;

	    			d = malloc(length * 2);
	    			memcpy(d, &xmfile[ofs], length * 2);
	    			xm->samples[sampleOfs + j].sampleInfo.sampledata = d;
	    			ofs += length * 2;

	    			/* Convert the sampledata */
	    			old = 0;
	    			for (k = 0; k < length; k++) {

	      			new = getu16((u8*)&d[k]) + old;
	      			d[k] = new;
	      			old = new;
	    			}
	  			} else {

	    			s8 * d;
	    			s8 old, new;
	    			u32 k;

	    			d = malloc(length);
	    			memcpy(d, &xmfile[ofs], length);
	    			xm->samples[sampleOfs + j].sampleInfo.sampledata = d;
	    			ofs += length;

	    			/* Convert the sampledata */
	    			old = 0;
	    			for (k = 0; k < length; k++) {

	      			new = d[k] + old;
	      			d[k] = new;
	      			old = new;
	    			}
	  			}
				}
      }

      sampleOfs += samplesininstr;

    }  else {/* if (samplesininstr > 0) */

      ofs += headersize;
    }
	} /* for (i = 0; i < xm->nInstruments; i++) */

  xm->unsigned_samples = FALSE;
  xm->master_volume = 64;
  xm->filetype = MODULE_XM;

  for (i = 0; i < xm->nChannels; i++) {

    xm->channels[i].voiceInfo.enabled = TRUE;
    xm->channels[i].default_panning = 128;
  }
  
  return 0;
}




BOOL MODFILE_IsXM(u8 *xmfile, int xmlength) {

  if ((xmfile == NULL) || (xmlength < 1024))
    return FALSE;

  if (memcmp(&xmfile[0], "Extended Module: ", 17) != 0)
    return FALSE;

  if (xmfile[37] != 0x1a)
    return FALSE;

  return TRUE;
}




int MODFILE_XMGetFormatID(void) {

  return MODULE_XM;
}


char *MODFILE_XMGetDescription(void) {

  return DESCRIPTION;
}


char *MODFILE_XMGetAuthor(void) {

  return AUTHOR;
}


char *MODFILE_XMGetVersion(void) {

  return VERSION;
}


char *MODFILE_XMGetCopyright(void) {

  return COPYRIGHT;
}
