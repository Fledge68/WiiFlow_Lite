#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

int BootHomebrew();
int SetupARGV(struct __argv * args);
void AddBootArgument(const char * arg);
int LoadHomebrew(const char * filepath);
void writeStub();

#endif
