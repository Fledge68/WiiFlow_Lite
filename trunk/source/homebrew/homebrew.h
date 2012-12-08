
#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

typedef void (*entry)(void);

void BootHomebrew();
void AddBootArgument(const char * arg);
int LoadHomebrew(const char * filepath);
void JumpToEntry(entry EntryPoint);
void writeStub();

#endif
