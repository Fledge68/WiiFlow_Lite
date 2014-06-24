
#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

typedef void (*entry)(void);

void BootHomebrew();
void AddBootArgument(const char *argv);
void AddBootArgument(const char *argv, unsigned int size);
bool LoadHomebrew(const char *filepath);
char *GetHomebrew(unsigned int *size);
bool LoadAppBooter(const char *filepath);
void JumpToEntry(entry EntryPoint);
void writeStub();

#endif
