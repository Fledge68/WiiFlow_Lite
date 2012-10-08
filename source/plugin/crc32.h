
#ifndef CRC32_H
#define CRC32_H

#ifdef __cplusplus
extern "C"
{
#endif

#define UPDC32(octet, crc) (crc_32_tab[((crc)\
			^ (octet)) & 0xff] ^ ((crc) >> 8))

u32 crc32file(char *name);
u32 crc32buffer(const u8 *s, const u32 len);

#ifdef __cplusplus
}
#endif
#endif /* CRC32_H */
