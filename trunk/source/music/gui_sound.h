/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef GUI_SOUND_H_
#define GUI_SOUND_H_

#include <gccore.h>
#include <string>

//!Sound conversion and playback. A wrapper for other sound libraries - ASND, libmad, ltremor, etc
class GuiSound
{
public:
	//!Constructor
	GuiSound();
	//!Copy Constructor
	GuiSound(GuiSound *g);
	//!Constructor
	//!\param sound Pointer to the sound data
	//!\param filesize Length of sound data
	GuiSound(const char *path, int voice = -1);
	GuiSound(const u8 * snd, u32 len, const char *name, bool allocated = false, int voice = -1);
	//!Stops sound and frees all memory/closes files
	void FreeMemory();
	//!Destructor
	~GuiSound();
	//!Load a file and replace the old one
	bool Load(const char *path);
	//!Load a file and replace the old one
	bool Load(const u8 * snd, u32 len, bool allocated = true);
	//!For quick playback of the internal soundeffects
	bool LoadSoundEffect(const u8 * snd, u32 len);
	//!Start sound playback
	void Play();
	//!Start sound playback
	void Play(int vol, bool restart = false);
	//!Stop sound playback
	void Stop();
	//!Pause sound playback
	void Pause();
	//!Resume sound playback
	void Resume();
	//!Checks if a sound is currently loaded
	//!\return true if sound is loaded, false otherwise
	bool IsLoaded() { return sound != NULL; };
	//!Get the filepath for finding sounds which already have an instance.
	//!\return the current instance's filepath
	const char *GetName() { return filepath; };
	//!Checks if the sound is currently playing
	//!\return true if sound is playing, false otherwise
	bool IsPlaying();
	//!Rewind the music
	void Rewind();
	//!Get sound volume
	//!\returns the current sound volume
	int GetVolume();
	//!Set sound volume
	//!\param v Sound volume (0-100)
	void SetVolume(int v);
	//!\param l Loop (true to loop)
	void SetLoop(u8 l);
	//!Needed for music etc
	void SetVoice(s8 v);
	//!Needed for music :P
	s8 GetVoice() { return voice; }
private:
	//!Initializes the GuiSound object by setting the default values
	void Init();
protected:
	char filepath[1024]; /* Wasting Memory but who knows what people wanna load */
	u8 *sound; //!< Pointer to the sound data
	u32 length; //!< Length of sound data
	s8 voice; //!< Currently assigned ASND voice channel
	int volume; //!< Sound volume (0-100)
	u8 loop; //!< Loop sound playback
	u32 SoundEffectLength; //!< Check if it is an app soundeffect for faster playback
	bool allocated; //!< Is the file allocated or not
};

void soundInit(void);
void soundDeinit(void);

#endif
