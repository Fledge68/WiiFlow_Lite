/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * svpe June 2007
 * crunchy2 May-July 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * filebrowser.cpp
 *
 * Generic file routines - reading, writing, browsing
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <sys/dir.h>
#include <malloc.h>

#ifdef HW_RVL
#include <di/di.h>
#endif

#include "snes9xgx.h"
#include "filebrowser.h"
#include "menu.h"
#include "video.h"
#include "networkop.h"
#include "fileop.h"
#include "input.h"
#include "gcunzip.h"
#include "freeze.h"
#include "sram.h"

#include "snes9x/snes9x.h"
#include "snes9x/memmap.h"
#include "snes9x/cheats.h"

BROWSERINFO browser;
BROWSERENTRY * browserList = NULL; // list of files/folders in browser

static char szpath[MAXPATHLEN];
char szname[MAXPATHLEN];
bool inSz = false;

unsigned long SNESROMSize = 0;
bool loadingFile = false;

/****************************************************************************
* autoLoadMethod()
* Auto-determines and sets the load device
* Returns device set
****************************************************************************/
int autoLoadMethod()
{
	ShowAction ("Attempting to determine load device...");

	int device = DEVICE_AUTO;

	if(ChangeInterface(DEVICE_SD, SILENT))
		device = DEVICE_SD;
	else if(ChangeInterface(DEVICE_USB, SILENT))
		device = DEVICE_USB;
	else if(ChangeInterface(DEVICE_SD_SLOTA, SILENT))
		device = DEVICE_SD_SLOTA;
	else if(ChangeInterface(DEVICE_SD_SLOTB, SILENT))
		device = DEVICE_SD_SLOTB;
	else if(ChangeInterface(DEVICE_DVD, SILENT))
		device = DEVICE_DVD;
	else if(ChangeInterface(DEVICE_SMB, SILENT))
		device = DEVICE_SMB;
	else
		ErrorPrompt("Unable to locate a load device!");

	if(GCSettings.LoadMethod == DEVICE_AUTO)
		GCSettings.LoadMethod = device; // save device found for later use
	CancelAction();
	return device;
}

/****************************************************************************
* autoSaveMethod()
* Auto-determines and sets the save device
* Returns device set
****************************************************************************/
int autoSaveMethod(bool silent)
{
	if(!silent)
		ShowAction ("Attempting to determine save device...");

	int device = DEVICE_AUTO;

	if(ChangeInterface(DEVICE_SD, SILENT))
		device = DEVICE_SD;
	else if(ChangeInterface(DEVICE_USB, SILENT))
		device = DEVICE_USB;
	else if(ChangeInterface(DEVICE_SD_SLOTA, SILENT))
		device = DEVICE_SD_SLOTA;
	else if(ChangeInterface(DEVICE_SD_SLOTB, SILENT))
		device = DEVICE_SD_SLOTB;
	else if(ChangeInterface(DEVICE_SMB, SILENT))
		device = DEVICE_SMB;
	else if(!silent)
		ErrorPrompt("Unable to locate a save device!");

	if(GCSettings.SaveMethod == DEVICE_AUTO)
		GCSettings.SaveMethod = device; // save device found for later use

	CancelAction();
	return device;
}

/****************************************************************************
 * ResetBrowser()
 * Clears the file browser memory, and allocates one initial entry
 ***************************************************************************/
void ResetBrowser()
{
	browser.numEntries = 0;
	browser.selIndex = 0;
	browser.pageIndex = 0;
	browser.size = 0;
}

bool AddBrowserEntry()
{
	if(browser.size >= MAX_BROWSER_SIZE)
	{
		ErrorPrompt("Out of memory: too many files!");
		return false; // out of space
	}

	memset(&(browserList[browser.size]), 0, sizeof(BROWSERENTRY)); // clear the new entry
	browser.size++;
	return true;
}

/****************************************************************************
 * CleanupPath()
 * Cleans up the filepath, removing double // and replacing \ with /
 ***************************************************************************/
static void CleanupPath(char * path)
{
	if(!path || path[0] == 0)
		return;
	
	int pathlen = strlen(path);
	int j = 0;
	for(int i=0; i < pathlen && i < MAXPATHLEN; i++)
	{
		if(path[i] == '\\')
			path[i] = '/';

		if(j == 0 || !(path[j-1] == '/' && path[i] == '/'))
			path[j++] = path[i];
	}
	path[j] = 0;
}

bool IsDeviceRoot(char * path)
{
	if(path == NULL || path[0] == 0)
		return false;

	if( strcmp(path, "sd:/")    == 0 ||
		strcmp(path, "usb:/")   == 0 ||
		strcmp(path, "dvd:/")   == 0 ||
		strcmp(path, "smb:/")   == 0 ||
		strcmp(path, "carda:/") == 0 ||
		strcmp(path, "cardb:/") == 0)
	{
		return true;
	}
	return false;
}

/****************************************************************************
 * UpdateDirName()
 * Update curent directory name for file browser
 ***************************************************************************/
int UpdateDirName()
{
	int size=0;
	char * test;
	char temp[1024];
	int device = 0;

	if(browser.numEntries == 0)
		return 1;

	FindDevice(browser.dir, &device);

	/* current directory doesn't change */
	if (strcmp(browserList[browser.selIndex].filename,".") == 0)
	{
		return 0;
	}
	/* go up to parent directory */
	else if (strcmp(browserList[browser.selIndex].filename,"..") == 0)
	{
		// already at the top level
		if(IsDeviceRoot(browser.dir))
		{
			browser.dir[0] = 0; // remove device - we are going to the device listing screen
		}
		else
		{
			/* determine last subdirectory namelength */
			sprintf(temp,"%s",browser.dir);
			test = strtok(temp,"/");
			while (test != NULL)
			{
				size = strlen(test);
				test = strtok(NULL,"/");
			}
	
			/* remove last subdirectory name */
			size = strlen(browser.dir) - size - 1;
			strncpy(GCSettings.LastFileLoaded, &browser.dir[size], strlen(browser.dir) - size - 1); //set as loaded file the previous dir
			GCSettings.LastFileLoaded[strlen(browser.dir) - size - 1] = 0;
			browser.dir[size] = 0;
		}

		return 1;
	}
	/* Open a directory */
	else
	{
		/* test new directory namelength */
		if ((strlen(browser.dir)+1+strlen(browserList[browser.selIndex].filename)) < MAXPATHLEN)
		{
			/* update current directory name */
			sprintf(browser.dir, "%s%s/",browser.dir, browserList[browser.selIndex].filename);
			return 1;
		}
		else
		{
			ErrorPrompt("Directory name is too long!");
			return -1;
		}
	}
}

bool MakeFilePath(char filepath[], int type, char * filename, int filenum)
{
	char file[512];
	char folder[1024];
	char ext[4];
	char temppath[MAXPATHLEN];

	if(type == FILE_ROM)
	{
		// Check path length
		if ((strlen(browser.dir)+1+strlen(browserList[browser.selIndex].filename)) >= MAXPATHLEN)
		{
			ErrorPrompt("Maximum filepath length reached!");
			filepath[0] = 0;
			return false;
		}
		else
		{
			sprintf(temppath, "%s%s",browser.dir,browserList[browser.selIndex].filename);
		}
	}
	else
	{
		if(GCSettings.SaveMethod == DEVICE_AUTO)
			GCSettings.SaveMethod = autoSaveMethod(SILENT);

		if(GCSettings.SaveMethod == DEVICE_AUTO)
			return false;

		switch(type)
		{
			case FILE_SRAM:
			case FILE_SNAPSHOT:
				sprintf(folder, GCSettings.SaveFolder);

				if(type == FILE_SRAM) sprintf(ext, "srm");
				else sprintf(ext, "frz");

				if(filenum >= -1)
				{
					if(filenum == -1)
						sprintf(file, "%s.%s", filename, ext);
					else if(filenum == 0)
						sprintf(file, "%s Auto.%s", filename, ext);
					else
						sprintf(file, "%s %i.%s", filename, filenum, ext);
				}
				else
				{
					sprintf(file, "%s", filename);
				}
				break;
			case FILE_CHEAT:
				sprintf(folder, GCSettings.CheatFolder);
				sprintf(file, "%s.cht", Memory.ROMFilename);
				break;
		}
		sprintf (temppath, "%s%s/%s", pathPrefix[GCSettings.SaveMethod], folder, file);
	}
	CleanupPath(temppath); // cleanup path
	snprintf(filepath, MAXPATHLEN, "%s", temppath);
	return true;
}

/****************************************************************************
 * FileSortCallback
 *
 * Quick sort callback to sort file entries with the following order:
 *   .
 *   ..
 *   <dirs>
 *   <files>
 ***************************************************************************/
int FileSortCallback(const void *f1, const void *f2)
{
	/* Special case for implicit directories */
	if(((BROWSERENTRY *)f1)->filename[0] == '.' || ((BROWSERENTRY *)f2)->filename[0] == '.')
	{
		if(strcmp(((BROWSERENTRY *)f1)->filename, ".") == 0) { return -1; }
		if(strcmp(((BROWSERENTRY *)f2)->filename, ".") == 0) { return 1; }
		if(strcmp(((BROWSERENTRY *)f1)->filename, "..") == 0) { return -1; }
		if(strcmp(((BROWSERENTRY *)f2)->filename, "..") == 0) { return 1; }
	}

	/* If one is a file and one is a directory the directory is first. */
	if(((BROWSERENTRY *)f1)->isdir && !(((BROWSERENTRY *)f2)->isdir)) return -1;
	if(!(((BROWSERENTRY *)f1)->isdir) && ((BROWSERENTRY *)f2)->isdir) return 1;

	return stricmp(((BROWSERENTRY *)f1)->filename, ((BROWSERENTRY *)f2)->filename);
}

/****************************************************************************
 * IsValidROM
 *
 * Checks if the specified file is a valid ROM
 * For now we will just check the file extension and file size
 * If the file is a zip, we will check the file extension / file size of the
 * first file inside
 ***************************************************************************/
static bool IsValidROM()
{
	// file size should be between 96K and 8MB
	if(browserList[browser.selIndex].length < (1024*96) ||
		browserList[browser.selIndex].length > Memory.MAX_ROM_SIZE)
	{
		ErrorPrompt("Invalid file size!");
		return false;
	}

	if (strlen(browserList[browser.selIndex].filename) > 4)
	{
		char * p = strrchr(browserList[browser.selIndex].filename, '.');

		if (p != NULL)
		{
			char * zippedFilename = NULL;
			
			if(stricmp(p, ".zip") == 0 && !inSz)
			{
				// we need to check the file extension of the first file in the archive
				zippedFilename = GetFirstZipFilename ();

				if(zippedFilename && strlen(zippedFilename) > 4)
					p = strrchr(zippedFilename, '.');
				else
					p = NULL;
			}

			if(p != NULL)
			{
				if (stricmp(p, ".smc") == 0 ||
					stricmp(p, ".fig") == 0 ||
					stricmp(p, ".sfc") == 0 ||
					stricmp(p, ".swc") == 0)
				{
					if(zippedFilename) free(zippedFilename);
					return true;
				}
			}
			if(zippedFilename) free(zippedFilename);
		}
	}
	ErrorPrompt("Unknown file type!");
	return false;
}

/****************************************************************************
 * IsSz
 *
 * Checks if the specified file is a 7z
 ***************************************************************************/
bool IsSz()
{
	if (strlen(browserList[browser.selIndex].filename) > 4)
	{
		char * p = strrchr(browserList[browser.selIndex].filename, '.');

		if (p != NULL)
			if(stricmp(p, ".7z") == 0)
				return true;
	}
	return false;
}

/****************************************************************************
 * StripExt
 *
 * Strips an extension from a filename
 ***************************************************************************/
void StripExt(char* returnstring, char * inputstring)
{
	char* loc_dot;

	snprintf (returnstring, MAXJOLIET, "%s", inputstring);

	if(inputstring == NULL || strlen(inputstring) < 4)
		return;

	loc_dot = strrchr(returnstring,'.');
	if (loc_dot != NULL)
		*loc_dot = 0; // strip file extension
}

/****************************************************************************
 * BrowserLoadSz
 *
 * Opens the selected 7z file, and parses a listing of the files within
 ***************************************************************************/
int BrowserLoadSz()
{
	memset(szpath, 0, MAXPATHLEN);
	strncpy(szpath, browser.dir, strlen(browser.dir) - 1);
	
	strncpy(szname, strrchr(szpath, '/') + 1, strrchr(szpath, '.') - strrchr(szpath, '/'));
	*strrchr(szname, '.') = '\0';

	int szfiles = SzParse(szpath);
	if(szfiles)
	{
		browser.numEntries = szfiles;
		inSz = true;
	}
	else
		ErrorPrompt("Error opening archive!");

	return szfiles;
}

int WiiFileLoader()
{
	int size;
	char filepath[1024];

	memset(Memory.NSRTHeader, 0, sizeof(Memory.NSRTHeader));
	Memory.HeaderCount = 0;
	loadingFile = true;

	if(!inSz)
	{
		if(!MakeFilePath(filepath, FILE_ROM))
			return 0;

		size = LoadFile ((char *)Memory.ROM, filepath, browserList[browser.selIndex].length, NOTSILENT);
	}
	else
	{
		size = LoadSzFile(szpath, (unsigned char *)Memory.ROM);

		if(size <= 0)
		{
			browser.selIndex = 0;
			BrowserChangeFolder();
		}
	}
	loadingFile = false;

	if(size <= 0)
		return 0;

	SNESROMSize = Memory.HeaderRemove(size, Memory.HeaderCount, Memory.ROM);
	return SNESROMSize;
}

/****************************************************************************
 * BrowserLoadFile
 *
 * Loads the selected ROM
 ***************************************************************************/
int BrowserLoadFile()
{
	int loaded = 0;
	int device;

	if(!FindDevice(browser.dir, &device))
		return 0;

	GetFileSize(browser.selIndex);

	// check that this is a valid ROM
	if(!IsValidROM())
		goto done;

	// store the filename (w/o ext) - used for sram/freeze naming
	StripExt(Memory.ROMFilename, browserList[browser.selIndex].filename);
	snprintf(GCSettings.LastFileLoaded, MAXPATHLEN, "%s", browserList[browser.selIndex].filename);

	SNESROMSize = 0;
	S9xDeleteCheats();
	Memory.LoadROM("ROM");

	if (SNESROMSize <= 0)
	{
		ErrorPrompt("Error loading game!");
	}
	else
	{
		// load SRAM or snapshot
		if (GCSettings.AutoLoad == 1)
			LoadSRAMAuto(SILENT);
		else if (GCSettings.AutoLoad == 2)
			LoadSnapshotAuto(SILENT);

		ResetBrowser();
		loaded = 1;
	}
done:
	CancelAction();
	return loaded;
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
int BrowserChangeFolder()
{
	int device = 0;
	FindDevice(browser.dir, &device);
	
	if(inSz && browser.selIndex == 0) // inside a 7z, requesting to leave
	{
		inSz = false;
		SzClose();
	}

	if(!UpdateDirName()) 
		return -1;

	HaltParseThread();
	CleanupPath(browser.dir);
	ResetBrowser();

	if(browser.dir[0] != 0)
	{
		if(strstr(browser.dir, ".7z"))
		{
			BrowserLoadSz();
		}
		else 
		{
			ParseDirectory(true, true);
		}
		FindAndSelectLastLoadedFile();
	}

	if(browser.numEntries == 0)
	{
		browser.dir[0] = 0;
		int i=0;
		
#ifdef HW_RVL
		AddBrowserEntry();
		sprintf(browserList[i].filename, "sd:/");
		sprintf(browserList[i].displayname, "SD Card");
		browserList[i].length = 0;
		browserList[i].isdir = 1;
		browserList[i].icon = ICON_SD;
		i++;

		AddBrowserEntry();
		sprintf(browserList[i].filename, "usb:/");
		sprintf(browserList[i].displayname, "USB Mass Storage");
		browserList[i].length = 0;
		browserList[i].isdir = 1;
		browserList[i].icon = ICON_USB;
		i++;
#else
		AddBrowserEntry();
		sprintf(browserList[i].filename, "carda:/");
		sprintf(browserList[i].displayname, "SD Gecko Slot A");
		browserList[i].length = 0;
		browserList[i].isdir = 1;
		browserList[i].icon = ICON_SD;
		i++;
		
		AddBrowserEntry();
		sprintf(browserList[i].filename, "cardb:/");
		sprintf(browserList[i].displayname, "SD Gecko Slot B");
		browserList[i].length = 0;
		browserList[i].isdir = 1;
		browserList[i].icon = ICON_SD;
		i++;
#endif
		AddBrowserEntry();
		sprintf(browserList[i].filename, "smb:/");
		sprintf(browserList[i].displayname, "Network Share");
		browserList[i].length = 0;
		browserList[i].isdir = 1;
		browserList[i].icon = ICON_SMB;
		i++;
		
		AddBrowserEntry();
		sprintf(browserList[i].filename, "dvd:/");
		sprintf(browserList[i].displayname, "Data DVD");
		browserList[i].length = 0;
		browserList[i].isdir = 1;
		browserList[i].icon = ICON_DVD;
		i++;
		
		browser.numEntries += i;
	}
	
	if(browser.dir[0] == 0)
	{
		GCSettings.LoadFolder[0] = 0;
		GCSettings.LoadMethod = 0;
	}
	else
	{
		char * path = StripDevice(browser.dir);
		if(path != NULL)
			strcpy(GCSettings.LoadFolder, path);
		FindDevice(browser.dir, &GCSettings.LoadMethod);
	}

	return browser.numEntries;
}

/****************************************************************************
 * OpenROM
 * Displays a list of ROMS on load device
 ***************************************************************************/
int
OpenGameList ()
{
	int device = GCSettings.LoadMethod;

	if(device == DEVICE_AUTO && strlen(GCSettings.LoadFolder) > 0)
		device = autoLoadMethod();

	// change current dir to roms directory
	if(device > 0)
		sprintf(browser.dir, "%s%s/", pathPrefix[device], GCSettings.LoadFolder);
	else
		browser.dir[0] = 0;
	
	BrowserChangeFolder();
	return browser.numEntries;
}
