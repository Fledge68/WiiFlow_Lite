/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#ifndef __S3M_H__
#define __S3M_H__

#include "modplay.h"

#ifdef __cplusplus
extern "C" {
#endif

int MODFILE_SetS3M(u8 *s3mfile, int s3mlength, MODFILE *s3m);
BOOL MODFILE_IsS3M(u8 *modfile, int modlength);
int MODFILE_S3MGetFormatID(void);
char *MODFILE_S3MGetDescription(void);
char *MODFILE_S3MGetAuthor(void);
char *MODFILE_S3MGetVersion(void);
char *MODFILE_S3MGetCopyright(void);

#ifdef __cplusplus
}
#endif

#endif
