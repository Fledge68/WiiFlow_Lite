/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/* This code comes from HBC's stub which was based on dhewg's geckoloader stub */
// Copyright 2008-2009  Andre Heider  <dhewg@wiibrew.org>
// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>
#ifndef _VIDEO_TINYLOAD_H_
#define _VIDEO_TINYLOAD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void video_init(void);
void video_clear(void);
void prog(int p);
void prog10(void);
void setprog(int p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
