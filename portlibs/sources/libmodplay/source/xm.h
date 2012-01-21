/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#ifndef __XM_H__
#define __XM_H__

#include "modplay.h"

#ifdef __cplusplus
extern "C" {
#endif

int MODFILE_SetXM(u8 *xmfile, int xmlength, MODFILE *xm);
BOOL MODFILE_IsXM(u8 *xmfile, int xmlength);
int MODFILE_XMGetFormatID(void);
char *MODFILE_XMGetDescription(void);
char *MODFILE_XMGetAuthor(void);
char *MODFILE_XMGetVersion(void);
char *MODFILE_XMGetCopyright(void);

#ifdef __cplusplus
}
#endif


#endif
