/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * crunchy2 April 2007-July 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * sram.cpp
 *
 * SRAM save/load/import/export handling
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "snes9xgx.h"
#include "menu.h"
#include "fileop.h"
#include "filebrowser.h"
#include "input.h"
#include "snes9x/snes9x.h"
#include "snes9x/memmap.h"
#include "snes9x/srtc.h"

/****************************************************************************
 * Load SRAM
 ***************************************************************************/
bool
LoadSRAM (char * filepath, bool silent)
{
	int len = 0;
	int device;
	bool result = false;

	if(!FindDevice(filepath, &device))
		return 0;

	Memory.ClearSRAM();

	int size = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;

	if (size > 0x20000)
		size = 0x20000;

	if (size)
	{
		len = LoadFile((char *)Memory.SRAM, filepath, size, silent);

		if (len > 0)
		{
			if (len - size == 512)
				memmove(Memory.SRAM, Memory.SRAM + 512, size);

			if (Settings.SRTC || Settings.SPC7110RTC)
			{
				int pathlen = strlen(filepath);
				filepath[pathlen-3] = 'r';
				filepath[pathlen-2] = 't';
				filepath[pathlen-1] = 'c';
				LoadFile((char *)RTCData.reg, filepath, 20, silent);
			}
			result = true;
		}
		else if(!silent)
		{
			// if we reached here, nothing was done!
			ErrorPrompt("SRAM file not found");
		}
		S9xSoftReset();
	}
	return result;
}

bool
LoadSRAMAuto (bool silent)
{
	char filepath[MAXPATHLEN];

	// look for Auto save file
	if(!MakeFilePath(filepath, FILE_SRAM, Memory.ROMFilename, 0))
		return false;

	if (LoadSRAM(filepath, silent))
		return true;

	// look for file with no number or Auto appended
	if(!MakeFilePath(filepath, FILE_SRAM, Memory.ROMFilename, -1))
		return false;

	if(LoadSRAM(filepath, silent))
		return true;

	return false;
}

/****************************************************************************
 * Save SRAM
 ***************************************************************************/
bool
SaveSRAM (char * filepath, bool silent)
{
	bool retval = false;
	int offset = 0;
	int device;

	if(!FindDevice(filepath, &device))
		return 0;

	if (Settings.SuperFX && Memory.ROMType < 0x15) // doesn't have SRAM
		return true;

	if (Settings.SA1 && Memory.ROMType == 0x34)    // doesn't have SRAM
		return true;

	// determine SRAM size
	int size = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;

	if (size > 0x20000)
		size = 0x20000;

	if (size > 0)
	{
		offset = SaveFile((char *)Memory.SRAM, filepath, size, silent);

		if (Settings.SRTC || Settings.SPC7110RTC)
		{
			int pathlen = strlen(filepath);
			filepath[pathlen-3] = 'r';
			filepath[pathlen-2] = 't';
			filepath[pathlen-1] = 'c';
			SaveFile((char *)RTCData.reg, filepath, 20, silent);
		}

		if (offset > 0)
		{
			if (!silent)
				InfoPrompt("Save successful");
			retval = true;
		}
	}
	else
	{
		if(!silent)
			ErrorPrompt("No SRAM data to save!");
	}
	return retval;
}

bool
SaveSRAMAuto (bool silent)
{
	char filepath[1024];

	// look for file with no number or Auto appended
	if(!MakeFilePath(filepath, FILE_SRAM, Memory.ROMFilename, -1))
		return false;

	FILE * fp = fopen (filepath, "rb");

	if(fp) // file found
	{
		fclose (fp);
	}	
	else
	{
		if(!MakeFilePath(filepath, FILE_SRAM, Memory.ROMFilename, 0))
			return false;
	}

	return SaveSRAM(filepath, silent);
}
