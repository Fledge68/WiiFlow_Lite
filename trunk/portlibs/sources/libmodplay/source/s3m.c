/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include <stdlib.h>
#include <string.h>

#include "modplay.h"
#include "s3m.h"
#include "effects.h"


#define DESCRIPTION "Future Crew Screamtracker 3"
#define AUTHOR      "Christian Nowak <chnowak@web.de>"
#define VERSION     "v0.01b"
#define COPYRIGHT   "Copyright (c) 2003, 2007"


#define SAFE_MALLOC(dest, a) \
  dest = malloc(a); \
  if (dest == NULL) { \
    MODFILE_Free(s3m); \
    return -2; \
  }


/**
 * int MODFILE_SetS3M(u8 *modfile, int modlength, MODFILE *mod);
 *
 * Processes the raw data of an S3M file and copies it to a
 * structure. The structure can then be used as a handle
 * of the S3M file. The original raw data isn't needed by the
 * handle.
 *
 * Returns a value <0 on error.
 *
 * Parameters:
 * modfile   - A pointer to the raw S3M data
 * modlength - The length of the raw data in bytes
 * mod       - A pointer to the S3M handle
 **/
int MODFILE_SetS3M(u8 *s3mfile, int s3mlength, MODFILE *s3m) {

  int ofs = 0;
  int i;
  BOOL panning_present = FALSE;
  int remapChannels[MODPLAY_MAX_CHANNELS];

  if (!MODFILE_IsS3M(s3mfile, s3mlength) || (s3m == NULL))
    return -1;

  /* 0 */
  memcpy(s3m->songname, &s3mfile[ofs], 28);
  ofs+= 28;
  /* 1c */
  ofs++;
  /* 1d */
  if (s3mfile[ofs++] != 0x10) {

    return -1;
  }

  ofs += 2;

  /* 20 */
  s3m->songlength  = s3mfile[ofs++];
  s3m->songlength |= s3mfile[ofs++] << 8;

  /* 22 */
  s3m->nInstruments  = s3mfile[ofs++];
  s3m->nInstruments |= s3mfile[ofs++] << 8;
  s3m->nSamples = s3m->nInstruments; /* S3M doesn't have multisamples */

  /* 24 */
  s3m->nPatterns  = s3mfile[ofs++];
  s3m->nPatterns |= s3mfile[ofs++] << 8;

  /* 26 */
  s3m->st2_vibrato      = (s3mfile[ofs] & 0x02) != 0;
  s3m->st2_tempo        = (s3mfile[ofs] & 0x04) != 0;
  s3m->amiga_sliding    = (s3mfile[ofs] & 0x08) != 0;
  s3m->optimize_vols    = (s3mfile[ofs] & 0x10) != 0;
  s3m->amiga_boundaries = (s3mfile[ofs] & 0x20) != 0;
  s3m->enable_sfx       = (s3mfile[ofs] & 0x40) != 0; /* Fast volume slides */
  ofs += 2;

  /* 28 */
  s3m->tracker_version = (s3mfile[ofs] << 8) | s3mfile[ofs + 1];
  ofs += 2;

  /* 2a */
  s3m->unsigned_samples = (s3mfile[ofs] == 2);
  ofs += 2;

  /* 2c */
  if (memcmp(&s3mfile[ofs], "SCRM", 4) != 0) {

    return -1;
  }
  ofs += 4;

  /* 30 */
  s3m->master_volume = s3mfile[ofs++];
  /* 31 */
  s3m->start_speed = s3mfile[ofs++];
  /* 32 */
  s3m->start_tempo = s3mfile[ofs++];
  /* 33 */
  ofs++;
  /* 34 */
  ofs++;
  /* 35 */
  panning_present = (s3mfile[ofs++] == 252);
  /* 36 */
  ofs = 0x40;

  /* 40 */
  for (i = 0; i < MODPLAY_MAX_CHANNELS; i++) {

    s3m->channels[i].voiceInfo.enabled = FALSE;
    remapChannels[i] = 255;
  }

  for (i = s3m->nChannels = 0; i < 32; i++, ofs++) {

    if (s3mfile[ofs] < 16) {

      remapChannels[i] = s3m->nChannels;
      s3m->nChannels++;
      s3m->channels[remapChannels[i]].voiceInfo.enabled = TRUE;
      s3m->channels[remapChannels[i]].default_panning = s3mfile[ofs] < 8 ? 86 : 167;
    }
  }
  s3m->nChannels++; /* Global FX channel */

  /* 60 */
  memcpy(s3m->playlist, &s3mfile[ofs], s3m->songlength);
  ofs += s3m->songlength;

  /* Parapointers to intruments */
  /*  s3m->instruments = malloc(s3m->nInstruments * sizeof(MOD_Instrument));*/
  SAFE_MALLOC(s3m->instruments, s3m->nInstruments * sizeof(MOD_Instrument));
  memset(s3m->instruments, 0, s3m->nInstruments * sizeof(MOD_Instrument));
  /*  s3m->samples = malloc(s3m->nSamples * sizeof(MOD_Sample));*/
  SAFE_MALLOC(s3m->samples, s3m->nSamples * sizeof(MOD_Sample));
  memset(s3m->samples, 0, s3m->nSamples * sizeof(MOD_Sample));

  for (i = 0; i < s3m->nInstruments; i++, ofs += 2) {

    int instrPointer = (s3mfile[ofs] | s3mfile[ofs+1] << 8) << 4;

    s3m->instruments[i].volumeFade = 32767;
    /* 0 */
    /*    s3m->instruments[i].adlib = s3mfile[instrPointer++] != 1;*/
    if (/*!s3m->instruments[i].adlib*/s3mfile[instrPointer++] == 1) {

      int sampleseg;
      int j;
      int temp;

      s3m->samples[i].relative_note = 0;
      s3m->samples[i].panning = 255;

      /* 1 */
      /*      memcpy(s3m->instruments[i].dosname, &s3mfile[instrPointer], 12);
	      s3m->instruments[i].dosname[12] = '\0';*/
      instrPointer += 12;

      /* d */
      sampleseg = s3mfile[instrPointer + 1] |
	(s3mfile[instrPointer + 2] << 8) |
	(s3mfile[instrPointer + 0] << 16);
      sampleseg <<= 4;
      instrPointer += 3;

      /* 10 */
      s3m->samples[i].sampleInfo.length = s3mfile[instrPointer] |
	(s3mfile[instrPointer + 1] << 8) |
	(s3mfile[instrPointer + 2] << 16) |
	(s3mfile[instrPointer + 2] << 24);
      instrPointer += 4;

      /* 14 */
      s3m->samples[i].sampleInfo.loop_start = s3mfile[instrPointer] |
	(s3mfile[instrPointer + 1] << 8) |
	(s3mfile[instrPointer + 2] << 16) |
	(s3mfile[instrPointer + 3] << 24);
      instrPointer += 4;

      /* 18 */
      s3m->samples[i].sampleInfo.loop_end = s3mfile[instrPointer] |
	(s3mfile[instrPointer + 1] << 8) |
	(s3mfile[instrPointer + 2] << 16) |
	(s3mfile[instrPointer + 3] << 24);
      instrPointer += 4;

      /* 1c */
      s3m->samples[i].default_volume = s3mfile[instrPointer++];

      /* 1d */
      instrPointer++;

      /* 1e */
      /*      s3m->instruments[i].packed = (s3mfile[instrPointer++] == 1);*/
      instrPointer++;

      /* 1f */
      s3m->samples[i].sampleInfo.looped = (s3mfile[instrPointer] & 1) != 0;
      s3m->samples[i].sampleInfo.stereo = (s3mfile[instrPointer] & 2) != 0;
      s3m->samples[i].sampleInfo.bit_16 = (s3mfile[instrPointer] & 4) != 0;
      s3m->samples[i].sampleInfo.pingpong = FALSE;
      instrPointer++;

      if (!s3m->samples[i].sampleInfo.looped) {

	s3m->samples[i].sampleInfo.loop_start =
	  s3m->samples[i].sampleInfo.loop_end =
	  s3m->samples[i].sampleInfo.length - 1;
      }

      /* 20 */
      s3m->samples[i].default_middle_c = s3m->samples[i].middle_c =
	s3mfile[instrPointer] |
	(s3mfile[instrPointer + 1] << 8) |
	(s3mfile[instrPointer + 2] << 16) |
	(s3mfile[instrPointer + 3] << 24);
      instrPointer += 4;

      /* 24 */
      instrPointer += 4;

      /* 28 */
      instrPointer += 2;

      /* 2a */
      instrPointer += 2;

      /* 2c */
      instrPointer += 4;

      /* 30 */
      memcpy(s3m->instruments[i].name, &s3mfile[instrPointer], 28);
      instrPointer += 28;

      /* 4c */
      if (memcmp(&s3mfile[instrPointer], "SCRS", 4) != 0) {

	return -1;
      }

      /* Define a new instrument */
      strcpy(s3m->instruments[i].name, s3m->samples[i].name);
      for (j = 0; j < 256; j++) {

	s3m->instruments[i].samples[j] = &s3m->samples[i];
	s3m->instruments[i].note[j] = j;
      }

      /* Disable instrument envelopes */
      s3m->instruments[i].envVolume.enabled = FALSE;
      s3m->instruments[i].envPanning.enabled = FALSE;

      /* Copy sample data */
      temp = s3m->samples[i].sampleInfo.length *
	(s3m->samples[i].sampleInfo.bit_16 || s3m->samples[i].sampleInfo.stereo ? 2 : 1);

      /*      s3m->samples[i].sampleInfo.sampledata = malloc(temp);*/
      SAFE_MALLOC(s3m->samples[i].sampleInfo.sampledata, temp);
      memset(s3m->samples[i].sampleInfo.sampledata, 0, temp);

      if (s3m->samples[i].sampleInfo.bit_16) {

	u8 *di = &s3mfile[sampleseg];
	int l = s3m->samples[i].sampleInfo.length;

	for (j = 0; j < l; j++) {

	  u16 d;

	  d = di[j<<1] | di[(j << 1) + 1] << 8;
	  if (s3m->unsigned_samples)
	    d ^= 0x8000;
	  ((u16*)s3m->samples[i].sampleInfo.sampledata)[j] = d;
	}
      } else {

	u8 *di = &s3mfile[sampleseg];
	int l = s3m->samples[i].sampleInfo.length;

	for (j = 0;j < l; j++) {

	  u8 d;

	  d = di[j];
	  if (s3m->unsigned_samples)
	    d ^= 0x80;
	  ((u8*)s3m->samples[i].sampleInfo.sampledata)[j] = d;
	}
      }
    }
  }

  /* Parapointers to patterns */
  /*  s3m->patterns = malloc(s3m->nPatterns * sizeof(MOD_Note*));*/
  SAFE_MALLOC(s3m->patterns, s3m->nPatterns * sizeof(MOD_Note*));
  memset(s3m->patterns, 0, s3m->nPatterns * sizeof(MOD_Note*));
  /*  s3m->patternLengths = malloc(s3m->nPatterns * sizeof(int));*/
  SAFE_MALLOC(s3m->patternLengths, s3m->nPatterns * sizeof(int));

  for (i = 0; i < s3m->nPatterns; i++, ofs += 2) {

    int pattPointer = (s3mfile[ofs] | s3mfile[ofs + 1] << 8) << 4;
    int pattPackedLen;
    int destChannel;
    int destLine;

    s3m->patternLengths[i] = 64;
    /*    s3m->patterns[i] = malloc(sizeof(MOD_Note) * s3m->nChannels * 64);*/
    SAFE_MALLOC(s3m->patterns[i], sizeof(MOD_Note) * s3m->nChannels * 64);
    memset(s3m->patterns[i], 255, sizeof(MOD_Note) * s3m->nChannels * 64);

    pattPackedLen = s3mfile[pattPointer] |
      (s3mfile[pattPointer + 1] << 8);
    pattPointer += 2;

    destChannel = destLine = 0;

    while (1) {

      int dest;
      u8 compByte = s3mfile[pattPointer++];

      if (compByte == 0) {

				destLine++;
				if (destLine >= 64) {

	  			break;
				}
      } else {

				destChannel = remapChannels[compByte & 31];
				dest = (destLine * s3m->nChannels) + destChannel ;
				if (destChannel != 255) {

	  			/* Note/Instrument */
	  			if (compByte & 32) {

	    			s3m->patterns[i][dest].note = s3mfile[pattPointer++];
	    			s3m->patterns[i][dest].instrument = s3mfile[pattPointer++];
	  			}

	  			/* Volume byte */
	  			if (compByte & 64) {

	    			s3m->patterns[i][dest].volume = s3mfile[pattPointer++];
	  			}

	  			/* Effect number/parameter */
	  			if (compByte & 128) {

		  			u16 effect = s3mfile[pattPointer++];
		  			u8 operand = s3mfile[pattPointer++];

		  			/* Convert effect/operand to internal format */
		  			switch (effect) {
			  			
			  			case 'A' - 'A' + 1:	/* Set Speed */
			  				effect = EFFECT_S3Ma0;
			  				break;

			  			case 'B' - 'A' + 1: /* Jump To Order xy */
								s3m->patterns[i][(destLine * s3m->nChannels) + s3m->nChannels - 1].effect[0] = EFFECT_STb0;
								s3m->patterns[i][(destLine * s3m->nChannels) + s3m->nChannels - 1].operand[0] = operand;
	  						effect = EFFECT_NONE;
	  						operand = 0;
			  				break;
			  				
			  			case 'C' - 'A' + 1: /* Break Pattern */
								s3m->patterns[i][(destLine * s3m->nChannels) + s3m->nChannels - 1].effect[0] = EFFECT_STd0;
								s3m->patterns[i][(destLine * s3m->nChannels) + s3m->nChannels - 1].operand[0] = operand;
	  						effect = EFFECT_NONE;
	  						operand = 0;
			  				break;
			  				
			  			case 'D' - 'A' + 1: /* Volume Slide */
			  				effect = EFFECT_S3Md0;
			  				break;

			  			case 'E' - 'A' + 1: /* Portamento Down */
			  				effect = EFFECT_S3Me0;
			  				break;

			  			case 'F' - 'A' + 1: /* Portamento Up */
			  				effect = EFFECT_S3Mf0;
			  				break;
			  			
			  			case 'G' - 'A' + 1: /* Tone Portamento */
			  				effect = EFFECT_ST30;
			  				break;
			  			
			  			case 'H' - 'A' + 1: /* Vibrato */
			  				effect = EFFECT_ST40;
			  				break;
			  			
			  			case 'I' - 'A' + 1: /* Tremor */
			  				effect = EFFECT_NONE;
			  				break;
			  			
			  			case 'J' - 'A' + 1: /* Arpeggio */
			  				effect = EFFECT_ST00;
			  				break;
			  			
			  			case 'K' - 'A' + 1: /* Vibrato + Volume Slide */
			  				effect = EFFECT_S3Mk0;
			  				break;
			  				
			  			case 'L' - 'A' + 1: /* Tone Portamento + Volume Slide */
			  				effect = EFFECT_S3Ml0;
			  				break;
			  				
			  			case 'O' - 'A' + 1: /* Sample Offset */
			  				effect = EFFECT_ST90;
			  				break;
			  			
			  			case 'Q' - 'A' + 1: /* Retrig + Volume Slide */
			  				effect = EFFECT_S3Mq0;
			  				break;
			  			
			  			case 'R' - 'A' + 1: /* Tremolo */
			  				effect = EFFECT_S3Mr0;
			  				break;
			  			
			  			case 'S' - 'A' + 1: /* Various */
			  			
			  				switch ((operand >> 4) & 0x0f) {
				  				
				  				case 0x02: /* Set Finetune */
				  					effect = EFFECT_STe5;
				  					operand = operand & 0x0f;
				  					break;
				  				
				  				case 0x03: /* Vibrato Waveform */
				  					effect = EFFECT_STe4;
				  					operand = operand & 0x0f;
				  					break;
				  				
				  				case 0x04: /* Tremolo Waveform */
				  					effect = EFFECT_STe7;
				  					operand = operand & 0x0f;
				  					break;
				  				
				  				case 0x08: /* Panning */
				  					effect = EFFECT_STe8;
				  					operand = operand & 0x0f;
				  					break;
				  				
				  				case 0x0b: /* Pattern Loop */
										operand = operand & 0x0f;
										s3m->patterns[i][(destLine * s3m->nChannels) + s3m->nChannels - 1].effect[0] = EFFECT_STe6;
										s3m->patterns[i][(destLine * s3m->nChannels) + s3m->nChannels - 1].operand[0] = operand;
	  								effect = EFFECT_NONE;
	  								operand = 0;
				  					break;
				  				
				  				case 0x0c: /* Note Cut */
										effect = EFFECT_STec;
										operand = operand & 0x0f;
				  					break;
				  				
				  				case 0x0d: /* Note Delay */
				  					effect = EFFECT_STed;
				  					operand = operand & 0x0f;
				  					break;

				  				case 0x0e: /* Pattern Delay */
										operand = operand & 0x0f;
										s3m->patterns[i][(destLine * s3m->nChannels) + s3m->nChannels - 1].effect[0] = EFFECT_STee;
										s3m->patterns[i][(destLine * s3m->nChannels) + s3m->nChannels - 1].operand[0] = operand;
	  								effect = EFFECT_NONE;
	  								operand = 0;
				  					break;

				  				default:
				  					effect = EFFECT_NONE;
				  					break;
			  				}
			  				break;
			  				
			  			case 'T' - 'A' + 1: /* Set Tempo */
			  				effect = EFFECT_S3Mt0;
			  				break;
			  			
			  			case 'V' - 'A' + 1: /* Set Global Volume */
			  				effect = EFFECT_XM10;
			  				break;

			  			default:
			  				effect = EFFECT_NONE;
			  				break;
		  			}
		  			
	    			s3m->patterns[i][dest].effect[0] = effect;
	    			s3m->patterns[i][dest].operand[0] = operand;
	  			}
				} else {

	  			if (compByte & 32)
	    			pattPointer += 2;
	  			if (compByte & 64)
	    			pattPointer++;
	  			if (compByte & 128)
	    			pattPointer += 2;
				}
      }
    }
  }

  s3m->filetype = MODULE_S3M;
  return 0;
}




/**
 * BOOL MODFILE_IsS3M(u8 *modfile, int modlength);
 *
 * Checks whether the raw data in memory is a valid
 * S3M file.
 *
 * Returns TRUE if the data is an S3M file, FALSE
 * if not.
 *
 * Parameters:
 *
 * modfile   - Pointer to the raw data to be checked
 * modlength - Length of the raw data in bytes
 **/
BOOL MODFILE_IsS3M(u8 *modfile, int modlength) {

  if ((modfile == NULL) || (modlength < 0x30))
    return FALSE;

  return memcmp(&modfile[0x2c], "SCRM", 4) == 0;
}




int MODFILE_S3MGetFormatID(void) {

  return MODULE_S3M;
}

char *MODFILE_S3MGetDescription(void) {

  return DESCRIPTION;
}

char *MODFILE_S3MGetAuthor(void) {

  return AUTHOR;
}

char *MODFILE_S3MGetVersion(void) {

  return VERSION;
}

char *MODFILE_S3MGetCopyright(void) {

  return COPYRIGHT;
}
