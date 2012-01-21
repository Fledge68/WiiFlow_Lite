/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#ifndef __MOD_H__
#define __MOD_H__

#include "modplay.h"
#include "modplay_core.h"
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

int MODFILE_SetMOD(u8 *modfile, int modlength, MODFILE *mod);
BOOL MODFILE_IsMOD(u8 *modfile, int modlength);
int MODFILE_MODGetFormatID(void);
char *MODFILE_MODGetDescription(void);
char *MODFILE_MODGetAuthor(void);
char *MODFILE_MODGetVersion(void);
char *MODFILE_MODGetCopyright(void);


#ifdef __cplusplus
}
#endif

#endif
