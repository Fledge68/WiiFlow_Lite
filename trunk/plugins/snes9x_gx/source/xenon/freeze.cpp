#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snes9x.h"
#include "memmap.h"
#include "soundux.h"
#include "snapshot.h"
#include "srtc.h"

int
GetMem (char *buffer, int len)
{
	printf("%s:%d\n", __FILE__, __LINE__); while (1);
}

void
NGCFreezeBlock (char *name, uint8 * block, int size)
{
	printf("%s:%d\n", __FILE__, __LINE__); while (1);
}

/****************************************************************************
 * NGCFreezeMembuffer
 *
 * Copies a snapshot of Snes9x state into memory
 ***************************************************************************/
static int
NGCFreezeMemBuffer ()
{
	printf("%s:%d\n", __FILE__, __LINE__); while (1);

    return 0;
}


/****************************************************************************
 * NGCFreezeGame
 *
 * Do freeze game for Nintendo Gamecube
 ***************************************************************************/
int
NGCFreezeGame (char * filepath, int method, bool silent)
{
	printf("%s:%d\n", __FILE__, __LINE__); while (1);
}

int
NGCFreezeGameAuto (int method, bool silent)
{
	printf("%s:%d\n", __FILE__, __LINE__); while (1);
}

/****************************************************************************
 * NGCUnFreezeBlock
 ***************************************************************************/
int
NGCUnFreezeBlock (char *name, uint8 * block, int size)
{
	printf("%s:%d\n", __FILE__, __LINE__); while (1);
}

/****************************************************************************
 * NGCUnfreezeGame
 ***************************************************************************/
int
NGCUnfreezeGame (char * filepath, int method, bool silent)
{
	printf("%s:%d\n", __FILE__, __LINE__); while (1);
}
