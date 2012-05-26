#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include <vector>
#include <string>
#include "smartptr.hpp"
#include "gecko.h"

#define EXECUTE_ADDR	((u8 *)0x92000000)
#define BOOTER_ADDR		((u8 *)0x93000000)
#define ARGS_ADDR		((u8 *)0x93200000)

using namespace std;

extern const u8 app_booter_bin[];
extern const u32 app_booter_bin_size;

extern const u8 stub_bin[];
extern const u32 stub_bin_size;

typedef void (*entrypoint) (void);
extern "C" { void __exception_closeall(); }

static u8 *homebrewbuffer = EXECUTE_ADDR;
static u32 homebrewsize = 0;
static vector<string> Arguments;

bool IsDollZ (u8 *buff)
{
	u8 dollz_stamp[] = {0x3C};
	int dollz_offs = 0x100;

	int ret = memcmp (&buff[dollz_offs], dollz_stamp, sizeof(dollz_stamp));
	if (ret == 0)
		return true;

	return false;
}

void AddBootArgument(const char * argv)
{
	string arg(argv);
	Arguments.push_back(arg);
}

int LoadHomebrew(const char *filepath)
{
	if(!filepath) 
		return -1;

	FILE *file = fopen(filepath ,"rb");
	if(!file) 
		return -2;

	fseek(file, 0, SEEK_END);
	u32 filesize = ftell(file);
	rewind(file);

	fread(homebrewbuffer, 1, filesize, file);
	fclose(file);

	homebrewsize += filesize;
	return 1;
}

static int SetupARGV(struct __argv * args)
{
	if(!args) 
		return -1;

	bzero(args, sizeof(struct __argv));
	args->argvMagic = ARGV_MAGIC;

	u32 argc = 0;
	u32 position = 0;
	u32 stringlength = 1;

	/** Append Arguments **/
	for(u32 i = 0; i < Arguments.size(); i++)
		stringlength += Arguments[i].size()+1;

	args->length = stringlength;
	//! Put the argument into mem2 too, to avoid overwriting it
	args->commandLine = (char *) ARGS_ADDR + sizeof(struct __argv);

	/** Append Arguments **/
	for(u32 i = 0; i < Arguments.size(); i++)
	{
		strcpy(&args->commandLine[position], Arguments[i].c_str());
		position += Arguments[i].size() + 1;
		argc++;
	}

	args->argc = argc;

	args->commandLine[args->length - 1] = '\0';
	args->argv = &args->commandLine;
	args->endARGV = args->argv + 1;

	Arguments.clear();

	return 0;
}

int BootHomebrew(bool wiiflow_stub)
{
	struct __argv args;
	if (!IsDollZ(homebrewbuffer))
		SetupARGV(&args);

	memcpy(BOOTER_ADDR, app_booter_bin, app_booter_bin_size);
	DCFlushRange(BOOTER_ADDR, app_booter_bin_size);

	entrypoint entry = (entrypoint)BOOTER_ADDR;

	memmove(ARGS_ADDR, &args, sizeof(args));
	DCFlushRange(ARGS_ADDR, sizeof(args) + args.length);

	if(wiiflow_stub)
	{
		/* Clear potential homebrew channel stub */
		memset((void*)0x80001800, 0, 0x1800);

		/* Copy our own stub into memory */
		memcpy((void*)0x80001800, stub_bin, stub_bin_size);

		/* Lower Title ID */
		*(vu8*)0x80001bf2 = 0x00;
		*(vu8*)0x80001bf3 = 0x01;
		*(vu8*)0x80001c06 = 0x00;
		*(vu8*)0x80001c07 = 0x08;

		/* Upper Title ID */
		*(vu8*)0x80001bfa = 0x57;
		*(vu8*)0x80001bfb = 0x49;
		*(vu8*)0x80001c0a = 0x49;
		*(vu8*)0x80001c0b = 0x48;

		DCFlushRange((void*)0x80001800, stub_bin_size);
	}

	/* Shutdown IOS subsystems */
	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();
	entry();
	IRQ_Restore(level);
	return 0;
}
