/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * Tantric 2008-2010
 *
 * preferences.h
 *
 * Preferences save/load to XML file
 ***************************************************************************/

void FixInvalidSettings();
void DefaultSettings();
bool SavePrefs (bool silent);
bool LoadPrefs ();
