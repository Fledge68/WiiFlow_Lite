/* 7zDecode.h */

#if defined(_LZMA_OUT_READ) && !defined(_LZMA_IN_CB)
#error "Fixme: _LZMA_OUT_READ && _LZMA_IN_CB isn't currently possible!"
#endif

#ifndef __7Z_DECODE_H
#define __7Z_DECODE_H

#include "7zItem.h"
#include "7zAlloc.h"
#ifdef _LZMA_IN_CB
#include "7zIn.h"
#endif

SZ_RESULT SzDecode(const CFileSize *packSizes, const CFolder *folder,
    #ifdef _LZMA_IN_CB
    ISzInStream *stream,
    #else
    const Byte *inBuffer,
    #endif
    Byte *outBuffer, size_t outSize,
    size_t *outSizeProcessed, ISzAlloc *allocMain);

#ifdef _LZMA_OUT_READ
#ifndef _LZMA_TEMP_BUFFER_SIZE
#define _LZMA_TEMP_BUFFER_SIZE (2048) // size of the temporary buffer in bytes
#endif

SZ_RESULT SzDecode2(const CFileSize *packSizes, const CFolder *folder,
    ISzInStream *stream,
    Byte *outBuffer, size_t outSize,
    size_t *outSizeProcessed, ISzAlloc *allocMain,
	size_t *fileOffset, size_t *fileSize);
#endif // #ifdef _LZMA_OUT_READ

#endif
