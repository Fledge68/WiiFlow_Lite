#ifndef SAVEPATH_H_
#define SAVEPATH_H_

#include "disc.h"

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

void CreateTitleTMD(const char *path, struct dir_discHdr *hdr);

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif