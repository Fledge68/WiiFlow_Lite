/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;
typedef volatile unsigned long long vu64;

typedef volatile signed char vs8;
typedef volatile signed short vs16;
typedef volatile signed int vs32;
typedef volatile signed long long vs64;

typedef unsigned int size_t;
typedef signed int ssize_t;

#define NULL ((void *)0)

#define ALIGNED(n) __attribute__((aligned(n)))

enum
{
	TYPE_WII_DISC = 0,
	TYPE_WII_WBFS,
	TYPE_WII_WBFS_EXT,
};

enum
{
    TYPE_WII_GAME = 0,
    TYPE_GC_GAME,
    TYPE_CHANNEL,
    TYPE_PLUGIN,
    TYPE_HOMEBREW,
    TYPE_END
};
#define NoGameID(x)			(x == TYPE_PLUGIN || x == TYPE_HOMEBREW)

enum
{
	IOS_TYPE_D2X = 0,
	IOS_TYPE_WANIN,
	IOS_TYPE_HERMES,
	IOS_TYPE_KWIIRK,
	IOS_TYPE_NEEK2O,
	IOS_TYPE_NORMAL_IOS,
	IOS_TYPE_STUB,
};
#define CustomIOS(x)		(x != IOS_TYPE_NORMAL_IOS && x != IOS_TYPE_STUB)

#endif

