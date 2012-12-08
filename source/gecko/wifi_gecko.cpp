/****************************************************************************
 * Copyright (C) 2010 by Dimok
 *           (C) 2012 by FIX94
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "wifi_gecko.hpp"
#include "loader/utils.h"

/* set to use TCP socket instead of UDP */
//#define WIFI_GECKO_USE_TCP 1

WifiGecko WiFiDebugger;

WifiGecko::WifiGecko()
{
	connection = -1;
	inited = false;
	buffer = true;
	dest_ip = NULL;
	dest_port = 0;
	memset(wifigeckobuffer, 0, WIFIGECKO_SIZE);
}

void WifiGecko::SetBuffer(bool buf)
{
	buffer = buf;
}

void WifiGecko::Init(const char *ip, const u16 port)
{
	dest_ip = ip;
	dest_port = port;
	inited = true;
}

void WifiGecko::Close()
{
    if(connection >= 0)
        net_close(connection);

    connection = -1;
	inited = false;
	dest_ip = NULL;
	dest_port = 0;
}

int WifiGecko::Connect()
{
	if(inited == false)
		return -2;

    if(connection != -1 || dest_ip == NULL || dest_port == 0)
        return connection;

	int tmp_con = -1;
	memset(&connect_addr, 0, sizeof(connect_addr));
#ifdef WIFI_GECKO_USE_TCP
	connect_addr.sin_family = PF_INET;
    tmp_con = net_socket(connect_addr.sin_family, SOCK_STREAM, 0);
#else
	connect_addr.sin_family = AF_INET;
    tmp_con = net_socket(connect_addr.sin_family, SOCK_DGRAM, IPPROTO_IP);
#endif
    if(tmp_con < 0)
        return -3;

	connect_addr.sin_port = htons(dest_port);
	inet_aton(dest_ip, &connect_addr.sin_addr);

    if(net_connect(tmp_con, (sockaddr*)&connect_addr, sizeof(connect_addr)) < 0)
	{
        Close();
		return -4;
	}
	connection = tmp_con;
	return connection;
}

int WifiGecko::Send(const char *data, int datasize)
{
	if(buffer == false)
		return -1;

	if((strlen(wifigeckobuffer) + datasize) < WIFIGECKO_SIZE)
		strcat(wifigeckobuffer, data);

	if(Connect() < 0)
		return connection;

	u32 sendsize = strlen(wifigeckobuffer);

	while(net_get_status() == -EBUSY)
		usleep(100);
	int ret = net_send(connection, wifigeckobuffer, sendsize, 0);
	if(ret < 0)
		Close();

	memset(wifigeckobuffer, 0, WIFIGECKO_SIZE);
    return ret;
}
