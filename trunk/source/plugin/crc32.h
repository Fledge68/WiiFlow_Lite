
#ifndef CRC32_H
#define CRC32_H

#ifdef __cplusplus
extern "C"
{
#endif

#define UPDC32(octet, crc) (crc_32_tab[((crc)\
			^ (octet)) & 0xff] ^ ((crc) >> 8))

u32 crc32file(char *name);

#ifdef __cplusplus
}
#endif
#endif /* CRC32_H */
