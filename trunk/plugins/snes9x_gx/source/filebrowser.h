/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * filebrowser.h
 *
 * Generic file routines - reading, writing, browsing
 ****************************************************************************/

#ifndef _FILEBROWSER_H_
#define _FILEBROWSER_H_

#include <unistd.h>
#include <gccore.h>

#define MAXJOLIET 255
#ifdef HW_DOL
#define MAX_BROWSER_SIZE	1000
#else
#define MAX_BROWSER_SIZE	5000
#endif

typedef struct
{
	char dir[MAXPATHLEN + 1]; // directory path of browserList
	int numEntries; // # of entries in browserList
	int selIndex; // currently selected index of browserList
	int pageIndex; // starting index of browserList page display
	int size; // # of entries browerList has space allocated to store
} BROWSERINFO;

typedef struct
{
	size_t length; // file length
	int isdir; // 0 - file, 1 - directory
	char filename[MAXJOLIET + 1]; // full filename
	char displayname[MAXJOLIET + 1]; // name for browser display
	int filenum; // file # (for 7z support)
	int icon; // icon to display
} BROWSERENTRY;

extern BROWSERINFO browser;
extern BROWSERENTRY * browserList;

enum
{
	ICON_NONE,
	ICON_FOLDER,
	ICON_SD,
	ICON_USB,
	ICON_DVD,
	ICON_SMB
};

extern unsigned long SNESROMSize;
extern bool loadingFile;
extern char szname[MAXPATHLEN];
extern bool inSz;

bool MakeFilePath(char filepath[], int type, char * filename = NULL, int filenum = -2);
int UpdateDirName();
int OpenGameList();
int autoLoadMethod();
int autoSaveMethod(bool silent);
int FileSortCallback(const void *f1, const void *f2);
void StripExt(char* returnstring, char * inputstring);
bool IsSz();
void ResetBrowser();
bool AddBrowserEntry();
bool IsDeviceRoot(char * path);
int BrowserLoadSz();
int BrowserChangeFolder();
int BrowserLoadFile();

#endif
