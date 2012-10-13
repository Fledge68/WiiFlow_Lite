/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/* This code comes from HBC's stub which was based on dhewg's geckoloader stub */
// Copyright 2008-2009  Andre Heider  <dhewg@wiibrew.org>
// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

#include "utils.h"
#include "debug.h"
#include "usb.h"
#include "ios.h"
#include "cache.h"
#include "di.h"
#include "disc.h"
#include "apploader.h"
#include "patchcode.h"
#include "Config.h"
#include "video.h"
#include "fst.h"

IOS_Info CurrentIOS;
static the_CFG *conf = (the_CFG*)0x93100000;

//static struct ioctlv vecs[16] ALIGNED(64);
static u32 disc_id[0x40] ALIGNED(32);
static u32 AppEntrypoint = 0;
static u8 tmd[0x5000] ALIGNED(64);

static struct {
	u32 count;
	u32 offset;
	u32 pad[6];
} part_table_info ALIGNED(32);

static struct {
	u32 offset;
	u32 type;
} partition_table[32] ALIGNED(32);

//static int es_fd;

void fail(void);

static s32 ios_getversion()
{
	u32 vercode;
	u16 version;
	sync_before_read((void*)0x80003140,8);
	vercode = *((u32*)0x80003140);
	version = vercode >> 16;
	if(version == 0) return -1;
	if(version > 0xff) return -1;
	return version;
}

static void printversion(void)
{
	debug_string("IOS version: ");
	debug_uint(ios_getversion());
	debug_string("\n");
}

static u8 conf_buffer[0x4000] ALIGNED(32);

static u32 get_counter_bias(void)
{
	int fd;
	u32 bias = 0;

	fd = ios_open("/shared2/sys/SYSCONF", 1);
	if(ios_read(fd, conf_buffer, 0x4000) != 0x4000) {
		debug_string("Failed to get conf buffer\n");
		fail();
		//goto finish;
	}
	debug_string("Read SYSCONF\n");

	u16 count = *((u16*)(&conf_buffer[4]));
	u16 *offset = (u16*)&conf_buffer[6];

	while(count--) {
		if(/*(6 == ((conf_buffer[*offset]&0x0F)+1)) &&*/ !memcmp("IPL.CB", &conf_buffer[*offset+1], 6))
			break;
		offset++;
	}
	if(count == -1) {
		debug_string("Failed to find IPL.CB setting\n");
		fail();
		//goto finish;
	}
	u8 *b = &conf_buffer[*offset];
	//memcpy(&bias, &conf_buffer[*offset+7], 4);
	bias = (b[7]<<24) | (b[8]<<16) | (b[9]<<8) | b[10];
	debug_string("Counter bias: ");
	debug_uint(bias);
	debug_string("\n");

//finish:
	ios_close(fd);
	return bias;
}

static u32 read_rtc(void)
{
	u32 rtc;
	exi_chan0sr = 0x00000130;
	exi_chan0data = 0x20000000;
	exi_chan0cr = 0x39;
	while((exi_chan0cr)&1);
	exi_chan0cr = 0x39;
	while((exi_chan0cr)&1);
	rtc = exi_chan0data;
	debug_string("RTC: ");
	debug_uint(rtc);
	debug_string("\n");
	exi_chan0sr = 0x00000030;
	//udelay(100);
	return rtc;
}

static inline void write32(u32 w, u32 addr)
{
  *((u32 *)(addr + 0x80000000)) = w;
}

static inline void write16(u16 w, u16 addr)
{
  *((u16 *)(addr + 0x80000000)) = w;
}

void fail(void)
{
	debug_string("\nFAILURE\n");
	void (*entrypoint)(void) = (void(*)(void))0x80001800;
	entrypoint();
	while(1);
}

#ifdef DEBUG
#define FAILNOTONE(val,msg) do { int __v = (val); if(__v != 1) { debug_string(msg "Value: "); debug_uint(__v); debug_string("\n"); fail(); } } while(0)
#else
#define FAILNOTONE(val,msg) failnotone((val))

static void failnotone(int v) __attribute__((noinline));
static void failnotone(int v) {
	if(v != 1)
		fail();
}
#endif

u32 hooktype;
extern s32 wbfsDev;
extern u32 wbfs_part_idx;
extern FragList *frag_list;

void _main (void)
{
	int ret;
	int i;
	//u64 disc_ios;
	u32 bias;
	u32 rtc = 0;
	u32 rtc2 = 1;
	u64 tbtime;
	u32 tmp;

	debug_string("WiiFlow External Booter by FIX94 - based on TinyLoad v0.2\n");
	video_init();
	ipc_init();
	printversion();

	bias = get_counter_bias();
	while(rtc != rtc2) {
		rtc2 = rtc;
		rtc = read_rtc();
	}
	
	tbtime = ((u64)((u32)(rtc+bias)))*(243000000u/4);
	asm volatile(
		"li             %0,0\n\
		mttbl  %0\n\
		mttbu  %1\n\
		mttbl  %2\n"
		: "=&r" (tmp)
		: "b" ((u32)(tbtime >> 32)), "b" ((u32)(tbtime & 0xFFFFFFFF))
	);
	debug_string("Initializing DI\n");
	if(di_init() < 0) {
		debug_string("di_init() failed\n");
		fail();
	}
	prog10();

	app_gameconfig_set(conf->gameconf, conf->gameconfsize);
	ocarina_set_codes(conf->codelist, conf->codelistend, conf->cheats, conf->cheatSize);
	debuggerselect = conf->debugger;
	hooktype = conf->hooktype;
	frag_list = conf->fragments;
	wbfsDev = conf->wbfsDevice;
	wbfs_part_idx = conf->wbfsPart;
	configbytes[0] = conf->configbytes[0];
	configbytes[1] = conf->configbytes[1];

	if(conf->GameBootType == TYPE_WII_DISC)
	{
		Disc_SetUSB(NULL, 0);
		if(CurrentIOS.Type == IOS_TYPE_HERMES)
			Hermes_Disable_EHC();
	}
	else
	{
		Disc_SetUSB((u8*)conf->gameID, conf->GameBootType == TYPE_WII_WBFS_EXT);
		if(CurrentIOS.Type == IOS_TYPE_HERMES)
			Hermes_shadow_mload(conf->mload_rev);
	}

	prog10();
	//di_closepartition();
	if((ret = di_getcoverstatus()) != 2) {
		debug_string("di_getcoverstatus() failed\n");
		fail();
	}
	debug_string("Resetting drive\n");
	FAILNOTONE(di_reset(), "di_reset() failed\n");
	prog10();
	debug_string("Identifying disc\n");
	FAILNOTONE(di_identify(), "di_identify() failed\n");
	prog(40);
	debug_string("Reading Disc ID\n");
	FAILNOTONE(di_readdiscid(disc_id), "di_readdiscid() failed\n");
	prog10();
	//*(vu32*)0xcd8000c0 |= 0x100;
	debug_string("Gamecode: ");
	debug_uint(disc_id[0]);
	debug_string("\n");
	if(disc_id[6] != 0x5d1c9ea3) {
		debug_string("Not a valid Wii disc!\n");
		fail();
	}
	debug_string("Reading partition table info\n");
	FAILNOTONE(di_unencryptedread(&part_table_info, sizeof(part_table_info), 0x10000), "Read failed\n");
	prog10();
	debug_string("Reading partition table\n");
	FAILNOTONE(di_unencryptedread(partition_table, sizeof(partition_table), part_table_info.offset), "Read failed\n");
	prog10();
	for(i=0; i<part_table_info.count; i++) {
		if(partition_table[i].type == 0) {
			break;
		}
	}
	if(i >= part_table_info.count) {
		debug_string("Could not find valid partition table\n");
		fail();
	}
	debug_string("Opening partition at ");
	debug_uint(partition_table[i].offset);
	debug_string("\n");
	FAILNOTONE(di_openpartition(partition_table[i].offset, tmd), "Failed to open partition");
	prog10();
	debug_string("Reading partition header\n");
	FAILNOTONE(di_read((void*)0x80000000, 0x20, 0), "Failed to read partition header");
	prog10();

	CurrentIOS = conf->IOS;
	AppEntrypoint = Apploader_Run(conf->vidMode, conf->vipatch, conf->countryString,
								conf->patchVidMode, conf->aspectRatio, conf->returnTo);
	debug_string("Apploader complete\n");

	prog10();

	extern u32 vimode;
	if(((u8*)disc_id)[3] == 'P' && vimode == 0)
		vimode = 1;
	debug_string("VI mode: ");
	debug_uint(vimode);
	debug_string("\n");

	Disc_SetLowMem();
	*(vu32*)0x800000cc = vimode;
	sync_after_write((void*)0x80000000, 0x3f00);

	setprog(320);
	udelay(60000);

	debug_string("Game entry: ");
	debug_uint(AppEntrypoint);
	debug_string("\n");

	debug_string("ok, taking the plunge\n");
	video_setblack();
	di_shutdown();

	*(vu32*)0xCD006C00 = 0x00000000;	// deinit audio due to libogc fail

	/* Originally from tueidj - taken from NeoGamma (thx) */
	*(vu32*)0xCC003024 = 1;

	if(AppEntrypoint == 0x3400)
	{
		if(hooktype)
		{
			asm volatile (
				"lis %r3, returnpoint@h\n"
				"ori %r3, %r3, returnpoint@l\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"nop\n"
				"mtctr %r3\n"
				"bctr\n"
				"returnpoint:\n"
				/*"bl DCDisable\n"
				"bl ICDisable\n"*/
				"li %r3, 0\n"
				"mtsrr1 %r3\n"
				"lis %r4, AppEntrypoint@h\n"
				"ori %r4,%r4,AppEntrypoint@l\n"
				"lwz %r4, 0(%r4)\n"
				"mtsrr0 %r4\n"
				"rfi\n"
			);
		}
		else
		{
			asm volatile (
				"isync\n"
				"lis %r3, AppEntrypoint@h\n"
				"ori %r3, %r3, AppEntrypoint@l\n"
				"lwz %r3, 0(%r3)\n"
				"mtsrr0 %r3\n"
				"mfmsr %r3\n"
				"li %r4, 0x30\n"
				"andc %r3, %r3, %r4\n"
				"mtsrr1 %r3\n"
				"rfi\n"
			);
		}
	}
	else if(hooktype)
	{
		asm volatile (
			"lis %r3, AppEntrypoint@h\n"
			"ori %r3, %r3, AppEntrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"lis %r3, 0x8000\n"
			"ori %r3, %r3, 0x18A8\n"
			"nop\n"
			"mtctr %r3\n"
			"bctr\n"
		);
	}
	else
	{
		asm volatile (
			"lis %r3, AppEntrypoint@h\n"
			"ori %r3, %r3, AppEntrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}
	fail();
}

