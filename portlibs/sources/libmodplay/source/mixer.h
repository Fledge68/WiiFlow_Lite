/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#ifndef __MIXER_H__
#define __MIXER_H__

#include "defines.h"

/*
#define MIXER_TYPE  u64
#define MIXER_SHIFT 32
*/

#define MIXER_TYPE u32
#define MIXER_SHIFT 10


typedef struct MOD_SAMPLEINFO16 {

  u32 length;
  u32 loop_start;
  u32 loop_end;
  BOOL looped;
  BOOL pingpong;
  void *sampledata;
  BOOL bit_16;
  BOOL stereo;
} MOD_SAMPLEINFO16;

typedef struct MOD_VOICEINFO16 {

  BOOL enabled;
  BOOL playing;
  BOOL forward;
  u8 panning;
  u8 envPanning;
  /*  u32 playpos;
      u32 incval;*/
  MIXER_TYPE playpos;
  MIXER_TYPE incval;
  u8 volume;
  u8 envVolume;
  MOD_SAMPLEINFO16 *sampleInfo;
} MOD_VOICEINFO16;

#define MIXER_USE_S32     1
#define MIXER_3232BIT     2
#define MIXER_SRC_SIGNED  4
#define MIXER_DEST_STEREO 8
#define MIXER_USE_DOUBLE  16
#define MIXER_USE_FLOAT   32
#define MIXER_DEST_16BIT  64
#define MIXER_DEST_SIGNED 128
#define MIXER_SRC_16BIT   256

int mix_s8m_to_s32m_1616bit (s32 *, int, MOD_VOICEINFO16 *, u8);
int mix_s8m_to_s32s_1616bit (s32 *, int, MOD_VOICEINFO16*, u8);

int mix_s16m_to_s32s_1616bit(s32 *, int, MOD_VOICEINFO16 *, u8);
int mix_s16m_to_s32m_1616bit(s32 *, int, MOD_VOICEINFO16 *, u8);

void clearbuf_s32(s32 *, int, int);

void copybuf_s32_to_s16(s16 *, s32 *, int, int);
void copybuf_s32_to_u16(u16 *, s32 *, int, int);
void copybuf_s32_to_s8 (s8 *,  s32 *, int, int);
void copybuf_s32_to_u8 (u8 *,  s32 *, int, int);

int mix_final_1616bit(int flags, void *dest, int nSamples, MOD_VOICEINFO16 *vinfo, u8 mainvol);
int copybuf_final(int flags, void *dest, void *src, int nSamples);
int clearbuf_final(int flags, void *dest, int nSamples);

int mix_destbufsize(int flags);

#endif
