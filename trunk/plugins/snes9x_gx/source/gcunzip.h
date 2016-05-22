/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * gcunzip.h
 *
 * File unzip routines
 ****************************************************************************/
#ifndef _GCUNZIP_H_
#define _GCUNZIP_H_

int IsZipFile (char *buffer);
char * GetFirstZipFilename();
size_t UnZipBuffer (unsigned char *outbuffer);
int SzParse(char * filepath);
size_t SzExtractFile(int i, unsigned char *buffer);
void SzClose();

#endif
