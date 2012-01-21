/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modplay.h"
#include "s3m.h"
#include "mod.h"
#include "xm.h"


const MODFORMAT mod_formats[] = {

  { MODFILE_SetS3M,
    MODFILE_IsS3M,
    MODFILE_S3MGetFormatID,
    MODFILE_S3MGetDescription,
    MODFILE_S3MGetAuthor,
    MODFILE_S3MGetVersion,
    MODFILE_S3MGetCopyright
  },

  { MODFILE_SetXM,
    MODFILE_IsXM,
    MODFILE_XMGetFormatID,
    MODFILE_XMGetDescription,
    MODFILE_XMGetAuthor,
    MODFILE_XMGetVersion,
    MODFILE_XMGetCopyright
  },

  { MODFILE_SetMOD,
    MODFILE_IsMOD,
    MODFILE_MODGetFormatID,
    MODFILE_MODGetDescription,
    MODFILE_MODGetAuthor,
    MODFILE_MODGetVersion,
    MODFILE_MODGetCopyright
  },

  { NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};




/**
 * void *loadFile(const char *fname, int *filelength);
 *
 * Tries to load a file specified by the fname parameter
 * into a chunk of memory which the function allocates
 * with the malloc() function prior to loading.
 *
 * Returns NULL on error and the address of the file in
 * memory on success.
 *
 * Parameters:
 * fname      - The file to be loaded
 * filelength - The length of the file in bytes is
 *              written to this location on success.
 **/
static void *loadFile(const char *fname, int *filelength) {

  FILE *fhandle;
  void *file;

  fhandle = fopen(fname, "rb");

  if (fhandle == NULL)
    return NULL;

  fseek(fhandle, 0, SEEK_END);
  (*filelength) = ftell(fhandle);
  fseek(fhandle, 0, SEEK_SET);

  file = malloc(*filelength);
  if (file == NULL) {

    fclose(fhandle);
    return NULL;
  }

  fread(file, 1, *filelength, fhandle);
  fclose(fhandle);

  return file;
}




/**
 * BOOL MODFILE_Is(u8 *modfile, int modlength);
 *
 * Checks whether the raw data in memory is a valid
 * MOD file of any supported format.
 *
 * Returns TRUE if the data is of any supported
 * format, FALSE if not.
 *
 * Parameters:
 *
 * modfile   - Pointer to the raw data to be checked
 * modlength - Length of the raw data in bytes
 **/
BOOL MODFILE_Is(u8 *modfile, int modlength) {

  int i = 0;

  while ((mod_formats[i].is != NULL) &&
	 (mod_formats[i].set != NULL)) {

    if (mod_formats[i].is(modfile, modlength))
      return TRUE;

    i++;
  }

  return FALSE;
}




/**
 * int MODFILE_Set(u8 *modfile, int modlength, MODFILE *mod);
 *
 * Processes the raw data of a MOD file of any supported format
 * and copies it to a structure. The structure can then be used
 * as a handle of the MOD file. The original raw data isn't
 * needed by the handle.
 * The function works non-destructive if it fails, ie. it doesn't
 * alter any data in the handle.
 *
 * Returns a value <0 on error.
 *
 * Parameters:
 * modfile   - A pointer to the raw MOD data
 * modlength - The length of the raw data in bytes
 * mod       - A pointer to the MOD handle
 **/
int MODFILE_Set(u8 *modfile, int modlength, MODFILE *mod) {

  int i = 0, retval;

  if ((mod == NULL) || (modfile == NULL) || (modlength <= 0))
    return -1;

  if (mod->set)
    return -1;

  while ((mod_formats[i].set != NULL) &&
	 (mod_formats[i].is != NULL)) {

    if (mod_formats[i].is(modfile, modlength)) {

      if ((retval = mod_formats[i].set(modfile, modlength, mod)) >= 0) {

				mod->set = TRUE;
				return 0;
      } else {

	return retval;
      }
    }

    i++;
  }

  return -1;
}




/**
 * int MODFILE_Load(const char *fname, MODFILE *mod);
 *
 * Loads a MOD file of any supported format and copies
 * it to a MODFILE structure. The structure can then
 * be used as a handle of the module.
 *
 * Returns <0 on error.
 *
 * Parameters:
 *
 * fname - Name of the file to be loaded
 * mod   - Pointer to the MODFILE structure
 **/
int MODFILE_Load(const char *fname, MODFILE *mod) {

  u8 *modfile = NULL;
  int modlength = 0;
  int ret;

  if ((fname == NULL) || (mod == NULL))
    return -1;

  modfile = loadFile(fname, &modlength);
  if (modfile == NULL)
    return -1;

  ret = MODFILE_Set(modfile, modlength, mod);

  free(modfile);

  return ret;
}




/**
 * void MODFILE_Start(MODFILE *mod);
 *
 * Resets all runtime-data in the MODFILE structure,
 * prepares the music for playback and allocates
 * a mixing buffer.
 *
 * Parameters:
 * mod - A pointer too the module handle
 **/
void MODFILE_Start(MODFILE *mod) {

  int i;

  if (mod == NULL)
    return;

  mod->speed = mod->start_speed;
  MODFILE_SetBPM(mod, mod->start_tempo);
  mod->pattern_line = 0;
  mod->play_position = 0;

  mod->patterndelay = 0;
  mod->samplescounter = 0;
  mod->speedcounter = 0;

  mod->patternloop_to = 0;
  mod->patternloop_count = 0;

  mod->cur_master_volume = mod->master_volume;

  mod->tempmixbuf = malloc(MODFILE_BPM2SamplesPerTick(mod, 32) * sizeof(s32) * 2);

  for (i = 0; i < mod->nSamples; i++) {

    mod->samples[i].middle_c = mod->samples[i].default_middle_c;
  }

  for (i = 0; i < MODPLAY_MAX_CHANNELS; i++) {

    int c;

    mod->channels[i].voiceInfo.playing = FALSE;
    mod->channels[i].voiceInfo.volume = 64;
    mod->channels[i].last_instrument = 0;

    for (c = 0; c < MODPLAY_NUM_COMMANDS; c++) {

      mod->channels[i].effects[c].cur_effect = 255;
      mod->channels[i].effects[c].cur_operand = 255;
      mod->channels[i].effects[c].vibrato_wave = 0;
      mod->channels[i].effects[c].tremolo_wave = 0;
      mod->channels[i].effects[c].tremolo_sintabpos = 0;
      mod->channels[i].effects[c].vibrato_sintabpos = 0;
    }

    mod->channels[i].voiceInfo.panning = mod->channels[i].default_panning;
  }
  mod->playing = TRUE;
}




/**
 * void MODFILE_Stop(MODFILE *mod);
 *
 * Stops music playback and deallocates the
 * mixing buffer.
 **/
void MODFILE_Stop(MODFILE *mod) {

  if (mod->tempmixbuf != NULL) {

    free(mod->tempmixbuf);
    mod->tempmixbuf = NULL;
    mod->playing = FALSE;
  }
}




/**
 * void MODFILE_Player(MODFILE *mod);
 *
 * Calculates mod->mixingbuflen bytes of music data
 * in the format specified with the MODFILE_SetFormat()
 * format and stores the resulting data in the memory
 * pointed to by mod->mixingbuf.
 *
 * Parameters:
 *
 * mod - A pointer to the MODFILE structure which defines
 *       the music to be calculated.
 **/
void MODFILE_Player(MODFILE *mod) {

  int len = mod->mixingbuflen;
  int remain, l;
  int mixflags;
  u32 retval = 0;
  u8 *buf8 = (u8*)mod->mixingbuf;

  if (mod->mixchannels == 2)
    len >>= 1;
  if (mod->bits == 16)
    len >>= 1;

  mixflags = 0;
  mixflags |= MIXER_USE_S32;
  if (mod->mixchannels == 2)
    mixflags |= MIXER_DEST_STEREO;
  if (mod->bits == 16)
    mixflags |= MIXER_DEST_16BIT;
  if (mod->mixsigned)
    mixflags |= MIXER_DEST_SIGNED;
  remain = len;
  l = 0;

  do {

    int tick_remain = mod->samplespertick - mod->samplescounter;
    int res;

    res = MODFILE_Mix(mod, mixflags, &buf8[mix_destbufsize(mixflags) * l], tick_remain <= remain ? tick_remain : remain);

    l += res;
    remain -= res;

    mod->samplescounter += res;

    if (mod->samplescounter >= mod->samplespertick) {

      mod->samplescounter -= mod->samplespertick;
      mod->speedcounter++;

      if (mod->speedcounter >= (mod->speed + mod->patterndelay)) {

				mod->patterndelay = 0;

				retval |= MODFILE_Process(mod);

				mod->speedcounter = 0;
     	}
      retval |= MODFILE_EffectHandler(mod);
    }
  } while (remain > 0);

  mod->notebeats = retval;

  if (mod->callback != NULL)
    mod->callback(mod);
}




/**
 * void MODFILE_Free(MODFILE *mod);
 *
 * Deallocates all resources occupied by the module
 * in the MODFILE structure after they have been
 * allocated by the MODFILE_Load() or any of the
 * MODFILE_Set*() functions.
 *
 * Parameters:
 *
 * mod - A pointer to the MODFILE structure of which
 *       the resource shall be deallocated
 **/
void MODFILE_Free(MODFILE *mod) {

  int i;

  if (mod == NULL)
    return;

  if (!mod->set)
    return;

  /* Free patterns */
  if (mod->patterns != NULL) {

    for (i = 0; i < mod->nPatterns; i++) {

      if (mod->patterns[i] != NULL) {

	free(mod->patterns[i]);
	mod->patterns[i] = NULL;
      }
    }
    free(mod->patterns);
    mod->patterns = NULL;
  }

  /* Free instruments */
  if (mod->instruments != NULL) {

    for (i = 0; i < mod->nInstruments; i++) {

      if (mod->instruments[i].envVolume.envPoints != NULL) {

	free(mod->instruments[i].envVolume.envPoints);
	mod->instruments[i].envVolume.envPoints = NULL;
      }

      if (mod->instruments[i].envPanning.envPoints != NULL) {

	free(mod->instruments[i].envPanning.envPoints);
	mod->instruments[i].envPanning.envPoints = NULL;
      }
    }
    free(mod->instruments);
    mod->instruments = NULL;
  }

  /* Free samples */
  if (mod->samples != NULL ) {

    for (i = 0; i < mod->nSamples; i++) {

      if (mod->samples[i].sampleInfo.sampledata != NULL) {

	free(mod->samples[i].sampleInfo.sampledata);
	mod->samples[i].sampleInfo.sampledata = NULL;
      }
    }
    free(mod->samples);
    mod->samples = NULL;
  }

  if (mod->patternLengths != NULL) {

    free(mod->patternLengths);
    mod->patternLengths = NULL;
  }

  mod->set = FALSE;
}




/**
 * void MODFILE_Init(MODFILE *mod);
 *
 * Initializes a MODFILE structure for usage. Must
 * be called before the structure can be used by
 * any other function.
 *
 * Parameters:
 *
 * mod - A pointer to the MODFILE structure
 **/
void MODFILE_Init(MODFILE *mod) {

  if (mod == NULL)
    return;

  memset(mod, 0, sizeof(MODFILE));
}




/**
 * void MODFILE_SetFormat(MODFILE *mod, int freq, int channels, int bits, BOOL mixsigned);
 *
 * Sets the format of the output audio stream. Must
 * be called prior to calling MODFILE_Start() and
 * MODFILE_Player().
 *
 * Parameters:
 *
 * mod       - A pointer to the MODFILE structure of
 *             which the output format shall be changed
 * freq      - Output frequency. Common values are
 *             11025Hz, 22050Hz and 44100Hz
 * channels  - 1 for mono and 2 for stereo
 * bits      - 8 or 16 are valid values
 * mixsigned - TRUE if the output stream shall consist
 *             of signed values
 **/
void MODFILE_SetFormat(MODFILE *mod, int freq, int channels, int bits, BOOL mixsigned) {

  mod->playfreq = freq;
  if ((channels == 1) || (channels == 2))
    mod->mixchannels = channels;
  if ((bits == 8) || (bits == 16))
    mod->bits = bits;
  mod->mixsigned = mixsigned;

  if (mod->playing) {

    if (mod->tempmixbuf != NULL) {

      free(mod->tempmixbuf);
      mod->tempmixbuf = malloc(MODFILE_BPM2SamplesPerTick(mod,32) * sizeof(s32) * 2);
    }
    MODFILE_SetBPM(mod, mod->bpm);
  }
}


int MODFILE_AllocSFXChannels(MODFILE *mod, int nChannels) {

  int i;

  if (mod == NULL || nChannels < 0 || nChannels > 32)
    return -1;

  if (mod->nChannels + nChannels > MODPLAY_MAX_CHANNELS)
    return -1;

  mod->nSFXChannels = nChannels;

  for (i = mod->nChannels; i < mod->nChannels + mod->nSFXChannels; i++) {

    mod->channels[i].voiceInfo.enabled = TRUE;
    printf("%d\n", i);
  }

  return 0;
}

MOD_Instrument *MODFILE_MakeInstrument(void *rawData, int nBytes, int nBits) {

  MOD_Instrument *instr;
  MOD_Sample *smpl;
  int shiftVal = nBits == 16 ? 1 : 0;
  int i;

  if (rawData == NULL || nBytes <= 0 || (nBits != 8 && nBits != 16))
    return NULL;

  instr = malloc(sizeof(MOD_Instrument));
  if (instr == NULL)
    return NULL;
  memset(instr, 0, sizeof(MOD_Instrument));

  smpl = malloc(sizeof(MOD_Sample));
  if (smpl == NULL) {

    free(instr);
    return NULL;
  }
  memset(smpl, 0, sizeof(MOD_Sample));

  for (i = 0; i < 256; i++) {

    instr->samples[i] = smpl;
    instr->note[i] = i;
  }

  instr->name[0] = '\0';
  instr->volumeFade = 4096;

  instr->envVolume.sustain = 255;
  instr->envVolume.loop_start = 255;
  instr->envVolume.loop_end = 255;

  instr->envPanning.sustain = 255;
  instr->envPanning.loop_start = 255;
  instr->envPanning.loop_end = 255;

  smpl->name[0] = '\0';
  smpl->default_volume = 64;
  smpl->middle_c = 8363;
  smpl->default_middle_c = 8363;
  smpl->finetune = 0;
  smpl->relative_note = 0;
  smpl->panning = 32;
  smpl->volume = 64;

  smpl->sampleInfo.length = nBytes >> shiftVal;
  smpl->sampleInfo.loop_start = 0;
  smpl->sampleInfo.loop_end = (nBytes >> shiftVal) - 1;
  smpl->sampleInfo.looped = FALSE;
  smpl->sampleInfo.pingpong = FALSE;
  smpl->sampleInfo.sampledata = rawData;
  smpl->sampleInfo.bit_16 = nBits == 16;
  smpl->sampleInfo.stereo = FALSE;

  return instr;
}

void MODFILE_TriggerSFX(MODFILE *mod, MOD_Instrument *instr, int channel, u8 note) {

  u8 oct, sem;
  u8 dnote;
  int sfxchan;

  if (mod == NULL || instr == NULL)
    return;

  if (channel < 0 || channel >= mod->nSFXChannels)
    return;

  oct = note >> 4;
  sem = note & 0x0f;

  if (sem >= 12)
    return;

  sfxchan = mod->nChannels + channel;

  mod->channels[sfxchan].voiceInfo.playpos = 0;
  mod->channels[sfxchan].voiceInfo.forward = TRUE;
  mod->channels[sfxchan].instrument = instr;
  mod->channels[sfxchan].sample = instr->samples[note];
  mod->channels[sfxchan].voiceInfo.sampleInfo =
    &mod->channels[sfxchan].sample->sampleInfo;

  dnote = ((note >> 4) * 12) + (note & 0x0f);
  dnote += mod->channels[sfxchan].sample->relative_note;
  dnote = ((dnote / 12) << 4) | (dnote % 12);
  MODFILE_SetNote(mod, sfxchan,
                  mod->channels[sfxchan].instrument->note[dnote],
                  mod->channels[sfxchan].sample->middle_c,
                  mod->channels[sfxchan].sample->finetune);
  mod->channels[sfxchan].cur_note = dnote;

  mod->channels[sfxchan].voiceInfo.volume = mod->channels[sfxchan].sample->default_volume;
  mod->channels[sfxchan].envVolume.envConfig = &mod->channels[sfxchan].instrument->envVolume;
  EnvTrigger(&mod->channels[sfxchan].envVolume);
  mod->channels[sfxchan].envPanning.envConfig = &mod->channels[sfxchan].instrument->envPanning;
  EnvTrigger(&mod->channels[sfxchan].envPanning);
  mod->channels[sfxchan].volumeFade = 32768;
  mod->channels[sfxchan].volumeFadeDec = 0;

  mod->channels[sfxchan].voiceInfo.playing = TRUE;

  mod->channels[sfxchan].voiceInfo.panning = 128;
}
