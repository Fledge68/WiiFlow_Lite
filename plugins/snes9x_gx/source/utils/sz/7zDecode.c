/* 7zDecode.c */

#include "7zDecode.h"
#ifdef _SZ_ONE_DIRECTORY
#include "LzmaDecode.h"
#else
#include "../../Compress/LZMA_C/LzmaDecode.h"
#endif

#ifdef _LZMA_OUT_READ
#include <string.h> // for memcpy
#endif

CMethodID k_Copy = { { 0x0 }, 1 };
CMethodID k_LZMA = { { 0x3, 0x1, 0x1 }, 3 };

#ifdef _LZMA_IN_CB

typedef struct _CLzmaInCallbackImp
{
  ILzmaInCallback InCallback;
  ISzInStream *InStream;
  size_t Size;
} CLzmaInCallbackImp;

int LzmaReadImp(void *object, const unsigned char **buffer, SizeT *size)
{
  CLzmaInCallbackImp *cb = (CLzmaInCallbackImp *)object;
  size_t processedSize;
  SZ_RESULT res;
  *size = 0;
  res = cb->InStream->Read((void *)cb->InStream, (void **)buffer, cb->Size, &processedSize);
  *size = (SizeT)processedSize;
  if (processedSize > cb->Size)
    return (int)SZE_FAIL;
  cb->Size -= processedSize;
  if (res == SZ_OK)
    return 0;
  return (int)res;
}

#endif

SZ_RESULT SzDecode(const CFileSize *packSizes, const CFolder *folder,
    #ifdef _LZMA_IN_CB
    ISzInStream *inStream,
    #else
    const Byte *inBuffer,
    #endif
    Byte *outBuffer, size_t outSize,
    size_t *outSizeProcessed, ISzAlloc *allocMain)
{
  UInt32 si;
  size_t inSize = 0;
  CCoderInfo *coder;
  if (folder->NumPackStreams != 1)
    return SZE_NOTIMPL;
  if (folder->NumCoders != 1)
    return SZE_NOTIMPL;
  coder = folder->Coders;
  *outSizeProcessed = 0;

  for (si = 0; si < folder->NumPackStreams; si++)
    inSize += (size_t)packSizes[si];

  if (AreMethodsEqual(&coder->MethodID, &k_Copy))
  {
    size_t i;
    if (inSize != outSize)
      return SZE_DATA_ERROR;
    #ifdef _LZMA_IN_CB
    for (i = 0; i < inSize;)
    {
      size_t j;
      Byte *inBuffer;
      size_t bufferSize;
      RINOK(inStream->Read((void *)inStream,  (void **)&inBuffer, inSize - i, &bufferSize));
      if (bufferSize == 0)
        return SZE_DATA_ERROR;
      if (bufferSize > inSize - i)
        return SZE_FAIL;
      *outSizeProcessed += bufferSize;
      for (j = 0; j < bufferSize && i < inSize; j++, i++)
        outBuffer[i] = inBuffer[j];
    }
    #else
    for (i = 0; i < inSize; i++)
      outBuffer[i] = inBuffer[i];
    *outSizeProcessed = inSize;
    #endif
    return SZ_OK;
  }

  if (AreMethodsEqual(&coder->MethodID, &k_LZMA))
  {
    #ifdef _LZMA_IN_CB
    CLzmaInCallbackImp lzmaCallback;
    #else
    SizeT inProcessed;
    #endif

    CLzmaDecoderState state;  /* it's about 24-80 bytes structure, if int is 32-bit */
    int result;
    SizeT outSizeProcessedLoc;

    #ifdef _LZMA_IN_CB
    lzmaCallback.Size = inSize;
    lzmaCallback.InStream = inStream;
    lzmaCallback.InCallback.Read = LzmaReadImp;
    #endif

    if (LzmaDecodeProperties(&state.Properties, coder->Properties.Items,
        coder->Properties.Capacity) != LZMA_RESULT_OK)
      return SZE_FAIL;

    state.Probs = (CProb *)allocMain->Alloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));
    if (state.Probs == 0)
      return SZE_OUTOFMEMORY;

    #ifdef _LZMA_OUT_READ
    if (state.Properties.DictionarySize == 0)
      state.Dictionary = 0;
    else
    {
      state.Dictionary = (unsigned char *)allocMain->Alloc(state.Properties.DictionarySize);
      if (state.Dictionary == 0)
      {
        allocMain->Free(state.Probs);
        return SZE_OUTOFMEMORYDIC;
      }
    }
    LzmaDecoderInit(&state);
    #endif

    result = LzmaDecode(&state,
        #ifdef _LZMA_IN_CB
        &lzmaCallback.InCallback,
        #else
        inBuffer, (SizeT)inSize, &inProcessed,
        #endif
        outBuffer, (SizeT)outSize, &outSizeProcessedLoc);
    *outSizeProcessed = (size_t)outSizeProcessedLoc;
    allocMain->Free(state.Probs);
    #ifdef _LZMA_OUT_READ
    allocMain->Free(state.Dictionary);
    #endif
    if (result == LZMA_RESULT_DATA_ERROR)
      return SZE_DATA_ERROR;
    if (result != LZMA_RESULT_OK)
      return SZE_FAIL;
    return SZ_OK;
  }
  return SZE_NOTIMPL;
}

#ifdef _LZMA_OUT_READ
// like SzDecode but uses less memory
SZ_RESULT SzDecode2(const CFileSize *packSizes, const CFolder *folder,
    ISzInStream *inStream,
    Byte *outBuffer, size_t outSize,
    size_t *outSizeProcessed, ISzAlloc *allocMain,
	size_t *fileOffset, size_t *fileSize)
{
  UInt32 si;
  size_t inSize = 0;
  CCoderInfo *coder;
  if (folder->NumPackStreams != 1)
    return SZE_NOTIMPL;
  if (folder->NumCoders != 1)
    return SZE_NOTIMPL;
  coder = folder->Coders;
  *outSizeProcessed = 0;

  for (si = 0; si < folder->NumPackStreams; si++)
    inSize += (size_t)packSizes[si];

  if (AreMethodsEqual(&coder->MethodID, &k_Copy))
  {
    size_t i;
    if (inSize != outSize)
      return SZE_DATA_ERROR;
    #ifdef _LZMA_IN_CB
    for (i = 0; i < inSize;)
    {
      size_t j;
      Byte *inBuffer;
      size_t bufferSize;
      RINOK(inStream->Read((void *)inStream,  (void **)&inBuffer, inSize - i, &bufferSize));
      if (bufferSize == 0)
        return SZE_DATA_ERROR;
      if (bufferSize > inSize - i)
        return SZE_FAIL;
      *outSizeProcessed += bufferSize;
      for (j = 0; j < bufferSize && i < inSize; j++, i++)
        outBuffer[i] = inBuffer[j];
    }
    #else
    for (i = 0; i < inSize; i++)
      outBuffer[i] = inBuffer[i];
    *outSizeProcessed = inSize;
    #endif
    return SZ_OK;
  }

  if (AreMethodsEqual(&coder->MethodID, &k_LZMA))
  {
    #ifdef _LZMA_IN_CB
    CLzmaInCallbackImp lzmaCallback;
    #else
    SizeT inProcessed;
    #endif

    CLzmaDecoderState state;  /* it's about 24-80 bytes structure, if int is 32-bit */
    int result;
    SizeT outSizeProcessedLoc;

    #ifdef _LZMA_IN_CB
    lzmaCallback.Size = inSize;
    lzmaCallback.InStream = inStream;
    lzmaCallback.InCallback.Read = LzmaReadImp;
    #endif

    if (LzmaDecodeProperties(&state.Properties, coder->Properties.Items,
        coder->Properties.Capacity) != LZMA_RESULT_OK)
      return SZE_FAIL;

    state.Probs = (CProb *)allocMain->Alloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));
    if (state.Probs == 0)
      return SZE_OUTOFMEMORY;

    if (state.Properties.DictionarySize == 0)
      state.Dictionary = 0;
    else
    {
      state.Dictionary = (unsigned char *)allocMain->Alloc(state.Properties.DictionarySize);
      if (state.Dictionary == 0)
      {
        allocMain->Free(state.Probs);
        return SZE_OUTOFMEMORYDIC;
      }
    }
    LzmaDecoderInit(&state);

    // allocate memory for the temporary buffer
    Byte *tmpBuffer = (Byte *)allocMain->Alloc(_LZMA_TEMP_BUFFER_SIZE);

    // variables containing the number of the first and the last bytes of the buffer
        size_t bufferStart, bufferEnd;
        bufferStart = bufferEnd = 0;

        // integers contains the offset, the size and the already copied data which will be
        // copied from the tmpBuffer to outBuffer
        size_t copyOffset, copySize, copyDone;
        copyOffset = copySize = copyDone = 0;

        UInt32 i = 0;
    	int bytesToCopy = 0;

        // decompress data in _LZMA_TEMP_BUFFER_SIZE byte steps and copy the wanted file to outBuffer
        do
        {
    		if((*fileSize - copyDone) >= _LZMA_TEMP_BUFFER_SIZE)
    			bytesToCopy = _LZMA_TEMP_BUFFER_SIZE;
    		else
    			bytesToCopy = (*fileSize - copyDone);

    		// decompress next bytes
    		result = LzmaDecode(&state,
           	                    #ifdef _LZMA_IN_CB
           	                    &lzmaCallback.InCallback,
                    	        #else
           	                    //inBuffer, (SizeT)inSize, &inProcessed, //TODO!
                    	        #endif
                    	        tmpBuffer, bytesToCopy, &outSizeProcessedLoc
                    	        );

            // check result
    		if(result == LZMA_RESULT_DATA_ERROR)
    		{
    			return SZE_DATA_ERROR;
    		}
    		if(result != LZMA_RESULT_OK)
    		{
    			return SZE_FAIL;
    		}

    		// normally this should never happen
    		if(outSizeProcessedLoc > _LZMA_TEMP_BUFFER_SIZE)
    		{
    			return SZE_FAIL;
    		}

    		// update bufferStart and bufferEnd
    		bufferStart = _LZMA_TEMP_BUFFER_SIZE * i;
    		bufferEnd = bufferStart + outSizeProcessedLoc;
    		i++;

    		// calculate copy offset and size
    		if(*fileOffset > bufferEnd)
    		{
    			// we haven't reached the start of the file yet
    			continue;
    		}

    		// calculate offset
    		if(*fileOffset < bufferStart)
    		{
    			// the file has already started before this decompression step
    			copyOffset = 0;
    		}
    		else
    		{
    			// the file starts somewhere inside this buffer
    			copyDone = 0;
    			copyOffset = _LZMA_TEMP_BUFFER_SIZE - (bufferEnd - *fileOffset);
    		}

    		// calculate size
    		if((*fileOffset + *fileSize) > bufferEnd)
    		{
    			// we'll need the whole buffer after copyOffset
    			copySize = _LZMA_TEMP_BUFFER_SIZE - copyOffset;
    		}
    		else
    		{
    			// we'll stop somewhere inside the buffer
    			copySize = (*fileOffset + *fileSize) - (bufferStart + copyOffset);
    		}

    		// copy bytes to the real output buffer
    		if(copySize == 0)
    		{
    			continue;
    		}
    	//	printf("memcpy(outBuffer + %d, tmpBuffer + %d, %d)\n", copyDone, copyOffset, copySize);
    		memcpy(outBuffer + copyDone, tmpBuffer + copyOffset, copySize);
    		copyDone += copySize;
    	}
        while((*fileOffset + *fileSize) > bufferEnd);

    /*    result = LzmaDecode(&state,
            #ifdef _LZMA_IN_CB
            &lzmaCallback.InCallback,
            #else
            inBuffer, (SizeT)inSize, &inProcessed,
            #endif
            outBuffer, (SizeT)outSize, &outSizeProcessedLoc);*/
        //*outSizeProcessed = (size_t)outSizeProcessedLoc;
        *outSizeProcessed = copyDone;
        allocMain->Free(tmpBuffer); // free the temporary buffer again
        allocMain->Free(state.Probs);
        allocMain->Free(state.Dictionary);
    /*    if (result == LZMA_RESULT_DATA_ERROR)
          return SZE_DATA_ERROR;
        if (result != LZMA_RESULT_OK)
          return SZE_FAIL;*/
        return SZ_OK;
      }
      return SZE_NOTIMPL;
    }
    #endif
