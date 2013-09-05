// Copyright 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
#ifndef _NET_H_
#define _NET_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void initialise_network();
s32 set_blocking(s32 s, bool blocking);
s32 net_close_blocking(s32 s);
bool create_server(void);
s32 get_server_num(void);
void end_server(void);
s32 send_exact(s32 s, char *buf, s32 length);
s32 send_from_file(s32 s, FILE *f);
s32 recv_to_file(s32 s, FILE *f);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NET_H_ */
