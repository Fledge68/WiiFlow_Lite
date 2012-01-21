/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#ifndef __MODPLAY_CORE_H__
#define __MODPLAY_CORE_H__

#include "defines.h"
#include "mixer.h"
#include "envelope.h"

#define MODPLAY_MAX_CHANNELS 33
#define MODPLAY_NUM_COMMANDS 2

typedef struct MOD_Note {

  u32 instrument;
  u8  volume;
  u8  note;
  u16 effect[MODPLAY_NUM_COMMANDS];
  u8 operand[MODPLAY_NUM_COMMANDS];
} MOD_Note;

typedef struct MOD_ChannelEffect {

  u16 cur_effect;
  u8 cur_operand;
  u16 last_effect;

  /* Panning slide */
  u8 panslide_bak;
  /* Volume Slide */
  u8 volslide_bak;
  /* Portamento */
  u8 porta_bak;
  /* Tone portamento */
  u8 toneporta_bak;
  u32 toneporta_dest;
  /* Vibrato */
  u8 vibrato_depth;
  u8 vibrato_freq;
  u8 vibrato_bak;
  int vibrato_wave;
  u32 vibrato_base;
  int vibrato_sintabpos;
  /* Tremolo */
  u8 tremolo_depth;
  u8 tremolo_freq;
  u8 tremolo_bak;
  int tremolo_wave;
  u8 tremolo_base;
  int tremolo_sintabpos;
  /* Retrig */
  u8 retrig_count;
  u8 retrig_bak;
  /* Note delay */
/*  u8 notedelay_note;
  u8 notedelay_instrument;
  u8 notedelay_volume;
  u8 notedelay_tick;*/
  MOD_Note *notedelay_note;
  /* Note cut */
  u8 notecut_tick;
  /* Arpeggio */
  u8 arpeggio_count;
  u8 arpeggio_bak;
  u32 arpeggio_base;
  /* Offset */
  u8 offset_bak;
  /* Fine volslide */
  u8 finevolslidedown_bak;
  u8 finevolslideup_bak;
  u8 gvolslide_bak;
} MOD_ChannelEffect;

typedef struct MOD_Sample {

  MOD_SAMPLEINFO16 sampleInfo;

  char name[28];
  u8 default_volume;
  u32 middle_c;
  u32 default_middle_c;
  s8 finetune;
  s8 relative_note;
  u8 panning;
  u8 volume;
} MOD_Sample;

typedef struct MOD_Instrument {

  char name[28];
  MOD_Sample *samples[256];  /* Instrument note -> Sample number mapping */
  u8  note[256];     /* Instrument note -> Sample note mapping */

  EnvelopeConfig envPanning;
  EnvelopeConfig envVolume;
  u16 volumeFade;
} MOD_Instrument;

typedef struct MOD_Channel {

  u16 volumeFade;
  u16 volumeFadeDec;
  /*  u32 instrument;*/
  MOD_Instrument *instrument;
  /*  u32 sample;*/
  MOD_Sample *sample;
  MOD_VOICEINFO16 voiceInfo;
  Envelope envPanning;
  Envelope envVolume;

  u8 default_panning;
  u8 cur_note;
  u32 st3_period;
  s32 st3_periodofs;
  u8 last_instrument;
  u8 last_note;

  MOD_ChannelEffect effects[MODPLAY_NUM_COMMANDS];
} MOD_Channel;

typedef struct MODFILE {

  char songname[28];
  int nChannels;
  int nSFXChannels;
  int songlength;
  int nInstruments;
  int nSamples;
  int nPatterns;
  int restart_position;
  int period_type; /* 0 - Amiga, 1 - Linear (XM) */
  BOOL st2_vibrato;
  BOOL st2_tempo;
  BOOL amiga_sliding;
  BOOL optimize_vols;
  BOOL amiga_boundaries;
  BOOL enable_sfx;    /* Fast volume slides */
  BOOL unsigned_samples;
  u8 master_volume;
  u8 cur_master_volume;
  u8 musicvolume;
  u8 sfxvolume;
  u8 start_speed;
  u8 start_tempo;
  u16 tracker_version;

  u8 playlist[256];

  MOD_Channel channels[MODPLAY_MAX_CHANNELS];
  MOD_Instrument *instruments;
  MOD_Sample *samples;
  MOD_Note **patterns;
  int *patternLengths;

  int playfreq;       /* Output frequency (11025, 22050 or 44100) */
  int bits;           /* Output resolution (8 or 16 bits) */
  u16 *mixingbuf;    /* Output buffer */
  int mixingbuflen;   /* Output buffer length in bytes */
  int mixchannels;    /* 1 = mono, 2 = stereo */
  BOOL mixsigned;     /* mixingbuf is signed */

  /* Play time variables */
  int patterndelay;
  int pattern_line;
  int play_position;
  int speedcounter;
  int speed;
  int bpm;
  int samplespertick;
  int samplescounter;

  /* Pattern loop */
  int patternloop_to;
  int patternloop_count;

  void *tempmixbuf;

  int filetype;

  BOOL playing;
  BOOL set;

  u32 notebeats;
  void (*callback)(void*);

} MODFILE;

#include "modplay.h"

int MODFILE_Mix(MODFILE *mod, int flags, void *buf, int nSamples);
u32 MODFILE_EffectHandler(MODFILE *mod);
u32 MODFILE_Process(MODFILE *mod);
BOOL MODFILE_TriggerNote(MODFILE *mod, int channel, u8 note, u8 instrument, u8 volume, u16 *commands);
void MODFILE_SetBPM(MODFILE *mod, int bpm);
int MODFILE_BPM2SamplesPerTick(MODFILE *mod, int bpm);
char * MODFILE_GetNoteString(u8 note);
u16 MODFILE_GetEffect(MODFILE*,int,int,int);
u8 MODFILE_GetEffectOp(MODFILE*,int,int,int);
u8 MODFILE_GetNote(MODFILE *mod, int patternline, int channel);
u32 MODFILE_GetInstr(MODFILE *mod, int patternline, int channel);
void MODFILE_ClearPattern(MODFILE *mod, int pattern);

void MODFILE_SetNote(MODFILE *mod, int channel, u8 note, int middle_c, s8 finetune);
void MODFILE_SetPeriodOfs(MODFILE *mod, int channel, s32 periodofs);
void MODFILE_SetPeriod(MODFILE *mod, int channel, u32 period);
void MODFILE_SubVolume(MODFILE *mod, int channel, u8 sub);
void MODFILE_AddVolume(MODFILE *mod, int channel, u8 add);

#endif
