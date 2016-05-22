/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * gcunzip.cpp
 *
 * File unzip routines
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>

#include "snes9xgx.h"
#include "fileop.h"
#include "filebrowser.h"
#include "menu.h"
#include "gcunzip.h"

extern "C" {
#include "utils/sz/7zCrc.h"
#include "utils/sz/7zIn.h"
#include "utils/sz/7zExtract.h"
}

#define ZIPCHUNK 2048

/*
 * Zip file header definition
 */
typedef struct
{
  unsigned int zipid __attribute__ ((__packed__));	// 0x04034b50
  unsigned short zipversion __attribute__ ((__packed__));
  unsigned short zipflags __attribute__ ((__packed__));
  unsigned short compressionMethod __attribute__ ((__packed__));
  unsigned short lastmodtime __attribute__ ((__packed__));
  unsigned short lastmoddate __attribute__ ((__packed__));
  unsigned int crc32 __attribute__ ((__packed__));
  unsigned int compressedSize __attribute__ ((__packed__));
  unsigned int uncompressedSize __attribute__ ((__packed__));
  unsigned short filenameLength __attribute__ ((__packed__));
  unsigned short extraDataLength __attribute__ ((__packed__));
}
PKZIPHEADER;

/*
 * Zip files are stored little endian
 * Support functions for short and int types
 */
static u32
FLIP32 (u32 b)
{
	unsigned int c;

	c = (b & 0xff000000) >> 24;
	c |= (b & 0xff0000) >> 8;
	c |= (b & 0xff00) << 8;
	c |= (b & 0xff) << 24;

	return c;
}

static u16
FLIP16 (u16 b)
{
	u16 c;

	c = (b & 0xff00) >> 8;
	c |= (b & 0xff) << 8;

	return c;
}

/****************************************************************************
 * IsZipFile
 *
 * Returns TRUE when 0x504b0304 is first four characters of buffer
 ***************************************************************************/
int
IsZipFile (char *buffer)
{
	unsigned int *check = (unsigned int *) buffer;

	if (check[0] == 0x504b0304)
		return 1;

	return 0;
}

/*****************************************************************************
* UnZipBuffer
******************************************************************************/

size_t
UnZipBuffer (unsigned char *outbuffer)
{
	PKZIPHEADER pkzip;
	size_t zipoffset = 0;
	size_t zipchunk = 0;
	char out[ZIPCHUNK];
	z_stream zs;
	int res;
	size_t bufferoffset = 0;
	size_t have = 0;
	char readbuffer[ZIPCHUNK];
	size_t sizeread = 0;

	// Read Zip Header
	fseek(file, 0, SEEK_SET);
	sizeread = fread (readbuffer, 1, ZIPCHUNK, file);

	if(sizeread <= 0)
		return 0;

	/*** Copy PKZip header to local, used as info ***/
	memcpy (&pkzip, readbuffer, sizeof (PKZIPHEADER));

	pkzip.uncompressedSize = FLIP32 (pkzip.uncompressedSize);

	ShowProgress ("Loading...", 0, pkzip.uncompressedSize);

	/*** Prepare the zip stream ***/
	memset (&zs, 0, sizeof (z_stream));
	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;
	zs.avail_in = 0;
	zs.next_in = Z_NULL;
	res = inflateInit2 (&zs, -MAX_WBITS);

	if (res != Z_OK)
		goto done;

	/*** Set ZipChunk for first pass ***/
	zipoffset =
	(sizeof (PKZIPHEADER) + FLIP16 (pkzip.filenameLength) +
	FLIP16 (pkzip.extraDataLength));
	zipchunk = ZIPCHUNK - zipoffset;

	/*** Now do it! ***/
	do
	{
		zs.avail_in = zipchunk;
		zs.next_in = (Bytef *) & readbuffer[zipoffset];

		/*** Now inflate until input buffer is exhausted ***/
		do
		{
			zs.avail_out = ZIPCHUNK;
			zs.next_out = (Bytef *) & out;

			res = inflate (&zs, Z_NO_FLUSH);

			if (res == Z_MEM_ERROR)
			{
				goto done;
			}

			have = ZIPCHUNK - zs.avail_out;
			if (have)
			{
				/*** Copy to normal block buffer ***/
				memcpy (&outbuffer[bufferoffset], &out, have);
				bufferoffset += have;
			}
		}
		while (zs.avail_out == 0);

		// Readup the next 2k block
		zipoffset = 0;
		zipchunk = ZIPCHUNK;

		sizeread = fread (readbuffer, 1, ZIPCHUNK, file);
		if(sizeread <= 0)
			goto done; // read failure

		ShowProgress ("Loading...", bufferoffset, pkzip.uncompressedSize);
	}
	while (res != Z_STREAM_END);

done:
	inflateEnd (&zs);
	CancelAction();

	if (res == Z_STREAM_END)
		return pkzip.uncompressedSize;
	else
		return 0;
}

/****************************************************************************
* GetFirstZipFilename
*
* Returns the filename of the first file in the zipped archive
* The idea here is to do the least amount of work required
***************************************************************************/

char *
GetFirstZipFilename ()
{
	char * firstFilename = NULL;
	char tempbuffer[ZIPCHUNK];
	char filepath[1024];

	if(!MakeFilePath(filepath, FILE_ROM))
		return NULL;

	// read start of ZIP
	if(LoadFile (tempbuffer, filepath, ZIPCHUNK, NOTSILENT) < 35)
		return NULL;

	tempbuffer[28] = 0; // truncate - filename length is 2 bytes long (bytes 26-27)
	int namelength = tempbuffer[26]; // filename length starts 26 bytes in

	if(namelength < 0 || namelength > 200) // filename is not a reasonable length
	{
		ErrorPrompt("Error - Invalid ZIP file!");
		return NULL;
	}
	
	firstFilename = &tempbuffer[30]; // first filename of a ZIP starts 31 bytes in
	firstFilename[namelength] = 0; // truncate at filename length
	return strdup(firstFilename);
}

/****************************************************************************
* 7z functions
***************************************************************************/

typedef struct _SzFileInStream
{
   ISzInStream InStream;
   u64 offset; // offset of the file
   unsigned int len; // length of the file
   u64 pos;  // current position of the file pointer
} SzFileInStream;

// 7zip error list
static char szerrormsg[][100] = {
   "File is corrupt.", // 7z: Data error
   "Archive contains too many files.", // 7z: Out of memory
   "File is corrupt (CRC mismatch).", // 7z: CRC Error
   "File uses unsupported compression settings.", // 7z: Not implemented
   "File is corrupt.", // 7z: Fail
   "Failed to read file data.", // 7z: Data read failure
   "File is corrupt.", // 7z: Archive error
   "File uses too high of compression settings (dictionary size is too large).", // 7z: Dictionary too large
};

static SZ_RESULT SzRes;
static SzFileInStream SzArchiveStream;
static CArchiveDatabaseEx SzDb;
static ISzAlloc SzAllocImp;
static ISzAlloc SzAllocTempImp;
static UInt32 SzBlockIndex = 0xFFFFFFFF;
static size_t SzBufferSize;
static size_t SzOffset;
static size_t SzOutSizeProcessed;
static CFileItem *SzF;

static char sz_buffer[2048];
static int szMethod = 0;

/****************************************************************************
* Is7ZipFile
*
* Returns 1 when 7z signature is found
****************************************************************************/
int
Is7ZipFile (char *buffer)
{
	// 7z signature
	static Byte Signature[6] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};

	int i;
	for(i = 0; i < 6; i++)
		if(buffer[i] != Signature[i])
			return 0;

	return 1; // 7z archive found
}

// display an error message
static void SzDisplayError(SZ_RESULT res)
{
	char msg[1024];
	sprintf(msg, "7z decompression failed: %s", szerrormsg[(res - 1)]);
	ErrorPrompt(msg);
}

// function used by the 7zip SDK to read data from SD/USB/DVD/SMB
static SZ_RESULT SzFileReadImp(void *object, void **buffer, size_t maxRequiredSize, size_t *processedSize)
{
	size_t sizeread = 0;

	if(maxRequiredSize == 0)
		return SZ_OK;

	// the void* object is a SzFileInStream
	SzFileInStream *s = (SzFileInStream *) object;

	if (maxRequiredSize > 2048)
		maxRequiredSize = 2048;

	// read data
	sizeread = fread(sz_buffer, 1, maxRequiredSize, file);

	if(sizeread <= 0)
		return SZE_FAILREAD;

	*buffer = sz_buffer;
	*processedSize = sizeread;
	s->pos += sizeread;

	if(sizeread > 1024) // only show progress for large reads
		// this isn't quite right, but oh well
		ShowProgress ("Loading...", s->pos, browserList[browser.selIndex].length);

	return SZ_OK;
}

// function used by the 7zip SDK to change the filepointer
static SZ_RESULT SzFileSeekImp(void *object, CFileSize pos)
{
	// the void* object is a SzFileInStream
	SzFileInStream *s = (SzFileInStream *) object;

	// check if the 7z SDK wants to move the pointer to somewhere after the EOF
	if (pos >= s->len)
		return SZE_FAIL;

	// save new position and return
	if(fseek(file, (long)pos, SEEK_SET) != 0)
		return SZE_FAIL;

	s->pos = pos;
	return SZ_OK;
}

/****************************************************************************
* SzClose
*
* Closes a 7z file
***************************************************************************/

void SzClose()
{
	if(SzDb.Database.NumFiles > 0)
		SzArDbExFree(&SzDb, SzAllocImp.Free);
}


/****************************************************************************
* SzParse
*
* Opens a 7z file, and parses it
* It parses the entire 7z for full browsing capability
***************************************************************************/

int SzParse(char * filepath)
{
	if(!filepath)
		return 0;
	
	int device;
	
	struct stat filestat;
	if(stat(filepath, &filestat) < 0)
		return 0;
	unsigned int filelen = filestat.st_size;
	
	if(!FindDevice(filepath, &device) || !filelen)
		return 0;

	int nbfiles = 0;
	// setup archive stream
	SzArchiveStream.offset = 0;
	SzArchiveStream.len = filelen;
	SzArchiveStream.pos = 0;

	// open file
	file = fopen (filepath, "rb");
	if(!file)
		return 0;

	// set szMethod to current chosen load device
	szMethod = device;

	// set handler functions for reading data from SD/USB/SMB/DVD
	SzArchiveStream.InStream.Read = SzFileReadImp;
	SzArchiveStream.InStream.Seek = SzFileSeekImp;

	// set default 7Zip SDK handlers for allocation and freeing memory
	SzAllocImp.Alloc = SzAlloc;
	SzAllocImp.Free = SzFree;
	SzAllocTempImp.Alloc = SzAllocTemp;
	SzAllocTempImp.Free = SzFreeTemp;

	// prepare CRC and 7Zip database structures
	InitCrcTable();
	SzArDbExInit(&SzDb);

	// open the archive
	SzRes = SzArchiveOpen(&SzArchiveStream.InStream, &SzDb, &SzAllocImp,
			&SzAllocTempImp);

	if (SzRes != SZ_OK)
	{
		SzDisplayError(SzRes);
		// free memory used by the 7z SDK
		SzClose();
	}
	else // archive opened successfully
	{
		if(SzDb.Database.NumFiles > 0)
		{
			// Parses the 7z into a full file listing

			HaltParseThread(); // halt parsing
			ResetBrowser(); // reset browser

			// add '..' folder in case the user wants exit the 7z
			AddBrowserEntry();
			sprintf(browserList[0].filename, "..");
			sprintf(browserList[0].displayname, "Up One Level");
			browserList[0].isdir = 1;
			browserList[0].length = filelen;
			browserList[0].icon = ICON_FOLDER;

			// get contents and parse them into file list structure
			unsigned int SzI, SzJ;
			SzJ = 1;
			for (SzI = 0; SzI < SzDb.Database.NumFiles; SzI++)
			{
				SzF = SzDb.Database.Files + SzI;

				// skip directories
				if (SzF->IsDirectory)
					continue;

				if(!AddBrowserEntry())
				{
					ResetBrowser();
					ErrorPrompt("Out of memory: too many files!");
					SzClose();
					SzJ = 0;
					break;
				}

				// parse information about this file to the file list structure
				snprintf(browserList[SzJ].filename, MAXJOLIET, "%s", SzF->Name);
				StripExt(browserList[SzJ].displayname, browserList[SzJ].filename);
				char* strPos = strstr(browserList[SzJ].displayname, szname);
				if(strPos)
				{
					snprintf(browserList[SzJ].displayname, MAXJOLIET, "%s", strPos + strlen(szname));
				}
				
				browserList[SzJ].length = SzF->Size; // filesize
				browserList[SzJ].isdir = 0; // only files will be displayed (-> no flags)
				browserList[SzJ].filenum = SzI; // the extraction function identifies the file with this number
				SzJ++;
			}
			nbfiles = SzJ;
		}
		else
		{
			SzClose();
		}
	}

	CancelAction();

	// close file
	fclose(file);
	return nbfiles;
}

/****************************************************************************
* SzExtractFile
*
* Extracts the given file # into the buffer specified
* Must parse the 7z BEFORE running this function
***************************************************************************/

size_t SzExtractFile(int i, unsigned char *buffer)
{
	// prepare some variables
	SzBlockIndex = 0xFFFFFFFF;
	SzOffset = 0;

	// Unzip the file

	SzRes = SzExtract2(
		&SzArchiveStream.InStream,
		&SzDb,
		i,                      // index of file
		&SzBlockIndex,          // index of solid block
		&buffer,
		&SzBufferSize,
		&SzOffset,              // offset of stream for required file in *outBuffer
		&SzOutSizeProcessed,    // size of file in *outBuffer
		&SzAllocImp,
		&SzAllocTempImp);

	// close 7Zip archive and free memory
	SzClose();

	CancelAction();

	// check for errors
	if(SzRes != SZ_OK)
	{
		// display error message
		SzDisplayError(SzRes);
		return 0;
	}
	else
	{
		return SzOutSizeProcessed;
	}
}
