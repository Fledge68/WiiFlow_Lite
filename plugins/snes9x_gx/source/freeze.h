/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May 2007-July 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * freeze.h
 ***************************************************************************/

#ifndef _FREEZE_H_
#define _FREEZE_H_

int SaveSnapshot (char * filepath, bool silent);
int SaveSnapshotAuto (bool silent);
int LoadSnapshot (char * filepath, bool silent);
int LoadSnapshotAuto (bool silent);
int SavePreviewImg (char * filepath, bool silent);
#endif
