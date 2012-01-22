/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *
 *  this file is part of GeckoOS for USB Gecko
 *  http://www.usbgecko.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __DVD_BROADWAY_H__
#define __DVD_BROADWAY_H__
 
#include <gctypes.h>
#include <ogc/ipc.h>
#include <ogc/dvd.h>
 
#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */
 
typedef void (*dvdcallbacklow)(s32 result);
 
s32 bwDVD_LowInit();
s32 bwDVD_LowClose();
s32 bwDVD_LowInquiry(dvddrvinfo *info,dvdcallbacklow cb);
s32 bwDVD_LowReadID(dvddiskid *diskID,dvdcallbacklow cb);
s32 bwDVD_LowClosePartition(dvdcallbacklow cb);
s32 bwDVD_LowOpenPartition(u32 offset,void *eticket,u32 certin_len,void *certificate_in,void *certificate_out,dvdcallbacklow cb);
s32 bwDVD_LowUnencryptedRead(void *buf,u32 len,u32 offset,dvdcallbacklow cb);
s32 bwDVD_LowReset(dvdcallbacklow cb);
s32 bwDVD_LowWaitCoverClose(dvdcallbacklow cb);
s32 bwDVD_LowRead(void *buf,u32 len,u32 offset,dvdcallbacklow cb);
s32 bwDVD_EnableVideo(dvdcallbacklow cb);
s32 bwDVD_LowReadVideo(void *buf,u32 len,u32 offset,dvdcallbacklow cb);
s32 bwDVD_SetDecryption(s32 mode, dvdcallbacklow cb);
s32 bwDVD_SetOffset(u32 offset, dvdcallbacklow cb);
s32 bwDVD_LowStopMotor(u8 stop1,u8 stop2);
s32 bwDVD_GetCoverStatus(u32 *status);
s32 bwDVD_LowRequestError( void );
s32 bwDVD_LowAudioStatus( void );
s32 bwDVD_LowClearCoverInterrupt(dvdcallbacklow cb);
s32 bwDVD_LowReportKey(dvdcallbacklow cb);
//s32 bwDVD_SetDiscType(bool dvd5);
s32 SetWBFSMode(u8 device, void*discid);
s32 setstreaming();

s32 Sneek_DVDReadGameInfo( u32 Offset, u32 Length, void *Data );
s32 Sneek_DVDSelectGame( u32 SlotID );
s32 Sneek_DVDGetGameCount( u32 *Count );

#ifdef __cplusplus
   }
#endif /* __cplusplus */
 
#endif
