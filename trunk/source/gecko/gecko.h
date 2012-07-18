

#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C" {
#endif

	extern bool geckoinit;
	extern bool bufferMessages;
	extern bool WriteToSD;

	//use this just like printf();
	void gprintf(const char *format, ...);
	void ghexdump(void *d, int len);
	bool InitGecko();
	void AllocSDGeckoBuffer();
	void ClearLogBuffer();
	void GeckoDisable();

#ifdef __cplusplus
}
#endif

#endif
