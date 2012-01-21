/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "effects.h"
#include "modplay.h"
#include "mod.h"


#define FORMAT_ID   1
#define DESCRIPTION "Amiga Pro/Soundtracker"
#define AUTHOR      "Christian Nowak <chnowak@web.de>"
#define VERSION     "v0.03b"
#define COPYRIGHT   "Copyright (c) 2002, 2003, 2007"


#define SAFE_MALLOC(dest, a) \
  dest = malloc(a); \
  if (dest == NULL) { \
    MODFILE_Free(mod); \
    return -2; \
  }



static u32 s3m_finetunes[16] = {

  7895,7941,7985,8046,8107,8169,8232,8280,
  8363,8413,8463,8529,8581,8651,8723,8757
};



#define NUM_AMIGA_FREQS	((22 * 5) + 5)
static const struct {

  u16 amigafreq;
  u8 st3note;
} s3m_amiga2st3[] = {
  { 0x1ac0, 0x00 }, { 0x1940, 0x01 }, { 0x17d0, 0x02 }, { 0x1680, 0x03 }, { 0x1530, 0x04 },
  { 0x1400, 0x05 }, { 0x12e0, 0x06 }, { 0x11d0, 0x07 }, { 0x10d0, 0x08 }, { 0x0fe0, 0x09 },
  { 0x0f00, 0x0a }, { 0x0e2c, 0x0b }, { 0x0d60, 0x10 }, { 0x0ca0, 0x11 }, { 0x0be8, 0x12 },
  { 0x0b40, 0x13 }, { 0x0a98, 0x14 }, { 0x0a00, 0x15 }, { 0x0970, 0x16 }, { 0x08e8, 0x17 },
  { 0x0868, 0x18 }, { 0x07f0, 0x19 }, { 0x0780, 0x1a }, { 0x0716, 0x1b }, { 0x06b0, 0x20 },
  { 0x0650, 0x21 }, { 0x05f5, 0x22 }, { 0x05a0, 0x23 }, { 0x054f, 0x24 }, { 0x0503, 0x25 },
  { 0x04bb, 0x26 }, { 0x0477, 0x27 }, { 0x0436, 0x28 }, { 0x03fa, 0x29 }, { 0x03c1, 0x2a },
  { 0x0386, 0x2b }, { 0x0358, 0x30 }, { 0x0328, 0x31 }, { 0x02fa, 0x32 }, { 0x02d0, 0x33 },
  { 0x02a6, 0x34 }, { 0x0280, 0x35 }, { 0x025c, 0x36 }, { 0x023a, 0x37 }, { 0x021a, 0x38 },
  { 0x01fc, 0x39 }, { 0x01e0, 0x3a }, { 0x01c5, 0x3b }, { 0x01ac, 0x40 }, { 0x0194, 0x41 },
  { 0x017d, 0x42 }, { 0x0168, 0x43 }, { 0x0153, 0x44 }, { 0x0140, 0x45 }, { 0x012e, 0x46 },
  { 0x011d, 0x47 }, { 0x010d, 0x48 }, { 0x00fe, 0x49 }, { 0x00f0, 0x4a }, { 0x00e2, 0x4b },
  { 0x00d6, 0x50 }, { 0x00ca, 0x51 }, { 0x00be, 0x52 }, { 0x00b4, 0x53 }, { 0x00aa, 0x54 },
  { 0x00a0, 0x55 }, { 0x0097, 0x56 }, { 0x008f, 0x57 }, { 0x0087, 0x58 }, { 0x007f, 0x59 },
  { 0x0078, 0x5a }, { 0x0071, 0x5b }, { 0x006b, 0x60 }, { 0x0065, 0x61 }, { 0x005f, 0x62 },
  { 0x005a, 0x63 }, { 0x0055, 0x64 }, { 0x0050, 0x65 }, { 0x004c, 0x66 }, { 0x0047, 0x67 },
  { 0x0043, 0x68 }, { 0x003f, 0x69 }, { 0x003c, 0x6a }, { 0x0039, 0x6b }, { 0x0035, 0x70 },
  { 0x0032, 0x71 }, { 0x002f, 0x72 }, { 0x002d, 0x73 }, { 0x002a, 0x74 }, { 0x0028, 0x75 },
  { 0x0025, 0x76 }, { 0x0023, 0x77 }, { 0x0021, 0x78 }, { 0x001f, 0x79 }, { 0x001e, 0x7a },
  { 0x001c, 0x7b }, { 0x001a, 0x80 }, { 0x0019, 0x81 }, { 0x0017, 0x82 }, { 0x0016, 0x83 },
  { 0x0015, 0x84 }, { 0x0014, 0x85 }, { 0x0012, 0x86 }, { 0x0011, 0x87 }, { 0x0010, 0x88 },
  { 0x000f, 0x89 }, { 0x000f, 0x8a }, { 0x000e, 0x8b }, { 0x004b, 0x66 }, { 0x0474, 0x27 },
  { 0x0500, 0x25 }, { 0x05f4, 0x22 }, { 0x054c, 0x24 }, { 0x03f8, 0x29 },
  { 0x02b4, 0x34 }
};


static int getSamplesSize(MODFILE *mod) {
	
	int i, s;
	
	for (i = s = 0; i < mod->nSamples; i++)
		s += mod->samples[i].sampleInfo.length;
	
	return s;
}

static int calcNumOfPatterns(MODFILE *mod, int modlength) {

	int n1, n2;
	int i;
	int patternsSize = 256 * (mod->nChannels - 1);
	
  for (i = n1 = 0; i < mod->songlength; i++) {

    if (mod->playlist[i] > n1)
      n1 = mod->playlist[i];
  }
  n1++;

  n2 = modlength - getSamplesSize(mod) - 1084;
  
  return n2 % patternsSize == 0 ? n2 / patternsSize : n1;
}



/**
 * int MODFILE_SetMOD(u8 *modfile, int modlength, MODFILE *mod);
 *
 * Processes the raw data of a Protracker MOD file and copies
 * it to a structure. The structure can then be used as a handle
 * of the MOD file. The original raw data isn't needed by the
 * handle.
 *
 * Returns a value <0 on error.
 *
 * Parameters:
 * modfile   - A pointer to the raw MOD data
 * modlength - The length of the raw data in bytes
 * mod       - A pointer to the MOD handle
 **/
int MODFILE_SetMOD(u8 *modfile, int modlength, MODFILE *mod) {

  int ofs = 0;
  int i;
  int sampledatalen;
  int retval = 0;

  if (modfile == NULL || mod == NULL)
    return -1;

  mod->nInstruments = 31;

  if ( (memcmp(&modfile[1080], "M.K.", 4) == 0) ||
       (memcmp(&modfile[1080], "FLT4", 4) == 0) ) {

    mod->nChannels = 4;
  } else if (memcmp(&modfile[1080], "2CHN", 4) == 0) {

    mod->nChannels = 2;
  } else if (memcmp(&modfile[1080], "6CHN", 4) == 0) {

    mod->nChannels = 6;
  } else if (memcmp(&modfile[1080], "8CHN", 4) == 0) {

    mod->nChannels = 8;
  } else if (memcmp(&modfile[1080], "10CH", 4) == 0) {

    mod->nChannels = 10;
  } else if (memcmp(&modfile[1080], "12CH", 4) == 0) {

    mod->nChannels = 12;
  } else if (memcmp(&modfile[1080], "14CH", 4) == 0) {

    mod->nChannels = 14;
  } else if (memcmp(&modfile[1080], "16CH", 4) == 0) {

    mod->nChannels = 16;
  } else if (memcmp(&modfile[1080], "18CH", 4) == 0) {

    mod->nChannels = 18;
  } else if (memcmp(&modfile[1080], "20CH", 4) == 0) {

    mod->nChannels = 20;
  } else if (memcmp(&modfile[1080], "22CH", 4) == 0) {

    mod->nChannels = 22;
  } else if (memcmp(&modfile[1080], "24CH", 4) == 0) {

    mod->nChannels = 24;
  } else if (memcmp(&modfile[1080], "26CH", 4) == 0) {

    mod->nChannels = 26;
  } else if (memcmp(&modfile[1080], "28CH", 4) == 0) {

    mod->nChannels = 28;
  } else if (memcmp(&modfile[1080], "30CH", 4) == 0) {

    mod->nChannels = 30;
  } else if (memcmp(&modfile[1080], "32CH", 4) == 0) {

    mod->nChannels = 32;
  } else {

    mod->nInstruments = 15;
    mod->nChannels = 4;
  }

  mod->nChannels++;	/* Global fx channel */
  mod->nSamples = mod->nInstruments;  /* The MOD format doesn't support multisamples */

  /* 0 */
  memcpy(mod->songname, &modfile[ofs], 20);
  ofs += 20;
  /* Instruments */
  /* 20 */
  /*  mod->instruments = malloc(mod->nInstruments * sizeof(MOD_Instrument));*/
  SAFE_MALLOC(mod->instruments, mod->nInstruments * sizeof(MOD_Instrument));
  memset(mod->instruments, 0, mod->nInstruments * sizeof(MOD_Instrument));
  /*  mod->samples = malloc(mod->nSamples * sizeof(MOD_Sample));*/
  SAFE_MALLOC(mod->samples, mod->nSamples * sizeof(MOD_Sample));
  memset(mod->samples, 0, mod->nSamples * sizeof(MOD_Sample));

  for (i = 0; i < mod->nInstruments; i++) {

    int temp, j;

    /* Name */
    memcpy(mod->samples[i].name, &modfile[ofs], 22);
    ofs += 22;
    /* Length */
    temp  = modfile[ofs++] << 8;
    temp |= modfile[ofs++];
    temp *= 2;
    mod->samples[i].sampleInfo.length = temp;
    /* Fine tune */
    temp = modfile[ofs++];
    if (temp > 7) temp -= 16;
    temp += 8;
    mod->samples[i].default_middle_c = s3m_finetunes[temp];
    /* Volume */
    mod->samples[i].default_volume = modfile[ofs++];
    /* Loop start */
    temp  = modfile[ofs++] << 8;
    temp |= modfile[ofs++];
    temp *= 2;
    mod->samples[i].sampleInfo.loop_start = temp;
    /* Loop end */
    temp  = modfile[ofs++] << 8;
    temp |= modfile[ofs++];
    temp *= 2;
    mod->samples[i].sampleInfo.loop_end = mod->samples[i].sampleInfo.loop_start + temp;

    mod->samples[i].panning = 255;
    mod->samples[i].sampleInfo.bit_16 = FALSE;
    mod->samples[i].sampleInfo.stereo = FALSE;
    mod->samples[i].sampleInfo.pingpong = FALSE;
    mod->samples[i].relative_note = 0;

    mod->samples[i].sampleInfo.looped = TRUE;
    if (temp <= 2) {

      mod->samples[i].sampleInfo.looped = FALSE;
      mod->samples[i].sampleInfo.loop_start = mod->samples[i].sampleInfo.loop_end =
			mod->samples[i].sampleInfo.length - 1;
    }

    /* Define a new instrument */
    strcpy(mod->instruments[i].name, mod->samples[i].name);
    for (j = 0; j < 256; j++) {

      mod->instruments[i].samples[j] = &mod->samples[i];
      mod->instruments[i].note[j] = j;
    }

    /* Disable instrument envelopes */
    mod->instruments[i].envPanning.enabled = FALSE;
    mod->instruments[i].envVolume.enabled = FALSE;
    mod->instruments[i].volumeFade = 32767;
  }

  /* Song length */
  mod->songlength = modfile[ofs++];
  /* CIAA speed */
  ofs++;
  /* Arrangement */
  memcpy(mod->playlist, &modfile[ofs], 128);
  ofs += 128;
  /* I.D. */
  if (mod->nInstruments != 15)
    ofs += 4;

  /* Calculate number of patterns */
  mod->nPatterns = calcNumOfPatterns(mod, modlength);
/*  for (i = mod->nPatterns = 0; i < mod->songlength; i++) {

    if (mod->playlist[i] > mod->nPatterns)
      mod->nPatterns = mod->playlist[i];
  }

  mod->nPatterns++;*/

  /* Extract the patterns */
  /*  mod->patterns = malloc(sizeof(MOD_Note*) * mod->nPatterns);
      mod->patternLengths = malloc(sizeof(int) * mod->nPatterns);*/
  SAFE_MALLOC(mod->patterns, sizeof(MOD_Note*) * mod->nPatterns);
  SAFE_MALLOC(mod->patternLengths, sizeof(int) * mod->nPatterns);

  for (i = 0; i < mod->nPatterns; i++) {

    int pline, pchannel;
    u8 * curPattern;

    mod->patternLengths[i] = 64;

    /* Alloc mem for current pattern */
    /*    mod->patterns[i] = malloc(sizeof(MOD_Note) * mod->nChannels * 64);*/
    SAFE_MALLOC(mod->patterns[i], sizeof(MOD_Note) * mod->nChannels * 64);
    memset(mod->patterns[i], 255, sizeof(MOD_Note) * mod->nChannels * 64);

    /* Convert MOD pattern to our format */
    curPattern = &modfile[ofs];
    for (pline = 0; pline < 64; pline++) {

      u8 *curLine = &curPattern[(mod->nChannels - 1) * 4 * pline];
			MOD_Note *globalEffect = &mod->patterns[i][(pline * mod->nChannels) + mod->nChannels - 1];

      for (pchannel = 0; pchannel < mod->nChannels - 1; pchannel++) {

	int j;
	u8 *curNote = &curLine[4 * pchannel];
	u16 note;
	u8 dnote;
	u32 instrument;
	u16 effect;
	u8 operand;
	u8 volume;

	note  = (curNote[0] & 0x0f) << 8;
	note |= curNote[1];
	instrument  = curNote[0] & 0xf0;
	instrument |= (curNote[2] & 0xf0) >> 4;
	effect = curNote[2] & 0x0f;
	operand = curNote[3];
	volume = 255;

	/* Convert note */
	dnote = 0xff;
	if (note != 0) {

	  for (j = 0; j < NUM_AMIGA_FREQS; j++) {

	    if (s3m_amiga2st3[j].amigafreq == note)
	      dnote = s3m_amiga2st3[j].st3note;
	  }
	}

	if ((dnote == 0xff) && (note != 0)) { /* Note not found */

  	fprintf(stderr, "Note not found: %x\n", note);

	  retval = 1;
	}

	/* Convert effect */
	switch (effect) {

	  case 0x00:  /* Arpeggio */
	    if (operand != 0)
	      effect = EFFECT_ST00;
	    else
	      effect = EFFECT_NONE;
	    break;
	  case 0x01:  /* Porta up */
	    effect = EFFECT_ST10;
	    break;
	  case 0x02:  /* Porta down */
	    effect = EFFECT_ST20;
	    break;
	  case 0x03:  /* Porta to note */
	    effect = EFFECT_ST30;
	    break;
	  case 0x04:  /* Vibrato */
	    effect = EFFECT_ST40;
	    break;
	  case 0x05:  /* Porta + volume slide */
	    effect = EFFECT_ST50;
	    break;
	  case 0x06:  /* Vibrato + volume slide */
	    effect = EFFECT_ST60;
	    break;
	  case 0x07:  /* Tremolo */
	    effect = EFFECT_ST70;
	    break;
	  case 0x09:  /* Sample offset */
	    effect = EFFECT_ST90;
	    break;
	  case 0x0a:  /* Volume slide */
	    effect = EFFECT_STa0;
	    break;
	  case 0x0b:  /* Pattern jump */
			globalEffect->effect[0] = EFFECT_STb0;
			globalEffect->operand[0] = operand;
	  	effect = EFFECT_NONE;
	  	operand = 0;
	    break;
	  case 0x0c:  /* Set volume */
	    effect = EFFECT_NONE;
	    if (operand > 64)
	    	operand = 64;
	    volume = operand;
	    break;
	  case 0x0d:  /* Pattern break */
	    operand = ((operand >> 4) * 10) + (operand & 0x0f);
	    if (operand >= 64)
	      operand = 0;

	    if (globalEffect->effect[0] == EFFECT_STb0) {
		    
		    globalEffect->effect[0] = EFFECT_STg0;
		    globalEffect->operand[1] = operand;
	    } else {
		    
				globalEffect->effect[0] = EFFECT_STd0;
				globalEffect->operand[0] = operand;
  		}
  		effect = EFFECT_NONE;
  		operand = 0;
	    break;
	  case 0x0e:
	    switch (operand >> 4) {

	      int temp;

	      case 0x01:  /* Fine porta up */
					effect = EFFECT_STe1;
					operand = operand & 0x0f;
					break;
	      case 0x02:  /* Fine porta down */
					effect = EFFECT_STe2;
					operand = operand & 0x0f;
					break;
	      case 0x04:  /* Set vibrato waveform */
					effect = EFFECT_STe4;
					operand = operand & 0x0f;
					break;
	      case 0x05:  /* Set finetune */
					effect = EFFECT_STe5;
					temp = operand & 0x0f;
					if (temp > 7) temp -= 16;
						temp += 8;
					operand = operand & 0x0f;
					break;
	      case 0x06:  /* Pattern loop */
					operand = operand & 0x0f;
					globalEffect->effect[0] = EFFECT_STe6;
					globalEffect->operand[0] = operand;
	  			effect = EFFECT_NONE;
	  			operand = 0;
					break;
	      case 0x07:  /* Set tremolo waveform */
					effect = EFFECT_STe7;
					operand = operand & 0x0f;
					break;
	      case 0x08:  /* Panning */
					effect = EFFECT_STe8;
					operand = operand & 0x0f;
					break;
	      case 0x09:  /* Retrig */
					effect = EFFECT_STe9;
					operand = operand & 0x0f;
					break;
	      case 0x0a:  /* Fine volume slide up */
					effect = EFFECT_STea;
					operand = operand & 0x0f;
					break;
	      case 0x0b:  /* Fine volume slide down */
					effect = EFFECT_STeb;
					operand = operand & 0x0f;
					break;
	      case 0x0c:  /* Note cut */
					effect = EFFECT_STec;
					operand = operand & 0x0f;
					break;
	      case 0x0d:  /* Note delay */
					effect = EFFECT_STed;
					operand = operand & 0x0f;
					break;
	      case 0x0e:  /* Pattern delay */
					operand = operand & 0x0f;
					globalEffect->effect[0] = EFFECT_STee;
					globalEffect->operand[0] = operand;
	  			effect = EFFECT_NONE;
	  			operand = 0;
					break;
	      default:
					effect = EFFECT_NONE;
		break;
	    }
	    break;
	  case 0x0f:  /* Set speed/tempo */
/*	    if (operand < 32)
	      effect = 'A' - 'A' + 1;
	    else
	      effect = 'T' - 'A' + 1;*/
	      effect = EFFECT_STf0;
	    break;
	  default:
	    effect = EFFECT_NONE;
	    break;
	}

	j = (pline * mod->nChannels) + pchannel;
	mod->patterns[i][j].note = dnote;
	mod->patterns[i][j].instrument = instrument;
	mod->patterns[i][j].volume = volume;
	mod->patterns[i][j].effect[0] = effect;
	mod->patterns[i][j].operand[0] = operand;
      }
    }

    ofs += 4 * (mod->nChannels - 1) * 64;
  }

  /* Sample data */
  for (i = sampledatalen = 0; i < mod->nInstruments; i++) {

    sampledatalen += mod->samples[i].sampleInfo.length;
  }

  if ((sampledatalen + 1084 + (mod->nPatterns * 64 * (mod->nChannels - 1) * 4)) != modlength) {

    ofs = modlength - sampledatalen;
  }

  for (i = 0; i < mod->nInstruments; i++) {

    mod->samples[i].sampleInfo.sampledata = NULL;
    if (mod->samples[i].sampleInfo.length != 0) {

      /*      mod->samples[i].sampleInfo.sampledata = malloc(mod->samples[i].sampleInfo.length);*/
      SAFE_MALLOC(mod->samples[i].sampleInfo.sampledata, mod->samples[i].sampleInfo.length);
      memcpy(mod->samples[i].sampleInfo.sampledata, &modfile[ofs], mod->samples[i].sampleInfo.length);
      ofs += mod->samples[i].sampleInfo.length;
    }
  }

  for ( i = 0; i < mod->nChannels; i++) {

    mod->channels[i].voiceInfo.enabled = TRUE;
    mod->channels[i].default_panning = (((i - 1) >> 1) & 1) ^ 1 ? 86 : 167;
  }

  mod->start_speed = 6;
  mod->start_tempo = 125;
  mod->master_volume = 64;

  mod->filetype = MODULE_MOD;

  return retval;
}




/**
 * BOOL MODFILE_IsMOD(u8 *modfile, int modlength);
 *
 * Checks whether the raw data in memory is a valid
 * Protracker MOD file.
 *
 * Returns TRUE if the data is a Protracker MOD,
 * FALSE if not.
 *
 * Parameters:
 *
 * modfile   - Pointer to the raw data to be checked
 * modlength - Length of the raw data in bytes
 **/
BOOL MODFILE_IsMOD(u8 *modfile, int modlength) {

  MODFILE temp;
  int ret;

  if ((modfile == NULL) || (modlength < 1080))
    return FALSE;

  MODFILE_Init(&temp);

  if ((ret = MODFILE_SetMOD(modfile, modlength, &temp)) >= 0) {

    temp.set = TRUE;
    MODFILE_Free(&temp);
  }

  return ret == 0;
}




int MODFILE_MODGetFormatID(void) {

  return MODULE_MOD;
}

char *MODFILE_MODGetDescription(void) {

  return DESCRIPTION;
}

char *MODFILE_MODGetAuthor(void) {

  return AUTHOR;
}

char *MODFILE_MODGetVersion(void) {

  return VERSION;
}

char *MODFILE_MODGetCopyright(void) {

  return COPYRIGHT;
}
