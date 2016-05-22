/*
	nfs.h
	Simple functionality for mounting and unmounting of NFS-based network storage.

 Copyright (c) 2012 r-win

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


#ifndef _LIBNFS_H
#define _LIBNFS_H

#ifdef __cplusplus
extern "C" {
#endif

#define NFS_READWRITE 0
#define NFS_READONLY 1

/*
Mount the network storage specified by the ipAddress of the server, and the mountdirectory 
for the mountpoint.
You can then access the filesystem using "name:/".
This will also call net_init in order to initialize the network. If you want the network to 
initiliaze async, make sure you've done so before calling nfsMount.
*/
extern bool nfsMount(const char *name, const char *ipAddress, const char *mountdir);

extern bool nfsMountEx(const char *name, const char *ipAddress, const char *mountdir, uint32_t uid, uint32_t gid, uint32_t readonly);

/*
Unmount the remote mountpoint specified by name.
*/
extern void nfsUnmount (const char* name);

#ifdef __cplusplus
}
#endif

#endif // _LIBNFS_H
