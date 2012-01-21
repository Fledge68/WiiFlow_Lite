#ifndef _DNS_H_
#define _DNS_H_

#include <network.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h> //usleep

u32 getipbyname(char *domain);
u32 getipbynamecached(char *domain);

#endif /* _DNS_H_ */
