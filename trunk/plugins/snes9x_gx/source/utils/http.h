/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * Tantric 2008-2010
 *
 * http.h
 *
 * HTTP operations
 ***************************************************************************/

#ifndef _HTTP_H_
#define _HTTP_H_

typedef enum {
	HTTPR_OK,
	HTTPR_ERR_CONNECT,
	HTTPR_ERR_REQUEST,
	HTTPR_ERR_STATUS,
	HTTPR_ERR_TOOBIG,
	HTTPR_ERR_RECEIVE
} http_res;

int http_request (const char *url, FILE * hfile, u8 * buffer, const u32 max_size, bool silent);

#endif
