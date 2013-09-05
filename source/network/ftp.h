// Copyright 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
#ifndef _FTP_H_
#define _FTP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void accept_ftp_client(s32 server);
void set_ftp_password(const char *new_password);
bool process_ftp_events(s32 server);
void cleanup_ftp();

extern bool ftp_allow_active;
extern u16 ftp_server_port;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FTP_H_ */
