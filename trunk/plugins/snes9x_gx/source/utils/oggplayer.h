/*
 Copyright (c) 2008 Francisco Muñoz 'Hermes' <www.elotrolado.net>
 All rights reserved.
 
 Proper (standard) vorbis usage by Tantric, 2009
 Threading modifications/corrections by Tantric, 2009

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice, 
 this list of conditions and the following disclaimer in the documentation 
 and/or other materials provided with the distribution.
 - The names of the contributors may not be used to endorse or promote products 
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE 
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __OGGPLAYER_H__
#define __OGGPLAYER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define OGG_ONE_TIME         0
#define OGG_INFINITE_TIME    1

#define OGG_STATUS_RUNNING   1
#define OGG_STATUS_ERR      -1
#define OGG_STATUS_PAUSED    2
#define OGG_STATUS_EOF     255

/****************************************************************************
 * PlayOgg
 *
 * Creates a thread that starts playing from the specific Ogg buffer
 * buffer - pointer to the start of the Ogg data
 * len - length of Ogg file
 * time_pos - initial time position at which to start playback
 * mode - playback mode (OGG_ONE_TIME or OGG_INFINITE_TIME)
 * returns: -1 on error, 0 on success
 ***************************************************************************/
int PlayOgg(const void *buffer, s32 len, int time_pos, int mode);

/****************************************************************************
 * StopOgg
 *
 * Stops playback. The player thread is shut down.
 ***************************************************************************/
void StopOgg();

/****************************************************************************
 * PauseOgg
 *
 * Pauses playback. 0 -> continue, 1-> pause
 ***************************************************************************/
void PauseOgg(int pause);

/****************************************************************************
 * StatusOgg
 *
 * Returns the Ogg player's status
 * returns: 
 * OGG_STATUS_RUNNING
 * OGG_STATUS_ERR    -> not initialized
 * OGG_STATUS_PAUSED
 * OGG_STATUS_EOF    -> player stopped by End Of File
 ***************************************************************************/
int StatusOgg();

/****************************************************************************
 * SetVolumeOgg
 *
 * Sets the Ogg playback volume (0 to 255 (max))
 ***************************************************************************/
void SetVolumeOgg(int volume);

/****************************************************************************
 * GetTimeOgg
 *
 * Gets current Ogg position
 * returns -1 on error, or the time in milliseconds from the start
 ***************************************************************************/
s32 GetTimeOgg();

/****************************************************************************
 * SetTimeOgg
 *
 * Sets the time position
 * time_pos: time position (in milliseconds) to advance
 ***************************************************************************/
void SetTimeOgg(s32 time_pos);

#ifdef __cplusplus
}
#endif

#endif
