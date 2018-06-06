#ifndef _HTTP_H_
#define _HTTP_H_

#include <errno.h>
#include <network.h>
#include <ogcsys.h>
#include <string.h>

#include "dns.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * A simple structure to keep track of the size of a malloc()ated block of memory 
 */
struct block
{
	u32 size;
	unsigned char *data;
};

extern const struct block emptyblock;

struct block downloadfile(const char *url);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HTTP_H_ */
