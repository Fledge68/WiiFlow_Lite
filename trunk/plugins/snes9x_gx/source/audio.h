/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * Tantric 2008-2010
 *
 * audio.h
 *
 * Audio driver
 * Audio is fixed to 32Khz/16bit/Stereo
 ***************************************************************************/

void InitAudio ();
void AudioStart ();
void SwitchAudioMode(int mode);
void ShutdownAudio();
