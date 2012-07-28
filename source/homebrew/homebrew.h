#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

int BootHomebrew();
void AddBootArgument(const char * arg);
int LoadHomebrew(const char * filepath);
void writeStub();

#endif
