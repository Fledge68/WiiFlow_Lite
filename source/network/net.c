// Copyright 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
#include <errno.h>
#include <gccore.h>
#include <network.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>

#include "net.h"
#include "ftp.h"
#include "FTP_Dir.hpp"
#include "loader/sys.h"
#include "gecko/gecko.hpp"

#define MAX_NET_BUFFER_SIZE 32768
#define MIN_NET_BUFFER_SIZE 4096
#define FREAD_BUFFER_SIZE 32768

static u32 NET_BUFFER_SIZE = MAX_NET_BUFFER_SIZE;
s32 ftp_server = -1;

s32 set_blocking(s32 s, bool blocking) {
    s32 flags;
    flags = net_fcntl(s, F_GETFL, 0);
    if (flags >= 0) flags = net_fcntl(s, F_SETFL, blocking ? (flags&~4) : (flags|4));
    return flags;
}

s32 net_close_blocking(s32 s) {
    set_blocking(s, true);
    return net_close(s);
}

bool create_server(void) {
	ftp_init();
    ftp_server = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (ftp_server < 0) return false;
    set_blocking(ftp_server, false);

    struct sockaddr_in bindAddress;
    memset(&bindAddress, 0, sizeof(bindAddress));
    bindAddress.sin_family = AF_INET;
    bindAddress.sin_port = htons(ftp_server_port);
    bindAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    s32 ret;
    if ((ret = net_bind(ftp_server, (struct sockaddr *)&bindAddress, sizeof(bindAddress))) < 0) {
        net_close(ftp_server);
		ftp_server = -1;
        gprintf("Error binding socket: [%i] %s\n", -ret, strerror(-ret));
        return false;
    }
    if ((ret = net_listen(ftp_server, 3)) < 0) {
        net_close(ftp_server);
		ftp_server = -1;
        gprintf("Error listening on socket: [%i] %s\n", -ret, strerror(-ret));
        return false;
    }
	gprintf("FTP Server started\n");
    return true;
}

void end_server(void)
{
	if(ftp_server < 0)
		return;
	cleanup_ftp();
	net_close(ftp_server);
	ftp_server = -1;
	gprintf("FTP Server ended\n");
}

s32 get_server_num(void)
{
	return ftp_server;
}

typedef s32 (*transferrer_type)(s32 s, void *mem, s32 len);
static s32 transfer_exact(s32 s, char *buf, s32 length, transferrer_type transferrer) {
    s32 result = 0;
    s32 remaining = length;
    s32 bytes_transferred;
    set_blocking(s, true);
    while (remaining) {
        try_again_with_smaller_buffer:
        bytes_transferred = transferrer(s, buf, MIN(remaining, (int)NET_BUFFER_SIZE));
        if (bytes_transferred > 0) {
            remaining -= bytes_transferred;
            buf += bytes_transferred;
        } else if (bytes_transferred < 0) {
            if (bytes_transferred == -EINVAL && NET_BUFFER_SIZE == MAX_NET_BUFFER_SIZE) {
                NET_BUFFER_SIZE = MIN_NET_BUFFER_SIZE;
                goto try_again_with_smaller_buffer;
            }
            result = bytes_transferred;
            break;
        } else {
            result = -ENODATA;
            break;
        }
    }
    set_blocking(s, false);
    return result;
}

s32 send_exact(s32 s, char *buf, s32 length) {
    return transfer_exact(s, buf, length, (transferrer_type)net_write);
}

s32 send_from_file(s32 s, FILE *f) {
    char buf[FREAD_BUFFER_SIZE];
    s32 bytes_read;
    s32 result = 0;

    DCFlushRange(buf, FREAD_BUFFER_SIZE);
    ICInvalidateRange(buf, FREAD_BUFFER_SIZE);
    bytes_read = fread(buf, 1, FREAD_BUFFER_SIZE, f);
    if (bytes_read > 0) {
        DCFlushRange(buf, bytes_read);
        result = send_exact(s, buf, bytes_read);
        if (result < 0) goto end;
    }
    if (bytes_read < FREAD_BUFFER_SIZE) {
        result = -!feof(f);
        goto end;
    }
    return -EAGAIN;
    end:
    return result;
}

s32 recv_to_file(s32 s, FILE *f) {
    char buf[NET_BUFFER_SIZE];
    s32 bytes_read;
    while (1) {
        try_again_with_smaller_buffer:
        DCFlushRange(buf, NET_BUFFER_SIZE);
        ICInvalidateRange(buf, NET_BUFFER_SIZE);
        bytes_read = net_read(s, buf, NET_BUFFER_SIZE);
        if (bytes_read < 0) {
            if (bytes_read == -EINVAL && NET_BUFFER_SIZE == MAX_NET_BUFFER_SIZE) {
                NET_BUFFER_SIZE = MIN_NET_BUFFER_SIZE;
                goto try_again_with_smaller_buffer;
            }
            return bytes_read;
        } else if (bytes_read == 0) {
            return 0;
        }
        DCFlushRange(buf, bytes_read);
        s32 bytes_written = fwrite(buf, 1, bytes_read, f);
        if (bytes_written < bytes_read) return -1;
    }
}
