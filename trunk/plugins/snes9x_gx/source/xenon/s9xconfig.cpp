/****************************************************************************
 * Snes9x 1.51 Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May 2007
 * Michniewski 2008
 * Tantric 2008-2009
 *
 * s9xconfig.cpp
 *
 * Configuration parameters are here for easy maintenance.
 * Refer to Snes9x.h for all combinations.
 * The defaults used here are taken directly from porting.html
 ***************************************************************************/

#include "snes9x.h"

// struct SGCSettings GCSettings;

/****************************************************************************
 * FixInvalidSettings
 *
 * Attempts to correct at least some invalid settings - the ones that
 * might cause crashes
 ***************************************************************************/
void FixInvalidSettings()
{

/*	if(!(GCSettings.ZoomLevel > 0.5 && GCSettings.ZoomLevel < 1.5))
		GCSettings.ZoomLevel = 1.0;
	if(!(GCSettings.xshift > -50 && GCSettings.xshift < 50))
		GCSettings.xshift = 0;
	if(!(GCSettings.yshift > -50 && GCSettings.yshift < 50))
		GCSettings.yshift = 0;
	if(!(GCSettings.MusicVolume >= 0 && GCSettings.MusicVolume <= 100))
		GCSettings.MusicVolume = 40;
	if(!(GCSettings.SFXVolume >= 0 && GCSettings.SFXVolume <= 100))
		GCSettings.SFXVolume = 40;
	if(GCSettings.Controller > CTRL_PAD4 || GCSettings.Controller < CTRL_MOUSE)
		GCSettings.Controller = CTRL_PAD2;
	if(!(GCSettings.render >= 0 && GCSettings.render < 3))
		GCSettings.render = 2;
	if(!(GCSettings.videomode >= 0 && GCSettings.videomode < 5))
		GCSettings.videomode = 0; */
}

/****************************************************************************
 * DefaultSettings
 *
 * Sets all the defaults!
 ***************************************************************************/
void
DefaultSettings ()
{
#if 0
	/************** GameCube/Wii Settings *********************/
	ResetControls(); // controller button mappings

	GCSettings.LoadMethod = METHOD_AUTO; // Auto, SD, DVD, USB, Network (SMB)
	GCSettings.SaveMethod = METHOD_AUTO; // Auto, SD, Memory Card Slot A, Memory Card Slot B, USB, Network (SMB)
	sprintf (GCSettings.LoadFolder,"snes9x/roms"); // Path to game files
	sprintf (GCSettings.SaveFolder,"snes9x/saves"); // Path to save files
	sprintf (GCSettings.CheatFolder,"snes9x/cheats"); // Path to cheat files
	GCSettings.AutoLoad = 1;
	GCSettings.AutoSave = 1;

	GCSettings.VerifySaves = 0;

	// custom SMB settings
	strncpy (GCSettings.smbip, "", 15); // IP Address of share server
	strncpy (GCSettings.smbuser, "", 19); // Your share user
	strncpy (GCSettings.smbpwd, "", 19); // Your share user password
	strncpy (GCSettings.smbshare, "", 19); // Share name on server

	GCSettings.smbip[15] = 0;
	GCSettings.smbuser[19] = 0;
	GCSettings.smbpwd[19] = 0;
	GCSettings.smbshare[19] = 0;

	GCSettings.Controller = CTRL_PAD2;

	GCSettings.videomode = 0; // automatic video mode detection
	GCSettings.ZoomLevel = 1.0; // zoom level
	GCSettings.render = 2; // Unfiltered
	GCSettings.widescreen = 0; // no aspect ratio correction
	GCSettings.FilterMethod = FILTER_NONE;	// no hq2x

	GCSettings.xshift = 0; // horizontal video shift
	GCSettings.yshift = 0; // vertical video shift

	GCSettings.WiimoteOrientation = 0;
	GCSettings.ExitAction = 0;
	GCSettings.MusicVolume = 40;
	GCSettings.SFXVolume = 40;
	GCSettings.Rumble = 1;
#endif

	/****************** SNES9x Settings ***********************/

	// Default ALL to false
	memset (&Settings, 0, sizeof (Settings));

	// General

	Settings.MouseMaster = false;
	Settings.SuperScopeMaster = false;
	Settings.MultiPlayer5Master = false;
	Settings.JustifierMaster = false;
	Settings.ShutdownMaster = true; // needs to be on for ActRaiser 2
	Settings.ApplyCheats = true;

	Settings.BlockInvalidVRAMAccess = false;
	Settings.HDMATimingHack = 100;

	// Sound defaults. On GC this is 32Khz/16bit/Stereo/InterpolatedSound
	Settings.APUEnabled = true;
	Settings.NextAPUEnabled = true;
	Settings.SoundPlaybackRate = 32000;
	Settings.Stereo = true;
	Settings.SixteenBitSound = true;
	Settings.SoundEnvelopeHeightReading = true;
	Settings.SoundSync = true;
	Settings.FixFrequency = false;
	Settings.DisableSampleCaching = true;
	Settings.InterpolatedSound = true;
	Settings.ReverseStereo = true;

	// Graphics
	Settings.Transparency = true;
	Settings.SupportHiRes = true;
	Settings.SkipFrames = AUTO_FRAMERATE;
	Settings.TurboSkipFrames = 19;
	Settings.DisplayFrameRate = false;
	Settings.AutoDisplayMessages = 0;
	Settings.InitialInfoStringTimeout = 200; // # frames to display messages for

	// Frame timings in 50hz and 60hz cpu mode
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;

	// SDD1 - Star Ocean Returns
	Settings.SDD1Pack = true;

	Settings.ForceNTSC = 0;
	Settings.ForcePAL = 0;
	Settings.ForceHiROM = 0;
	Settings.ForceLoROM = 0;
	Settings.ForceHeader = 0;
	Settings.ForceNoHeader = 0;
	Settings.ForceTransparency = 0;
	Settings.ForceInterleaved = 0;
	Settings.ForceInterleaved2 = 0;
	Settings.ForceInterleaveGD24 = 0;
	Settings.ForceNotInterleaved = 0;
	Settings.ForceNoSuperFX = 0;
	Settings.ForceSuperFX = 0;
	Settings.ForceDSP1 = 0;
	Settings.ForceNoDSP1 = 0;
}
