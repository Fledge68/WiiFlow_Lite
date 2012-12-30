#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <vector>
#include <string>
#include "homebrew.h"
#include "gecko/gecko.hpp"

#define EXECUTE_ADDR	((u8 *)0x91000000)
#define BOOTER_ADDR		((u8 *)0x93000000)
#define ARGS_ADDR		((u8 *)0x93200000)
#define BOOTER_ENTRY	((entry)BOOTER_ADDR)

using namespace std;

extern const u8 app_booter_bin[];
extern const u32 app_booter_bin_size;

extern const u8 stub_bin[];
extern const u32 stub_bin_size;

u32 buffer_size = 0;

static vector<string> Arguments;

static bool IsDollZ(u8 *buf)
{
	u8 cmp1[] = {0x3C};
	return memcmp(&buf[0x100], cmp1, sizeof(cmp1)) == 0;
}

static bool IsSpecialELF(u8 *buf)
{
	u32 cmp1[] = {0x7F454C46};
	u8 cmp2[] = {0x00};
	return memcmp(buf, cmp1, sizeof(cmp1)) == 0 && memcmp(&buf[0x24], cmp2, sizeof(cmp2)) == 0;
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

	buffer_size = filesize;
	fread(EXECUTE_ADDR, 1, buffer_size, file);
	DCFlushRange(EXECUTE_ADDR, buffer_size);
	fclose(file);

	return 1;
}

int SetupARGV(struct __argv * args)
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

void writeStub()
{
	/* Clear potential homebrew channel stub */
	memset((void*)0x80001800, 0, 0x1800);

	/* Copy our own stub into memory */
	memcpy((void*)0x80001800, stub_bin, stub_bin_size);
	DCFlushRange((void*)0x80001800, stub_bin_size);
}

void BootHomebrew()
{
	__argv args;
	if(!IsDollZ(EXECUTE_ADDR) && !IsSpecialELF(EXECUTE_ADDR))
		SetupARGV(&args);
	else
		gprintf("Homebrew Boot Arguments disabled\n");

	memcpy(BOOTER_ADDR, app_booter_bin, app_booter_bin_size);
	DCFlushRange(BOOTER_ADDR, app_booter_bin_size);

	memmove(ARGS_ADDR, &args, sizeof(args));
	DCFlushRange(ARGS_ADDR, sizeof(args) + args.length);

	JumpToEntry(BOOTER_ENTRY);
}

void JumpToEntry(entry EntryPoint)
{
	gprintf("Jumping to %08x\n", EntryPoint);
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	__lwp_thread_stopmultitasking(EntryPoint);
}
