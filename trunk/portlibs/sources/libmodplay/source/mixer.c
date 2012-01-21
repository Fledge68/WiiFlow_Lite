/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include "mixer.h"

#define DO_LOOPING \
				if (forward) \
					{ \
						playpos += incval; \
						if (playpos >= loop_end) \
							{ \
								if (looped) \
									{ \
										if (pingpong) \
											{ \
												forward = !forward; \
												playpos -= incval; \
											} else \
											playpos -= loop_end - loop_start; \
									} else \
									{ \
										playpos = loop_end; \
										vinfo->playing = FALSE; \
										break; \
									} \
							} \
					} else \
					{ \
						playpos -= incval; \
						if (/*playpos <= loop_start*/playpos - loop_start <= incval) \
							{ \
								if (looped) \
									{ \
										if (pingpong) \
											{ \
												forward = !forward; \
												playpos += incval; \
											} else \
											playpos += loop_end - loop_start; \
									} else \
									{ \
										playpos = loop_start; \
										vinfo->playing = FALSE; \
										break; \
									} \
							} \
					}

#define DO_LOOPING_OLD \
        playpos += incval; \
        if (playpos >= loop_end) \
          { \
            if (looped) \
              { \
                playpos -= loop_end-loop_start; \
              } else \
              { \
                playpos = loop_end; \
                vinfo->playing = FALSE; \
                break; \
              } \
          }

#define DO_LOOPING_PINGPONG \
				playpos += incval; \
				if (forward && playpos >= loop_end) \
					{ \
						if (looped) \
							{ \
								incval = -incval; \
								forward = FALSE; \
							} else \
							{ \
								playpos = loop_end; \
								vinfo->playing = FALSE; \
								break; \
							} \
					} else \
				if (!forward && playpos <= loop_start) \
					{ \
						if (looped) \
							{ \
								incval = -incval; \
								forward = TRUE; \
							} else \
							{ \
								playpos = loop_start; \
								vinfo->playing = FALSE; \
								break; \
							} \
					}

int mix_destbufsize(int flags) {

  int size = 1;

  if (flags & MIXER_DEST_STEREO)
    size *= 2;
  if (flags & MIXER_DEST_16BIT)
    size *= 2;

  return size;
}

int clearbuf_final(int flags, void *dest, int nSamples) {

  if (flags & MIXER_USE_S32) {

    if (flags & MIXER_DEST_STEREO) {

      clearbuf_s32((s32*)dest, 2, nSamples);
    } else {

      clearbuf_s32((s32*)dest, 1, nSamples);
    }
  }

  return nSamples;
}

int mix_final_1616bit(int flags, void *dest, int nSamples, MOD_VOICEINFO16 *vinfo, u8 mainvol) {

  if (flags & MIXER_USE_S32) {

    if (flags & MIXER_DEST_STEREO) {

      if (vinfo->sampleInfo->bit_16) {

	mix_s16m_to_s32s_1616bit((s32*)dest, nSamples, vinfo, mainvol);
      } else {

	mix_s8m_to_s32s_1616bit((s32*)dest, nSamples, vinfo, mainvol);
      }
    } else {

      if (vinfo->sampleInfo->bit_16) {

	mix_s16m_to_s32m_1616bit((s32*)dest, nSamples, vinfo, mainvol);
      } else {

	mix_s8m_to_s32m_1616bit((s32*)dest, nSamples, vinfo, mainvol);
      }
    }
  }

  return nSamples;
}

int copybuf_final(int flags, void *dest, void *src, int nSamples) {

  if (flags & MIXER_USE_S32) {

    if (flags & MIXER_DEST_SIGNED) {

      if (flags & MIXER_DEST_16BIT) {

	if (flags & MIXER_DEST_STEREO) {

	  copybuf_s32_to_s16((s16*)dest, (s32*)src, 2, nSamples);
	} else {

	  copybuf_s32_to_s16((s16*)dest, (s32*)src, 1, nSamples);
	}
      } else {

	if (flags & MIXER_DEST_STEREO) {

	  copybuf_s32_to_s8((s8*)dest, (s32*)src, 2, nSamples);
	} else {

	  copybuf_s32_to_s8((s8*)dest, (s32*)src, 1, nSamples);
	}
      }
    } else {

      if (flags & MIXER_DEST_16BIT) {

	if (flags & MIXER_DEST_STEREO) {

	  copybuf_s32_to_u16((u16*)dest, (s32*)src, 2, nSamples);
	} else {

	  copybuf_s32_to_u16((u16*)dest, (s32*)src, 1, nSamples);
	}
      } else {

	if (flags & MIXER_DEST_STEREO) {

	  copybuf_s32_to_u8((u8*)dest, (s32*)src, 2, nSamples);
	} else {

	  copybuf_s32_to_u8((u8*)dest, (s32*)src, 1, nSamples);
	}
      }
    }
  }

  return nSamples;
}

/* Mix s8 mono -> s32 mono, 16.16bit fixed point */
int mix_s8m_to_s32m_1616bit(s32 *dest, int nSamples, MOD_VOICEINFO16 *vinfo, u8 mainvol) {

  int k;
  s32 volume;
  MIXER_TYPE playpos = vinfo->playpos;
  MIXER_TYPE incval = vinfo->incval;
  MIXER_TYPE loop_end = ((MIXER_TYPE)vinfo->sampleInfo->loop_end) << MIXER_SHIFT;
  MIXER_TYPE loop_start = ((MIXER_TYPE)vinfo->sampleInfo->loop_start) << MIXER_SHIFT;
  BOOL looped = vinfo->sampleInfo->looped;
  BOOL forward = vinfo->forward;
  BOOL pingpong = vinfo->sampleInfo->pingpong;
  s8 *sampledata = vinfo->sampleInfo->sampledata;

  if (!vinfo->playing)
    return nSamples;

  volume = ((s32)vinfo->volume * (s32)mainvol * (s32)vinfo->envVolume) >> 12;

  for (k = 0; k < nSamples; k++) {

    s32 value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    dest[k] += value * volume;
    DO_LOOPING;
  }

  vinfo->playpos = playpos;
  vinfo->forward = forward;

  return nSamples;
}

/* Mix s16 mono -> s32 mono, 16.16bit fixed point */
int mix_s16m_to_s32m_1616bit(s32 *dest, int nSamples, MOD_VOICEINFO16 *vinfo, u8 mainvol) {

  int k;
  s32 volume;
  MIXER_TYPE playpos = vinfo->playpos;
  MIXER_TYPE incval = vinfo->incval;
  MIXER_TYPE loop_end = ((MIXER_TYPE)vinfo->sampleInfo->loop_end) << MIXER_SHIFT;
  MIXER_TYPE loop_start = ((MIXER_TYPE)vinfo->sampleInfo->loop_start) << MIXER_SHIFT;
  BOOL looped = vinfo->sampleInfo->looped;
  BOOL forward = vinfo->forward;
  BOOL pingpong = vinfo->sampleInfo->pingpong;
  s16 *sampledata = vinfo->sampleInfo->sampledata;

  if (!vinfo->playing)
    return nSamples;

  volume = ((s32)vinfo->volume * (s32)mainvol * (s32)vinfo->envVolume) >> 12;

  for (k = 0; k <nSamples; k++) {

    s32 value = (s32)((s16*)sampledata)[playpos >> MIXER_SHIFT];
    dest[k] += value * volume;
    DO_LOOPING;
  }

  vinfo->playpos = playpos;
  vinfo->forward = forward;

  return nSamples;
}

/* Mix s8 mono -> s32 stereo, 16.16bit fixed point */
int mix_s8m_to_s32s_1616bit(s32 *dest, int nSamples, MOD_VOICEINFO16 *vinfo, u8 mainvol) {

  int k;
  s32 volume;
  u8 lvolume, rvolume;
  MIXER_TYPE playpos = vinfo->playpos;
  MIXER_TYPE incval = vinfo->incval;
  MIXER_TYPE loop_end = ((MIXER_TYPE)vinfo->sampleInfo->loop_end) << MIXER_SHIFT;
  MIXER_TYPE loop_start = ((MIXER_TYPE)vinfo->sampleInfo->loop_start) << MIXER_SHIFT;
  BOOL looped = vinfo->sampleInfo->looped;
  BOOL forward = vinfo->forward;
  BOOL pingpong = vinfo->sampleInfo->pingpong;
  s8 * sampledata = vinfo->sampleInfo->sampledata;
  s32 panning;

  int mxamount = nSamples >> 3;
  int mxrest   = nSamples & ((1 << 3) - 1);

  if (!vinfo->playing)
    return nSamples;

  volume = ((s32)vinfo->volume * (s32)mainvol * (s32)vinfo->envVolume) >> 12;

  panning = ((s32)(vinfo->panning >> 2) - 32) + ((s32)vinfo->envPanning - 32);
  if (panning > 32)
    panning = 32;
  if (panning < -32)
    panning = -32;
  panning += 32;

  lvolume = (((u32)64 - panning) * volume) >> 6;
  rvolume = (((u32)panning) * volume) >> 6;

 	for (k = 0; k < mxamount; k++) {

    s32 value;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;
  }

  for (k = 0; k < mxrest; k++) {

    s32 value;

    value = ((s32)(s16)((s8*)sampledata)[playpos >> MIXER_SHIFT]) << 8;
    (*(dest++))   += value * lvolume;
    (*(dest++))   += value * rvolume;
    DO_LOOPING;
  }

  vinfo->playpos = playpos;
  vinfo->forward = forward;

  return nSamples;
}

/* Mix s16 mono -> s32 stereo, 16.16bit fixed point */
int mix_s16m_to_s32s_1616bit(s32 *dest, int nSamples, MOD_VOICEINFO16 *vinfo, u8 mainvol) {

  int k;
  s32 volume;
  u8 lvolume, rvolume;
  MIXER_TYPE playpos = vinfo->playpos;
  MIXER_TYPE incval = vinfo->incval;
  MIXER_TYPE loop_end = ((MIXER_TYPE)vinfo->sampleInfo->loop_end) << MIXER_SHIFT;
  MIXER_TYPE loop_start = ((MIXER_TYPE)vinfo->sampleInfo->loop_start) << MIXER_SHIFT;
  BOOL looped = vinfo->sampleInfo->looped;
  BOOL forward = vinfo->forward;
  BOOL pingpong = vinfo->sampleInfo->pingpong;
  s16 * sampledata = vinfo->sampleInfo->sampledata;

  if (!vinfo->playing)
    return nSamples;

  volume = ((s32)vinfo->volume * (s32)mainvol * (s32)vinfo->envVolume) >> 12;

  lvolume = (((u32)64-(vinfo->panning >> 2))*volume) >> 6;
  rvolume = (((u32)(vinfo->panning >> 2))*volume) >> 6;

  for (k = 0; k < nSamples * 2; k += 2) {

    s32 value = (s32)((s16*)sampledata)[playpos >> MIXER_SHIFT];
    dest[k]   += value * lvolume;
    dest[k + 1] += value * rvolume;
    DO_LOOPING;
  }

  vinfo->playpos = playpos;
	vinfo->forward = forward;

  return nSamples;
}

void clearbuf_s32(s32 *dest, int channels, int nSamples) {

  int k;
  int mxamount;
  int mxrest;

  mxamount = (nSamples * channels) >> 3;
  mxrest   = (nSamples * channels) & ((1 << 3) - 1);

  for (k = 0; k < mxamount; k++) {

    (*(dest++)) = 0;
    (*(dest++)) = 0;
    (*(dest++)) = 0;
    (*(dest++)) = 0;
    (*(dest++)) = 0;
    (*(dest++)) = 0;
    (*(dest++)) = 0;
    (*(dest++)) = 0;
  }

  for (k = 0; k < mxrest; k++) {

    (*(dest++)) = 0;
  }

/*    for (k = 0; k < nSamples * channels; k++)
      dest[k] = 0;*/
}

void copybuf_s32_to_s16(s16 *dest, s32 *src, int channels, int nSamples) {

  int k;

  for (k = 0; k < nSamples*channels; k++) {

    s32 value = src[k] >> 8;

    if (value < -32768)
      value = -32768;
    else if (value > 32767)
      value = 32767;

    dest[k] = (s16)value;
  }
}

#define CLIP_S32(a) \
  if (a < -32768) a = 32768; else if (a > 32767) a = 32767;

void copybuf_s32_to_u16(u16 *dest, s32 *src, int channels, int nSamples) {

  int k;
  int mxamount;
  int mxrest;

  mxamount = (nSamples * channels) >> 3;
  mxrest   = (nSamples * channels) & ((1 << 3) - 1);

  for (k = 0; k < mxamount; k++) {

    s32 value;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;
  }

  for (k = 0; k < mxrest; k++) {

    s32 value;

    value = (*(src++)) >> 7;
    CLIP_S32(value);
    (*(dest++)) = ((u16)(s16)value) ^ 0x8000;
  }

/*    for (k=0; k<nSamples*channels; k++)
      {
        s32 value = src[k]>>7;
        if (value<-32768)
          value = -32768;
        else
        if (value>32767)
          value = 32767;
        dest[k] = ((u16)(s16)value)^0x8000;
      }*/
}

void copybuf_s32_to_s8(s8 *dest, s32 *src, int channels, int nSamples) {

  int k;

  for (k = 0; k < nSamples * channels; k++) {

    s32 value = src[k] >> 16;

    if (value < -128)
      value = -128;
    else if (value > 127)
      value = 127;
    dest[k] = (s8)value;
  }
}

void copybuf_s32_to_u8(u8 *dest, s32 *src, int channels, int nSamples) {

  int k;

  for (k = 0; k < nSamples * channels; k++) {

    s32 value = src[k] >> 16;

    if (value < -128)
      value = -128;
    else if (value > 127)
      value = 127;
    dest[k] = ((u8)(s8)value) ^ 0x80;
  }
}
