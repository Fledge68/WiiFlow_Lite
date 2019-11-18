/*
    Code by blackb0x @ GBAtemp.net
    This allows the Wii to download from servers that use SNI.
*/
#ifndef _HTTPS_H_
#define _HTTPS_H_

#include <libwolfssl/ssl.h>
#include "dns.h"

#ifdef __cplusplus
extern "C"
{
#endif

// #define DEBUG_NETWORK
#define REDIRECT_LIMIT 3
#define TCP_CONNECT_TIMEOUT 5000
#define READ_WRITE_TIMEOUT 10

    struct download
    {
        u8 skip_response;   // Used by WiinnerTag
        u64 size;
        char *data;
    };

    typedef struct
    {
        u8 use_https;
        s32 sock;
        WOLFSSL *ssl;
        WOLFSSL_CTX *ctx;
    } HTTP_INFO;

    void downloadfile(const char *url, struct download *buffer);
    int wolfSSL_CTX_UseSNI(WOLFSSL_CTX *ctx, unsigned char type, const void *data, unsigned short size);

#ifdef __cplusplus
}
#endif

#endif /* _HTTPS_H_ */
