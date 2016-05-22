/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May 2007-July 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * freeze.cpp
 ***************************************************************************/

#include <malloc.h>
#include <gccore.h>
#include <stdio.h>

#include "snes9xgx.h"
#include "fileop.h"
#include "filebrowser.h"
#include "menu.h"
#include "video.h"
#include "utils/pngu.h"

#include "snes9x/snes9x.h"
#include "snes9x/port.h"
#include "snes9x/memmap.h"
#include "snes9x/snapshot.h"
#include "snes9x/language.h"

bool8 S9xOpenSnapshotFile(const char *filepath, bool8 readonly, STREAM *file)
{
	return FALSE;
}

void S9xCloseSnapshotFile(STREAM s)
{

}

/****************************************************************************
 * SaveSnapshot
 ***************************************************************************/

int
SaveSnapshot (char * filepath, bool silent)
{
	int device;

	if(!FindDevice(filepath, &device))
		return 0;

	// save screenshot
	if(gameScreenPngSize > 0)
	{
		char screenpath[1024];
		strcpy(screenpath, filepath);
		screenpath[strlen(screenpath)-4] = 0;
		sprintf(screenpath, "%s.png", screenpath);
		SaveFile((char *)gameScreenPng, screenpath, gameScreenPngSize, silent);
	}

	STREAM fp = OPEN_STREAM(filepath, "wb");
	
	if(!fp)
	{
		if(!silent)
			ErrorPrompt("Save failed!");
		return 0;
	}

	S9xFreezeToStream(fp);
	CLOSE_STREAM(fp);

	if(!silent)
		InfoPrompt("Save successful");
	return 1;
}

int
SaveSnapshotAuto (bool silent)
{
	char filepath[1024];

	if(!MakeFilePath(filepath, FILE_SNAPSHOT, Memory.ROMFilename, 0))
		return false;

	return SaveSnapshot(filepath, silent);
}

/****************************************************************************
 * LoadSnapshot
 ***************************************************************************/
int
LoadSnapshot (char * filepath, bool silent)
{
	int device;
				
	if(!FindDevice(filepath, &device))
		return 0;

	STREAM fp = OPEN_STREAM(filepath, "rb");

	if(!fp)
	{
		if(!silent)
			ErrorPrompt("Unable to open snapshot!");
		return 0;
	}

	int	result = S9xUnfreezeFromStream(fp);
	CLOSE_STREAM(fp);

	if (result == SUCCESS)
		return 1;

	switch (result)
	{
		case WRONG_FORMAT:
			ErrorPrompt(SAVE_ERR_WRONG_FORMAT);
			break;

		case WRONG_VERSION:
			ErrorPrompt(SAVE_ERR_WRONG_VERSION);
			break;

		case SNAPSHOT_INCONSISTENT:
			ErrorPrompt(MOVIE_ERR_SNAPSHOT_INCONSISTENT);
			break;
	}
	return 0;
}

int
LoadSnapshotAuto (bool silent)
{
	char filepath[1024];

	if(!MakeFilePath(filepath, FILE_SNAPSHOT, Memory.ROMFilename, 0))
		return false;

	return LoadSnapshot(filepath, silent);
}

/****************************************************************************
 * SavePreview
 ***************************************************************************/

int
SavePreviewImg (char * filepath, bool silent)
{
	int device;

	if(!FindDevice(filepath, &device))
		return 0;

	// save screenshot
	if(gameScreenPngSize > 0)
	{
		char screenpath[1024];
		strcpy(screenpath, filepath);
		screenpath[strlen(screenpath)] = 0;
		sprintf(screenpath, "%s.png", screenpath);
		SaveFile((char *)gameScreenPng, screenpath, gameScreenPngSize, silent);
	}

	if(!silent)
		InfoPrompt("Save successful");
	return 1;
}