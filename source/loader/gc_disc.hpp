/***************************************************************************
 * Copyright (C) 2012
 * by OverjoY and FIX94 for Wiiflow
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
 * gc_disc.hpp
 *
 ***************************************************************************/

#ifndef GC_DISC_H_
#define GC_DISC_H_

typedef void (*progress_callback_t)(int status,int total,void *user_data);

class GCDump
{
public:
	void Init(bool skip, bool comp, bool wexf, bool align, u32 nretry, u32 rsize)
	{
		skiponerror = skip;
		compressed = comp;
		writeexfiles = wexf;
		force_32k_align = align;
		gc_nbrretry = nretry;
		gc_readsize = rsize;
		gc_skipped = 0;
	}
	s32 DumpGame(progress_callback_t spinner, void *spinner_data);
	s32 CheckSpace(u32 *needed, bool comp);
private:
	bool force_32k_align;
	bool skiponerror;
	bool compressed;
	bool writeexfiles;
	u32 gc_nbrretry;
	u32 gc_error;
	u32 gc_retry;
	u32 gc_skipped;
	u32 gc_readsize;
	u32 DiscSize;
	typedef struct
	{
		union
		{
			struct
			{
				u32 Type		:8;
				u32 NameOffset	:24;
			};
			u32 TypeName;
		};
		union
		{
			struct
			{
				u32 FileOffset;
				u32 FileLength;
			};
			struct
			{
				u32 ParentOffset;
				u32 NextOffset;
			};
			u32 entry[2];
		};
	} FST;
	s32 __DiscReadRaw(void *outbuf, u32 offset, u32 length);
	s32 __DiscWrite(char * path, u32 offset, u32 length, progress_callback_t spinner , void *spinner_data);
	s32 __DiscWriteAligned(FILE *f, u32 offset, u32 length);
};
#endif