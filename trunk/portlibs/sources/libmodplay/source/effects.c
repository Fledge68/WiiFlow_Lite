/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include "effects.h"
#include "modplay_core.h"

static u32 mod_finetunes[16] = {

  7895,7941,7985,8046,8107,8169,8232,8280,
  8363,8413,8463,8529,8581,8651,8723,8757
};

static u16 wavetab[4][64] = {

  { /* Sine */
    (u16)0,    (u16)24,  (u16)49,  (u16)74,  (u16)97,  (u16)120, (u16)141, (u16)161,
    (u16)180,  (u16)197, (u16)212, (u16)224, (u16)235, (u16)244, (u16)250, (u16)253,
    (u16)255,  (u16)253, (u16)250, (u16)244, (u16)235, (u16)224, (u16)212, (u16)197,
    (u16)180,  (u16)161, (u16)141, (u16)120, (u16)97,  (u16)74,  (u16)49,  (u16)24,
    (u16)0,    (u16)-24, (u16)-49, (u16)-74, (u16)-97, (u16)-120,(u16)-141,(u16)-161,
    (u16)-180,(u16)-197,(u16)-212,(u16)-224,(u16)-235,(u16)-244,(u16)-250,(u16)-253,
    (u16)-255,(u16)-253,(u16)-250,(u16)-244,(u16)-235,(u16)-224,(u16)-212,(u16)-197,
    (u16)-180,(u16)-161,(u16)-141,(u16)-120,(u16)-97, (u16)-74, (u16)-49, (u16)-24
  }, { /* Ramp down */

    (u16)255, (u16)247, (u16)239, (u16)231, (u16)223, (u16)215, (u16)207, (u16)199,
    (u16)191, (u16)183, (u16)175, (u16)167, (u16)159, (u16)151, (u16)143, (u16)135,
    (u16)127, (u16)119, (u16)111, (u16)103, (u16)95,  (u16)87,  (u16)79,  (u16)71,
    (u16)63,  (u16)55,  (u16)47,  (u16)39,  (u16)31,  (u16)23,  (u16)15,  (u16)7,
    (u16)-1,  (u16)-9,  (u16)-17, (u16)-25, (u16)-33, (u16)-41, (u16)-49, (u16)-57,
    (u16)-65, (u16)-73, (u16)-81, (u16)-89, (u16)-97, (u16)-105,(u16)-113,(u16)-121,
    (u16)-129,(u16)-137,(u16)-145,(u16)-153,(u16)-161,(u16)-169,(u16)-177,(u16)-185,
    (u16)-193,(u16)-201,(u16)-209,(u16)-217,(u16)-225,(u16)-233,(u16)-241,(u16)-249
  }, { /* Square wave */

    (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255,
    (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255,
    (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255,
    (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255, (u16)255,
    (u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,
    (u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,
    (u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,
    (u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255,(u16)-255
  }, { /* Random */

    (u16)-26, (u16)-251,(u16)-198,(u16)-46, (u16)-96, (u16)198, (u16)168, (u16)228,
    (u16)-49, (u16)-153,(u16)-236,(u16)-174,(u16)-37, (u16)61,  (u16)187, (u16)120,
    (u16)56,  (u16)-197,(u16)248, (u16)-58, (u16)-204,(u16)172, (u16)58,  (u16)253,
    (u16)-155,(u16)57,  (u16)62,  (u16)-62, (u16)60,  (u16)-137,(u16)-101,(u16)-184,
    (u16)66,  (u16)-160,(u16)160, (u16)-29, (u16)-91, (u16)243, (u16)175, (u16)-175,
    (u16)149, (u16)97,  (u16)3,   (u16)-113,(u16)7,   (u16)249, (u16)-241,(u16)-247,
    (u16)110, (u16)-180,(u16)-139,(u16)-20, (u16)246, (u16)-86, (u16)-80, (u16)-134,
    (u16)219, (u16)117, (u16)-143,(u16)-226,(u16)-166,(u16)120, (u16)-47, (u16)29
  }
};


/*****************************************************************************
 *                        Pro/Soundtracker Effects                           *
 *****************************************************************************/


/**** EFFECT_NONE */
static int effect_None_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	return 0;
}

static int effect_None_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_None_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}



/**** EFFECT_ST00 - Arpeggio */
static int effect_ST00_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	mod->channels[channel].effects[ecol].arpeggio_base = mod->channels[channel].st3_period;
	return 0;
}

static int effect_ST00_Process(MODFILE *mod, int channel, int ecol) {

	u8 note, dnote;
  u8 octave;
  u8 semitone;

  if (!mod->channels[channel].sample)
  	return 0;

	note = mod->channels[channel].last_note;
	dnote = ((note >> 4) * 12) + (note & 0x0f);
 	dnote += mod->channels[channel].sample->relative_note;
 	dnote = ((dnote / 12) << 4) | (dnote % 12);

  octave = dnote >> 4;
	semitone = dnote & 0x0f;

  if (mod->channels[channel].effects[ecol].arpeggio_count == 0) {

    MODFILE_SetNote(mod, channel,
                    mod->channels[channel].instrument->note[(octave << 4) | semitone],
                    mod->channels[channel].sample->middle_c,
                    mod->channels[channel].sample->finetune);
  } else if (mod->channels[channel].effects[ecol].arpeggio_count == 1) {

    semitone += mod->channels[channel].effects[ecol].cur_operand >> 4;
    if (semitone > 11) {

      semitone -= 12;
      octave++;
    }
    MODFILE_SetNote(mod, channel,
                    mod->channels[channel].instrument->note[(octave << 4) | semitone],
                    mod->channels[channel].sample->middle_c,
                    mod->channels[channel].sample->finetune);
  } else if (mod->channels[channel].effects[ecol].arpeggio_count == 2) {

    semitone += mod->channels[channel].effects[ecol].cur_operand & 0x0f;
    if (semitone > 11) {

      semitone -= 12;
      octave++;
    }
    MODFILE_SetNote(mod, channel,
                    mod->channels[channel].instrument->note[(octave << 4) | semitone],
                    mod->channels[channel].sample->middle_c,
                    mod->channels[channel].sample->finetune);
  }

  if (++mod->channels[channel].effects[ecol].arpeggio_count > 2)
    mod->channels[channel].effects[ecol].arpeggio_count = 0;
	
	return 0;
}

static int effect_ST00_Stop(MODFILE *mod, int channel, int ecol) {

	u8 dnote, note;

	if (!mod->channels[channel].sample ||
			!mod->channels[channel].instrument)
		return 0;

	note = mod->channels[channel].last_note;
	dnote = ((note >> 4) * 12) + (note & 0x0f);
 	dnote += mod->channels[channel].sample->relative_note;
 	dnote = ((dnote / 12) << 4) | (dnote % 12);

  MODFILE_SetNote(mod, channel,
                  mod->channels[channel].instrument->note[dnote],
                  mod->channels[channel].sample->middle_c,
                  mod->channels[channel].sample->finetune);
	
	return 0;
}




/**** EFFECT_ST10 - Portamento Up */
static int effect_ST10_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	return 0;
}

static int effect_ST10_Process(MODFILE *mod, int channel, int ecol) {

  if (mod->speedcounter != 0) {

    u32 period = mod->channels[channel].st3_period;
    period -= 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;
    MODFILE_SetPeriod(mod, channel, period);
  }
	
	return 0;
}

static int effect_ST10_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_ST20 - Portamento Down */
static int effect_ST20_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	return 0;
}

static int effect_ST20_Process(MODFILE *mod, int channel, int ecol) {

  if (mod->speedcounter != 0) {

    u32 period = mod->channels[channel].st3_period;
    period += 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;
    MODFILE_SetPeriod(mod, channel, period);
  }
	
	return 0;
}

static int effect_ST20_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_ST30 - Tone Portamento */
static int effect_ST30_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	if (note->operand[ecol] == 0)
		mod->channels[channel].effects[ecol].cur_operand =
			mod->channels[channel].effects[ecol].toneporta_bak;
	else
		mod->channels[channel].effects[ecol].toneporta_bak = note->operand[ecol];

	return EFFECT_DONOTCHANGEPERIOD |
				 EFFECT_DONOTRESETSAMPLEPOS;
}

static int effect_ST30_Process(MODFILE *mod, int channel, int ecol) {
	
  if (mod->speedcounter != 0) {

    if (mod->channels[channel].st3_period > mod->channels[channel].effects[ecol].toneporta_dest) {

      mod->channels[channel].st3_period -= 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;

      if ((mod->channels[channel].st3_period < mod->channels[channel].effects[ecol].toneporta_dest) ||
          (mod->channels[channel].st3_period & 0x80000000))
        mod->channels[channel].st3_period = mod->channels[channel].effects[ecol].toneporta_dest;
    } else if (mod->channels[channel].st3_period < mod->channels[channel].effects[ecol].toneporta_dest) {

      mod->channels[channel].st3_period += 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;

      if (mod->channels[channel].st3_period > mod->channels[channel].effects[ecol].toneporta_dest)
        mod->channels[channel].st3_period = mod->channels[channel].effects[ecol].toneporta_dest;
    }
    MODFILE_SetPeriod(mod, channel, mod->channels[channel].st3_period);
  }

	return 0;
}

static int effect_ST30_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_ST40 - Vibrato */
static int effect_ST40_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand;
	
	if (note->operand[ecol] != 0)
		mod->channels[channel].effects[ecol].vibrato_bak = note->operand[ecol];

	operand = mod->channels[channel].effects[ecol].vibrato_bak;
	mod->channels[channel].effects[ecol].cur_operand = operand;

	if (operand & 0x0f)
  	mod->channels[channel].effects[ecol].vibrato_depth = operand & 0x0f;
  if (operand & 0xf0)
  	mod->channels[channel].effects[ecol].vibrato_freq = (operand >> 4) & 0x0f;
	
	return 0;
}

static int effect_ST40_Process(MODFILE *mod, int channel, int ecol) {
	
  if (mod->speedcounter != 0) {
	  
	  s32 wavetabval = (s32)(s16)
	  									wavetab[mod->channels[channel].effects[ecol].vibrato_wave & 3]
	  												 [mod->channels[channel].effects[ecol].vibrato_sintabpos];

		s32 periodofs = ((s32)(wavetabval * 4 *
    	(s32)(s16)mod->channels[channel].effects[ecol].vibrato_depth)) >> 7;
			
		MODFILE_SetPeriodOfs(mod, channel, periodofs);

    mod->channels[channel].effects[ecol].vibrato_sintabpos += mod->channels[channel].effects[ecol].vibrato_freq;
    mod->channels[channel].effects[ecol].vibrato_sintabpos &= 63;
  }

	return 0;
}

static int effect_ST40_Stop(MODFILE *mod, int channel, int ecol) {

	MODFILE_SetPeriodOfs(mod, channel, 0);	
	return 0;
}




/**** EFFECT_STa0 - Volume Slide */
static int effect_STa0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	return 0;
}

static void doSTVolumeSlide(MODFILE *mod, int channel, int ecol) {

  if (mod->channels[channel].effects[ecol].cur_operand & 0xf0) { /* Up */

    MODFILE_AddVolume(mod, channel, mod->channels[channel].effects[ecol].cur_operand >> 4);
  } else
  if (mod->channels[channel].effects[ecol].cur_operand & 0x0f) { /* Down */

 	  MODFILE_SubVolume(mod, channel, mod->channels[channel].effects[ecol].cur_operand & 0x0f);
  }
}

static int effect_STa0_Process(MODFILE *mod, int channel, int ecol) {

  if (mod->speedcounter != 0) {
	
	  doSTVolumeSlide(mod, channel, ecol);  
  }
	
	return 0;
}

static int effect_STa0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_ST50 - Tone Portamento + Volume Slide */
static int effect_ST50_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
/*	effect_ST30_Start(mod, channel, ecol, note);*/
	effect_STa0_Start(mod, channel, ecol, note);
	return 0;
}

static int effect_ST50_Process(MODFILE *mod, int channel, int ecol) {
	
	effect_ST30_Process(mod, channel, ecol);
	effect_STa0_Process(mod, channel, ecol);
	return 0;
}

static int effect_ST50_Stop(MODFILE *mod, int channel, int ecol) {
	
	effect_ST30_Stop(mod, channel, ecol);
	effect_STa0_Stop(mod, channel, ecol);
	return 0;
}




/**** EFFECT_ST60 - Vibrato + Volume Slide */
static int effect_ST60_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
/*	effect_ST40_Start(mod, channel, ecol, note);*/
	effect_STa0_Start(mod, channel, ecol, note);
	return 0;
}

static int effect_ST60_Process(MODFILE *mod, int channel, int ecol) {
	
	effect_ST40_Process(mod, channel, ecol);
	effect_STa0_Process(mod, channel, ecol);
	return 0;
}

static int effect_ST60_Stop(MODFILE *mod, int channel, int ecol) {
	
	effect_ST40_Stop(mod, channel, ecol);
	effect_STa0_Stop(mod, channel, ecol);
	return 0;
}




/**** EFFECT_ST70 - Tremolo */
static int effect_ST70_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand;

	if (note->operand[ecol] == 0)
		mod->channels[channel].effects[ecol].cur_operand =
			mod->channels[channel].effects[ecol].tremolo_bak;
	else
		mod->channels[channel].effects[ecol].tremolo_bak = note->operand[ecol];

	operand = mod->channels[channel].effects[ecol].cur_operand;
	
  mod->channels[channel].effects[ecol].tremolo_base = mod->channels[channel].voiceInfo.volume;

  if (operand & 0x0f)
  	mod->channels[channel].effects[ecol].tremolo_depth = operand & 0x0f;
  if (operand & 0xf0)
  	mod->channels[channel].effects[ecol].tremolo_freq = (operand >> 4) & 0x0f;

	return 0;
}

static int effect_ST70_Process(MODFILE *mod, int channel, int ecol) {

  u16 v;
  s16 delta = wavetab[mod->channels[channel].effects[ecol].tremolo_wave & 3][mod->channels[channel].effects[ecol].tremolo_sintabpos];
  delta *= mod->channels[channel].effects[ecol].tremolo_depth;
  delta >>= 7;

  v = mod->channels[channel].effects[ecol].tremolo_base + delta;

  if (v > 64)
    v = 64;
  if (v & 0xff80)
    v = 0;

  mod->channels[channel].voiceInfo.volume = v;
	
  if (mod->speedcounter != 0) {

  	mod->channels[channel].effects[ecol].tremolo_sintabpos += mod->channels[channel].effects[ecol].tremolo_freq;
  	mod->channels[channel].effects[ecol].tremolo_sintabpos &= 63;
  }

	return 0;
}

static int effect_ST70_Stop(MODFILE *mod, int channel, int ecol) {
	
	mod->channels[channel].voiceInfo.volume = mod->channels[channel].effects[ecol].tremolo_base;
	return 0;
}




/**** EFFECT_ST80 - Panning */
static int effect_ST80_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	mod->channels[channel].voiceInfo.panning = note->operand[ecol];
	return EFFECT_DONOTCHANGEPANNING;
}

static int effect_ST80_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_ST80_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_ST90 - Sample Offset */
static int effect_ST90_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
/*	return EFFECT_DONOTRESETSAMPLEPOS;*/
	return 0;
}

static int effect_ST90_Process(MODFILE *mod, int channel, int ecol) {

	if (mod->speedcounter == 0) {
		
  	MIXER_TYPE ofs;

  	ofs = ((MIXER_TYPE)mod->channels[channel].effects[ecol].cur_operand) << (8 + MIXER_SHIFT);
  	if (ofs > ((MIXER_TYPE)mod->channels[channel].sample->sampleInfo.length) << MIXER_SHIFT)
	  	mod->channels[channel].voiceInfo.playpos =
    		((MIXER_TYPE)mod->channels[channel].sample->sampleInfo.length) << MIXER_SHIFT;
  	else
  		mod->channels[channel].voiceInfo.playpos = ofs;

  	mod->channels[channel].voiceInfo.forward = TRUE;
	}

	return 0;
}

static int effect_ST90_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STb0 - Position Jump */
static int effect_STb0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

  mod->play_position = note->operand[ecol];
  mod->pattern_line = 0;

	return EFFECT_DONOTADVANCEPATTERN;
}

static int effect_STb0_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STb0_Stop(MODFILE *mod, int channel, int ecol) {

	return 0;
}




/**** EFFECT_STc0 - Set Volume */
static int effect_STc0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	if (note->operand[ecol] > 64)
		mod->channels[channel].voiceInfo.volume = 64;
	else
		mod->channels[channel].voiceInfo.volume = note->operand[ecol];

	return EFFECT_DONOTCHANGEVOLUME;
}

static int effect_STc0_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STc0_Stop(MODFILE *mod, int channel, int ecol) {

	return 0;
}




/**** EFFECT_STd0 - Pattern Break */
static int effect_STd0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

  do {

  	mod->play_position++;
    if (mod->play_position >= mod->songlength)
      mod->play_position = 0;
  } while (mod->playlist[mod->play_position] >= 0xfe);

  mod->pattern_line = note->operand[ecol];

	return EFFECT_DONOTADVANCEPATTERN;
}

static int effect_STd0_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STd0_Stop(MODFILE *mod, int channel, int ecol) {

	return 0;
}




/**** EFFECT_STe1 - Fineslide Up */
static int effect_STe1_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

 	u32 period = mod->channels[channel].st3_period;
 	period -= 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;
  MODFILE_SetPeriod(mod, channel, period);

	return 0;
}

static int effect_STe1_Process(MODFILE *mod, int channel, int ecol) {

	return 0;
}

static int effect_STe1_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STe2 - Fineslide Down */
static int effect_STe2_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

 	u32 period = mod->channels[channel].st3_period;
 	period += 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;
 	MODFILE_SetPeriod(mod, channel, period);

	return 0;
}

static int effect_STe2_Process(MODFILE *mod, int channel, int ecol) {

	return 0;
}

static int effect_STe2_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STe4 - Vibrato Control*/
static int effect_STe4_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	mod->channels[channel].effects[ecol].vibrato_wave = note->operand[ecol] & 0x07;
	return 0;
}

static int effect_STe4_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STe4_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STe5 - Set Finetune */
static int effect_STe5_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	if (mod->channels[channel].sample)	
		mod->channels[channel].sample->middle_c =
			mod_finetunes[mod->channels[channel].effects[ecol].cur_operand & 0x0f];

	return 0;
}

static int effect_STe5_Process(MODFILE *mod, int channel, int ecol) {

	if (mod->channels[channel].sample)	
		mod->channels[channel].sample->middle_c =
			mod_finetunes[mod->channels[channel].effects[ecol].cur_operand & 0x0f];
	
	return 0;
}

static int effect_STe5_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STe6 - Pattern Loop */
static int effect_STe6_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	BOOL doPatternLoop = FALSE;
	int retVal = 0;

	if ((note->operand[ecol] & 0x0f) == 0)
  	mod->patternloop_to = mod->pattern_line;
  else {

  	doPatternLoop = TRUE;
    if (mod->patternloop_count == 0) {

    	mod->patternloop_count = note->operand[ecol] & 0x0f;
    } else {

    	if (--mod->patternloop_count == 0)
      	doPatternLoop = FALSE;
    }
  }

	if (doPatternLoop) {
		
		mod->pattern_line = mod->patternloop_to;
		retVal = EFFECT_DONOTADVANCEPATTERN;
	}

	return retVal;
}

static int effect_STe6_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STe6_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STe7 - Tremolo Control */
static int effect_STe7_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	mod->channels[channel].effects[ecol].tremolo_wave = note->operand[ecol] & 0x07;
	return 0;
}

static int effect_STe7_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STe7_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STe8 - Panning */
static int effect_STe8_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	mod->channels[channel].voiceInfo.panning =
		((mod->channels[channel].effects[ecol].cur_operand & 0x0f) + 1) * 16;
	return 0;
}

static int effect_STe8_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STe8_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STe9 - Retrig Note */
static int effect_STe9_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	return 0;
}

static int effect_STe9_Process(MODFILE *mod, int channel, int ecol) {

	if (mod->speedcounter != 0) {
		
		if ((mod->speedcounter %
				 (int)mod->channels[channel].effects[ecol].cur_operand) == 0)
			MODFILE_TriggerNote(mod, channel,
					mod->channels[channel].last_note,
					mod->channels[channel].last_instrument,
					0xff, 0);
	}
	
	return 0;
}

static int effect_STe9_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STea - Fine Volume Up */
static int effect_STea_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand;
	
	operand = mod->channels[channel].effects[ecol].cur_operand;
	if (operand != 0)
		mod->channels[channel].effects[ecol].finevolslideup_bak = operand;
	else
		operand = mod->channels[channel].effects[ecol].finevolslideup_bak;

	MODFILE_AddVolume(mod, channel, operand);
	
	return 0;
}

static int effect_STea_Process(MODFILE *mod, int channel, int ecol) {

	return 0;
}

static int effect_STea_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STeb - Fine Volume Down */
static int effect_STeb_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand;
	
	operand = mod->channels[channel].effects[ecol].cur_operand;
	if (operand != 0)
		mod->channels[channel].effects[ecol].finevolslidedown_bak = operand;
	else
		operand = mod->channels[channel].effects[ecol].finevolslidedown_bak;

	MODFILE_SubVolume(mod, channel, operand);	

	return 0;
}

static int effect_STeb_Process(MODFILE *mod, int channel, int ecol) {

	return 0;
}

static int effect_STeb_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STec - Note Cut */
static int effect_STec_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	return 0;
}

static int effect_STec_Process(MODFILE *mod, int channel, int ecol) {
	
	if (mod->speedcounter != 0) {
		
		if (mod->speedcounter == mod->channels[channel].effects[ecol].cur_operand)
			mod->channels[channel].voiceInfo.volume = 0;
	}
	
	return 0;
}

static int effect_STec_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STed - Note Delay */
static int effect_STed_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	mod->channels[channel].effects[ecol].notedelay_note = note;

	return EFFECT_DONOTTRIGGERNOTE;
}

static int effect_STed_Process(MODFILE *mod, int channel, int ecol) {
	
	MOD_Note *note = mod->channels[channel].effects[ecol].notedelay_note;
	
	if (mod->speedcounter != 0) {
		
		if (mod->speedcounter == mod->channels[channel].effects[ecol].cur_operand) {

			MODFILE_TriggerNote(mod, channel, note->note, note->instrument, note->volume, note->effect);
		}
	}

	return 0;
}

static int effect_STed_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STee - Pattern Delay */
static int effect_STee_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	return 0;
}

static int effect_STee_Process(MODFILE *mod, int channel, int ecol) {
	
	if (mod->speedcounter == 0) {
		
		mod->patterndelay = mod->channels[channel].effects[ecol].cur_operand * mod->speed;
	}
	
	return 0;
}

static int effect_STee_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STef - Invert Loop */
static int effect_STef_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	return 0;
}

static int effect_STef_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STef_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STf0 - Set Speed */
static int effect_STf0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	if (note->operand[ecol] > 0) {
	
		if (note->operand[ecol] < 0x20)	
			mod->speed = note->operand[ecol];
		else
			MODFILE_SetBPM(mod, note->operand[ecol]);
	}

	return 0;
}

static int effect_STf0_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STf0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_STg0 - Goto Line */
static int effect_STg0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
  mod->play_position = note->operand[ecol];
  mod->pattern_line = note->operand[ecol + 1];

	return EFFECT_DONOTADVANCEPATTERN;
}

static int effect_STg0_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_STg0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}








/*****************************************************************************
 *                         Extended Module Effects                           *
 *****************************************************************************/

/**** EFFECT_XM01 - Portamento Up */
static int effect_XM01_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	if (note->operand[ecol] == 0)
		mod->channels[channel].effects[ecol].cur_operand =
			mod->channels[channel].effects[ecol].porta_bak;
	else
		mod->channels[channel].effects[ecol].porta_bak = note->operand[ecol];

	return 0;
}

static int effect_XM01_Process(MODFILE *mod, int channel, int ecol) {

  if (mod->speedcounter != 0) {

    u32 period = mod->channels[channel].st3_period;
    period -= 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;
    MODFILE_SetPeriod(mod, channel, period);
  }
	
	return 0;
}

static int effect_XM01_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_XM02 - Portamento Down */
static int effect_XM02_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	if (note->operand[ecol] == 0)
		mod->channels[channel].effects[ecol].cur_operand =
			mod->channels[channel].effects[ecol].porta_bak;
	else
		mod->channels[channel].effects[ecol].porta_bak = note->operand[ecol];

	return 0;
}

static int effect_XM02_Process(MODFILE *mod, int channel, int ecol) {

  if (mod->speedcounter != 0) {

    u32 period = mod->channels[channel].st3_period;
    period += 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;
    MODFILE_SetPeriod(mod, channel, period);
  }
	
	return 0;
}

static int effect_XM02_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_XM10 - Global Volume */
static int effect_XM10_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
  if (note->operand[ecol] > 64)
  	mod->cur_master_volume = 64;
  else
  	mod->cur_master_volume = note->operand[ecol];

	return 0;
}

static int effect_XM10_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_XM10_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_XM11 - Global Volume Slide */
static int effect_XM11_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	if (note->operand[ecol] != 0)
		mod->channels[channel].effects[ecol].gvolslide_bak = note->operand[ecol];

	mod->channels[channel].effects[ecol].cur_operand =
		mod->channels[channel].effects[ecol].gvolslide_bak;

	return 0;
}

static int effect_XM11_Process(MODFILE *mod, int channel, int ecol) {
	
	if (mod->speedcounter != 0) {

  	u8 operand = mod->channels[channel].effects[ecol].cur_operand;

    if (operand & 0x0f) {

    	if ((operand & 0x0f) > mod->cur_master_volume)
      	mod->cur_master_volume = 0;
      else
      	mod->cur_master_volume -= operand & 0x0f;
    } else {

      if ((operand >> 4) + mod->cur_master_volume > 64)
      	mod->cur_master_volume = 64;
      else
      	mod->cur_master_volume += operand >> 4;
    }
  }

	return 0;
}

static int effect_XM11_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_XM19 - Panning Slide */
static int effect_XM19_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
	if (note->operand[ecol] != 0)
		mod->channels[channel].effects[ecol].panslide_bak = note->operand[ecol];

	mod->channels[channel].effects[ecol].cur_operand =
		mod->channels[channel].effects[ecol].panslide_bak;

	return 0;
}

static int effect_XM19_Process(MODFILE *mod, int channel, int ecol) {

	if (mod->speedcounter != 0) {

		u8 operand = mod->channels[channel].effects[ecol].cur_operand;

		if (operand & 0xf0) { /* Right */

  		if (mod->channels[channel].voiceInfo.panning > 255 - (operand >> 4))
    		mod->channels[channel].voiceInfo.panning = 255;
    	else
    		mod->channels[channel].voiceInfo.panning += operand >> 4;
  	} else
  	if (operand & 0x0f) { /* Left */

  		if (mod->channels[channel].voiceInfo.panning < (operand & 0x0f))
    		mod->channels[channel].voiceInfo.panning = 0;
    	else
      	mod->channels[channel].voiceInfo.panning -= operand & 0x0f;
  	}
	}

	return 0;
}

static int effect_XM19_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_XMa0 - Volume Slide */
static int effect_XMa0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	if (note->operand[ecol] != 0)
		mod->channels[channel].effects[ecol].volslide_bak = note->operand[ecol];

	mod->channels[channel].effects[ecol].cur_operand = 
		mod->channels[channel].effects[ecol].volslide_bak;

	return 0;
}

static int effect_XMa0_Process(MODFILE *mod, int channel, int ecol) {

  if (mod->speedcounter != 0) {
	
	  doSTVolumeSlide(mod, channel, ecol);  
  }
	
	return 0;
}

static int effect_XMa0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}








/*****************************************************************************
 *                         Screamtracker 3 Effects                           *
 *****************************************************************************/


/**** EFFECT_S3Ma0 - Set Speed */
static int effect_S3Ma0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	if (note->operand[ecol] > 0) {
	
		mod->speed = note->operand[ecol];
	}

	return 0;
}

static int effect_S3Ma0_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_S3Ma0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_S3Md0 - Volume Slide */
static int effect_S3Md0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand;
	
	if (note->operand[ecol] != 0)
		mod->channels[channel].effects[ecol].volslide_bak = note->operand[ecol];

	mod->channels[channel].effects[ecol].cur_operand = operand =
		mod->channels[channel].effects[ecol].volslide_bak;

  if ((operand & 0x0f) == 0x0f) { /* Fine up */

	  MODFILE_AddVolume(mod, channel, operand >> 4);
  } else
  if ((operand & 0xf0) == 0xf0) { /* Fine down */

  	MODFILE_SubVolume(mod, channel, operand & 0x0f);
	}

	return 0;
}

static int effect_S3Md0_Process(MODFILE *mod, int channel, int ecol) {

	u8 operand = mod->channels[channel].effects[ecol].cur_operand;
	
	if ((((operand & 0xf0) != 0xf0) &&
			 ((operand & 0x0f) != 0x0f)) || (operand == 0xf0) || (operand == 0x0f)) {
		
  	if (mod->channels[channel].effects[ecol].cur_operand & 0x0f) { /* Down */

 	  	MODFILE_SubVolume(mod, channel, mod->channels[channel].effects[ecol].cur_operand & 0x0f);
  	} else
  	if (mod->channels[channel].effects[ecol].cur_operand & 0xf0) { /* Up */

    	MODFILE_AddVolume(mod, channel, mod->channels[channel].effects[ecol].cur_operand >> 4);
  	}
	}

	return 0;
}

static int effect_S3Md0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_S3Me0 - Slide Down */
static int effect_S3Me0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand;

	if (note->operand[ecol] != 0)
		mod->channels[channel].effects[ecol].porta_bak = note->operand[ecol];

	mod->channels[channel].effects[ecol].cur_operand = operand =
		mod->channels[channel].effects[ecol].porta_bak;

	if ((operand & 0xf0) == 0xf0) { /* Fine Slide Down */
			  				
    u32 period = mod->channels[channel].st3_period;
    period += (operand & 0x0f) * 4;
  	MODFILE_SetPeriod(mod, channel, period);
	} else
	if ((operand & 0xf0) == 0xe0) { /* Extra Fine Slide Down */

    u32 period = mod->channels[channel].st3_period;
    period += operand & 0x0f;
  	MODFILE_SetPeriod(mod, channel, period);
	}

	return 0;
}

static int effect_S3Me0_Process(MODFILE *mod, int channel, int ecol) {

	u8 operand = mod->channels[channel].effects[ecol].cur_operand;
	
	if (((operand & 0xf0) != 0xf0) &&
			((operand & 0xf0) != 0xe0)) {

		if (mod->speedcounter != 0) {
			
    	u32 period = mod->channels[channel].st3_period;
    	period += 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;
    	MODFILE_SetPeriod(mod, channel, period);
  	}
	}

	return 0;
}

static int effect_S3Me0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_S3Mfe - Slide Up */
static int effect_S3Mf0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand;

	if (note->operand[ecol] != 0)
		mod->channels[channel].effects[ecol].porta_bak = note->operand[ecol];

	mod->channels[channel].effects[ecol].cur_operand = operand =
		mod->channels[channel].effects[ecol].porta_bak;

	if ((operand & 0xf0) == 0xf0) { /* Fine Slide Up */
			  				
    u32 period = mod->channels[channel].st3_period;
    period -= (operand & 0x0f) * 4;
  	MODFILE_SetPeriod(mod, channel, period);
	} else
	if ((operand & 0xf0) == 0xe0) { /* Extra Fine Slide Up */

    u32 period = mod->channels[channel].st3_period;
    period -= operand & 0x0f;
  	MODFILE_SetPeriod(mod, channel, period);
	}

	return 0;
}

static int effect_S3Mf0_Process(MODFILE *mod, int channel, int ecol) {

	u8 operand = mod->channels[channel].effects[ecol].cur_operand;
	
	if (((operand & 0xf0) != 0xf0) &&
			((operand & 0xf0) != 0xe0)) {

		if (mod->speedcounter != 0) {
			
    	u32 period = mod->channels[channel].st3_period;
    	period -= 4 * (u32)mod->channels[channel].effects[ecol].cur_operand;
    	MODFILE_SetPeriod(mod, channel, period);
  	}
	}
	
	return 0;
}

static int effect_S3Mf0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_S3Mk0 - Vibrato + Volume Slide */
static int effect_S3Mk0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

/*	effect_ST40_Start(mod, channel, ecol, note);*/
	effect_S3Md0_Start(mod, channel, ecol, note);
	return 0;
}

static int effect_S3Mk0_Process(MODFILE *mod, int channel, int ecol) {

	effect_ST40_Process(mod, channel, ecol);
	effect_S3Md0_Process(mod, channel, ecol);
	return 0;
}

static int effect_S3Mk0_Stop(MODFILE *mod, int channel, int ecol) {

	effect_ST40_Stop(mod, channel, ecol);
	effect_S3Md0_Stop(mod, channel, ecol);
	return 0;
}




/**** EFFECT_S3Ml0 - Tone Portamento + Volume Slide */
static int effect_S3Ml0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {
	
/*	effect_ST30_Start(mod, channel, ecol, note);*/
	effect_S3Md0_Start(mod, channel, ecol, note);
	return 0;
}

static int effect_S3Ml0_Process(MODFILE *mod, int channel, int ecol) {
	
	effect_ST30_Process(mod, channel, ecol);
	effect_S3Md0_Process(mod, channel, ecol);
	return 0;
}

static int effect_S3Ml0_Stop(MODFILE *mod, int channel, int ecol) {
	
	effect_ST30_Stop(mod, channel, ecol);
	effect_S3Md0_Stop(mod, channel, ecol);
	return 0;
}




/**** EFFECT_S3Mq0 - Retrig + Volume Slide */
static int effect_S3Mq0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand = mod->channels[channel].effects[ecol].cur_operand;

	if (operand != 0)
  	mod->channels[channel].effects[ecol].retrig_bak = operand;
  else
  	operand = mod->channels[channel].effects[ecol].retrig_bak;

	mod->channels[channel].effects[ecol].cur_operand = operand;
	mod->channels[channel].effects[ecol].retrig_count = 0;

	return 0;
}

static int effect_S3Mq0_Process(MODFILE *mod, int channel, int ecol) {

	mod->channels[channel].effects[ecol].retrig_count++;
  if (mod->speedcounter != 0) {

  	if (mod->channels[channel].effects[ecol].retrig_count >=
  			(mod->channels[channel].effects[ecol].cur_operand & 0x0f)) {

    	if (mod->channels[channel].effects[ecol].retrig_count >=
    			(mod->channels[channel].effects[ecol].cur_operand & 0x0f))
      	mod->channels[channel].effects[ecol].retrig_count = 0;

      /* Volume slide */
      if (mod->channels[channel].effects[ecol].cur_operand & 0xf0) {

      	switch ((mod->channels[channel].effects[ecol].cur_operand >> 4) & 0x0f) {

        	case 0:
          case 8:
          	break;
          case 1:
          	MODFILE_SubVolume(mod, channel, 1);
            break;
          case 2:
          	MODFILE_SubVolume(mod, channel, 2);
            break;
          case 3:
          	MODFILE_SubVolume(mod, channel, 4);
            break;
          case 4:
          	MODFILE_SubVolume(mod, channel, 8);
            break;
          case 5:
          	MODFILE_SubVolume(mod, channel, 16);
            break;
          case 6: {
          	u8 tmpvol = mod->channels[channel].voiceInfo.volume;
            tmpvol = (tmpvol * 2) / 3;
            mod->channels[channel].voiceInfo.volume = tmpvol;
            break;
          }
          case 7:
          	mod->channels[channel].voiceInfo.volume >>= 1;
            break;
          case 9:
          	MODFILE_AddVolume(mod, channel, 1);
            break;
          case 10:
          	MODFILE_AddVolume(mod, channel, 2);
            break;
          case 11:
          	MODFILE_AddVolume(mod, channel, 4);
          	break;
          case 12:
          	MODFILE_AddVolume(mod, channel, 8);
            break;
          case 13:
          	MODFILE_AddVolume(mod, channel, 16);
            break;
          case 14: {
          	u8 tmpvol = mod->channels[channel].voiceInfo.volume;
            tmpvol = (tmpvol * 3) / 2;
            if (tmpvol > 64) tmpvol = 64;
            mod->channels[channel].voiceInfo.volume = tmpvol;
            break;
          }
          case 15:
          	mod->channels[channel].voiceInfo.volume *= 2;
            if (mod->channels[channel].voiceInfo.volume > 64)
            	mod->channels[channel].voiceInfo.volume = 64;
            break;
        }
      }
      /* Retrigger */
      mod->channels[channel].voiceInfo.playpos = 0;
      mod->channels[channel].voiceInfo.playing = TRUE;
      mod->channels[channel].voiceInfo.forward = TRUE;
/*			MODFILE_TriggerNote(mod, channel,
					mod->channels[channel].last_note,
					mod->channels[channel].last_instrument,
					0xff, 0);*/

    }
  }

	return 0;
}

static int effect_S3Mq0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}




/**** EFFECT_S3Mr0 - Tremolo */
static int effect_S3Mr0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	u8 operand;

	if (note->operand[ecol] == 0)
		mod->channels[channel].effects[ecol].cur_operand =
			mod->channels[channel].effects[ecol].tremolo_bak;
	else
		mod->channels[channel].effects[ecol].tremolo_bak = note->operand[ecol];

	operand = mod->channels[channel].effects[ecol].cur_operand;
	
  mod->channels[channel].effects[ecol].tremolo_base = mod->channels[channel].voiceInfo.volume;

  if (operand & 0x0f)
  	mod->channels[channel].effects[ecol].tremolo_depth = operand & 0x0f;
  if (operand & 0xf0)
  	mod->channels[channel].effects[ecol].tremolo_freq = (operand >> 4) & 0x0f;

	return 0;
}

static int effect_S3Mr0_Process(MODFILE *mod, int channel, int ecol) {

  u16 v;
  s16 delta = wavetab[mod->channels[channel].effects[ecol].tremolo_wave & 3][mod->channels[channel].effects[ecol].tremolo_sintabpos];
  delta *= mod->channels[channel].effects[ecol].tremolo_depth;
  delta >>= 7;

  v = mod->channels[channel].effects[ecol].tremolo_base + delta;

  if (v > 64)
    v = 64;
  if (v & 0xff80)
    v = 0;

  mod->channels[channel].voiceInfo.volume = v;
	
 	mod->channels[channel].effects[ecol].tremolo_sintabpos += mod->channels[channel].effects[ecol].tremolo_freq;
 	mod->channels[channel].effects[ecol].tremolo_sintabpos &= 63;

	return 0;
}

static int effect_S3Mr0_Stop(MODFILE *mod, int channel, int ecol) {
	
	mod->channels[channel].voiceInfo.volume = mod->channels[channel].effects[ecol].tremolo_base;
	return 0;
}




/**** EFFECT_S3Mt0 - Set Tempo */
static int effect_S3Mt0_Start(MODFILE *mod, int channel, int ecol, MOD_Note *note) {

	if (note->operand[ecol] > 0) {
		
		MODFILE_SetBPM(mod, note->operand[ecol]);
	}
	
	return 0;
}

static int effect_S3Mt0_Process(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}

static int effect_S3Mt0_Stop(MODFILE *mod, int channel, int ecol) {
	
	return 0;
}















const MODPLAY_EffectHandler MODPLAY_EffectHandlers[] = {

	/* Soundtracker */
	{ effect_None_Start,  effect_None_Process,  effect_None_Stop  },	/* EFFECT_NONE */
	{ effect_ST00_Start,  effect_ST00_Process,  effect_ST00_Stop  },	/* EFFECT_ST00 */
	{ effect_ST10_Start,  effect_ST10_Process,  effect_ST10_Stop  },	/* EFFECT_ST10 */
	{ effect_ST20_Start,  effect_ST20_Process,  effect_ST20_Stop  },	/* EFFECT_ST20 */
	{ effect_ST30_Start,  effect_ST30_Process,  effect_ST30_Stop  },	/* EFFECT_ST30 */
	{ effect_ST40_Start,  effect_ST40_Process,  effect_ST40_Stop  },	/* EFFECT_ST40 */
	{ effect_ST50_Start,  effect_ST50_Process,  effect_ST50_Stop  },	/* EFFECT_ST50 */
	{ effect_ST60_Start,  effect_ST60_Process,  effect_ST60_Stop  },	/* EFFECT_ST60 */
	{ effect_ST70_Start,  effect_ST70_Process,  effect_ST70_Stop  },	/* EFFECT_ST70 */
	{ effect_ST80_Start,  effect_ST80_Process,  effect_ST80_Stop  },	/* EFFECT_ST80 */
	{ effect_ST90_Start,  effect_ST90_Process,  effect_ST90_Stop  },	/* EFFECT_ST90 */
	{ effect_STa0_Start,  effect_STa0_Process,  effect_STa0_Stop  },	/* EFFECT_STa0 */
	{ effect_STb0_Start,  effect_STb0_Process,  effect_STb0_Stop  },	/* EFFECT_STb0 */
	{ effect_STc0_Start,  effect_STc0_Process,  effect_STc0_Stop  },	/* EFFECT_STc0 */
	{ effect_STd0_Start,  effect_STd0_Process,  effect_STd0_Stop  },	/* EFFECT_STd0 */
	{ effect_STe1_Start,  effect_STe1_Process,  effect_STe1_Stop  },	/* EFFECT_STe1 */
	{ effect_STe2_Start,  effect_STe2_Process,  effect_STe2_Stop  },	/* EFFECT_STe2 */
	{ effect_STe4_Start,  effect_STe4_Process,  effect_STe4_Stop  },	/* EFFECT_STe4 */
	{ effect_STe5_Start,  effect_STe5_Process,  effect_STe5_Stop  },	/* EFFECT_STe5 */
	{ effect_STe6_Start,  effect_STe6_Process,  effect_STe6_Stop  },	/* EFFECT_STe6 */
	{ effect_STe7_Start,  effect_STe7_Process,  effect_STe7_Stop  },	/* EFFECT_STe7 */
	{ effect_STe8_Start,  effect_STe8_Process,  effect_STe8_Stop  },	/* EFFECT_STe8 */
	{ effect_STe9_Start,  effect_STe9_Process,  effect_STe9_Stop  },	/* EFFECT_STe9 */
	{ effect_STea_Start,  effect_STea_Process,  effect_STea_Stop  },	/* EFFECT_STea */
	{ effect_STeb_Start,  effect_STeb_Process,  effect_STeb_Stop  },	/* EFFECT_STeb */
	{ effect_STec_Start,  effect_STec_Process,  effect_STec_Stop  },	/* EFFECT_STec */
	{ effect_STed_Start,  effect_STed_Process,  effect_STed_Stop  },	/* EFFECT_STed */
	{ effect_STee_Start,  effect_STee_Process,  effect_STee_Stop  },	/* EFFECT_STee */
	{ effect_STef_Start,  effect_STef_Process,  effect_STef_Stop  },	/* EFFECT_STef */
	{ effect_STf0_Start,  effect_STf0_Process,  effect_STf0_Stop  },	/* EFFECT_STf0 */
	{ effect_STg0_Start,  effect_STg0_Process,  effect_STg0_Stop  },	/* EFFECT_STg0 */

	/* Extended Module */
	{ effect_XM01_Start,  effect_XM01_Process,  effect_XM01_Stop  },	/* EFFECT_XM01 */
	{ effect_XM02_Start,  effect_XM02_Process,  effect_XM02_Stop  },	/* EFFECT_XM02 */
	{ effect_XM10_Start,  effect_XM10_Process,  effect_XM10_Stop  },	/* EFFECT_XM10 */
	{ effect_XM11_Start,  effect_XM11_Process,  effect_XM11_Stop  },	/* EFFECT_XM11 */
	{ effect_XM19_Start,  effect_XM19_Process,  effect_XM19_Stop  },	/* EFFECT_XM19 */
	{ effect_XMa0_Start,  effect_XMa0_Process,  effect_XMa0_Stop  },	/* EFFECT_XMa0 */
	
	/* S3M */
	{ effect_S3Ma0_Start, effect_S3Ma0_Process, effect_S3Ma0_Stop },	/* EFFECT_S3Ma0 */
	{ effect_S3Md0_Start, effect_S3Md0_Process, effect_S3Md0_Stop },	/* EFFECT_S3Md0 */
	{ effect_S3Me0_Start, effect_S3Me0_Process, effect_S3Me0_Stop },	/* EFFECT_S3Me0 */
	{ effect_S3Mf0_Start, effect_S3Mf0_Process, effect_S3Mf0_Stop },	/* EFFECT_S3Mfe */
	{ effect_S3Mk0_Start, effect_S3Mk0_Process, effect_S3Mk0_Stop },	/* EFFECT_S3Mk0 */
	{ effect_S3Ml0_Start, effect_S3Ml0_Process, effect_S3Ml0_Stop },	/* EFFECT_S3Ml0 */
	{ effect_S3Mq0_Start, effect_S3Mq0_Process, effect_S3Mq0_Stop },	/* EFFECT_S3Mq0 */
	{ effect_S3Mr0_Start, effect_S3Mr0_Process, effect_S3Mr0_Stop },	/* EFFECT_S3Mr0 */
	{ effect_S3Mt0_Start, effect_S3Mt0_Process, effect_S3Mt0_Stop },	/* EFFECT_S3Mt0 */

	{ NULL, NULL, NULL }
};
