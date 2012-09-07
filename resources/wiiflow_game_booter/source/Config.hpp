
#ifndef _CFG_HPP_
#define _CFG_HPP_

#include "cios.h"

typedef struct _the_CFG {
	u8 vidMode;
	bool vipatch;
	bool countryString;
	u8 patchVidMode;
	int aspectRatio;
	u32 returnTo;
	u8 configbytes[2];
	IOS_Info IOS;
	void *codelist;
	u8 *codelistend;
	u8 *cheats;
	u32 cheatSize;
	u32 hooktype;
	u8 debugger;
	u32 *gameconf;
	u32 gameconfsize;
	u8 BootType;
	/* needed for channels */
	u64 title;
} the_CFG;

static the_CFG *conf = (the_CFG*)0x90000000;

#endif /* _CFG_HPP_ */
