
#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

#define EXECUTE_ADDR	((u8 *)0x92000000)
#define BOOTER_ADDR		((u8 *)0x93000000)
#define ARGS_ADDR		((u8 *)0x93200000)

int BootHomebrew();
int SetupARGV(struct __argv * args);
void AddBootArgument(const char * arg);
int LoadHomebrew(const char * filepath);
void JumpToBooter();
void writeStub();

#endif
