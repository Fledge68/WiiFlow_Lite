/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include "envelope.h"

void EnvReset(Envelope *env) {

  if (env == NULL)
    return;

  if (env->envConfig == NULL)
    return;

  if (!env->envConfig->enabled)
    return;

  env->hold = FALSE;
  env->triggered = FALSE;
  env->position = 0;
  env->value = 0;
  env->curPoint = 0;

  if (env->envConfig->numPoints > 0) {

    env->value = env->envConfig->envPoints[0].y;
  }
}



void EnvTrigger(Envelope *env) {

  if (env == NULL)
    return;

  if (env->envConfig == NULL)
    return;

  if (!env->envConfig->enabled)
    return;

  EnvReset(env);
  env->triggered = TRUE;
  env->hold = TRUE;
}



BOOL EnvProcess(Envelope *env) {

  if (env == NULL)
    return FALSE;

  if (env->envConfig == NULL)
    return FALSE;

  if (!env->envConfig->enabled)
    return FALSE;

  if (env->envConfig->numPoints <= 1)
    return FALSE;

  if (!env->triggered)
    return FALSE;

  /* Only process if we're not at the sustain point */
  if (!(env->hold && (env->curPoint == env->envConfig->sustain))) {

    /* ... and only when we're not beyond the last env point */
    if (env->position < env->envConfig->envPoints[env->envConfig->numPoints-1].x) {

      s32 x1, y1, x2, y2, dx, dy;
      s32 relativePos;

      x1 = env->envConfig->envPoints[env->curPoint].x;
      y1 = env->envConfig->envPoints[env->curPoint].y;
      x2 = env->envConfig->envPoints[env->curPoint + 1].x;
      y2 = env->envConfig->envPoints[env->curPoint + 1].y;

      dx = x2 - x1;
      dy = y2 - y1;

      env->position++;
      relativePos = env->position - x1;
      env->value = ((dy * relativePos) / dx) + y1;

      if (env->position >= env->envConfig->envPoints[env->curPoint + 1].x) {

	env->curPoint++;
	/* Handle env loops */
	if ((env->envConfig->loop_end >= env->envConfig->loop_start) &&
	    ((env->envConfig->loop_end < env->envConfig->numPoints) &&
	     (env->envConfig->loop_start < env->envConfig->numPoints))) {

	  if (env->curPoint == env->envConfig->loop_end) {

	    env->curPoint = env->envConfig->loop_start;
	    env->position = env->envConfig->envPoints[env->curPoint].x;
	    env->value = env->envConfig->envPoints[env->curPoint].y;
	  }
	}
      }
    }
  }

  return TRUE;
}



void EnvRelease(Envelope *env) {

  if (env == NULL)
    return;

  if (env->envConfig == NULL)
    return;

  if (!env->envConfig->enabled)
    return;

  env->hold = FALSE;
}
