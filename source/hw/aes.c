/****************************************************************************
 * Copyright (C) 2013 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <string.h>
#include <gctypes.h>
#include <malloc.h>
#include <ogc/cache.h>
#include <ogc/system.h>
#include <ogc/machine/processor.h>
#include "loader/utils.h"
#include "memory/memory.h"
#include "aes.h"

#define AES_CMD_FLAG_EXEC (1<<31)
#define AES_CMD_FLAG_ENA  (1<<28)
#define AES_CMD_FLAG_DEC  (1<<27)
#define AES_CMD_FLAG_IV   (1<<12)

u8 aes_mode = 0;

void AES_ResetEngine(void)
{
	/* Reset Engine */
	write32(HW_AES_CMD, 0x00000000);
	while ((read32(HW_AES_CMD) & AES_CMD_FLAG_EXEC) != 0);
}

void AES_EnableDecrypt(const u8 *key, const u8 *iv)
{
	const u32 *aes_iv = (const u32*)iv;
	const u32 *aes_key = (const u32*)key;
	/* write in IV and Key */
	u8 i;
	for(i = 0; i < 4; ++i)
	{
		write32(HW_AES_IV, *(aes_iv+i));
		write32(HW_AES_KEY, *(aes_key+i));
	}
}

#define AES_LIMIT 0x1000
static u8 AES_BUF[AES_LIMIT*16] ATTRIBUTE_ALIGN(16); /* 64KB */
void AES_Decrypt(u8 *inbuf, u8 *outbuf, u16 num_blocks)
{
	/* set mode back to 0 */
	aes_mode = 0;
	/* split cause of limit */
	u32 buf_pos = 0;
	u16 blocks_done = 0;
	u32 buf_done = 0;
	u16 blocks_full = num_blocks;
	while(blocks_done < blocks_full)
	{
		/* Calculate Position */
		blocks_done = blocks_full;
		if(blocks_done > AES_LIMIT)
			blocks_done = AES_LIMIT;
		buf_done = blocks_done * 16;
		/* Copy into our aligned buffer */
		memcpy(AES_BUF, (inbuf + buf_pos), buf_done);
		/* Flush and Invalidate */
		DCFlushRange(AES_BUF, buf_done);
		ICInvalidateRange(AES_BUF, buf_done);
		/* set location */
		write32(HW_AES_SRC, (u32)MEM_VIRTUAL_TO_PHYSICAL(AES_BUF));
		write32(HW_AES_DEST, (u32)MEM_VIRTUAL_TO_PHYSICAL(AES_BUF));
		/* enable engine, set size, set iv flag and enable decryption */
		write32(HW_AES_CMD, AES_CMD_FLAG_ENA | AES_CMD_FLAG_DEC | AES_CMD_FLAG_EXEC | 
						((aes_mode == 0) ? 0 : AES_CMD_FLAG_IV) | (blocks_done - 1));
		while (read32(HW_AES_CMD) & AES_CMD_FLAG_EXEC);
		/* set down num blocks */
		blocks_full -= blocks_done;
		/* increase aes mode to keep iv */
		if(aes_mode == 0) aes_mode++;
		/* flush and write outbuf */
		DCFlushRange(AES_BUF, buf_done);
		memcpy((outbuf + buf_pos), AES_BUF, buf_done);
		/* increase block pos */
		buf_pos += buf_done;
	}
}
