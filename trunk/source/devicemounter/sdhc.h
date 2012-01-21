#ifndef _SDHC_H_
#define _SDHC_H_

/* Constants */
#define SDHC_SECTOR_SIZE	0x200

/* Disc interfaces */
extern const DISC_INTERFACE __io_sdhc;

/* Prototypes */
bool SDHC_Init(void);
bool SDHC_Close(void);
bool SDHC_ReadSectors(u32, u32, void *);
bool SDHC_WriteSectors(u32, u32, void *);
extern int sdhc_mode_sd;
extern int sdhc_inited;

#endif
