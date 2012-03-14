#ifndef _NAND_H_
#define _NAND_H_

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>

#define REAL_NAND	0
#define EMU_SD		1
#define EMU_USB		2

#define DOWNLOADED_CHANNELS	0x00010001
#define SYSTEM_CHANNELS		0x00010002
#define RF_NEWS_CHANNEL		0x48414741
#define RF_FORECAST_CHANNEL	0x48414641

/* 'NAND Device' structure */
typedef struct nandDevice
{
	const char *Name;
	u32 Mode;
	u32 Mount;
	u32 Unmount;
} NandDevice; 

typedef struct _config_header 
{
	u8		magic[4];
	u16		ncnt;
	u16		noff[];
} config_header;

using namespace std;

class Nand
{
	public:
		static Nand * Instance();
		static void DestroyInstance();

		/* Prototypes */
		void Init(string path, u32 partition, bool disable = false);
		s32 Enable_Emu();
		s32 Disable_Emu();

		void Set_Partition(u32 partition) { Partition = partition; };
		void Set_FullMode(bool fullmode) { FullMode = fullmode ? 0x100 : 0; };
		void Set_RCMode(bool rcmode) { FullMode = rcmode ? 0x40 : 0; };

		const char * Get_NandPath(void) { return NandPath; };
		u32 Get_Partition(void) { return Partition; };

		void Set_NandPath(string path);
		
		s32 CreateConfig(const char *path);
		s32 Do_Region_Change(string id, const char *path);

	private:
		Nand() : MountedDevice(0), EmuDevice(REAL_NAND), Disabled(true), Partition(0), FullMode(0x100), NandPath() {}
		~Nand(void){}

		/* Prototypes */
		s32 Nand_Mount(NandDevice *Device);
		s32 Nand_Unmount(NandDevice *Device);
		s32 Nand_Enable(NandDevice *Device);
		s32 Nand_Disable(void);
		void __Dec_Enc_TB(void);
		void __configshifttxt(char *str);
		s32 __configread(void);
		s32 __configwrite(void);
		u32 __configsetbyte(const char *item, u8 val);
		u32 __configsetbigarray(const char *item, void *val, u32 size);
		u32 __configsetsetting(const char *item, const char *val);
		void __CreatePath(const char *path, ...);
		

		u32 MountedDevice;
		u32 EmuDevice;
		bool Disabled;

		u32 Partition ATTRIBUTE_ALIGN(32);
		u32 FullMode ATTRIBUTE_ALIGN(32);
		char NandPath[32] ATTRIBUTE_ALIGN(32);
		char cfgpath[1024] ATTRIBUTE_ALIGN(32);
		char settxtpath[1024] ATTRIBUTE_ALIGN(32);

		static Nand * instance;
};

#endif
