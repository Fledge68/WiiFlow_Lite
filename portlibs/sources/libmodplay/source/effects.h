/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include "modplay_core.h"

enum Effect {
	
	EFFECT_NONE = 0,
	
	/* Soundtracker */
	EFFECT_ST00,	/* Arpegggio */
	EFFECT_ST10,	/* Portamento Up */
	EFFECT_ST20,	/* Portamento Down */
	EFFECT_ST30,	/* Tone Portamento */
	EFFECT_ST40,	/* Vibrato */
	EFFECT_ST50,	/* Tone Portamento + Volume Slide */
	EFFECT_ST60,	/* Vibrato + Volume Slide */
	EFFECT_ST70,	/* Tremolo */
	EFFECT_ST80,	/* Panning */
	EFFECT_ST90,	/* Sample Offset */
	EFFECT_STa0,	/* Volume Slide */
	EFFECT_STb0,	/* Position Jump */
	EFFECT_STc0,	/* Set Volume */
	EFFECT_STd0,	/* Pattern Break */
	EFFECT_STe1,	/* Fineslide Up */
	EFFECT_STe2,	/* Fineslide Down */
	EFFECT_STe4,	/* Vibrato Control */
	EFFECT_STe5,	/* Set Finetune */
	EFFECT_STe6,	/* Pattern Loop */
	EFFECT_STe7,	/* Tremolo Control */
	EFFECT_STe8,	/* Panning */
	EFFECT_STe9,	/* Retrig Note */
	EFFECT_STea,	/* Fine Volume Up */
	EFFECT_STeb,	/* Fine Volume Down */
	EFFECT_STec,	/* Note Cut */
	EFFECT_STed,	/* Note Delay */
	EFFECT_STee,	/* Pattern Delay */
	EFFECT_STef,	/* Invert Loop */
	EFFECT_STf0,	/* Set Speed */
	EFFECT_STg0,	/* Goto line */

	/* Extended Module */
	EFFECT_XM01,	/* Portamento Up */
	EFFECT_XM02,	/* Portamento Down */
	EFFECT_XM10,	/* Global Volume */
	EFFECT_XM11,	/* Global Volume Slide */
	EFFECT_XM19,	/* Panning Slide */
	EFFECT_XMa0,	/* Volume Fade */

	/* S3M */
	EFFECT_S3Ma0,	/* Set Speed */
	EFFECT_S3Md0, /* Volume Slide */
	EFFECT_S3Me0,	/* Slide Down */
	EFFECT_S3Mf0,	/* Slide Up */
	EFFECT_S3Mk0, /* Vibrato + Volume Slide */
	EFFECT_S3Ml0, /* Tone Portamento + Volume Slide */
	EFFECT_S3Mq0, /* Retrig + Volume Slide */
	EFFECT_S3Mr0, /* Tremolo */
	EFFECT_S3Mt0, /* Set Tempo */
	
	/* Insert new effects before this one */
	EFFECT_NUMEFFECTS
};


#define EFFECT_DONOTTRIGGERNOTE			1
#define EFFECT_DONOTADVANCEPATTERN	2
#define EFFECT_DONOTRESETSAMPLEPOS	4
#define EFFECT_DONOTCHANGEPERIOD		8
#define EFFECT_DONOTCHANGEPANNING		16
#define EFFECT_DONOTCHANGEVOLUME		32


typedef struct MODPLAY_EffectHandler {
	
	int (*start)(MODFILE *mod, int channel, int ecol, MOD_Note *note);
	int (*process)(MODFILE *mod, int channel, int ecol);
	int (*stop)(MODFILE *mod, int channel, int ecol);
} MODPLAY_EffectHandler;

extern const MODPLAY_EffectHandler MODPLAY_EffectHandlers[];

#endif
