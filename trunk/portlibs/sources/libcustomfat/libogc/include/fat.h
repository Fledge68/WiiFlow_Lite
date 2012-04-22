/*
	fat.h
	Simple functionality for startup, mounting and unmounting of FAT-based devices.
	
 Copyright (c) 2006 Michael "Chishm" Chisholm
	
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _LIBFAT_H
#define _LIBFAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ogc/disc_io.h>

/*
Initialise any inserted block-devices.
Add the fat device driver to the devoptab, making it available for standard file functions.
cacheSize: The number of pages to allocate for each inserted block-device
setAsDefaultDevice: if true, make this the default device driver for file operations
*/
extern bool fatInit (uint32_t cacheSize, bool setAsDefaultDevice);

/*
Calls fatInit with setAsDefaultDevice = true and cacheSize optimised for the host system.
*/
extern bool fatInitDefault (void);

/*
Mount the device pointed to by interface, and set up a devoptab entry for it as "name:".
You can then access the filesystem using "name:/".
This will mount the active partition or the first valid partition on the disc, 
and will use a cache size optimized for the host system.
*/
extern bool fatMountSimple (const char* name, const DISC_INTERFACE* interface);

/*
Mount the device pointed to by interface, and set up a devoptab entry for it as "name:".
You can then access the filesystem using "name:/".
If startSector = 0, it will mount the active partition of the first valid partition on
the disc. Otherwise it will try to mount the partition starting at startSector.
cacheSize specifies the number of pages to allocate for the cache.
This will not startup the disc, so you need to call interface->startup(); first.
*/
extern bool fatMount (const char* name, const DISC_INTERFACE* interface, sec_t startSector, uint32_t cacheSize, uint32_t SectorsPerPage);

/*
Unmount the partition specified by name.
If there are open files, it will attempt to synchronise them to disc.
*/
extern void fatUnmount (const char* name);

/*
Get Volume Label
*/
extern void fatGetVolumeLabel (const char* name, char *label);

#ifdef __cplusplus
}
#endif

#endif // _LIBFAT_H
