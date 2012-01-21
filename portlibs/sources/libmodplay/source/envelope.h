/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#ifndef __ENVELOPE_H__
#define __ENVELOPE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "defines.h"

#define ENV_WIDTH   65536
#define ENV_HEIGHT  65536

typedef struct EnvPoint {

  u16 x,y;
} EnvPoint;

typedef struct EnvelopeConfig {

  BOOL enabled;
  u8 numPoints;  /* # of envelope points */
  u8 loop_start;
  u8 loop_end;
  u8 sustain;

  EnvPoint *envPoints;
} EnvelopeConfig;

typedef struct Envelope {

  EnvelopeConfig *envConfig;

  BOOL triggered;
  BOOL hold;
  u8 curPoint;
  u16 value;
  u16 position;
} Envelope;


void EnvReset(Envelope *env);
void EnvTrigger(Envelope *env);
BOOL EnvProcess(Envelope *env);
void EnvRelease(Envelope *env);


#ifdef __cplusplus
}
#endif

#endif
