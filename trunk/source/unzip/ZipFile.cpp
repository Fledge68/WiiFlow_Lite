/****************************************************************************
 * Copyright (C) 2009
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
 * ZipFile.cpp
 *
 * ZipFile Class
 * for Wii-FileXplorer 2009
 *
 * STILL UNCOMPLETE AND UNDER CONSTRUCTION
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ZipFile.h"
#include "fileOps/fileOps.h"
#include "memory/mem2.hpp"

ZipFile::ZipFile(const char *filepath)
{
	File = unzOpen(filepath);
	if(File)
		this->LoadList();
}

ZipFile::~ZipFile()
{
	unzClose(File);
}

bool ZipFile::LoadList()
{
	return true;
}

bool ZipFile::ExtractAll(const char *dest)
{
	if(!File)
		return false;

	bool Stop = false;

	u32 blocksize = 1024 * 50;
	u8 *buffer = (u8*)MEM2_memalign(32, blocksize);
	if(buffer == NULL)
		return false;

	char writepath[MAXPATHLEN];
	char filename[MAXPATHLEN];
	memset(filename, 0, sizeof(filename));

	int ret = unzGoToFirstFile(File);
	if(ret != UNZ_OK)
		Stop = true;

	while(!Stop)
	{
		if(unzGetCurrentFileInfo(File, &cur_file_info, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK)
			Stop = true;

		if(!Stop && filename[strlen(filename) - 1] != '/')
		{
			u32 uncompressed_size = cur_file_info.uncompressed_size;

			u32 done = 0;
			char *pointer = NULL;

			ret = unzOpenCurrentFile(File);

			snprintf(writepath, sizeof(writepath), "%s/%s", dest, filename);

			pointer = strrchr(writepath, '/');
			int position = pointer - writepath + 2;

			char temppath[strlen(writepath)];
			snprintf(temppath, position, "%s", writepath);

			fsop_MakeFolder(temppath);

			if(ret == UNZ_OK)
			{
				FILE *pfile = fopen(writepath, "wb");

				do
				{
					if(uncompressed_size - done < blocksize)
						blocksize = uncompressed_size - done;

					ret = unzReadCurrentFile(File, buffer, blocksize);

					if(ret == 0)
						break;

					fwrite(buffer, 1, blocksize, pfile);

					done += ret;

				} while(done < uncompressed_size);

				fclose(pfile);
				unzCloseCurrentFile(File);
			}
		}
		if(unzGoToNextFile(File) != UNZ_OK)
			Stop = true;
	}
	MEM2_free(buffer);
	buffer = NULL;

	return true;
}
