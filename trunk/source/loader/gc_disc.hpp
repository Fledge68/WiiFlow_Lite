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
typedef void (*message_callback_t)(int message, int info, char *cinfo, void *user_data);

enum spcall
{
	KB = 0,
	BL,
	MB,
	GB,
};

class GCDump
{
public:
	void Init(bool skip, bool comp, bool wexf, bool align, u32 nretry, u32 rsize, const char* partition, const char* m_DMLgameDir, progress_callback_t i_spinner, message_callback_t i_message, void *i_udata)
	{
		skiponerror = skip;
		compressed = comp;
		writeexfiles = wexf;
		force_32k_align = align;
		gc_nbrretry = nretry;
		gc_readsize = rsize;
		gamepartition = partition;
		usb_dml_game_dir = m_DMLgameDir;
		gc_skipped = 0;
		spinner = i_spinner;
		message = i_message;
		u_data = i_udata;
		waitonerror = true;		
	}
	s32 DumpGame( );
	s32 CheckSpace(u32 *needed, bool comp);
	u32 GetFreeSpace(char *path, u32 Value);
private:
	progress_callback_t spinner;
	message_callback_t message;
	void *u_data;
	bool force_32k_align;
	bool skiponerror;
	bool compressed;
	bool writeexfiles;
	bool waitonerror;
	bool gamedone;
	bool multigamedisc;
	const char *gamepartition;
	const char *usb_dml_game_dir;
	char minfo[74];
	u8 Disc;
	u8 Disc2;
	u32 gc_nbrretry;
	u32 gc_error;
	u32 gc_retry;
	u32 gc_skipped;
	u32 gc_readsize;
	u32 gc_done;
	u32 ID;
	u32 ID2;	
	u32 ApploaderSize;
	u32 DOLOffset;
	u32 DOLSize;
	u32 FSTOffset;
	u32 FSTSize;
	u32 FSTTotal;
	u32 FSTEnt;
	u32 GamePartOffset;
	u32 DataSize;
	u32 DiscSize;
	u32 DiscSizeCalculated;
	u32 MultiGameCnt;
	u32 MultiGameDump;
	u32 Gamesize[10];
	u64 NextOffset;
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
	s32 __DiscReadRaw(void *outbuf, u64 offset, u32 length);
	s32 __DiscWrite(char * path, u64 offset, u32 length, u8 *ReadBuffer);
	s32 __DiscWriteFile(FILE *f, u64 offset, u32 length, u8 *ReadBuffer);
	void __AnalizeMultiDisc();
	bool __WaitForDisc(u8 dsc, u32 msg);
};
#endif